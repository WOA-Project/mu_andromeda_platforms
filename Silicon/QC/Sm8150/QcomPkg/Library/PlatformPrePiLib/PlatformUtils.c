/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <PiPei.h>

#include <Library/IoLib.h>
#include <Library/MemoryMapHelperLib.h>
#include <Library/PlatformPrePiLib.h>
#include <Library/QcomMmuDetachHelperLib.h>

#include "PlatformUtils.h"

VOID DisableMDSSDSIController(UINT32 MdssDsiBase)
{
  UINT32 DsiControlAddr  = MdssDsiBase + DSI_CTRL;
  UINT32 DsiControlValue = MmioRead32(DsiControlAddr);
  DsiControlValue &=
      ~(DSI_CTRL_ENABLE | DSI_CTRL_VIDEO_MODE_ENABLE |
        DSI_CTRL_COMMAND_MODE_ENABLE);
  MmioWrite32(DsiControlAddr, DsiControlValue);
}

VOID SetWatchdogState(BOOLEAN Enable)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX WDTMemoryRegion;
  LocateMemoryMapAreaByName("APSS_WDT_TMR1", &WDTMemoryRegion);

  MmioWrite32(WDTMemoryRegion.Address + APSS_WDT_ENABLE_OFFSET, Enable);
}

VOID PlatformInitialize(VOID)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX MDSSMemoryRegion;
  LocateMemoryMapAreaByName("MDSS", &MDSSMemoryRegion);

  // Disable MDSS DSI0 Controller
  DisableMDSSDSIController(MDSSMemoryRegion.Address + MDSS_DSI0);

  // Disable MDSS DSI1 Controller
  DisableMDSSDSIController(MDSSMemoryRegion.Address + MDSS_DSI1);

  // Disable WatchDog Timer
  SetWatchdogState(FALSE);

  MmuDetach();
}