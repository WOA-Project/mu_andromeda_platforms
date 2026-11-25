/** @file
  Save the early display capture to the ESP without requiring a working screen.

  SPDX-License-Identifier: MIT
**/
#include <Uefi.h>

#include <Guid/DisplayResumeDiagnostic.h>
#include <Guid/FileInfo.h>
#include <Guid/Gpt.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/EFIPmicVersion.h>
#include <Protocol/PartitionInfo.h>
#include <Protocol/QcomPmicVreg.h>
#include <Protocol/SimpleFileSystem.h>

STATIC EFI_EVENT mFileSystemEvent;
STATIC VOID *mRegistration;
STATIC CHAR8 *mReport;
STATIC UINTN mReportSize;
STATIC BOOLEAN mSaving;

STATIC VOID AppendPmicReport(UINTN Capacity)
{
  STATIC CONST struct {
    EFI_PM_MODEL_TYPE Model;
    UINT32 Id;
    CONST CHAR8 *Name;
  } Rails[] = {
    {EFI_PMIC_IS_PM8150, 37, "PM8150 S4 panel vddi"},
    {EFI_PMIC_IS_PM8150, 16, "PM8150 L17 panel vpnl"},
    {EFI_PMIC_IS_PM8150, 4,  "PM8150 L5 DSI PHY 0p9"},
    {EFI_PMIC_IS_PM8150L, 2, "PM8150L L3 DSI 1p2"},
    {EFI_PMIC_IS_PM8150L, 38, "PM8150L S5 MMCX"},
  };
  EFI_QCOM_PMIC_VERSION_PROTOCOL *Version;
  PM_VREG_PROTOCOL *Vreg;
  EFI_PM_DEVICE_INFO_TYPE Info;
  PM_VREG_STATUS RailStatus;
  EFI_STATUS VersionStatus;
  EFI_STATUS VregStatus;
  EFI_STATUS Status;
  UINT32 Voltage;
  UINT32 Pmic;
  UINTN Index;

  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize,
      "\r\nDXE PMIC state (before DisplayDxe, not the PrePi snapshot):\r\n"
      "Voltages are programmed levels, not measurements. "
      "Software enable may not reflect RPMh votes.\r\n");
  VersionStatus = gBS->LocateProtocol(
      &gQcomPmicVersionProtocolGuid, NULL, (VOID **)&Version);
  VregStatus = gBS->LocateProtocol(
      &gQcomPmicVregProtocolGuid, NULL, (VOID **)&Vreg);
  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize,
      "Version protocol: %r; VREG protocol: %r\r\n", VersionStatus, VregStatus);
  if (EFI_ERROR(VersionStatus)) {
    return;
  }
  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize,
      "Version revision: 0x%016lx\r\n", Version->Revision);
  if (Version->Revision < 0x10001 || Version->GetPmicInfo == NULL) {
    mReportSize += AsciiSPrint(
        mReport + mReportSize, Capacity - mReportSize,
        "PMIC identification unavailable: unsupported version protocol.\r\n");
    return;
  }
  if (!EFI_ERROR(VregStatus)) {
    mReportSize += AsciiSPrint(
        mReport + mReportSize, Capacity - mReportSize,
        "VREG revision: 0x%016lx\r\n", Vreg->Revision);
    if (Vreg->Revision < 0x10005 ||
        Vreg->VregGetLevel == NULL || Vreg->VregGetStatus == NULL) {
      VregStatus = EFI_UNSUPPORTED;
      mReportSize += AsciiSPrint(
          mReport + mReportSize, Capacity - mReportSize,
          "Rail queries unavailable: unsupported VREG protocol.\r\n");
    }
  }

  for (Pmic = 0; Pmic < 8; Pmic++) {
    ZeroMem(&Info, sizeof(Info));
    Status = Version->GetPmicInfo(Pmic, &Info);
    if (EFI_ERROR(Status)) {
      mReportSize += AsciiSPrint(
          mReport + mReportSize, Capacity - mReportSize,
          "PMIC %u identification: %r\r\n", Pmic, Status);
      continue;
    }
    mReportSize += AsciiSPrint(
        mReport + mReportSize, Capacity - mReportSize,
        "PMIC %u model=0x%x all-layer=0x%x metal=0x%x\r\n",
        Pmic, Info.PmicModel, Info.PmicAllLayerRevision, Info.PmicMetalRevision);
    if (EFI_ERROR(VregStatus)) {
      continue;
    }
    for (Index = 0; Index < ARRAY_SIZE(Rails); Index++) {
      if (Info.PmicModel != Rails[Index].Model) {
        continue;
      }
      ZeroMem(&RailStatus, sizeof(RailStatus));
      Status = Vreg->VregGetStatus(Pmic, Rails[Index].Id, &RailStatus);
      mReportSize += AsciiSPrint(
          mReport + mReportSize, Capacity - mReportSize,
          "  %a status=%r", Rails[Index].Name, Status);
      if (!EFI_ERROR(Status)) {
        mReportSize += AsciiSPrint(
            mReport + mReportSize, Capacity - mReportSize,
            " sw_enable=%u ready=%u pin_control=%u mode=%u pull_down=%u",
            RailStatus.SwEnable, RailStatus.VregOk, RailStatus.PinCtrled,
            RailStatus.SwMode, RailStatus.PullDown);
      }
      Voltage = 0;
      Status = Vreg->VregGetLevel(Pmic, Rails[Index].Id, &Voltage);
      mReportSize += AsciiSPrint(
          mReport + mReportSize, Capacity - mReportSize, " level=%r", Status);
      if (!EFI_ERROR(Status)) {
        mReportSize += AsciiSPrint(
            mReport + mReportSize, Capacity - mReportSize, " %u uV", Voltage);
      }
      mReportSize += AsciiSPrint(mReport + mReportSize, Capacity - mReportSize, "\r\n");
    }
  }
}

