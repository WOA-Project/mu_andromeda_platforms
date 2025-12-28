/** @file

  Copyright (c) 2022-2025 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>

/* Used to read chip serial number */
#include <Protocol/EFIChipInfo.h>
#include <Protocol/EFIPlatformInfo.h>

/* Used to hash device serial number */
#include <Protocol/Hash.h>
#include <Protocol/Hash2.h>

#include "PlatformUtils.h"

CONST CHAR8 *PlatformTypeStrings[EFI_PLATFORMINFO_NUM_TYPES] = {
    "UNKNOWN", "CDP (SURF)", "FFA",    "FLUID",       "FUSION", "OEM", "QT",
    "MTP_MDM", "MTP",        "LiQUID", "DragonBoard", "QRD",    "EVB", "HRD",
    "DTV",     "RUMI",       "VIRTIO", "GOBI",        "CBH",    "BTS", "XPM",
    "RCM",     "DMA",        "STP",    "SBC",         "ADP",    "CHI", "SDP",
    "RRP",     "CLS",        "TTP",    "HDK",         "IOT",    "ATP", "IDP"};

EFI_STATUS
EFIAPI
GetUUIDFromEFIChipInfoSerialNumType(VOID *Buffer, UINT32 BufferSize)
{
  EFI_STATUS               Status;
  UINT8                    chipSerialNumArray[16];
  EFI_HASH2_OUTPUT         efiHash2Output;
  EFI_HASH2_PROTOCOL      *efiHash2Protocol = NULL;
  EFI_GUID                *hashAlgorithm    = &gEfiHashAlgorithmSha1Guid;
  UINTN                    digestSize       = 0;
  EFIChipInfoSerialNumType chipSerialNum;
  EFI_CHIPINFO_PROTOCOL   *mBoardProtocol = NULL;

  if ((Buffer == NULL) || (BufferSize > 16)) {
    return EFI_INVALID_PARAMETER;
  }

  // Locate Qualcomm Board Protocol
  Status = gBS->LocateProtocol(
      &gEfiChipInfoProtocolGuid, NULL, (VOID *)&mBoardProtocol);
  if (EFI_ERROR(Status) || mBoardProtocol == NULL) {
    return Status;
  }

  // Get Serial Number, Chip Version, Chip Family
  Status = mBoardProtocol->GetSerialNumber(mBoardProtocol, &chipSerialNum);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  ZeroMem(chipSerialNumArray, sizeof(chipSerialNumArray));
  CopyMem(chipSerialNumArray, &chipSerialNum, sizeof(EFIChipInfoSerialNumType));

  Status = gBS->LocateProtocol(
      &gEfiHash2ProtocolGuid, NULL, (VOID **)&efiHash2Protocol);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = efiHash2Protocol->GetHashSize(
      efiHash2Protocol, hashAlgorithm, &digestSize);

  if (EFI_ERROR(Status) || digestSize != 20) {
    return Status;
  }

  Status = efiHash2Protocol->HashInit(efiHash2Protocol, hashAlgorithm);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = efiHash2Protocol->HashUpdate(
      efiHash2Protocol, (UINT8 *)&chipSerialNumArray,
      sizeof(chipSerialNumArray));
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = efiHash2Protocol->HashFinal(efiHash2Protocol, &efiHash2Output);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  CopyMem(
      Buffer, (UINT8 *)&efiHash2Output.Sha1Hash[digestSize - BufferSize],
      BufferSize);

  return Status;
}

