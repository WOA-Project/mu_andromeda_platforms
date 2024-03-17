/** @file
*
*  Copyright (c) 2011-2023, Arm Limited. All rights reserved.
*
*  SPDX-License-Identifier: BSD-2-Clause-Patent
*
**/

#include <Base.h>
#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>

// In GICv3, there are 2 x 64KB frames:
// Redistributor control frame + SGI Control & Generation frame
#define GIC_V3_REDISTRIBUTOR_GRANULARITY  (ARM_GICR_CTLR_FRAME_SIZE           \
                                           + ARM_GICR_SGI_PPI_FRAME_SIZE)

// In GICv4, there are 2 additional 64KB frames:
// VLPI frame + Reserved page frame
#define GIC_V4_REDISTRIBUTOR_GRANULARITY  (GIC_V3_REDISTRIBUTOR_GRANULARITY   \
                                           + ARM_GICR_SGI_VLPI_FRAME_SIZE     \
                                           + ARM_GICR_SGI_RESERVED_FRAME_SIZE)

#define ISENABLER_ADDRESS(base, offset)  ((base) +\
          ARM_GICR_CTLR_FRAME_SIZE + ARM_GICR_ISENABLER + 4 * (offset))

#define ICENABLER_ADDRESS(base, offset)  ((base) +\
          ARM_GICR_CTLR_FRAME_SIZE + ARM_GICR_ICENABLER + 4 * (offset))

#define IPRIORITY_ADDRESS(base, offset)  ((base) +\
          ARM_GICR_CTLR_FRAME_SIZE + ARM_GIC_ICDIPR + 4 * (offset))

// MU_CHANGE Starts: Added pending interrupts definitions
#define ISPENDR_ADDRESS(base, offset)  ((base) +\
          ARM_GICR_CTLR_FRAME_SIZE + ARM_GICR_ISPENDR + 4 * (offset))

#define ICPENDR_ADDRESS(base, offset)  ((base) +\
          ARM_GICR_CTLR_FRAME_SIZE + ARM_GICR_ICPENDR + 4 * (offset))
// MU_CHANGE Ends

/**
 *
 * Return whether the Source interrupt index refers to a shared interrupt (SPI)
 */
STATIC
BOOLEAN
SourceIsSpi (
  IN UINTN  Source
  )
{
  return Source >= 32 && Source < 1020;
}

/**
 * Return the base address of the GIC redistributor for the current CPU
 *
 * @param Revision  GIC Revision. The GIC redistributor might have a different
 *                  granularity following the GIC revision.
 *
 * @retval Base address of the associated GIC Redistributor
 */
STATIC
UINTN
GicGetCpuRedistributorBase (
  IN UINTN                  GicRedistributorBase,
  IN ARM_GIC_ARCH_REVISION  Revision
  )
{
  UINTN   MpId;
  UINTN   CpuAffinity;
  UINTN   Affinity;
  UINTN   GicCpuRedistributorBase;
  UINT64  TypeRegister;

  MpId = ArmReadMpidr ();
  // Define CPU affinity as:
  // Affinity0[0:8], Affinity1[9:15], Affinity2[16:23], Affinity3[24:32]
  // whereas Affinity3 is defined at [32:39] in MPIDR
  CpuAffinity = (MpId & (ARM_CORE_AFF0 | ARM_CORE_AFF1 | ARM_CORE_AFF2)) |
                ((MpId & ARM_CORE_AFF3) >> 8);

  if (Revision < ARM_GIC_ARCH_REVISION_3) {
    ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
    return 0;
  }

  GicCpuRedistributorBase = GicRedistributorBase;

  do {
    TypeRegister = MmioRead64 (GicCpuRedistributorBase + ARM_GICR_TYPER);
    Affinity     = ARM_GICR_TYPER_GET_AFFINITY (TypeRegister);
    if (Affinity == CpuAffinity) {
      return GicCpuRedistributorBase;
    }

    // Move to the next GIC Redistributor frame.
    // The GIC specification does not forbid a mixture of redistributors
    // with or without support for virtual LPIs, so we test Virtual LPIs
    // Support (VLPIS) bit for each frame to decide the granularity.
    // Note: The assumption here is that the redistributors are adjacent
    // for all CPUs. However this may not be the case for NUMA systems.
    GicCpuRedistributorBase += (((ARM_GICR_TYPER_VLPIS & TypeRegister) != 0)
                                ? GIC_V4_REDISTRIBUTOR_GRANULARITY
                                : GIC_V3_REDISTRIBUTOR_GRANULARITY);
  } while ((TypeRegister & ARM_GICR_TYPER_LAST) == 0);

  // The Redistributor has not been found for the current CPU
  ASSERT_EFI_ERROR (EFI_NOT_FOUND);
  return 0;
}

