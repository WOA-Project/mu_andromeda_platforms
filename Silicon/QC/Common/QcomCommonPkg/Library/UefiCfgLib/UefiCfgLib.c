/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Library/PlatformHobs.h>
#include <Library/ShLib.h>
#include <Library/UefiCfgLib.h>

STATIC UefiCfgLibType *UefiCfgLibPtr = NULL;

EFI_STATUS UefiCfgInitialize(VOID)
{
  return LoadLib("UEFI Config Lib", 0, (VOID *)&UefiCfgLibPtr);
}

EFI_STATUS EFIAPI GetMemInfoByAddress(UINT64 RegionBaseAddress, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLibPtr == NULL) {
    Status = UefiCfgInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (UefiCfgLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return UefiCfgLibPtr->GetMemInfoByAddress(RegionBaseAddress, MemRegions);
}

EFI_STATUS EFIAPI GetMemInfoByName(CHAR8 *RegionName, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLibPtr == NULL) {
    Status = UefiCfgInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (UefiCfgLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return UefiCfgLibPtr->GetMemInfoByName(RegionName, MemRegions);
}

EFI_STATUS GetCfgInfoString(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLibPtr == NULL) {
    Status = UefiCfgInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (UefiCfgLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return UefiCfgLibPtr->GetCfgInfoString(Key, Value, ValBuffSize);
}

EFI_STATUS GetCfgInfoVal(CHAR8 *Key, UINT32 *Value)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLibPtr == NULL) {
    Status = UefiCfgInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (UefiCfgLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return UefiCfgLibPtr->GetCfgInfoVal(Key, Value);
}

EFI_STATUS GetCfgInfoVal64(CHAR8 *Key, UINT64 *Value)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLibPtr == NULL) {
    Status = UefiCfgInitialize();
    if (EFI_ERROR(Status)) {
      return Status;
    }
  }
  
  if (UefiCfgLibPtr == NULL) {
      return EFI_NOT_READY;
  }

  return UefiCfgLibPtr->GetCfgInfoVal64(Key, Value);
}