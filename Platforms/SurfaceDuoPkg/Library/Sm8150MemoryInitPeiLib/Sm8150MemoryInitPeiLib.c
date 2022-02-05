/** @file
*
*  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <PiPei.h>

#include <Library/ArmMmuLib.h>
#include <Library/ArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>

extern UINT64 mSystemMemoryEnd;

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

  VOID                          *TranslationTableBase;
  UINTN                         TranslationTableSize;
  RETURN_STATUS                 Status;

  //Note: Because we called PeiServicesInstallPeiMemory() before to call InitMmu() the MMU Page Table resides in
  //      DRAM (even at the top of DRAM as it is the first permanent memory allocation)
  Status = ArmConfigureMmu (MemoryTable, &TranslationTableBase, &TranslationTableSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "Error: Failed to enable MMU: %r\n", Status));
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
  IN EFI_PHYSICAL_ADDRESS               UefiMemoryBase,
  IN UINT64                             UefiMemorySize
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR *MemoryTable;

  // Get Virtual Memory Map from the Platform Library
  ArmPlatformGetVirtualMemoryMap (&MemoryTable);

  // Ensure PcdSystemMemorySize has been set
  ASSERT (PcdGet64 (PcdSystemMemorySize) != 0);

  // Build Memory Allocation Hob
  DEBUG ((EFI_D_INFO, "Configure MMU In \n"));
  InitMmu(MemoryTable);
  DEBUG ((EFI_D_INFO, "Configure MMU Out \n"));

  if (FeaturePcdGet (PcdPrePiProduceMemoryTypeInformationHob)) {
    // Optional feature that helps prevent EFI memory map fragmentation.
    BuildMemoryTypeInformationHob ();
  }

  return EFI_SUCCESS;
}
