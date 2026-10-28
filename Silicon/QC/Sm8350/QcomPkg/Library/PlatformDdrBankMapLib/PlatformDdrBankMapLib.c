/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Library/PlatformDdrBankMapLib.h>
#include <Configuration/XblHlosHob.h>

#define XBL_HLOS_HOB_ADDRESS  0xE3400000

STATIC CONST PLATFORM_DDR_BANK_REGION gDdrBankRegionsEV[] = {
  // DDR Bank 0 Start
  { 0x80000000, 0x39300000 },
  // DDR Bank 0 End
  // Carveout: [0xB9300000, 0xB9900000), size 0x00600000.
  // DDR Bank 1 Start
  { 0xB9900000, 0x01E00000 },
  // DDR Bank 1 End
  // Carveout: [0xBB700000, 0xC0000000), size 0x04900000.
  // DDR Bank 2 Start
  { 0xC0000000, 0x1C0000000 },
  // DDR Bank 2 End
};

STATIC CONST PLATFORM_DDR_BANK_REGION gDdrBankRegionsDV[] = {
  // DDR Bank 0 Start
  { 0x80000000, 0x39700000 },
  // DDR Bank 0 End
  // Carveout: [0xB9700000, 0xB9D00000), size 0x00600000.
  // DDR Bank 1 Start
  { 0xB9D00000, 0x01E00000 },
  // DDR Bank 1 End
  // Carveout: [0xBBB00000, 0xC0000000), size 0x04500000.
  // DDR Bank 2 Start
  { 0xC0000000, 0x1C0000000 },
  // DDR Bank 2 End
};

EFI_STATUS
EFIAPI
GetPlatformDdrBankRegions (
  OUT CONST PLATFORM_DDR_BANK_REGION  **Regions,
  OUT UINTN                           *RegionCount
  )
{
  PXBL_HLOS_HOB  XblHlosHob;

  if ((Regions == NULL) || (RegionCount == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  XblHlosHob = (PXBL_HLOS_HOB)XBL_HLOS_HOB_ADDRESS;
  if (XblHlosHob->BoardID < 0xA) {
    *Regions     = gDdrBankRegionsEV;
    *RegionCount = ARRAY_SIZE (gDdrBankRegionsEV);
  } else {
    *Regions     = gDdrBankRegionsDV;
    *RegionCount = ARRAY_SIZE (gDdrBankRegionsDV);
  }

  return EFI_SUCCESS;
}
