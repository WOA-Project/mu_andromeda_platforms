/** @file
  Early UFS initialization.

  SPDX-License-Identifier: MIT
**/

#ifndef PRE_PI_UFS_LIB_H_
#define PRE_PI_UFS_LIB_H_

#include <Uefi.h>

// Call after the PrePi allocator and MMU are available.
EFI_STATUS PlatformPrepareInheritedUfs (VOID);
EFI_STATUS PlatformReinitializeUfs (VOID);
EFI_STATUS PlatformFinishUfs (VOID);

// Internal polling transport for a single serialized read-only PrePi session.
EFI_STATUS PrePiUfsTransportOpen (VOID);
EFI_STATUS PrePiUfsTransportClose (VOID);
EFI_STATUS PrePiUfsTransportDescriptor (IN UINT8 Idn, IN UINT8 Index, OUT UINT8 *Descriptor);
EFI_STATUS PrePiUfsTransportReadBlock (
  IN UINT8 Lun,
  IN UINT64 Lba,
  IN UINT32 BlockSize,
  OUT VOID *DmaBuffer
  );

#endif
