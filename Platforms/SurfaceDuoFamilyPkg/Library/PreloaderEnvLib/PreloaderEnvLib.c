#include <Base.h>
#include <Uefi.h>

#include <Configuration/Hob.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DebugPrintErrorLevelLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/PcdLib.h>
#include <Library/PreloaderEnvLib.h>
#include <Library/PrintLib.h>

ARM_MEMORY_REGION_DESCRIPTOR_EX PreloaderEnv;

VOID SetSkipBoot(BOOLEAN SkipBoot)
{
  PRELOADER_ENVIRONMENT *PreloaderEnvPtr =
      (PRELOADER_ENVIRONMENT *)PreloaderEnv.Address;

  if (SkipBoot) {
    PreloaderEnvPtr->SkipBoot = SKIP_BOOT_FLAG;
  }
  else {
    PreloaderEnvPtr->SkipBoot = 0;
  }
}

BOOLEAN GetSkipBoot(VOID)
{
  PRELOADER_ENVIRONMENT *PreloaderEnvPtr =
      (PRELOADER_ENVIRONMENT *)PreloaderEnv.Address;
  return PreloaderEnvPtr->SkipBoot == SKIP_BOOT_FLAG;
}

/**
  The constructor function initialize the Preloader Library

  @retval EFI_SUCCESS   The constructor always returns RETURN_SUCCESS.

**/
RETURN_STATUS
EFIAPI
PreloaderEnvLibConstructor(VOID)
{
  return LocateMemoryMapAreaByName(PRELOADER_ENV_NAME, &PreloaderEnv);
}