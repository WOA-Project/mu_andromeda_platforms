/** @file
  DSI1 D-PHY (7nm, v4.2) and PLL programming for a 760MHz bit clock.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

STATIC CONST UINT8 mTimings[14] = {
  0x00, 0x1A, 0x07, 0x06, 0x16, 0x21, 0x07, 0x07, 0x07, 0x02, 0x04, 0x00, 0x17, 0x0B
};

STATIC
VOID
PllProgramRate (VOID)
{
  UINT64 Multiplier = 1ULL << 18;
  UINT64 DecMultiple = (PANEL_BITCLK_HZ * Multiplier) / (PLL_XTAL_HZ * 2);
  UINT32 Dec  = (UINT32)(DecMultiple >> 18);
  UINT32 Frac = (UINT32)(DecMultiple & (Multiplier - 1));

  // PllBiasEnable and PllSetSource, as done by XBL
  MmioWrite32 (PLL (0x28), 0xC0);  // SYSTEM_MUXES
  MmioWrite32 (PLL (0x260), 0x22); // PERF_OPTIMIZE
  MmioWrite32 (PHY (CMN_CLK_CFG1), (MmioRead32 (PHY (CMN_CLK_CFG1)) & ~0x3U) | 0x1); // DSICLK_SEL = 1

  MmioWrite32 (PLL (0x258), 0x01); // ANALOG_CONTROLS_FIVE_1
  MmioWrite32 (PLL (0x240), 0x08); // VCO_CONFIG_1, VCO < 1.52GHz
  MmioWrite32 (PLL (0x18), 0x01);
  MmioWrite32 (PLL (0x04), 0x03);
  MmioWrite32 (PLL (0x10), 0x00);
  MmioWrite32 (PLL (0x20), 0x00);
  MmioWrite32 (PLL (0x24), 0x4E);
  MmioWrite32 (PLL (0x44), 0x40);
  MmioWrite32 (PLL (0x68), 0xBA);
  MmioWrite32 (PLL (0x78), 0x0C);
  MmioWrite32 (PLL (0xA8), 0x00);
  MmioWrite32 (PLL (0xB8), 0x00);
  MmioWrite32 (PLL (0xC8), 0x08);
  MmioWrite32 (PLL (0x160), 0x0A);
  MmioWrite32 (PLL (0x168), 0xC0);
  MmioWrite32 (PLL (0x170), 0x84);
  MmioWrite32 (PLL (0x170), 0x82);
  MmioWrite32 (PLL (0x178), 0x4C);
  MmioWrite32 (PLL (0x190), 0x80);
  MmioWrite32 (PLL (0x90), 0x29);
  MmioWrite32 (PLL (0x90), 0x2F);
  MmioWrite32 (PLL (0x94), 0x2A);
  MmioWrite32 (PLL (0x94), 0x3F);

  MmioWrite32 (PLL (0xBC), 0x10);
  MmioWrite32 (PLL (0xBC), 0x12);
  MmioWrite32 (PLL (0xE0), Dec);
  // Like XBL, only program the low nibble of the low fractional byte
  MmioWrite32 (PLL (0xE4), Frac & 0xF);
  MmioWrite32 (PLL (0xE8), (Frac >> 8) & 0xFF);
  MmioWrite32 (PLL (0xEC), (Frac >> 16) & 0x3);
  MmioWrite32 (PLL (0x154), 0x00); // PLL_OUTDIV_RATE, post divider 1
  MmioWrite32 (PLL (0x1F0), 0x00); // FAST_PWRUP
  MmioWrite32 (PLL (0x234), 0x22); // GEAR_BAND_SELECT_CONTROLS
  MmioWrite32 (PLL (0x158), 0x40);
  MmioWrite32 (PLL (0x194), 0x06);
  MmioWrite32 (PLL (0x250), 0x10);
  MmioWrite32 (PLL (0x248), 0xA0); // VCO <= 1GHz

  // ConfigSSC: SSC disabled, clear SSCEN
  MmioWrite32 (PLL (0x150), MmioRead32 (PLL (0x150)) & ~BIT1);
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

  // 5nm PHY power-up order, as done by XBL
  MmioWrite32 (PHY (CMN_CTRL_3), 0x00);
  MmioWrite32 (PHY (CMN_CTRL_0), BIT5);
  MmioWrite32 (PHY (CMN_CTRL_0), BIT6 | BIT5);
  MmioWrite32 (PHY (CMN_RBUF_CTRL), 0);
  MmioWrite32 (PHY (CMN_CTRL_4), 0x04);

  MmioWrite32 (PHY (CMN_LANE_CFG0), 0x21);
  MmioWrite32 (PHY (CMN_LANE_CFG1), 0x84);
  MmioWrite32 (PHY (CMN_VREG_CTRL_0), 0x53);
  MmioWrite32 (PHY (CMN_VREG_CTRL_1), 0x5C);
  MmioWrite32 (PHY (CMN_CTRL_3), 0x00);
  MmioWrite32 (PHY (CMN_STR_SWI_CAL), 0x00);
  MmioWrite32 (PHY (CMN_HSTX_STR), 0x88);
  MmioWrite32 (PHY (CMN_PEMPH_0), 0x00);
  MmioWrite32 (PHY (CMN_RESCODE_TOP), 0x3C);
  MmioWrite32 (PHY (CMN_RESCODE_BOT), 0x38);
  MmioWrite32 (PHY (CMN_LPTX_STR), 0x55);
  MmioWrite32 (PHY (CMN_CTRL_0), 0x7F);
  MmioWrite32 (PHY (CMN_LANE_CTRL0), 0x1F);
  MmioWrite32 (PHY (CMN_CTRL_2), 0x40);
  MmioWrite32 (PHY (CMN_CLK_CFG0), 0x01);
  MmioWrite32 (PHY (CMN_CLK_CFG1), 0x00);

  for (Lane = 0; Lane < 5; Lane++) {
    MmioWrite32 (PHY (LANE (Lane, 0x14)), 0);                 // LPRX_CTRL
    MmioWrite32 (PHY (LANE (Lane, 0x14)), Lane == 0 ? 3 : 0);
    MmioWrite32 (PHY (LANE (Lane, 0x10)), 0);                 // PIN_SWAP
    MmioWrite32 (PHY (LANE (Lane, 0x00)), 0);
    MmioWrite32 (PHY (LANE (Lane, 0x04)), 0);
    MmioWrite32 (PHY (LANE (Lane, 0x08)), Lane == 4 ? 0x8A : 0x0A);
    MmioWrite32 (PHY (LANE (Lane, 0x18)), TxDctrl[Lane]);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DsiPllEnable (VOID)
{
  UINTN i;

  // PHY reset: pulse DSIPHY_SW_RESET for 1 ms
  MmioWrite32 (PHY (CMN_CTRL_1), BIT6);
  gBS->Stall (1000);
  MmioWrite32 (PHY (CMN_CTRL_1), 0);

  // CalcClockDivider: byte divider 1, pixel divider 3 (D-PHY, 24bpp, 4 lanes)
  MmioWrite32 (PHY (CMN_CLK_CFG0), 1 | (3 << 4));

  PllProgramRate ();

  MmioWrite32 (PHY (CMN_PLL_CNTRL), 1);

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

  // TriggerSleepB
  MmioWrite32 (PHY (CMN_CTRL_3), MmioRead32 (PHY (CMN_CTRL_3)) | BIT2);
  MmioWrite32 (PHY (CMN_CTRL_4), 0x04);

  // PllGlobalClockEnable
  MmioWrite32 (PHY (CMN_CLK_CFG1), MmioRead32 (PHY (CMN_CLK_CFG1)) | BIT5 | BIT4 | BIT0);

  // Resync FIFOs after the global clock enable
  MmioWrite32 (PHY (CMN_RBUF_CTRL), 0);
  MmioWrite32 (PHY (CMN_RBUF_CTRL), 1);

  // The reset clears the timing registers, so program them last
  for (i = 0; i < ARRAY_SIZE (mTimings); i++) {
    MmioWrite32 (PHY (CMN_TIMING_CTRL (i)), mTimings[i]);
  }

  return EFI_SUCCESS;
}
