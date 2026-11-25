/** @file
  Polling PMIC buttons for the PrePi menu.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/DebugLib.h>
#include <Library/PrePiMenuButtonsLib.h>
#include <Library/PrePiSpmiLib.h>

#ifdef PREPI_BUTTONS_SM8350
#define SPMI_PON_PPID    0x013U
#define SPMI_VOLUP_PPID  0x18DU
#define PON_DOWN         BIT6
#define PON_SELECT       BIT7
#else
#define SPMI_PON_PPID    0x08U
#define SPMI_VOLUP_PPID  0xC5U
#define PON_DOWN         BIT1
#define PON_SELECT       BIT0
#endif

STATIC UINTN mPonChannel;
STATIC UINTN mUpChannel;

EFI_STATUS
EFIAPI
PrePiMenuButtonsRead (
  OUT UINT32 *Buttons
  )
{
  EFI_STATUS Status;
  UINT8 Pon;
  UINT8 Up;

  if (Buttons == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = PrePiSpmiRead (mPonChannel, 0x10, &Pon);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PrePiSpmiRead (mUpChannel, 0x10, &Up);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  *Buttons = ((Up & BIT0) == 0 ? PRE_PI_MENU_BUTTON_UP : 0) |
             ((Pon & PON_DOWN) != 0 ? PRE_PI_MENU_BUTTON_DOWN : 0) |
             ((Pon & PON_SELECT) != 0 ? PRE_PI_MENU_BUTTON_SELECT : 0);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PrePiMenuButtonsInitialize (
  VOID
  )
{
  EFI_STATUS Status;
  UINT8 Type;
  UINT8 Subtype;
  UINT8 Mode;
  UINT8 Pull;
  UINT8 Enable;
#ifdef PREPI_BUTTONS_SM8350
  UINT8 PonSubtype;
  UINT8 Voltage;
#endif

  Status = PrePiSpmiInitialize ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = PrePiSpmiFindChannel (SPMI_PON_PPID, &mPonChannel);
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiFindChannel (SPMI_VOLUP_PPID, &mUpChannel);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

#ifdef PREPI_BUTTONS_SM8350
  Status = PrePiSpmiRead (mPonChannel, 0x05, &PonSubtype);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (PonSubtype != 0x09) {
    DEBUG ((DEBUG_ERROR, "PrePiMenu: unsupported PMK8350 PON subtype 0x%x\n", PonSubtype));
    return EFI_UNSUPPORTED;
  }
#endif

  Status = PrePiSpmiRead (mUpChannel, 0x04, &Type);
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiRead (mUpChannel, 0x05, &Subtype);
  }
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiRead (mUpChannel, 0x40, &Mode);
  }
#ifdef PREPI_BUTTONS_SM8350
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiRead (mUpChannel, 0x41, &Voltage);
  }
#endif
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiRead (mUpChannel, 0x42, &Pull);
  }
  if (!EFI_ERROR (Status)) {
    Status = PrePiSpmiRead (mUpChannel, 0x46, &Enable);
  }
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Type != 0x10) ||
      ((Subtype != 1) && (Subtype != 5) && (Subtype != 9) &&
       (Subtype != 13) && (Subtype != 0x10) && (Subtype != 0x11)) ||
      (((Subtype == 0x10) || (Subtype == 0x11)) ?
        ((Mode & 3U) != 0) : ((Mode & 0x70U) != 0)) ||
#ifdef PREPI_BUTTONS_SM8350
      ((Voltage & 7U) != 1) ||
#endif
      ((Pull & 7U) > 3U) || ((Enable & BIT7) == 0)) {
    DEBUG ((DEBUG_ERROR, "PrePiMenu: volume-up GPIO is not a configured pulled-up input\n"));
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}
