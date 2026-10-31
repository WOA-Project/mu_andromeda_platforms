/** @file
  DSI1 D-PHY (7nm, v4.2) and PLL programming for a 760MHz bit clock.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#define PHY(off) ((UINTN)(DSI1_PHY_BASE + (off)))
#define PLL(off) ((UINTN)(DSI1_PHY_BASE + 0x500 + (off)))

#define CMN_REVISION_ID0 0x00
#define CMN_CLK_CFG0     0x10
#define CMN_CLK_CFG1     0x14
#define CMN_RBUF_CTRL    0x1C
#define CMN_VREG_CTRL_0  0x20
#define CMN_CTRL_0       0x24
#define CMN_CTRL_2       0x2C
#define CMN_CTRL_3       0x30
#define CMN_LANE_CFG0    0x34
#define CMN_LANE_CFG1    0x38
#define CMN_PLL_CNTRL    0x3C
#define CMN_LANE_CTRL0   0xA0
#define CMN_TIMING_CTRL(n) (0xB4 + 4 * (n))
#define CMN_HSTX_STR     0xEC
#define CMN_RESCODE_TOP  0xF4
#define CMN_RESCODE_BOT  0xF8
#define CMN_LPTX_STR     0x100
#define CMN_PEMPH_0      0x104
#define CMN_STR_SWI_CAL  0x10C
#define CMN_VREG_CTRL_1  0x110
#define CMN_CTRL_4       0x114
#define CMN_DIGTOP_SPARE4 0x128
#define CMN_PHY_STATUS   0x140

#define LANE(n, off) (0x200 + 0x80 * (n) + (off))

#define PLL_XTAL_HZ 19200000ULL

STATIC CONST UINT8 mTimings[14] = {
  0x00, 0x1A, 0x07, 0x06, 0x16, 0x21, 0x07, 0x07, 0x07, 0x02, 0x04, 0x00, 0x17, 0x0B
};

STATIC
VOID
W (
  IN UINTN  Addr,
  IN UINT32 Val
  )
{
  MmioWrite32 (Addr, Val);
}

STATIC
VOID
PllProgramRate (VOID)
{
  UINT64 Multiplier = 1ULL << 18;
  UINT64 DecMultiple = (PANEL_BITCLK_HZ * Multiplier) / (PLL_XTAL_HZ * 2);
  UINT32 Dec  = (UINT32)(DecMultiple >> 18);
  UINT32 Frac = (UINT32)(DecMultiple & (Multiplier - 1));

  W (PLL (0x258), 0x01); // ANALOG_CONTROLS_FIVE_1
  W (PLL (0x240), 0x08); // VCO_CONFIG_1, VCO < 1.52GHz
  W (PLL (0x18), 0x01);
  W (PLL (0x04), 0x03);
  W (PLL (0x10), 0x00);
  W (PLL (0x20), 0x00);
  W (PLL (0x24), 0x4E);
  W (PLL (0x44), 0x40);
  W (PLL (0x68), 0xBA);
  W (PLL (0x78), 0x0C);
  W (PLL (0xA8), 0x00);
  W (PLL (0xB8), 0x00);
  W (PLL (0xC8), 0x08);
  W (PLL (0x160), 0x0A);
  W (PLL (0x168), 0xC0);
  W (PLL (0x170), 0x84);
  W (PLL (0x170), 0x82);
  W (PLL (0x178), 0x4C);
  W (PLL (0x190), 0x80);
  W (PLL (0x90), 0x29);
  W (PLL (0x90), 0x2F);
  W (PLL (0x94), 0x2A);
  W (PLL (0x94), 0x3F);
  W (PLL (0x260), 0x22);

  W (PLL (0xBC), 0x12);
  W (PLL (0xE0), Dec);
  W (PLL (0xE4), Frac & 0xFF);
  W (PLL (0xE8), (Frac >> 8) & 0xFF);
  W (PLL (0xEC), (Frac >> 16) & 0x3);
  W (PLL (0x158), 0x40);
  W (PLL (0x194), 0x06);
  W (PLL (0x250), 0x10);
  W (PLL (0x248), 0xA0); // VCO <= 1GHz
}

EFI_STATUS
DsiPhyEnable (VOID)
{
  UINTN Lane;
  UINTN i;
  STATIC CONST UINT8 TxDctrl[5] = {0x40, 0x40, 0x40, 0x46, 0x41};

  // Controller-side PHY reset (resets PLL, lanes and regulator); must precede
  // all PHY/PLL programming
  MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x12C), 0x1);
  gBS->Stall (10);
  MmioWrite32 ((UINTN)(DSI1_CTRL_BASE + 0x12C), 0x0);
  gBS->Stall (100);

  for (i = 0; i < 1000; i++) {
    if (MmioRead32 (PHY (CMN_PHY_STATUS)) & BIT0) {
      break;
    }
    gBS->Stall (10);
  }
  if (i == 1000) {
    DEBUG ((DEBUG_WARN, "RightDisplay: PHY refgen not ready\n"));
  }

  W (PHY (CMN_CTRL_0), BIT6 | BIT5);
  W (PHY (CMN_PLL_CNTRL), 0);
  W (PHY (CMN_RBUF_CTRL), 0);

  if ((MmioRead32 (PHY (CMN_REVISION_ID0)) & 0xF0) == 0x20) {
    W (PHY (CMN_CTRL_4), 0x04);
  }

  W (PHY (CMN_LANE_CFG0), 0x21);
  W (PHY (CMN_LANE_CFG1), 0x84);
  W (PHY (CMN_VREG_CTRL_0), 0x53);
  W (PHY (CMN_VREG_CTRL_1), 0x5C);
  W (PHY (CMN_CTRL_3), 0x00);
  W (PHY (CMN_STR_SWI_CAL), 0x00);
  W (PHY (CMN_HSTX_STR), 0x88);
  W (PHY (CMN_PEMPH_0), 0x00);
  W (PHY (CMN_RESCODE_TOP), 0x3C);
  W (PHY (CMN_RESCODE_BOT), 0x38);
  W (PHY (CMN_LPTX_STR), 0x55);
  W (PHY (CMN_CTRL_0), 0x7F);
  W (PHY (CMN_LANE_CTRL0), 0x1F);
  W (PHY (CMN_CTRL_2), 0x40);
  W (PHY (CMN_CLK_CFG1), 0x00);

  PllProgramRate ();

  for (i = 0; i < ARRAY_SIZE (mTimings); i++) {
    W (PHY (CMN_TIMING_CTRL (i)), mTimings[i]);
  }

  for (Lane = 0; Lane < 5; Lane++) {
    W (PHY (LANE (Lane, 0x14)), Lane == 0 ? 3 : 0); // LPRX_CTRL
    W (PHY (LANE (Lane, 0x10)), 0);                 // PIN_SWAP
    W (PHY (LANE (Lane, 0x00)), 0);
    W (PHY (LANE (Lane, 0x04)), 0);
    W (PHY (LANE (Lane, 0x08)), Lane == 4 ? 0x8A : 0x0A);
    W (PHY (LANE (Lane, 0x18)), TxDctrl[Lane]);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DsiPllEnable (VOID)
{
  UINTN i;

  W (PHY (CMN_CTRL_0), MmioRead32 (PHY (CMN_CTRL_0)) | BIT5);
  W (PLL (0x28), 0xC0); // SYSTEM_MUXES
  gBS->Stall (1);
  W (PHY (CMN_PLL_CNTRL), 1);

  for (i = 0; i < 100; i++) {
    if (MmioRead32 (PLL (0x1B0)) & BIT0) {
      break;
    }
    gBS->Stall (100);
  }
  if (i == 100) {
    DEBUG ((DEBUG_ERROR, "RightDisplay: DSI1 PLL failed to lock\n"));
    return EFI_TIMEOUT;
  }

  W (PHY (CMN_DIGTOP_SPARE4), 1);
  W (PHY (CMN_DIGTOP_SPARE4), 0);
  W (PHY (CMN_CTRL_3), 0x04);

  // byte divider 1, pixel divider 6 (760 / 6 = 126.67MHz), enable both clocks
  // Left (XBL-programmed) DSI0 reads CLK_CFG0=0x31 and CLK_CFG1[1:0]=1
  // (DSICLK_SEL=1, as XBL HAL_DSI_Phy_DsiClk_Sel picks for D-PHY).
  W (PHY (CMN_CLK_CFG0), 1 | (3 << 4));
  W (PHY (CMN_CLK_CFG1), MmioRead32 (PHY (CMN_CLK_CFG1)) | BIT5 | BIT4 | BIT0);

  // Resync FIFOs (XBL HAL_DSI_Phy_ResyncFIFO) after the global clock enable
  W (PHY (CMN_RBUF_CTRL), 0);
  W (PHY (CMN_RBUF_CTRL), 1);

  return EFI_SUCCESS;
}
