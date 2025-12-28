/** @file
    Copyright (c) 2024-2025. Project Aloha Authors. All rights reserved.
    Copyright (c) 2022-2025. DuoWoA Authors.
    SPDX-License-Identifier: MIT

    Portion of this code is written by referencing
    linux/drivers/iommu/arm/arm-smmu/arm-smmu.c
*/

#include <Uefi.h>

#include "MmuDetach.h"
#include "MmuDetachInternal.h"

#include <Library/BaseLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/TimerLib.h>

// ============================================================================
// Register Access Functions
// ============================================================================

UINT32 arm_smmu_gr0_read(ARM_SMMU_V2_DEVICE *smmuDevice, UINTN offset)
{
  return MmioRead32(
      smmuDevice->smmu_base_address + (ARM_SMMU_GR0 << smmuDevice->pgshift) +
      offset);
}

VOID arm_smmu_gr0_write(
    ARM_SMMU_V2_DEVICE *smmuDevice, UINTN offset, UINT32 value)
{
  MmioWrite32(
      smmuDevice->smmu_base_address + (ARM_SMMU_GR0 << smmuDevice->pgshift) +
          offset,
      value);
  MemoryFence();
}

UINT32 arm_smmu_gr1_read(ARM_SMMU_V2_DEVICE *smmuDevice, UINTN offset)
{
  return MmioRead32(
      smmuDevice->smmu_base_address + (ARM_SMMU_GR1 << smmuDevice->pgshift) +
      offset);
}

VOID arm_smmu_gr1_write(
    ARM_SMMU_V2_DEVICE *smmuDevice, UINTN offset, UINT32 value)
{
  MmioWrite32(
      smmuDevice->smmu_base_address + (ARM_SMMU_GR1 << smmuDevice->pgshift) +
          offset,
      value);
  MemoryFence();
}

UINT32
arm_smmu_cb_read(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 bank, UINTN offset)
{
  return MmioRead32(
      smmuDevice->smmu_base_address +
      (UINTN)(ARM_SMMU_CB(smmuDevice, bank) << smmuDevice->pgshift) + offset);
}

VOID arm_smmu_cb_write(
    ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 bank, UINTN offset, UINT32 value)
{
  MmioWrite32(
      smmuDevice->smmu_base_address +
          (UINTN)(ARM_SMMU_CB(smmuDevice, bank) << smmuDevice->pgshift) +
          offset,
      value);
  MemoryFence();
}

VOID arm_smmu_cb_writeq(
    ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 bank, UINTN offset, UINT64 value)
{
  arm_smmu_cb_write(smmuDevice, bank, offset, (UINT32)value);
  arm_smmu_cb_write(smmuDevice, bank, offset + 4, (UINT32)(value >> 32));
}

// ============================================================================
// TLB Invalidation
// ============================================================================

static VOID smmu_tlb_invalidate_all(ARM_SMMU_V2_DEVICE *smmuDevice)
{
  UINT32 status;
  UINTN  timeout;

  // Invalidate all non-secure, non-Hyp TLB entries
  arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_TLBIALLNSNH, 0);

  // Wait for TLB sync to complete
  timeout = 1000000;
  do {
    status = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_sTLBGSTATUS);
    if (!(status & sTLBGSTATUS_GSACTIVE)) {
      break;
    }
    MicroSecondDelay(1);
  } while (--timeout);
}

// ============================================================================
// Context Bank Management
// ============================================================================

static VOID disable_context_bank(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 cbndx)
{
  UINT32 sctlr;
  UINT32 fsr;
  UINTN  timeout;

  // Read current SCTLR
  sctlr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR);

  // Disable MMU
  sctlr &= ~SCTLR_M;
  arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR, sctlr);

  // Wait for MMU disable to complete
  timeout = 1000;
  while (timeout--) {
    sctlr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR);
    if (!(sctlr & SCTLR_M)) {
      break;
    }
    MicroSecondDelay(10);
  }

  // Clear page table base addresses
  arm_smmu_cb_writeq(smmuDevice, cbndx, ARM_SMMU_CB_TTBR0, 0);
  arm_smmu_cb_writeq(smmuDevice, cbndx, ARM_SMMU_CB_TTBR1, 0);

  // Clear TCR
  arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_TCR, 0);

  // Clear fault status
  fsr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_FSR);
  if (fsr) {
    arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_FSR, fsr);
  }
}

