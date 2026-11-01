/** @file
  Duo 1 right panel power-up and DCS initialization sequence (panel XML
  used by XBL for DSI controller 1).

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

typedef struct {
  UINT8        Type;
  UINT8        WaitMs;
  UINT16       Length;
  CONST UINT8  *Payload;
} PANEL_CMD;

#define C(t, w, ...)                                          \
  { t, w, sizeof ((UINT8[]){__VA_ARGS__}),                    \
    (CONST UINT8[]){__VA_ARGS__} }

CONST UINT8 gPanelPps[128] = {
  0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x07, 0x08, 0x05, 0x46, 0x03, 0x84, 0x02, 0xA3, 0x02, 0xA3,
  0x02, 0x00, 0x02, 0x51, 0x00, 0x20, 0x5F, 0x4B, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x1C, 0x00, 0x18,
  0x18, 0x00, 0x10, 0xF0, 0x03, 0x0C, 0x20, 0x00, 0x06, 0x0B, 0x0B, 0x33, 0x0E, 0x1C, 0x2A, 0x38,
  0x46, 0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7B, 0x7D, 0x7E, 0x01, 0x02, 0x01, 0x00, 0x09, 0x40,
  0x09, 0xBE, 0x19, 0xFC, 0x19, 0xFA, 0x19, 0xF8, 0x1A, 0x38, 0x1A, 0x78, 0x1A, 0xB6, 0x2A, 0xF6,
  0x2B, 0x34, 0x2B, 0x74, 0x3B, 0x74, 0x6B, 0xF4
};

// The panel XML lists the display-on command last; it is sent separately by
// PanelDisplayOn once the first frame has been started.
STATIC CONST PANEL_CMD mPreCommands[] = {
  // Compression mode (DSC enable), then 120 ms
  C (0x07, 120, 0x11, 0x00),
  {0x0A, 0, sizeof (gPanelPps), gPanelPps},
  C (0x15, 0, 0xB0, 0xAC),
  C (0x39, 0, 0x2A, 0x00, 0x00, 0x05, 0x45),
  C (0x39, 0, 0x2B, 0x00, 0x00, 0x07, 0x07),
  C (0x15, 0, 0x26, 0x02),
  C (0x39, 0, 0x44, 0x03, 0x84),
  C (0x05, 0, 0x35),
  C (0x39, 0, 0x51, 0x01, 0x47),
  C (0x39, 0, 0x53, 0x0C, 0x30),
  C (0x39, 0, 0x55, 0x04, 0x70, 0xDB, 0x00, 0x78, 0xDB),
  C (0x15, 0, 0xEE, 0x24),
  C (0x15, 0, 0xFB, 0xAC),
  C (0x15, 0, 0xB0, 0xCA),
  C (0x05, 130, 0x11),
};

EFI_STATUS
PanelReset (VOID)
{
  // High for 20 us, low (reset asserted) for 40 us, then high and wait 10 ms
  // before the first command. Pin function 0, output, pull-up.
  TlmmConfigGpioOutput (PANEL_RESET_GPIO, TRUE);
  gBS->Stall (20);
  TlmmSetGpio (PANEL_RESET_GPIO, FALSE);
  gBS->Stall (40);
  TlmmSetGpio (PANEL_RESET_GPIO, TRUE);
  gBS->Stall (MS (10));

  return EFI_SUCCESS;
}

EFI_STATUS
PanelSendInit (VOID)
{
  EFI_STATUS Status;
  UINTN      i;

  DsiCtrlSetLowPower (TRUE);
  Status = EFI_SUCCESS;
  for (i = 0; i < ARRAY_SIZE (mPreCommands); i++) {
    Status = DsiCtrlSendPacket (
               mPreCommands[i].Type, mPreCommands[i].Payload,
               mPreCommands[i].Length);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "RightDisplay: panel command %u failed: %r\n", (UINT32)i, Status));
      break;
    }
    if (mPreCommands[i].WaitMs) {
      gBS->Stall (MS (mPreCommands[i].WaitMs));
    }
  }

  DsiCtrlSetLowPower (FALSE);

  return Status;
}

EFI_STATUS
PanelPowerOnAndInit (VOID)
{
  EFI_STATUS Status;

  Status = PanelReset ();
  if (!EFI_ERROR (Status)) {
    Status = PanelSendInit ();
  }
  return Status;
}

//
// Display On is sent in HS mode only after the first frame has been kicked off,
// so the panel never shows stale GRAM content.
//
EFI_STATUS
PanelDisplayOn (VOID)
{
  EFI_STATUS Status;

  Status = DsiCtrlSendPacket (0x05, (CONST UINT8[]){0x29}, 1);
  gBS->Stall (MS (130));

  return Status;
}
