/** @file
  Unavailable PrePi UFS read interface.
  SPDX-License-Identifier: MIT
**/
#include <Library/PrePiUfsReadLib.h>

EFI_STATUS
EFIAPI
PrePiUfsReadOpen (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PrePiUfsReadGetLuns (
  IN OUT UINTN            *Count,
  OUT PRE_PI_UFS_LUN      *Luns OPTIONAL
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PrePiUfsReadBlocks (
  IN UINT8   Lun,
  IN UINT64  Lba,
  IN UINTN   BlockCount,
  IN UINTN   BufferSize,
  OUT VOID   *Buffer
  )
{
  return EFI_UNSUPPORTED;
}

EFI_STATUS
EFIAPI
PrePiUfsReadClose (
  VOID
  )
{
  return EFI_UNSUPPORTED;
}
