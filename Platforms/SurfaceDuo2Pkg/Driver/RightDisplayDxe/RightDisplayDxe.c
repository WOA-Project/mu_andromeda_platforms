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

#define RIGHT_FB_BYTES_PER_PIXEL 4
#define RIGHT_FB_ALIGNMENT       SIZE_2MB

#define RIGHT_DISPLAY_VENDOR_GUID \
  {0x5e3b7a14, 0x9c2d, 0x4f6e, {0xa1, 0x58, 0x0d, 0x73, 0xb4, 0x2c, 0x96, 0xe8}}

typedef struct {
  VENDOR_DEVICE_PATH DisplayDevicePath;
  EFI_DEVICE_PATH    EndDevicePath;
} DISPLAY_DEVICE_PATH;

STATIC DISPLAY_DEVICE_PATH mDevicePath = {
  {{HARDWARE_DEVICE_PATH,
    HW_VENDOR_DP,
    {(UINT8)sizeof (VENDOR_DEVICE_PATH), (UINT8)(sizeof (VENDOR_DEVICE_PATH) >> 8)}},
   RIGHT_DISPLAY_VENDOR_GUID},
  {END_DEVICE_PATH_TYPE,
   END_ENTIRE_DEVICE_PATH_SUBTYPE,
   {sizeof (EFI_DEVICE_PATH_PROTOCOL), 0}}};

STATIC FRAME_BUFFER_CONFIGURE *mBltConfigure;
STATIC UINTN                   mBltConfigureSize;

