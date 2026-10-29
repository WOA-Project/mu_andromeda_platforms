/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _PLATFORM_HWIO_LIB_H_
#define _PLATFORM_HWIO_LIB_H_

#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/PlatformMemoryMapLib.h>

EFI_STATUS EFIAPI GetPlatformDisplayFrameBuffer(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);
EFI_STATUS EFIAPI GetPlatformLogBuffer(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);
EFI_STATUS EFIAPI GetPlatformPStore(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);
EFI_STATUS EFIAPI GetPlatformDXEHeap(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);
EFI_STATUS EFIAPI GetPlatformUEFIStack(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);
EFI_STATUS EFIAPI GetPlatformUEFIFD(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor);

#endif /* _PLATFORM_HWIO_LIB_H_ */