EFI_STATUS
EFIAPI
RetrievePlatformName(CHAR8 *PlatformName)
{
  EFI_PLATFORMINFO_PROTOCOL          *pEfiPlatformInfoProtocol = NULL;
  EFI_CHIPINFO_PROTOCOL              *mBoardProtocol           = NULL;
  EFI_PLATFORMINFO_PLATFORM_INFO_TYPE PlatformInfo;

  if (PlatformName == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Locate Qualcomm Board Protocol
  EFI_STATUS Status = gBS->LocateProtocol(
      &gEfiPlatformInfoProtocolGuid, NULL, (VOID *)&pEfiPlatformInfoProtocol);

  if (EFI_ERROR(Status) || pEfiPlatformInfoProtocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = pEfiPlatformInfoProtocol->GetPlatformInfo(
      pEfiPlatformInfoProtocol, &PlatformInfo);

  if (!EFI_ERROR(Status)) {
    EFI_PLATFORMINFO_PLATFORM_TYPE platformInfoType = PlatformInfo.platform;

    if (PlatformInfo.platform >= EFI_PLATFORMINFO_NUM_TYPES) {
      platformInfoType = EFI_PLATFORMINFO_TYPE_UNKNOWN;
    }

    // HHG Platform was introduced with Lahaina later in the release cycle
    // It lacks a proper platform type, and instead makes use of the HDK type
    // with specific subtype values.
    // On lahaina, these subtype values are 1 and 2.
    // On kailua, this subtype value is 1.
    // It is confirmed on official kailua kernel sources that HHG is a
    // dedicated platform It also would not make sense to merge it with HDKs
    // due to numerous differences Detect HHG and override the type
    // accordingly.
    UINT16 IsHHGPlatform = 0;

    // Locate Qualcomm Board Protocol
    if (!EFI_ERROR(gBS->LocateProtocol(
            &gEfiChipInfoProtocolGuid, NULL, (VOID *)&mBoardProtocol)) &&
        mBoardProtocol != NULL) {
      UINT16 SDFE = 0;

      mBoardProtocol->GetChipFamily(
          mBoardProtocol, (EFIChipInfoFamilyType *)&SDFE);

      // CHIPINFO_FAMILY_LAHAINA = 105
      if (SDFE == 105) {
        if (platformInfoType == EFI_PLATFORMINFO_TYPE_HDK &&
            (PlatformInfo.subtype == 1 || PlatformInfo.subtype == 2)) {
          // HHG
          IsHHGPlatform = 1;
        }
      }
      // CHIPINFO_FAMILY_KAILUA = 127
      else if (SDFE == 127) {
        if (platformInfoType == EFI_PLATFORMINFO_TYPE_HDK &&
            PlatformInfo.subtype == 1) {
          // HHG
          IsHHGPlatform = 1;
        }
      }
    }

    if (IsHHGPlatform == 1) {
      AsciiStrnCpyS(
          PlatformName, PLATFORM_TYPE_STRING_MAX_SIZE,
          HAND_HELD_GAMING_PLATFORM_NAME,
          AsciiStrLen(HAND_HELD_GAMING_PLATFORM_NAME));
    }
    else {
      AsciiStrnCpyS(
          PlatformName, PLATFORM_TYPE_STRING_MAX_SIZE,
          PlatformTypeStrings[platformInfoType],
          AsciiStrLen(PlatformTypeStrings[platformInfoType]));
    }
  }

  return Status;
}

EFI_STATUS
EFIAPI
RetrieveChipVersion(CHAR8 *VersionString)
{
  EFI_CHIPINFO_PROTOCOL *mBoardProtocol = NULL;
  UINT32                 SIDV           = 0;

  if (VersionString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Locate Qualcomm Board Protocol
  EFI_STATUS Status = gBS->LocateProtocol(
      &gEfiChipInfoProtocolGuid, NULL, (VOID *)&mBoardProtocol);

  if (EFI_ERROR(Status) || mBoardProtocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = mBoardProtocol->GetChipVersion(mBoardProtocol, &SIDV);
  if (!EFI_ERROR(Status)) {
    UINT16 SVMJ = (UINT16)((SIDV >> 16) & 0xFFFF);
    UINT16 SVMI = (UINT16)(SIDV & 0xFFFF);
    AsciiSPrint(
        VersionString, PLATFORM_TYPE_STRING_MAX_SIZE, "%d.%d", SVMJ, SVMI);
  }

  return Status;
}

EFI_STATUS
EFIAPI
RetrieveSerialNumber(CHAR8 *SerialNumberString)
{
  EFI_CHIPINFO_PROTOCOL   *mBoardProtocol = NULL;
  EFIChipInfoSerialNumType serial;

  if (SerialNumberString == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  // Locate Qualcomm Board Protocol
  EFI_STATUS Status = gBS->LocateProtocol(
      &gEfiChipInfoProtocolGuid, NULL, (VOID *)&mBoardProtocol);

  if (EFI_ERROR(Status) || mBoardProtocol == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = mBoardProtocol->GetSerialNumber(mBoardProtocol, &serial);
  if (!EFI_ERROR(Status)) {
    AsciiSPrint(SerialNumberString, sizeof(SerialNumberString), "%lld", serial);
  }

  return Status;
}