STATIC
EFI_STATUS
EFIAPI
RightQueryMode (
  IN  EFI_GRAPHICS_OUTPUT_PROTOCOL          *This,
  IN  UINT32                                ModeNumber,
  OUT UINTN                                 *SizeOfInfo,
  OUT EFI_GRAPHICS_OUTPUT_MODE_INFORMATION  **Info
  )
{
  if (ModeNumber != 0) {
    return EFI_INVALID_PARAMETER;
  }

  *Info = AllocateCopyPool (sizeof (*This->Mode->Info), This->Mode->Info);
  if (*Info == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  *SizeOfInfo = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EFIAPI
RightSetMode (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL  *This,
  IN UINT32                        ModeNumber
  )
{
  return (ModeNumber == 0) ? EFI_SUCCESS : EFI_UNSUPPORTED;
}

STATIC
EFI_STATUS
EFIAPI
RightBlt (
  IN EFI_GRAPHICS_OUTPUT_PROTOCOL       *This,
  IN EFI_GRAPHICS_OUTPUT_BLT_PIXEL      *BltBuffer OPTIONAL,
  IN EFI_GRAPHICS_OUTPUT_BLT_OPERATION  BltOperation,
  IN UINTN                              SourceX,
  IN UINTN                              SourceY,
  IN UINTN                              DestinationX,
  IN UINTN                              DestinationY,
  IN UINTN                              Width,
  IN UINTN                              Height,
  IN UINTN                              Delta OPTIONAL
  )
{
  RETURN_STATUS Status;
  EFI_TPL       Tpl;
  UINT8         *Row;

  Tpl    = gBS->RaiseTPL (TPL_NOTIFY);
  Status = FrameBufferBlt (
             mBltConfigure, BltBuffer, BltOperation, SourceX, SourceY,
             DestinationX, DestinationY, Width, Height, Delta);
  gBS->RestoreTPL (Tpl);

  // The DPU fetches straight from DDR, so push the touched rows out of the CPU caches
  if (!RETURN_ERROR (Status) && (BltOperation != EfiBltVideoToBltBuffer) &&
      (DestinationY < PANEL_HEIGHT)) {
    if (DestinationY + Height > PANEL_HEIGHT) {
      Height = PANEL_HEIGHT - DestinationY;
    }
    Row = (UINT8 *)(UINTN)This->Mode->FrameBufferBase +
          DestinationY * PANEL_WIDTH * RIGHT_FB_BYTES_PER_PIXEL;
    WriteBackDataCacheRange (Row, Height * PANEL_WIDTH * RIGHT_FB_BYTES_PER_PIXEL);
  }

  return RETURN_ERROR (Status) ? EFI_INVALID_PARAMETER : EFI_SUCCESS;
}

STATIC EFI_GRAPHICS_OUTPUT_PROTOCOL mDisplay = {
  RightQueryMode, RightSetMode, RightBlt, NULL};

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
  // The serial port/debug libs keep their console state right after the left
  // framebuffer, so skip a page before placing ours.
  Start = ALIGN_VALUE (Region.Address + LeftSize + SIZE_4KB, (UINT64)RIGHT_FB_ALIGNMENT);

  if (Region.Address != 0 && Start + Size <= (UINT64)Region.Address + Region.Length) {
    *Address = Start;
    return EFI_SUCCESS;
  }

  *Address = 0xFFFFFFFF;
  return gBS->AllocatePages (
           AllocateMaxAddress, EfiReservedMemoryType, EFI_SIZE_TO_PAGES (Size),
           Address);
}

//
// Color bars on the top two thirds, grayscale ramp below, white 4px border.
//
STATIC
VOID
DrawTestPattern (
  IN UINT32  *Fb
  )
{
  STATIC CONST UINT32 Bars[8] = {
    0xFFFFFF, 0xFFFF00, 0x00FFFF, 0x00FF00, 0xFF00FF, 0xFF0000, 0x0000FF, 0x000000
  };
  UINT32 X, Y, Color, Gray;

  for (Y = 0; Y < PANEL_HEIGHT; Y++) {
    for (X = 0; X < PANEL_WIDTH; X++) {
      if (Y < (PANEL_HEIGHT * 2 / 3)) {
        Color = Bars[X * 8 / PANEL_WIDTH];
      } else {
        Gray  = X * 255 / (PANEL_WIDTH - 1);
        Color = (Gray << 16) | (Gray << 8) | Gray;
      }

      if (X < 4 || Y < 4 || X >= PANEL_WIDTH - 4 || Y >= PANEL_HEIGHT - 4) {
        Color = 0xFFFFFF;
      }

      Fb[Y * PANEL_WIDTH + X] = Color;
    }
  }
}

//
// Prints every register where the right pipe differs from the left one that XBL
// brought up: DSI controller, D-PHY, and INTF. Differences in things like
// strengths, lane config or panel format are candidates for what we got wrong.
//
//
// Compact register diff of one MDP block pair (left XBL block vs ours),
// three diffs per line, at most MaxDiffs shown.
//
STATIC
VOID
DiffBlock (
  IN CONST CHAR8  *Tag,
  IN UINTN        LeftBase,
  IN UINTN        RightBase,
  IN UINTN        End,
  IN UINTN        MaxDiffs
  )
{
  UINTN  Off, Shown = 0, Total = 0, NPend = 0, k;
  UINT32 P[3][3];
  UINT32 A, B;

  for (Off = 0; Off < End; Off += 4) {
    A = MmioRead32 (LeftBase + Off);
    B = MmioRead32 (RightBase + Off);
    if (A == B) {
      continue;
    }
    Total++;
    if (Shown >= MaxDiffs) {
      continue;
    }
    Shown++;
    P[NPend][0] = (UINT32)Off;
    P[NPend][1] = A;
    P[NPend][2] = B;
    if (++NPend == 3) {
      DEBUG ((DEBUG_ERROR, "RD: %a %03x %08x %08x|%03x %08x %08x|%03x %08x %08x\n", Tag,
        P[0][0], P[0][1], P[0][2], P[1][0], P[1][1], P[1][2], P[2][0], P[2][1], P[2][2]));
      NPend = 0;
    }
  }
  for (k = 0; k < NPend; k++) {
    DEBUG ((DEBUG_ERROR, "RD: %a %03x %08x %08x\n", Tag, P[k][0], P[k][1], P[k][2]));
  }
  DEBUG ((DEBUG_ERROR, "RD: %a diffs total %d\n", Tag, (UINT32)Total));
}

STATIC
VOID
CompareWithLeft (VOID)
{
  STATIC CONST UINT16 Ctrl[] = {
    0x04, 0x08, 0x0C, 0x3C, 0x40, 0x44, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x84, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8,
    0xBC, 0xC0, 0xC4, 0xCC, 0x10C, 0x11C, 0x1B8, 0x2A8, 0x2AC, 0x2B4, 0x318,
  };
  STATIC CONST UINT16 Phy[] = {
    0x10, 0x14, 0x1C, 0x20, 0x24, 0x2C, 0x30, 0x34, 0x38, 0x3C, 0xA0,
    0xB4, 0xB8, 0xBC, 0xC0, 0xC4, 0xC8, 0xCC, 0xD0, 0xD4, 0xD8, 0xDC, 0xE0, 0xE4, 0xE8,
    0xEC, 0xF4, 0xF8, 0x100, 0x104, 0x10C, 0x110, 0x114,
  };
  STATIC CONST UINT16 LaneRegs[] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};
  STATIC CONST UINT16 Intf[] = {
    0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34,
    0x38, 0x3C, 0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60, 0x64, 0x68,
    0x84, 0x90, 0x94, 0x25C, 0x280, 0x284, 0x288, 0x28C, 0x290, 0x294, 0x298, 0x29C,
    0x2A0, 0x2A4, 0x2A8, 0x2B4, 0x2B8, 0x2BC, 0x2C0, 0x2AC, 0x2B0,
  };
  UINTN  i, n;
  UINT32 A, B;

  DEBUG ((DEBUG_ERROR, "RD: diff left(dsi0/intf1) vs right (offset left right)\n"));
  for (i = 0; i < ARRAY_SIZE (Ctrl); i++) {
    A = MmioRead32 ((UINTN)(DSI0_CTRL_BASE + Ctrl[i]));
    B = MmioRead32 ((UINTN)(DSI1_CTRL_BASE + Ctrl[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: C %03x %08x %08x\n", Ctrl[i], A, B));
    }
  }
  for (i = 0; i < ARRAY_SIZE (Phy); i++) {
    A = MmioRead32 ((UINTN)(DSI0_PHY_BASE + Phy[i]));
    B = MmioRead32 ((UINTN)(DSI1_PHY_BASE + Phy[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: P %03x %08x %08x\n", Phy[i], A, B));
    }
  }
  for (n = 0; n < 5; n++) {
    for (i = 0; i < ARRAY_SIZE (LaneRegs); i++) {
      UINTN Off = 0x200 + 0x80 * n + LaneRegs[i];
      A = MmioRead32 ((UINTN)(DSI0_PHY_BASE + Off));
      B = MmioRead32 ((UINTN)(DSI1_PHY_BASE + Off));
      if (A != B) {
        DEBUG ((DEBUG_ERROR, "RD: L %03x %08x %08x\n", Off, A, B));
      }
    }
  }
  for (i = 0; i < ARRAY_SIZE (Intf); i++) {
    A = MmioRead32 ((UINTN)(MDP_BASE + 0x35000 + Intf[i]));
    B = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + Intf[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: I %03x %08x %08x\n", Intf[i], A, B));
    }
  }

  DiffBlock ("CTL", (UINTN)MDP_BASE + 0x15000, (UINTN)MDP_BASE + RD_CTL_OFF, 0x140, 9);
  DiffBlock ("LM", (UINTN)MDP_BASE + 0x44000, (UINTN)MDP_BASE + RD_LM_OFF, 0x80, 6);
  DiffBlock ("SSPP", (UINTN)MDP_BASE + 0x4000, (UINTN)MDP_BASE + RD_SSPP_OFF, 0x180, 9);
  DiffBlock ("DSI", (UINTN)DSI0_CTRL_BASE, (UINTN)DSI1_CTRL_BASE, 0x400, 12);
  DiffBlock ("ITOP", (UINTN)MDP_BASE + 0x35000, (UINTN)MDP_BASE + 0x36000, 0x10, 3);
  // INTF status/counters (frame, line, underrun...) and the PHY PLL block
  DiffBlock ("IST", (UINTN)MDP_BASE + 0x35080, (UINTN)MDP_BASE + 0x36080, 0x80, 6);
  DiffBlock ("PLL", (UINTN)DSI0_PHY_BASE + 0x500, (UINTN)DSI1_PHY_BASE + 0x500, 0x200, 12);
  DEBUG ((
    DEBUG_ERROR, "RD: dsi intr L=%08x R=%08x (b8=mdp done, b24=err)\n",
    MmioRead32 ((UINTN)(DSI0_CTRL_BASE + 0x110)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x110))));

  // DISP_CC: CBCs (bit31 = branch off) and every non-zero RCG word
  {
    UINTN  Off, N = 0, Lines = 0;
    UINT32 V, Q[4][2];

    for (Off = 0x2000; Off < 0x2180 && Lines < 12; Off += 4) {
      V = MmioRead32 ((UINTN)(DISP_CC_BASE + Off));
      if (V == 0) {
        continue;
      }
      Q[N][0] = (UINT32)Off;
      Q[N][1] = V;
      if (++N == 4) {
        Lines++;
        DEBUG ((DEBUG_ERROR, "RD: CC %04x=%08x %04x=%08x %04x=%08x %04x=%08x\n",
          Q[0][0], Q[0][1], Q[1][0], Q[1][1], Q[2][0], Q[2][1], Q[3][0], Q[3][1]));
        N = 0;
      }
    }
    for (Off = 0; Off < N; Off++) {
      DEBUG ((DEBUG_ERROR, "RD: CC %04x=%08x\n", Q[Off][0], Q[Off][1]));
    }
  }

  // DSC core 0 (left, XBL) against our core, compact: three diffs per line,
  // at most 12 diffs, wrapper (0xF00) first because it is the most telling.
  {
    UINTN  Off, Shown = 0, Total = 0, Pass;
    UINT32 Pending[3][3];
    UINTN  NPend = 0, k;

    for (Pass = 0; Pass < 2; Pass++) {
      UINTN Start = (Pass == 0) ? 0xF00 : 0x100;
      UINTN End   = (Pass == 0) ? 0xF80 : 0x200;

      for (Off = Start; Off < End; Off += 4) {
        A = MmioRead32 ((UINTN)(MDP_BASE + 0x80000 + Off));
        B = MmioRead32 ((UINTN)(MDP_BASE + ((Pass == 0) ? RD_DSC_CTL_OFF - 0xF00 : RD_DSC_ENC_OFF - 0x100) + Off));
        if (A == B) {
          continue;
        }
        Total++;
        if (Shown >= 6) {
          continue;
        }
        Shown++;
        Pending[NPend][0] = (UINT32)Off;
        Pending[NPend][1] = A;
        Pending[NPend][2] = B;
        if (++NPend == 3) {
          DEBUG ((DEBUG_ERROR, "RD: D %03x %08x %08x|%03x %08x %08x|%03x %08x %08x\n",
            Pending[0][0], Pending[0][1], Pending[0][2],
            Pending[1][0], Pending[1][1], Pending[1][2],
            Pending[2][0], Pending[2][1], Pending[2][2]));
          NPend = 0;
        }
      }
    }
    for (k = 0; k < NPend; k++) {
      DEBUG ((DEBUG_ERROR, "RD: D %03x %08x %08x\n", Pending[k][0], Pending[k][1], Pending[k][2]));
    }
    DEBUG ((DEBUG_ERROR, "RD: dsc diffs total %d (left ours)\n", (UINT32)Total));
  }
}

//
// The left display console (FrameBufferSerialPortLib) keeps its cursor in
// memory right after the left framebuffer. Clearing the framebuffer and zeroing
// that cursor makes the RD: log start at the top of an empty screen.
//
STATIC
VOID
ResetLeftConsole (VOID)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX  Region;
  UINTN                            FbBytes;

  if (EFI_ERROR (GetPlatformDisplayFrameBuffer (&Region))) {
    return;
  }

  FbBytes = (UINTN)FixedPcdGet32 (PcdMipiFrameBufferWidth) *
            FixedPcdGet32 (PcdMipiFrameBufferHeight) *
            FixedPcdGet32 (PcdMipiFrameBufferPixelBpp) / 8;

  ZeroMem ((VOID *)(UINTN)Region.Address, FbBytes);
  ZeroMem ((VOID *)(UINTN)(Region.Address + FbBytes), sizeof (FBCON_POSITION));
}

//
// Hardware state snapshot for bring-up debugging, held on the left console.
//
STATIC
VOID
DiagDump (
  IN BOOLEAN     PanelUp,
  IN EFI_STATUS  LastStatus
  )
{
  STATIC CONST struct {
    UINT32      Off;
    CONST CHAR8 *Name;
  } Clks[] = {
    {0x207C, "ahb"}, {0x200C, "mdp"}, {0x2008, "pclk1"}, {0x2030, "byte1"},
    {0x2034, "byte1_intf"}, {0x203C, "esc1"}, {0x3000, "gdsc"},
  };
  UINTN i;
  UINT32 Frame0, Frame1;

  DEBUG ((DEBUG_ERROR, "RD: panelup=%u last=%r\n", PanelUp, LastStatus));
  (VOID)Clks;
  (VOID)i;
  DEBUG ((
    DEBUG_ERROR, "RD: pll lock=%08x phy_status=%08x\n",
    MmioRead32 ((UINTN)(DSI1_PHY_BASE + 0x500 + 0x1B0)),
    MmioRead32 ((UINTN)(DSI1_PHY_BASE + 0x140))));
  DEBUG ((
    DEBUG_ERROR, "RD: dsi ctrl=%08x clk=%08x intr=%08x mdp=%08x\n",
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x04)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x11C)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x110)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x40))));
  DEBUG ((
    DEBUG_ERROR, "RD: dsi st=%08x fifo=%08x ack=%08x lane=%08x phyerr=%08x/%08x tmo=%08x\n",
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x08)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x0C)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x68)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xA8)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xB4)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x318)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xC0))));
  DEBUG ((
    DEBUG_ERROR, "RD: ctl top=%08x flush=%08x intf_act=%08x\n",
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0x14)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0x18)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xF4))));
  DEBUG ((
    DEBUG_ERROR, "RD: ctl dsc=%08x/%08x(L) fetch=%08x/%08x(L) lay=%08x/%08x(L)\n",
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xE8)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0xE8)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xFC)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0xFC)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + RD_CTL_INDEX * 4)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0x00))));
  DEBUG ((
    DEBUG_ERROR, "RD: intf en=%08x cfg2=%08x tear=%08x\n",
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x0)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x60)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x284))));

  // INTF frame counter should advance if frames are being pushed
  Frame0 = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0xAC));
  gBS->Stall (MS (100));
  Frame1 = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0xAC));
  DEBUG ((DEBUG_ERROR, "RD: intf frame cnt %08x -> %08x\n", Frame0, Frame1));

  CompareWithLeft ();

  gBS->Stall (MS (RIGHT_DISPLAY_DIAG_HOLD_MS));
}

