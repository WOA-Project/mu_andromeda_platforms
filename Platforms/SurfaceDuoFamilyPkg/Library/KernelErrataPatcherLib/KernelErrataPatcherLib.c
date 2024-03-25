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
#include "KernelErrataPatcherLib.h"
#include "KernelErrataHandler.h"

STATIC EFI_EXIT_BOOT_SERVICES mOriginalEfiExitBootServices = NULL;
EFI_EVENT                     mReadyToBootEvent;

VOID KernelErrataPatcherApplyReadACTLREL1Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size)
{
  // Fix up #0 (28 10 38 D5 -> 08 00 80 D2) (ACTRL_EL1 Register Read)
  UINT8                FixedInstruction0[] = {0x08, 0x00, 0x80, 0xD2};
  EFI_PHYSICAL_ADDRESS IllegalInstruction0 =
      FindPattern(Base, Size, "28 10 38 D5");

  if (IllegalInstruction0 != 0) {
    FirmwarePrint("mrs x8, actlr_el1         -> 0x%p\n", IllegalInstruction0);

    CopyMemory(
        IllegalInstruction0, (EFI_PHYSICAL_ADDRESS)FixedInstruction0,
        sizeof(FixedInstruction0));
  }
}

EFI_STATUS
EFIAPI
KernelErrataPatcherExitBootServices(
    IN EFI_HANDLE ImageHandle, IN UINTN MapKey,
    IN EFI_PHYSICAL_ADDRESS fwpKernelSetupPhase1)
{
  // Might be called multiple times by winload in a loop failing few times
  gBS->ExitBootServices = mOriginalEfiExitBootServices;
  gBS->Hdr.CRC32        = 0;
  gBS->CalculateCrc32(gBS, sizeof(EFI_BOOT_SERVICES), &gBS->Hdr.CRC32);

  FirmwarePrint(
      "OslFwpKernelSetupPhase1   -> (phys) 0x%p\n", fwpKernelSetupPhase1);

  UINTN                tempSize = SCAN_MAX;
  EFI_PHYSICAL_ADDRESS winloadBase =
      LocateWinloadBase(fwpKernelSetupPhase1, &tempSize);

  EFI_STATUS Status = UnprotectWinload(winloadBase + EFI_PAGE_SIZE, tempSize);
  if (EFI_ERROR(Status)) {
    FirmwarePrint(
        "Could not unprotect winload -> (phys) 0x%p (size) 0x%p %r\n",
        winloadBase, tempSize, Status);

    goto exit;
  }

  // Fix up winload.efi
  // This fixes Boot Debugger
  FirmwarePrint(
      "Patching OsLoader         -> (phys) 0x%p (size) 0x%p\n",
      fwpKernelSetupPhase1, SCAN_MAX);

  KernelErrataPatcherApplyReadACTLREL1Patches(fwpKernelSetupPhase1, SCAN_MAX);

  BOOLEAN InjectedShellCode = FALSE;

  EFI_PHYSICAL_ADDRESS OslArm64TransferToKernelAddr =
      winloadBase + 0xC00 +
      NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL_FUNCTION_OFFSET_GERMANIUM;
  EFI_PHYSICAL_ADDRESS NewOslArm64TransferToKernelAddr =
      OslArm64TransferToKernelAddr - ARM64_INSTRUCTION_LENGTH;

  for (EFI_PHYSICAL_ADDRESS current = OslArm64TransferToKernelAddr;
       current < OslArm64TransferToKernelAddr + SCAN_MAX;
       current += sizeof(UINT32)) {

    if (ARM64_BRANCH_LOCATION_INSTRUCTION(
            current, OslArm64TransferToKernelAddr) == *(UINT32 *)current &&
        *(UINT32 *)(current + sizeof(UINT32)) == 0xD2800002) {
      FirmwarePrint(
          "Patching bl OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
          current);

      *(UINT32 *)current = ARM64_BRANCH_LOCATION_INSTRUCTION(
          current, NewOslArm64TransferToKernelAddr);

      FirmwarePrint(
          "Patching OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
          OslArm64TransferToKernelAddr);

      // I love C99
      EFI_PHYSICAL_ADDRESS PreOslArm64TransferToKernelAddr =
          (EFI_PHYSICAL_ADDRESS)&PreOslArm64TransferToKernel;

      // But I hate the C compiler
      PreOslArm64TransferToKernelAddr =
          PreOslArm64TransferToKernelAddr & 0xFFFFFFFF;

      // Copy shell code right before the
      // OsLoaderArm64TransferToKernelFunction
      *(UINT32 *)NewOslArm64TransferToKernelAddr =
          ARM64_BRANCH_LOCATION_INSTRUCTION(
              NewOslArm64TransferToKernelAddr, PreOslArm64TransferToKernelAddr);

      InjectedShellCode = TRUE;

      break;
    }
  }

  if (!InjectedShellCode) {
    OslArm64TransferToKernelAddr =
        winloadBase + 0xC00 +
        NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL_FUNCTION_OFFSET;
    NewOslArm64TransferToKernelAddr =
        OslArm64TransferToKernelAddr - ARM64_INSTRUCTION_LENGTH;

    for (EFI_PHYSICAL_ADDRESS current = OslArm64TransferToKernelAddr;
         current < OslArm64TransferToKernelAddr + SCAN_MAX;
         current += sizeof(UINT32)) {

      if (ARM64_BRANCH_LOCATION_INSTRUCTION(
              current, OslArm64TransferToKernelAddr) == *(UINT32 *)current &&
          *(UINT32 *)(current + sizeof(UINT32)) == 0xD2800002) {
        FirmwarePrint(
            "Patching bl OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
            current);

        *(UINT32 *)current = ARM64_BRANCH_LOCATION_INSTRUCTION(
            current, NewOslArm64TransferToKernelAddr);

        FirmwarePrint(
            "Patching OsLoaderArm64TransferToKernel -> (phys) 0x%p\n",
            OslArm64TransferToKernelAddr);

        // I love C99
        EFI_PHYSICAL_ADDRESS PreOslArm64TransferToKernelAddr =
            (EFI_PHYSICAL_ADDRESS)&PreOslArm64TransferToKernel;

        // But I hate the C compiler
        PreOslArm64TransferToKernelAddr =
            PreOslArm64TransferToKernelAddr & 0xFFFFFFFF;

        // Copy shell code right before the
        // OsLoaderArm64TransferToKernelFunction
        *(UINT32 *)NewOslArm64TransferToKernelAddr =
            ARM64_BRANCH_LOCATION_INSTRUCTION(
                NewOslArm64TransferToKernelAddr,
                PreOslArm64TransferToKernelAddr);

        InjectedShellCode = TRUE;

        break;
      }
    }
  }

  FirmwarePrint(
      "Protecting winload        -> (phys) 0x%p (size) 0x%p\n", winloadBase,
      tempSize);

  Status = ReProtectWinload(winloadBase + EFI_PAGE_SIZE, tempSize);
  if (EFI_ERROR(Status)) {
    FirmwarePrint(
        "Could not reprotect winload -> (phys) 0x%p (size) 0x%p %r\n",
        winloadBase, tempSize, Status);

    goto exit;
  }

exit:
  FirmwarePrint(
      "OslFwpKernelSetupPhase1   <- (phys) 0x%p\n", fwpKernelSetupPhase1);

  // Call the original
  return gBS->ExitBootServices(ImageHandle, MapKey);
}

VOID EFIAPI ReadyToBootHandler(IN EFI_EVENT Event, IN VOID *Context)
{
  PERF_CALLBACK_BEGIN(&gEfiEventReadyToBootGuid);

  mOriginalEfiExitBootServices = gBS->ExitBootServices;
  gBS->ExitBootServices        = ExitBootServicesWrapper;
  gBS->Hdr.CRC32               = 0;
  gBS->CalculateCrc32(gBS, sizeof(EFI_BOOT_SERVICES), &gBS->Hdr.CRC32);

  gBS->CloseEvent(mReadyToBootEvent);

  PERF_CALLBACK_END(&gEfiEventReadyToBootGuid);
}

EFI_STATUS
EFIAPI
KernelErrataPatcherLibConstructor(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  Status = gBS->CreateEventEx(
      EVT_NOTIFY_SIGNAL, TPL_CALLBACK, ReadyToBootHandler, NULL,
      &gEfiEventReadyToBootGuid, &mReadyToBootEvent);
  if (EFI_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR,
         "KernelErrataPatcherLibConstructor: Failed to create event %r\n",
         Status));
    ASSERT(FALSE);
  }

  return InitMemoryAttributeProtocol();
}