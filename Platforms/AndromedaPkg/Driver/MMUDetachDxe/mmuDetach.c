/** @file
    Copyright (c) 2024-2026. Project Aloha Authors. All rights reserved.
    SPDX-License-Identifier: MIT
*/

// UEFI includes
#include "mmuDetach.h"
#include <Guid/Acpi.h>
#include <IndustryStandard/Acpi.h>
#include <IndustryStandard/IoRemappingTable.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/DxeServicesLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/FirmwareVolume2.h>
#include <Uefi.h>

// ============================================================================
// Firmware Volume and IORT Functions
// ============================================================================

/**
  Read IORT.aml from firmware volume by GUID and scanning RAW sections.

  @param[out] IortTable   Pointer to receive allocated IORT table buffer
  @param[out] TableSize   Size of the IORT table

  @retval EFI_SUCCESS     IORT table loaded successfully
  @retval other           Error occurred
**/
static EFI_STATUS
LoadIortFromFirmwareVolume(OUT VOID **IortTable, OUT UINTN *TableSize)
{
  EFI_STATUS                       Status;
  EFI_GUID                         FileGuid = ACPI_IORT_FILE_GUID;
  UINTN                            NumHandles;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                            FvIndex;
  UINT32                           AuthenticationStatus;
  VOID                            *Buffer;
  UINTN                            Size;
  EFI_FIRMWARE_VOLUME2_PROTOCOL   *FvProtocol;
  EFI_ACPI_6_0_IO_REMAPPING_TABLE *IortPtr;
  UINTN                            SectionIndex;
  BOOLEAN                          Found = FALSE;

  DEBUG((EFI_D_WARN, "Searching for IORT.aml in firmware volumes...\n"));

  // Locate all Firmware Volume protocols
  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiFirmwareVolume2ProtocolGuid, NULL, &NumHandles,
      &HandleBuffer);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_WARN, "Failed to locate firmware volumes: %r\n", Status));
    return Status;
  }

  DEBUG((EFI_D_WARN, "Found %u firmware volume(s)\n", NumHandles));

  // Search through all firmware volumes
  for (FvIndex = 0; FvIndex < NumHandles && !Found; FvIndex++) {
    Status = gBS->HandleProtocol(
        HandleBuffer[FvIndex], &gEfiFirmwareVolume2ProtocolGuid,
        (VOID **)&FvProtocol);
    if (EFI_ERROR(Status)) {
      continue;
    }

    DEBUG((EFI_D_WARN, "Scanning FV[%u] for ACPI tables...\n", FvIndex));

    // Try to read RAW sections from the FREEFORM file with our GUID
    // Iterate through all possible section indices
    for (SectionIndex = 0; SectionIndex < 20 && !Found; SectionIndex++) {
      Buffer = NULL;
      Size   = 0;

      Status = FvProtocol->ReadSection(
          FvProtocol, &FileGuid, EFI_SECTION_RAW, SectionIndex, &Buffer, &Size,
          &AuthenticationStatus);

      if (EFI_ERROR(Status)) {
        // No more sections in this FV
        break;
      }

      if (Size > 0 && Size < 0x200000) {
        DEBUG(
            (EFI_D_WARN, "  Section[%u]: Size=%u bytes\n", SectionIndex, Size));

        // Check if this section contains IORT signature
        if (Size >= sizeof(EFI_ACPI_6_0_IO_REMAPPING_TABLE)) {
          IortPtr = (EFI_ACPI_6_0_IO_REMAPPING_TABLE *)Buffer;

          // Check for IORT signature
          if (IortPtr->Header.Signature == SIGNATURE_32('I', 'O', 'R', 'T')) {
            // Validate IORT table structure
            if (IortPtr->Header.Length <= Size &&
                IortPtr->Header.Length >=
                    sizeof(EFI_ACPI_6_0_IO_REMAPPING_TABLE) &&
                IortPtr->NodeOffset < IortPtr->Header.Length &&
                IortPtr->NumNodes > 0) {

              DEBUG(
                  (EFI_D_WARN, "Found valid IORT in FV[%u] Section[%u]\n",
                   FvIndex, SectionIndex));
              DEBUG(
                  (EFI_D_WARN, "  IORT Table Length: 0x%x\n",
                   IortPtr->Header.Length));
              DEBUG(
                  (EFI_D_WARN, "  Node Offset: 0x%x, Num Nodes: %u\n",
                   IortPtr->NodeOffset, IortPtr->NumNodes));

              // Allocate and copy the IORT table
              *TableSize = IortPtr->Header.Length;
              *IortTable = AllocatePool(*TableSize);
              if (*IortTable == NULL) {
                DEBUG((EFI_D_WARN, "Failed to allocate memory for IORT\n"));
                if (Buffer != NULL) {
                  FreePool(Buffer);
                }
                FreePool(HandleBuffer);
                return EFI_OUT_OF_RESOURCES;
              }

              CopyMem(*IortTable, IortPtr, *TableSize);
              Found = TRUE;
            }
          }
        }
      }

      if (Buffer != NULL) {
        FreePool(Buffer);
      }
    }
  }

  FreePool(HandleBuffer);

  if (!Found) {
    DEBUG((EFI_D_WARN, "IORT.aml not found in any firmware volume\n"));
    return EFI_NOT_FOUND;
  }

  DEBUG((
      EFI_D_WARN, "IORT table loaded successfully (%u bytes)\n\n", *TableSize));
  return EFI_SUCCESS;
}

