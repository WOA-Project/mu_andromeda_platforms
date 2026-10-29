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

ShLibLoaderType *ShLib = NULL;
UefiCfgLibType *gUefiCfgLib = NULL;

EFI_STATUS EFIAPI GetUefiCfgLib(UefiCfgLibType** UefiCfgLib)
{
  EFI_STATUS Status = EFI_SUCCESS;

  if (UefiCfgLib == NULL) {
    return EFI_COMPROMISED_DATA;
  }

  if (ShLib == NULL) {
    ShLib = GetEfiShLibHob();
    
    if (ShLib == NULL) {
      return EFI_NOT_READY;
    }
  }

  if (gUefiCfgLib == NULL) {
    Status = ShLib->LoadLib("UEFI Config Lib", 0, (VOID **)&gUefiCfgLib);
    if (EFI_SUCCESS(Status)) {
      *UefiCfgLib = gUefiCfgLib;
    }
  }

  return Status;
}