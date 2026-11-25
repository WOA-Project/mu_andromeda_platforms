/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <Library/BaseLib.h>
#include <Library/AndromedaArmPlatformLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/SerialPortLib.h>

#include <Library/PlatformHwioLib.h>

#include "InitializationUtils.h"

VOID InitializeSharedUartBuffers(VOID)
{
#if USE_SCREEN_FOR_SERIAL_OUTPUT == 1
  ARM_MEMORY_REGION_DESCRIPTOR_EX DisplayMemoryRegion;
  GetPlatformDisplayFrameBuffer(&DisplayMemoryRegion);

  // Setup Position counter
  INTN *pFbConPosition =
      (INTN *)(DisplayMemoryRegion.Address + GetPlatformDisplayConsoleOffset());

  *(pFbConPosition + 0) = 0;
  *(pFbConPosition + 1) = 0;
#endif

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  ARM_MEMORY_REGION_DESCRIPTOR_EX PStoreMemoryRegion;
  GetPlatformPStore(&PStoreMemoryRegion);

  // Clear PStore area
  UINT8 *base = (UINT8 *)PStoreMemoryRegion.Address;
  for (UINTN i = 0; i < PStoreMemoryRegion.Length; i++) {
    base[i] = 0;
  }
#endif
}

VOID UartInit(VOID)
{
  SerialPortInitialize();
  InitializeSharedUartBuffers();

  DEBUG((EFI_D_INFO, "\nProject Mu on Surface Duo (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID EFIAPI ArmPlatformInitializeConsole(VOID)
{
  // Initialize UART Serial
  UartInit();
}