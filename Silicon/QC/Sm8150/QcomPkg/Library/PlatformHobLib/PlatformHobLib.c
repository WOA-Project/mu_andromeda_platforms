/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <Library/BaseLib.h>
#include <Library/PlatformHobLib.h>
#include <Configuration/XblHlosHob.h>

PXBL_HLOS_HOB GetPlatformHob()
{
  return (PXBL_HLOS_HOB)0x146BFA94;
}