/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/QcomConfigParametersHelperLib.h>
#include <Library/HobLib.h>
#include <Library/PlatformHobs.h>
#include <Library/ShLib.h>
#include <Library/UefiCfgLib.h>

EFI_STATUS EFIAPI QcomLocateConfigurationMapStringByName(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize)
{
  UefiCfgLibType *UefiCfgLib = NULL;

  if (EFI_ERROR(GetUefiCfgLib(&UefiCfgLib)) || UefiCfgLib == NULL) {
    return EFI_NOT_READY;
  }

  return UefiCfgLib->GetCfgInfoString(Key, Value, ValBuffSize);
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT32ByName(CHAR8 *Key, UINT32 *Value)
{
  UefiCfgLibType *UefiCfgLib = NULL;

  if (EFI_ERROR(GetUefiCfgLib(&UefiCfgLib)) || UefiCfgLib == NULL) {
    return EFI_NOT_READY;
  }

  return UefiCfgLib->CfgGetCfgInfoVal(Key, Value);
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT64ByName(CHAR8 *Key, UINT64 *Value)
{
  UefiCfgLibType *UefiCfgLib = NULL;

  if (EFI_ERROR(GetUefiCfgLib(&UefiCfgLib)) || UefiCfgLib == NULL) {
    return EFI_NOT_READY;
  }

  return UefiCfgLib->CfgGetCfgInfoVal64(Key, Value);
}