/** @file
  Minimal TLMM GPIO helper.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

VOID
TlmmSetGpio (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  )
{
  MmioWrite32 ((UINTN)TLMM_GPIO_IN_OUT (Gpio), Level ? TLMM_IN_OUT_OUT : 0);
}

VOID
TlmmConfigGpioOutput (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  )
{
  // Function 0 (GPIO), no pull, minimum drive strength, output enabled
  TlmmSetGpio (Gpio, Level);
  MmioWrite32 ((UINTN)TLMM_GPIO_CFG (Gpio), TLMM_CFG_GPIO_OE);
}

VOID
TlmmConfigFunction (
  IN UINT32 Gpio,
  IN UINT32 Function,
  IN UINT32 Pull,
  IN UINT32 DriveStrength
  )
{
  // Pull in bits 0-1, function in bits 2-5, drive strength in bits 6-8, OE off
  MmioWrite32 (
    (UINTN)TLMM_GPIO_CFG (Gpio),
    (Pull & 3) | ((Function & 0xF) << 2) | ((DriveStrength & 7) << 6));
}
