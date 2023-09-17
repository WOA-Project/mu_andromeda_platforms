/** @file SystemIntegrityPolicyProvisioningDxe.c

    This file contains functions for provisioning Secure Boot.

    Copyright (C) DuoWoA authors. All rights reserved.
    Copyright (C) Microsoft Corporation. All rights reserved.
    SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/PartitionInfo.h>

#include "SystemIntegrityPolicyDefaultVars.h"

STATIC EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *efiespSfsProtocol = NULL;

EFI_STATUS
TryWritePlatformSiPolicy()
{
  EFI_STATUS         Status               = EFI_SUCCESS;
  EFI_FILE_PROTOCOL *efiespFileProtocol   = NULL;
  EFI_FILE_PROTOCOL *payloadFileProtocol  = NULL;
  UINT64             mSiPolicyDefaultSize = sizeof(mSiPolicyDefault);

  if (efiespSfsProtocol == NULL) {
    return EFI_NOT_FOUND;
  }

  DEBUG((EFI_D_INFO, "EFIESP Partition Found.\n"));

  Status =
      efiespSfsProtocol->OpenVolume(efiespSfsProtocol, &efiespFileProtocol);
  if (EFI_ERROR(Status)) {
    return NULL;
  }

  //
  // Check if this is a Windows ESP partition first
  //
  Status = efiespFileProtocol->Open(
      efiespFileProtocol, &payloadFileProtocol,
      L"\\EFI\\Microsoft\\Boot\\bootmgfw.efi", EFI_FILE_MODE_READ,
      EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to open bootmgfw.efi: %r\n", Status));
    goto exit;
  }

  payloadFileProtocol->Close(payloadFileProtocol);

  Status = efiespFileProtocol->Open(
      efiespFileProtocol, &payloadFileProtocol,
      L"\\EFI\\Microsoft\\Boot\\SiPolicy.p7b",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE, 0);

  //
  // If the file already exists, do not touch it
  //
  if (!EFI_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR, "SiPolicy.p7b already exists, do not update: %r\n",
         Status));
    payloadFileProtocol->Close(payloadFileProtocol);
    goto exit;
  }

  Status = efiespFileProtocol->Open(
      efiespFileProtocol, &payloadFileProtocol,
      L"\\EFI\\Microsoft\\Boot\\SiPolicy.p7b",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to open SiPolicy.p7b: %r\n", Status));
    goto exit;
  }

  Status = payloadFileProtocol->Write(
      payloadFileProtocol, &mSiPolicyDefaultSize, mSiPolicyDefault);
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to write SiPolicy.p7b: %r\n", Status));
    goto exit;
  }

  payloadFileProtocol->Close(payloadFileProtocol);
  efiespFileProtocol->Close(efiespFileProtocol);

exit:
  return Status;
}

EFI_STATUS
TryLocateEfiespOnAllHandles()
{
  EFI_STATUS                       Status;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                            HandleCount;
  UINTN                            Index;
  EFI_PARTITION_INFO_PROTOCOL     *PartitionInfo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *EfiSfsProtocol;

  HandleBuffer = NULL;

  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiPartitionInfoProtocolGuid, NULL, &HandleCount,
      &HandleBuffer);
  if (EFI_ERROR(Status)) {
    return EFI_ABORTED;
  }

  for (Index = 0; Index < HandleCount; Index++) {
    Status = gBS->HandleProtocol(
        HandleBuffer[Index], &gEfiPartitionInfoProtocolGuid,
        (VOID **)&PartitionInfo);
    if (EFI_ERROR(Status)) {
      continue;
    }

    if (PartitionInfo->Type == PARTITION_TYPE_GPT) {
      if (CompareGuid(
              &PartitionInfo->Info.Gpt.PartitionTypeGUID,
              &gEfiPartTypeSystemPartGuid) == 0) {
        Status = gBS->HandleProtocol(
            HandleBuffer[Index], &gEfiSimpleFileSystemProtocolGuid,
            (VOID **)&EfiSfsProtocol);

        if (EFI_ERROR(Status)) {
          continue;
        }

        efiespSfsProtocol = EfiSfsProtocol;
        TryWritePlatformSiPolicy();
      }
    }
    else {
      continue;
    }
  }

  FreePool(HandleBuffer);
  return EFI_SUCCESS;
}

//
// Global variables.
//
VOID *mFileSystemRegistration = NULL;

EFI_STATUS
EFIAPI
TryOpenEfiesp(EFI_HANDLE SfsHandle)
{
  EFI_STATUS                       Status = EFI_SUCCESS;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *EfiSfsProtocol;
  EFI_PARTITION_INFO_PROTOCOL     *PartitionInfo;

  Status = gBS->HandleProtocol(
      SfsHandle, &gEfiPartitionInfoProtocolGuid, (VOID **)&PartitionInfo);
  if (!EFI_ERROR(Status)) {
    if (PartitionInfo->Type == PARTITION_TYPE_GPT) {
      if (CompareGuid(
              &PartitionInfo->Info.Gpt.PartitionTypeGUID,
              &gEfiPartTypeSystemPartGuid) == 0) {
        Status = gBS->HandleProtocol(
            SfsHandle, &gEfiSimpleFileSystemProtocolGuid,
            (VOID **)&EfiSfsProtocol);

        if (EFI_ERROR(Status)) {
          goto exit;
        }

        efiespSfsProtocol = EfiSfsProtocol;
        TryWritePlatformSiPolicy();
      }
    }
  }

exit:
  return Status;
}

/**
  New File System Discovered.

  Register this device as a possible UEFI Log device.

  @param    Event           Not Used.
  @param    Context         Not Used.

  @retval   none

  This may be called for multiple device arrivals, so the Event is not closed.

**/
VOID EFIAPI OnFileSystemNotification(IN EFI_EVENT Event, IN VOID *Context)
{
  UINTN       HandleCount;
  EFI_HANDLE *HandleBuffer;
  EFI_STATUS  Status;

  for (;;) {
    //
    // Get the next handle
    //
    Status = gBS->LocateHandleBuffer(
        ByRegisterNotify, NULL, mFileSystemRegistration, &HandleCount,
        &HandleBuffer);
    //
    // If not found, or any other error, we're done
    //
    if (EFI_ERROR(Status)) {
      break;
    }

    // Spec says we only get one at a time using ByRegisterNotify
    ASSERT(HandleCount == 1);

    TryOpenEfiesp(HandleBuffer[0]);

    FreePool(HandleBuffer);
  }
}

