/** @file
  Platform-specific DDR bank region provider interface.
**/

#ifndef PLATFORM_DDR_BANK_MAP_LIB_H_
#define PLATFORM_DDR_BANK_MAP_LIB_H_

#include <Uefi.h>

typedef struct {
  EFI_PHYSICAL_ADDRESS Base;
  UINT64               Length;
} PLATFORM_DDR_BANK_REGION;

/**
  Get the DDR bank ranges for the current platform and board revision.

  @param[out] Regions      Pointer to the platform's static DDR bank ranges.
  @param[out] RegionCount  Number of ranges in Regions.

  @retval EFI_SUCCESS           DDR bank ranges were returned.
  @retval EFI_INVALID_PARAMETER A required parameter is NULL.
**/
EFI_STATUS
EFIAPI
GetPlatformDdrBankRegions (
  OUT CONST PLATFORM_DDR_BANK_REGION  **Regions,
  OUT UINTN                           *RegionCount
  );

#endif