/**
  Get SMMU base address from IORT table.

  @param[in]  IortTable     Pointer to IORT table
  @param[out] SmmuBaseAddr  Pointer to receive SMMU base address

  @retval EFI_SUCCESS       SMMU base address found
  @retval EFI_NOT_FOUND     SMMU node not found
**/
static EFI_STATUS
GetSmmuInfoFromIort(IN VOID *IortTable, OUT ARM_SMMU_V2_DEVICE *SmmuDevice)
{
  EFI_ACPI_6_0_IO_REMAPPING_TABLE     *Iort;
  EFI_ACPI_6_0_IO_REMAPPING_NODE      *Node;
  EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE *SmmuNode;
  UINT8                               *TableBase;
  UINT8                               *NodeBase;
  UINT8                               *TableEnd;
  UINT32                               NodeIndex;
  UINTN                                RemainingSize;

  if (IortTable == NULL || SmmuDevice == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Iort      = (EFI_ACPI_6_0_IO_REMAPPING_TABLE *)IortTable;
  TableBase = (UINT8 *)IortTable;
  TableEnd  = TableBase + Iort->Header.Length;

  DEBUG((EFI_D_WARN, "Searching for SMMU node in IORT...\n"));
  DEBUG(
      (EFI_D_WARN, "  Table Length: 0x%x, Node Offset: 0x%x, Num Nodes: %u\n",
       Iort->Header.Length, Iort->NodeOffset, Iort->NumNodes));

  // Validate NodeOffset
  if (Iort->NodeOffset >= Iort->Header.Length) {
    DEBUG(
        (EFI_D_WARN, "Invalid NodeOffset 0x%x (exceeds table length 0x%x)\n",
         Iort->NodeOffset, Iort->Header.Length));
    return EFI_INVALID_PARAMETER;
  }

  // Initialize NodeBase to first node
  NodeBase = TableBase + Iort->NodeOffset;

  // Iterate through all nodes to find SMMUv1v2 node
  for (NodeIndex = 0; NodeIndex < Iort->NumNodes; NodeIndex++) {
    // Check if NodeBase is within valid range
    if (NodeBase >= TableEnd) {
      DEBUG(
          (EFI_D_WARN, "Node pointer exceeded table boundary at index %u\n",
           NodeIndex));
      break;
    }

    RemainingSize = (UINTN)(TableEnd - NodeBase);
    if (RemainingSize < sizeof(EFI_ACPI_6_0_IO_REMAPPING_NODE)) {
      DEBUG(
          (EFI_D_WARN,
           "Insufficient space for node at index %u (remaining: %u)\n",
           NodeIndex, (UINT32)RemainingSize));
      break;
    }

    Node = (EFI_ACPI_6_0_IO_REMAPPING_NODE *)NodeBase;

    DEBUG(
        (EFI_D_WARN, "  Node[%u]: Type=%u, Length=%u\n", NodeIndex, Node->Type,
         Node->Length));

    // Validate node length
    if (Node->Length < sizeof(EFI_ACPI_6_0_IO_REMAPPING_NODE)) {
      DEBUG(
          (EFI_D_WARN, "Invalid node length %u at index %u\n", Node->Length,
           NodeIndex));
      break;
    }

    // Look for SMMUv1v2 node
    if (Node->Type == EFI_ACPI_IORT_TYPE_SMMUv1v2) {
      // Validate size for SMMU node
      if (Node->Length < sizeof(EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE)) {
        DEBUG((
            EFI_D_WARN, "SMMU node too small: %u (expected %u)\n", Node->Length,
            (UINT32)sizeof(EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE)));
        break;
      }

      SmmuNode = (EFI_ACPI_6_0_IO_REMAPPING_SMMU_NODE *)NodeBase;
      SmmuDevice->smmu_base_address = SmmuNode->Base;
      SmmuDevice->smmu_span_size    = SmmuNode->Span;
      DEBUG((EFI_D_WARN, "Found SMMU in IORT:\n"));
      DEBUG((EFI_D_WARN, "  Base Address: 0x%lx\n", SmmuNode->Base));
      DEBUG((EFI_D_WARN, "  Span: 0x%lx\n", SmmuNode->Span));
      DEBUG((EFI_D_WARN, "  Model: %u\n", SmmuNode->Model));

      return EFI_SUCCESS;
    }

    // Move to next node
    NodeBase = NodeBase + Node->Length;
  }

  DEBUG((EFI_D_WARN, "SMMU node not found in IORT table\n"));
  return EFI_NOT_FOUND;
}

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

UINT64
arm_smmu_cb_readq(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 bank, UINTN offset)
{
  UINT64 low  = arm_smmu_cb_read(smmuDevice, bank, offset);
  UINT64 high = arm_smmu_cb_read(smmuDevice, bank, offset + 4);
  return (high << 32) | low;
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

  DEBUG((EFI_D_WARN, "Invalidating TLB.. .\n"));

  // Invalidate all non-secure, non-Hyp TLB entries
  arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_TLBIALLNSNH, 0);
  MemoryFence();

  // Wait for TLB sync to complete
  timeout = 1000000;
  do {
    status = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_sTLBGSTATUS);
    if (!(status & sTLBGSTATUS_GSACTIVE)) {
      break;
    }
    MicroSecondDelay(1);
  } while (--timeout);

  if (timeout == 0) {
    DEBUG((EFI_D_WARN, "Warning: TLB sync timeout\n"));
  }
  else {
    DEBUG((EFI_D_WARN, "TLB invalidated successfully\n"));
  }
}

