/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Based on https://github.com/SamuelTulach/rainbow

  Copyright (c) 2021 Samuel Tulach
  Copyright (c) 2022-2023 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _KERNEL_ERRATA_PATCHER_H_
#define _KERNEL_ERRATA_PATCHER_H_

#include <PiDxe.h>

#include <Library/UefiLib.h>
#include <Uefi.h>

#include <Library/ArmLib.h>
#include <Library/ArmMmuLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiBootServicesTableLib.h>

#include "ntdef.h"

#define SWAP_ENDIANNESS(x)                                                     \
  ((((x)&0xFF000000ull) >> 0x18) | (((x)&0xFF0000ull) >> 0x08) |               \
   (((x)&0xFF00ull) << 0x08) | (((x)&0xFFull) << 0x18))

#define NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL_FUNCTION_OFFSET 0x400

#define ARM64_INSTRUCTION_LENGTH 4
#define ARM64_TOTAL_INSTRUCTION_LENGTH(x) (ARM64_INSTRUCTION_LENGTH * x)
#define ARM64_BRANCH_LOCATION_INSTRUCTION(CurrentOffset, TargetOffset)         \
  SWAP_ENDIANNESS(                                                             \
      0x94000000ull |                                                          \
      (((TargetOffset - CurrentOffset) / ARM64_INSTRUCTION_LENGTH) &           \
       0x7FFFFFFull))

#define SCAN_MAX 0x300000

#define IN_RANGE(x, a, b) (x >= a && x <= b)
#define GET_BITS(x)                                                            \
  (IN_RANGE((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA)             \
                                     : (IN_RANGE(x, '0', '9') ? x - '0' : 0))
#define GET_BYTE(a, b) (GET_BITS(a) << 4 | GET_BITS(b))

EFI_STATUS
EFIAPI
KernelErrataPatcherExitBootServices(
    IN EFI_HANDLE ImageHandle, IN UINTN MapKey,
    IN EFI_PHYSICAL_ADDRESS fwpKernelSetupPhase1);

EFI_STATUS
EFIAPI
ExitBootServicesWrapper(IN EFI_HANDLE ImageHandle, IN UINTN MapKey);

UINT64 GetExport(EFI_PHYSICAL_ADDRESS base, const CHAR8 *functionName);
EFI_PHYSICAL_ADDRESS LocateWinloadBase(EFI_PHYSICAL_ADDRESS base);

VOID CopyMemory(
    EFI_PHYSICAL_ADDRESS destination, EFI_PHYSICAL_ADDRESS source, UINTN size);
UINT64 FindPattern(
    EFI_PHYSICAL_ADDRESS baseAddress, UINT64 size, const CHAR8 *pattern);

#endif /* _KERNEL_ERRATA_PATCHER_H_ */