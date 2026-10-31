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

extern EFI_GUID gQcomPmicVregProtocolGuid;
extern EFI_GUID gQcomPmicClkBuffProtocolGuid;

typedef struct {
  UINT32  PullDown;
  UINT32  SwMode;
  UINT32  PinCtrled;
  UINT32  SwEnable;
  BOOLEAN VregOk;
} PM_VREG_STATUS;

typedef struct {
  UINT64 Revision;
  EFI_STATUS (EFIAPI *VregControl)(UINT32 Pmic, UINT32 VregId, BOOLEAN Enable);
  EFI_STATUS (EFIAPI *VregSetLevel)(UINT32 Pmic, UINT32 VregId, UINT32 Mv);
  EFI_STATUS (EFIAPI *VregGetLevel)(UINT32 Pmic, UINT32 VregId, UINT32 *Mv);
  EFI_STATUS (EFIAPI *VregSetPwrMode)(UINT32 Pmic, UINT32 VregId, UINT32 Mode);
  EFI_STATUS (EFIAPI *VregMultiphaseCtrl)(UINT32 Pmic, UINT32 VregId, UINT32 Phases);
  EFI_STATUS (EFIAPI *VregGetStatus)(UINT32 Pmic, UINT32 VregId, PM_VREG_STATUS *Status);
  EFI_STATUS (EFIAPI *VregSetLevelUv)(UINT32 Pmic, UINT32 VregId, UINT32 Uv);
} PM_VREG_PROTOCOL;

typedef struct {
  UINT32  DrvStrength;
  BOOLEAN SwEnable;
  BOOLEAN PinCtrled;
  BOOLEAN PullDown;
  UINT32  OutDiv;
} PM_CLK_STATUS;

typedef struct {
  UINT64 Revision;
  EFI_STATUS (EFIAPI *ClkBuffEnable)(UINT32 Pmic, UINT32 ClkType, BOOLEAN Enable);
  EFI_STATUS (EFIAPI *SetOutDrvStrength)(UINT32 Pmic, UINT32 ClkType, UINT32 Strength);
  EFI_STATUS (EFIAPI *SetOutClkDiv)(UINT32 Pmic, UINT32 ClkType, UINT32 Div);
  EFI_STATUS (EFIAPI *GetStatus)(UINT32 Pmic, UINT32 ClkType, PM_CLK_STATUS *Status);
} PM_CLKBUFF_PROTOCOL;

// PMIC device indices (XBL MDPPlatformLib): A = PMK8350, B = PM8350, C = PM8350C, D = PM8350B
#define PMIC_A  0
#define PMIC_B  1
#define PMIC_C  2
#define PMIC_D  3

// EFI_PM_LDO_n enumerators start at LDO_1 = 0
#define PM_LDO(n)  ((n) - 1)

#define PM_ON  1

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

//
// LN_BB_CLK3 (RPMH_LN_BB_CLK3, "ext_trig_clk") is enabled by the vendor driver
// right before the panel is enabled. PmicDxe casts the protocol clock type
// straight to its internal pm_clk_type, whose BB_3 (== LN_BB_CLK3) value is 9
// (SLEEP 0, XO 1, RF_1..5 2..6, LN_BB/BB_1 7, BB_2 8, BB_3 9), which does not
// match the EFI_PM_CLK_BUFF_TYPE enumerator names, so the number is used here.
//
#define PM_CLK_BB_3_RAW   9
#define EXT_TRIG_CLK_PMIC PMIC_A

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
