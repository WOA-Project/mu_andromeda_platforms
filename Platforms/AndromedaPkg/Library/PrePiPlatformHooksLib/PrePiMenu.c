/** @file
  Surface Duo menu at the PrePi lifecycle boundary.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformPrePiLib.h>
#include <Library/PrePiBootMenuLib.h>
#include <Library/PrePiDisplayLib.h>
#include <Library/PrePiPlatformHooksLib.h>
#include <Library/PrePiUfsLib.h>

VOID
EFIAPI
PrePiShowBootMenu (
  VOID
  )
{
  EFI_STATUS Status;

  if (!FixedPcdGetBool (PcdPrePiBootMenu)) {
    return;
  }

  Status = PlatformStartDisplay ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi boot menu display initialization failed: %r\n", Status));
    return;
  }

  Status = PlatformPrepareInheritedUfs ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi menu UFS preparation failed: %r\n", Status));
    CpuDeadLoop ();
  }

  Status = PrePiBootMenuShow ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi boot menu failed: %r\n", Status));
  }

  Status = PlatformFinishUfs ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi menu UFS cleanup failed: %r\n", Status));
    CpuDeadLoop ();
  }

  Status = PlatformStopDisplay ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "PrePi display DMA handoff failed: %r\n", Status));
    CpuDeadLoop ();
  }
}
