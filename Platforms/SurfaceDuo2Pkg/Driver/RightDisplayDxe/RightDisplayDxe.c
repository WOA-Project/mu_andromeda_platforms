/** @file
  RightDisplayDxe: Surface Duo 2 right display (DSI1) bring-up and GOP.

  Panel supplies, the ext-trigger clock and the MDSS clocks are checked and
  only enabled when XBL left them off (the left panel shares them). This driver
  then brings up DSI1 PHY/PLL/clocks, the DSI
  controller and panel, the DPU pipeline (DMA2 -> LM_2 -> PP_2 -> DSC_2 ->
  INTF_2 via CTL_1) and a second GOP backed by its own framebuffer.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#include <Library/FrameBufferBltLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformHwioLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>

/// Defines
/*
 * Convert enum video_log2_bpp to bytes and bits. Note we omit the outer
 * brackets to allow multiplication by fractional pixels.
 */
#define VNBYTES(bpix) (1 << (bpix)) / 8
#define VNBITS(bpix) (1 << (bpix))

#define POS_TO_FB(posX, posY)                                                  \
  ((UINT8                                                                      \
        *)((UINTN)This->Mode->FrameBufferBase + (posY)*This->Mode->Info->PixelsPerScanLine * FB_BYTES_PER_PIXEL + (posX)*FB_BYTES_PER_PIXEL))

#define FB_BITS_PER_PIXEL (32)
#define FB_BYTES_PER_PIXEL (FB_BITS_PER_PIXEL / 8)
#define DISPLAYDXE_PHYSICALADDRESS32(_x_) (UINTN)((_x_)&0xFFFFFFFF)

#define DISPLAYDXE_RED_MASK 0xFF0000
#define DISPLAYDXE_GREEN_MASK 0x00FF00
#define DISPLAYDXE_BLUE_MASK 0x0000FF
#define DISPLAYDXE_ALPHA_MASK 0x000000

/*
 * Bits per pixel selector. Each value n is such that the bits-per-pixel is
 * 2 ^ n
 */
enum video_log2_bpp {
  VIDEO_BPP1 = 0,
  VIDEO_BPP2,
  VIDEO_BPP4,
  VIDEO_BPP8,
  VIDEO_BPP16,
  VIDEO_BPP32,
};

typedef struct {
  VENDOR_DEVICE_PATH DisplayDevicePath;
  EFI_DEVICE_PATH    EndDevicePath;
} DISPLAY_DEVICE_PATH;

DISPLAY_DEVICE_PATH mDisplayDevicePath =
{
  {
    {
      HARDWARE_DEVICE_PATH,
      HW_VENDOR_DP,
      {
        (UINT8)sizeof (VENDOR_DEVICE_PATH),
        (UINT8)(sizeof (VENDOR_DEVICE_PATH) >> 8)
      }
    },
    RIGHT_DISPLAY_VENDOR_GUID
  },
  {
    END_DEVICE_PATH_TYPE,
    END_ENTIRE_DEVICE_PATH_SUBTYPE,
    {
      sizeof (EFI_DEVICE_PATH_PROTOCOL),
      0
    }
  }
};

/// Declares

STATIC FRAME_BUFFER_CONFIGURE *mFrameBufferBltLibConfigure;
STATIC UINTN mFrameBufferBltLibConfigureSize;

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
  RightQueryMode, RightSetMode, RightBlt, NULL};