/**
  Return the GIC CPU Interrupt Interface ID.

  @param GicInterruptInterfaceBase  Base address of the GIC Interrupt Interface.

  @retval CPU Interface Identification information.
**/
UINT32
EFIAPI
ArmGicGetInterfaceIdentification (
  IN  UINTN  GicInterruptInterfaceBase
  )
{
  // Read the GIC Identification Register
  return MmioRead32 (GicInterruptInterfaceBase + ARM_GIC_ICCIIDR);
}

UINTN
EFIAPI
ArmGicGetMaxNumInterrupts (
  IN  UINTN  GicDistributorBase
  )
{
  UINTN  ItLines;

  ItLines = MmioRead32 (GicDistributorBase + ARM_GIC_ICDICTR) & 0x1F;

  //
  // Interrupt ID 1020-1023 are reserved.
  //
  return (ItLines == 0x1f) ? 1020 : 32 * (ItLines + 1);
}

VOID
EFIAPI
ArmGicSendSgiTo (
  IN  UINTN  GicDistributorBase,
  IN  UINT8  TargetListFilter,
  IN  UINT8  CPUTargetList,
  IN  UINT8  SgiId
  )
{
  // MU_CHANGE Starts: Added check for supported GIC version and expanded support for sending SGI on GICv3 and GICv4.
  ARM_GIC_ARCH_REVISION  Revision;
  UINT32                 ApplicableTargets;
  UINT32                 AFF3;
  UINT32                 AFF2;
  UINT32                 AFF1;
  UINT32                 AFF0;
  UINT32                 Irm;
  UINT64                 SGIValue;

  Revision = ArmGicGetSupportedArchRevision ();
  if (Revision == ARM_GIC_ARCH_REVISION_2) {
    MmioWrite32 (
      GicDistributorBase + ARM_GIC_ICDSGIR,
      (UINT32)(((TargetListFilter & 0x3) << 24) | ((CPUTargetList & 0xFF) << 16) | SgiId)
      );      // MS_CHANGE
  } else {
    // Below routine is adopted from gicv3_raise_secure_g0_sgi in TF-A

    /* Extract affinity fields from target */
    AFF0 = GET_MPIDR_AFF0 (CPUTargetList);
    AFF1 = GET_MPIDR_AFF1 (CPUTargetList);
    AFF2 = GET_MPIDR_AFF2 (CPUTargetList);
    AFF3 = GET_MPIDR_AFF3 (CPUTargetList);

    /*
     * Make target list from affinity 0, and ensure GICv3 SGI can target
     * this PE.
     */
    ApplicableTargets = (1 << AFF0);

    /*
     * Evaluate the filter to see if this is for the target or all others
     */
    Irm = (TargetListFilter == ARM_GIC_ICDSGIR_FILTER_EVERYONEELSE) ? SGIR_IRM_TO_OTHERS : SGIR_IRM_TO_AFF;

    /* Raise SGI to PE specified by its affinity */
    SGIValue = GICV3_SGIR_VALUE (AFF3, AFF2, AFF1, SgiId, Irm, ApplicableTargets);

    /*
     * Ensure that any shared variable updates depending on out of band
     * interrupt trigger are observed before raising SGI.
     */
    ArmGicV3SendNsG1Sgi (SGIValue);
  }

  // MU_CHANGE Ends
}

/*
 * Acknowledge and return the value of the Interrupt Acknowledge Register
 *
 * InterruptId is returned separately from the register value because in
 * the GICv2 the register value contains the CpuId and InterruptId while
 * in the GICv3 the register value is only the InterruptId.
 *
 * @param GicInterruptInterfaceBase   Base Address of the GIC CPU Interface
 * @param InterruptId                 InterruptId read from the Interrupt
 *                                    Acknowledge Register
 *
 * @retval value returned by the Interrupt Acknowledge Register
 *
 */
