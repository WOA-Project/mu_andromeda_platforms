/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

/* Used to read Ram Info */
#include <Protocol/EFIRamPartition.h>

#include "MemoryUtils.h"

EFI_STATUS
EFIAPI
GetSystemMemorySize(UINT64 *SystemMemorySize)
{
  EFI_STATUS                 Status;
  EFI_RAMPARTITION_PROTOCOL *mRamPartitionProtocol = NULL;
  RamPartitionEntry         *RamPartitions         = NULL;
  UINT32                     NumPartitions         = 0;

  // Locate Qualcomm RamPartition Protocol (Needs EnvDxe!)
  Status = gBS->LocateProtocol(
      &gEfiRamPartitionProtocolGuid, NULL, (VOID *)&mRamPartitionProtocol);

  if (EFI_ERROR(Status) || mRamPartitionProtocol == NULL) {
    return Status;
  }

  Status = mRamPartitionProtocol->GetRamPartitions(
      mRamPartitionProtocol, NULL, &NumPartitions);

  if (Status == EFI_BUFFER_TOO_SMALL) {
    RamPartitions = AllocateZeroPool(NumPartitions * sizeof(RamPartitionEntry));

    Status = mRamPartitionProtocol->GetRamPartitions(
        mRamPartitionProtocol, RamPartitions, &NumPartitions);
  }

  if (EFI_ERROR(Status) || (NumPartitions < 1)) {
    DEBUG((EFI_D_ERROR, "Failed to get RAM partitions\n"));

    if (RamPartitions != NULL) {
      FreePool(RamPartitions);
      RamPartitions = NULL;
    }

    return Status;
  }

  *SystemMemorySize = 0;

  for (UINTN i = 0; i < NumPartitions; i++) {
    *SystemMemorySize += RamPartitions[i].AvailableLength;
  }

  if (RamPartitions != NULL) {
    FreePool(RamPartitions);
    RamPartitions = NULL;
  }

  DEBUG(
      (EFI_D_WARN, "The Total SystemMemorySize is 0x%016llx\n",
       *SystemMemorySize));

  UINTN DesignMemorySize = 0;

  while (*SystemMemorySize > DesignMemorySize) {
    DesignMemorySize += 0x40000000; // 1GB
  }

  DEBUG(
      (EFI_D_WARN, "The Total DesignMemorySize is 0x%016llx\n",
       DesignMemorySize));

  *SystemMemorySize = DesignMemorySize;

  return Status;
}