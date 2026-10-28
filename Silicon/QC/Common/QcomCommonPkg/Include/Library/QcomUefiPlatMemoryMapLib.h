/** @file
  Public interfaces for loading memory-map records from boot configuration.

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT
**/

#ifndef QCOM_MEMORY_MAP_CONFIG_LIB_H_
#define QCOM_MEMORY_MAP_CONFIG_LIB_H_

#include <Library/PlatformDdrBankMapLib.h>
#include <Library/AndromedaMemoryMapLib.h>

#define MEMORY_MAP_CONFIG_ENTRY_SIZE 512

EFI_STATUS
MemoryMapConfigLoadDescriptorSection (
  IN      CONST CHAR8                       *Section,
  IN OUT  ARM_MEMORY_REGION_DESCRIPTOR_EX   *Descriptors,
  IN      UINTN                             DescriptorCapacity,
  IN OUT  UINTN                             *DescriptorCount
  );

#endif
