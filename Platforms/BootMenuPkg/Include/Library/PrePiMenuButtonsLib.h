/** @file
  Platform button input for the PrePi menu.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_MENU_BUTTONS_LIB_H_
#define PRE_PI_MENU_BUTTONS_LIB_H_

#include <Uefi.h>

#define PRE_PI_MENU_BUTTON_UP      BIT0
#define PRE_PI_MENU_BUTTON_DOWN    BIT1
#define PRE_PI_MENU_BUTTON_SELECT  BIT2

EFI_STATUS EFIAPI PrePiMenuButtonsInitialize (VOID);
EFI_STATUS EFIAPI PrePiMenuButtonsRead (OUT UINT32 *Buttons);

#endif
