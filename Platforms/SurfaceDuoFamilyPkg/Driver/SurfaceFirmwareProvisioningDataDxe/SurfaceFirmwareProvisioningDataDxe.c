/** @file SurfaceFirmwareProvisioningDataDxe.c

    This file contains functions for retrieving provisioning data.

    Copyright (C) DuoWoA authors. All rights reserved.
    Copyright (C) Microsoft Corporation. All rights reserved.
    SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Base.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

#include <Protocol/BlockIo.h>
#include <Protocol/PartitionInfo.h>

EFI_STATUS
TestCode()
{
  EFI_STATUS                       Status;
  EFI_HANDLE                      *HandleBuffer;
  UINTN                            HandleCount;
  UINTN                            Index;
  EFI_PARTITION_INFO_PROTOCOL     *PartitionInfo;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *SimpleFileSystem;
  EFI_FILE_PROTOCOL               *FileProtocol;
  CHAR8                            PartitionName[36];

  HandleBuffer = NULL;

  DEBUG((EFI_D_ERROR, "Test Code Entry\n"));

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
      for (int i = 0; i < 36; i++) {
        PartitionName[i] = (CHAR8)PartitionInfo->Info.Gpt.PartitionName[i];
      }
      DEBUG(
          (EFI_D_ERROR, "Found GPT Partition With Name: %a\n", PartitionName));
    }
    else {
      DEBUG((EFI_D_ERROR, "Found Unknown Partition\n"));
      continue;
    }

    Status = gBS->HandleProtocol(
        HandleBuffer[Index], &gEfiSimpleFileSystemProtocolGuid,
        (VOID **)&SimpleFileSystem);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Unable to open SFS on partition found\n"));
      continue;
    }

    Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &FileProtocol);
    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Unable to open FileProtocol on SFS\n"));
      continue;
    }

    FileProtocol->Close(FileProtocol);
  }

  DEBUG((EFI_D_ERROR, "Test Code Exit\n"));

  FreePool(HandleBuffer);
  return EFI_SUCCESS;
}

//
// Global variables.
//
VOID *mFileSystemRegistration = NULL;

EFI_STATUS
EFIAPI
TryOpenSfpd(EFI_HANDLE SfsHandle)
{
  EFI_STATUS                       Status = EFI_SUCCESS;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *EfiSfsProtocol;
  EFI_FILE_PROTOCOL               *FileProtocol;
  EFI_PARTITION_INFO_PROTOCOL     *PartitionInfo;
  CHAR8                            PartitionName[36];

  DEBUG(
      (DEBUG_ERROR, "%a: HandleProtocol gEfiSimpleFileSystemProtocolGuid\n",
       __FUNCTION__));

  Status = gBS->HandleProtocol(
      SfsHandle, &gEfiPartitionInfoProtocolGuid, (VOID **)&PartitionInfo);
  if (!EFI_ERROR(Status)) {
    if (PartitionInfo->Type == PARTITION_TYPE_GPT) {
      for (int i = 0; i < 36; i++) {
        PartitionName[i] = (CHAR8)PartitionInfo->Info.Gpt.PartitionName[i];
      }
      DEBUG(
          (EFI_D_ERROR, "Found GPT Partition With Name: %a\n", PartitionName));
    }
    else {
      DEBUG((EFI_D_ERROR, "Found Unknown Partition\n"));
    }
  }
  else {
    DEBUG(
        (EFI_D_ERROR, "Found Unknown Partition with no attached "
                      "gEfiPartitionInfoProtocolGuid\n"));
  }

  Status = gBS->HandleProtocol(
      SfsHandle, &gEfiSimpleFileSystemProtocolGuid, (VOID **)&EfiSfsProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Failed to invoke HandleProtocol.\n"));
    goto exit;
  }

  DEBUG((DEBUG_ERROR, "%a: OpenVolume EfiSfsProtocol\n", __FUNCTION__));

  Status = EfiSfsProtocol->OpenVolume(EfiSfsProtocol, &FileProtocol);

  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "Fail to get file protocol handle\n"));
    goto exit;
  }

  FileProtocol->Close(FileProtocol);

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

  DEBUG((DEBUG_INFO, "%a: Entry...\n", __FUNCTION__));

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

    TryOpenSfpd(HandleBuffer[0]);

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
SurfaceFirmwareProvisioningDataDxeInitialize(
    IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  EFI_STATUS Status;

  TestCode();

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