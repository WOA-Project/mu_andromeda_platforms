/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/QcomUefiCfgConfigParametersHelperLib.h>
#include <Library/HobLib.h>
#include <Library/PlatformHobs.h>
#include <Library/ShLib.h>
#include <Library/UefiCfgLib.h>

EFI_STATUS EFIAPI QcomLocateConfigurationMapStringByName(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize)
{
  return GetCfgInfoString(Key, Value, ValBuffSize);
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT32ByName(CHAR8 *Key, UINT32 *Value)
{
  return CfgGetCfgInfoVal(Key, Value);
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT64ByName(CHAR8 *Key, UINT64 *Value)
{
  return CfgGetCfgInfoVal64(Key, Value);
}