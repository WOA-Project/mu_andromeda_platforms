/** @file SecureBootProvisioningDxe.c
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Base.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <Library/MuSecureBootKeySelectorLib.h>

#include "SystemIntegrityPolicyDefaultVars.h"

STATIC EFI_IMAGE_LOAD EfiImageLoad = NULL;

EFI_STATUS
EFIAPI
TryWritePlatformSiPolicy()
{
  EFI_STATUS Status = EFI_SUCCESS;

  UINTN       NumHandles = 0;
  EFI_HANDLE *SfsHandles;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *EfiSfsProtocol;
  EFI_FILE_PROTOCOL               *FileProtocol;
  EFI_FILE_PROTOCOL               *PayloadFileProtocol;

  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &NumHandles,
      &SfsHandles);

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Fail to locate Simple File System Handles\n"));
    goto exit;
  }

  for (UINTN index = 0; index < NumHandles; index++) {
    Status = gBS->HandleProtocol(
        SfsHandles[index], &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&EfiSfsProtocol);

    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to invoke HandleProtocol.\n"));
      continue;
    }

    Status = EfiSfsProtocol->OpenVolume(EfiSfsProtocol, &FileProtocol);

    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Fail to get file protocol handle\n"));
      continue;
    }

    Status = FileProtocol->Open(
        FileProtocol, &PayloadFileProtocol,
        L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", EFI_FILE_MODE_READ,
        EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to open bootmgfw.efi: %r\n", Status));
      continue;
    }

    Status = PayloadFileProtocol->Close(PayloadFileProtocol);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to close bootmgfw.efi: %r\n", Status));
    }

    Status = FileProtocol->Open(
        FileProtocol, &PayloadFileProtocol,
        L"\\EFI\\Microsoft\\Boot\\SiPolicy.p7b",
        EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);

    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to open SiPolicy.p7b: %r\n", Status));
      continue;
    }

    Status = PayloadFileProtocol->Write(
        PayloadFileProtocol, sizeof(mSiPolicyDefault), mSiPolicyDefault);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to write SiPolicy.p7b: %r\n", Status));
    }

    Status = PayloadFileProtocol->Close(PayloadFileProtocol);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "Failed to close SiPolicy.p7b: %r\n", Status));
    }
  }

exit:
  return Status;
}

EFI_STATUS
EFIAPI
SecureBootProvisioningDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  // Microsoft Plus 3rd Party Plus Windows On Andromeda
  Status  = SetSecureBootConfig(0);

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to enroll SecureBoot config %r!\n", __FUNCTION__, Status));
    goto exit;
  }

  Status =  TryWritePlatformSiPolicy();

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "%a - Failed to write System Integrity Policy %r!\n", __FUNCTION__, Status));
    goto exit;
  }

exit:
  return Status;
}