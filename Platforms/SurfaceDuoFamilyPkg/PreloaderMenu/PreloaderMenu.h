/** @file
  PreloaderMenu routines to handle the callbacks and browser calls

Copyright (c) 2004 - 2012, Intel Corporation. All rights reserved.<BR>
Copyright (c), Microsoft Corporation. All rights reserved.<BR>
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#ifndef _PRELOADER_MENU_H_
#define _PRELOADER_MENU_H_

#include <Protocol/FormBrowser2.h>
#include <Protocol/HiiConfigAccess.h>
#include "PreloaderMenuVfr.h"  // all shared VFR / C constants here.
#include <DfciSystemSettingTypes.h>
#include <Protocol/DfciSettingAccess.h>
#include <Library/HiiLib.h>
#include <Protocol/MsFrontPageAuthTokenProtocol.h>
#include <Protocol/DfciAuthentication.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/HiiConfigRouting.h>

//
// These are the VFR compiler generated data representing our VFR data.
//
extern UINT8  PreloaderMenuVfrBin[];

extern UINTN  mCallbackKey;

#define PRELOADER_MENU_CALLBACK_DATA_SIGNATURE  SIGNATURE_32 ('F', 'P', 'C', 'B')

typedef struct {
  UINTN                             Signature;

  //
  // HII relative handles
  //
  EFI_HII_HANDLE                    HiiHandle;
  EFI_HANDLE                        DriverHandle;
  EFI_STRING_ID                     *LanguageToken;

  //
  // Produced protocols
  //
  EFI_HII_CONFIG_ACCESS_PROTOCOL    ConfigAccess;
} PRELOADER_MENU_CALLBACK_DATA;

extern PRELOADER_MENU_CALLBACK_DATA  mPreloaderMenuPrivate;
extern EFI_GUID                  gMuPreloaderMenuConfigFormSetGuid;

/**
  Initialize HII information for the PreloaderMenu


  @param InitializeHiiData    TRUE if HII elements need to be initialized.

  @retval  EFI_SUCCESS        The operation is successful.
  @retval  EFI_DEVICE_ERROR   If the dynamic opcode creation failed.

**/
EFI_STATUS
InitializePreloaderMenu (
  BOOLEAN  InitializeHiiData
  );

/**
  Acquire an Auth Token and save it in a protocol
**/
EFI_STATUS
GetAuthToken (
  CHAR16  *PasswordBuffer
  );

#endif // _PRELOADER_MENU_H_
