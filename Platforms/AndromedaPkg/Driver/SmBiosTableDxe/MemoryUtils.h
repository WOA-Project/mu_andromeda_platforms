/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#ifndef _SMBIOS_MEMORY_UTILS_H_
#define _SMBIOS_MEMORY_UTILS_H_

#include <Uefi.h>

EFI_STATUS
EFIAPI
GetSystemMemorySize(UINT64 *SystemMemorySize);

#endif