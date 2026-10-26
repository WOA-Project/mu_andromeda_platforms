/** @file
  Installs the EDK II Crypto PPI backed directly by Mbed TLS.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <PiPei.h>

#include <Library/DebugLib.h>
#include <Ppi/MemoryDiscovered.h>
#include <Protocol/Crypto.h>

#include "MbedTlsHash.h"

extern EFI_GUID  gEdkiiCryptoPpiGuid;

/**
  Returns the version of the EDK II Crypto PPI.
  The Version of EDKII Crypto PPI comes from the MU_BASECORE version that
  is consumed to compile the binary.

  @return  The version of the EDK II Crypto PPI.
**/
STATIC
UINTN
EFIAPI
MbedTlsCryptoGetVersion (
  VOID
  )
{
  return EDKII_CRYPTO_VERSION;
}

STATIC CONST EDKII_CRYPTO_PROTOCOL  mMbedTlsCryptoPpi = {
  .GetVersion           = MbedTlsCryptoGetVersion,
  .Sha1GetContextSize   = MbedTlsSha1GetContextSize,
  .Sha1Init             = MbedTlsSha1Init,
  .Sha1Duplicate        = MbedTlsSha1Duplicate,
  .Sha1Update           = MbedTlsSha1Update,
  .Sha1Final            = MbedTlsSha1Final,
  .Sha1HashAll          = MbedTlsSha1HashAll,
  .Sha256GetContextSize = MbedTlsSha256GetContextSize,
  .Sha256Init           = MbedTlsSha256Init,
  .Sha256Duplicate      = MbedTlsSha256Duplicate,
  .Sha256Update         = MbedTlsSha256Update,
  .Sha256Final          = MbedTlsSha256Final,
  .Sha256HashAll        = MbedTlsSha256HashAll,
  .Sha384GetContextSize = MbedTlsSha384GetContextSize,
  .Sha384Init           = MbedTlsSha384Init,
  .Sha384Duplicate      = MbedTlsSha384Duplicate,
  .Sha384Update         = MbedTlsSha384Update,
  .Sha384Final          = MbedTlsSha384Final,
  .Sha384HashAll        = MbedTlsSha384HashAll,
  .Sha512GetContextSize = MbedTlsSha512GetContextSize,
  .Sha512Init           = MbedTlsSha512Init,
  .Sha512Duplicate      = MbedTlsSha512Duplicate,
  .Sha512Update         = MbedTlsSha512Update,
  .Sha512Final          = MbedTlsSha512Final,
  .Sha512HashAll        = MbedTlsSha512HashAll,
};

STATIC CONST EFI_PEI_PPI_DESCRIPTOR  mMbedTlsCryptoPpiDescriptor = {
  EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST,
  &gEdkiiCryptoPpiGuid,
  (VOID *)&mMbedTlsCryptoPpi
};

/**
  Installs the EDK II Crypto PPI backed by Mbed TLS.

  Before permanent memory is available, this function registers the PEIM for
  shadowing and installs the Crypto PPI. After shadowing, it reinstalls the PPI
  from permanent memory or installs it if no existing instance is present.

  @param[in] FileHandle   Handle of the file being invoked.
  @param[in] PeiServices  Pointer to the PEI Services table.

  @retval EFI_SUCCESS  The Crypto PPI was installed or reinstalled successfully.
  @retval Others       An error returned by a PEI service.
**/
EFI_STATUS
EFIAPI
MbedTlsCryptoPeiEntry (
  IN EFI_PEI_FILE_HANDLE     FileHandle,
  IN CONST EFI_PEI_SERVICES  **PeiServices
  )
{
  EFI_STATUS              Status;
  VOID                    *MemoryDiscoveredPpi;
  VOID                    *CryptoPpi;
  EFI_PEI_PPI_DESCRIPTOR  *CryptoPpiDescriptor;

  Status = (*PeiServices)->LocatePpi (
                             PeiServices,
                             &gEfiPeiMemoryDiscoveredPpiGuid,
                             0,
                             NULL,
                             &MemoryDiscoveredPpi
                             );
  if (Status == EFI_NOT_FOUND) {
    Status = (*PeiServices)->RegisterForShadow (FileHandle);
    ASSERT_EFI_ERROR (Status);
    if (!EFI_ERROR (Status)) {
      Status = (*PeiServices)->InstallPpi (
                                 PeiServices,
                                 &mMbedTlsCryptoPpiDescriptor
                                 );
      ASSERT_EFI_ERROR (Status);
    }
  } else if (!EFI_ERROR (Status)) {
    Status = (*PeiServices)->LocatePpi (
                               PeiServices,
                               &gEdkiiCryptoPpiGuid,
                               0,
                               &CryptoPpiDescriptor,
                               &CryptoPpi
                               );
    if (!EFI_ERROR (Status)) {
      Status = (*PeiServices)->ReInstallPpi (
                                 PeiServices,
                                 CryptoPpiDescriptor,
                                 &mMbedTlsCryptoPpiDescriptor
                                 );
    } else {
      Status = (*PeiServices)->InstallPpi (
                                 PeiServices,
                                 &mMbedTlsCryptoPpiDescriptor
                                 );
    }

    ASSERT_EFI_ERROR (Status);
  } else {
    ASSERT_EFI_ERROR (Status);
  }

  return Status;
}
