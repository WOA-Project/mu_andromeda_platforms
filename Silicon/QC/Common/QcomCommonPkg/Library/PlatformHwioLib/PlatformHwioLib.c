/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/HobLib.h>
#include <Library/PlatformHobs.h>
#include <Library/QcomMemoryMapHelperLib.h>

EFI_STATUS EFIAPI GetPlatformDisplayFrameBuffer(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("Display Reserved", MemoryDescriptor);
}

EFI_STATUS EFIAPI GetPlatformLogBuffer(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("Log Buffer", MemoryDescriptor);
}

EFI_STATUS EFIAPI GetPlatformPStore(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("PStore", MemoryDescriptor);
}

EFI_STATUS EFIAPI GetPlatformDXEHeap(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("DXE Heap", MemoryDescriptor);
}

EFI_STATUS EFIAPI GetPlatformUEFIStack(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("UEFI Stack", MemoryDescriptor);
}

EFI_STATUS EFIAPI GetPlatformUEFIFD(ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  return QcomLocateMemoryMapAreaByName("UEFI FD", MemoryDescriptor);
}