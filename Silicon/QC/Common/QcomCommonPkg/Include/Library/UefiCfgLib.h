/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _UEFI_CFG_LIB_H_
#define _UEFI_CFG_LIB_H_

EFI_STATUS EFIAPI GetMemInfoByAddress(UINT64 RegionBaseAddress, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions);
EFI_STATUS EFIAPI GetMemInfoByName(CHAR8 *RegionName, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions);
EFI_STATUS GetCfgInfoString(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize);
EFI_STATUS GetCfgInfoVal(CHAR8 *Key, UINT32 *Value);
EFI_STATUS GetCfgInfoVal64(CHAR8 *Key, UINT64 *Value);

#endif /* _UEFI_CFG_LIB_H_ */