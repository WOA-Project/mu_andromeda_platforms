/** @file
  Preserve early display diagnostics in a HOB.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/PrePiDisplayDiagnosticLib.h>

VOID
PublishDisplayDiagnostic (
  VOID
  )
{
  CONST DISPLAY_RESUME_DIAGNOSTIC *DisplayDiagnostic;
  VOID *DisplayDiagnosticHob;

  DisplayDiagnostic = GetDisplayDiagnostic ();
  if (DisplayDiagnostic != NULL) {
    DisplayDiagnosticHob = BuildGuidHob (
                             &gDisplayResumeDiagnosticHobGuid,
                             sizeof (*DisplayDiagnostic)
                             );
    if (DisplayDiagnosticHob == NULL) {
      DEBUG ((DEBUG_ERROR, "Unable to preserve early display diagnostics\n"));
    } else {
      CopyMem (DisplayDiagnosticHob, DisplayDiagnostic, sizeof (*DisplayDiagnostic));
    }
  }
}