EFI_STATUS
EFIAPI
RightDisplayDxeInitialize (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS            Status;
  EFI_HANDLE            Handle = NULL;
  EFI_PHYSICAL_ADDRESS  FbBase;
  BOOLEAN               PanelUp;
  UINTN                 FbSize = (UINTN)PANEL_WIDTH * PANEL_HEIGHT * RIGHT_FB_BYTES_PER_PIXEL;

  gBS->Stall (MS (10));

  ResetLeftConsole ();

  Status = EFI_ABORTED;

  if (RIGHT_DISPLAY_STAGE >= 1) {
    PowerEnablePanelSupplies ();
  }

  if (RIGHT_DISPLAY_STAGE >= 2) {
    PowerEnableExtTrigClock ();
  }

  if (RIGHT_DISPLAY_STAGE >= 3) {
    TlmmConfigGpioOutput (PANEL_RESET_GPIO, FALSE);

    // TE input from the panel, pull-down, 2 mA drive as in the downstream sde_te_active state
    TlmmConfigFunction (PANEL_TE_GPIO, PANEL_TE_GPIO_FUNC, 1, 0);
    DispCcEnableMdssClocks ();
    DsiPhyEnable ();
    Status = DsiPllEnable ();
    if (!EFI_ERROR (Status)) {
      Status = DispCcEnableDsi1Clocks ();
    }
  }
  if (RIGHT_DISPLAY_STAGE >= 4 && !EFI_ERROR (Status)) {
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
  if (EFI_ERROR (Status) && RIGHT_DISPLAY_STAGE >= 3) {
    DEBUG ((DEBUG_ERROR, "RightDisplay: panel bring-up failed: %r\n", Status));
  }
  PanelUp = !EFI_ERROR (Status) && RIGHT_DISPLAY_STAGE >= 5;

  Status = AllocateRightFrameBuffer (FbSize, &FbBase);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDisplay.Mode = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE));
  if (mDisplay.Mode == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  mDisplay.Mode->Info = AllocateZeroPool (sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION));
  if (mDisplay.Mode->Info == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  mDisplay.Mode->MaxMode                    = 1;
  mDisplay.Mode->Mode                       = 0;
  mDisplay.Mode->SizeOfInfo                 = sizeof (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION);
  mDisplay.Mode->FrameBufferBase            = FbBase;
  mDisplay.Mode->FrameBufferSize            = FbSize;
  mDisplay.Mode->Info->HorizontalResolution = PANEL_WIDTH;
  mDisplay.Mode->Info->VerticalResolution   = PANEL_HEIGHT;
  mDisplay.Mode->Info->PixelsPerScanLine    = PANEL_WIDTH;
  mDisplay.Mode->Info->PixelFormat          = PixelBlueGreenRedReserved8BitPerColor;

  Status = FrameBufferBltConfigure (
             (VOID *)(UINTN)FbBase, mDisplay.Mode->Info, mBltConfigure,
             &mBltConfigureSize);
  if (Status == RETURN_BUFFER_TOO_SMALL) {
    mBltConfigure = AllocatePool (mBltConfigureSize);
    if (mBltConfigure == NULL) {
      return EFI_OUT_OF_RESOURCES;
    }
    Status = FrameBufferBltConfigure (
               (VOID *)(UINTN)FbBase, mDisplay.Mode->Info, mBltConfigure,
               &mBltConfigureSize);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  ZeroMem ((VOID *)(UINTN)FbBase, FbSize);
  DrawTestPattern ((UINT32 *)(UINTN)FbBase);
  WriteBackDataCacheRange ((VOID *)(UINTN)FbBase, FbSize);

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
        Status = DpuStart (FbBase);
      }
    }
