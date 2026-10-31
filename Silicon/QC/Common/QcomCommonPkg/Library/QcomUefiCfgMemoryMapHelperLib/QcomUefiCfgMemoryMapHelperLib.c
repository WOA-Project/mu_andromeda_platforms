/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/QcomUefiCfgMemoryMapHelperLib.h>
#include <Library/PlatformHobs.h>
#include <Library/UefiCfgLib.h>

EFI_STATUS EFIAPI QcomLocateMemoryMapAreaByName(
    CHAR8 *MemoryMapAreaName, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return GetMemInfoByName(MemoryMapAreaName, MemoryDescriptor);
}

EFI_STATUS EFIAPI QcomLocateMemoryMapAreaByAddress(
    EFI_PHYSICAL_ADDRESS             MemoryMapAreaAddress,
    ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return GetMemInfoByAddress(MemoryMapAreaAddress, MemoryDescriptor);
}