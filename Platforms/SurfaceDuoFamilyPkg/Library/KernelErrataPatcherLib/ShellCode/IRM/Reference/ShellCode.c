/** @file

  Patches NTOSKRNL to not cause a SError when reading/writing ACTLR_EL1
  Patches NTOSKRNL to not cause a SError when reading/writing AMCNTENSET0_EL0
  Patches NTOSKRNL to not cause a bugcheck when attempting to use
  PSCI_MEMPROTECT Due to an issue in QHEE

  Shell Code to patch kernel mode components before NTOSKRNL

  Copyright (c) 2022-2023 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

/*
 * All sorts of defines related to Windows kernel
 * https://www.vergiliusproject.com/
 * Windows SDK/DDK
 */

typedef unsigned long long UINT64;
typedef long long          INT64;
typedef unsigned int       UINT32;
typedef int                INT32;
typedef unsigned short     UINT16;
typedef unsigned short     CHAR16;
typedef short              INT16;
typedef unsigned char      BOOLEAN;
typedef unsigned char      UINT8;
typedef char               CHAR8;
typedef signed char        INT8;

///
/// Unsigned value of native width.  (4 bytes on supported 32-bit processor
/// instructions, 8 bytes on supported 64-bit processor instructions)
///
typedef UINT64 UINTN;

///
/// Signed value of native width.  (4 bytes on supported 32-bit processor
/// instructions, 8 bytes on supported 64-bit processor instructions)
///
typedef INT64 INTN;

///
/// Datum is read-only.
///
#define CONST const

///
/// Datum is scoped to the current file or function.
///
#define STATIC static

///
/// Undeclared type.
///
#define VOID void

//
// Modifiers for Data Types used to self document code.
// This concept is borrowed for UEFI specification.
//

///
/// Datum is passed to the function.
///
#define IN

///
/// Datum is returned from the function.
///
#define OUT

///
/// Passing the datum to the function is optional, and a NULL
/// is passed if the value is not supplied.
///
#define OPTIONAL

//
// 8-bytes unsigned value that represents a physical system address.
//
typedef UINT64 PHYSICAL_ADDRESS;

///
/// LIST_ENTRY structure definition.
///
typedef struct _LIST_ENTRY LIST_ENTRY;

typedef UINT64 EFI_PHYSICAL_ADDRESS;

///
/// _LIST_ENTRY structure definition.
///
struct _LIST_ENTRY {
  LIST_ENTRY *ForwardLink;
  LIST_ENTRY *BackLink;
};

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

typedef void (*NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL)(
    VOID *OsLoaderBlock, VOID *KernelAddress);

VOID DoSomething(VOID *OsLoaderBlock, VOID *KernelAddress)
{
  ((NT_OS_LOADER_ARM64_TRANSFER_TO_KERNEL)KernelAddress)(
      OsLoaderBlock, KernelAddress);
}

