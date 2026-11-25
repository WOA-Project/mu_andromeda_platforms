/** @file
  Polling graphical menu for an active PrePi framebuffer.

  SPDX-License-Identifier: MIT
**/

#ifndef PRE_PI_MENU_LIB_H_
#define PRE_PI_MENU_LIB_H_

#include <Uefi.h>

/**
  Display caller-provided ASCII entries until the power button is pressed.
  The caller must have started the display and must serialize access
  to the framebuffer and platform button hardware. All buttons must be
  released before the menu accepts input. Selection is an input/output index.
**/
EFI_STATUS
EFIAPI
PrePiMenuShow (
  IN CONST CHAR8        *Title,
  IN CONST CHAR8 *CONST *Entries,
  IN UINTN              EntryCount,
  IN OUT UINTN          *Selection
  );

#endif
