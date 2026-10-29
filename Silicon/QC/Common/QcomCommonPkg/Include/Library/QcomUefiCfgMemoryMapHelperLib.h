/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _QCOM_MEMORY_MAP_HELPER_LIB_H_
#define _QCOM_MEMORY_MAP_HELPER_LIB_H_

#include <Library/PlatformMemoryMapLib.h>

EFI_STATUS EFIAPI QcomLocateMemoryMapAreaByName(
    CHAR8                           *MemoryMapAreaName,
    ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);

EFI_STATUS EFIAPI QcomLocateMemoryMapAreaByAddress(
    EFI_PHYSICAL_ADDRESS             MemoryMapAreaAddress,
    ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);

#endif /* _QCOM_MEMORY_MAP_HELPER_LIB_H_ */