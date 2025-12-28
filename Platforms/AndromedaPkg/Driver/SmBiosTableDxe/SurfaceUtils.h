/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#ifndef _SMBIOS_SURFACE_UTILS_H_
#define _SMBIOS_SURFACE_UTILS_H_

#include <Uefi.h>

EFI_STATUS
EFIAPI
RetrieveSurfaceInformation(
    CHAR8 *SerialNumberString, CHAR8 *VersionString, CHAR8 *RetailSkuString);

#endif