STATIC
EFI_STATUS
EFIAPI
RightQueryMode(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber,
    OUT UINTN *SizeOfInfo, OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION **Info)
{
  EFI_STATUS Status;
  Status = gBS->AllocatePool(
      EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
      (VOID **)Info);

  ASSERT_EFI_ERROR(Status);

  *SizeOfInfo                   = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  (*Info)->Version              = This->Mode->Info->Version;
  (*Info)->HorizontalResolution = This->Mode->Info->HorizontalResolution;
  (*Info)->VerticalResolution   = This->Mode->Info->VerticalResolution;
  (*Info)->PixelFormat          = This->Mode->Info->PixelFormat;
  (*Info)->PixelsPerScanLine    = This->Mode->Info->PixelsPerScanLine;

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
RightSetMode(IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This, IN UINT32 ModeNumber)
{
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
RightBlt(
    IN EFI_GRAPHICS_OUTPUT_PROTOCOL *This,
    IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL *BltBuffer,
    OPTIONAL IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION BltOperation,
    IN UINTN SourceX, IN UINTN SourceY, IN UINTN DestinationX,
    IN UINTN DestinationY, IN UINTN Width, IN UINTN Height,
    IN UINTN Delta OPTIONAL)
{
  RETURN_STATUS Status;
  EFI_TPL       Tpl;
  //
  // We have to raise to TPL_NOTIFY, so we make an atomic write to the frame
  // buffer. We would not want a timer based event (Cursor, ...) to come in
  // while we are doing this operation.
  //
  Tpl    = gBS->RaiseTPL(TPL_NOTIFY);
  Status = FrameBufferBlt(
      mFrameBufferBltLibConfigure, BltBuffer, BltOperation, SourceX, SourceY,
      DestinationX, DestinationY, Width, Height, Delta);
  gBS->RestoreTPL(Tpl);

  return RETURN_ERROR(Status) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

//
// Place the right framebuffer right after the left (XBL) one inside the
// reserved display region, falling back to a dedicated reserved allocation.
//
STATIC
EFI_STATUS
AllocateRightFrameBuffer (
  IN  UINTN                 Size,
  OUT EFI_PHYSICAL_ADDRESS  *Address
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX Region;
  UINT64                          LeftSize;
  UINT64                          Start;

  ZeroMem (&Region, sizeof (Region));
  GetPlatformDisplayFrameBuffer (&Region);

  LeftSize = (UINT64)FixedPcdGet32 (PcdMipiFrameBufferWidth) *
             FixedPcdGet32 (PcdMipiFrameBufferHeight) * RIGHT_FB_BYTES_PER_PIXEL;
  Start = ALIGN_VALUE (Region.Address + LeftSize, (UINT64)RIGHT_FB_ALIGNMENT);

  if (Region.Address != 0 && Start + Size <= (UINT64)Region.Address + Region.Length) {
    *Address = Start;
    return EFI_SUCCESS;
  }

  *Address = 0xFFFFFFFF;
  return gBS->AllocatePages (
           AllocateMaxAddress, EfiReservedMemoryType, EFI_SIZE_TO_PAGES (Size),
           Address);
}

EFI_STATUS
EFIAPI
RightDisplayDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{

  EFI_STATUS Status             = EFI_SUCCESS;
  EFI_HANDLE hUEFIDisplayHandle = NULL;

  EFI_PHYSICAL_ADDRESS  MipiFrameBufferAddr;
  BOOLEAN               PanelUp;
  UINTN                 FbSize = (UINTN)PANEL_WIDTH * PANEL_HEIGHT * RIGHT_FB_BYTES_PER_PIXEL;

#if RIGHT_DISPLAY_DIAG
  gBS->Stall (MS (10));

  ResetLeftConsole ();
#endif

  Status = EFI_ABORTED;

  PowerEnablePanelSupplies ();
  PowerEnableExtTrigClock ();

  TlmmConfigGpioOutput (PANEL_RESET_GPIO, FALSE);

  // TE input from the panel, pull-down, 2 mA drive as in the downstream sde_te_active state
  TlmmConfigFunction (PANEL_TE_GPIO, PANEL_TE_GPIO_FUNC, 1, 0);
  DispCcEnableMdssClocks ();
  DsiPhyEnable ();
  Status = DsiPllEnable ();
  if (!EFI_ERROR (Status)) {
    Status = DispCcEnableDsi1Clocks ();
  }
  if (!EFI_ERROR (Status)) {
    Status = DsiCtrlInit ();
    if (!EFI_ERROR (Status)) {
      DsiCtrlSetupStream ();
      Status = PanelReset ();
#if !RIGHT_DISPLAY_INIT_AFTER_PIPE
      if (!EFI_ERROR (Status)) {
        Status = PanelSendInit ();
      }
#endif
    }
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "RightDisplay: panel bring-up failed: %r\n", Status));
  }
  PanelUp = !EFI_ERROR (Status);

  Status = AllocateRightFrameBuffer (FbSize, &MipiFrameBufferAddr);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  /* Prepare struct */
  if (mDisplay.Mode == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE),
        (VOID **)&mDisplay.Mode);

    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
      return Status;

    ZeroMem(mDisplay.Mode, sizeof(EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  }

  if (mDisplay.Mode->Info == NULL) {
    Status = gBS->AllocatePool(
        EfiBootServicesData, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION),
        (VOID **)&mDisplay.Mode->Info);

    ASSERT_EFI_ERROR(Status);
    if (EFI_ERROR(Status))
      return Status;

    ZeroMem(mDisplay.Mode->Info, sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  }

  /* Set information */
  mDisplay.Mode->MaxMode       = 1;
  mDisplay.Mode->Mode          = 0;
  mDisplay.Mode->Info->Version = 0;

  mDisplay.Mode->Info->HorizontalResolution = PANEL_WIDTH;
  mDisplay.Mode->Info->VerticalResolution   = PANEL_HEIGHT;

  /* RightDisplayDxe runs on a8r8g8b8 (VIDEO_BPP32) for WoA devices */
  UINT32               LineLength = PANEL_WIDTH * VNBYTES(VIDEO_BPP32);
  UINT32               FrameBufferSize    = LineLength * PANEL_HEIGHT;
  EFI_PHYSICAL_ADDRESS FrameBufferAddress = MipiFrameBufferAddr;

  mDisplay.Mode->Info->PixelsPerScanLine = PANEL_WIDTH;
  mDisplay.Mode->Info->PixelFormat = PixelBlueGreenRedReserved8BitPerColor;
  mDisplay.Mode->SizeOfInfo      = sizeof(EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  mDisplay.Mode->FrameBufferBase = FrameBufferAddress;
  mDisplay.Mode->FrameBufferSize = FrameBufferSize;

  /* Create the FrameBufferBltLib configuration. */
  Status = FrameBufferBltConfigure(
      (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->Info,
      mFrameBufferBltLibConfigure, &mFrameBufferBltLibConfigureSize);

  if (Status == RETURN_BUFFER_TOO_SMALL) {
    mFrameBufferBltLibConfigure = AllocatePool(mFrameBufferBltLibConfigureSize);
    if (mFrameBufferBltLibConfigure != NULL) {
      Status = FrameBufferBltConfigure(
          (VOID *)(UINTN)mDisplay.Mode->FrameBufferBase, mDisplay.Mode->Info,
          mFrameBufferBltLibConfigure, &mFrameBufferBltLibConfigureSize);
    }
  }

  ASSERT_EFI_ERROR(Status);
  ZeroMem((VOID *)FrameBufferAddress, FrameBufferSize);

#if RIGHT_DISPLAY_DIAG
  DrawTestPattern ((UINT32 *)(UINTN)FrameBufferAddress);
#endif

  // Supplies and clocks were brought up (or confirmed on) at the start
  if (PanelUp) {
#if RIGHT_DISPLAY_MIRROR_LEFT
    // Diagnostic: fetch the left display's own framebuffer, which XBL's
    // setup is known to scan out. The log then appears on the right too.
    {
      ARM_MEMORY_REGION_DESCRIPTOR_EX  LeftFb;

      if (!EFI_ERROR (GetPlatformDisplayFrameBuffer (&LeftFb))) {
        DEBUG ((DEBUG_ERROR, "RD: mirroring left fb at %lx\n", (UINT64)LeftFb.Address));
        Status = DpuStart (LeftFb.Address);
      } else {
        Status = DpuStart (FrameBufferAddress);
      }
    }
#else
    DEBUG ((DEBUG_ERROR, "RD: starting right fb at %lx\n", (UINT64)FrameBufferAddress));
    Status = DpuStart (FrameBufferAddress);
#endif
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "RightDisplay: DPU start failed: %r\n", Status));
    } else {
      // Let the first frame (and the first TE) go through before lighting the panel
      gBS->Stall (MS (40));
#if RIGHT_DISPLAY_INIT_AFTER_PIPE
      // Pipe and autorefresh running first, then INIT + PPS (probe reads are
      // unreliable under continuous autorefresh, so none are done here)
      DpuEnableAutoRefresh ();
      Status = PanelSendInit ();
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "RightDisplay: panel init failed: %r\n", Status));
      }
      Status = PanelDisplayOn ();
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "RightDisplay: display on failed: %r\n", Status));
      }
#else
      Status = PanelDisplayOn ();
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "RightDisplay: display on failed: %r\n", Status));
      }
  #if RIGHT_DISPLAY_DIAG
      // Panel state after display-on and a few frames
      gBS->Stall (MS (100));
      PanelProbe ();
  #endif
      DpuEnableAutoRefresh ();
#endif
    }
  }

  #if RIGHT_DISPLAY_DIAG
    DiagDump (PanelUp, Status);
  #endif

  /* Register handle */
  Status = gBS->InstallMultipleProtocolInterfaces(
      &hUEFIDisplayHandle, &gEfiDevicePathProtocolGuid, &mDisplayDevicePath,
      &gEfiGraphicsOutputProtocolGuid, &mDisplay, NULL);

  ASSERT_EFI_ERROR(Status);

  return Status;
}
