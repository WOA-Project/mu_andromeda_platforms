/** @file
  Surface Duo UEFI setup at PrePi lifecycle boundaries.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/ArmPlatformLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PrePiDisplayLib.h>
#include <Library/PrePiPlatformHooksLib.h>
#include <Library/PrePiUfsLib.h>
#include <Library/TimerLib.h>

STATIC
VOID
ShowUefiDisplayTest (
  VOID
  )
{
  EFI_STATUS Status;

  if (!FixedPcdGetBool (PcdPrePiDisplayTestImage)) {
    return;
  }

  Status = PlatformStartDisplay ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi display test initialization failed: %r\n", Status));
    return;
  }

  Status = PlatformDisplayTestImage ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi display test image failed: %r\n", Status));
  } else {
    MicroSecondDelay (3000000);
  }

  Status = PlatformStopDisplay ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi display test DMA handoff failed: %r\n", Status));
    CpuDeadLoop ();
  }
}

STATIC
VOID
PrepareUefiUfs (
  VOID
  )
{
  EFI_STATUS Status;

  if (!FixedPcdGetBool (PcdPrePiUfsReinit)) {
    return;
  }

  Status = PlatformReinitializeUfs ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi UFS initialization failed: %r\n", Status));
    CpuDeadLoop ();
  }

  Status = PlatformFinishUfs ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi UFS DMA handoff failed: %r\n", Status));
    CpuDeadLoop ();
  }
}

VOID
EFIAPI
PrePiExecutePriorToDxeCoreHandoff (
  VOID
  )
{
  RETURN_STATUS Status;

  ShowUefiDisplayTest ();
  PrepareUefiUfs ();

  // Initialize the platform specific controllers
  Status = ArmPlatformInitialize (0);
  if (RETURN_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi platform initialization failed: %r\n", Status));
    CpuDeadLoop ();
  }
}
