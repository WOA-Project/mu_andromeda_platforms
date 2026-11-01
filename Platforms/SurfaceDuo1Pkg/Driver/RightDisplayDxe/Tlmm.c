/** @file
  Minimal TLMM GPIO helper.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

//
// The SM8150 TLMM is split into tiles; each pin's registers live in its tile at
// tile_base + 0x1000 * gpio. Only the pins this driver uses are mapped.
//
STATIC
UINTN
TlmmCfgAddr (
  IN UINT32 Gpio
  )
{
  UINT64 Tile;

  switch (Gpio) {
    case PANEL_RESET_GPIO:
      Tile = TLMM_SOUTH_BASE;
      break;
    case PANEL_TE_GPIO:
      Tile = TLMM_NORTH_BASE;
      break;
    default:
      ASSERT (FALSE);
      Tile = TLMM_NORTH_BASE;
      break;
  }

  return (UINTN)(Tile + 0x1000ULL * Gpio);
}

VOID
TlmmSetGpio (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  )
{
  MmioWrite32 (TlmmCfgAddr (Gpio) + 0x4, Level ? TLMM_IN_OUT_OUT : 0);
}

VOID
TlmmConfigGpioOutput (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  )
{
  // Function 0 (GPIO), no pull, minimum drive strength, output enabled
  TlmmSetGpio (Gpio, Level);
  MmioWrite32 (TlmmCfgAddr (Gpio), TLMM_CFG_GPIO_OE);
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
    TlmmCfgAddr (Gpio),
    (Pull & 3) | ((Function & 0xF) << 2) | ((DriveStrength & 7) << 6));
}
