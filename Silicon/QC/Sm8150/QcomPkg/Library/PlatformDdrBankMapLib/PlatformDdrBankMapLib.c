/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Library/PlatformDdrBankMapLib.h>

STATIC CONST PLATFORM_DDR_BANK_REGION gDdrBankRegions[] = {
  // DDR Bank 0 Start
  { 0x80000000, 0x3CC00000 },
  // DDR Bank 0 End
  // Carveout: [0xBCC00000, 0xC0000000), size 0x03400000.
  // DDR Bank 1 Start
  { 0xC0000000, 0x80000000 },
  // DDR Bank 1 End
  // DDR Bank 2 Start
  { 0x140000000, 0xC0000000 },
  // DDR Bank 2 End
};

EFI_STATUS
EFIAPI
GetPlatformDdrBankRegions (
  OUT CONST PLATFORM_DDR_BANK_REGION  **Regions,
  OUT UINTN                           *RegionCount
  )
{
  if ((Regions == NULL) || (RegionCount == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  *Regions     = gDdrBankRegions;
  *RegionCount = ARRAY_SIZE (gDdrBankRegions);
  return EFI_SUCCESS;
}
