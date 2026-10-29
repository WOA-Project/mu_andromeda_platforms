/** @file
  Configuration-map lookups for SEC.
**/

#include <Guid/AndromedaBootConfig.h>
#include <Library/AndromedaBootConfigLib.h>
#include <Library/QcomConfigParametersHelperLib.h>

EFI_STATUS EFIAPI QcomLocateConfigurationMapStringByName(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize)
{
  EFI_STATUS  Status;
  UINTN       Index;
  UINTN       ValueSize;

  if ((Key == NULL) ||
      (ValBuffSize == NULL) ||
      ((*ValBuffSize > 0) && (Value == NULL))) {
    return EFI_INVALID_PARAMETER;
  }

  ValueSize = *ValBuffSize;
  Status = GetAndromedaBootConfigValue (
             ANDROMEDA_BOOT_CONFIG_CONFIG_PARAMETERS_SECTION,
             Key,
             Value,
             &ValueSize
             );
  *ValBuffSize = ValueSize;
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((ValueSize > 1) &&
      Value[0] == '"') {
    if ((ValueSize < 3) || (Value[ValueSize - 2] != '"')) {
      return EFI_COMPROMISED_DATA;
    }

    for (Index = 1; Index < ValueSize - 2; Index++) {
      Value[Index - 1] = Value[Index];
    }

    Value[ValueSize - 3] = '\0';
    *ValBuffSize = ValueSize - 2;
  }

  return EFI_SUCCESS;
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT32ByName(CHAR8 *Key, UINT32 *Value)
{
  if ((Key == NULL) || (Value == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return GetAndromedaBootConfigUint32 (
           ANDROMEDA_BOOT_CONFIG_CONFIG_PARAMETERS_SECTION,
           Key,
           Value
           );
}

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT64ByName(CHAR8 *Key, UINT64 *Value)
{
  if ((Key == NULL) || (Value == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return GetAndromedaBootConfigUint64 (
           ANDROMEDA_BOOT_CONFIG_CONFIG_PARAMETERS_SECTION,
           Key,
           Value
           );
}
