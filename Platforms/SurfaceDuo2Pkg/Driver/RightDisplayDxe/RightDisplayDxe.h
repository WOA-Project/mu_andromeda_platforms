/** @file
  RightDisplayDxe: bring-up of the Surface Duo 2 right display (DSI1).

  Register programming follows XBL's display bring-up, with
  hardware details cross-checked against the open-source Surface Duo kernel.

  SPDX-License-Identifier: MIT
**/
#ifndef RIGHT_DISPLAY_DXE_H_
#define RIGHT_DISPLAY_DXE_H_

#include <PiDxe.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "HwCatalog.h"

//
// Tlmm.c
//
VOID
TlmmConfigGpioOutput (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  );

VOID
TlmmSetGpio (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  );

VOID
TlmmConfigFunction (
  IN UINT32 Gpio,
  IN UINT32 Function,
  IN UINT32 Pull,
  IN UINT32 DriveStrength
  );

//
// DsiPhy.c
//
EFI_STATUS
DsiPhyEnable (VOID);

EFI_STATUS
DsiPllEnable (VOID);

//
// Power.c
VOID
PowerEnablePanelSupplies (VOID);

VOID
PowerEnableExtTrigClock (VOID);

// DispCc.c
//
EFI_STATUS
DispCcEnableMdssClocks (VOID);

EFI_STATUS
DispCcEnableDsi1Clocks (VOID);

//
// DsiCtrl.c
//
EFI_STATUS
DsiCtrlInit (VOID);

EFI_STATUS
DsiCtrlSendPacket (
  IN UINT8        DataType,
  IN CONST UINT8  *Payload,
  IN UINT32       PayloadLength
  );

VOID
DsiCtrlSetLowPower (
  IN BOOLEAN  LowPower
  );

VOID
DsiCtrlSetupStream (VOID);

//
// Panel.c
//
extern CONST UINT8 gPanelPps[128];

EFI_STATUS
DsiCtrlReadDcs (
  IN  UINT8   Command,
  OUT UINT32  *Data,
  OUT UINT32  *RdbkType,
  OUT UINT32  *AckErr
  );

VOID DrawTestPattern (
  IN UINT32  *Fb
  );

VOID
ResetLeftConsole (VOID);

VOID
DiagDump (
  IN BOOLEAN     PanelUp,
  IN EFI_STATUS  LastStatus
  );

EFI_STATUS
PanelProbe (VOID);

EFI_STATUS
PanelPowerOnAndInit (VOID);

EFI_STATUS
PanelReset (VOID);

EFI_STATUS
PanelSendInit (VOID);

EFI_STATUS
PanelDisplayOn (VOID);

//
// Dpu.c
//
EFI_STATUS
DpuStart (
  IN EFI_PHYSICAL_ADDRESS FbBase
  );

#endif