// ============================================================================
// Context Bank Management
// ============================================================================

static VOID disable_context_bank(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 cbndx)
{
  UINT32 sctlr;
  UINT32 fsr;
  UINTN  timeout;

  DEBUG((EFI_D_WARN, "Disabling Context Bank %u\n", cbndx));

  // Read current SCTLR
  sctlr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR);
  DEBUG((EFI_D_WARN, "  Current SCTLR: 0x%08x\n", sctlr));

  // Disable MMU
  sctlr &= ~SCTLR_M;
  arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR, sctlr);
  MemoryFence();

  // Wait for MMU disable to complete
  timeout = 1000;
  while (timeout--) {
    sctlr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_SCTLR);
    if (!(sctlr & SCTLR_M)) {
      break;
    }
    MicroSecondDelay(10);
  }

  if (sctlr & SCTLR_M) {
    DEBUG((EFI_D_WARN, "  Warning: MMU disable timeout\n"));
  }

  // Clear page table base addresses
  arm_smmu_cb_writeq(smmuDevice, cbndx, ARM_SMMU_CB_TTBR0, 0);
  arm_smmu_cb_writeq(smmuDevice, cbndx, ARM_SMMU_CB_TTBR1, 0);
  MemoryFence();

  // Clear TCR
  arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_TCR, 0);

  // Clear fault status
  fsr = arm_smmu_cb_read(smmuDevice, cbndx, ARM_SMMU_CB_FSR);
  if (fsr) {
    DEBUG((EFI_D_WARN, "  Clearing FSR: 0x%08x\n", fsr));
    arm_smmu_cb_write(smmuDevice, cbndx, ARM_SMMU_CB_FSR, fsr);
  }

  MemoryFence();
  DEBUG((EFI_D_WARN, "  Context Bank %u disabled\n", cbndx));
}

// ============================================================================
// Stream ID Management
// ============================================================================

