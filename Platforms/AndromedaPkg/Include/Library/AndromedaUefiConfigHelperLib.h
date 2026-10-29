/** @file
  Accessors for numeric values in the [AndromedaUefi] boot-config section.
**/

#ifndef ANDROMEDA_UEFI_CONFIG_HELPER_LIB_H_
#define ANDROMEDA_UEFI_CONFIG_HELPER_LIB_H_

#include <Uefi.h>

/**
  Get a UINT32 value from the [AndromedaUefi] section.

  @param[in]  Key    Configuration key.
  @param[out] Value  Parsed value.

  @retval EFI_SUCCESS           Value was found and parsed.
  @retval EFI_INVALID_PARAMETER A required parameter is NULL.
  @retval EFI_NOT_FOUND         The key was not found.
  @retval EFI_COMPROMISED_DATA  The value is invalid or exceeds UINT32 range.
**/
EFI_STATUS
EFIAPI
GetAndromedaUefiConfigUint32ByName (
  IN  CONST CHAR8  *Key,
  OUT UINT32       *Value
  );

/**
  Get a UINT64 value from the [AndromedaUefi] section.

  @param[in]  Key    Configuration key.
  @param[out] Value  Parsed value.

  @retval EFI_SUCCESS           Value was found and parsed.
  @retval EFI_INVALID_PARAMETER A required parameter is NULL.
  @retval EFI_NOT_FOUND         The key was not found.
  @retval EFI_COMPROMISED_DATA  The value is invalid or exceeds UINT64 range.
**/
EFI_STATUS
EFIAPI
GetAndromedaUefiConfigUint64ByName (
  IN  CONST CHAR8  *Key,
  OUT UINT64       *Value
  );

#endif
