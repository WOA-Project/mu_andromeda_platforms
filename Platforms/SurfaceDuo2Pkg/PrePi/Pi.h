/** @file

  Copyright (c) 2011 - 2020, Arm Limited. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PREPI_H_
#define _PREPI_H_

#include <PiPei.h>

#include <Library/PcdLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/HobLib.h>
#include <Library/SerialPortLib.h>

VOID ContinueToLinuxIfAllowed(IN VOID *DeviceTreeLoadAddress, IN VOID *KernelLoadAddress);

VOID QGicCpuInit(VOID);
VOID QgicCpuInitSecondary(VOID);

UINTN EFIAPI ArmGicAcknowledgeInterrupt(
    IN UINTN GicInterruptInterfaceBase, OUT UINTN *InterruptId);
VOID EFIAPI ArmGicEnableInterruptInterface(IN INTN GicInterruptInterfaceBase);

VOID EFIAPI
ArmGicEndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source);

UINTN EFIAPI ArmGicGetMaxNumInterrupts(IN INTN GicDistributorBase);

BOOLEAN IsLinuxBootRequested(VOID);
VOID PlatformInitialize();

EFI_STATUS
EFIAPI
QGicPeim(VOID);

#endif /* _PREPI_H_ */