static INTN
find_smr_for_streamid(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 streamid)
{
  UINT32 smr;
  UINT32 smr_id;
  UINT32 smr_mask;
  UINT32 numsmrg;
  UINT32 id0;
  UINT32 sid16 = streamid & 0xFFFF;

  // Read ID0 register to get number of SMRs
  id0     = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
  numsmrg = (id0 >> 0) & 0xFF;

  DEBUG((EFI_D_WARN, "SMMU has %u stream mapping groups\n", numsmrg));
  DEBUG(
      (EFI_D_WARN, "Searching for streamid 0x%04x (low16=0x%04x)\n", streamid,
       sid16));

  // Search through all SMRs
  for (UINT32 i = 0; i < numsmrg; i++) {
    smr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_SMR(i));

    // Check if SMR is valid
    if (!(smr & SMR_VALID)) {
      continue;
    }

    // Extract StreamID and mask (16-bit fields)
    smr_id   = (smr >> SMR_ID_SHIFT) & SMR_ID_MASK;
    smr_mask = (smr >> SMR_MASK_SHIFT) & SMR_ID_MASK;

    DEBUG(
        (EFI_D_WARN, "SMR[%2u]: raw=0x%08x ID=0x%04x MASK=0x%04x\n", i, smr,
         smr_id, smr_mask));

    // Check if StreamID matches (compare low 16 bits masked)
    if ((sid16 & ~smr_mask) == (smr_id & ~smr_mask)) {
      DEBUG(
          (EFI_D_WARN,
           "Matched StreamID 0x%04x (low16) with SMR[%u] (ID=0x%04x, "
           "MASK=0x%04x)\n",
           sid16, i, smr_id, smr_mask));
      return i;
    }
  }

  DEBUG(
      (EFI_D_WARN, "StreamID 0x%x (low16=0x%04x) not found in any SMR\n",
       streamid, sid16));
  return -1;
}

static VOID dump_smmu_mappings(ARM_SMMU_V2_DEVICE *smmuDevice)
{
  UINT32 numsmrg;
  UINT32 id0;
  UINT32 smr;
  UINT32 s2cr;
  UINT32 s2cr_type;
  UINT32 valid_count = 0;
  UINT32 invalid_count = 0;

  id0     = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
  numsmrg = (id0 >> 0) & 0xFF;

  DEBUG((EFI_D_WARN, "\n=== SMMU Stream Mappings ===\n numsmrg: %u\n", numsmrg));

  // First pass: display valid SMRs
  for (UINT32 i = 0; i < numsmrg; i++) {
    smr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_SMR(i));

    if (smr & SMR_VALID) {
      valid_count++;
      s2cr      = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_S2CR(i));
      s2cr_type = (s2cr >> S2CR_TYPE_SHIFT) & 0x3;
      DEBUG(
          (EFI_D_WARN,
           "SMR[%2u]: 0x%08x (ID=0x%04x, MASK=0x%04x) -> S2CR:  0x%08x "
           "(Type=%a, CBNDX=%u)\n",
           i, smr, (smr >> SMR_ID_SHIFT) & SMR_ID_MASK,
           (smr >> SMR_MASK_SHIFT) & SMR_ID_MASK, s2cr,
           s2cr_type == 0   ? "TRANS"
           : s2cr_type == 1 ? "BYPASS"
                            : "FAULT",
           s2cr & S2CR_CBNDX_MASK));
    }
    else {
      invalid_count++;
    }
  }

  DEBUG(
      (EFI_D_WARN,
       "===========================\n"
       "Total: %u VALID, %u invalid SMRs\n"
       "===========================\n\n",
       valid_count, invalid_count));
}

// ============================================================================
// Main Detach Function
// ============================================================================

