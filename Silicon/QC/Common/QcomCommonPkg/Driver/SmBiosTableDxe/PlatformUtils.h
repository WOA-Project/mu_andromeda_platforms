/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#ifndef _SMBIOS_PLATFORM_UTILS_H_
#define _SMBIOS_PLATFORM_UTILS_H_

#include <Uefi.h>

#define PLATFORM_TYPE_STRING_MAX_SIZE 64
#define UNKNOWN_STRING_NAME "Not Specified"
#define HAND_HELD_GAMING_PLATFORM_NAME "HHG"

EFI_STATUS
EFIAPI
GetUUIDFromEFIChipInfoSerialNumType(VOID *Buffer, UINT32 BufferSize);

EFI_STATUS
EFIAPI
RetrievePlatformName(CHAR8 *PlatformName);

EFI_STATUS
EFIAPI
RetrieveChipVersion(CHAR8 *VersionString);

EFI_STATUS
EFIAPI
RetrieveSerialNumber(CHAR8 *SerialNumberString);

#endif