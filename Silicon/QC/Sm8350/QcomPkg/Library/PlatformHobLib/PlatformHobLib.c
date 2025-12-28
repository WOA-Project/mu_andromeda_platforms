/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Uefi.h>

#include <Configuration/XblHlosHob.h>
#include <Library/BaseLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/PlatformHobLib.h>

CHAR8 *PlatformSubTypeSkuString[] = {
    [PLATFORM_SUBTYPE_UNKNOWN]                 = "Unknown",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV1]        = "5G",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV1_1]      = "5G",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2]        = "5G",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_1]      = "5G",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_2]      = "5G",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_wifi]   = "Wifi",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_SKIP]       = "Unknown",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV3_Retail] = "Retail",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV3_Debug]  = "Debug",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_DV_Retail]  = "Retail",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_DV_Debug]   = "Debug",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_MP_Retail]  = "Retail"};

CHAR8 *PlatformSubTypeBuildString[] = {
    [PLATFORM_SUBTYPE_UNKNOWN]                 = "Unknown",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV1]        = "Zeta EV1",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV1_1]      = "Zeta EV1.1",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2]        = "Zeta EV2",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_1]      = "Zeta EV2.1",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_2]      = "Zeta EV2.2",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV2_wifi]   = "Zeta EV2.2",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_SKIP]       = "Unknown",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV3_Retail] = "Zeta EV3",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_EV3_Debug]  = "Zeta EV3",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_DV_Retail]  = "Zeta DV",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_DV_Debug]   = "Zeta DV",
    [SURFACE_DUO2_PLATFORM_SUBTYPE_MP_Retail]  = "Zeta MP"};

PXBL_HLOS_HOB GetPlatformHob()
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX AppsHobMemoryRegion;
  LocateMemoryMapAreaByName("Apps Hob", &AppsHobMemoryRegion);
  return (PXBL_HLOS_HOB)AppsHobMemoryRegion.Address;
}

CHAR8 *GetPlatformSubTypeSkuString()
{
  sboard_id_t BoardID = GetBoardID();
  return PlatformSubTypeSkuString[BoardID];
}

CHAR8 *GetPlatformSubTypeBuildString()
{
  sboard_id_t BoardID = GetBoardID();
  return PlatformSubTypeBuildString[BoardID];
}

CHAR8 *GetTouchFWVersion()
{
  PXBL_HLOS_HOB PlatformHob = GetPlatformHob();
  return PlatformHob->TouchFWVersion;
}

sboard_id_t GetBoardID()
{
  PXBL_HLOS_HOB PlatformHob = GetPlatformHob();
  UINT8         BoardID     = PlatformHob->BoardID;

  if (BoardID < SURFACE_DUO2_PLATFORM_SUBTYPE_EV1 ||
      BoardID > SURFACE_DUO2_PLATFORM_SUBTYPE_MP_Retail) {
    return PLATFORM_SUBTYPE_UNKNOWN;
  }

  if (BoardID == SURFACE_DUO2_PLATFORM_SUBTYPE_SKIP) {
    return PLATFORM_SUBTYPE_UNKNOWN;
  }

  return (sboard_id_t)BoardID;
}

sproduct_id_t GetProductID()
{
  PXBL_HLOS_HOB PlatformHob = GetPlatformHob();
  UINT16        ProductId   = PlatformHob->ProductId;

  if (ProductId <= OEM_UNINITIALISED || ProductId > SURFACE_DUO2) {
    return OEM_UNINITIALISED;
  }

  return (sproduct_id_t)ProductId;
}