/**
  Detach device from SMMU by device name.
  This function will try to detach ALL valid StreamID mappings for the device.
  Additionally, it will detach all SMRs using the same Context Banks.

  @param[in] smmuDevice       SMMU device structure
  @param[in] IortTable        Pointer to IORT table
  @param[in] DeviceName       Device name string (e.g., "UFS_MEM")

  @retval EFI_SUCCESS         At least one StreamID was detached successfully
  @retval EFI_NOT_FOUND       Device not found in IORT or no StreamIDs found in SMMU
  @retval other               Error occurred
**/
EFI_STATUS
smmu_v2_detach_device_by_name(
    ARM_SMMU_V2_DEVICE *smmuDevice, VOID *IortTable, CONST CHAR8 *DeviceName)
{
  EFI_STATUS                                 Status;
  EFI_ACPI_6_0_IO_REMAPPING_TABLE           *Iort;
  EFI_ACPI_6_0_IO_REMAPPING_NODE            *Node;
  EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE *NamedNode;
  EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE        *IdMapping;
  UINT8                                     *TableBase;
  UINT8                                     *NodeBase;
  CHAR8                                     *NodeName;
  UINT32                                     NodeIndex;
  BOOLEAN                                    DeviceFound = FALSE;
  BOOLEAN                                    AnyDetached = FALSE;
  UINT32                                     ContextBanks[16]; // Track context banks used
  UINT32                                     NumContextBanks = 0;

  DEBUG((EFI_D_WARN, "\n========================================\n"));
  DEBUG((EFI_D_WARN, "Detaching device: \"%a\"\n", DeviceName));
  DEBUG((EFI_D_WARN, "========================================\n"));

  Iort      = (EFI_ACPI_6_0_IO_REMAPPING_TABLE *)IortTable;
  TableBase = (UINT8 *)IortTable;
  ZeroMem(ContextBanks, sizeof(ContextBanks));

  // Search for the device in IORT
  for (NodeIndex = 0; NodeIndex < Iort->NumNodes; NodeIndex++) {
    if (NodeIndex == 0) {
      NodeBase = TableBase + Iort->NodeOffset;
    }
    else {
      NodeBase = NodeBase + Node->Length;
    }

    Node = (EFI_ACPI_6_0_IO_REMAPPING_NODE *)NodeBase;

    if (Node->Type == EFI_ACPI_IORT_TYPE_NAMED_COMP) {
      NamedNode = (EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE *)NodeBase;
      NodeName  = (CHAR8 *)(NodeBase +
                           sizeof(EFI_ACPI_6_0_IO_REMAPPING_NAMED_COMP_NODE));

      if (AsciiStriCmp(NodeName, DeviceName) == 0) {
        DeviceFound = TRUE;
        DEBUG((EFI_D_WARN, "Device \"%a\" found in IORT\n", NodeName));
        DEBUG((EFI_D_WARN, "  NumIdMappings: %u\n", Node->NumIdMappings));

        // Dump current SMMU mappings
        dump_smmu_mappings(smmuDevice);

        // Iterate through all ID mappings and try to detach each valid
        // StreamID
        if (Node->NumIdMappings > 0) {
          UINTN  OffsetToMappings = (UINTN)Node->IdReference;
          UINT32 MappingIndex;

          for (MappingIndex = 0; MappingIndex < Node->NumIdMappings;
               MappingIndex++) {
            IdMapping = (EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE
                             *)(NodeBase + OffsetToMappings +
                                (MappingIndex *
                                 sizeof(EFI_ACPI_6_0_IO_REMAPPING_ID_TABLE)));

            // Skip invalid mappings (OutputBase == 0)
            if (IdMapping->OutputBase == 0) {
              DEBUG(
                  (EFI_D_WARN,
                   "  Skipping Mapping[%u]: OutputBase is 0 (invalid)\n",
                   MappingIndex));
              continue;
            }

            DEBUG(
                (EFI_D_WARN,
                 "\n  Trying to detach Mapping[%u]: StreamID 0x%x\n",
                 MappingIndex, IdMapping->OutputBase));

            // Try to find existing SMR for this StreamID
            INTN smr_idx =
                find_smr_for_streamid(smmuDevice, IdMapping->OutputBase);
            
            if (smr_idx >= 0) {
              // Found in active SMR - detach normally
              UINT32 s2cr  = arm_smmu_gr0_read(
                  smmuDevice, ARM_SMMU_GR0_S2CR(smr_idx));
              UINT32 cbndx = s2cr & S2CR_CBNDX_MASK;

              // Track this context bank
              BOOLEAN AlreadyTracked = FALSE;
              for (UINT32 i = 0; i < NumContextBanks; i++) {
                if (ContextBanks[i] == cbndx) {
                  AlreadyTracked = TRUE;
                  break;
                }
              }
              if (!AlreadyTracked && NumContextBanks < 16) {
                ContextBanks[NumContextBanks++] = cbndx;
                DEBUG((EFI_D_WARN, "  Tracking Context Bank %u\n", cbndx));
              }

              Status =
                  smmu_v2_detach_device(smmuDevice, IdMapping->OutputBase);
              if (!EFI_ERROR(Status)) {
                DEBUG(
                    (EFI_D_WARN, "  ✓ Successfully detached StreamID 0x%x\n",
                     IdMapping->OutputBase));
                AnyDetached = TRUE;
              }
            }
            else {
              // StreamID not in active SMR list
              // Still need to configure it as FAULT to prevent future use
              DEBUG(
                  (EFI_D_WARN,
                   "  StreamID 0x%x not in active SMR, configuring as "
                   "FAULT...\n",
                   IdMapping->OutputBase));

              // Find a free SMR slot or use the OutputBase as index (modulo
              // numsmrg)
              UINT32 id0     = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
              UINT32 numsmrg = (id0 >> 0) & 0xFF;
              UINT32 target_smr_idx =
                  IdMapping->OutputBase %
                  numsmrg; // Simple hash to avoid conflicts

              // Configure this SMR to match the StreamID
              UINT32 new_smr = (IdMapping->OutputBase << SMR_ID_SHIFT) |
                               (0x0000 << SMR_MASK_SHIFT); // No masking
              arm_smmu_gr0_write(
                  smmuDevice, ARM_SMMU_GR0_SMR(target_smr_idx), new_smr);

              // Set corresponding S2CR to FAULT mode
              UINT32 fault_s2cr = S2CR_TYPE_FAULT;
              arm_smmu_gr0_write(
                  smmuDevice, ARM_SMMU_GR0_S2CR(target_smr_idx), fault_s2cr);
              MemoryFence();

              DEBUG(
                  (EFI_D_WARN,
                   "  ✓ Configured SMR[%u] and S2CR[%u] for StreamID 0x%x as "
                   "FAULT\n",
                   target_smr_idx, target_smr_idx, IdMapping->OutputBase));
              AnyDetached = TRUE;
            }
          }
        }

        // Now detach ALL other SMRs using the same Context Banks
        // Also, check for any SMRs using common banks that might belong to this device
        DEBUG(
            (EFI_D_WARN,
             "\n  Cleaning up additional SMRs using tracked Context Banks...\n"));
        DEBUG(
            (EFI_D_WARN, "  Tracked %u Context Banks: ", NumContextBanks));
        for (UINT32 j = 0; j < NumContextBanks; j++) {
          DEBUG((EFI_D_WARN, "%u ", ContextBanks[j]));
        }
        DEBUG((EFI_D_WARN, "\n"));
        
        UINT32 id0     = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
        UINT32 numsmrg = (id0 >> 0) & 0xFF;

        for (UINT32 i = 0; i < numsmrg; i++) {
          UINT32 smr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_SMR(i));
          
          if (!(smr & SMR_VALID)) {
            continue; // Skip invalid SMRs
          }

          UINT32 s2cr  = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_S2CR(i));
          UINT32 cbndx = s2cr & S2CR_CBNDX_MASK;

          // Check if this SMR uses one of our tracked Context Banks
          BOOLEAN should_detach = FALSE;
          for (UINT32 j = 0; j < NumContextBanks; j++) {
            if (cbndx == ContextBanks[j]) {
              should_detach = TRUE;
              break;
            }
          }

          if (should_detach) {
            UINT32 streamid = (smr >> SMR_ID_SHIFT) & SMR_ID_MASK;
            DEBUG(
                (EFI_D_WARN,
                 "  Found additional SMR[%u]: StreamID=0x%x, CBNDX=%u\n", i,
                 streamid, cbndx));
            
            // Detach this SMR
            Status = smmu_v2_detach_device(smmuDevice, streamid);
            if (!EFI_ERROR(Status)) {
              DEBUG(
                  (EFI_D_WARN,
                   "Successfully detached additional StreamID 0x%x\n",
                   streamid));
              AnyDetached = TRUE;
            }
            else {
              DEBUG(
                  (EFI_D_WARN,
                   "Failed to detach StreamID 0x%x\n",
                   streamid));
            }
          }
        }

        // Final status dump
        DEBUG((EFI_D_WARN, "\n  Final SMMU state after cleanup:\n"));
        dump_smmu_mappings(smmuDevice);

        break; // Device found, stop searching
      }
    }
  }

  if (!DeviceFound) {
    DEBUG((EFI_D_WARN, "Device \"%a\" not found in IORT table\n", DeviceName));
    return EFI_NOT_FOUND;
  }

  if (!AnyDetached) {
    DEBUG(
        (EFI_D_WARN,
         "Warning: No valid StreamIDs found in SMMU for device \"%a\"\n",
         DeviceName));
    return EFI_NOT_FOUND;
  }

  DEBUG(
      (EFI_D_WARN, "\n=== Device \"%a\" detached successfully ===\n\n",
       DeviceName));

  return EFI_SUCCESS;
}

