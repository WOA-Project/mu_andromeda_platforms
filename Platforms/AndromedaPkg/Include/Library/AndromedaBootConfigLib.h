/** @file
  Accessors for the plain-text boot configuration passed to UEFI.
**/

#ifndef ANDROMEDA_BOOT_CONFIG_LIB_H_
#define ANDROMEDA_BOOT_CONFIG_LIB_H_

#include <Uefi.h>

extern CONST CHAR8  * volatile  gAndromedaBootConfigBuffer;
extern volatile UINTN            gAndromedaBootConfigBufferSize;

/**
  Set the boot configuration buffer before the SEC phase has created its HOB list.

  Later callers can retrieve the same data from the configuration HOB.

  @param[in] Config      Pointer to the boot configuration bytes.
  @param[in] ConfigSize  Configuration size in bytes.

  @retval EFI_SUCCESS           Buffer is available to configuration accessors.
  @retval EFI_INVALID_PARAMETER A required parameter is invalid.
  @retval EFI_COMPROMISED_DATA  Buffer contains a NUL byte or exceeds the size limit.
**/
EFI_STATUS
EFIAPI
SetAndromedaBootConfigBuffer (
  IN  CONST CHAR8  *Config,
  IN  UINTN         ConfigSize
  );

/**
  Get a value from a section and key in the boot configuration file.

  Section names and keys are case-insensitive. Values are returned as
  NUL-terminated text bytes, including any surrounding quotes. Full-line '#'
  and ';' comments and trailing inline comments introduced by whitespace
  followed by '#' or ';' are supported. Non-key records in a section are
  ignored by this accessor and can be retrieved with
  GetAndromedaBootConfigSectionEntry().

  @param[in]      Section       Section name, without brackets.
  @param[in]      Key           Key name.
  @param[out]     Value         Buffer for the value.
  @param[in, out] ValueSize     On input, buffer size including the terminator.
                                On EFI_BUFFER_TOO_SMALL, required buffer size.

  @retval EFI_SUCCESS            Value was found and copied.
  @retval EFI_INVALID_PARAMETER   A required parameter is NULL.
  @retval EFI_NOT_FOUND           No config HOB or matching key was found.
  @retval EFI_BUFFER_TOO_SMALL    Value buffer is too small.
  @retval EFI_COMPROMISED_DATA    Config data is malformed or contains duplicate keys.
**/
EFI_STATUS
EFIAPI
GetAndromedaBootConfigValue (
  IN      CONST CHAR8  *Section,
  IN      CONST CHAR8  *Key,
  OUT     CHAR8        *Value,
  IN OUT  UINTN        *ValueSize
  );

/**
  Get an unsigned decimal or hexadecimal integer from a section and key.

  Hexadecimal values use a 0x or 0X prefix. Values outside UINT64 range and
  values containing non-numeric characters are rejected.

  @param[in]  Section  Section name, without brackets.
  @param[in]  Key      Key name.
  @param[out] Value    Parsed integer.

  @retval EFI_SUCCESS           Value was found and parsed.
  @retval EFI_INVALID_PARAMETER A required parameter is NULL.
  @retval EFI_NOT_FOUND         No config HOB or matching key was found.
  @retval EFI_COMPROMISED_DATA  Value is not a valid UINT64.
**/
EFI_STATUS
EFIAPI
GetAndromedaBootConfigUint64 (
  IN  CONST CHAR8  *Section,
  IN  CONST CHAR8  *Key,
  OUT UINT64       *Value
  );

/**
  Get a non-comment data entry from a section by its zero-based index.

  This supports both key/value entries and record-oriented sections such as
  Qualcomm's [MemoryMap] and [RegisterMap]. Record text is returned without
  surrounding whitespace or an inline comment; CSV quoting is preserved.

  @param[in]      Section       Section name, without brackets.
  @param[in]      EntryIndex    Zero-based index among non-comment entries.
  @param[out]     Entry         Buffer for the entry.
  @param[in, out] EntrySize     On input, buffer size including the terminator.
                                On EFI_BUFFER_TOO_SMALL, required buffer size.

  @retval EFI_SUCCESS            Entry was found and copied.
  @retval EFI_INVALID_PARAMETER   A required parameter is NULL.
  @retval EFI_NOT_FOUND           No config HOB or matching entry was found.
  @retval EFI_BUFFER_TOO_SMALL    Entry buffer is too small.
  @retval EFI_COMPROMISED_DATA    Config data or section header is malformed.
**/
EFI_STATUS
EFIAPI
GetAndromedaBootConfigSectionEntry (
  IN      CONST CHAR8  *Section,
  IN      UINTN         EntryIndex,
  OUT     CHAR8        *Entry,
  IN OUT  UINTN        *EntrySize
  );

#endif
