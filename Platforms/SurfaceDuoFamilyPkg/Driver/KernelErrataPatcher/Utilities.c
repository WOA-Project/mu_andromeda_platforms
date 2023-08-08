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

UINT64 GetExport(EFI_PHYSICAL_ADDRESS base, const CHAR8 *functionName)
{
  PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(base);
  if (dosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
    return 0;
  }

  PIMAGE_NT_HEADERS64 ntHeaders =
      (PIMAGE_NT_HEADERS64)(base + dosHeader->e_lfanew);

  UINT32 exportsRva =
      ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT]
          .VirtualAddress;
  if (!exportsRva) {
    return 0;
  }

  PIMAGE_EXPORT_DIRECTORY exports =
      (PIMAGE_EXPORT_DIRECTORY)(base + exportsRva);
  UINT32 *nameRva = (UINT32 *)(base + exports->AddressOfNames);

  for (UINT32 i = 0; i < exports->NumberOfNames; ++i) {
    CHAR8 *func = (CHAR8 *)(base + nameRva[i]);
    if (AsciiStrCmp(func, functionName) == 0) {
      UINT32 *funcRva    = (UINT32 *)(base + exports->AddressOfFunctions);
      UINT16 *ordinalRva = (UINT16 *)(base + exports->AddressOfNameOrdinals);

      return base + funcRva[ordinalRva[i]];
    }
  }

  return 0;
}

EFI_PHYSICAL_ADDRESS LocateWinloadBase(EFI_PHYSICAL_ADDRESS base, UINTN *size)
{
  if (base & (EFI_PAGE_SIZE - 1)) {
    base &= ~(EFI_PAGE_SIZE - 1);
    base += EFI_PAGE_SIZE;
  }

  do {
    if (*(UINT16 *)base == IMAGE_DOS_SIGNATURE) {
      UINT32               newBaseOffset = *(UINT32 *)(base + 0x3C);
      EFI_PHYSICAL_ADDRESS newBase       = base + newBaseOffset;

      if (*(UINT16 *)newBase == IMAGE_NT_SIGNATURE) {
        *size = *(UINT32 *)(newBase + 0x110);
        if (*size & (EFI_PAGE_SIZE - 1)) {
          *size &= ~(EFI_PAGE_SIZE - 1);
          *size += EFI_PAGE_SIZE;
        }
        break;
      }
    }

    base -= EFI_PAGE_SIZE;
  } while (TRUE);

  return base;
}

VOID CopyMemory(
    EFI_PHYSICAL_ADDRESS destination, EFI_PHYSICAL_ADDRESS source, UINTN size)
{
  UINT8 *dst = (UINT8 *)(destination);
  UINT8 *src = (UINT8 *)(source);
  for (UINTN i = 0; i < size; i++) {
    dst[i] = src[i];
  }
}

EFI_PHYSICAL_ADDRESS
FindPattern(EFI_PHYSICAL_ADDRESS baseAddress, UINTN size, const CHAR8 *pattern)
{
  EFI_PHYSICAL_ADDRESS firstMatch     = 0;
  const CHAR8         *currentPattern = pattern;

  for (EFI_PHYSICAL_ADDRESS current = baseAddress; current < baseAddress + size;
       current++) {
    UINT8 byte = currentPattern[0];
    if (!byte)
      return firstMatch;
    if (byte == '\?' ||
        *(UINT8 *)(current) == GET_BYTE(byte, currentPattern[1])) {
      if (!firstMatch)
        firstMatch = current;
      if (!currentPattern[2])
        return firstMatch;
      ((byte == '\?') ? (currentPattern += 2) : (currentPattern += 3));
    }
    else {
      currentPattern = pattern;
      firstMatch     = 0;
    }
  }

  return 0;
}