/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Shell Code to patch kernel mode components before NTOSKRNL

  Copyright (c) 2022-2023 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _KERNEL_ERRATA_HANDLER_H_
#define _KERNEL_ERRATA_HANDLER_H_

#include <PiDxe.h>

#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Library/ArmLib.h>
#include <Library/ArmMmuLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PerformanceLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include <Protocol/MemoryAttribute.h>

VOID PreOslArm64TransferToKernel(VOID *OsLoaderBlock, VOID *KernelAddress);

#endif /* _KERNEL_ERRATA_HANDLER_H_ */