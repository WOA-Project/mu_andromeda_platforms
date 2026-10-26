/** @file
  GoogleTest coverage for the Mbed TLS Crypto PPI.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Library/GoogleTestLib.h>

#include <vector>

extern "C" {
  #include <PiPei.h>
  #include <Library/BaseMemoryLib.h>
  #include <Ppi/MemoryDiscovered.h>
  #include <Protocol/Crypto.h>

  EFI_STATUS
  EFIAPI
  MbedTlsCryptoPeiEntry (
    IN EFI_PEI_FILE_HANDLE     FileHandle,
    IN CONST EFI_PEI_SERVICES  **PeiServices
    );

  extern EFI_GUID  gEdkiiCryptoPpiGuid;
}

using namespace testing;

namespace {
CONST UINT8  mSha1Abc[] = {
  0xA9, 0x99, 0x3E, 0x36, 0x47, 0x06, 0x81, 0x6A, 0xBA, 0x3E,
  0x25, 0x71, 0x78, 0x50, 0xC2, 0x6C, 0x9C, 0xD0, 0xD8, 0x9D
};

CONST UINT8  mSha256Abc[] = {
  0xBA, 0x78, 0x16, 0xBF, 0x8F, 0x01, 0xCF, 0xEA,
  0x41, 0x41, 0x40, 0xDE, 0x5D, 0xAE, 0x22, 0x23,
  0xB0, 0x03, 0x61, 0xA3, 0x96, 0x17, 0x7A, 0x9C,
  0xB4, 0x10, 0xFF, 0x61, 0xF2, 0x00, 0x15, 0xAD
};

CONST UINT8  mSha384Abc[] = {
  0xCB, 0x00, 0x75, 0x3F, 0x45, 0xA3, 0x5E, 0x8B,
  0xB5, 0xA0, 0x3D, 0x69, 0x9A, 0xC6, 0x50, 0x07,
  0x27, 0x2C, 0x32, 0xAB, 0x0E, 0xDE, 0xD1, 0x63,
  0x1A, 0x8B, 0x60, 0x5A, 0x43, 0xFF, 0x5B, 0xED,
  0x80, 0x86, 0x07, 0x2B, 0xA1, 0xE7, 0xCC, 0x23,
  0x58, 0xBA, 0xEC, 0xA1, 0x34, 0xC8, 0x25, 0xA7
};

CONST UINT8  mSha512Abc[] = {
  0xDD, 0xAF, 0x35, 0xA1, 0x93, 0x61, 0x7A, 0xBA,
  0xCC, 0x41, 0x73, 0x49, 0xAE, 0x20, 0x41, 0x31,
  0x12, 0xE6, 0xFA, 0x4E, 0x89, 0xA9, 0x7E, 0xA2,
  0x0A, 0x9E, 0xEE, 0xE6, 0x4B, 0x55, 0xD3, 0x9A,
  0x21, 0x92, 0x99, 0x2A, 0x27, 0x4F, 0xC1, 0xA8,
  0x36, 0xBA, 0x3C, 0x23, 0xA3, 0xFE, 0xEB, 0xBD,
  0x45, 0x4D, 0x44, 0x23, 0x64, 0x3C, 0xE8, 0x0E,
  0x2A, 0x9A, 0xC9, 0x4F, 0xA5, 0x4C, 0xA4, 0x9F
};

struct HASH_INTERFACE {
  UINTN (EFIAPI *GetContextSize)(VOID);
  BOOLEAN (EFIAPI *Init)(OUT VOID *Context);
  BOOLEAN (EFIAPI *Duplicate)(IN CONST VOID *Context, OUT VOID *NewContext);
  BOOLEAN (EFIAPI *Update)(IN OUT VOID *Context, IN CONST VOID *Data, IN UINTN DataSize);
  BOOLEAN (EFIAPI *Final)(IN OUT VOID *Context, OUT UINT8 *HashValue);
  BOOLEAN (EFIAPI *HashAll)(IN CONST VOID *Data, IN UINTN DataSize, OUT UINT8 *HashValue);
  CONST UINT8    *ExpectedDigest;
  UINTN          DigestSize;
  CONST CHAR8    *Name;
};

/**
  Builds the list of SHA interfaces and known SHA digests used by the tests.

  @param[in] CryptoPpi  Pointer to the EDK II Crypto PPI under test.

  @return  The SHA-1, SHA-256, SHA-384, and SHA-512 interfaces, including the
           expected digest of the message "abc" for each algorithm.
**/
std::vector<HASH_INTERFACE>
GetHashInterfaces (
  IN CONST EDKII_CRYPTO_PROTOCOL  *CryptoPpi
  )
{
  return {
    {
      CryptoPpi->Sha1GetContextSize,
      CryptoPpi->Sha1Init,
      CryptoPpi->Sha1Duplicate,
      CryptoPpi->Sha1Update,
      CryptoPpi->Sha1Final,
      CryptoPpi->Sha1HashAll,
      mSha1Abc,
      sizeof (mSha1Abc),
      "SHA-1"
    },
    {
      CryptoPpi->Sha256GetContextSize,
      CryptoPpi->Sha256Init,
      CryptoPpi->Sha256Duplicate,
      CryptoPpi->Sha256Update,
      CryptoPpi->Sha256Final,
      CryptoPpi->Sha256HashAll,
      mSha256Abc,
      sizeof (mSha256Abc),
      "SHA-256"
    },
    {
      CryptoPpi->Sha384GetContextSize,
      CryptoPpi->Sha384Init,
      CryptoPpi->Sha384Duplicate,
      CryptoPpi->Sha384Update,
      CryptoPpi->Sha384Final,
      CryptoPpi->Sha384HashAll,
      mSha384Abc,
      sizeof (mSha384Abc),
      "SHA-384"
    },
    {
      CryptoPpi->Sha512GetContextSize,
      CryptoPpi->Sha512Init,
      CryptoPpi->Sha512Duplicate,
      CryptoPpi->Sha512Update,
      CryptoPpi->Sha512Final,
      CryptoPpi->Sha512HashAll,
      mSha512Abc,
      sizeof (mSha512Abc),
      "SHA-512"
    }
  };
}

