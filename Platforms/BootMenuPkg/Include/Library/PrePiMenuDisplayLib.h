/** @file
  Framebuffer access for the PrePi menu.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_MENU_DISPLAY_LIB_H_
#define PRE_PI_MENU_DISPLAY_LIB_H_

#include <Uefi.h>

typedef struct {
  UINT32  *Pixels;
  UINTN   Width;
  UINTN   Height;
  UINTN   Stride;
} PRE_PI_MENU_SURFACE;

// Pixels are linear XRGB8888. Stride is measured in pixels.
EFI_STATUS EFIAPI PrePiMenuDisplayGetSurface (OUT PRE_PI_MENU_SURFACE *Surface);
// Returns a non-NULL ASCII heading for this platform.
CONST CHAR8 *EFIAPI PrePiMenuDisplayGetHeading (VOID);
VOID EFIAPI PrePiMenuDisplayFlush (IN CONST PRE_PI_MENU_SURFACE *Surface);

#endif
