/** @file
  Best-effort SM8150 DPU setup; not built or hardware tested.

  This is the single-interface, single-mixer profile-6 path from XBL
  MDPLib/MDPConfig.c, MDPLib/MDPLib.c and HALMDSSLib (including mdssreg.h).
  The patched DisplayDxe OEM XML supplies the 1350x1800, 60 Hz, DSC 1.1,
  675x900 slice and DSI/TE parameters. The primary path uses VIG0, LM0,
  CTL0, PP0, DSC0 and INTF1. The optional secondary path uses VIG2, LM2,
  CTL1, PP2, DSC2 and INTF2. XBL routes PP2 to DSC2 but its DSC setup
  routine programs DSC0; the secondary path explicitly programs DSC2.

  Caller contract:
  - Make MDSS and DSI0 registers accessible and supply the XBL-equivalent
    core, AXI, AHB, DSI host and 19.2 MHz vsync clocks before calling these
    entry points. Reads or writes to an unpowered block can hang the processor.
  - Establish bypass/identity DMA access to the framebuffer region. Reserve
    one 1350 * 1800 * 4 byte buffer per enabled panel. Buffers must be linear
    XRGB8888 (little-endian B,G,R,X), stride 5400 bytes.
  - Separately initialize DSI0, PHY and panel, including the matching DSC PPS,
    command stream, dedicated TE pin and 500 MHz link. Stop reads DSI status;
    no function here configures DSI, panel GPIOs, power, SMMU or clocks.
    The caller owns cache cleaning/coherency before and during scanout.
  - Serialize all accesses to the shared DPU/VBIF registers. Init changes
    shared VBIF clock gating, burst length and arbitration; other clients
    must tolerate XBL's settings. Stop also halts shared VBIF clients C0 and
    C7, and C8 if the secondary is started; the caller must own or quiesce
    all their users through Release.
  - Call Init while the old scanout is quiesced, then Start after DSI host
    setup and before the panel sequence, as in XBL's forced-HS path.
    Keep the mapping, framebuffer and clocks
    on any Stop failure. On success retain the halt until after unmapping
    and prevent all future fetches of unmapped memory before Release.

  Init applies the relevant XBL global VBIF defaults, primary clock-control
  fields and VIG0 traffic shaper; it does not initialize unrelated pipelines,
  UBWC, global interrupts, interconnect bandwidth votes or driver handoff.
  Init/Start success means writes were issued, not that TE arrived, pixels
  were transmitted, or the panel displayed them. Stop's halt/drain sequence
  is inferred from XBL register names, not an XBL-documented software
  sequence or a proven AXI/SMMU drain guarantee; it is hardware unvalidated.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

#include "DisplayResumeConfig.h"
#include "DisplayResumeDpu.h"

#define MDSS_BASE       0x0AE00000U
#define CTL0            (MDSS_BASE + 0x02000U)
#define VIG0            (MDSS_BASE + 0x05000U)
#define VIG0_SCALER     (MDSS_BASE + 0x05A00U)
#define VIG0_CSC        (MDSS_BASE + 0x06A00U)
#define LM0             (MDSS_BASE + 0x45000U)
#define INTF1           (MDSS_BASE + 0x6B800U)
#define TEAR1           (MDSS_BASE + 0x6BA80U)
#define PP0             (MDSS_BASE + 0x71000U)
#define DSC0            (MDSS_BASE + 0x81000U)
#define DSC_CTL         (MDSS_BASE + 0x82800U)
#define VBIF            (MDSS_BASE + 0xB0000U)
#define DSI0            (MDSS_BASE + 0x94000U)
#define CTL1            (MDSS_BASE + 0x02200U)
#define VIG2            (MDSS_BASE + 0x09000U)
#define VIG2_SCALER     (MDSS_BASE + 0x09A00U)
#define VIG2_CSC        (MDSS_BASE + 0x0AA00U)
#define LM2             (MDSS_BASE + 0x47000U)
#define INTF2           (MDSS_BASE + 0x6C000U)
#define TEAR2           (MDSS_BASE + 0x6C280U)
#define PP2             (MDSS_BASE + 0x72000U)
#define DSC2            (MDSS_BASE + 0x81800U)
#define DSI1            (MDSS_BASE + 0x96000U)

#define WIDTH           DISPLAY_FRAMEBUFFER_WIDTH
#define HEIGHT          DISPLAY_FRAMEBUFFER_HEIGHT
#define STRIDE          (WIDTH * 4U)
#define SIZE_WH         ((HEIGHT << 16) | WIDTH)

#define CTL_FLUSH_VIG0       BIT0
#define CTL_FLUSH_LM0        BIT6
#define CTL_FLUSH_ATTACH     BIT17
#define CTL_FLUSH_DSC        BIT22
#define CTL_FLUSH_PERIPH     BIT30
#define CTL_FLUSH_INTF       BIT31
#define INTF1_BIT            BIT1
#define STOP_TIMEOUT_US      100000U
#define DSI_BUSY_MASK        0x1FFU
#define VIG_IDLE_MASK        (BIT1 | BIT0)
#define VBIF_HALT_MASK       (BIT7 | BIT0)
#define VBIF_HALTED_MASK     (BIT23 | BIT16 | BIT7 | BIT0)

STATIC BOOLEAN  mDpuInitialized;
STATIC BOOLEAN  mDpuStarted;
STATIC BOOLEAN  mDpuStopping;
STATIC BOOLEAN  mHaltRequested;
STATIC BOOLEAN  mStopSucceeded;
STATIC BOOLEAN  mSecondaryStarted;
STATIC UINT32   mOriginalHaltBits;

STATIC
VOID
UpdateRegister (
  IN UINTN   Address,
  IN UINT32  Mask,
  IN UINT32  Value
  )
{
  MmioWrite32 (Address, (MmioRead32 (Address) & ~Mask) | (Value & Mask));
}

STATIC
VOID
InitDpuAndVbif (
  VOID
  )
{
  // HAL_MDP_Vbif_Init's SDE defaults; the AOOO fields are limited to C0.
  UpdateRegister (VBIF + 0x04, 0x0000FF03U, 0);
  MmioWrite32 (VBIF + 0xD8, 0x00000707U);
  UpdateRegister (VBIF + 0x178, BIT0, BIT0);
  UpdateRegister (VBIF + 0x17C, BIT16 | BIT0, BIT16 | BIT0);
  UpdateRegister (VBIF + 0x124, BIT1 | BIT0, BIT1 | BIT0);

  // HAL_MDP_ClockControl_Set: hysteresis 3, not forced on or off.
  // Preserve all other pipeline fields in these shared registers.
  UpdateRegister (MDSS_BASE + 0x12AC, 0x0000000FU, 0x0000000CU); // VIG0
  UpdateRegister (MDSS_BASE + 0x12B4, 0xF0000000U, 0xC0000000U); // AHB
  UpdateRegister (MDSS_BASE + 0x12BC, 0x00E00000U, 0x00C00000U); // VBIF arbiter
  UpdateRegister (MDSS_BASE + 0x13A8, 0x00F0F000U, 0x00C0C000U); // PP0, LM0

  // HAL_MDP_TrafficCtrl_Init disables the primary VIG traffic shaper.
  MmioWrite32 (VIG0 + 0x130, 0);
}

STATIC
VOID
ProgramDsc (
  IN UINTN  DscBase
  )
{
  // Rate-control values correspond to the 8-bpc/8-bpp SCR0 OEM PPS.
  STATIC CONST UINT8  Threshold[14] = {
    14, 28, 42, 56, 70, 84, 98, 105, 112, 119, 121, 123, 125, 126
  };
  STATIC CONST UINT8  MinQp[15] = {
    0, 0, 1, 1, 3, 3, 3, 3, 3, 3, 5, 5, 5, 7, 12
  };
  STATIC CONST UINT8  MaxQp[15] = {
    4, 4, 5, 6, 7, 7, 7, 8, 9, 10, 11, 12, 13, 13, 15
  };
  STATIC CONST INT8  BpgOffset[15] = {
    2, 0, 0, -2, -4, -6, -8, -8, -8, -10, -10, -12, -12, -12, -12
  };
  UINTN  Index;

  MmioWrite32 (DscBase + 0x00, 0);                      // Single command-mode encoder
  MmioWrite32 (DscBase + 0x04, (3U << 20) | (2U << 18) |
               (128U << 8) | BIT7 | (9U << 3) | BIT1);
  MmioWrite32 (DscBase + 0x08, (WIDTH << 16) | HEIGHT);
  MmioWrite32 (DscBase + 0x0C, (675U << 16) | 900U);
  MmioWrite32 (DscBase + 0x10, 675);
  MmioWrite32 (DscBase + 0x14, (593U << 16) | 512U);
  MmioWrite32 (DscBase + 0x18, 32);
  MmioWrite32 (DscBase + 0x1C, 9);
  MmioWrite32 (DscBase + 0x20, 0x5F4B);
  MmioWrite32 (DscBase + 0x24, 12);
  MmioWrite32 (DscBase + 0x28, (28U << 16) | 24U);
  MmioWrite32 (DscBase + 0x2C, (6144U << 16) | 4336U);
  MmioWrite32 (DscBase + 0x30, (2U << 10) | (12U << 5) | 3U);
  MmioWrite32 (DscBase + 0x34, 8192);
  MmioWrite32 (DscBase + 0x38, (3U << 18) | (3U << 14) |
               (11U << 9) | (11U << 4) | 6U);

  for (Index = 0; Index < 14; Index++) {
    MmioWrite32 (DscBase + 0x3C + 4 * Index, Threshold[Index]);
  }

  for (Index = 0; Index < 15; Index++) {
    MmioWrite32 (DscBase + 0x74 + 4 * Index, MinQp[Index]);
    MmioWrite32 (DscBase + 0xB0 + 4 * Index, MaxQp[Index]);
    MmioWrite32 (DscBase + 0xEC + 4 * Index, (UINT32)(INT32)BpgOffset[Index]);
  }
}

STATIC
VOID
ProgramSurface (
  IN UINTN  Vig,
  IN UINTN  Scaler,
  IN UINTN  Csc,
  IN UINTN  Mixer,
  IN UINT32  Framebuffer
  )
{
  UINTN  Index;

  MmioWrite32 (Vig + 0x00, SIZE_WH);
  MmioWrite32 (Vig + 0x08, 0);
  MmioWrite32 (Vig + 0x0C, SIZE_WH);
  MmioWrite32 (Vig + 0x10, 0);
  MmioWrite32 (Vig + 0x14, Framebuffer);
  MmioWrite32 (Vig + 0x18, 0);
  MmioWrite32 (Vig + 0x1C, 0);
  MmioWrite32 (Vig + 0x20, 0);
  MmioWrite32 (Vig + 0x24, STRIDE);
  MmioWrite32 (Vig + 0x28, 0);
  MmioWrite32 (Vig + 0x30, 0x000237FF);
  MmioWrite32 (Vig + 0x34, 0x03020001);
  MmioWrite32 (Vig + 0x38, 0);
  MmioWrite32 (Vig + 0xB4, 0);
  MmioWrite32 (Scaler + 0x04, 0);
  MmioWrite32 (Csc, 0);

  MmioWrite32 (Mixer + 0x00, 0);
  MmioWrite32 (Mixer + 0x04, SIZE_WH);
  MmioWrite32 (Mixer + 0x08, 0);
  MmioWrite32 (Mixer + 0x10, 0);
  for (Index = 0; Index < 6; Index++) {
    MmioWrite32 (Mixer + 0x20 + 0x18 * Index, BIT10);
    MmioWrite32 (Mixer + 0x24 + 0x18 * Index, 0x00FF0000);
  }
}

STATIC
VOID
FlushPipeline (
  VOID
  )
{
  MmioWrite32 (CTL0 + 0x104, BIT0);       // DSC0
  MmioWrite32 (CTL0 + 0x110, INTF1_BIT);
  MmioWrite32 (CTL0 + 0x128, INTF1_BIT);
  MmioWrite32 (CTL0 + 0x18, CTL_FLUSH_VIG0 | CTL_FLUSH_LM0 |
               CTL_FLUSH_ATTACH | CTL_FLUSH_DSC |
               CTL_FLUSH_PERIPH | CTL_FLUSH_INTF);
}

STATIC
VOID
FlushSecondaryPipeline (
  VOID
  )
{
  MmioWrite32 (CTL1 + 0x104, BIT2);
  MmioWrite32 (CTL1 + 0x110, BIT2);
  MmioWrite32 (CTL1 + 0x128, BIT2);
  MmioWrite32 (CTL1 + 0x18, BIT2 | BIT8 | CTL_FLUSH_ATTACH |
               CTL_FLUSH_DSC | CTL_FLUSH_PERIPH | CTL_FLUSH_INTF);
}

STATIC
VOID
InitSecondaryPipeline (
  VOID
  )
{
  UpdateRegister (MDSS_BASE + 0x12BC, 0x0000000FU, 0x0000000CU);
  UpdateRegister (MDSS_BASE + 0x13B8, 0x00F0F000U, 0x00C0C000U);
  MmioWrite32 (VIG2 + 0x130, 0);

  MmioWrite32 (INTF2 + 0x00, 0);
  MmioWrite32 (TEAR2 + 0x04, 0);
  MmioWrite32 (TEAR2 + 0x34, 0);
  MmioWrite32 (TEAR2 + 0x08, 0);

  ProgramSurface (VIG2, VIG2_SCALER, VIG2_CSC, LM2, DISPLAY_SECONDARY_FRAMEBUFFER);
  ProgramDsc (DSC2);
  MmioWrite32 (PP2 + 0xC8, BIT18 | (5U << 15) | (4U << 12) |
               (3U << 9) | (2U << 6) | (1U << 3));
  MmioWrite32 (PP2 + 0xA0, 1);
  MmioWrite32 (DSC_CTL + 0x08, 2);
  MmioWrite32 (INTF2 + 0x25C, 2);
  MmioWrite32 (CTL1 + 0x08, BIT24 | (5U << 6));
  MmioWrite32 (CTL1 + 0x48, 0);
  MmioWrite32 (CTL1 + 0x78, 0);
  MmioWrite32 (CTL1 + 0x14, BIT17);
  MmioWrite32 (CTL1 + 0xE8, BIT2);
  MmioWrite32 (CTL1 + 0xF4, BIT2);
  MmioWrite32 (CTL1 + 0x134, BIT2);

  MmioWrite32 (TEAR2 + 0x00, DISPLAY_SECONDARY_TE_SELECT);
  MmioWrite32 (TEAR2 + 0x08, BIT20 | 177U);
  MmioWrite32 (TEAR2 + 0x0C, 0xFFFF);
  MmioWrite32 (TEAR2 + 0x14, HEIGHT);
  MmioWrite32 (TEAR2 + 0x1C, (5U << 16) | 4U);
  MmioWrite32 (TEAR2 + 0x20, HEIGHT);
  MmioWrite32 (TEAR2 + 0x24, HEIGHT + 1U);
  MmioWrite32 (TEAR2 + 0x28, HEIGHT);
  MmioWrite32 (TEAR2 + 0x34, 1);
  FlushSecondaryPipeline ();
  MemoryFence ();
}

EFI_STATUS
DisplayResumeDpuInit (
  VOID
  )
{
  if (mDpuStarted || mDpuStopping) {
    return EFI_ALREADY_STARTED;
  }

  InitDpuAndVbif ();

  // Command mode uses the tear engine rather than INTF video timing.
  MmioWrite32 (INTF1 + 0x00, 0);
  MmioWrite32 (TEAR1 + 0x04, 0);
  MmioWrite32 (TEAR1 + 0x34, 0);
  MmioWrite32 (TEAR1 + 0x08, 0);

  ProgramSurface (VIG0, VIG0_SCALER, VIG0_CSC, LM0, DISPLAY_FRAMEBUFFER_BASE);
  ProgramDsc (DSC0);
  // DSC output byte order is the XBL PP0 big-endian byte mapping.
  MmioWrite32 (PP0 + 0xC8, BIT18 | (5U << 15) | (4U << 12) |
               (3U << 9) | (2U << 6) | (1U << 3));
  MmioWrite32 (PP0 + 0xA0, 1);
  MmioWrite32 (DSC_CTL, 0);                // PP0 -> DSC0
  MmioWrite32 (INTF1 + 0x25C, 0);         // PP0 -> INTF1

  MmioWrite32 (CTL0 + 0x00, BIT24 | 5U);  // Border and VIG0 at stage 3
  MmioWrite32 (CTL0 + 0x40, 0);
  MmioWrite32 (CTL0 + 0x70, 0);
  MmioWrite32 (CTL0 + 0x14, BIT17);        // Command mode
  MmioWrite32 (CTL0 + 0xE8, BIT0);
  MmioWrite32 (CTL0 + 0xF4, INTF1_BIT);
  MmioWrite32 (CTL0 + 0x134, INTF1_BIT);

  // XBL: 19.2 MHz / (1800 * 60) = 177 ticks/line, external TE.
  MmioWrite32 (TEAR1 + 0x00, DISPLAY_PRIMARY_TE_SELECT);
  MmioWrite32 (TEAR1 + 0x08, BIT20 | 177U);
  MmioWrite32 (TEAR1 + 0x0C, 0xFFFF);
  MmioWrite32 (TEAR1 + 0x14, HEIGHT);
  MmioWrite32 (TEAR1 + 0x1C, (5U << 16) | 4U);
  MmioWrite32 (TEAR1 + 0x20, HEIGHT);
  MmioWrite32 (TEAR1 + 0x24, HEIGHT + 1U);
  MmioWrite32 (TEAR1 + 0x28, HEIGHT);
  MmioWrite32 (TEAR1 + 0x34, 1);          // One frame per TE, disabled

  FlushPipeline ();
  MemoryFence ();
  mDpuInitialized = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDpuStart (
  VOID
  )
{
  if (!mDpuInitialized) {
    return EFI_NOT_READY;
  }

  if (mDpuStarted || mDpuStopping) {
    return EFI_ALREADY_STARTED;
  }

  // Match XBL command-mode start: auto-refresh, vsync, TE, then flush.
  MmioWrite32 (TEAR1 + 0x34, BIT31 | 1U);
  MmioWrite32 (TEAR1 + 0x08, BIT20 | BIT19 | 177U);
  MmioWrite32 (TEAR1 + 0x04, 1);
  FlushPipeline ();
  MemoryFence ();
  mDpuStarted = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDpuStartSecondary (
  VOID
  )
{
  if (!mDpuStarted || mDpuStopping || mSecondaryStarted ||
      !FixedPcdGetBool (PcdPrePiSecondaryDisplay)) {
    return EFI_NOT_READY;
  }

  InitSecondaryPipeline ();
  MmioWrite32 (TEAR2 + 0x34, BIT31 | 1U);
  MmioWrite32 (TEAR2 + 0x08, BIT20 | BIT19 | 177U);
  MmioWrite32 (TEAR2 + 0x04, 1);
  FlushSecondaryPipeline ();
  MemoryFence ();
  mSecondaryStarted = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDpuStop (
  VOID
  )
{
  UINT32  Attempt;
  UINT32  HaltMask;
  UINT32  HaltedMask;
  UINT32  Status;

  if (!mDpuInitialized) {
    return EFI_NOT_READY;
  }

  if (mStopSucceeded) {
    return EFI_ALREADY_STARTED;
  }

  // Stop new periodic refresh, then follow XBL MDPStopPipe's TE, counter,
  // and interface-disable ordering. This does not abort an in-flight frame.
  mDpuStopping = TRUE;
  UpdateRegister (TEAR1 + 0x34, BIT31, 0);
  MmioWrite32 (TEAR1 + 0x04, 0);
  UpdateRegister (TEAR1 + 0x08, BIT19, 0);
  MmioWrite32 (INTF1 + 0x00, 0);
  if (mSecondaryStarted) {
    UpdateRegister (TEAR2 + 0x34, BIT31, 0);
    MmioWrite32 (TEAR2 + 0x04, 0);
    UpdateRegister (TEAR2 + 0x08, BIT19, 0);
    MmioWrite32 (INTF2 + 0x00, 0);
  }

  MemoryFence ();
  HaltMask = VBIF_HALT_MASK | (mSecondaryStarted ? BIT8 : 0);
  HaltedMask = VBIF_HALTED_MASK | (mSecondaryStarted ? (BIT24 | BIT8) : 0);

  // Let an active transfer finish before halting its memory clients. On a
  // retry after a halt timeout, keep the already asserted requests intact.
  if (!mHaltRequested) {
    for (Attempt = 0; Attempt < STOP_TIMEOUT_US; Attempt++) {
      if (((MmioRead32 (DSI0 + 0x08) & DSI_BUSY_MASK) == 0) &&
          ((MmioRead32 (VIG0 + 0xF4) & VIG_IDLE_MASK) == VIG_IDLE_MASK) &&
          ((MmioRead32 (CTL0 + 0x64) & BIT0) != 0) &&
          (!mSecondaryStarted ||
           (((MmioRead32 (DSI1 + 0x08) & DSI_BUSY_MASK) == 0) &&
            ((MmioRead32 (VIG2 + 0xF4) & VIG_IDLE_MASK) == VIG_IDLE_MASK) &&
            ((MmioRead32 (CTL1 + 0x64) & BIT0) != 0)))) {
        break;
      }

      MicroSecondDelay (1);
    }

    if (Attempt == STOP_TIMEOUT_US) {
      return EFI_TIMEOUT;
    }

    Status = MmioRead32 (VBIF + 0x200);
    mOriginalHaltBits = Status & HaltMask;
    MmioWrite32 (VBIF + 0x200, Status | HaltMask);
    MemoryFence ();
    mHaltRequested = TRUE;
  }

  // Register names suggest these conditions, but XBL does not specify their
  // ordering or prove that HALT_IDLE/ACK drain downstream transactions.
  for (Attempt = 0; Attempt < STOP_TIMEOUT_US; Attempt++) {
    if (((MmioRead32 (VBIF + 0x204) & HaltedMask) == HaltedMask) &&
        ((MmioRead32 (DSI0 + 0x08) & DSI_BUSY_MASK) == 0) &&
        ((MmioRead32 (VIG0 + 0xF4) & VIG_IDLE_MASK) == VIG_IDLE_MASK) &&
        ((MmioRead32 (CTL0 + 0x64) & BIT0) != 0) &&
        (!mSecondaryStarted ||
         (((MmioRead32 (DSI1 + 0x08) & DSI_BUSY_MASK) == 0) &&
          ((MmioRead32 (VIG2 + 0xF4) & VIG_IDLE_MASK) == VIG_IDLE_MASK) &&
          ((MmioRead32 (CTL1 + 0x64) & BIT0) != 0))) &&
        ((MmioRead32 (VBIF + 0x200) & HaltMask) == HaltMask)) {
      MemoryFence ();
      mStopSucceeded = TRUE;
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  return EFI_TIMEOUT;
}

EFI_STATUS
DisplayResumeDpuRelease (
  VOID
  )
{
  if (!mStopSucceeded || !mHaltRequested) {
    return EFI_NOT_READY;
  }

  UpdateRegister (VBIF + 0x200,
                  VBIF_HALT_MASK | (mSecondaryStarted ? BIT8 : 0),
                  mOriginalHaltBits);
  MemoryFence ();
  mHaltRequested = FALSE;
  mDpuInitialized = FALSE;
  mDpuStarted = FALSE;
  mDpuStopping = FALSE;
  mStopSucceeded = FALSE;
  mSecondaryStarted = FALSE;
  return EFI_SUCCESS;
}
