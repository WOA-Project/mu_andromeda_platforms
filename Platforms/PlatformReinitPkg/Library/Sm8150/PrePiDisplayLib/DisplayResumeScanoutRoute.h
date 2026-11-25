/** @file
  Shared DSI route selected from the primary LG_QHD OEM profile.

  SPDX-License-Identifier: MIT
**/

#ifndef DISPLAY_RESUME_SCANOUT_ROUTE_H_
#define DISPLAY_RESUME_SCANOUT_ROUTE_H_

#include <Uefi.h>

typedef struct {
  BOOLEAN  Valid;
  UINT8    Controller;
  UINT8    Interface;
  UINTN    DsiBase;
  UINTN    InterfaceBase;
  UINTN    TearBase;
} DISPLAY_RESUME_DSI_ROUTE;

extern DISPLAY_RESUME_DSI_ROUTE  gDisplayResumeDsiRoute;

#endif
