/** @file
  Surface Duo display diagnostics at the PrePi lifecycle boundary.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/PrePiDisplayDiagnosticLib.h>
#include <Library/PrePiPlatformHooksLib.h>

VOID
EFIAPI
PrePiExecuteEarlyDisplayDiagnostics (
  VOID
  )
{
  CaptureDisplayDiagnostic ();
}

VOID
EFIAPI
PrePiExecutePriorToCpuHob (
  VOID
  )
{
  PublishDisplayDiagnostic ();
}
