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

/*VOID QGicCpuLateConfig()
{
  for (UINTN i = 0; i < FixedPcdGet32(PcdCoreCount); i++) {
    UINT32 GICR = FixedPcdGet32(PcdGicRedistributorsBase) + i * 0x20000;

    MmioWrite32(GICR + 0x10280, 0x08000000);
    MmioWrite32(GICR + 0x10180, 0);

    // Keep the first CPU GIC Distributor awake (unlike the stock firmware)
    if (i != 0) {
      MmioWrite32(GICR + 0x0014, 6);
    }
  }

  MmioWrite32(FixedPcdGet32(PcdGicDistributorBase), 0x53);
}*/

VOID KernelErrataPatcherApplyReadACTLREL1Patches(
    COPY_TO CopyTo, VOID *Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #0 (28 10 38 D5 -> E8 7F 40 B2) (ACTRL_EL1 Register Read)
  UINT8  FixedInstruction0[] = {0xE8, 0x7F, 0x40, 0xB2};
  UINT64 IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
  UINT8  PatchCounter        = 0;

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

    // Commenting out for boot speed optimization purposes, as there's only
    // two read occurences really in the kernel and one in winload

    // IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
    if (IsInFirmwareContext) {
      IllegalInstruction0 = 0;
    }
    else {
      PatchCounter++;
      if (PatchCounter == 1) {
        IllegalInstruction0 = FindPattern(Base, Size, "28 10 38 D5");
      }
      else {
        IllegalInstruction0 = 0;
      }
    }
  }
}

VOID KernelErrataPatcherApplyWriteACTLREL1Patches(
    COPY_TO CopyTo, VOID *Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
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

    // Commenting out for boot speed optimization purposes, as there's only
    // one write occurence really in the kernel

    // IllegalInstruction1 = FindPattern(Base, Size, "29 10 18 D5");
    IllegalInstruction1 = 0;
  }
}

VOID KernelErrataPatcherApplyIncoherentCacheConfigurationPatches(
    COPY_TO CopyTo, VOID *Base, UINTN Size, BOOLEAN IsInFirmwareContext)
{
  // Fix up #3 (KiCacheInitialize (Bugcheck call (first)) -> 1F 20 03 D5)
  // (KiCacheInitialize (Bugcheck call (first)) -> NOP)
  UINT8  NopInstruction[] = {0x1F, 0x20, 0x03, 0xD5, 0x1F, 0x20, 0x03, 0xD5,
                             0x1F, 0x20, 0x03, 0xD5, 0x1F, 0x20, 0x03, 0xD5};
  UINT64 KiCacheInitializeBC1 =
      FindPattern(Base, Size, "04 00 80 D2 03 00 80 D2 C0 07 80 52");

  if (KiCacheInitializeBC1 != 0) {
    FirmwarePrint(
        L"KiCacheInitialize/BC#1    -> (phys) 0x%p\n", KiCacheInitializeBC1);

    CopyToReadOnly(
        (UINT64 *)KiCacheInitializeBC1, NopInstruction, sizeof(NopInstruction));
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
      (BL_ARCH_SWITCH_CONTEXT)(FindPattern((VOID *)returnAddress, SCAN_MAX, "1F 04 00 71 33 11 88 9A 28 00 40 B9 1F 01 00 6B") - ARM64_TOTAL_INSTRUCTION_LENGTH(9));

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

  KernelErrataPatcherApplyReadACTLREL1Patches(
      CopyMemory, (VOID *)(returnAddress), SCAN_MAX, TRUE);

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
    KernelErrataPatcherApplyReadACTLREL1Patches(
        CopyToReadOnly, (VOID *)kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyWriteACTLREL1Patches(
        CopyToReadOnly, (VOID *)kernelBase, kernelSize, FALSE);
    KernelErrataPatcherApplyIncoherentCacheConfigurationPatches(
        CopyToReadOnly, (VOID *)kernelBase, kernelSize, FALSE);
  }

exitToFirmware:
  // Switch back to firmware context before calling real ExitBootServices
  BlpArchSwitchContext(FirmwareContext);

exit:
  // Reconfigure the GIC as it was before entry of our firmware
  // QGicCpuLateConfig();

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