static VOID free_context_bank(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 cbndx)
{
  UINT32 cbar;
  UINT32 cbar_type;
  UINT32 cbar_vmid;

  // Disable the Context Bank
  disable_context_bank(smmuDevice, cbndx);

  // Mark the Context Bank as free to use
  cbar = arm_smmu_gr1_read(smmuDevice, ARM_SMMU_GR1_CBAR(cbndx));

  cbar_type = (cbar >> CBAR_TYPE_SHIFT) & CBAR_TYPE_MASK;
  cbar_vmid = (cbar >> CBAR_VMID_SHIFT) & CBAR_VMID_MASK;

  if (cbar_type != CBAR_TYPE_S2_TRANS &&
      cbar_type != CBAR_TYPE_S1_TRANS_S2_FAULT &&
      (cbar_type != CBAR_TYPE_S1_TRANS_S2_BYPASS || cbar_vmid != 0xFF)) {
    cbar =
        ((CBAR_TYPE_S1_TRANS_S2_BYPASS & CBAR_TYPE_MASK) << CBAR_TYPE_SHIFT) |
        ((0xFF & CBAR_VMID_MASK) << CBAR_VMID_SHIFT);
    arm_smmu_gr1_write(smmuDevice, ARM_SMMU_GR1_CBAR(cbndx), cbar);
  }
}

// ============================================================================
// Stream Management
// ============================================================================

static VOID smmu_v2_detach_all(ARM_SMMU_V2_DEVICE *smmuDevice)
{
  UINT32 smr;
  UINT32 s2cr;
  UINT32 cbndx;
  UINT32 s2cr_type;

  UINT32 smr_id;
  UINT32 smr_mask;
  UINT32 masked_smr_id;
  UINT32 ContextBanks[16]; // Track context banks used
  UINT32 NumContextBanks = 0;

  UINT32 id0     = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
  UINT32 numsmrg = (id0 >> 0) & 0xFF;

  for (UINT32 i = 0; i < numsmrg; i++) {
    smr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_SMR(i));

    if (smr & SMR_VALID) {
      // Read S2CR register
      s2cr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_S2CR(i));

      // Extract Context Bank index and type
      cbndx     = (s2cr >> S2CR_CBNDX_SHIFT) & S2CR_CBNDX_MASK;
      s2cr_type = (s2cr >> S2CR_TYPE_SHIFT) & S2CR_TYPE_MASK;

      // Extract StreamID and mask (16-bit fields)
      smr_id   = (smr >> SMR_ID_SHIFT) & SMR_ID_MASK;
      smr_mask = (smr >> SMR_MASK_SHIFT) & SMR_ID_MASK;

      masked_smr_id = smr_id & ~smr_mask;

      // Skip MDP related StreamIDs
      // Check if StreamID matches (compare low 16 bits masked)
      if ((0x0820 & ~smr_mask) == masked_smr_id ||
          (0x0821 & ~smr_mask) == masked_smr_id ||
          (0x0C21 & ~smr_mask) == masked_smr_id) {

        BOOLEAN AlreadyTracked = FALSE;

        for (UINT32 i = 0; i < NumContextBanks; i++) {
          if (ContextBanks[i] == cbndx) {
            AlreadyTracked = TRUE;
            break;
          }
        }

        if (!AlreadyTracked && NumContextBanks < 16) {
          ContextBanks[NumContextBanks++] = cbndx;
        }

        // Skip removing the SMR entry
        continue;
      }

      // Set S2CR to FAULT mode to block all accesses
      s2cr = ((S2CR_TYPE_FAULT & S2CR_TYPE_MASK) << S2CR_TYPE_SHIFT) |
             ((cbndx & S2CR_CBNDX_MASK) << S2CR_CBNDX_SHIFT);
      arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_S2CR(i), s2cr);

      // Invalidate the SMR entry
      smr &= ~SMR_VALID;
      arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_SMR(i), smr);
    }
  }

  // Invalidate TLB
  smmu_tlb_invalidate_all(smmuDevice);

  for (cbndx = 0; cbndx < smmuDevice->num_context_banks; cbndx++) {
    BOOLEAN AlreadyTracked = FALSE;

    for (UINT32 i = 0; i < NumContextBanks; i++) {
      if (ContextBanks[i] == cbndx) {
        AlreadyTracked = TRUE;
        break;
      }
    }

    // Check if the context bank is reserved by streams we deliberately keep
    if (AlreadyTracked) {
      continue;
    }

    free_context_bank(smmuDevice, cbndx);
  }
}

VOID MmuDetach()
{
  ARM_SMMU_V2_DEVICE smmuDevice = {0};

  ARM_MEMORY_REGION_DESCRIPTOR_EX SMMUMemoryRegion;
  LocateMemoryMapAreaByName("SMMU", &SMMUMemoryRegion);
  smmuDevice.smmu_base_address = SMMUMemoryRegion.Address;

  UINT32 id1         = arm_smmu_gr0_read(&smmuDevice, ARM_SMMU_GR0_ID1);
  smmuDevice.pgshift = (id1 & ARM_SMMU_ID1_PAGESIZE) ? 16 : 12;
  smmuDevice.numpage = 1 << ((READ_FIELD(ARM_SMMU_ID1_NUMPAGENDXB, id1)) + 1);
  smmuDevice.num_context_banks = id1 & ARM_SMMU_ID1_NUMCB_MASK;

  smmu_v2_detach_all(&smmuDevice);
}