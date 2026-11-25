/** @file
  Early display scanout and handoff.

  SPDX-License-Identifier: MIT
**/

#ifndef PRE_PI_DISPLAY_LIB_H_
#define PRE_PI_DISPLAY_LIB_H_

#include <Uefi.h>

EFI_STATUS PlatformStartDisplay (VOID);
EFI_STATUS PlatformDisplayTestImage (VOID);
EFI_STATUS PlatformStopDisplay (VOID);

#endif