VOID PreOslArm64TransferToKernel(VOID *OsLoaderBlock, VOID *KernelAddress)
{
  PLOADER_PARAMETER_BLOCK loaderBlock = (PLOADER_PARAMETER_BLOCK)OsLoaderBlock;

  for (LIST_ENTRY *entry = (&loaderBlock->LoadOrderListHead)->ForwardLink;
       entry != (&loaderBlock->LoadOrderListHead); entry = entry->ForwardLink) {

    PKLDR_DATA_TABLE_ENTRY kernelModule =
        CONTAINING_RECORD(entry, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

    EFI_PHYSICAL_ADDRESS base = (EFI_PHYSICAL_ADDRESS)kernelModule->DllBase;
    UINTN                size = kernelModule->SizeOfImage;

    for (EFI_PHYSICAL_ADDRESS current = base; current < base + size;
         current += sizeof(UINT32)) {
      if (*(UINT32 *)current == 0xD518CBAA) { // msr icc_sgi1r_el1, x10
        // The offending code starts 2 instructions above us
        // ie:
        // 
        // AND             X8, X0, #0xF            // <--------- this is the IRQ being AND with 0xF
        //                                         // (honestly, this doesn't look needed as we check prior if it's > 0xF...)
        // We want to patch starting from here:
        //
        // ORR             X8, X8, #0x10000        // <--------- this is the IRM bit being ORR with the IRQ (but shifted by 0x18 to the left)
        // LSL             X10, X8, #0x18          // <--------- this is the SGI value being shifted by 0x18 to the right
        // 
        // MSR             MPIDR_EL1, X10          // <--------- we are here
        //
        // we have to inject code into unused routines to fix this
        //
        // x8 contains part of the sgir register value for the requested irq already
        // it was setup before us with:
        // and   x8, x0, #0xf
        //
        // we have to shift it now, without the irm bit
        // lsl x10, x8, #0x18
        // then we have to add in the aff1 bits
        // aff1 is by spec, specified in bits 16 to 23
        // cpu0 has aff1=0, cpu1 has aff1=1, etc. up to 7
        // we can get the aff1 bit of the current cpu from the mpidr_el1 register
        // and then shift it into the correct position by 8 bits
        // then we iterate through all the cpus and send the sgi to them
        // to do this, we iterate from 0 to 7, skip the current cpu, and for each iteration,
        // we can skip the current cpu by comparing the aff1 bit of the current cpu with the aff1 bit of the cpu we're sending the sgi to
        // if they match, we skip the current cpu
        // if they don't match, we add the aff1 bit to the sgir register value and send the sgi

        // The actual code we need to inject into unused routines:
        //
        // START OF CODE (12 instructions)
        //
        //  lsl   x10, x8, #0x18    // shift the irq number into the correct position
        //  mrs   x8, mpidr_el1     // get the mpidr_el1 register of the current cpu
        //  mov   x11, #0x0         // setup a counter
        //
        //  .loop:
        //  cmp   x11, x8           // compare the aff1 bit of the current cpu with the aff1 bit of the cpu we're sending the sgi to
        //  b.eq  .skip             // if they match, skip the current cpu
        //
        //  lsl   x9, x11, #0x8     // shift the aff1 bit into the correct position
        //  orr   x9, x9, x10       // add the aff1 bit to the sgir register value
        //
        //  msr   icc_sgi1r_el1, x9 // send the sgi
        //  dsb   sy                // ensure the sgi is sent
        //  
        //  .skip:
        //  add   x11, x11, #0x100  // increment the counter
        //  cmp   x11, #0x800       // check if we've iterated through all cpus (max: 8)
        //  b.ne  .loop             // if we have not, loop
        //
        // END OF CODE

        // This only works with specific kernel versions I know...
        // Needs to be improved obviously...

        *(UINT32 *)(current - ARM64_TOTAL_INSTRUCTION_LENGTH(2)) = 0x1400001A; //(0x14000000 | (26 & 0x7FFFFFF));

        EFI_PHYSICAL_ADDRESS patch = current + ARM64_TOTAL_INSTRUCTION_LENGTH(24);

        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(0)) = 0xD3689D0A;  // lsl x10, x8, #0x18
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(1)) = 0xD53800A8;  // mrs x8, mpidr_el1
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(2)) = 0xD280000B;  // movz x11, #0
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(3)) = 0xEB08017F;  // cmp x11, x8
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(4)) = 0x540000A0;  // b.eq #0x24
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(5)) = 0xD378DD69;  // lsl x9, x11, #8
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(6)) = 0xAA0A0129;  // orr x9, x9, x10
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(7)) = 0xD518CBA9;  // msr icc_sgi1r_el1, x9
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(8)) = 0xD5033F9F;  // dsb sy
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(9)) = 0x9104016B;  // add x11, x11, #0x100
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(10)) = 0xF120017F; // cmp x11, #0x800
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(11)) = 0x54FFFF01; // b.ne #0xc

        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(12)) = 0xD503201F; // nop
        *(UINT32 *)(patch + ARM64_TOTAL_INSTRUCTION_LENGTH(13)) = 0xD503201F; // nop
      }
    }
  }

  DoSomething(OsLoaderBlock, KernelAddress);
}