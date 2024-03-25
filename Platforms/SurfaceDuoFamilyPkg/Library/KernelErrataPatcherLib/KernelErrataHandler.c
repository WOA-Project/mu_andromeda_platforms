/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Shell Code to patch kernel mode components before NTOSKRNL

  Copyright (c) 2022-2023 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include "KernelErrataHandler.h"

typedef struct _UNICODE_STRING {
  UINT16  Length;
  UINT16  MaximumLength;
  CHAR16 *Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _KLDR_DATA_TABLE_ENTRY {
  LIST_ENTRY                    InLoadOrderLinks;
  VOID                         *ExceptionTable;
  UINT32                        ExceptionTableSize;
  VOID                         *GpValue;
  struct _NON_PAGED_DEBUG_INFO *NonPagedDebugInfo;
  VOID                         *DllBase;
  VOID                         *EntryPoint;
  UINT32                        SizeOfImage;
  UNICODE_STRING                FullDllName;
  UNICODE_STRING                BaseDllName;
  UINT32                        Flags;
  UINT16                        LoadCount;
  union {
    UINT16 SignatureLevel : 4;
    UINT16 SignatureType : 3;
    UINT16 Unused : 9;
    UINT16 EntireField;
  } u1;
  VOID  *SectionPointer;
  UINT32 CheckSum;
  UINT32 CoverageSectionSize;
  VOID  *CoverageSection;
  VOID  *LoadedImports;
  VOID  *Spare;
  UINT32 SizeOfImageNotRounded;
  UINT32 TimeDateStamp;
} KLDR_DATA_TABLE_ENTRY, *PKLDR_DATA_TABLE_ENTRY;

typedef struct _LOADER_PARAMETER_BLOCK {
  UINT32     OsMajorVersion;
  UINT32     OsMinorVersion;
  UINT32     Size;
  UINT32     OsLoaderSecurityVersion;
  LIST_ENTRY LoadOrderListHead;
  LIST_ENTRY MemoryDescriptorListHead;
  LIST_ENTRY BootDriverListHead;
  LIST_ENTRY EarlyLaunchListHead;
  LIST_ENTRY CoreDriverListHead;
  LIST_ENTRY CoreExtensionsDriverListHead;
  LIST_ENTRY TpmCoreDriverListHead;
} LOADER_PARAMETER_BLOCK, *PLOADER_PARAMETER_BLOCK;

#define CONTAINING_RECORD(address, type, field)                                \
  ((type *)((char *)(address) - (unsigned long long)(&((type *)0)->field)))

#define ARM64_INSTRUCTION_LENGTH 4
#define ARM64_TOTAL_INSTRUCTION_LENGTH(x) (ARM64_INSTRUCTION_LENGTH * x)

VOID PreOslArm64TransferToKernel(VOID *OsLoaderBlock, VOID *KernelAddress)
{
  PLOADER_PARAMETER_BLOCK loaderBlock = (PLOADER_PARAMETER_BLOCK)OsLoaderBlock;
  LIST_ENTRY            *entry = (&loaderBlock->LoadOrderListHead)->ForwardLink;
  PKLDR_DATA_TABLE_ENTRY kernelModule =
      CONTAINING_RECORD(entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

  EFI_PHYSICAL_ADDRESS base = (EFI_PHYSICAL_ADDRESS)kernelModule->DllBase;
  UINTN                size = kernelModule->SizeOfImage;

  for (EFI_PHYSICAL_ADDRESS current = base; current < base + size;
       current += sizeof(UINT32)) {
    if (*(UINT32 *)current == 0xD5381028 || // mrs x8, actlr_el1
        *(UINT32 *)current == 0xD53BD2A8) { // mrs x8,
                                            // amcntenset0_el0
      *(UINT32 *)current = 0xD2800008;      // movz x8, #0
    }
    else if (
        *(UINT32 *)current == 0xD5181028 || // msr actlr_el1, x8
        *(UINT32 *)current == 0xD5181029) { // msr actlr_el1, x9
      *(UINT32 *)current = 0xD503201F;      // nop
    }
    else if (
        *(UINT64 *)current == 0xD2800003180002D5 &&
        *(UINT64 *)(current + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) ==
            0xD2800001D2800002) { // ldr w21, #0x58 - movz x3, #0 - movz x2,
                                  // #0 - movz x1, #0
      *(UINT32 *)(current - ARM64_TOTAL_INSTRUCTION_LENGTH(8)) =
          0xD65F03C0; // ret
    }
    else if (
        *(UINT64 *)current == 0xD2800002D2800003 &&
        (*(UINT64 *)(current + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) ==
             0x18000240D2800001 ||
         *(UINT64 *)(current + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) ==
             0x180002C0D2800001)) { // movz x3, #0 - movz x2, #0 - movz x1, #0
                                    // - (ldr w0, #0x48 || ldr w0, #0x58)
      *(UINT32 *)(current - ARM64_TOTAL_INSTRUCTION_LENGTH(7)) =
          0xD65F03C0; // ret
    }
  }
}