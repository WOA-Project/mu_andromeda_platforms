/** @file
  Shared definitions for the runtime boot configuration HOB.
**/

#ifndef ANDROMEDA_BOOT_CONFIG_H_
#define ANDROMEDA_BOOT_CONFIG_H_

#include <Uefi.h>

#define ANDROMEDA_BOOT_CONFIG_MAX_SIZE                  0xFFFF
#define ANDROMEDA_BOOT_CONFIG_UEFI_SECTION              "AndromedaUefi"
#define ANDROMEDA_BOOT_CONFIG_CONFIG_PARAMETERS_SECTION "ConfigParameters"
#define ANDROMEDA_BOOT_CONFIG_SECURE_BOOT_ENABLED_KEY  "SecureBootEnabled"

extern EFI_GUID  gAndromedaBootConfigHobGuid;

#endif
