/** @file
  Andromeda-specific ArmPlatformLib interfaces.
**/

#ifndef ANDROMEDA_ARM_PLATFORM_LIB_H_
#define ANDROMEDA_ARM_PLATFORM_LIB_H_

#include <Uefi.h>

/**
  Retrieve the boot configuration published by ArmPlatformPeiBootAction.

  @param[out] Config      Pointer to the boot configuration bytes.
  @param[out] ConfigSize  Configuration size in bytes.

  @retval EFI_SUCCESS           The published values were returned.
  @retval EFI_INVALID_PARAMETER A required output parameter is NULL.
**/
EFI_STATUS
EFIAPI
ArmPlatformGetBootConfig (
  OUT CONST CHAR8  **Config,
  OUT UINTN        *ConfigSize
  );

#endif