/**
  Legacy detach function using StreamID directly.

  @param[in] smmuDevice   SMMU device structure
  @param[in] streamid     Stream ID to detach

  @retval EFI_SUCCESS     Device detached successfully
  @retval other           Error occurred
**/
EFI_STATUS
smmu_v2_detach_device(ARM_SMMU_V2_DEVICE *smmuDevice, UINT32 streamid)
{
  INTN   smr_idx;
  UINT32 s2cr;
  UINT32 cbndx;
  UINT32 s2cr_type;
  UINT32 smr;

  DEBUG(
      (EFI_D_WARN, "=== Detaching device with StreamID 0x%x ===\n", streamid));

  // Dump current mappings for debugging
  dump_smmu_mappings(smmuDevice);

  // Find the SMR index for this StreamID
  smr_idx = find_smr_for_streamid(smmuDevice, streamid);
  if (smr_idx < 0) {
    DEBUG((EFI_D_ERROR, "Error: StreamID 0x%x not found\n", streamid));
    return EFI_NOT_FOUND;
  }

  // Read S2CR register
  s2cr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_S2CR(smr_idx));
  DEBUG((EFI_D_WARN, "Current S2CR[%d]: 0x%08x\n", smr_idx, s2cr));

  // Extract Context Bank index and type
  cbndx     = s2cr & S2CR_CBNDX_MASK;
  s2cr_type = (s2cr >> S2CR_TYPE_SHIFT) & 0x3;

  DEBUG(
      (EFI_D_WARN, "  Type: %a, CBNDX: %u\n",
       s2cr_type == 0   ? "TRANS"
       : s2cr_type == 1 ? "BYPASS"
                        : "FAULT",
       cbndx));

  // If translation type, disable the Context Bank
  if (s2cr_type == 0) { // S2CR_TYPE_TRANS
    if (cbndx < smmuDevice->num_context_banks) {
      disable_context_bank(smmuDevice, cbndx);
    }
    else {
      DEBUG(
          (EFI_D_WARN, "Warning: Invalid CBNDX %u (max %u)\n", cbndx,
           smmuDevice->num_context_banks - 1));
    }
  }

  // Set S2CR to FAULT mode to block all accesses
  s2cr = S2CR_TYPE_FAULT | cbndx;
  arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_S2CR(smr_idx), s2cr);
  MemoryFence();

  DEBUG((EFI_D_WARN, "Set S2CR[%d] to FAULT mode:  0x%08x\n", smr_idx, s2cr));

  // Invalidate the SMR entry
  smr = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_SMR(smr_idx));
  smr &= ~SMR_VALID;
  arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_SMR(smr_idx), smr);
  MemoryFence();

  DEBUG((EFI_D_WARN, "Invalidated SMR[%d]:  0x%08x\n", smr_idx, smr));

  // Invalidate TLB
  smmu_tlb_invalidate_all(smmuDevice);

  DEBUG((EFI_D_WARN, "=== Device detached successfully ===\n\n"));

  return EFI_SUCCESS;
}

