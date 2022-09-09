//
//  Copyright (c) 2012-2013, ARM Limited. All rights reserved.
//
//  SPDX-License-Identifier: BSD-2-Clause-Patent
//
//

#include <AsmMacroIoLibV8.h>
#include <Library/ArmLib.h>
#include "Pi.h"

GCC_ASM_IMPORT (ArmDeInitialize)
GCC_ASM_IMPORT (ContinueToLinuxIfAllowed)

.text
.align 3

ASM_FUNC(ArmPlatformPeiBootAction)
  /* Check if we're located at expected location */
  adr	   x4, .
  ldr	   x5, =ArmPlatformPeiBootAction
  cmp	   x4, x5
  bne	   _CopyUEFI

  /* If we are, just return, we have nothing to do here */
  b      _ContinuePlatformBootAction

  _CopyUEFI:
  /* Find our start address by getting our expected offset, then subtracting it from our actual address */
  ldr	   x6, =FixedPcdGet64 (PcdFdBaseAddress)

  /* x5 holds offset of ArmPlatformPeiBootAction from start of FD base */
  sub	   x5, x5, x6

  /* x4 holds address of actual FD base */
  sub	   x4, x4, x5

  // tweak the return address
  // note: x30 is lr; gcc5 doesn't have the alias
  sub	   x30, x30, x4
  add	   x30, x30, x6
  ldr	   x5, =FixedPcdGet64 (PcdFdSize)

  /* Copy UEFI code into place */
  _CopyLoop:
  ldp	   x2, x3, [x4], #16
  stp	   x2, x3, [x6], #16
  subs   x5, x5, #16
  b.ne   _CopyLoop

_ContinuePlatformBootAction:
  /* Continue to linux if allowed */
  bl ASM_PFX(ContinueToLinuxIfAllowed)

  /* World reset */
  bl ASM_PFX(ArmDeInitialize)

  /* Clear x0 for function calls below */
  mov x0, #0
  mov x1, #0

  ret
.ltorg

//UINTN
//ArmPlatformGetCorePosition (
//  IN UINTN MpId
//  );
// With this function: CorePos = (ClusterId * 4) + CoreId
ASM_FUNC(ArmPlatformGetCorePosition)
  and   x1, x0, #ARM_CORE_MASK
  and   x0, x0, #ARM_CLUSTER_MASK
  add   x0, x1, x0, LSR #6
  ret

//UINTN
//ArmPlatformGetPrimaryCoreMpId (
//  VOID
//  );
ASM_FUNC(ArmPlatformGetPrimaryCoreMpId)
  MOV32  (w0, FixedPcdGet32 (PcdArmPrimaryCore))
  ret

//UINTN
//ArmPlatformIsPrimaryCore (
//  IN UINTN MpId
//  );
ASM_FUNC(ArmPlatformIsPrimaryCore)
  MOV32  (w1, FixedPcdGet32 (PcdArmPrimaryCoreMask))
  and   x0, x0, x1
  MOV32  (w1, FixedPcdGet32 (PcdArmPrimaryCore))
  cmp   w0, w1
  mov   x0, #1
  mov   x1, #0
  csel  x0, x0, x1, eq
  ret
