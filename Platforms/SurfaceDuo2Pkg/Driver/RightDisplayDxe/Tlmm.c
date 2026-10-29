/** @file
  Minimal TLMM GPIO helper.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#define TLMM_GPIO_CFG(n)    (TLMM_BASE + 0x1000ULL * (n))
#define TLMM_GPIO_IN_OUT(n) (TLMM_BASE + 0x1000ULL * (n) + 0x4)

#define TLMM_CFG_GPIO_OE BIT9
#define TLMM_IN_OUT_OUT  BIT1

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
