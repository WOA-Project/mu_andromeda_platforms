/** @file
  Mbed TLS hash adapters for the EDK II Crypto PPI.

  Copyright (c) Microsoft Corporation.
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "MbedTlsHash.h"

#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>
#include <mbedtls/sha512.h>

/**
  Validates the common parameters for a one-shot hash operation.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to the buffer that receives the hash value.

  @retval TRUE   The parameters are valid.
  @retval FALSE  The parameters are invalid.
**/
STATIC
BOOLEAN
HashParametersValid (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  IN CONST VOID  *HashValue
  )
{
  return (BOOLEAN)((HashValue != NULL) && ((Data != NULL) || (DataSize == 0)));
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-1 operations.

  @return  The size, in bytes, of the SHA-1 context buffer.
**/
UINTN
EFIAPI
MbedTlsSha1GetContextSize (
  VOID
  )
{
  return sizeof (mbedtls_sha1_context);
}

/**
  Initializes a SHA-1 context.

  @param[out] Context  Pointer to the SHA-1 context to initialize.

  @retval TRUE   The context was initialized successfully.
  @retval FALSE  Context is NULL or initialization failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha1Init (
  OUT VOID  *Context
  )
{
  if (Context == NULL) {
    return FALSE;
  }

  mbedtls_sha1_init (Context);
  if (mbedtls_sha1_starts (Context) != 0) {
    mbedtls_sha1_free (Context);
    return FALSE;
  }

  return TRUE;
}

/**
  Copies an existing SHA-1 context.

  @param[in]  Context     Pointer to the SHA-1 context to copy.
  @param[out] NewContext  Pointer to the destination SHA-1 context.

  @retval TRUE   The context was copied successfully.
  @retval FALSE  Context or NewContext is NULL.
**/
BOOLEAN
EFIAPI
MbedTlsSha1Duplicate (
  IN CONST VOID  *Context,
  OUT VOID       *NewContext
  )
{
  if ((Context == NULL) || (NewContext == NULL)) {
    return FALSE;
  }

  mbedtls_sha1_clone (NewContext, Context);
  return TRUE;
}

/**
  Digests input data and updates a SHA-1 context.

  @param[in, out] Context   Pointer to the initialized SHA-1 context.
  @param[in]      Data      Pointer to the buffer containing the data to hash.
  @param[in]      DataSize  Size of Data in bytes.

  @retval TRUE   The data was processed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha1Update (
  IN OUT VOID    *Context,
  IN CONST VOID  *Data,
  IN UINTN       DataSize
  )
{
  if ((Context == NULL) || ((Data == NULL) && (DataSize != 0))) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha1_update (Context, Data, DataSize) == 0);
}

/**
  Completes a SHA-1 hash operation and releases the context resources.

  @param[in, out] Context    Pointer to the initialized SHA-1 context.
  @param[out]     HashValue  Pointer to a 20-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha1Final (
  IN OUT VOID  *Context,
  OUT UINT8    *HashValue
  )
{
  INT32  Result;

  if ((Context == NULL) || (HashValue == NULL)) {
    return FALSE;
  }

  Result = mbedtls_sha1_finish (Context, HashValue);
  mbedtls_sha1_free (Context);
  return (BOOLEAN)(Result == 0);
}

/**
  Computes the SHA-1 digest of a data buffer.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to a 20-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha1HashAll (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  OUT UINT8      *HashValue
  )
{
  if (!HashParametersValid (Data, DataSize, HashValue)) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha1 (Data, DataSize, HashValue) == 0);
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-256 operations.

  @return  The size, in bytes, of the SHA-256 context buffer.
**/
UINTN
EFIAPI
MbedTlsSha256GetContextSize (
  VOID
  )
{
  return sizeof (mbedtls_sha256_context);
}

/**
  Initializes a SHA-256 context.

  @param[out] Context  Pointer to the SHA-256 context to initialize.

  @retval TRUE   The context was initialized successfully.
  @retval FALSE  Context is NULL or initialization failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha256Init (
  OUT VOID  *Context
  )
{
  if (Context == NULL) {
    return FALSE;
  }

  mbedtls_sha256_init (Context);
  if (mbedtls_sha256_starts (Context, FALSE) != 0) {
    mbedtls_sha256_free (Context);
    return FALSE;
  }

  return TRUE;
}

/**
  Copies an existing SHA-256 context.

  @param[in]  Context     Pointer to the SHA-256 context to copy.
  @param[out] NewContext  Pointer to the destination SHA-256 context.

  @retval TRUE   The context was copied successfully.
  @retval FALSE  Context or NewContext is NULL.
**/
BOOLEAN
EFIAPI
MbedTlsSha256Duplicate (
  IN CONST VOID  *Context,
  OUT VOID       *NewContext
  )
{
  if ((Context == NULL) || (NewContext == NULL)) {
    return FALSE;
  }

  mbedtls_sha256_clone (NewContext, Context);
  return TRUE;
}

/**
  Digests input data and updates a SHA-256 context.

  @param[in, out] Context   Pointer to the initialized SHA-256 context.
  @param[in]      Data      Pointer to the buffer containing the data to hash.
  @param[in]      DataSize  Size of Data in bytes.

  @retval TRUE   The data was processed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha256Update (
  IN OUT VOID    *Context,
  IN CONST VOID  *Data,
  IN UINTN       DataSize
  )
{
  if ((Context == NULL) || ((Data == NULL) && (DataSize != 0))) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha256_update (Context, Data, DataSize) == 0);
}

/**
  Completes a SHA-256 hash operation and releases the context resources.

  @param[in, out] Context    Pointer to the initialized SHA-256 context.
  @param[out]     HashValue  Pointer to a 32-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha256Final (
  IN OUT VOID  *Context,
  OUT UINT8    *HashValue
  )
{
  INT32  Result;

  if ((Context == NULL) || (HashValue == NULL)) {
    return FALSE;
  }

  Result = mbedtls_sha256_finish (Context, HashValue);
  mbedtls_sha256_free (Context);
  return (BOOLEAN)(Result == 0);
}

/**
  Computes the SHA-256 digest of a data buffer.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to a 32-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha256HashAll (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  OUT UINT8      *HashValue
  )
{
  if (!HashParametersValid (Data, DataSize, HashValue)) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha256 (Data, DataSize, HashValue, FALSE) == 0);
}

/**
  Initializes a SHA-384 or SHA-512 context.

  @param[out] Context   Pointer to the context to initialize.
  @param[in]  IsSha384  TRUE to initialize SHA-384; FALSE to initialize SHA-512.

  @retval TRUE   The context was initialized successfully.
  @retval FALSE  Context is NULL or initialization failed.
**/
STATIC
BOOLEAN
MbedTlsSha512InitInternal (
  OUT VOID    *Context,
  IN BOOLEAN  IsSha384
  )
{
  if (Context == NULL) {
    return FALSE;
  }

  mbedtls_sha512_init (Context);
  if (mbedtls_sha512_starts (Context, IsSha384) != 0) {
    mbedtls_sha512_free (Context);
    return FALSE;
  }

  return TRUE;
}

/**
  Copies an existing SHA-384 or SHA-512 context.

  @param[in]  Context     Pointer to the context to copy.
  @param[out] NewContext  Pointer to the destination context.

  @retval TRUE   The context was copied successfully.
  @retval FALSE  Context or NewContext is NULL.
**/
STATIC
BOOLEAN
MbedTlsSha512DuplicateInternal (
  IN CONST VOID  *Context,
  OUT VOID       *NewContext
  )
{
  if ((Context == NULL) || (NewContext == NULL)) {
    return FALSE;
  }

  mbedtls_sha512_clone (NewContext, Context);
  return TRUE;
}

/**
  Digests input data and updates a SHA-384 or SHA-512 context.

  @param[in, out] Context   Pointer to the initialized context.
  @param[in]      Data      Pointer to the buffer containing the data to hash.
  @param[in]      DataSize  Size of Data in bytes.

  @retval TRUE   The data was processed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
STATIC
BOOLEAN
MbedTlsSha512UpdateInternal (
  IN OUT VOID    *Context,
  IN CONST VOID  *Data,
  IN UINTN       DataSize
  )
{
  if ((Context == NULL) || ((Data == NULL) && (DataSize != 0))) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha512_update (Context, Data, DataSize) == 0);
}

/**
  Completes a SHA-384 or SHA-512 hash operation and releases the context resources.

  @param[in, out] Context    Pointer to the initialized context.
  @param[out]     HashValue  Pointer to a buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
STATIC
BOOLEAN
MbedTlsSha512FinalInternal (
  IN OUT VOID  *Context,
  OUT UINT8    *HashValue
  )
{
  INT32  Result;

  if ((Context == NULL) || (HashValue == NULL)) {
    return FALSE;
  }

  Result = mbedtls_sha512_finish (Context, HashValue);
  mbedtls_sha512_free (Context);
  return (BOOLEAN)(Result == 0);
}

/**
  Computes the SHA-384 or SHA-512 digest of a data buffer.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to a buffer that receives the digest.
  @param[in]  IsSha384   TRUE to compute SHA-384; FALSE to compute SHA-512.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
STATIC
BOOLEAN
MbedTlsSha512HashAllInternal (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  OUT UINT8      *HashValue,
  IN BOOLEAN     IsSha384
  )
{
  if (!HashParametersValid (Data, DataSize, HashValue)) {
    return FALSE;
  }

  return (BOOLEAN)(mbedtls_sha512 (Data, DataSize, HashValue, IsSha384) == 0);
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-384 operations.

  @return  The size, in bytes, of the SHA-384 context buffer.
**/
UINTN
EFIAPI
MbedTlsSha384GetContextSize (
  VOID
  )
{
  return sizeof (mbedtls_sha512_context);
}

/**
  Initializes a SHA-384 context.

  @param[out] Context  Pointer to the SHA-384 context to initialize.

  @retval TRUE   The context was initialized successfully.
  @retval FALSE  Context is NULL or initialization failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha384Init (
  OUT VOID  *Context
  )
{
  return MbedTlsSha512InitInternal (Context, TRUE);
}

/**
  Copies an existing SHA-384 context.

  @param[in]  Context     Pointer to the SHA-384 context to copy.
  @param[out] NewContext  Pointer to the destination SHA-384 context.

  @retval TRUE   The context was copied successfully.
  @retval FALSE  Context or NewContext is NULL.
**/
BOOLEAN
EFIAPI
MbedTlsSha384Duplicate (
  IN CONST VOID  *Context,
  OUT VOID       *NewContext
  )
{
  return MbedTlsSha512DuplicateInternal (Context, NewContext);
}

/**
  Digests input data and updates a SHA-384 context.

  @param[in, out] Context   Pointer to the initialized SHA-384 context.
  @param[in]      Data      Pointer to the buffer containing the data to hash.
  @param[in]      DataSize  Size of Data in bytes.

  @retval TRUE   The data was processed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha384Update (
  IN OUT VOID    *Context,
  IN CONST VOID  *Data,
  IN UINTN       DataSize
  )
{
  return MbedTlsSha512UpdateInternal (Context, Data, DataSize);
}

/**
  Completes a SHA-384 hash operation and releases the context resources.

  @param[in, out] Context    Pointer to the initialized SHA-384 context.
  @param[out]     HashValue  Pointer to a 48-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha384Final (
  IN OUT VOID  *Context,
  OUT UINT8    *HashValue
  )
{
  return MbedTlsSha512FinalInternal (Context, HashValue);
}

/**
  Computes the SHA-384 digest of a data buffer.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to a 48-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha384HashAll (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  OUT UINT8      *HashValue
  )
{
  return MbedTlsSha512HashAllInternal (Data, DataSize, HashValue, TRUE);
}

/**
  Retrieves the size, in bytes, of the context buffer required for SHA-512 operations.

  @return  The size, in bytes, of the SHA-512 context buffer.
**/
UINTN
EFIAPI
MbedTlsSha512GetContextSize (
  VOID
  )
{
  return sizeof (mbedtls_sha512_context);
}

/**
  Initializes a SHA-512 context.

  @param[out] Context  Pointer to the SHA-512 context to initialize.

  @retval TRUE   The context was initialized successfully.
  @retval FALSE  Context is NULL or initialization failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha512Init (
  OUT VOID  *Context
  )
{
  return MbedTlsSha512InitInternal (Context, FALSE);
}

/**
  Copies an existing SHA-512 context.

  @param[in]  Context     Pointer to the SHA-512 context to copy.
  @param[out] NewContext  Pointer to the destination SHA-512 context.

  @retval TRUE   The context was copied successfully.
  @retval FALSE  Context or NewContext is NULL.
**/
BOOLEAN
EFIAPI
MbedTlsSha512Duplicate (
  IN CONST VOID  *Context,
  OUT VOID       *NewContext
  )
{
  return MbedTlsSha512DuplicateInternal (Context, NewContext);
}

/**
  Digests input data and updates a SHA-512 context.

  @param[in, out] Context   Pointer to the initialized SHA-512 context.
  @param[in]      Data      Pointer to the buffer containing the data to hash.
  @param[in]      DataSize  Size of Data in bytes.

  @retval TRUE   The data was processed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha512Update (
  IN OUT VOID    *Context,
  IN CONST VOID  *Data,
  IN UINTN       DataSize
  )
{
  return MbedTlsSha512UpdateInternal (Context, Data, DataSize);
}

/**
  Completes a SHA-512 hash operation and releases the context resources.

  @param[in, out] Context    Pointer to the initialized SHA-512 context.
  @param[out]     HashValue  Pointer to a 64-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha512Final (
  IN OUT VOID  *Context,
  OUT UINT8    *HashValue
  )
{
  return MbedTlsSha512FinalInternal (Context, HashValue);
}

/**
  Computes the SHA-512 digest of a data buffer.

  @param[in]  Data       Pointer to the buffer containing the data to hash.
  @param[in]  DataSize   Size of Data in bytes.
  @param[out] HashValue  Pointer to a 64-byte buffer that receives the digest.

  @retval TRUE   The digest was computed successfully.
  @retval FALSE  A parameter is invalid or the hash operation failed.
**/
BOOLEAN
EFIAPI
MbedTlsSha512HashAll (
  IN CONST VOID  *Data,
  IN UINTN       DataSize,
  OUT UINT8      *HashValue
  )
{
  return MbedTlsSha512HashAllInternal (Data, DataSize, HashValue, FALSE);
}

/**
  Uses ZeroMem to clear the buffer

  Glue function is used instead of using platform_util.c due to
  additional includes/defines required in that file.

  @param[in, out] buf  Pointer to the buffer to clear.
  @param[in]      len  Size of the buffer in bytes.
**/
VOID
mbedtls_platform_zeroize (
  VOID    *buf,
  size_t  len
  )
{
  ZeroMem (buf, len);
}
