/** @file
  Read-only UFS logical-unit access for PrePi.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrePiUfsLib.h>
#include <Library/PrePiUfsReadLib.h>

#define UFS_MAX_LUNS       8U
#define UFS_DESCRIPTOR_MAX 255U
#define UFS_BOUNCE_PAGES   16U

STATIC PRE_PI_UFS_LUN  mLuns[UFS_MAX_LUNS];
STATIC UINTN           mLunCount;
STATIC UINT8           *mBounce;
STATIC BOOLEAN         mOpen;

EFI_STATUS
EFIAPI
PrePiUfsReadClose (
  VOID
  )
{
  EFI_STATUS  Status;

  if (!mOpen) {
    return EFI_NOT_STARTED;
  }

  Status = PrePiUfsTransportClose ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mOpen = FALSE;
  mLunCount = 0;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PrePiUfsReadOpen (
  VOID
  )
{
  UINT8       Descriptor[UFS_DESCRIPTOR_MAX];
  UINTN       Index;
  UINTN       Byte;
  UINT64      Blocks;
  EFI_STATUS  Status;
  EFI_STATUS  CleanupStatus;

  if (mOpen) {
    return EFI_ALREADY_STARTED;
  }

  Status = PrePiUfsTransportOpen ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mOpen = TRUE;
  mLunCount = 0;
  Status = PrePiUfsTransportDescriptor (0, 0, Descriptor);
  if (EFI_ERROR (Status)) {
    goto Error;
  }

  if ((Descriptor[0] <= 6) || (Descriptor[6] > UFS_MAX_LUNS)) {
    Status = EFI_UNSUPPORTED;
    goto Error;
  }

  for (Index = 0; Index < UFS_MAX_LUNS; Index++) {
    Status = PrePiUfsTransportDescriptor (2, (UINT8)Index, Descriptor);
    if (Status == EFI_NOT_FOUND) {
      continue;
    }

    if (EFI_ERROR (Status)) {
      goto Error;
    }

    if ((Descriptor[0] < 0x13) || (Descriptor[2] != Index)) {
      Status = EFI_DEVICE_ERROR;
      goto Error;
    }

    if (Descriptor[3] == 0) {
      continue;
    }

    if ((Descriptor[3] != 1) || (Descriptor[10] < 2) || (Descriptor[10] > 16)) {
      Status = EFI_UNSUPPORTED;
      goto Error;
    }

    Blocks = 0;
    for (Byte = 0; Byte < 8; Byte++) {
      Blocks = (Blocks << 8) | Descriptor[11 + Byte];
    }

    if (Blocks == 0) {
      Status = EFI_DEVICE_ERROR;
      goto Error;
    }

    mLuns[mLunCount].Lun = (UINT8)Index;
    mLuns[mLunCount].BlockSize = 1U << Descriptor[10];
    mLuns[mLunCount].BlockCount = Blocks;
    mLunCount++;
  }

  if (mBounce == NULL) {
    mBounce = AllocatePages (UFS_BOUNCE_PAGES);
    if (mBounce == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto Error;
    }
  }

  return EFI_SUCCESS;

Error:
  DEBUG ((DEBUG_ERROR, "PrePiUfsRead: enumeration failed: %r\n", Status));
  CleanupStatus = PrePiUfsReadClose ();
  if (EFI_ERROR (CleanupStatus)) {
    DEBUG ((DEBUG_ERROR, "PrePiUfsRead: DMA cleanup failed: %r\n", CleanupStatus));
    CpuDeadLoop ();
  }

  return Status;
}

EFI_STATUS
EFIAPI
PrePiUfsReadGetLuns (
  IN OUT UINTN            *Count,
  OUT PRE_PI_UFS_LUN      *Luns OPTIONAL
  )
{
  UINTN  Capacity;

  if (!mOpen) {
    return EFI_NOT_STARTED;
  }

  if (Count == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Capacity = *Count;
  *Count = mLunCount;
  if (Capacity < mLunCount) {
    return EFI_BUFFER_TOO_SMALL;
  }

  if ((mLunCount != 0) && (Luns == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  if (mLunCount != 0) {
    CopyMem (Luns, mLuns, mLunCount * sizeof (mLuns[0]));
  }

  return EFI_SUCCESS;
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
  UINTN       Index;
  UINTN       Block;
  UINT32      Size;
  EFI_STATUS  Status;

  if (!mOpen) {
    return EFI_NOT_STARTED;
  }

  if ((Buffer == NULL) || (BlockCount == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < mLunCount; Index++) {
    if (mLuns[Index].Lun == Lun) {
      break;
    }
  }

  if (Index == mLunCount) {
    return EFI_NOT_FOUND;
  }

  Size = mLuns[Index].BlockSize;
  if ((Lba >= mLuns[Index].BlockCount) ||
      ((UINT64)BlockCount > mLuns[Index].BlockCount - Lba) ||
      (BlockCount > MAX_UINTN / Size) || (BufferSize < BlockCount * Size)) {
    return EFI_BAD_BUFFER_SIZE;
  }

  for (Block = 0; Block < BlockCount; Block++) {
    Status = PrePiUfsTransportReadBlock (Lun, Lba + Block, Size, mBounce);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    CopyMem ((UINT8 *)Buffer + Block * Size, mBounce, Size);
  }

  return EFI_SUCCESS;
}