UINTN
EFIAPI
ArmGicAcknowledgeInterrupt (
  IN  UINTN  GicInterruptInterfaceBase,
  OUT UINTN  *InterruptId
  )
{
  UINTN                  Value;
  UINTN                  IntId;
  ARM_GIC_ARCH_REVISION  Revision;

  ASSERT (InterruptId != NULL);
  Revision = ArmGicGetSupportedArchRevision ();
  if (Revision == ARM_GIC_ARCH_REVISION_2) {
    Value = ArmGicV2AcknowledgeInterrupt (GicInterruptInterfaceBase);
    IntId = Value & ARM_GIC_ICCIAR_ACKINTID;
  } else if (Revision == ARM_GIC_ARCH_REVISION_3) {
    Value = ArmGicV3AcknowledgeInterrupt ();
    IntId = Value;
  } else {
    ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
    // Report Spurious interrupt which is what the above controllers would
    // return if no interrupt was available
    Value = 1023;
  }

  if (InterruptId != NULL) {
    // InterruptId is required for the caller to know if a valid or spurious
    // interrupt has been read
    *InterruptId = IntId;
  }

  return Value;
}

VOID
EFIAPI
ArmGicEndOfInterrupt (
  IN  UINTN  GicInterruptInterfaceBase,
  IN UINTN   Source
  )
{
  ARM_GIC_ARCH_REVISION  Revision;

  Revision = ArmGicGetSupportedArchRevision ();
  if (Revision == ARM_GIC_ARCH_REVISION_2) {
    ArmGicV2EndOfInterrupt (GicInterruptInterfaceBase, Source);
  } else if (Revision == ARM_GIC_ARCH_REVISION_3) {
    ArmGicV3EndOfInterrupt (Source);
  } else {
    ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
  }
}

VOID
EFIAPI
ArmGicSetInterruptPriority (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source,
  IN UINTN  Priority
  )
{
  UINTN                  RegOffset;   // MU_CHANGE TCBZ3399
  UINT8                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;

  // MU_CHANGE [BEGIN] - TCBZ3399
  ASSERT (Priority <= MAX_UINT32);
  if (Priority > MAX_UINT32) {
    ASSERT_EFI_ERROR (EFI_INVALID_PARAMETER);
    return;
  }

  // MU_CHANGE [END] - TCBZ3399

  // Calculate register offset and bit position
  RegOffset = (Source / 4);
  RegShift  = (UINT8)((Source % 4) * 8);

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    MmioAndThenOr32 (
      GicDistributorBase + ARM_GIC_ICDIPR + (4 * RegOffset),
      ~(0xff << RegShift),
      (UINT32)Priority << RegShift    // MU_CHANGE TCBZ3399
      );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      return;
    }

    MmioAndThenOr32 (
      IPRIORITY_ADDRESS (GicCpuRedistributorBase, RegOffset),
      ~(0xff << RegShift),
      (UINT32)Priority << RegShift    // MU_CHANGE TCBZ3399
      );
  }
}

VOID
EFIAPI
ArmGicEnableInterrupt (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINT8                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);
  RegShift  = (UINT8)(Source % 32);

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    // Write set-enable register
    MmioWrite32 (
      GicDistributorBase + ARM_GIC_ICDISER + (4 * RegOffset),
      1 << RegShift
      );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      ASSERT_EFI_ERROR (EFI_NOT_FOUND);
      return;
    }

    // Write set-enable register
    MmioWrite32 (
      ISENABLER_ADDRESS (GicCpuRedistributorBase, RegOffset),
      1 << RegShift
      );
  }
}

VOID
EFIAPI
ArmGicDisableInterrupt (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINT8                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);
  RegShift  = (UINT8)(Source % 32);

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    // Write clear-enable register
    MmioWrite32 (
      GicDistributorBase + ARM_GIC_ICDICER + (4 * RegOffset),
      1 << RegShift
      );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      return;
    }

    // Write clear-enable register
    MmioWrite32 (
      ICENABLER_ADDRESS (GicCpuRedistributorBase, RegOffset),
      1 << RegShift
      );
  }
}

BOOLEAN
EFIAPI
ArmGicIsInterruptEnabled (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINT8                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;
  UINT32                 Interrupts;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);
  RegShift  = (UINT8)(Source % 32);

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    Interrupts = MmioRead32 (
                   GicDistributorBase + ARM_GIC_ICDISER + (4 * RegOffset)
                   );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      return 0;
    }

    // Read set-enable register
    Interrupts = MmioRead32 (
                   ISENABLER_ADDRESS (GicCpuRedistributorBase, RegOffset)
                   );
  }

  return ((Interrupts & (1 << RegShift)) != 0);
}

