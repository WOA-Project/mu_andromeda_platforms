/** @file
  SM8150 command-mode DPU restoration.

  SPDX-License-Identifier: MIT
**/

#ifndef DISPLAY_RESUME_DPU_H_
#define DISPLAY_RESUME_DPU_H_

#include <Uefi.h>

EFI_STATUS
DisplayResumeDpuInit (
  VOID
  );

EFI_STATUS
DisplayResumeDpuStart (
  VOID
  );

EFI_STATUS
DisplayResumeDpuStartSecondary (
  VOID
  );

/**
  Stop active refresh and request VBIF C0/C7 halt, and C8 when the secondary
  was started, after the corresponding DSI, VIG and CTL blocks appear idle.
  Require their continued idle plus both halt ACK and idle bits.
  Each polling phase is bounded at 100,000 microseconds.
  The caller must serialize access and own or quiesce all affected clients.

  This is inferred from XBL HAL register names, not a documented XBL halt
  sequence or proven DMA drain. It is not hardware validated. EFI_SUCCESS
  means only that the requested indications were observed, not that safe
  unmapping is guaranteed. On failure retain mapping, memory, clocks and
  power; a timeout after the halt request leaves the halt asserted. Retry
  Stop to recheck. On success keep halt asserted until explicit Release.
**/
EFI_STATUS
DisplayResumeDpuStop (
  VOID
  );

/**
  Restore the original halt request bits after a successful Stop and after
  the caller completes unmapping. Before Release the caller must ensure no
  old or future DPU/client request can target unmapped memory; stopping TE
  does not detach VIG0 or prevent an external forced commit. Other halt bits
  are preserved. Start and Init remain disabled after stopping.
**/
EFI_STATUS
DisplayResumeDpuRelease (
  VOID
  );

#endif