STATIC EFI_STATUS SaveReport(EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem)
{
  EFI_FILE_PROTOCOL *Root;
  EFI_FILE_PROTOCOL *File;
  EFI_FILE_INFO *Info;
  EFI_STATUS Status;
  EFI_STATUS CloseStatus;
  UINTN Size;

  Status = FileSystem->OpenVolume(FileSystem, &Root);
  if (EFI_ERROR(Status)) {
    return Status;
  }

  Status = Root->Open(
      Root, &File, L"DisplayResume.log",
      EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE, 0);
  CloseStatus = Root->Close(Root);
  if (EFI_ERROR(CloseStatus)) {
    DEBUG((DEBUG_ERROR, "DisplayResume: closing ESP root failed: %r\n", CloseStatus));
  }
  if (EFI_ERROR(Status)) {
    return Status;
  }
  if (EFI_ERROR(CloseStatus)) {
    Status = File->Close(File);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "DisplayResume: closing report failed: %r\n", Status));
    }
    return CloseStatus;
  }

  Status = File->SetPosition(File, 0);
  if (!EFI_ERROR(Status)) {
    Size = mReportSize;
    Status = File->Write(File, &Size, mReport);
    if (!EFI_ERROR(Status) && Size != mReportSize) {
      Status = EFI_DEVICE_ERROR;
    }
  }
  if (!EFI_ERROR(Status)) {
    Info = NULL;
    Size = 0;
    Status = File->GetInfo(File, &gEfiFileInfoGuid, &Size, NULL);
    if (Status == EFI_BUFFER_TOO_SMALL) {
      if (Size < SIZE_OF_EFI_FILE_INFO + sizeof(CHAR16)) {
        Status = EFI_COMPROMISED_DATA;
      } else {
        Info = AllocatePool(Size);
        if (Info == NULL) {
          Status = EFI_OUT_OF_RESOURCES;
        } else {
          Status = File->GetInfo(File, &gEfiFileInfoGuid, &Size, Info);
        }
      }
    } else if (!EFI_ERROR(Status)) {
      Status = EFI_DEVICE_ERROR;
    }
    if (!EFI_ERROR(Status) && Info->FileSize != mReportSize) {
      Info->FileSize = mReportSize;
      // Zero timestamps tell SetInfo to preserve them, even if the RTC was invalid.
      ZeroMem(&Info->CreateTime, sizeof(Info->CreateTime));
      ZeroMem(&Info->LastAccessTime, sizeof(Info->LastAccessTime));
      ZeroMem(&Info->ModificationTime, sizeof(Info->ModificationTime));
      Status = File->SetInfo(File, &gEfiFileInfoGuid, Size, Info);
    }
    if (Info != NULL) {
      FreePool(Info);
    }
  }
  if (!EFI_ERROR(Status)) {
    Status = File->Flush(File);
  }

  CloseStatus = File->Close(File);
  if (EFI_ERROR(CloseStatus)) {
    DEBUG((DEBUG_ERROR, "DisplayResume: closing report failed: %r\n", CloseStatus));
  }
  return EFI_ERROR(Status) ? Status : CloseStatus;
}

