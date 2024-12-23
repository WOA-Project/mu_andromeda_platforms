/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _PLATFORM_CONFIGURATION_MAP_LIB_H_
#define _PLATFORM_CONFIGURATION_MAP_LIB_H_

#include <PiPei.h>
#include <Library/ArmLib.h>

#define CONFIGURATION_NAME_MAX_LENGTH 64

typedef struct {
  CHAR8                        Name[CONFIGURATION_NAME_MAX_LENGTH];
  UINT64                       Value;
} CONFIGURATION_DESCRIPTOR_EX, *PCONFIGURATION_DESCRIPTOR_EX;

CONFIGURATION_DESCRIPTOR_EX* GetPlatformConfigurationMap();

#endif /* _PLATFORM_CONFIGURATION_MAP_LIB_H_ */