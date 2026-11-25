/** @file
  SM8150 PrePi display-resume entry points.

  SPDX-License-Identifier: MIT
**/

#ifndef DISPLAY_RESUME_SCANOUT_H_
#define DISPLAY_RESUME_SCANOUT_H_

#include <Uefi.h>

// Requires the temporary display SMMU bypass before invocation; it must cover
// command DMA as well as framebuffer reads and remain active on a timeout.
EFI_STATUS
DisplayResumeDsiInit (
  IN BOOLEAN  Secondary
  );

EFI_STATUS
DisplayResumeDsiPanelInit (
  VOID
  );

EFI_STATUS
DisplayResumeDsiPanelShutdown (
  VOID
  );

EFI_STATUS
DisplayResumeDsiClose (
  VOID
  );

BOOLEAN
DisplayResumeDsiCommandPending (
  VOID
  );

#endif
