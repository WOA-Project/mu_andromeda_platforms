/** @file
  Unsupported early display diagnostics.
  SPDX-License-Identifier: MIT
**/
#include <Library/PrePiDisplayDiagnosticLib.h>

VOID
CaptureDisplayDiagnostic (
  VOID
  )
{
}

VOID
RecordDisplayDsiControl (
  UINTN  Address,
  UINT32 Value
  )
{
}

CONST DISPLAY_RESUME_DIAGNOSTIC *
GetDisplayDiagnostic (
  VOID
  )
{
  return NULL;
}
