/** @file
  Null early UFS implementation.

  SPDX-License-Identifier: MIT
**/

#include <Library/PrePiUfsLib.h>

EFI_STATUS
PlatformReinitializeUfs (
  VOID
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformPrepareInheritedUfs (
  VOID
  )
{
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformFinishUfs (
  VOID
  )
{
  return EFI_SUCCESS;
}
