/** @file
  Read-only UFS logical units for PrePi.

  SPDX-License-Identifier: MIT
**/

#ifndef PRE_PI_UFS_READ_LIB_H_
#define PRE_PI_UFS_READ_LIB_H_

#include <Uefi.h>

typedef struct {
  UINT8   Lun;
  UINT32  BlockSize;
  UINT64  BlockCount;
} PRE_PI_UFS_LUN;

// PlatformPrepareInheritedUfs or PlatformReinitializeUfs must have completed
// successfully first. The session owns the UFS DMA mapping until Close;
// it is not reentrant.
EFI_STATUS EFIAPI PrePiUfsReadOpen (VOID);

// Set Count to the buffer capacity on entry and required/returned count on exit.
EFI_STATUS EFIAPI PrePiUfsReadGetLuns (
  IN OUT UINTN *Count,
  OUT PRE_PI_UFS_LUN *Luns OPTIONAL
  );

EFI_STATUS EFIAPI PrePiUfsReadBlocks (
  IN UINT8 Lun,
  IN UINT64 Lba,
  IN UINTN BlockCount,
  IN UINTN BufferSize,
  OUT VOID *Buffer
  );

// A close failure leaves DMA resources owned. Do not enter DXE on failure.
EFI_STATUS EFIAPI PrePiUfsReadClose (VOID);

#endif
