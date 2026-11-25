/** @file
  Early display diagnostic capture and publication.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_DISPLAY_DIAGNOSTIC_LIB_H_
#define PRE_PI_DISPLAY_DIAGNOSTIC_LIB_H_

#include <Guid/DisplayResumeDiagnostic.h>

VOID CaptureDisplayDiagnostic (VOID);
VOID RecordDisplayDsiControl (UINTN Address, UINT32 Value);
CONST DISPLAY_RESUME_DIAGNOSTIC *GetDisplayDiagnostic (VOID);
VOID PublishDisplayDiagnostic (VOID);

#endif
