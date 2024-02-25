/** @file
  User interaction functions for the PreloaderMenu.

  Copyright (C) Microsoft Corporation. All rights reserved.
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include "PreloaderMenu.h"
#include "PreloaderMenuUi.h"

#include <PiDxe.h>          // This has to be here so Protocol/FirmwareVolume2.h doesn't puke errors.
#include <UefiSecureBoot.h>

#include <Guid/GlobalVariable.h>
#include <Guid/ImageAuthentication.h>
#include <Guid/SmmVariableCommon.h>
#include <Guid/MdeModuleHii.h>

#include <Protocol/OnScreenKeyboard.h>
#include <Protocol/SimpleWindowManager.h>
#include <Protocol/FirmwareVolume2.h>
#include <Protocol/SmmCommunication.h>
#include <Protocol/SmmVariable.h>
#include <Protocol/VariablePolicy.h>

#include <Library/BaseMemoryLib.h>
#include <Library/SafeIntLib.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/HiiLib.h>
#include <Library/PrintLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MsColorTableLib.h>
#include <Library/SwmDialogsLib.h>
#include <Library/SecureBootVariableLib.h>
#include <Library/MuSecureBootKeySelectorLib.h>
#include <Library/SecureBootKeyStoreLib.h>
#include <Library/PasswordPolicyLib.h>

#include <Settings/DfciSettings.h>
#include <Settings/FrontPageSettings.h>

extern BOOLEAN                         mResetRequired;
extern DFCI_SETTING_ACCESS_PROTOCOL    *mSettingAccess;
extern UINTN                           mAuthToken;
extern EDKII_VARIABLE_POLICY_PROTOCOL  *mVariablePolicyProtocol;
extern SECURE_BOOT_PAYLOAD_INFO        *mSecureBootKeys;
extern UINT8                           mSecureBootKeysCount;

/**
  Handle a request to change the SecureBoot configuration.

  @retval EFI_SUCCESS         Successfully updated SecureBoot default variables or user cancelled.
  @retval Others              Failed to update. SecureBoot state remains unchanged.

**/
EFI_STATUS
EFIAPI
HandleSecureBootChange (
  )
{
  EFI_STATUS          Status;
  CHAR16              *DialogTitleBarText, *DialogCaptionText, *DialogBodyText;
  CHAR16              **Options = NULL;
  SWM_MB_RESULT       SwmResult;
  UINTN               SelectedIndex;
  UINT8               IndexSetValue;
  UINT8               Index;
  DFCI_SETTING_FLAGS  Flags = 0;
  UINTN               OptionsCount;

  //
  // Load UI dialog strings.
  DialogTitleBarText = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_SB_CONFIG_TITLEBARTEXT), NULL);
  DialogCaptionText  = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_SB_CONFIG_CAPTION), NULL);
  DialogBodyText     = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_SB_CONFIG_BODY), NULL);

  OptionsCount = mSecureBootKeysCount + 1;
  Options      = AllocatePool (OptionsCount * sizeof (CHAR16 *));
  if (Options == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Load UI option strings.
  for (Index = 0; Index < mSecureBootKeysCount; Index++) {
    Options[Index] = (CHAR16 *)mSecureBootKeys[Index].SecureBootKeyName;
  }

  Options[mSecureBootKeysCount] = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_GENERIC_TEXT_NONE), NULL);

  //
  // Display the dialog to the user.
  Status = SwmDialogsSelectPrompt (
             DialogTitleBarText,
             DialogCaptionText,
             DialogBodyText,
             Options,
             OptionsCount,
             &SwmResult,
             &SelectedIndex
             );
  DEBUG ((DEBUG_INFO, "INFO [SFP] %a - SingleSelectDialog returning: Status = %r, SwmResult = 0x%X, SelectedIndex = %d\n", __FUNCTION__, Status, (UINTN)SwmResult, SelectedIndex));

  //
  // If the form was submitted, process the update.
  if (!EFI_ERROR (Status) && (SWM_MB_IDOK == SwmResult) && !EFI_ERROR (SafeUintnToUint8 (SelectedIndex, &IndexSetValue))) {
    mVariablePolicyProtocol->DisableVariablePolicy ();

    if (IndexSetValue == mSecureBootKeysCount) {
      IndexSetValue = MU_SB_CONFIG_NONE;
    }

    Status = mSettingAccess->Set (
                               mSettingAccess,
                               DFCI_SETTING_ID__SECURE_BOOT_KEYS_ENUM,
                               &mAuthToken,
                               DFCI_SETTING_TYPE_SECUREBOOTKEYENUM,
                               sizeof (IndexSetValue),
                               &IndexSetValue,
                               &Flags
                               );
    //
    // If successful, update the display.
    if (!EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "%a - Successfully changed keys. Updating form browser and requesting restart.\n", __FUNCTION__));

      //
      // Indicate that this change should also trigger a reboot.
      mResetRequired = TRUE;
    } else {
      DEBUG ((DEBUG_ERROR, "ERROR [SFP] %a - Failed to update SecureBoot config! %r\n", __FUNCTION__, Status));
      DialogTitleBarText = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_SB_UPDATE_FAILURE_TITLE), NULL);
      DialogBodyText     = (CHAR16 *)HiiGetString (mPreloaderMenuPrivate.HiiHandle, STRING_TOKEN (STR_SB_UPDATE_FAILURE), NULL);
      SwmDialogsMessageBox (
        DialogTitleBarText,                            // Dialog title bar text.
        DialogBodyText,                                // Dialog body text.
        L"",                                           // Dialog caption text.
        SWM_MB_OK,                                     // Show Ok button.
        0,                                             // No timeout
        &SwmResult
        );                                             // Return result.
    }
  }

  // IMPORTANT NOTE: Until further review, all attempts to set the SecureBoot keys should result in a reboot.
  //                  This is to account for possible edge cases in the suspension of VariablePolicy enforcement.
  mResetRequired = TRUE;

  if (Options != NULL) {
    FreePool (Options);
  }

  return Status;
}