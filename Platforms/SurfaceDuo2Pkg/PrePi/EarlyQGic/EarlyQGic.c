/** @file
This files implements early GIC initialization.

Copyright (c) 2018, Bingxing Wang. All rights reserved.
Copyright (c) 2016, Brian McKenzie. All rights reserved.
Copyright (c) 2015-2016, Linaro Limited. All rights reserved.
Copyright (c) 2015-2016, Hisilicon Limited. All rights reserved.

This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD
License which accompanies this distribution.  The full text of the license may
be found at http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Pi.h"

#include <Guid/LzmaDecompress.h>
#include <Guid/VariableFormat.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

#include "EarlyQGic.h"

#define GIC_WAKER_PROCESSORSLEEP 2
#define GIC_WAKER_CHILDRENASLEEP 4

/* Intialize distributor */
VOID QGicDistConfig(UINT32 NumIrq)
{
  UINT32 i;

  /* Set each interrupt line to use N-N software model
   * and edge sensitive, active high
   */
  for (i = 32; i < NumIrq; i += 16) {
    MmioWrite32(GIC_DIST_CONFIG + i * 4 / 16, 0xffffffff);
  }

  MmioWrite32(GIC_DIST_CONFIG + 4, 0xffffffff);

  /* Set priority of all interrupts */

  /*
   * In bootloader we dont care about priority so
   * setting up equal priorities for all
   */
  for (i = 0; i < NumIrq; i += 4) {
    MmioWrite32(GIC_DIST_PRI + i * 4 / 4, 0xa0a0a0a0);
  }

  /* Disabling interrupts */
  for (i = 0; i < NumIrq; i += 32) {
    MmioWrite32(GIC_DIST_ENABLE_CLEAR + i * 4 / 32, 0xffffffff);
  }

  MmioWrite32(GIC_DIST_ENABLE_SET, 0x0000ffff);
}

