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
#include "KernelErrataPatcher.h"

#define SILENT 1

STATIC BL_ARCH_SWITCH_CONTEXT BlpArchSwitchContext = NULL;
STATIC EFI_EXIT_BOOT_SERVICES EfiExitBootServices  = NULL;

#if SILENT == 0

#define FirmwarePrint(x, ...)                                                  \
  AsciiPrint(x, __VA_ARGS__);                                                  \
  DEBUG((EFI_D_ERROR, x, __VA_ARGS__));

#define ContextPrint(x, ...)                                                   \
  BlpArchSwitchContext(FirmwareContext);                                       \
  FirmwarePrint(x, __VA_ARGS__);                                               \
  BlpArchSwitchContext(ApplicationContext);

#else
#define FirmwarePrint(x, ...)
#define ContextPrint(x, ...)
#endif

#define ApplicationPrint(IsInFirmwareContext, x, ...)                          \
  if (IsInFirmwareContext) {                                                   \
    FirmwarePrint(x, __VA_ARGS__);                                             \
  }                                                                            \
  else {                                                                       \
    ContextPrint(x, __VA_ARGS__);                                              \
  }

VOID KernelErrataPatcherApplyReadAMCNTENSET0EL0Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #0 (A8 D2 3B D5 -> 08 00 80 D2) (AMCNTENSET0_EL0 Register Read)
  UINT8                FixedInstruction0[] = {0x08, 0x00, 0x80, 0xD2};
  EFI_PHYSICAL_ADDRESS IllegalInstruction0 =
      FindPattern(Base, Size, "A8 D2 3B D5");

  while (IllegalInstruction0 != 0) {
    ApplicationPrint(
        IsInFirmwareContext, "mrs x8, amcntenset0_el0         -> 0x%p\n",
        IllegalInstruction0);

    CopyMemory(
        IllegalInstruction0, (EFI_PHYSICAL_ADDRESS)FixedInstruction0,
        sizeof(FixedInstruction0));

    IllegalInstruction0 = FindPattern(Base, Size, "A8 D2 3B D5");
  }
}

VOID KernelErrataPatcherApplyReadACTLREL1Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #0 (28 10 38 D5 -> 08 00 80 D2) (ACTRL_EL1 Register Read)
  UINT8                FixedInstruction0[] = {0x08, 0x00, 0x80, 0xD2};
  EFI_PHYSICAL_ADDRESS IllegalInstruction0 =
      FindPattern(Base, Size, "28 10 38 D5");
  UINT8 PatchCounter = 0;

  while (IllegalInstruction0 != 0) {
    ApplicationPrint(
        IsInFirmwareContext, "mrs x8, actlr_el1         -> 0x%p\n",
        IllegalInstruction0);

    CopyMemory(
        IllegalInstruction0, (EFI_PHYSICAL_ADDRESS)FixedInstruction0,
        sizeof(FixedInstruction0));

    // Commenting out for boot speed optimization purposes, as there's only
    // two (3 for VB) read occurences really in the kernel and one in winload

    // IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
    if (IsInFirmwareContext) {
      IllegalInstruction0 = 0;
    }
    else {
      PatchCounter++;
      if (PatchCounter != 3) {
        IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
      }
      else {
        IllegalInstruction0 = 0;
      }
    }
  }
}

VOID KernelErrataPatcherApplyWriteAMCNTENSET0EL0Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #1 (A9 D2 1B D5 -> 1F 20 03 D5) (AMCNTENSET0_EL0 Register Write)
  UINT8                FixedInstruction1[] = {0x1F, 0x20, 0x03, 0xD5};
  EFI_PHYSICAL_ADDRESS IllegalInstruction1 =
      FindPattern(Base, Size, "A9 D2 1B D5");

  while (IllegalInstruction1 != 0) {
    ApplicationPrint(
        IsInFirmwareContext, "msr amcntenset0_el0, x9         -> 0x%p\n",
        IllegalInstruction1);

    CopyMemory(
        IllegalInstruction1, (EFI_PHYSICAL_ADDRESS)FixedInstruction1,
        sizeof(FixedInstruction1));

    IllegalInstruction1 = FindPattern(Base, Size, "A9 D2 1B D5");
  }
}

VOID KernelErrataPatcherApplyWriteACTLREL1Patches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #1 (29 10 18 D5 -> 1F 20 03 D5) (ACTRL_EL1 Register Write)
  UINT8                FixedInstruction1[] = {0x1F, 0x20, 0x03, 0xD5};
  EFI_PHYSICAL_ADDRESS IllegalInstruction1 =
      FindPattern(Base, Size, "29 10 18 D5");

  while (IllegalInstruction1 != 0) {
    ApplicationPrint(
        IsInFirmwareContext, "msr actlr_el1, x9         -> 0x%p\n",
        IllegalInstruction1);

    CopyMemory(
        IllegalInstruction1, (EFI_PHYSICAL_ADDRESS)FixedInstruction1,
        sizeof(FixedInstruction1));

    // Commenting out for boot speed optimization purposes, as there's only
    // one write occurence really in the kernel

    // IllegalInstruction1 = FindPattern(Base, Size, "29 10 18 D5");
    IllegalInstruction1 = 0;
  }
}

