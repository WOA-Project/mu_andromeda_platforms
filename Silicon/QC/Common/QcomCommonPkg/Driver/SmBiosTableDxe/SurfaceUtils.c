/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

/* Used to read device serial number */
#include <Protocol/SurfaceFirmwareProvisioningDataProtocol.h>

#include "PlatformUtils.h"
#include "SurfaceUtils.h"

EFI_STATUS
EFIAPI
RetrieveSurfaceInformation(
    CHAR8 *SerialNumberString, CHAR8 *VersionString, CHAR8 *RetailSkuString)
{
  EFI_STATUS Status;

  CHAR8 FccModelId[PLATFORM_TYPE_STRING_MAX_SIZE]   = {0};
  CHAR8 SerialNumber[PLATFORM_TYPE_STRING_MAX_SIZE] = {0};

  SFPD_PROTOCOL *mDeviceProtocol = NULL;

  // Locate Sfpd Protocol
  Status =
      gBS->LocateProtocol(&gSfpdProtocolGuid, NULL, (VOID *)&mDeviceProtocol);

  if (EFI_ERROR(Status) || mDeviceProtocol == NULL) {
    return Status;
  }

  UINTN serialNoLength = PLATFORM_TYPE_STRING_MAX_SIZE;
  Status =
      mDeviceProtocol->GetSurfaceSerialNumber(&serialNoLength, SerialNumber);
  if (!EFI_ERROR(Status)) {
    ZeroMem(SerialNumberString, sizeof(SerialNumberString));
    AsciiStrnCpyS(
        SerialNumberString, PLATFORM_TYPE_STRING_MAX_SIZE, SerialNumber,
        AsciiStrLen(SerialNumber));
  }

  UINTN versionLength = PLATFORM_TYPE_STRING_MAX_SIZE;
  Status = mDeviceProtocol->GetSurfaceFccModelId(&versionLength, FccModelId);
  if (!EFI_ERROR(Status)) {
    ZeroMem(VersionString, sizeof(VersionString));
    AsciiStrnCpyS(
        VersionString, PLATFORM_TYPE_STRING_MAX_SIZE, FccModelId,
        AsciiStrLen(FccModelId));
  }

  AsciiStrCatS(RetailSkuString, PLATFORM_TYPE_STRING_MAX_SIZE, VersionString);

  return EFI_SUCCESS;
}