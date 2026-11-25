/** @file
  Platform callbacks at PrePi lifecycle boundaries.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_PLATFORM_HOOKS_LIB_H_
#define PRE_PI_PLATFORM_HOOKS_LIB_H_

#include <Uefi.h>

VOID EFIAPI PrePiExecuteEarlyDisplayDiagnostics (VOID);
VOID EFIAPI PrePiExecutePriorToCpuHob (VOID);
VOID EFIAPI PrePiShowBootMenu (VOID);
VOID EFIAPI PrePiExecutePriorToDxeCoreHandoff (VOID);

#endif
