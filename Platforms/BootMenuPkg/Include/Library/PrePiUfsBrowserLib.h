/** @file
  Read-only PrePi UFS partition and FAT file browser.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_UFS_BROWSER_LIB_H_
#define PRE_PI_UFS_BROWSER_LIB_H_

#include <Uefi.h>

// Returns EFI_SUCCESS and a UTF-16 path when a file is selected, EFI_ABORTED
// when the user goes back, or an error. PathChars includes the terminator.
EFI_STATUS
EFIAPI
PrePiUfsBrowse (
  OUT UINT8   *Lun,
  OUT UINT32  *Partition,
  IN OUT UINTN *PathChars,
  OUT CHAR16  *Path
  );

#endif