struct MockPeiServices {
  MOCK_METHOD (
    EFI_STATUS,
    InstallPpi,
    (CONST EFI_PEI_PPI_DESCRIPTOR *PpiList)
    );
  MOCK_METHOD (
    EFI_STATUS,
    ReInstallPpi,
    (CONST EFI_PEI_PPI_DESCRIPTOR *OldPpi, CONST EFI_PEI_PPI_DESCRIPTOR *NewPpi)
    );
  MOCK_METHOD (
    EFI_STATUS,
    LocatePpi,
    (CONST EFI_GUID *Guid, UINTN Instance, EFI_PEI_PPI_DESCRIPTOR **PpiDescriptor, VOID **Ppi)
    );
  MOCK_METHOD (
    EFI_STATUS,
    RegisterForShadow,
    (EFI_PEI_FILE_HANDLE FileHandle)
    );
};

MockPeiServices  *mPeiServicesMock;

/**
  Forwards an InstallPpi() call to the active PEI Services mock.

  @param[in] PeiServices  Pointer to the PEI Services table.
  @param[in] PpiList      Pointer to the list of PPI descriptors to install.

  @return  The status returned by the mocked InstallPpi() service.
**/
EFI_STATUS
EFIAPI
MockInstallPpi (
  IN CONST EFI_PEI_SERVICES        **PeiServices,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *PpiList
  )
{
  return mPeiServicesMock->InstallPpi (PpiList);
}