/**
    ProcessFileSystemRegistration

    This function registers for FileSystemProtocols, and then
    checks for any existing FileSystemProtocols, and adds them
    to the LOG_DEVICE list.

    @param       VOID

    @retval      EFI_SUCCESS     FileSystem protocol registration successful
    @retval      error code      Something went wrong.

 **/
EFI_STATUS
ProcessFileSystemRegistration(VOID)
{
  EFI_EVENT  FileSystemCallBackEvent;
  EFI_STATUS Status;

  //
  // Always register for file system notifications.  They may arrive at any
  // time.
  //
  DEBUG((DEBUG_INFO, "Registering for file systems notifications\n"));
  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL, TPL_CALLBACK, OnFileSystemNotification, NULL,
      &FileSystemCallBackEvent);

  if (EFI_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR, "%a: failed to create callback event (%r)\n",
         __FUNCTION__, Status));
    goto Cleanup;
  }

  Status = gBS->RegisterProtocolNotify(
      &gEfiSimpleFileSystemProtocolGuid, FileSystemCallBackEvent,
      &mFileSystemRegistration);

  if (EFI_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR,
         "%a: failed to register for file system notifications (%r)\n",
         __FUNCTION__, Status));
    gBS->CloseEvent(FileSystemCallBackEvent);
    goto Cleanup;
  }

  //
  // Process any existing File System that were present before the registration.
  //
  OnFileSystemNotification(FileSystemCallBackEvent, NULL);

Cleanup:
  return Status;
}

EFI_STATUS
EFIAPI
SystemIntegrityPolicyProvisioningDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status = EFI_SUCCESS;

  TryLocateEfiespOnAllHandles();
  if (efiespSfsProtocol) {
    return EFI_SUCCESS;
  }

  Status = ProcessFileSystemRegistration();
  if (EFI_ERROR(Status)) {
    DEBUG(
        (DEBUG_ERROR, "%a - Failed to process file system registration %r!\n",
         __FUNCTION__, Status));
    goto exit;
  }

exit:
  return Status;
}