// MU_CHANGE Starts: Added new interfaces to support pending interrupt manipulation
VOID
EFIAPI
ArmGicSetPendingInterrupt (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINTN                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);    // MS_CHANGE
  RegShift  = Source % 32;

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    // Write set-pending register
    MmioWrite32 (
      GicDistributorBase + ARM_GIC_ICDSPR + (4 * RegOffset),
      1 << RegShift
      );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      ASSERT_EFI_ERROR (EFI_NOT_FOUND);
      return;
    }

    // Write set-enable register
    MmioWrite32 (
      ISPENDR_ADDRESS (GicCpuRedistributorBase, RegOffset),
      1 << RegShift
      );
  }
}

VOID
EFIAPI
ArmGicClearPendingInterrupt (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINTN                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);   // MS_CHANGE
  RegShift  = Source % 32;

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    // Write clear-enable register
    MmioWrite32 (
      GicDistributorBase + ARM_GIC_ICDICPR + (4 * RegOffset),
      1 << RegShift
      );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      return;
    }

    // Write clear-enable register
    MmioWrite32 (
      ICPENDR_ADDRESS (GicCpuRedistributorBase, RegOffset),
      1 << RegShift
      );
  }
}

BOOLEAN
EFIAPI
ArmGicIsInterruptPending (
  IN UINTN  GicDistributorBase,
  IN UINTN  GicRedistributorBase,
  IN UINTN  Source
  )
{
  UINT32                 RegOffset;
  UINTN                  RegShift;
  ARM_GIC_ARCH_REVISION  Revision;
  UINTN                  GicCpuRedistributorBase;
  UINT32                 Interrupts;

  // Calculate enable register offset and bit position
  RegOffset = (UINT32)(Source / 32);  // MS_CHANGE
  RegShift  = Source % 32;

  Revision = ArmGicGetSupportedArchRevision ();
  if ((Revision == ARM_GIC_ARCH_REVISION_2) ||
      FeaturePcdGet (PcdArmGicV3WithV2Legacy) ||
      SourceIsSpi (Source))
  {
    Interrupts = MmioRead32 (
                   GicDistributorBase + ARM_GIC_ICDSPR + (4 * RegOffset)
                   );
  } else {
    GicCpuRedistributorBase = GicGetCpuRedistributorBase (
                                GicRedistributorBase,
                                Revision
                                );
    if (GicCpuRedistributorBase == 0) {
      return 0;
    }

    // Read set-enable register
    Interrupts = MmioRead32 (
                   ISPENDR_ADDRESS (GicCpuRedistributorBase, RegOffset)
                   );
  }

  return ((Interrupts & (1 << RegShift)) != 0);
}

// MU_CHANGE Ends

VOID
EFIAPI
ArmGicDisableDistributor (
  IN  UINTN  GicDistributorBase
  )
{
  // Disable Gic Distributor
  MmioWrite32 (GicDistributorBase + ARM_GIC_ICDDCR, 0x0);
}

VOID
EFIAPI
ArmGicEnableInterruptInterface (
  IN  UINTN  GicInterruptInterfaceBase
  )
{
  ARM_GIC_ARCH_REVISION  Revision;

  Revision = ArmGicGetSupportedArchRevision ();
  if (Revision == ARM_GIC_ARCH_REVISION_2) {
    ArmGicV2EnableInterruptInterface (GicInterruptInterfaceBase);
  } else if (Revision == ARM_GIC_ARCH_REVISION_3) {
    ArmGicV3EnableInterruptInterface ();
  } else {
    ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
  }
}

VOID
EFIAPI
ArmGicDisableInterruptInterface (
  IN  UINTN  GicInterruptInterfaceBase
  )
{
  ARM_GIC_ARCH_REVISION  Revision;

  Revision = ArmGicGetSupportedArchRevision ();
  if (Revision == ARM_GIC_ARCH_REVISION_2) {
    ArmGicV2DisableInterruptInterface (GicInterruptInterfaceBase);
  } else if (Revision == ARM_GIC_ARCH_REVISION_3) {
    ArmGicV3DisableInterruptInterface ();
  } else {
    ASSERT_EFI_ERROR (EFI_UNSUPPORTED);
  }
}