#else
    Status = DpuStart (FbBase);
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
      // Panel state after display-on and a few frames
      gBS->Stall (MS (100));
      PanelProbe ();
#if RIGHT_DISPLAY_DSI_TPG
      // DSI controller test pattern on MDP command stream 0, replacing what
      // the DPU delivers: polynomial/init, EN | STREAM0_PATTERN_SEL = 1, then
      // a software trigger sends one frame; no autorefresh so nothing else
      // competes for the stream.
      MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x168), 0x0000A001);
      MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x16C), 0x00123456);
      MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x15C), BIT0 | (1 << 8));
      MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x184), BIT0);
#else
      DpuEnableAutoRefresh ();
      // Experiment: let several frames through, send HS all-pixels-on while the
      // stream runs, then read error count (05) and self diagnostic (0F) again
      gBS->Stall (MS (200));
      Status = DsiCtrlSendPacket (0x05, (CONST UINT8[]){0x23}, 1, FALSE);
      DEBUG ((DEBUG_ERROR, "RD: 0x23 under autorefresh: %r\n", Status));
      gBS->Stall (MS (200));
      PanelProbe ();
      // Hold the white briefly, then back to normal mode (DCS 0x22) so the
      // panel shows whatever the stream has put in its frame memory
      gBS->Stall (MS (2000));
      Status = DsiCtrlSendPacket (0x05, (CONST UINT8[]){0x22}, 1, FALSE);
      DEBUG ((DEBUG_ERROR, "RD: 0x22 normal mode: %r\n", Status));
#endif
#endif
    }
  }

  if (RIGHT_DISPLAY_DIAG) {
    DiagDump (PanelUp, Status);
  }

  return gBS->InstallMultipleProtocolInterfaces (
           &Handle,
           &gEfiDevicePathProtocolGuid, &mDevicePath,
           &gEfiGraphicsOutputProtocolGuid, &mDisplay,
           NULL);
}
