/** @file
*
*  Copyright (c) 2011, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>

// This varies by device
#include <Configuration/DeviceMemoryMap.h>

STATIC
VOID AddHob(ARM_MEMORY_REGION_DESCRIPTOR_EX Desc)
{
  BuildResourceDescriptorHob(
      Desc.ResourceType, Desc.ResourceAttribute, Desc.Address, Desc.Length);

  BuildMemoryAllocationHob(Desc.Address, Desc.Length, Desc.MemoryType);
}

/**
  Return the Virtual Memory Map of your platform

  This Virtual Memory Map is used by MemoryInitPei Module to initialize the MMU on your platform.

  @param[out]   VirtualMemoryMap    Array of ARM_MEMORY_REGION_DESCRIPTOR describing a Physical-to-
                                    Virtual Memory mapping. This array must be ended by a zero-filled
                                    entry

**/
VOID
ArmPlatformGetVirtualMemoryMap (
  IN ARM_MEMORY_REGION_DESCRIPTOR** VirtualMemoryMap
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR *MemoryDescriptor;
  UINTN                         Index = 0;

  MemoryDescriptor =
      (ARM_MEMORY_REGION_DESCRIPTOR *)AllocatePages(EFI_SIZE_TO_PAGES(
          sizeof(ARM_MEMORY_REGION_DESCRIPTOR) *
          MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT));

  // Run through each memory descriptor
  while (gDeviceMemoryDescriptorEx[Index].Address !=
         (EFI_PHYSICAL_ADDRESS)0xFFFFFFFF) {
    switch (gDeviceMemoryDescriptorEx[Index].HobOption) {
    case AddMem:
    case AddDev:
      AddHob(gDeviceMemoryDescriptorEx[Index]);
      break;
    case NoHob:
    default:
      goto update;
    }

  update:
    ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

    MemoryDescriptor[Index].PhysicalBase =
        gDeviceMemoryDescriptorEx[Index].Address;
    MemoryDescriptor[Index].VirtualBase =
        gDeviceMemoryDescriptorEx[Index].Address;
    MemoryDescriptor[Index].Length = gDeviceMemoryDescriptorEx[Index].Length;
    MemoryDescriptor[Index].Attributes =
        gDeviceMemoryDescriptorEx[Index].ArmAttributes;

    Index++;
  }

  // Last one (terminator)
  MemoryDescriptor[Index].PhysicalBase = 0;
  MemoryDescriptor[Index].VirtualBase  = 0;
  MemoryDescriptor[Index].Length       = 0;
  MemoryDescriptor[Index++].Attributes = (ARM_MEMORY_REGION_ATTRIBUTES)0;
  ASSERT(Index <= MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

  *VirtualMemoryMap = &MemoryDescriptor[0];
}
