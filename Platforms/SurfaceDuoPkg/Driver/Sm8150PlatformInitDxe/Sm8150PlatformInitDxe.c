/** @file
 *
 *  Copyright (c) 2018, Linaro Ltd. All rights reserved.
 *
 *  This program and the accompanying materials
 *  are licensed and made available under the terms and conditions of the BSD
 *License which accompanies this distribution.  The full text of the license may
 *be found at http://opensource.org/licenses/bsd-license.php
 *
 *  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR
 *IMPLIED.
 *
 **/

#include <Guid/EventGroup.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/EmbeddedGpio.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PlatformBootManager.h>

EFI_STATUS
EFIAPI
Sm8150PlatformInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS             Status;
  EFI_CPU_ARCH_PROTOCOL *gCpu;

  DEBUG((EFI_D_INFO, "Disabling Qualcomm Watchdog Reboot timer\n"));
  MmioWrite32(0x17C10008, 0x00000000);
  DEBUG((EFI_D_INFO, "Qualcomm Watchdog Reboot timer disabled\n"));

  Status = gBS->LocateProtocol (&gEfiCpuArchProtocolGuid, NULL, (VOID **)&gCpu);
  ASSERT_EFI_ERROR(Status);

  // Set Framebuffer memory region attributes manually to workaround a graphical issue
  Status = gCpu->SetMemoryAttributes(
      gCpu, 0x9c000000, 0x2400000, EFI_MEMORY_WC | EFI_MEMORY_XP);
  ASSERT_EFI_ERROR(Status);

  // Set SMEM region attributes to match WP counterparts (i.e. clear execution protection)
  Status = gCpu->SetMemoryAttributes(
      gCpu, 0x86000000, 0x00200000, EFI_MEMORY_WC | EFI_MEMORY_UC);
  ASSERT_EFI_ERROR(Status);

  return Status;
}
