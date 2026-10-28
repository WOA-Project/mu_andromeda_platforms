/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PlatformMemoryMapLib.h>
#include <Library/AndromedaMemoryMapLib.h>
#include <Library/QcomUefiPlatMemoryMapLib.h>

STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX
    gDeviceMemoryDescriptorEx[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];

STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX gMemoryMapTerminator = {
    "Terminator", 0, 0, 0, 0, 0, 0, 0};

ARM_MEMORY_REGION_DESCRIPTOR_EX *LoadPlatformMemoryMap()
{
  UINTN       ConfiguredCount;
  EFI_STATUS  Status;

  ConfiguredCount = 0;
  Status = MemoryMapConfigLoadDescriptorSection (
             "MemoryMap",
             gDeviceMemoryDescriptorEx,
             MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1,
             &ConfiguredCount
             );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  Status = MemoryMapConfigLoadDescriptorSection (
             "RegisterMap",
             gDeviceMemoryDescriptorEx,
             MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1,
             &ConfiguredCount
             );
  if (EFI_ERROR (Status)) {
    return NULL;
  }

  if (ConfiguredCount == 0) {
    return NULL;
  }

  gDeviceMemoryDescriptorEx[ConfiguredCount] = gMemoryMapTerminator;

  return gDeviceMemoryDescriptorEx;
}