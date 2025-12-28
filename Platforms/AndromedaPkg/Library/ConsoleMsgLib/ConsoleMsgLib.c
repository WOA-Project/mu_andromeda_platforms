/**@file Console Message Library

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include "ConsoleUtils.h"

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MuUefiVersionLib.h>
#include <Library/PlatformHobLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EFIChipInfo.h>

/**
Display the platform specific debug messages
**/
VOID EFIAPI ConsoleMsgLibDisplaySystemInfoOnConsole(VOID)
{
  EFI_STATUS             Status;
  CHAR8                 *uefiDate             = NULL;
  CHAR8                 *uefiVersion          = NULL;
  EFIChipInfoVersionType SoCID                = 0;
  UINT16                 chipInfoMajorVersion = 0;
  UINT16                 chipInfoMinorVersion = 0;
  UINTN                  DateBufferLength     = 0;
  UINTN                  VersionBufferLength  = 0;

  sproduct_id_t ProductID = 0;
  sboard_id_t   BoardID   = 0;
  CHAR8        *BuildID   = "Unknown";
  CHAR8        *SKUID     = "Unknown";

  EFI_CHIPINFO_PROTOCOL *mBoardProtocol = NULL;

  ConsolePrint(L"Firmware information:");

  Status = GetBuildDateStringAscii(NULL, &DateBufferLength);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    uefiDate = (CHAR8 *)AllocateZeroPool(DateBufferLength);
    if (uefiDate != NULL) {
      Status = GetBuildDateStringAscii(uefiDate, &DateBufferLength);
      if (Status == EFI_SUCCESS) {
        ConsolePrint(L"  UEFI build date: %a", uefiDate);
      }
      FreePool(uefiDate);
    }
  }

  Status = GetUefiVersionStringAscii(NULL, &VersionBufferLength);
  if (Status == EFI_BUFFER_TOO_SMALL) {
    uefiVersion = (CHAR8 *)AllocateZeroPool(VersionBufferLength);
    if (uefiVersion != NULL) {
      Status = GetUefiVersionStringAscii(uefiVersion, &VersionBufferLength);
      if (Status == EFI_SUCCESS) {
        ConsolePrint(L"  UEFI version:    %a", uefiVersion);
      }
      FreePool(uefiVersion);
    }
  }

  ProductID = GetProductID();
  if (ProductID == SURFACE_DUO) {
    ConsolePrint(L"  UEFI flavor:     Epsilon");
  }
  else if (ProductID == SURFACE_DUO2) {
    ConsolePrint(L"  UEFI flavor:     Zeta");
  }

  if (ProductID != OEM_UNINITIALISED) {
    ConsolePrint(L"  TouchFW version: %a", GetTouchFWVersion());
  }

  ConsolePrint(L"Hardware information:");

  // Locate Qualcomm Board Protocol
  Status = gBS->LocateProtocol(
      &gEfiChipInfoProtocolGuid, NULL, (VOID *)&mBoardProtocol);

  if (mBoardProtocol != NULL) {
    mBoardProtocol->GetChipVersion(mBoardProtocol, &SoCID);
    chipInfoMajorVersion = (UINT16)((SoCID >> 16) & 0xFFFF);
    chipInfoMinorVersion = (UINT16)(SoCID & 0xFFFF);
    ConsolePrint(
        L"  SoC ID:    %d.%d", chipInfoMajorVersion, chipInfoMinorVersion);
  }

  if (ProductID != OEM_UNINITIALISED) {
    BoardID = GetBoardID();
    SKUID   = GetPlatformSubTypeSkuString();
    BuildID = GetPlatformSubTypeBuildString();

    ConsolePrint(L"  SKU ID:    %d (%a)", BoardID, SKUID);

    if (ProductID == SURFACE_DUO) {
      ConsolePrint(L"  Memory ID: 0 (Hynix 6GB)");
    }
    else if (ProductID == SURFACE_DUO2) {
      ConsolePrint(L"  Memory ID: 0 (Hynix 8GB)");
    }

    ConsolePrint(L"  Build ID:  %d (%a)", BoardID, BuildID);
  }
}