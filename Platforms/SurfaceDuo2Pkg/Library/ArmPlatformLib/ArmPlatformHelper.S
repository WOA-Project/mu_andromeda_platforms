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
GCC_ASM_IMPORT (ArmRelocateFirmware)
GCC_ASM_IMPORT (ContinueToLinuxIfAllowed)

.text
.align 3

ASM_FUNC(ArmPlatformPeiBootAction)
  /* First ensure we are at the correct location */
  bl ASM_PFX(ArmRelocateFirmware)

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