/**
  Forwards a ReInstallPpi() call to the active PEI Services mock.

  @param[in] PeiServices  Pointer to the PEI Services table.
  @param[in] OldPpi       Pointer to the PPI descriptor being replaced.
  @param[in] NewPpi       Pointer to the replacement PPI descriptor.

  @return  The status returned by the mocked ReInstallPpi() service.
**/
EFI_STATUS
EFIAPI
MockReInstallPpi (
  IN CONST EFI_PEI_SERVICES        **PeiServices,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *OldPpi,
  IN CONST EFI_PEI_PPI_DESCRIPTOR  *NewPpi
  )
{
  return mPeiServicesMock->ReInstallPpi (OldPpi, NewPpi);
}

/**
  Forwards a LocatePpi() call to the active PEI Services mock.

  @param[in]     PeiServices    Pointer to the PEI Services table.
  @param[in]     Guid           Pointer to the GUID of the PPI to locate.
  @param[in]     Instance       Zero-based instance of the PPI to locate.
  @param[in, out] PpiDescriptor Optional pointer that receives the PPI descriptor.
  @param[in, out] Ppi           Pointer that receives the PPI interface.

  @return  The status returned by the mocked LocatePpi() service.
**/
EFI_STATUS
EFIAPI
MockLocatePpi (
  IN CONST EFI_PEI_SERVICES      **PeiServices,
  IN CONST EFI_GUID              *Guid,
  IN UINTN                       Instance,
  IN OUT EFI_PEI_PPI_DESCRIPTOR  **PpiDescriptor OPTIONAL,
  IN OUT VOID                    **Ppi
  )
{
  return mPeiServicesMock->LocatePpi (
                             Guid,
                             Instance,
                             PpiDescriptor,
                             Ppi
                             );
}

/**
  Forwards a RegisterForShadow() call to the active PEI Services mock.

  @param[in] FileHandle  Handle of the PEIM requesting shadow registration.

  @return  The status returned by the mocked RegisterForShadow() service.
**/
EFI_STATUS
EFIAPI
MockRegisterForShadow (
  IN EFI_PEI_FILE_HANDLE  FileHandle
  )
{
  return mPeiServicesMock->RegisterForShadow (FileHandle);
}

class MbedTlsCryptoPeiTest : public Test {
protected:
  StrictMock<MockPeiServices> PeiServicesMock;
  EFI_PEI_SERVICES PeiServices;
  CONST EFI_PEI_SERVICES *PeiServicesPointer;
  CONST EFI_PEI_PPI_DESCRIPTOR *CryptoPpiDescriptor;
  CONST EDKII_CRYPTO_PROTOCOL *CryptoPpi;

  /**
    Creates the mocked PEI Services table and invokes the Mbed TLS Crypto PEIM.

    The setup verifies that the PEIM checks for permanent memory, registers for
    shadowing when memory is unavailable, and installs a Crypto PPI descriptor.
    It captures and validates the installed descriptor and protocol pointers for
    use by each test.
  **/
  void
  SetUp (
    ) override
  {
    EFI_STATUS  Status;
    InSequence  Sequence;

    CryptoPpiDescriptor = nullptr;
    CryptoPpi           = nullptr;
    ZeroMem (&PeiServices, sizeof (PeiServices));
    PeiServices.InstallPpi        = MockInstallPpi;
    PeiServices.ReInstallPpi      = MockReInstallPpi;
    PeiServices.LocatePpi         = MockLocatePpi;
    PeiServices.RegisterForShadow = MockRegisterForShadow;
    PeiServicesPointer            = &PeiServices;
    mPeiServicesMock              = &PeiServicesMock;

    EXPECT_CALL (
      PeiServicesMock,
      LocatePpi (
        BufferEq (&gEfiPeiMemoryDiscoveredPpiGuid, sizeof (EFI_GUID)),
        0,
        IsNull (),
        NotNull ()
        )
      )
      .WillOnce (Return (EFI_NOT_FOUND));
    EXPECT_CALL (
      PeiServicesMock,
      RegisterForShadow (NotNull ())
      )
      .WillOnce (Return (EFI_SUCCESS));
    EXPECT_CALL (
      PeiServicesMock,
      InstallPpi (NotNull ())
      )
      .WillOnce (
         DoAll (
           SaveArg<0>(&CryptoPpiDescriptor),
           Return (EFI_SUCCESS)
           )
         );

    Status = MbedTlsCryptoPeiEntry (
               reinterpret_cast<EFI_PEI_FILE_HANDLE>(this),
               &PeiServicesPointer
               );
    ASSERT_EQ (EFI_SUCCESS, Status);
    ASSERT_NE (nullptr, CryptoPpiDescriptor);
    CryptoPpi = static_cast<CONST EDKII_CRYPTO_PROTOCOL *>(CryptoPpiDescriptor->Ppi);
    ASSERT_NE (nullptr, CryptoPpi);
  }