VOID QGicHardwareReset(VOID)
{
  UINT32 n;

  /* Disabling GIC */
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 1\n"));
  MmioWrite32(GIC_DIST_CTRL, 0);
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 2\n"));
  MmioWrite32(GIC_DIST_CGCR, 0);
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 3\n"));
  ArmGicV3DisableInterruptInterface();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 4\n"));
  ArmGicV3SetPriorityMask(0);
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 5\n"));
  ArmGicV3SetBinaryPointer(0);

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 6\n"));
  for (n = 0; n <= 11; n++) {
    MmioWrite32(GIC_DIST_REG(0x80 + 4 * n), 0);
    MmioWrite32(GIC_DIST_REG(0x180 + 4 * n), 0xFFFFFFFF);
    MmioWrite32(GIC_DIST_REG(0x280 + 4 * n), 0xFFFFFFFF);
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 7\n"));
  for (n = 0; n <= 95; n++) {
    MmioWrite32(GIC_DIST_REG(0x400 + 4 * n), 0);
    MmioWrite32(GIC_DIST_REG(0x800 + 4 * n), 0);
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 8\n"));
  for (n = 0; n <= 23; n++) {
    MmioWrite32(GIC_DIST_REG(0xc00 + 4 * n), 0);
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset 9\n"));
}

VOID QGicSetBinpoint(VOID)
{
  /* No PREEMPT */
  ArmGicV3SetBinaryPointer(0x7);
}

VOID QGicDistInit(VOID)
{
  UINT32 i;
  UINT32 num_irq = 0;
  UINT32 affinity;

	/* Read the mpidr register to find out the boot up cluster */
  affinity = ArmReadMpidr();

	/* For aarch32 mode we have only 3 affinity values: aff0:aff1:aff2*/
  affinity = affinity & 0x00ffffff;

  /* Find out how many interrupts are supported. */
  num_irq = MmioRead32(GIC_DIST_CTR) & 0x1f;
  num_irq = (num_irq + 1) * 32;

	/* Do the qgic dist initialization */
  QGicDistConfig(num_irq);

	/* Write the affinity value, for routing all the SPIs */
  for (i = 32; i < num_irq; i++) {
    MmioWrite32(GICD_IROUTER + i * 8, affinity);
  }

	/* Enable affinity routing of grp0/grp1 interrupts */
	MmioWrite32(GIC_DIST_CTRL, ENABLE_GRP0_SEC | ENABLE_GRP1_NS | ENABLE_ARE);
}

/* Intialize cpu specific controller */
VOID QGicCpuInit(VOID)
{
	UINT32 retry = 1000;
	UINT32 sre = 0;
	UINT32 pmr = 0xff;
	//UINT32 eoimode = 0;

	/* For cpu init need to wake up the redistributor */
	MmioWrite32(GICR_WAKER_CPU0, (MmioRead32(GICR_WAKER_CPU0) & ~GIC_WAKER_PROCESSORSLEEP));

	/* Wait until redistributor is up */
	while (MmioRead32(GICR_WAKER_CPU0) & GIC_WAKER_CHILDRENASLEEP)
	{
		retry--;
		if (!retry)
		{
			DEBUG((EFI_D_ERROR, "Failed to wake redistributor for CPU0\n"));
			ASSERT(FALSE);
		}

    //MicroSecondDelay(1);
	}


	/* Make sure the system register access is enabled for us */
	sre = ArmGicV3GetControlSystemRegisterEnable();
	sre |= 1;
	ArmGicV3SetControlSystemRegisterEnable(sre);

	/* If system register access is not set, we fail */
	sre = ArmGicV3GetControlSystemRegisterEnable();
	if (!(sre & 1))
	{
		DEBUG((EFI_D_ERROR, "Failed to set SRE for NS world\n"));
		ASSERT(FALSE);
	}

	/* Set the priortiy mask register, interrupts with priority
	 * higher than this value will be signalled to processor.
	 * Lower value means higher priority.
	 */
	ArmGicV3SetPriorityMask(pmr);

	/* Make sure EOI is handled in NS EL3 */
	//__asm__ volatile("mrc p15, 0, %0, c12, c12, 4" : "=r" (eoimode));
	//eoimode &= ~BIT(1);
	//__asm__ volatile("mcr p15, 0, %0, c12, c12, 4" :: "r" (eoimode));
	//isb();

	/* Enable grp1 interrupts for NS EL3*/
	ArmGicV3EnableInterruptInterface();
}

EFI_STATUS
EFIAPI
QGicPeim(VOID)
{
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicPeim Start!\n"));
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicHardwareReset\n"));
  QGicHardwareReset();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicSetBinpoint\n"));
  QGicSetBinpoint();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicDistInit\n"));
  QGicDistInit();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicCpuInit\n"));
  QGicCpuInit();
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "QGicPeim Done!\n"));

  return EFI_SUCCESS;
}

UINTN
EFIAPI
ArmGicAcknowledgeInterrupt(
    IN UINTN GicInterruptInterfaceBase, OUT UINTN *InterruptId)
{
  UINTN Value;

  Value = ArmGicV3AcknowledgeInterrupt();
  if (InterruptId != NULL) {
    *InterruptId = Value & ARM_GIC_ICCIAR_ACKINTID;
  }

  return Value;
}

VOID EFIAPI ArmGicSendSgiTo(
    IN INTN GicDistributorBase, IN INTN TargetListFilter, IN INTN CPUTargetList,
    IN INTN SgiId)
{
  MmioWrite32(
      GicDistributorBase + ARM_GIC_ICDSGIR, ((TargetListFilter & 0x3) << 24) |
                                                ((CPUTargetList & 0xFF) << 16) |
                                                SgiId);
}

VOID EFIAPI ArmGicEnableInterruptInterface(IN INTN GicInterruptInterfaceBase)
{
  ArmGicV3EnableInterruptInterface();
}

UINTN
EFIAPI
ArmGicGetMaxNumInterrupts(IN INTN GicDistributorBase)
{
  return 32 * ((MmioRead32(GicDistributorBase + ARM_GIC_ICDICTR) & 0x1F) + 1);
}

VOID EFIAPI
     ArmGicEndOfInterrupt(IN UINTN GicInterruptInterfaceBase, IN UINTN Source)
{
  ArmGicV3EndOfInterrupt(Source);
}