// ============================================================================
// MMU-500 Reset
// ============================================================================

INTN arm_mmu500_reset(ARM_SMMU_V2_DEVICE *smmuDevice)
{
  UINT32 reg = 0, major = 0;

  reg   = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID7);
  major = READ_FIELD(ARM_SMMU_ID7_MAJOR, reg);

  reg = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_sACR);

  if (major >= 2) {
    reg &= ~ARM_MMU500_ACR_CACHE_LOCK;
  }

  reg |= ARM_MMU500_ACR_SMTNMB_TLBEN | ARM_MMU500_ACR_S2CRB_TLBEN;
  arm_smmu_gr0_write(smmuDevice, ARM_SMMU_GR0_sACR, reg);

  for (UINT32 i = 0; i < smmuDevice->num_context_banks; ++i) {
    reg = arm_smmu_cb_read(smmuDevice, i, ARM_SMMU_CB_ACTLR);
    reg &= ~ARM_MMU500_ACTLR_CPRE;
    arm_smmu_cb_write(smmuDevice, i, ARM_SMMU_CB_ACTLR, reg);
    reg = arm_smmu_cb_read(smmuDevice, i, ARM_SMMU_CB_ACTLR);
    if ((reg & ARM_MMU500_ACTLR_CPRE)) {
      DEBUG(
          (EFI_D_WARN, "Failed to disable prefetcher for errata workarounds, "
                       "check SACR.CACHE_LOCK\n"));
    }
  }
  return 0;
}

// ============================================================================
// Initialization
// ============================================================================

