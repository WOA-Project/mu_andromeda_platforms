/** @file
  Panel/DSI supply and ext-trigger clock setup through the Qualcomm
  PmicDxe protocols. Protocol layouts are taken from boot.mxf.1.0

  Everything is conditional: a supply or clock that is already enabled (the
  left panel shares all of them and is driven by XBL) is left untouched, so
  this cannot disturb the left display.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#include <Library/UefiBootServicesTableLib.h>
#include "Protocols.h"

typedef struct {
  UINT32      Pmic;
  UINT32      Ldo;
  UINT32      Mv;
  UINT32      PostDelayMs;
  BOOLEAN     EnableIfOff;
  CONST CHAR8 *Name;
} SUPPLY;

// Same supplies for DSI0 and DSI1 in surface-duo2-display-mp.dtsi / lahaina-sde-display.dtsi
STATIC CONST SUPPLY mSupplies[] = {
  { PMIC_B, PM_LDO (5),  912,  0,  FALSE, "L5B dsi phy 0p9"  },
  { PMIC_B, PM_LDO (6),  1200, 0,  FALSE, "L6B dsi 1p2"      },
  { PMIC_B, PM_LDO (2),  3072, 0,  FALSE, "L2B panel vdda"   },
  { PMIC_C, PM_LDO (12), 1800, 1,  FALSE, "L12C panel vddio" },
  { PMIC_C, PM_LDO (13), 3000, 0,  FALSE, "L13C panel vdd"   },
  // L1D reads off, but VregSetLevel/VregControl on PM8350B LDO1 faulted inside
  // PmicDxe (synchronous exception), and the "off" reading may just be an
  // unmapped LDO; keep it read-only until the PMIC index is verified.
  { PMIC_D, PM_LDO (1),  1216, 10, FALSE, "L1D panel ddic"   },
};

// display_panel_avdd pinctrl is disabled on Duo 2 (surface-duo2-common.dtsi:
// "only needed for certain sharp panels"), so that GPIO must not be touched.

VOID
PowerEnablePanelSupplies (VOID)
{
  PM_VREG_PROTOCOL *Vreg;
  PM_VREG_STATUS   St;
  EFI_STATUS       Status;
  UINTN            i;

  Status = gBS->LocateProtocol (&gQcomPmicVregProtocolGuid, NULL, (VOID **)&Vreg);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "RightDisplay: PMIC VREG protocol missing: %r\n", Status));
    return;
  }

  for (i = 0; i < ARRAY_SIZE (mSupplies); i++) {
    CONST SUPPLY *S = &mSupplies[i];

    ZeroMem (&St, sizeof (St));
    Status = Vreg->VregGetStatus (S->Pmic, S->Ldo, &St);
    DEBUG ((
      DEBUG_ERROR, "RD: %a status %r sw_en %u ok %u\n", S->Name, Status,
      St.SwEnable, St.VregOk));

    // XBL powers rails shared with the left panel through RPMh, so the SPMI
    // enable bit can read "off" for a live rail and rewriting it cuts power to
    // the left display. A rail that reads both disabled and not-OK is not in
    // use, so it is always enabled; live rails are only touched when forced.
    if (!EFI_ERROR (Status) && (St.SwEnable == PM_ON || St.VregOk)) {
      continue;
    }
    if (!S->EnableIfOff) {
      continue;
    }
    DEBUG ((DEBUG_ERROR, "RD: enabling %a\n", S->Name));

    Status = Vreg->VregSetLevel (S->Pmic, S->Ldo, S->Mv);
    if (!EFI_ERROR (Status)) {
      Status = Vreg->VregControl (S->Pmic, S->Ldo, TRUE);
    }
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "RightDisplay: %a failed: %r\n", S->Name, Status));
      continue;
    }

    if (S->PostDelayMs != 0) {
      gBS->Stall (MS (S->PostDelayMs));
    }
  }
}

VOID
PowerEnableExtTrigClock (VOID)
{
  PM_CLKBUFF_PROTOCOL *Clk;
  PM_CLK_STATUS       St;
  EFI_STATUS          Status;

  Status = gBS->LocateProtocol (&gQcomPmicClkBuffProtocolGuid, NULL, (VOID **)&Clk);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_WARN, "RightDisplay: PMIC clk buffer protocol missing: %r\n", Status));
    return;
  }

  ZeroMem (&St, sizeof (St));
  Status = Clk->GetStatus (EXT_TRIG_CLK_PMIC, PM_CLK_BB_3_RAW, &St);
  DEBUG ((DEBUG_ERROR, "RD: ext_trig_clk status %r sw_en %u\n", Status, St.SwEnable));

  /*if (!EFI_ERROR (Status) && St.SwEnable) {
    return;
  }
  Status = Clk->ClkBuffEnable (EXT_TRIG_CLK_PMIC, PM_CLK_BB_3_RAW, TRUE);
  DEBUG ((DEBUG_INFO, "RightDisplay: ext_trig_clk enable: %r\n", Status));*/
}