VOID KernelErrataPatcherApplyPsciMemoryProtectionPatches(
    EFI_PHYSICAL_ADDRESS Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #0 (PsciMemProtect -> C0 03 5F D6) (PsciMemProtect -> RET)
  UINT8                RetInstruction[] = {0xC0, 0x03, 0x5F, 0xD6};
  EFI_PHYSICAL_ADDRESS PatternMatch     = FindPattern(
      Base, Size, "D5 02 00 18 03 00 80 D2 02 00 80 D2 01 00 80 D2");
  EFI_PHYSICAL_ADDRESS PsciMemProtect =
      PatternMatch - ARM64_TOTAL_INSTRUCTION_LENGTH(8);

  if (PatternMatch != 0) {
    ApplicationPrint(
        IsInFirmwareContext, "PsciMemProtect            -> 0x%p\n",
        PsciMemProtect);

    CopyMemory(
        PsciMemProtect, (EFI_PHYSICAL_ADDRESS)RetInstruction,
        sizeof(RetInstruction));
  }
  else {
    PatternMatch = FindPattern(
        Base, Size, "03 00 80 D2 02 00 80 D2 01 00 80 D2 40 02 00 18");
    PsciMemProtect = PatternMatch - ARM64_TOTAL_INSTRUCTION_LENGTH(7);

    if (PatternMatch != 0) {
      ApplicationPrint(
          IsInFirmwareContext, "PsciMemProtect            -> 0x%p\n",
          PsciMemProtect);

      CopyMemory(
          PsciMemProtect, (EFI_PHYSICAL_ADDRESS)RetInstruction,
          sizeof(RetInstruction));
    }
    else {
      ApplicationPrint(
          IsInFirmwareContext,
          "PsciMemProtect            -> Not Found! Base: 0x%p, Size: 0x%p\n",
          Base, Size);
    }
  }
}

