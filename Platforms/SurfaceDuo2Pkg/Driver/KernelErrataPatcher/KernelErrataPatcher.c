/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Based on https://github.com/SamuelTulach/rainbow

  Copyright (c) 2021 Samuel Tulach
  Copyright (c) 2022 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include "KernelErrataPatcher.h"

#define SILENT 1

STATIC BL_ARCH_SWITCH_CONTEXT BlpArchSwitchContext = NULL;
STATIC EFI_EXIT_BOOT_SERVICES EfiExitBootServices  = NULL;

#if SILENT == 0
#define FirmwarePrint(x, ...) Print(x, __VA_ARGS__);
#define ContextPrint(x, ...)                                                   \
  BlpArchSwitchContext(FirmwareContext);                                       \
  FirmwarePrint(x, __VA_ARGS__);                                               \
  BlpArchSwitchContext(ApplicationContext);
#else
#define FirmwarePrint(x, ...)
#define ContextPrint(x, ...)
#endif

VOID KernelErrataPatcherApplyPatches(
    COPY_TO CopyTo, VOID *Base, UINTN Size, BOOLEAN PatchWrites,
    BOOLEAN IsInFirmwareContext)
{
  // Fix up #0 (28 10 38 D5 -> E8 7F 40 B2) (ACTRL_EL1 Register Read)
  UINT8  FixedInstruction0[] = {0xE8, 0x7F, 0x40, 0xB2};
  UINT64 IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");

  while (IllegalInstruction0 != 0) {
    if (IsInFirmwareContext) {
      FirmwarePrint(
          L"mrs x8, actlr_el1         -> (phys) 0x%p\n", IllegalInstruction0);
    }
    else {
      ContextPrint(
          L"mrs x8, actlr_el1         -> (phys) 0x%p\n", IllegalInstruction0);
    }

    CopyTo(
        (UINT64 *)IllegalInstruction0, FixedInstruction0,
        sizeof(FixedInstruction0));

    IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
  }

  if (PatchWrites) {
    // Fix up #1 (29 10 18 D5 -> 1F 20 03 D5) (ACTRL_EL1 Register Write)
    UINT8  FixedInstruction1[] = {0x1F, 0x20, 0x03, 0xD5};
    UINT64 IllegalInstruction1 = FindPattern(Base, Size, "29 10 18 D5");

    while (IllegalInstruction1 != 0) {
      if (IsInFirmwareContext) {
        FirmwarePrint(
            L"msr actlr_el1, x9         -> (phys) 0x%p\n", IllegalInstruction1);
      }
      else {
        ContextPrint(
            L"msr actlr_el1, x9         -> (phys) 0x%p\n", IllegalInstruction1);
      }

      CopyTo(
          (UINT64 *)IllegalInstruction1, FixedInstruction1,
          sizeof(FixedInstruction1));

      IllegalInstruction1 = FindPattern(Base, Size, "29 10 18 D5");
    }
  }
}

EFI_STATUS
EFIAPI
KernelErrataPatcherExitBootServices(
    EFI_HANDLE ImageHandle, UINTN MapKey, PLOADER_PARAMETER_BLOCK loaderBlock,
    UINTN returnAddress)
{
  // Might be called multiple times by winload in a loop failing few times
  gBS->ExitBootServices = EfiExitBootServices;

  if (loaderBlock == NULL) {
    FirmwarePrint(
        L"Failed to find OslLoaderBlock! loaderBlock -> 0x%p\n", loaderBlock);
    goto exit;
  }

  // BlpArchSwitchContext
  BlpArchSwitchContext =
      (BL_ARCH_SWITCH_CONTEXT)(FindPattern((VOID *)returnAddress, SCAN_MAX, "00 06 80 52 60 00 3E D4") - ARM64_TOTAL_INSTRUCTION_LENGTH(17));

  if (!BlpArchSwitchContext ||
      ((UINTN)BlpArchSwitchContext & 0xFFFFFFF000000000) != 0) {
    FirmwarePrint(
        L"Failed to find BlpArchSwitchContext! BlpArchSwitchContext -> 0x%p\n",
        BlpArchSwitchContext);
    goto exit;
  }

  FirmwarePrint(L"OslFwpKernelSetupPhase1   -> (phys) 0x%p\n", returnAddress);
  FirmwarePrint(L"OslLoaderBlock            -> (virt) 0x%p\n", loaderBlock);
  FirmwarePrint(
      L"BlpArchSwitchContext      -> (phys) 0x%p\n", BlpArchSwitchContext);

  // Fix up winload.efi
  // This fixes Boot Debugger
  FirmwarePrint(
      L"Patching OsLoader         -> (phys) 0x%p (size) 0x%p\n", returnAddress,
      SCAN_MAX);

  KernelErrataPatcherApplyPatches(
      CopyMemory, (VOID *)(returnAddress), SCAN_MAX, FALSE, TRUE);

  /*
   * Switch context to (as defined by winload) application context
   * Within this context only the virtual addresses are valid
   * Real/physical addressing is not used
   * We can not use any EFI services unless we switch back!
   * To print on screen use ContextPrint define
   */
  BlpArchSwitchContext(ApplicationContext);

  ContextPrint(
      L"LOADER_PARAMETER_BLOCK    -> OsMajorVersion: %d OsMinorVersion: %d "
      L"Size: %d\n",
      loaderBlock->OsMajorVersion, loaderBlock->OsMinorVersion,
      loaderBlock->Size);

  KLDR_DATA_TABLE_ENTRY kernelModule =
      *GetModule(&loaderBlock->LoadOrderListHead, NT_OS_KERNEL_IMAGE_NAME);

  if (kernelModule.DllBase == NULL) {
    ContextPrint(
        L"Failed to find ntoskrnl.exe in OslLoaderBlock(0x%p)!\n",
        kernelModule.DllBase);

    goto exitToFirmware;
  }

  UINTN kernelBase = (UINTN)kernelModule.DllBase;
  UINTN kernelSize = kernelModule.SizeOfImage;

  ContextPrint(L"OsKernel                  -> (virt) 0x%p\n", kernelBase);

  if (kernelBase != 0 && kernelSize != 0) {
    ContextPrint(
        L"Patching OsKernel         -> (phys) 0x%p (size) 0x%p\n", kernelBase,
        kernelSize);

    // Fix up ntoskrnl.exe
    KernelErrataPatcherApplyPatches(
        CopyToReadOnly, (VOID *)kernelBase, kernelSize, TRUE, FALSE);
  }

exitToFirmware:
  // Switch back to firmware context before calling real ExitBootServices
  BlpArchSwitchContext(FirmwareContext);

exit:
  FirmwarePrint(L"OslFwpKernelSetupPhase1   <- (phys) 0x%p\n", returnAddress);

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

EFI_STATUS
EFIAPI
KernelErrataPatcherUnloadImage(IN EFI_HANDLE ImageHandle)
{
  return EFI_ACCESS_DENIED;
}