  /**
    Disconnects the global forwarding functions from the fixture mock.
  **/
  void
  TearDown (
    ) override
  {
    mPeiServicesMock = nullptr;
  }
};

/**
  Verifies that the PEIM publishes the expected Crypto PPI descriptor.

  This test checks that the descriptor has both the PPI and terminate-list
  flags, uses gEdkiiCryptoPpiGuid, provides a non-NULL GetVersion() callback,
  and reports the current EDKII_CRYPTO_VERSION value.
**/
TEST_F (MbedTlsCryptoPeiTest, InstallsExpectedPpiAndReportsVersion) {
  EXPECT_EQ (
    static_cast<UINTN>(EFI_PEI_PPI_DESCRIPTOR_PPI | EFI_PEI_PPI_DESCRIPTOR_TERMINATE_LIST),
    CryptoPpiDescriptor->Flags
    );
  EXPECT_EQ (
    static_cast<INTN>(0),
    CompareMem (CryptoPpiDescriptor->Guid, &gEdkiiCryptoPpiGuid, sizeof (EFI_GUID))
    );
  ASSERT_NE (nullptr, CryptoPpi->GetVersion);
  EXPECT_EQ (static_cast<UINTN>(EDKII_CRYPTO_VERSION), CryptoPpi->GetVersion ());
}

/**
  Verifies SHA-1, SHA-256, SHA-384, and SHA-512 hashing through the Crypto PPI.

  For each algorithm, this test verifies that all six hash callbacks are
  present and the reported context size is nonzero. It hashes "abc" using an
  incremental context, duplicates that context after hashing "a", completes
  the original and duplicate with different update boundaries, and also uses
  the one-shot HashAll() callback. All three results must match the published
  known-answer digest for the selected algorithm.
**/
TEST_F (MbedTlsCryptoPeiTest, HashFunctionsProduceExpectedDigests) {
  CONST UINT8                        Message[]      = { 'a', 'b', 'c' };
  CONST std::vector<HASH_INTERFACE>  HashInterfaces = GetHashInterfaces (CryptoPpi);

  for (CONST HASH_INTERFACE  &Hash : HashInterfaces) {
    SCOPED_TRACE (Hash.Name);

    ASSERT_NE (nullptr, Hash.GetContextSize);
    ASSERT_NE (nullptr, Hash.Init);
    ASSERT_NE (nullptr, Hash.Duplicate);
    ASSERT_NE (nullptr, Hash.Update);
    ASSERT_NE (nullptr, Hash.Final);
    ASSERT_NE (nullptr, Hash.HashAll);

    UINTN               ContextSize = Hash.GetContextSize ();
    std::vector<UINT8>  Context (ContextSize);
    std::vector<UINT8>  DuplicateContext (ContextSize);
    std::vector<UINT8>  Digest (Hash.DigestSize);
    std::vector<UINT8>  DuplicateDigest (Hash.DigestSize);
    std::vector<UINT8>  OneShotDigest (Hash.DigestSize);

    ASSERT_GT (ContextSize, 0U);
    ASSERT_TRUE (Hash.Init (Context.data ()));
    ASSERT_TRUE (Hash.Update (Context.data (), Message, 1));
    ASSERT_TRUE (Hash.Duplicate (Context.data (), DuplicateContext.data ()));
    ASSERT_TRUE (Hash.Update (Context.data (), &Message[1], 2));
    ASSERT_TRUE (Hash.Update (DuplicateContext.data (), &Message[1], 1));
    ASSERT_TRUE (Hash.Update (DuplicateContext.data (), &Message[2], 1));
    ASSERT_TRUE (Hash.Final (Context.data (), Digest.data ()));
    ASSERT_TRUE (Hash.Final (DuplicateContext.data (), DuplicateDigest.data ()));
    ASSERT_TRUE (Hash.HashAll (Message, sizeof (Message), OneShotDigest.data ()));

    EXPECT_EQ (0, CompareMem (Digest.data (), Hash.ExpectedDigest, Hash.DigestSize));
    EXPECT_EQ (0, CompareMem (DuplicateDigest.data (), Hash.ExpectedDigest, Hash.DigestSize));
    EXPECT_EQ (0, CompareMem (OneShotDigest.data (), Hash.ExpectedDigest, Hash.DigestSize));
  }
}

