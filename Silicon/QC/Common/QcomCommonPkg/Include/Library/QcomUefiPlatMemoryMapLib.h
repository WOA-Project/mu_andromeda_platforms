/** @file
  Public interfaces for loading memory-map records from boot configuration.

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT
**/

#ifndef QCOM_MEMORY_MAP_CONFIG_LIB_H_
#define QCOM_MEMORY_MAP_CONFIG_LIB_H_

#include <Library/PlatformDdrBankMapLib.h>
#include <Library/PlatformMemoryMapLib.h>

#define MEMORY_MAP_CONFIG_ENTRY_SIZE 512

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
EFI_STATUS
MemoryMapConfigLoadPStoreDescriptor (
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor
  );

EFI_STATUS
MemoryMapConfigOverlayDescriptor (
  IN OUT  ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptors,
  IN OUT  UINTN                            *DescriptorCount,
  IN      UINTN                            DescriptorCapacity,
  IN      CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Overlay,
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX  *Workspace
  );

EFI_STATUS
MemoryMapConfigValidatePStorePlacement (
  IN      CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptors,
  IN      UINTN                                  DescriptorCount,
  IN      CONST PLATFORM_DDR_BANK_REGION         *DdrBankRegions,
  IN      UINTN                                  DdrBankRegionCount
  );
#endif

EFI_STATUS
MemoryMapConfigLoadDescriptorSection (
  IN      CONST CHAR8                       *Section,
  IN OUT  ARM_MEMORY_REGION_DESCRIPTOR_EX   *Descriptors,
  IN      UINTN                             DescriptorCapacity,
  IN OUT  UINTN                             *DescriptorCount
  );

#endif