STATIC VOID EFIAPI FileSystemAvailable(EFI_EVENT Event, VOID *Context)
{
  EFI_HANDLE *Handles;
  EFI_PARTITION_INFO_PROTOCOL *Partition;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *FileSystem;
  EFI_STATUS Status;
  UINTN Count;
  UINTN Index;

  if (mSaving || mReport == NULL) {
    return;
  }
  mSaving = TRUE;
  Status = gBS->LocateHandleBuffer(
      ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &Count, &Handles);
  if (Status == EFI_NOT_FOUND) {
    mSaving = FALSE;
    return;
  }
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "DisplayResume: filesystem enumeration failed: %r\n", Status));
    mSaving = FALSE;
    return;
  }

  for (Index = 0; Index < Count; Index++) {
    Status = gBS->HandleProtocol(
        Handles[Index], &gEfiPartitionInfoProtocolGuid, (VOID **)&Partition);
    if (Status == EFI_UNSUPPORTED) {
      continue;
    }
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "DisplayResume: partition lookup failed: %r\n", Status));
      continue;
    }
    if (Partition->Type != PARTITION_TYPE_GPT ||
        !CompareGuid(&Partition->Info.Gpt.PartitionTypeGUID, &gEfiPartTypeSystemPartGuid)) {
      continue;
    }

    Status = gBS->HandleProtocol(
        Handles[Index], &gEfiSimpleFileSystemProtocolGuid, (VOID **)&FileSystem);
    if (!EFI_ERROR(Status)) {
      Status = SaveReport(FileSystem);
    }
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "DisplayResume: saving DisplayResume.log failed: %r\n", Status));
      continue;
    }

    DEBUG((DEBUG_INFO, "DisplayResume: saved DisplayResume.log to the ESP\n"));
    FreePool(mReport);
    mReport = NULL;
    Status = gBS->CloseEvent(mFileSystemEvent);
    if (EFI_ERROR(Status)) {
      DEBUG((DEBUG_ERROR, "DisplayResume: closing filesystem event failed: %r\n", Status));
    }
    break;
  }
  FreePool(Handles);
  mSaving = FALSE;
}

