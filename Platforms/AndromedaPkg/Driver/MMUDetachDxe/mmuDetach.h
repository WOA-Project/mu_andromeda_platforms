/** @file
    Copyright (c) 2024-2026. Project Aloha Authors. All rights reserved.
    SPDX-License-Identifier: MIT

    some codes reference linux/drivers/iommu/arm/arm-smmu/arm-smmu.c
*/
#pragma once

#include <Uefi.h>

#define GEN_MSK(h, l) (((1U << ((h) - (l) + 1)) - 1) << (l))
#define READ_FIELD(msk, val) (((val) & (msk)) >> (__builtin_ffsll(msk) - 1))

// Page selector constants (matching SMMU v2 layout)
#define ARM_SMMU_GR0 0
#define ARM_SMMU_GR1 1
#define ARM_SMMU_CB(s, n) ((s)->numpage + (n))
#define ARM_SMMU_ID1_NUMPAGENDXB GEN_MSK(30, 28)
// SMMU v2 Register offsets
#define ARM_SMMU_GR0_sCR0 0x0
#define ARM_SMMU_GR0_ID0 0x20
#define ARM_SMMU_GR0_ID1 0x24
#define ARM_SMMU_GR0_ID7 0x3c
#define ARM_SMMU_GR0_sGFSR 0x48
#define ARM_SMMU_GR0_sACR 0x10

// Stream Mapping Registers
#define ARM_SMMU_GR0_SMR(n) (0x800 + ((n) << 2))
#define ARM_SMMU_GR0_S2CR(n) (0xC00 + ((n) << 2))

// Context Bank registers
#define ARM_SMMU_CB_SCTLR 0x0
#define ARM_SMMU_CB_ACTLR 0x4
#define ARM_SMMU_CB_TTBR0 0x20
#define ARM_SMMU_CB_TTBR1 0x28
#define ARM_SMMU_CB_TCR 0x30
#define ARM_SMMU_CB_FSR 0x58
#define ARM_SMMU_CB_FAR 0x60
#define ARM_SMMU_CB_FSYNR0 0x68

// TLB maintenance
#define ARM_SMMU_GR0_TLBIALLNSNH 0x70
#define ARM_SMMU_GR0_sTLBGSTATUS 0x74
#define sTLBGSTATUS_GSACTIVE (1 << 0)

// Register bits
#define sCR0_CLIENTPD (1 << 0)
#define sCR0_GFRE (1 << 1)
#define sCR0_GFIE (1 << 2)

// SMR bits
#define SMR_VALID (1U << 31)
#define SMR_MASK_SHIFT 16
#define SMR_ID_SHIFT 0
#define SMR_ID_MASK 0xFFFF

// S2CR Type bits
#define S2CR_TYPE_SHIFT 16
#define S2CR_TYPE_TRANS (0 << S2CR_TYPE_SHIFT)
#define S2CR_TYPE_BYPASS (1 << S2CR_TYPE_SHIFT)
#define S2CR_TYPE_FAULT (2 << S2CR_TYPE_SHIFT)
#define S2CR_CBNDX_MASK 0xFF

// SCTLR bits
#define SCTLR_M (1 << 0) /* MMU enable */
#define SCTLR_TRE (1 << 1)
#define SCTLR_AFE (1 << 2)
#define SCTLR_CFRE (1 << 5)
#define SCTLR_CFIE (1 << 6)

// FSR bits
#define FSR_MULTI (1U << 31)
#define FSR_SS (1 << 30)

// ID register fields
#define ARM_SMMU_ID1_PAGESIZE (1 << 31)
#define ARM_SMMU_ID1_NUMPAGENDXB_SHIFT 28
#define ARM_SMMU_ID1_NUMPAGENDXB_MASK 0x7
#define ARM_SMMU_ID1_NUMCB_MASK 0xFF
#define ARM_SMMU_ID7_MAJOR GEN_MSK(7, 4)
#define ARM_SMMU_ID7_MINOR GEN_MSK(3, 0)

// MMU-500 specific
#define ARM_MMU500_ACTLR_CPRE (1 << 1)
#define ARM_MMU500_ACR_CACHE_LOCK (1 << 26)
#define ARM_MMU500_ACR_S2CRB_TLBEN (1 << 10)
#define ARM_MMU500_ACR_SMTNMB_TLBEN (1 << 8)

// ACPI IORT File GUID
#define ACPI_IORT_FILE_GUID                                                    \
  {0x7E374E25, 0x8E01, 0x4FEE, {0x87, 0xF2, 0x39, 0x0C, 0x23, 0xC6, 0x06, 0xCD}}

// IORT Signature (use standard definition)
#define EFI_ACPI_6_0_IO_REMAPPING_TABLE_SIGNATURE                              \
  SIGNATURE_32('I', 'O', 'R', 'T')

typedef struct {
  UINTN  smmu_base_address;
  UINTN  smmu_span_size;
  UINTN  pgshift;
  UINT32 num_context_banks;
  UINT32 numpage;
} ARM_SMMU_V2_DEVICE;

// Device information extracted from IORT
typedef struct {
  CHAR8   DeviceName[64];
  UINT32  InputBase;
  UINT32  NumIds;
  UINT32  OutputBase;
  UINT32  StreamId;
  BOOLEAN Found;
} IORT_DEVICE_INFO;

EFI_STATUS
smmu_v2_detach_device_by_name(
    ARM_SMMU_V2_DEVICE *smmuDevice, VOID *IortTable, CONST CHAR8 *DeviceName);

EFI_STATUS
smmu_v2_detach_device(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 streamid);