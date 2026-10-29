/** @file
  Accessors for numeric values in the [AndromedaUefi] boot-config section.
**/

#include <Guid/AndromedaBootConfig.h>
#include <Library/AndromedaBootConfigLib.h>
#include <Library/AndromedaUefiConfigHelperLib.h>

EFI_STATUS
EFIAPI
GetAndromedaUefiConfigUint32ByName (
  IN  CONST CHAR8  *Key,
  OUT UINT32       *Value
  )
{
  if ((Key == NULL) || (Value == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return GetAndromedaBootConfigUint32 (
           ANDROMEDA_BOOT_CONFIG_UEFI_SECTION,
           Key,
           Value
           );
}

EFI_STATUS
EFIAPI
GetAndromedaUefiConfigUint64ByName (
  IN  CONST CHAR8  *Key,
  OUT UINT64       *Value
  )
{
  if ((Key == NULL) || (Value == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  return GetAndromedaBootConfigUint64 (
           ANDROMEDA_BOOT_CONFIG_UEFI_SECTION,
           Key,
           Value
           );
}
