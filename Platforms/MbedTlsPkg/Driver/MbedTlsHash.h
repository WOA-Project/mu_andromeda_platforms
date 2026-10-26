/** @file
  Mbed TLS hash adapters for the EDK II Crypto PPI.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef MBED_TLS_HASH_H_
#define MBED_TLS_HASH_H_

#include <Base.h>

#define DECLARE_MBED_TLS_HASH_FUNCTIONS(Name) \
  UINTN EFIAPI MbedTls##Name##GetContextSize (VOID); \
  BOOLEAN EFIAPI MbedTls##Name##Init (OUT VOID *Context); \
  BOOLEAN EFIAPI MbedTls##Name##Duplicate (IN CONST VOID *Context, OUT VOID *NewContext); \
  BOOLEAN EFIAPI MbedTls##Name##Update (IN OUT VOID *Context, IN CONST VOID *Data, IN UINTN DataSize); \
  BOOLEAN EFIAPI MbedTls##Name##Final (IN OUT VOID *Context, OUT UINT8 *HashValue); \
  BOOLEAN EFIAPI MbedTls##Name##HashAll (IN CONST VOID *Data, IN UINTN DataSize, OUT UINT8 *HashValue)

DECLARE_MBED_TLS_HASH_FUNCTIONS (Sha1);
DECLARE_MBED_TLS_HASH_FUNCTIONS (Sha256);
DECLARE_MBED_TLS_HASH_FUNCTIONS (Sha384);
DECLARE_MBED_TLS_HASH_FUNCTIONS (Sha512);

#undef DECLARE_MBED_TLS_HASH_FUNCTIONS

#endif
