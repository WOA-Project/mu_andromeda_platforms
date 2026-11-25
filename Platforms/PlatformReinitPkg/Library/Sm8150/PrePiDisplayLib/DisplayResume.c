/** @file
  SM8150 early display initialization and DXE handoff.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/PrePiDisplayLib.h>
#include <Library/QcomUefiCfgMemoryMapHelperLib.h>

#include "DisplayResumeConfig.h"
#include "DisplayResumeDpu.h"
#include "DisplayResumeScanout.h"

EFI_STATUS
DisplayResumePower (
  VOID
  );

EFI_STATUS
DisplayResumePowerDown (
  VOID
  );

STATIC BOOLEAN  mDisplayStarted;

EFI_STATUS
PlatformStartDisplay (
  VOID
  )
{
  EFI_STATUS                       Status;
  EFI_STATUS                       CleanupStatus;
  ARM_MEMORY_REGION_DESCRIPTOR_EX  Region;

  if (mDisplayStarted) {
    return EFI_ALREADY_STARTED;
  }

  if (FixedPcdGetBool (PcdPrePiSecondaryDisplay)) {
    Status = QcomLocateMemoryMapAreaByName ("Display Reserved", &Region);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "DisplayResume: display reservation not found: %r\n", Status));
      return Status;
    }

    if ((Region.Address != DISPLAY_FRAMEBUFFER_BASE) ||
        (Region.Length < (2U * DISPLAY_FRAMEBUFFER_SIZE + 2U * sizeof (INTN))) ||
        (FixedPcdGet32 (PcdMipiFrameBufferWidth) != DISPLAY_FRAMEBUFFER_WIDTH) ||
        (FixedPcdGet32 (PcdMipiFrameBufferHeight) != DISPLAY_FRAMEBUFFER_HEIGHT) ||
        (FixedPcdGet32 (PcdMipiFrameBufferPixelBpp) != 32)) {
      DEBUG ((DEBUG_ERROR, "DisplayResume: incompatible dual framebuffer reservation\n"));
      return EFI_BAD_BUFFER_SIZE;
    }
  }

  Status = DisplayResumePower ();
  if (EFI_ERROR (Status)) {
    CleanupStatus = DisplayResumePowerDown ();
    if (EFI_ERROR (CleanupStatus)) {
      DEBUG ((DEBUG_ERROR, "DisplayResume: power cleanup failed: %r\n", CleanupStatus));
      CpuDeadLoop ();
    }

    return Status;
  }

  Status = DisplayResumeDpuInit ();
  if (!EFI_ERROR (Status)) {
    Status = DisplayResumeDsiInit (FALSE);
  }

  if (!EFI_ERROR (Status)) {
    WriteBackDataCacheRange (
      (VOID *)(UINTN)DISPLAY_FRAMEBUFFER_BASE,
      DISPLAY_FRAMEBUFFER_SIZE * (FixedPcdGetBool (PcdPrePiSecondaryDisplay) ? 2U : 1U)
      );
    MemoryFence ();
    Status = DisplayResumeDpuStart ();
    if (!EFI_ERROR (Status)) {
      mDisplayStarted = TRUE;
      Status = DisplayResumeDsiPanelInit ();
    }
  }

  if (!EFI_ERROR (Status) && FixedPcdGetBool (PcdPrePiSecondaryDisplay)) {
    Status = DisplayResumeDsiInit (TRUE);
    if (!EFI_ERROR (Status)) {
      Status = DisplayResumeDpuStartSecondary ();
    }

    if (!EFI_ERROR (Status)) {
      Status = DisplayResumeDsiPanelInit ();
    }
  }

  if (EFI_ERROR (Status)) {
    if (DisplayResumeDsiCommandPending ()) {
      DEBUG ((DEBUG_ERROR, "DisplayResume: command DMA remains pending; cannot hand off to DXE\n"));
      CpuDeadLoop ();
    }

    if (mDisplayStarted) {
      CleanupStatus = PlatformStopDisplay ();
    } else {
      CleanupStatus = DisplayResumeDsiClose ();
      if (!EFI_ERROR (CleanupStatus)) {
        CleanupStatus = DisplayResumePowerDown ();
      }
    }
    if (EFI_ERROR (CleanupStatus)) {
      DEBUG ((DEBUG_ERROR, "DisplayResume: failed to stop or unmap DMA: %r\n", CleanupStatus));
      CpuDeadLoop ();
    }

    return Status;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PlatformDisplayTestImage (
  VOID
  )
{
  STATIC CONST UINT32  Colors[] = {
    0x00FFFFFF, 0x00FFFF00, 0x0000FFFF, 0x0000FF00,
    0x00FF00FF, 0x00FF0000, 0x000000FF, 0x00000000
  };
  UINT32  *Framebuffer;
  UINT32  *SecondaryFramebuffer;
  UINT32  X;
  UINT32  Y;

  if (!mDisplayStarted) {
    return EFI_NOT_STARTED;
  }

  Framebuffer = (UINT32 *)(UINTN)DISPLAY_FRAMEBUFFER_BASE;
  SecondaryFramebuffer = (UINT32 *)(UINTN)DISPLAY_SECONDARY_FRAMEBUFFER;
  for (Y = 0; Y < DISPLAY_FRAMEBUFFER_HEIGHT; Y++) {
    for (X = 0; X < DISPLAY_FRAMEBUFFER_WIDTH; X++) {
      Framebuffer[Y * DISPLAY_FRAMEBUFFER_WIDTH + X] =
        Colors[(X * ARRAY_SIZE (Colors)) / DISPLAY_FRAMEBUFFER_WIDTH];
      if (FixedPcdGetBool (PcdPrePiSecondaryDisplay)) {
        SecondaryFramebuffer[Y * DISPLAY_FRAMEBUFFER_WIDTH + X] =
          Colors[(Y * ARRAY_SIZE (Colors)) / DISPLAY_FRAMEBUFFER_HEIGHT];
      }
    }
  }

  WriteBackDataCacheRange (
    Framebuffer,
    DISPLAY_FRAMEBUFFER_SIZE * (FixedPcdGetBool (PcdPrePiSecondaryDisplay) ? 2U : 1U)
    );
  MemoryFence ();
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformStopDisplay (
  VOID
  )
{
  EFI_STATUS  Status;

  if (!mDisplayStarted) {
    return EFI_NOT_STARTED;
  }

  Status = DisplayResumeDsiPanelShutdown ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplayResumeDpuStop ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplayResumeDsiClose ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplayResumeDpuRelease ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = DisplayResumePowerDown ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDisplayStarted = FALSE;
  return EFI_SUCCESS;
}
