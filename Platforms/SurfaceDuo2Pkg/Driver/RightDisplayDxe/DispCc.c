/** @file
  GCC/DISP_CC setup for MDSS and the DSI1 byte/escape/pixel clocks.

  Everything here is idempotent: XBL already runs MDSS for the left panel, so
  enabling an already running branch is a no-op. The MDSS GDSC is enabled
  idempotently (DISP_CC + 0x3000). Offsets are from the
  downstream drivers/clk/qcom/dispcc-lahaina.c and gcc-lahaina.c.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

STATIC
VOID
MdssGdscEnable (VOID)
{
  UINTN Addr = (UINTN)(DISP_CC_BASE + DISPCC_MDSS_CORE_GDSCR);
  UINT32 Val = MmioRead32 (Addr);

  if (((Val & BIT0) == 0) && ((Val & BIT31) != 0)) {
    return;
  }

  MmioWrite32 (Addr, Val & ~BIT0);
  for (UINTN i = 0; i < 1000; i++) {
    if ((MmioRead32 (Addr) & BIT31) != 0) {
      return;
    }
    gBS->Stall (1);
  }

  DEBUG ((DEBUG_ERROR, "RightDisplay: MDSS GDSC did not power up\n"));
}

STATIC
EFI_STATUS
RcgSet (
  IN UINT32 Cmd,
  IN UINT32 SrcIdx,
  IN UINT32 Div
  )
{
  UINTN Cfg = (UINTN)(DISP_CC_BASE + Cmd + 4);
  UINTN CmdReg = (UINTN)(DISP_CC_BASE + Cmd);

  MmioWrite32 (Cfg, (SrcIdx << RCG_SRC_SHIFT) | (Div & 0x1F));
  MmioWrite32 (CmdReg, MmioRead32 (CmdReg) | RCG_UPDATE);

  for (UINTN i = 0; i < 1000; i++) {
    if ((MmioRead32 (CmdReg) & RCG_UPDATE) == 0) {
      return EFI_SUCCESS;
    }
    gBS->Stall (1);
  }

  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
BranchEnableAt (
  IN UINT64 Base,
  IN UINT32 Reg
  )
{
  UINTN Addr = (UINTN)(Base + Reg);

  if ((MmioRead32 (Addr) & (BIT0 | BIT31)) == BIT0) {
    return EFI_SUCCESS;
  }

  MmioWrite32 (Addr, MmioRead32 (Addr) | BIT0);

  for (UINTN i = 0; i < 1000; i++) {
    if ((MmioRead32 (Addr) & BIT31) == 0) {
      return EFI_SUCCESS;
    }
    gBS->Stall (1);
  }

  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
BranchEnable (
  IN UINT32 Reg
  )
{
  return BranchEnableAt (DISP_CC_BASE, Reg);
}

//
// Bus, MDSS core and escape clocks that the DSI1 path depends on. They are
// normally already running because XBL drives the left panel; failures are
// logged but not fatal since a clock that is already on may still be fine.
//
EFI_STATUS
DispCcEnableMdssClocks (VOID)
{
  STATIC CONST struct {
    UINT64      Base;
    UINT32      Reg;
    CONST CHAR8 *Name;
  } Branches[] = {
    { GCC_BASE,     GCC_DISP_AHB_CBC,      "gcc_disp_ahb"     },
    { GCC_BASE,     GCC_DISP_HF_AXI_CBC,   "gcc_disp_hf_axi"  },
    { GCC_BASE,     GCC_DISP_SF_AXI_CBC,   "gcc_disp_sf_axi"  },
    { DISP_CC_BASE, DISPCC_MDSS_AHB_CBC,   "mdss_ahb"         },
    { DISP_CC_BASE, DISPCC_MDSS_MDP_CBC,   "mdss_mdp"         },
    { DISP_CC_BASE, DISPCC_MDSS_VSYNC_CBC, "mdss_vsync"       },
    { DISP_CC_BASE, DISPCC_ESC0_CBC,       "mdss_esc0 (m_esc)" },
    { DISP_CC_BASE, DISPCC_MDSS_ROT_CBC,   "mdss_rot"        },
    { DISP_CC_BASE, DISPCC_RSCC_AHB_CBC,   "mdss_rscc_ahb"   },
    { DISP_CC_BASE, DISPCC_RSCC_VSYNC_CBC, "mdss_rscc_vsync" },
  };
  EFI_STATUS Status;
  UINTN      i;

  MdssGdscEnable ();

  for (i = 0; i < ARRAY_SIZE (Branches); i++) {
    Status = BranchEnableAt (Branches[i].Base, Branches[i].Reg);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "RightDisplay: clock %a did not enable\n", Branches[i].Name));
    }
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DispCcEnableDsi1Clocks (VOID)
{
  EFI_STATUS Status;

  // Byte and pixel sources are the DSI1 PHY PLL outputs (parent index 2)
  Status = RcgSet (DISPCC_BYTE1_CMD, 2, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  // byte_intf = byte / 2
  MmioWrite32 ((UINTN)(DISP_CC_BASE + DISPCC_BYTE1_DIV), 1);

  // Escape clock from TCXO (19.2MHz)
  Status = RcgSet (DISPCC_ESC1_CMD, 0, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = RcgSet (DISPCC_PCLK1_CMD, 2, 0);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = BranchEnable (DISPCC_BYTE1_CBC);
  if (!EFI_ERROR (Status)) {
    Status = BranchEnable (DISPCC_BYTE1_INTF_CBC);
  }
  if (!EFI_ERROR (Status)) {
    Status = BranchEnable (DISPCC_ESC1_CBC);
  }
  if (!EFI_ERROR (Status)) {
    Status = BranchEnable (DISPCC_PCLK1_CBC);
  }

  return Status;
}