EFI_STATUS
EFIAPI
KernelErrataPatcherExitBootServices(
    IN EFI_HANDLE ImageHandle, IN UINTN MapKey,
    IN PLOADER_PARAMETER_BLOCK loaderBlockX19,
    IN PLOADER_PARAMETER_BLOCK loaderBlockX20,
    IN PLOADER_PARAMETER_BLOCK loaderBlockX24,
    IN EFI_PHYSICAL_ADDRESS    fwpKernelSetupPhase1)
{
  // Might be called multiple times by winload in a loop failing few times
  gBS->ExitBootServices = EfiExitBootServices;

  FirmwarePrint(
      "OslFwpKernelSetupPhase1   -> (phys) 0x%p\n", fwpKernelSetupPhase1);

  // Fix up winload.efi
  // This fixes Boot Debugger
  FirmwarePrint(
      "Patching OsLoader         -> (phys) 0x%p (size) 0x%p\n",
      fwpKernelSetupPhase1, SCAN_MAX);

  // KernelErrataPatcherApplyReadACTLREL1Patches(fwpKernelSetupPhase1, SCAN_MAX,
  // TRUE);

  PLOADER_PARAMETER_BLOCK loaderBlock = loaderBlockX19;

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    FirmwarePrint(
        "Failed to find OslLoaderBlock (X19)! loaderBlock -> 0x%p\n",
        loaderBlock);
    loaderBlock = loaderBlockX20;
  }

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    FirmwarePrint(
        "Failed to find OslLoaderBlock (X20)! loaderBlock -> 0x%p\n",
        loaderBlock);
    loaderBlock = loaderBlockX24;
  }

  if (loaderBlock == NULL ||
      ((EFI_PHYSICAL_ADDRESS)loaderBlock & 0xFFFFFFF000000000) == 0) {
    FirmwarePrint(
        "Failed to find OslLoaderBlock (X24)! loaderBlock -> 0x%p\n",
        loaderBlock);
    goto exit;
  }

  FirmwarePrint("OslLoaderBlock            -> (virt) 0x%p\n", loaderBlock);

  EFI_PHYSICAL_ADDRESS PatternMatch = FindPattern(
      fwpKernelSetupPhase1, SCAN_MAX,
      "1F 04 00 71 33 11 88 9A 28 00 40 B9 1F 01 00 6B");

  // BlpArchSwitchContext
  BlpArchSwitchContext =
      (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                               ARM64_TOTAL_INSTRUCTION_LENGTH(9));

  // First check if the version of BlpArchSwitchContext before Memory Management
  // v2 is found
  if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
    FirmwarePrint(
        "Failed to find BlpArchSwitchContext (v1)! BlpArchSwitchContext -> "
        "0x%p\n",
        BlpArchSwitchContext);

    // Okay, we maybe have the new Memory Management? Try again.
    PatternMatch =
        FindPattern(fwpKernelSetupPhase1, SCAN_MAX, "9F 06 00 71 33 11 88 9A");

    // BlpArchSwitchContext
    BlpArchSwitchContext =
        (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                                 ARM64_TOTAL_INSTRUCTION_LENGTH(24));

    if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
      FirmwarePrint(
          "Failed to find BlpArchSwitchContext (v2)! BlpArchSwitchContext -> "
          "0x%p\n",
          BlpArchSwitchContext);

      // Okay, we maybe have the new new Memory Management? Try again.
      PatternMatch = FindPattern(
          fwpKernelSetupPhase1, SCAN_MAX, "7F 06 00 71 37 11 88 9A");

      // BlpArchSwitchContext
      BlpArchSwitchContext =
          (BL_ARCH_SWITCH_CONTEXT)(PatternMatch -
                                   ARM64_TOTAL_INSTRUCTION_LENGTH(24));

      if (PatternMatch == 0 || (PatternMatch & 0xFFFFFFF000000000) != 0) {
        FirmwarePrint(
            "Failed to find BlpArchSwitchContext (v3)! BlpArchSwitchContext -> "
            "0x%p\n",
            BlpArchSwitchContext);
        goto exit;
      }
    }
  }

  FirmwarePrint(
      "BlpArchSwitchContext      -> (phys) 0x%p\n", BlpArchSwitchContext);

  /*
   * Switch context to (as defined by winload) application context
   * Within this context only the virtual addresses are valid
   * Real/physical addressing is not used
   * We can not use any EFI services unless we switch back!
   * To print on screen use ContextPrint define
   */
  BlpArchSwitchContext(ApplicationContext);

  UINT32 OsMajorVersion = loaderBlock->OsMajorVersion;
  UINT32 OsMinorVersion = loaderBlock->OsMinorVersion;
  UINT32 Size           = loaderBlock->Size;

  ContextPrint(
      "LOADER_PARAMETER_BLOCK    -> OsMajorVersion: %d OsMinorVersion: %d "
      "Size: %d\n",
      OsMajorVersion, OsMinorVersion, Size);

  if (OsMajorVersion != 10 || OsMinorVersion != 0 || Size == 0) {
    ContextPrint(
        "Incompatible!           -> OsMajorVersion: %d OsMinorVersion: %d "
        "Size: %d\n",
        OsMajorVersion, OsMinorVersion, Size);

    goto exitToFirmware;
  }

  KLDR_DATA_TABLE_ENTRY kernelModule =
      *GetModule(&loaderBlock->LoadOrderListHead, NT_OS_KERNEL_IMAGE_NAME);

  if (kernelModule.DllBase == NULL) {
    ContextPrint(
        "Failed to find ntoskrnl.exe in OslLoaderBlock(0x%p)!\n",
        kernelModule.DllBase);

    goto exitToFirmware;
  }

  EFI_PHYSICAL_ADDRESS kernelBase = (EFI_PHYSICAL_ADDRESS)kernelModule.DllBase;
  UINTN                kernelSize = kernelModule.SizeOfImage;

  ContextPrint("OsKernel                  -> (virt) 0x%p\n", kernelBase);

  if (kernelBase != 0 && kernelSize != 0) {
    ContextPrint(
        "Patching OsKernel         -> (virt) 0x%p (size) 0x%p\n", kernelBase,
        kernelSize);

    // Fix up ntoskrnl.exe
    KernelErrataPatcherApplyReadACTLREL1Patches(kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyWriteACTLREL1Patches(kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyReadAMCNTENSET0EL0Patches(kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyWriteAMCNTENSET0EL0Patches(kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyPsciMemoryProtectionPatches(
        kernelBase, kernelSize, FALSE);
  }

exitToFirmware:
  // Switch back to firmware context before calling real ExitBootServices
  BlpArchSwitchContext(FirmwareContext);

exit:
  FirmwarePrint(
      "OslFwpKernelSetupPhase1   <- (phys) 0x%p\n", fwpKernelSetupPhase1);

  // Call the original
  return gBS->ExitBootServices(ImageHandle, MapKey);
}

EFI_STATUS
EFIAPI
KernelErrataPatcherEntryPoint(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EfiExitBootServices   = gBS->ExitBootServices;
  gBS->ExitBootServices = ExitBootServicesWrapper;

  return EFI_SUCCESS;
}