EFI_STATUS EFIAPI DisplayResumeDiagnosticDxeInitialize(
    EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable)
{
  CONST DISPLAY_RESUME_DIAGNOSTIC *Diagnostic;
  CONST DISPLAY_DIAGNOSTIC_REGISTER *Register;
  VOID *Hob;
  EFI_STATUS Status;
  UINTN Capacity;
  UINTN Index;

  Hob = GetFirstGuidHob(&gDisplayResumeDiagnosticHobGuid);
  if (Hob == NULL) {
    DEBUG((DEBUG_ERROR, "DisplayResume: early display capture HOB is missing\n"));
    return EFI_NOT_FOUND;
  }
  if (GET_GUID_HOB_DATA_SIZE(Hob) != sizeof(*Diagnostic)) {
    DEBUG((DEBUG_ERROR, "DisplayResume: invalid capture HOB size\n"));
    return EFI_COMPROMISED_DATA;
  }
  Diagnostic = GET_GUID_HOB_DATA(Hob);
  if (Diagnostic->Signature != DISPLAY_DIAGNOSTIC_SIGNATURE ||
      Diagnostic->Version != DISPLAY_DIAGNOSTIC_VERSION ||
      Diagnostic->RegisterCount > DISPLAY_DIAGNOSTIC_MAX_REGISTERS ||
      Diagnostic->MdssAccessible > 1 ||
      Diagnostic->FramebufferSamples.SampleCount > DISPLAY_FRAMEBUFFER_SAMPLES ||
      Diagnostic->FramebufferSamples.NonzeroSamples > Diagnostic->FramebufferSamples.SampleCount ||
      Diagnostic->FramebufferSamples.Transitions > Diagnostic->FramebufferSamples.SampleCount) {
    DEBUG((DEBUG_ERROR, "DisplayResume: invalid early display capture\n"));
    return EFI_COMPROMISED_DATA;
  }
  for (Index = 0; Index < Diagnostic->RegisterCount; Index++) {
    if (Diagnostic->Registers[Index].Read > 1) {
      DEBUG((DEBUG_ERROR, "DisplayResume: invalid register capture state\n"));
      return EFI_COMPROMISED_DATA;
    }
  }
  for (Index = 0; Index < DISPLAY_XBL_CHUNKS; Index++) {
    CONST DISPLAY_MEMORY_SAMPLES *Samples = &Diagnostic->XblDisplaySamples[Index];
    if (Samples->SampleCount != DISPLAY_FRAMEBUFFER_SAMPLES ||
        Samples->NonzeroSamples > Samples->SampleCount ||
        Samples->Transitions >= Samples->SampleCount) {
      DEBUG((DEBUG_ERROR, "DisplayResume: invalid XBL display samples\n"));
      return EFI_COMPROMISED_DATA;
    }
  }

  Capacity = 16384 + Diagnostic->RegisterCount * 64;
  mReport = AllocatePool(Capacity);
  if (mReport == NULL) {
    DEBUG((DEBUG_ERROR, "DisplayResume: report allocation failed\n"));
    return EFI_OUT_OF_RESOURCES;
  }
  mReportSize = AsciiSPrint(
      mReport, Capacity,
      "SM8150 early display capture, version %u\r\n"
      "Diagnostic build: " __DATE__ " " __TIME__ "\r\n"
      "Initial capture before PrePi DSI disable and SMMU detach.\r\n"
      "Each DSI control value comes from the existing read immediately before its disable.\r\n"
      "Capture status: %r\r\nMDSS accessible: %u\r\n"
      "Registers: %u\r\n"
      "GCC AHB CLK_OFF may indicate hardware gating, not a missing enable vote.\r\n"
      "GDSC: 0x0AF03000, retention bit 11, power-on bit 31.\r\n"
      "UEFI framebuffer sampling status: %r; reserved base=0x%016lx size=0x%016lx\r\n"
      "Configured width=%u height=%u bpp=%u; samples=%u nonzero=%u "
      "transitions=%u checksum=0x%08x\r\n"
      "UEFI framebuffer samples are aggregates; the XBL framebuffer may differ.\r\n",
      Diagnostic->Version, Diagnostic->CaptureStatus,
      Diagnostic->MdssAccessible, Diagnostic->RegisterCount,
      Diagnostic->FramebufferStatus, Diagnostic->FramebufferAddress,
      Diagnostic->FramebufferRegionSize, Diagnostic->FramebufferWidth,
      Diagnostic->FramebufferHeight, Diagnostic->FramebufferBitsPerPixel,
      Diagnostic->FramebufferSamples.SampleCount, Diagnostic->FramebufferSamples.NonzeroSamples,
      Diagnostic->FramebufferSamples.Transitions, Diagnostic->FramebufferSamples.Checksum);
  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize,
      "\r\nEpsilon XBL display reservation (configured range, active framebuffer unknown):\r\n"
      "base=0x%016lx size=0x%08x; %u evenly spaced 32-bit samples per 4 MiB chunk.\r\n"
      "Captured before PrePi heap initialization; earlier changes are possible.\r\n",
      Diagnostic->XblDisplayAddress, DISPLAY_XBL_CHUNKS * DISPLAY_XBL_CHUNK_SIZE,
      DISPLAY_FRAMEBUFFER_SAMPLES);
  for (Index = 0; Index < DISPLAY_XBL_CHUNKS; Index++) {
    CONST DISPLAY_MEMORY_SAMPLES *Samples = &Diagnostic->XblDisplaySamples[Index];
    mReportSize += AsciiSPrint(
        mReport + mReportSize, Capacity - mReportSize,
        "  base=0x%016lx samples=%u nonzero=%u transitions=%u checksum=0x%08x"
        " first=0x%08x last=0x%08x\r\n",
        Diagnostic->XblDisplayAddress + Index * DISPLAY_XBL_CHUNK_SIZE,
        Samples->SampleCount, Samples->NonzeroSamples, Samples->Transitions,
        Samples->Checksum, Samples->FirstValue, Samples->LastValue);
  }
  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize, "\r\nADDRESS          VALUE    STATE\r\n");
  for (Index = 0; Index < Diagnostic->RegisterCount; Index++) {
    Register = &Diagnostic->Registers[Index];
    mReportSize += AsciiSPrint(
        mReport + mReportSize, Capacity - mReportSize,
        "%016lx %08x %a\r\n", Register->Address, Register->Value,
        Register->Read ? "read" : "skipped (access not verified)");
  }

  AppendPmicReport(Capacity);
  mReportSize += AsciiSPrint(
      mReport + mReportSize, Capacity - mReportSize, "\r\nEnd of DisplayResume report.\r\n");

  Status = gBS->CreateEvent(
      EVT_NOTIFY_SIGNAL, TPL_CALLBACK, FileSystemAvailable, NULL, &mFileSystemEvent);
  if (!EFI_ERROR(Status)) {
    Status = gBS->RegisterProtocolNotify(
        &gEfiSimpleFileSystemProtocolGuid, mFileSystemEvent, &mRegistration);
    if (EFI_ERROR(Status)) {
      EFI_STATUS CloseStatus;
      CloseStatus = gBS->CloseEvent(mFileSystemEvent);
      if (EFI_ERROR(CloseStatus)) {
        DEBUG((DEBUG_ERROR, "DisplayResume: closing filesystem event failed: %r\n", CloseStatus));
      }
    }
  }
  if (EFI_ERROR(Status)) {
    DEBUG((DEBUG_ERROR, "DisplayResume: registering filesystem notification failed: %r\n", Status));
    FreePool(mReport);
    mReport = NULL;
    return Status;
  }

  FileSystemAvailable(mFileSystemEvent, NULL);
  return EFI_SUCCESS;
}
