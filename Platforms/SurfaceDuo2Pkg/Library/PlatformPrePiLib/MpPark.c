#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>
#include <Library/PlatformPrePiLib.h>

#include <Library/ArmGicLib.h>

#include "MpPark.h"
#include <Configuration/DeviceMemoryMap.h>

static UINT32 ProcessorIdMapping[8] = {
    0x00000000, 0x00000100, 0x00000200, 0x00000300,
    0x00000400, 0x00000500, 0x00000600, 0x00000700,
};

EFI_STATUS
EFIAPI
MpParkLocateArea(PARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      gDeviceMemoryDescriptorEx;

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    if (AsciiStriCmp("MPPark Code", MemoryDescriptorEx->Name) == 0) {
      *MemoryDescriptor = MemoryDescriptorEx;
      return EFI_SUCCESS;
    }
    MemoryDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

VOID WaitForSecondaryCPUs(VOID)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MpParkMemoryRegion = NULL;
  MpParkLocateArea(&MpParkMemoryRegion);

  while (1) {

    BOOLEAN IsAllCpuLaunched = TRUE;

    for (UINTN Index = 1; Index < FixedPcdGet32(PcdCoreCount); Index++) {
      EFI_PHYSICAL_ADDRESS MailboxAddress =
          MpParkMemoryRegion->Address + 0x10000 * Index + 0x1000;
      PEFI_PROCESSOR_MAILBOX pMailbox =
          (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

      ArmDataSynchronizationBarrier();
      if (pMailbox->JumpFlag != REDIR_MAILBOX_READY) {
        IsAllCpuLaunched = FALSE;
        break;
      }
    }

    // All CPU started. Continue.
    if (IsAllCpuLaunched) {
      break;
    }
  }
}

VOID MpParkMain(UINTN MpIdr)
{
  UINTN MpId  = 0;

  for (MpId = 0; MpId < FixedPcdGet32(PcdCoreCount); MpId++) {
    if (MpIdr == ProcessorIdMapping[MpId]) {
      if (MpId == 0) {
        ASSERT(FALSE);
      }
      break;
    }

    if (MpId + 1 == FixedPcdGet32(PcdCoreCount)) {
      ASSERT(FALSE);
    }
  }

  // We must never get into this function on UniCore system
  ASSERT(MpId >= 1 && MpId <= 7);

  PARM_MEMORY_REGION_DESCRIPTOR_EX MpParkMemoryRegion = NULL;
  MpParkLocateArea(&MpParkMemoryRegion);

  EFI_PHYSICAL_ADDRESS MailboxAddress =
      MpParkMemoryRegion->Address + 0x10000 * MpId + 0x1000;
  PEFI_PROCESSOR_MAILBOX pMailbox =
      (PEFI_PROCESSOR_MAILBOX)(VOID *)MailboxAddress;

  UINT32 CurrentProcessorId = 0;
  VOID (*SecondaryStart)(VOID * pMailbox);
  UINTN SecondaryEntryAddr;
  UINTN InterruptId;
  UINTN AcknowledgeInterrupt;

  // MMU, cache and branch predicton must be disabled
  // Cache is disabled in CRT startup code
  ArmDisableMmu();
  ArmDisableBranchPrediction();

  // Clear mailbox
  pMailbox->JumpAddress = 0;
  pMailbox->ProcessorId = 0xffffffff;

  // Notify the main processor that we are here
  pMailbox->JumpFlag = REDIR_MAILBOX_READY;
  CurrentProcessorId = ProcessorIdMapping[MpId];
  ArmDataSynchronizationBarrier();

  // Turn on GIC CPU interface as well as SGI interrupts
  ArmGicEnableInterruptInterface(FixedPcdGet64(PcdGicInterruptInterfaceBase));
  // TODO: Fix me
  // MmioWrite32(FixedPcdGet64(PcdGicInterruptInterfaceBase) + 0x4, 0xf0);

  // But turn off interrupts
  ArmDisableInterrupts();

  do {
    // Technically we should do a WFI
    // But we just spin here instead
    ArmDataSynchronizationBarrier();

    // Technically the CPU ID should be checked
    // against request per MpPark spec,
    // but the actual Windows implementation guarantees
    // that no CPU will be started simultaneously,
    // so the check was made optional.
    //
    // This also enables "spin-table" startup method
    // for Linux.
    //
    // Example usage:
    // enable-method = "spin-table";
    // cpu-release-addr = <0 0xE3412008>;
    if (FixedPcdGetBool(SecondaryCpuIgnoreCpuIdCheck) ||
        pMailbox->ProcessorId == MpIdr) {
      SecondaryEntryAddr = pMailbox->JumpAddress;
    }

    AcknowledgeInterrupt = ArmGicAcknowledgeInterrupt(
        FixedPcdGet64(PcdGicInterruptInterfaceBase), &InterruptId);
    if (InterruptId <
        ArmGicGetMaxNumInterrupts(FixedPcdGet64(PcdGicDistributorBase))) {
      // Got a valid SGI number hence signal End of Interrupt
      ArmGicEndOfInterrupt(
          FixedPcdGet64(PcdGicInterruptInterfaceBase), AcknowledgeInterrupt);
    }
  } while (SecondaryEntryAddr == 0);

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "CPU %d is about to jump into HLOS!\n", MpId));

  // Acknowledge this one
  pMailbox->JumpAddress = 0;

  SecondaryStart = (VOID(*)())SecondaryEntryAddr;
  SecondaryStart(pMailbox);

  // Should never reach here
  ASSERT(FALSE);
}

UINTN
PSCI_CPU_ON(UINTN target_cpu, UINTN entry_point_address, UINTN context_id)
{
  ARM_HVC_ARGS ArmHvcArgs;
  ArmHvcArgs.Arg0 = ARM_SMC_ID_PSCI_CPU_ON_AARCH64;
  ArmHvcArgs.Arg1 = target_cpu;
  ArmHvcArgs.Arg2 = entry_point_address;
  ArmHvcArgs.Arg3 = context_id;

  ArmCallHvc(&ArmHvcArgs);
  return ArmHvcArgs.Arg0;
}

VOID LaunchAllCPUs(VOID)
{
  // Immediately launch all CPUs, 7 CPUs hold
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "Launching CPUs\n"));

  // Launch all CPUs
  if (ArmReadCurrentEL() == AARCH64_EL1) {
    for (UINTN i = 1; i < FixedPcdGet32(PcdCoreCount); i++) {
      DEBUG((EFI_D_INFO | EFI_D_LOAD, "Launching CPU %d\n", i));
      ASSERT(PSCI_CPU_ON(ProcessorIdMapping[i], (UINTN)&_SecondaryModuleEntryPoint, ProcessorIdMapping[i]) == ARM_SMC_PSCI_RET_SUCCESS);
    }
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Waiting for all CPUs...\n"));
  WaitForSecondaryCPUs();
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "All CPU started.\n"));
}