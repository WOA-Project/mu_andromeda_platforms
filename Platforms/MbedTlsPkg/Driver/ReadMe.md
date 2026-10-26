# Mbed TLS PEI Crypto Driver

The Mbed TLS PEI Crypto driver produces the EDK II Crypto PPI
(`gEdkiiCryptoPpiGuid`) for use during the Pre-EFI Initialization (PEI) phase.

This driver is intentionally limited to the SHA hashing functions needed by PEI
consumers. It does not provide advanced cryptographic services such as HMAC,
symmetric or asymmetric encryption, key agreement, digital signatures,
certificate processing, random-number generation, or password-based key
derivation. All platforms examined used hashing during PEI.

## Consumer Integration

PEIMs that consume cryptographic services from this driver must use the
`BaseCryptLib` instance at
`CryptoPkg/Library/BaseCryptLibOnProtocolPpi/PeiCryptLib.inf`:

```ini
[LibraryClasses]
    BaseCryptLib|CryptoPkg/Library/BaseCryptLibOnProtocolPpi/PeiCryptLib.inf
```

Do not locate the Crypto PPI once and cache or call its function pointers
directly. The `BaseCryptLibOnProtocolPpi` instance locates the current PPI and
validates its version on every service call. This is required for safe access
from XIP PEIMs and remains valid when the Crypto PPI is reinstalled after the
Mbed TLS PEIM is shadowed into permanent memory.

## Supported Functions

| Algorithm | PPI function | Description |
| --- | --- | --- |
| General | `GetVersion` | Returns the supported EDK II Crypto PPI version. |
| SHA-1 | `Sha1GetContextSize` | Returns the size of a SHA-1 context. |
| SHA-1 | `Sha1Init` | Initializes a SHA-1 context. |
| SHA-1 | `Sha1Duplicate` | Duplicates a SHA-1 context. |
| SHA-1 | `Sha1Update` | Adds data to a SHA-1 hash operation. |
| SHA-1 | `Sha1Final` | Completes a SHA-1 hash operation. |
| SHA-1 | `Sha1HashAll` | Computes a SHA-1 digest in one operation. |
| SHA-256 | `Sha256GetContextSize` | Returns the size of a SHA-256 context. |
| SHA-256 | `Sha256Init` | Initializes a SHA-256 context. |
| SHA-256 | `Sha256Duplicate` | Duplicates a SHA-256 context. |
| SHA-256 | `Sha256Update` | Adds data to a SHA-256 hash operation. |
| SHA-256 | `Sha256Final` | Completes a SHA-256 hash operation. |
| SHA-256 | `Sha256HashAll` | Computes a SHA-256 digest in one operation. |
| SHA-384 | `Sha384GetContextSize` | Returns the size of a SHA-384 context. |
| SHA-384 | `Sha384Init` | Initializes a SHA-384 context. |
| SHA-384 | `Sha384Duplicate` | Duplicates a SHA-384 context. |
| SHA-384 | `Sha384Update` | Adds data to a SHA-384 hash operation. |
| SHA-384 | `Sha384Final` | Completes a SHA-384 hash operation. |
| SHA-384 | `Sha384HashAll` | Computes a SHA-384 digest in one operation. |
| SHA-512 | `Sha512GetContextSize` | Returns the size of a SHA-512 context. |
| SHA-512 | `Sha512Init` | Initializes a SHA-512 context. |
| SHA-512 | `Sha512Duplicate` | Duplicates a SHA-512 context. |
| SHA-512 | `Sha512Update` | Adds data to a SHA-512 hash operation. |
| SHA-512 | `Sha512Final` | Completes a SHA-512 hash operation. |
| SHA-512 | `Sha512HashAll` | Computes a SHA-512 digest in one operation. |

## Unsupported Functions

All `EDKII_CRYPTO_PPI` functions not listed in the table above are unsupported.
Their function pointers are left `NULL` and must not be called. Consumers that
require any additional cryptographic service must use another crypto
implementation.