/**
  Verifies parameter validation for every supported SHA interface.

  For SHA-1, SHA-256, SHA-384, and SHA-512, this test verifies rejection of a
  NULL context, destination context, digest buffer, and nonempty NULL data
  buffer. It also verifies that NULL data is accepted when the data size is
  zero for Update() and HashAll(), and that a valid context remains usable and
  can be finalized after Final() rejects a NULL output buffer.
**/
TEST_F (MbedTlsCryptoPeiTest, HashFunctionsRejectInvalidParameters) {
  CONST UINT8                        Message[]      = { 'a', 'b', 'c' };
  CONST std::vector<HASH_INTERFACE>  HashInterfaces = GetHashInterfaces (CryptoPpi);

  for (CONST HASH_INTERFACE  &Hash : HashInterfaces) {
    SCOPED_TRACE (Hash.Name);

    UINTN               ContextSize = Hash.GetContextSize ();
    std::vector<UINT8>  Context (ContextSize);
    std::vector<UINT8>  NewContext (ContextSize);
    std::vector<UINT8>  Digest (Hash.DigestSize);

    EXPECT_FALSE (Hash.Init (nullptr));
    ASSERT_TRUE (Hash.Init (Context.data ()));
    EXPECT_FALSE (Hash.Duplicate (nullptr, NewContext.data ()));
    EXPECT_FALSE (Hash.Duplicate (Context.data (), nullptr));
    EXPECT_FALSE (Hash.Update (nullptr, Message, sizeof (Message)));
    EXPECT_FALSE (Hash.Update (Context.data (), nullptr, 1));
    EXPECT_TRUE (Hash.Update (Context.data (), nullptr, 0));
    EXPECT_FALSE (Hash.Final (nullptr, Digest.data ()));
    EXPECT_FALSE (Hash.Final (Context.data (), nullptr));
    EXPECT_FALSE (Hash.HashAll (Message, sizeof (Message), nullptr));
    EXPECT_FALSE (Hash.HashAll (nullptr, 1, Digest.data ()));
    EXPECT_TRUE (Hash.HashAll (nullptr, 0, Digest.data ()));
    EXPECT_TRUE (Hash.Final (Context.data (), Digest.data ()));
  }
}
} // namespace

/**
  Runs the Mbed TLS Crypto PEI GoogleTest suite.

  @param[in] argc  Number of command-line arguments.
  @param[in] argv  Array of command-line argument strings.

  @retval 0      All tests passed.
  @retval Other  One or more tests failed.
**/
int
main (
  int   argc,
  char  *argv[]
  )
{
  InitGoogleTest (&argc, argv);
  return RUN_ALL_TESTS ();
}
