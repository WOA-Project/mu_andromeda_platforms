/** @file
  Best-effort SM8150 display core clock and power-domain bring-up.

  Register addresses, branch-clock bits, and sequencing follow the Qualcomm
  SDM855 XBL ClockRamDump.c and MDSSPlatform_5xx.h sources. MMCX retention is
  an explicit platform assumption; this code does not create RPMh rail votes
  or ClockDxe software references, and does not change DSI external clocks.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/TimerLib.h>

#define GCC_BASE                       0x00100000
#define MDSS_BASE                      0x0AE00000
#define DISP_CC_BASE                   (MDSS_BASE + 0x00100000)

#define GCC_DISP_AHB_CBCR              (GCC_BASE + 0x0000B00C)
#define GCC_DISP_HF_AXI_CBCR           (GCC_BASE + 0x0000B038)
#define GCC_DISP_XO_CBCR               (GCC_BASE + 0x0000B048)
#define GCC_MMNOC_AT_CBCR              (GCC_BASE + 0x00009028)
#define GCC_MMNOC_HF_QX_CMD_RCGR       (GCC_BASE + 0x00009038)
#define GCC_CONFIG_NOC_CMD_RCGR        (GCC_BASE + 0x00005058)
#define GCC_XO_CMD_RCGR                (GCC_BASE + 0x0004302C)

#define DISP_CC_MDSS_MDP_CBCR          (DISP_CC_BASE + 0x0000200C)
#define DISP_CC_MDSS_ROT_CBCR          (DISP_CC_BASE + 0x00002014)
#define DISP_CC_MDSS_VSYNC_CBCR        (DISP_CC_BASE + 0x00002024)
#define DISP_CC_MDSS_AHB_CBCR          (DISP_CC_BASE + 0x00002080)
#define DISP_CC_MDSS_MDP_CMD_RCGR      (DISP_CC_BASE + 0x000020C8)
#define DISP_CC_MDSS_ROT_CMD_RCGR      (DISP_CC_BASE + 0x000020E0)
#define DISP_CC_MDSS_VSYNC_CMD_RCGR    (DISP_CC_BASE + 0x000020F8)
#define DISP_CC_MDSS_AHB_CMD_RCGR      (DISP_CC_BASE + 0x000022BC)
#define DISP_CC_XO_CBCR                (DISP_CC_BASE + 0x0000605C)
#define DISP_CC_XO_CMD_RCGR            (DISP_CC_BASE + 0x00006044)
#define DISP_CC_MDSS_CORE_GDSCR        (DISP_CC_BASE + 0x00003000)

#define GCC_GPLL0_MODE                 GCC_BASE
#define GCC_GPLL0_L_VAL                (GCC_BASE + 0x00000004)
#define GCC_GPLL0_USER_CTL             (GCC_BASE + 0x0000000C)
#define GCC_GPLL0_ALPHA_VAL            (GCC_BASE + 0x00000040)
#define GCC_GPLL0_OUTCTRL              BIT0
#define GCC_GPLL0_RESET_N              BIT2
#define GCC_GPLL0_LOCK_DET             BIT31
#define GCC_GPLL0_ACTIVE_FLAG          BIT30
#define GCC_GPLL0_VOTE_FSM_ENA         BIT20
#define GCC_GPLL0_ACTIVE_MASK          (GCC_GPLL0_OUTCTRL | GCC_GPLL0_RESET_N)
#define TCSR_SOC_HW_VERSION            0x01FC8000

#define CLOCK_ENABLE                   BIT0
#define CLOCK_HW_CONTROL               BIT1
#define CLOCK_OFF                      BIT31
#define GDSC_SW_COLLAPSE               BIT0
#define GDSC_POWER_ON                  BIT31
#define PLL_OUTCTRL                    BIT0
#define PLL_RESET_N                    BIT2
#define PLL_LOCK_DET                   BIT31
#define PLL_VOTE_FSM_ENA               BIT20
#define PLL_ACTIVE_MASK                (PLL_OUTCTRL | PLL_RESET_N)
#define PLL_OUTPUT_HW_CONTROL          0x00000038

#define RCGR_UPDATE                   BIT0
#define RCGR_CFG_MODE_MASK            0x00003000
#define RCGR_CFG_SOURCE_MASK          0x00000700
#define RCGR_CFG_DIVIDER_MASK         0x0000001F
#define RCGR_CFG_SOURCE_SHIFT         8

#define CLOCK_READY_TIMEOUT_US         200
#define GDSC_READY_TIMEOUT_US          150
#define PLL_LOCK_TIMEOUT_US            200
#define RCGR_UPDATE_TIMEOUT_US         1000
#define PLL_MODE_OFFSET                0x00
#define PLL_L_OFFSET                   0x04
#define PLL_CAL_L_OFFSET               0x08
#define PLL_USER_CTL_OFFSET            0x0C
#define PLL_USER_CTL_U_OFFSET          0x10
#define PLL_USER_CTL_U1_OFFSET         0x14
#define PLL_CONFIG_CTL_OFFSET          0x18
#define PLL_CONFIG_CTL_U_OFFSET        0x1C
#define PLL_CONFIG_CTL_U1_OFFSET       0x20
#define PLL_TEST_CTL_OFFSET            0x24
#define PLL_TEST_CTL_U_OFFSET          0x28
#define PLL_TEST_CTL_U1_OFFSET         0x2C
#define PLL_OPMODE_OFFSET              0x38
#define PLL_ALPHA_OFFSET               0x40
#define PLL_PROFILE_USER_CTL_MASK      0x0007FF03
#define PLL_PROFILE_USER_CTL_VALUE     0x00000103
#define GCC_GPLL0_TRION_DIVIDER_MASK   0x0007FF00
#define GCC_GPLL0_TRION_DIVIDER_VALUE  0x00000100

STATIC CONST UINTN  mCoreBranches[] = {
  GCC_DISP_HF_AXI_CBCR,
  DISP_CC_MDSS_AHB_CBCR,
  DISP_CC_MDSS_MDP_CBCR,
  DISP_CC_MDSS_ROT_CBCR,
  DISP_CC_MDSS_VSYNC_CBCR
};
STATIC UINT32   mOwnedCoreBranches;
STATIC UINT32   mRetainedCoreBranches;
STATIC BOOLEAN  mOwnedCoreDomain;

STATIC
EFI_STATUS
EnableBranchClock (
  IN UINTN       Address,
  IN CONST CHAR8 *Name
  )
{
  UINT32 Value;
  UINT32 Attempt;
  UINTN  Index;

  Value = MmioRead32 (Address);
  for (Index = 0; Index < ARRAY_SIZE (mCoreBranches); Index++) {
    if (Address == mCoreBranches[Index]) {
      if ((Value & CLOCK_ENABLE) == 0) {
        mOwnedCoreBranches |= (1U << Index);
      } else {
        mRetainedCoreBranches |= (1U << Index);
      }

      break;
    }
  }

  if ((Value & CLOCK_ENABLE) == 0) {
    MmioWrite32 (Address, Value | CLOCK_ENABLE);
    MemoryFence ();
  }

  for (Attempt = 0; Attempt < CLOCK_READY_TIMEOUT_US; Attempt++) {
    Value = MmioRead32 (Address);
    if ((Value & CLOCK_ENABLE) != 0) {
      break;
    }

    MicroSecondDelay (1);
  }

  if (Attempt == CLOCK_READY_TIMEOUT_US) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a clock enable readback timeout at 0x%lx (CBCR=0x%x)\n",
      Name,
      (UINT64)Address,
      Value
      ));
    return EFI_TIMEOUT;
  }

  for (Attempt = 0; Attempt < CLOCK_READY_TIMEOUT_US; Attempt++) {
    Value = MmioRead32 (Address);
    if ((Value & CLOCK_HW_CONTROL) != 0) {
      return EFI_SUCCESS;
    }

    if ((Value & CLOCK_OFF) == 0) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  DEBUG ((
    DEBUG_ERROR,
    "DisplayResumePower: %a clock timeout at 0x%lx (CBCR=0x%x)\n",
    Name,
    (UINT64)Address,
    Value
    ));
  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
EnableMdssCoreDomain (
  VOID
  )
{
  UINT32 Value;
  UINT32 Attempt;

  Value = MmioRead32 (DISP_CC_MDSS_CORE_GDSCR);
  if ((Value & GDSC_SW_COLLAPSE) != 0) {
    mOwnedCoreDomain = TRUE;
    MmioWrite32 (DISP_CC_MDSS_CORE_GDSCR, Value & ~GDSC_SW_COLLAPSE);
    MemoryFence ();
  }

  for (Attempt = 0; Attempt < GDSC_READY_TIMEOUT_US; Attempt++) {
    Value = MmioRead32 (DISP_CC_MDSS_CORE_GDSCR);
    if ((Value & GDSC_POWER_ON) != 0) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  DEBUG ((
    DEBUG_ERROR,
    "DisplayResumePower: mdss_core_gdsc timeout at 0x%lx (GDSCR=0x%x); "
    "VDDMM is not voted here\n",
    (UINT64)DISP_CC_MDSS_CORE_GDSCR,
    Value
    ));
  return EFI_TIMEOUT;
}

EFI_STATUS
DisplayResumePowerDown (
  VOID
  )
{
  UINTN  Index;
  UINTN  Address;

  for (Index = ARRAY_SIZE (mCoreBranches); Index > 0; Index--) {
    if ((mOwnedCoreBranches & (1U << (Index - 1))) == 0) {
      continue;
    }

    Address = mCoreBranches[Index - 1];
    MmioAnd32 (Address, ~CLOCK_ENABLE);
    MemoryFence ();
    if ((MmioRead32 (Address) & CLOCK_ENABLE) != 0) {
      DEBUG ((DEBUG_ERROR, "DisplayResumePower: clock 0x%lx did not disable\n", (UINT64)Address));
      return EFI_DEVICE_ERROR;
    }

    mOwnedCoreBranches &= ~(1U << (Index - 1));
  }

  if (mOwnedCoreDomain && (mRetainedCoreBranches == 0)) {
    MmioOr32 (DISP_CC_MDSS_CORE_GDSCR, GDSC_SW_COLLAPSE);
    MemoryFence ();
    if ((MmioRead32 (DISP_CC_MDSS_CORE_GDSCR) & GDSC_SW_COLLAPSE) == 0) {
      DEBUG ((DEBUG_ERROR, "DisplayResumePower: MDSS collapse request did not latch\n"));
      return EFI_DEVICE_ERROR;
    }

    mOwnedCoreDomain = FALSE;
  } else if (mOwnedCoreDomain) {
    DEBUG ((DEBUG_WARN, "DisplayResumePower: retaining MDSS domain for inherited clocks\n"));
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ReadChipRevision (
  OUT BOOLEAN *IsVersion2,
  OUT UINT32  *TestControlU
  )
{
  UINT32 Version;
  UINT32 Major;

  Version = MmioRead32 (TCSR_SOC_HW_VERSION);
  if ((Version == 0) || (Version == MAX_UINT32)) {
    DEBUG ((DEBUG_ERROR, "DisplayResumePower: invalid TCSR version 0x%x\n", Version));
    return EFI_DEVICE_ERROR;
  }

  Major = (Version >> 8) & 0xFF;
  if (Major > 2) {
    DEBUG ((DEBUG_ERROR, "DisplayResumePower: unsupported SM8150 major revision %u\n", Major));
    return EFI_UNSUPPORTED;
  }

  *IsVersion2  = (Major >= 2);
  *TestControlU = *IsVersion2 ? 0x20 : 0x2;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
WaitForPllLock (
  IN UINTN       ModeAddress,
  IN CONST CHAR8 *Name
  )
{
  UINT32 Value;
  UINT32 Attempt;

  for (Attempt = 0; Attempt < PLL_LOCK_TIMEOUT_US; Attempt++) {
    Value = MmioRead32 (ModeAddress);
    if ((Value & PLL_LOCK_DET) != 0) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  DEBUG ((
    DEBUG_ERROR,
    "DisplayResumePower: %a PLL lock timeout at 0x%lx (MODE=0x%x)\n",
    Name,
    (UINT64)ModeAddress,
    Value
    ));
  return EFI_TIMEOUT;
}

STATIC
BOOLEAN
DisplayPllProfileMatches (
  IN UINTN  ModeAddress,
  IN UINT32 LValue,
  IN UINT32 AlphaValue,
  IN UINT32 TestControlU
  )
{
  UINT32 UserControl;

  UserControl = MmioRead32 (ModeAddress + PLL_USER_CTL_OFFSET);
  return (MmioRead32 (ModeAddress + PLL_L_OFFSET) == LValue) &&
         (MmioRead32 (ModeAddress + PLL_ALPHA_OFFSET) == AlphaValue) &&
         (MmioRead32 (ModeAddress + PLL_CAL_L_OFFSET) == 68) &&
         ((UserControl & PLL_PROFILE_USER_CTL_MASK) == PLL_PROFILE_USER_CTL_VALUE) &&
         (MmioRead32 (ModeAddress + PLL_USER_CTL_U_OFFSET) == 0x00000805) &&
         (MmioRead32 (ModeAddress + PLL_USER_CTL_U1_OFFSET) == 0x000000D0) &&
         (MmioRead32 (ModeAddress + PLL_CONFIG_CTL_OFFSET) == 0x20485699) &&
         (MmioRead32 (ModeAddress + PLL_CONFIG_CTL_U_OFFSET) == 0x00002267) &&
         (MmioRead32 (ModeAddress + PLL_CONFIG_CTL_U1_OFFSET) == 0x00000024) &&
         (MmioRead32 (ModeAddress + PLL_TEST_CTL_OFFSET) == 0) &&
         (MmioRead32 (ModeAddress + PLL_TEST_CTL_U_OFFSET) == TestControlU) &&
         (MmioRead32 (ModeAddress + PLL_TEST_CTL_U1_OFFSET) == 0x00000020);
}

STATIC
EFI_STATUS
EnsureDisplayPll (
  IN UINTN       ModeAddress,
  IN UINT32      LValue,
  IN UINT32      AlphaValue,
  IN UINT32      TestControlU,
  IN BOOLEAN     IsVersion2,
  IN CONST CHAR8 *Name
  )
{
  UINT32 Mode;
  UINT32 UserControl;
  EFI_STATUS Status;

  Mode = MmioRead32 (ModeAddress);
  if ((Mode & PLL_ACTIVE_MASK) == PLL_ACTIVE_MASK) {
    Status = WaitForPllLock (ModeAddress, Name);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (!DisplayPllProfileMatches (ModeAddress, LValue, AlphaValue, TestControlU)) {
      DEBUG ((
        DEBUG_ERROR,
        "DisplayResumePower: active %a PLL does not match the XBL profile\n",
        Name
        ));
      return EFI_DEVICE_ERROR;
    }

    return EFI_SUCCESS;
  }

  if ((Mode & PLL_VOTE_FSM_ENA) != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a PLL is FSM-voted but not active; refusing an untracked vote\n",
      Name
      ));
    return EFI_UNSUPPORTED;
  }

  if ((Mode & PLL_ACTIVE_MASK) != 0) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a PLL has a partial active state 0x%x\n",
      Name,
      Mode
      ));
    return EFI_NOT_READY;
  }

  //
  // Match ClockDriver.c Clock_ConfigurePLL for the two non-voteable display
  // PLLs in ClockRamDump.c. Only program a PLL whose outputs are not active.
  //
  MmioWrite32 (ModeAddress + PLL_MODE_OFFSET, 0);
  MmioWrite32 (ModeAddress + PLL_USER_CTL_OFFSET, 0x3);
  MmioWrite32 (ModeAddress + PLL_USER_CTL_U_OFFSET, 0x00000805);
  MmioWrite32 (ModeAddress + PLL_USER_CTL_U1_OFFSET, 0x000000D0);
  MmioWrite32 (ModeAddress + PLL_CONFIG_CTL_OFFSET, 0x20485699);
  MmioWrite32 (ModeAddress + PLL_CONFIG_CTL_U_OFFSET, 0x00002267);
  MmioWrite32 (ModeAddress + PLL_CONFIG_CTL_U1_OFFSET, 0x00000024);
  MmioWrite32 (ModeAddress + PLL_TEST_CTL_OFFSET, 0);
  MmioWrite32 (ModeAddress + PLL_TEST_CTL_U_OFFSET, TestControlU);
  MmioWrite32 (ModeAddress + PLL_TEST_CTL_U1_OFFSET, 0x00000020);
  MmioWrite32 (ModeAddress + PLL_L_OFFSET, LValue);
  MmioWrite32 (ModeAddress + PLL_ALPHA_OFFSET, AlphaValue);
  MmioWrite32 (ModeAddress + PLL_CAL_L_OFFSET, 68);

  UserControl = MmioRead32 (ModeAddress + PLL_USER_CTL_OFFSET);
  UserControl &= ~(0x00070000 | 0x0000F000 | 0x00000F00);
  UserControl |= PLL_PROFILE_USER_CTL_VALUE;
  MmioWrite32 (ModeAddress + PLL_USER_CTL_OFFSET, UserControl);
  MemoryFence ();

  MmioWrite32 (ModeAddress + PLL_OPMODE_OFFSET, 0);
  Mode = PLL_RESET_N;
  if (IsVersion2) {
    Mode |= PLL_OUTPUT_HW_CONTROL;
  }
  MmioWrite32 (ModeAddress + PLL_MODE_OFFSET, Mode);
  MmioWrite32 (ModeAddress + PLL_OPMODE_OFFSET, 1);
  MemoryFence ();
  MicroSecondDelay (25);

  Status = WaitForPllLock (ModeAddress, Name);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UserControl = MmioRead32 (ModeAddress + PLL_USER_CTL_OFFSET);
  MmioWrite32 (ModeAddress + PLL_USER_CTL_OFFSET, UserControl | 0x3);
  Mode = MmioRead32 (ModeAddress + PLL_MODE_OFFSET) | PLL_OUTCTRL;
  MmioWrite32 (ModeAddress + PLL_MODE_OFFSET, Mode);
  MemoryFence ();

  Status = WaitForPllLock (ModeAddress, Name);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Mode = MmioRead32 (ModeAddress + PLL_MODE_OFFSET);
  if ((Mode & PLL_ACTIVE_MASK) != PLL_ACTIVE_MASK ||
      !DisplayPllProfileMatches (ModeAddress, LValue, AlphaValue, TestControlU)) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a PLL failed XBL profile verification (MODE=0x%x)\n",
      Name,
      Mode
      ));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
RequireGpll0 (
  VOID
  )
{
  UINT32 Mode;
  UINT32 UserControl;
  UINT32 Attempt;

  Mode = MmioRead32 (GCC_GPLL0_MODE);
  if (((Mode & GCC_GPLL0_VOTE_FSM_ENA) != 0) ?
      ((Mode & GCC_GPLL0_ACTIVE_FLAG) == 0) :
      ((Mode & GCC_GPLL0_ACTIVE_MASK) != GCC_GPLL0_ACTIVE_MASK)) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: GCC GPLL0 is not active (MODE=0x%x)\n",
      Mode
      ));
    return EFI_NOT_READY;
  }

  for (Attempt = 0; Attempt < PLL_LOCK_TIMEOUT_US; Attempt++) {
    Mode = MmioRead32 (GCC_GPLL0_MODE);
    if ((Mode & GCC_GPLL0_LOCK_DET) != 0) {
      break;
    }

    MicroSecondDelay (1);
  }

  if (Attempt == PLL_LOCK_TIMEOUT_US) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: GCC GPLL0 lock timeout at 0x%lx (MODE=0x%x)\n",
      (UINT64)GCC_GPLL0_MODE,
      Mode
      ));
    return EFI_TIMEOUT;
  }

  Mode = MmioRead32 (GCC_GPLL0_MODE);
  if (((Mode & GCC_GPLL0_VOTE_FSM_ENA) != 0) ?
      ((Mode & GCC_GPLL0_ACTIVE_FLAG) == 0) :
      ((Mode & GCC_GPLL0_ACTIVE_MASK) != GCC_GPLL0_ACTIVE_MASK)) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: GCC GPLL0 lost active state (MODE=0x%x)\n",
      Mode
      ));
    return EFI_NOT_READY;
  }

  //
  // Use GCC_GPLL0's own generated HWIO offsets and ClockSourceBSP_GCC_GPLL0
  // profile, not a DISP_CC PLL base. ClockBSP identifies the GCC source as a
  // Trion 600 MHz XO profile: L=31, alpha=0x4000, even divider=2. Only its
  // divider fields are compared here; the GCC output-control layout is not
  // inferred from the DISP_CC PLL's USER_CTL value.
  //
  UserControl = MmioRead32 (GCC_GPLL0_USER_CTL);
  if ((MmioRead32 (GCC_GPLL0_L_VAL) != 31) ||
      (MmioRead32 (GCC_GPLL0_ALPHA_VAL) != 0x4000) ||
      ((UserControl & GCC_GPLL0_TRION_DIVIDER_MASK) != GCC_GPLL0_TRION_DIVIDER_VALUE)) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: GCC GPLL0 is not the XBL 600 MHz parent profile\n"
      ));
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
RequireRootClock (
  IN UINTN       CommandAddress,
  IN UINT32      Source,
  IN UINT32      Divider,
  IN CONST CHAR8 *Name
  )
{
  UINT32 Command;
  UINT32 Config;
  UINT32 Expected;
  UINT32 Attempt;

  for (Attempt = 0; Attempt < RCGR_UPDATE_TIMEOUT_US; Attempt++) {
    Command = MmioRead32 (CommandAddress);
    if ((Command & RCGR_UPDATE) == 0) {
      break;
    }

    MicroSecondDelay (1);
  }

  if (Attempt == RCGR_UPDATE_TIMEOUT_US) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a shared RCGR update is not complete at 0x%lx\n",
      Name,
      (UINT64)CommandAddress
      ));
    return EFI_TIMEOUT;
  }

  Config = MmioRead32 (CommandAddress + sizeof (UINT32));
  Expected = ((Source << RCGR_CFG_SOURCE_SHIFT) & RCGR_CFG_SOURCE_MASK) |
             (Divider & RCGR_CFG_DIVIDER_MASK);
  if ((Config & (RCGR_CFG_MODE_MASK | RCGR_CFG_SOURCE_MASK | RCGR_CFG_DIVIDER_MASK)) != Expected) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a shared RCGR is not on its XBL plan at 0x%lx "
      "(CFG=0x%x, expected=0x%x)\n",
      Name,
      (UINT64)CommandAddress,
      Config,
      Expected
      ));
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ConfigureRootClock (
  IN UINTN       CommandAddress,
  IN UINT32      Source,
  IN UINT32      Divider,
  IN CONST CHAR8 *Name
  )
{
  UINT32 Command;
  UINT32 Config;
  UINT32 Expected;
  UINT32 Attempt;

  for (Attempt = 0; Attempt < RCGR_UPDATE_TIMEOUT_US; Attempt++) {
    Command = MmioRead32 (CommandAddress);
    if ((Command & RCGR_UPDATE) == 0) {
      break;
    }

    MicroSecondDelay (1);
  }

  if (Attempt == RCGR_UPDATE_TIMEOUT_US) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a RCGR was already updating at 0x%lx\n",
      Name,
      (UINT64)CommandAddress
      ));
    return EFI_TIMEOUT;
  }

  Config = MmioRead32 (CommandAddress + sizeof (UINT32));
  Config &= ~(RCGR_CFG_MODE_MASK | RCGR_CFG_SOURCE_MASK | RCGR_CFG_DIVIDER_MASK);
  Expected = ((Source << RCGR_CFG_SOURCE_SHIFT) & RCGR_CFG_SOURCE_MASK) |
             (Divider & RCGR_CFG_DIVIDER_MASK);
  Config |= Expected;
  MmioWrite32 (CommandAddress + sizeof (UINT32), Config);
  MemoryFence ();

  MmioWrite32 (CommandAddress, Command | RCGR_UPDATE);
  MemoryFence ();
  for (Attempt = 0; Attempt < RCGR_UPDATE_TIMEOUT_US; Attempt++) {
    Command = MmioRead32 (CommandAddress);
    if ((Command & RCGR_UPDATE) == 0) {
      break;
    }

    MicroSecondDelay (1);
  }

  if (Attempt == RCGR_UPDATE_TIMEOUT_US) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a RCGR update timeout at 0x%lx\n",
      Name,
      (UINT64)CommandAddress
      ));
    return EFI_TIMEOUT;
  }

  Config = MmioRead32 (CommandAddress + sizeof (UINT32));
  if ((Config & (RCGR_CFG_MODE_MASK | RCGR_CFG_SOURCE_MASK | RCGR_CFG_DIVIDER_MASK)) != Expected) {
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumePower: %a RCGR readback mismatch at 0x%lx (CFG=0x%x)\n",
      Name,
      (UINT64)CommandAddress,
      Config
      ));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

/**
  Bring up the source-defined MDSS core power domain and branch clocks.

  MMCX retention is the user's explicit assumption. The matching XBL
  Clock_OverrideMMCXArc helper is deliberately not used: it writes RPMh ARC
  override masks and requires prior direct PMIC programming. Success means
  only that the source-defined core PLLs, muxes, GDSC, and branch clocks were
  verified; it does not mean panel-specific DSI clocks or rails are ready.
  PrePi hardware writes do not transfer ClockDxe accounting. After the clock
  protocol is published, DXE must acquire its own clock and power-domain
  references before relying on or releasing this hardware state.

  @retval EFI_SUCCESS      Core PLLs, muxes, GDSC, and branches are ready.
  @retval EFI_NOT_READY    A required retained source is not active.
  @retval EFI_TIMEOUT      A PLL, RCGR, GDSC, or branch timed out.
  @retval EFI_DEVICE_ERROR A register profile or readback is inconsistent.
  @retval EFI_UNSUPPORTED  A source requires an untracked vote or revision.
**/
EFI_STATUS
DisplayResumePower (
  VOID
  )
{
  EFI_STATUS Status;
  BOOLEAN    IsVersion2;
  UINT32     TestControlU;

  Status = ReadChipRevision (&IsVersion2, &TestControlU);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // XBL's 240 MHz HF AXI plan uses GPLL0 at 600 MHz. The RAM-dump clock code
  // assumes that shared GCC source is already established; do not retune or
  // raw-vote this system-wide PLL from PrePi.
  //
  Status = RequireGpll0 ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Match ClockRamDump.c's access-clock prerequisites before DISP_CC access.
  // gcc_disp_ahb is shared and has frequency 0 in MDSSPlatform_5xx.h, so
  // validate its XBL parent instead of retuning GCC_CONFIG_NOC_CMD_RCGR.
  //
  Status = RequireRootClock (GCC_CONFIG_NOC_CMD_RCGR, 1, 15, "gcc_disp_ahb");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (GCC_DISP_AHB_CBCR, "gcc_disp_ahb");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (GCC_MMNOC_AT_CBCR, "gcc_mmnoc_at");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // ClockRamDump.c identifies GCC DISP XO as a prerequisite to mdss_core_gdsc.
  // These XO roots are inherited; verify their XBL plans without rewriting
  // GCC_XO_CMD_RCGR or DISP_CC_XO_CMD_RCGR.
  //
  Status = RequireRootClock (GCC_XO_CMD_RCGR, 0, 1, "gcc_xo");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (GCC_DISP_XO_CBCR, "gcc_disp_xo");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RequireRootClock (DISP_CC_XO_CMD_RCGR, 0, 1, "disp_cc_xo");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (DISP_CC_XO_CBCR, "disp_cc_xo");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableMdssCoreDomain ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // These are the fixed Trion PLL profiles in ClockRamDump.c. An active PLL
  // must already match the profile; an inactive, non-voteable PLL is set up
  // with bounded lock polling.
  //
  Status = EnsureDisplayPll (
             DISP_CC_BASE,
             71,
             0x0000E000,
             TestControlU,
             IsVersion2,
             "disp_cc_pll0"
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnsureDisplayPll (
             DISP_CC_BASE + 0x00001000,
             31,
             0x00004000,
             TestControlU,
             IsVersion2,
             "disp_cc_pll1"
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // MDPClocksBoot.c requests frequencies only for HF AXI and MDP here.
  // Apply those XBL plans before enabling their branches; the other listed
  // clocks have frequency 0, so verify their inherited ClockRamDump plans
  // rather than changing them.
  // SRC_DIV is HALF_DIVIDER = (2 * divider) - 1 in ClockDriver.c.
  //
  Status = ConfigureRootClock (GCC_MMNOC_HF_QX_CMD_RCGR, 1, 4, "gcc_disp_hf_axi");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (GCC_DISP_HF_AXI_CBCR, "gcc_disp_hf_axi");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RequireRootClock (DISP_CC_MDSS_AHB_CMD_RCGR, 4, 15, "disp_cc_mdss_ahb");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (DISP_CC_MDSS_AHB_CBCR, "disp_cc_mdss_ahb");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = ConfigureRootClock (DISP_CC_MDSS_MDP_CMD_RCGR, 1, 7, "disp_cc_mdss_mdp");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (DISP_CC_MDSS_MDP_CBCR, "disp_cc_mdss_mdp");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RequireRootClock (DISP_CC_MDSS_ROT_CMD_RCGR, 0, 1, "disp_cc_mdss_rot");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (DISP_CC_MDSS_ROT_CBCR, "disp_cc_mdss_rot");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RequireRootClock (DISP_CC_MDSS_VSYNC_CMD_RCGR, 0, 1, "disp_cc_mdss_vsync");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = EnableBranchClock (DISP_CC_MDSS_VSYNC_CBCR, "disp_cc_mdss_vsync");
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DEBUG ((
    DEBUG_WARN,
    "DisplayResumePower: hardware ready under the retained-MMCX assumption; "
    "ClockDxe/RPMh ownership is not transferred. DXE must acquire native "
    "clock/domain references before relying on or releasing this state\n"
    ));
  return EFI_SUCCESS;
}
