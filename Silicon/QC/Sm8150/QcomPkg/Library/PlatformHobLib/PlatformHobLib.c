/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Uefi.h>

#include <Configuration/XblHlosHob.h>
#include <Library/BaseLib.h>
#include <Library/PlatformHobLib.h>

CHAR8 *PlatformSubTypeSkuString[] = {
    [PLATFORM_SUBTYPE_UNKNOWN]                    = "Unknown",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_A]           = "A",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_B]           = "B",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_C]           = "C",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_5_A]         = "A",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_5_B]         = "B",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_A]           = "A",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_B]           = "B",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_A]         = "A",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_B]         = "B",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_C]         = "C",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_A_ALIAS_MP_A] = "A",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_B_ALIAS_MP_B] = "B",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_C_ALIAS_MP_C] = "C"
};

CHAR8 *PlatformSubTypeBuildString[] = {
    [PLATFORM_SUBTYPE_UNKNOWN]                    = "Unknown",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_A]           = "Epsilon EV1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_B]           = "Epsilon EV1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_C]           = "Epsilon EV1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_5_A]         = "Epsilon EV1.5",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV1_5_B]         = "Epsilon EV1.5",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_A]           = "Epsilon EV2",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_B]           = "Epsilon EV2",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_A]         = "Epsilon EV2.1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_B]         = "Epsilon EV2.1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_EV2_1_C]         = "Epsilon EV2.1",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_A_ALIAS_MP_A] = "Epsilon DV Alias MP",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_B_ALIAS_MP_B] = "Epsilon DV Alias MP",
    [SURFACE_DUO_PLATFORM_SUBTYPE_DV_C_ALIAS_MP_C] = "Epsilon DV Alias MP"
};

PXBL_HLOS_HOB GetPlatformHob() { return (PXBL_HLOS_HOB)0x146BFA94; }

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

  if (BoardID < SURFACE_DUO_PLATFORM_SUBTYPE_EV1_A ||
      BoardID > SURFACE_DUO_PLATFORM_SUBTYPE_DV_C_ALIAS_MP_C) {
    return PLATFORM_SUBTYPE_UNKNOWN;
  }

  return (sboard_id_t)BoardID;
}

sproduct_id_t GetProductID() { return SURFACE_DUO; }