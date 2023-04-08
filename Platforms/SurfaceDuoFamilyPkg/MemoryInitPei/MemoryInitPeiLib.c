/** @file

  Copyright (c) 2011-2015, ARM Limited. All rights reserved.

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformMemoryMapLib.h>

VOID
BuildMemoryTypeInformationHob (
  VOID
  );

STATIC
VOID
InitMmu (
  IN ARM_MEMORY_REGION_DESCRIPTOR  *MemoryTable
  )
{
  VOID           *TranslationTableBase;
  UINTN          TranslationTableSize;
  RETURN_STATUS  Status;

  // Note: Because we called PeiServicesInstallPeiMemory() before to call InitMmu() the MMU Page Table resides in
  //      DRAM (even at the top of DRAM as it is the first permanent memory allocation)
  Status = ArmConfigureMmu (MemoryTable, &TranslationTableBase, &TranslationTableSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "Error: Failed to enable MMU\n"));
  }
}

STATIC
VOID AddHob(PARM_MEMORY_REGION_DESCRIPTOR_EX Desc)
{
  if (Desc->HobOption != AllocOnly) {
    BuildResourceDescriptorHob(
        Desc->ResourceType, Desc->ResourceAttribute, Desc->Address, Desc->Length);
  }

  if (Desc->ResourceType == EFI_RESOURCE_SYSTEM_MEMORY ||
      Desc->MemoryType == EfiRuntimeServicesData)
  {
    BuildMemoryAllocationHob(Desc->Address, Desc->Length, Desc->MemoryType);
  }
}

/*++

Routine Description:



Arguments:

  FileHandle  - Handle of the file being invoked.
  PeiServices - Describes the list of possible PEI Services.

Returns:

  Status -  EFI_SUCCESS if the boot mode could be set

--*/
EFI_STATUS
EFIAPI
MemoryPeim (
  IN EFI_PHYSICAL_ADDRESS  UefiMemoryBase,
  IN UINT64                UefiMemorySize
  )
{

  DEBUG((EFI_D_INFO, "MemoryPeim Entry!\n"));

  DEBUG((EFI_D_INFO, "GetPlatformMemoryMap!\n"));

  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      GetPlatformMemoryMap();

  DEBUG((EFI_D_INFO, "GetPlatformMemoryMap Done!\n"));

  ARM_MEMORY_REGION_DESCRIPTOR
        MemoryTable[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
  UINTN Index = 0;

  DEBUG((EFI_D_INFO, "Ensure PcdSystemMemorySize has been set\n"));

  // Ensure PcdSystemMemorySize has been set
  ASSERT (PcdGet64 (PcdSystemMemorySize) != 0);

  DEBUG((EFI_D_INFO, "Ensure PcdSystemMemorySize has been set.. Done!\n"));

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    DEBUG((EFI_D_INFO, "Looping!\n"));

    switch (MemoryDescriptorEx->HobOption) {
    case AddMem:
    case AddDev:
    case HobOnlyNoCacheSetting:
    case AllocOnly:
      AddHob(MemoryDescriptorEx);
      break;
    case NoHob:
    default:
      goto update;
    }

    if (MemoryDescriptorEx->HobOption == HobOnlyNoCacheSetting) {
      MemoryDescriptorEx++;
      continue;
    }

  update:
    ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);

    MemoryTable[Index].PhysicalBase = MemoryDescriptorEx->Address;
    MemoryTable[Index].VirtualBase  = MemoryDescriptorEx->Address;
    MemoryTable[Index].Length       = MemoryDescriptorEx->Length;
    MemoryTable[Index].Attributes   = MemoryDescriptorEx->ArmAttributes;

    Index++;
    MemoryDescriptorEx++;
  }

  DEBUG((EFI_D_INFO, "Looping Done!\n"));

  // Last one (terminator)
  ASSERT(Index < MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT);
  MemoryTable[Index].PhysicalBase = 0;
  MemoryTable[Index].VirtualBase  = 0;
  MemoryTable[Index].Length       = 0;
  MemoryTable[Index].Attributes   = 0;

  DEBUG((EFI_D_INFO, "InitMmu!\n"));

  // Build Memory Allocation Hob
  InitMmu (MemoryTable);

  DEBUG((EFI_D_INFO, "InitMmu Done!\n"));

  if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional feature that helps prevent EFI memory map fragmentation.
    BuildMemoryTypeInformationHob ();
  }

  DEBUG((EFI_D_INFO, "MemoryPeim Done!\n"));

  return EFI_SUCCESS;
}