EFI_STATUS
mmuInit(IN OUT ARM_SMMU_V2_DEVICE *smmuDevice)
{
  UINT32 id0;
  UINT32 id1;
  UINT32 size;

  // Read ID registers
  id0 = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID0);
  id1 = arm_smmu_gr0_read(smmuDevice, ARM_SMMU_GR0_ID1);

  // Determine page size
  smmuDevice->pgshift = (id1 & ARM_SMMU_ID1_PAGESIZE) ? 16 : 12;

  // Calculate number of pages
  size                = 1 << ((READ_FIELD(ARM_SMMU_ID1_NUMPAGENDXB, id1)) + 1);
  smmuDevice->numpage = size;

  // Get number of context banks
  smmuDevice->num_context_banks = id1 & ARM_SMMU_ID1_NUMCB_MASK;

  DEBUG(
      (EFI_D_WARN,
       "SMMUv2 detected:\n"
       "  Base Address: 0x%lx\n"
       "  Page Size:  %uKB\n"
       "  Context Banks: %u\n"
       "  Total Size: %uKB\n"
       "  SMR Groups: %u\n",
       smmuDevice->smmu_base_address, (1 << (smmuDevice->pgshift - 10)),
       smmuDevice->num_context_banks, size << (smmuDevice->pgshift - 10),
       (id0 & 0xFF)));

  return EFI_SUCCESS;
}

EFI_STATUS
mmuDetachDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS         Status;
  ARM_SMMU_V2_DEVICE smmuDevice = {
      .smmu_base_address = 0,
      .pgshift           = 0,
      .numpage           = 0,
      .num_context_banks = 0,
  };
  VOID *IortTable = NULL;
  UINTN IortSize  = 0;

  // Load IORT table from firmware volume first
  Status = LoadIortFromFirmwareVolume(&IortTable, &IortSize);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to load IORT table: %r\n", Status));
    DEBUG((EFI_D_ERROR, "Exit because of non-exist IORT table\n"));
    return Status;
  }

  // Get SMMU base address/size from IORT
  Status = GetSmmuInfoFromIort(IortTable, &smmuDevice);
  if (EFI_ERROR(Status)) {
    DEBUG((
        EFI_D_WARN, "Failed to get SMMU base address from IORT: %r\n", Status));
    FreePool(IortTable);
    return Status;
  }

  // Initialize SMMU device structure
  Status = mmuInit(&smmuDevice);
  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_WARN, "Failed to initialize SMMU: %r\n", Status));
    FreePool(IortTable);
    return Status;
  }

  DEBUG(
      (EFI_D_WARN, "IORT table loaded successfully (%u bytes)\n\n", IortSize));

  // Detach UFS device by name (dynamic method)
  // Try multiple possible device names
  CONST CHAR8 *UfsDeviceNames[] = {
      "\\_SB_.UFS0", // Standard ACPI path name
      "\\_SB.UFS0",  // Alternate name
      "UFS_MEM",     // Alternate name
      NULL           // Terminator
  };
  UINTN   UfsDeviceIdx;
  BOOLEAN UfsDetached = FALSE;

  for (UfsDeviceIdx = 0; UfsDeviceNames[UfsDeviceIdx] != NULL; UfsDeviceIdx++) {
    DEBUG(
        (EFI_D_WARN, "Attempting detach for device: \"%a\"...\n",
         UfsDeviceNames[UfsDeviceIdx]));
    Status = smmu_v2_detach_device_by_name(
        &smmuDevice, IortTable, UfsDeviceNames[UfsDeviceIdx]);
    if (!EFI_ERROR(Status)) {
      DEBUG(
          (EFI_D_WARN, "Successfully detached \"%a\"\n",
           UfsDeviceNames[UfsDeviceIdx]));
      UfsDetached = TRUE;
      break;
    }
  }

  if (!UfsDetached) {
    DEBUG((EFI_D_WARN, "Warning: Could not detach any UFS device variant\n"));
  }

  // Free IORT table memory
  if (IortTable != NULL) {
    FreePool(IortTable);
  }

  // Reset MMU-500 (apply errata workarounds)
  arm_mmu500_reset(&smmuDevice);

  DEBUG((EFI_D_WARN, "\n========================================\n"));
  DEBUG((EFI_D_WARN, "SMMU v2 Detach Driver Initialization Complete\n"));
  DEBUG((EFI_D_WARN, "========================================\n\n"));

  return EFI_SUCCESS;
}