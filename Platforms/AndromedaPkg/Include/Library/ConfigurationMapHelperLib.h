/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _CONFIGURATION_MAP_HELPER_LIB_H_
#define _CONFIGURATION_MAP_HELPER_LIB_H_

#include <Library/PlatformConfigurationMapLib.h>

EFI_STATUS EFIAPI LocateConfigurationMapUINT32ByName(
    CHAR8  *ConfigurationMapUINT32Name,
    UINT32 *ConfigurationDescriptor);

EFI_STATUS EFIAPI LocateConfigurationMapUINT64ByName(
    CHAR8  *ConfigurationMapUINT64Name,
    UINT64 *ConfigurationDescriptor);

#endif /* _CONFIGURATION_MAP_HELPER_LIB_H_ */