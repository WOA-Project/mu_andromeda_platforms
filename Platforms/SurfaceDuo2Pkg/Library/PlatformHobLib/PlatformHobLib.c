#include "PlatformHobLibInternal.h"
#include <Library/PlatformHobLib.h>
#include <Protocol/EFIKernelInterface.h>

STATIC
EFI_STATUS
CfgGetMemInfoByName(
    CHAR8 *RegionName, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      gDeviceMemoryDescriptorEx;

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    if (AsciiStriCmp(RegionName, MemoryDescriptorEx->Name) == 0) {
      *MemRegions = *MemoryDescriptorEx;
      return EFI_SUCCESS;
    }
    MemoryDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
CfgGetMemInfoByAddress(
    UINT64 RegionBaseAddress, ARM_MEMORY_REGION_DESCRIPTOR_EX *MemRegions)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      gDeviceMemoryDescriptorEx;

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    if (MemoryDescriptorEx->Address == RegionBaseAddress) {
      *MemRegions = *MemoryDescriptorEx;
      return EFI_SUCCESS;
    }
    MemoryDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
CfgGetCfgInfoString(CHAR8 *Key, CHAR8 *Value, UINTN *ValBuffSize)
{
  if (AsciiStriCmp(Key, "OsTypeString") == 0) {
    AsciiStrCpyS(Value, *ValBuffSize, "LA");
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
CfgGetCfgInfoVal(CHAR8 *Key, UINT32 *Value)
{
  PCONFIGURATION_DESCRIPTOR_EX ConfigurationDescriptorEx =
      gDeviceConfigurationDescriptorEx;

  // Run through each configuration descriptor
  while (ConfigurationDescriptorEx->Value != 0xFFFFFFFF) {
    if (AsciiStriCmp(Key, ConfigurationDescriptorEx->Name) == 0) {
      *Value = (UINT32)(ConfigurationDescriptorEx->Value & 0xFFFFFFFF);
      return EFI_SUCCESS;
    }
    ConfigurationDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

STATIC
EFI_STATUS
CfgGetCfgInfoVal64(CHAR8 *Key, UINT64 *Value)
{
  PCONFIGURATION_DESCRIPTOR_EX ConfigurationDescriptorEx =
      gDeviceConfigurationDescriptorEx;

  // Run through each configuration descriptor
  while (ConfigurationDescriptorEx->Value != 0xFFFFFFFF) {
    if (AsciiStriCmp(Key, ConfigurationDescriptorEx->Name) == 0) {
      *Value = ConfigurationDescriptorEx->Value;
      return EFI_SUCCESS;
    }
    ConfigurationDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

STATIC
UINTN
SFlush(VOID) { return EFI_SUCCESS; }

STATIC
UINTN
SControl(IN UINTN Arg, IN UINTN Param) { return EFI_SUCCESS; }

STATIC
EFI_STATUS
ShInstallLib(IN CHAR8 *LibName, IN UINT32 LibVersion, IN VOID *LibIntf)
{
  return EFI_SUCCESS;
}

UefiCfgLibType ConfigLib = {0x00010002,          CfgGetMemInfoByName,
                            CfgGetCfgInfoString, CfgGetCfgInfoVal,
                            CfgGetCfgInfoVal64,  CfgGetMemInfoByAddress};

SioPortLibType SioLib = {
    0x00010001, SerialPortRead, SerialPortWrite, NULL,
    SFlush,     NULL,           SControl,        SerialPortSetAttributes,
};

STATIC
EFI_STATUS
ShLoadLib(CHAR8 *LibName, UINT32 LibVersion, VOID **LibIntf)
{
  if (LibIntf == NULL)
    return EFI_NOT_FOUND;

  if (AsciiStriCmp(LibName, "UEFI Config Lib") == 0) {
    *LibIntf = &ConfigLib;
    return EFI_SUCCESS;
  }

  if (AsciiStriCmp(LibName, "SerialPort Lib") == 0) {
    *LibIntf = &SioLib;
    return EFI_SUCCESS;
  }

  return EFI_NOT_FOUND;
}

ShLibLoaderType      ShLib      = {0x00010001, ShInstallLib, ShLoadLib};
EFI_KERNEL_PROTOCOL *pSchedIntf = (EFI_KERNEL_PROTOCOL *)0x9FC37980;

STATIC
VOID BuildMemHobForFv(IN UINT16 Type)
{
  EFI_PEI_HOB_POINTERS      HobPtr;
  EFI_HOB_FIRMWARE_VOLUME2 *Hob = NULL;

  HobPtr.Raw = GetHobList();
  while ((HobPtr.Raw = GetNextHob(Type, HobPtr.Raw)) != NULL) {
    if (Type == EFI_HOB_TYPE_FV2) {
      Hob = HobPtr.FirmwareVolume2;
      /* Build memory allocation HOB to mark it as BootServicesData */
      BuildMemoryAllocationHob(
          Hob->BaseAddress, EFI_SIZE_TO_PAGES(Hob->Length) * EFI_PAGE_SIZE,
          EfiBootServicesData);
    }
    HobPtr.Raw = GET_NEXT_HOB(HobPtr);
  }
}

STATIC GUID gEfiShLibHobGuid   = EFI_SHIM_LIBRARY_GUID;
STATIC GUID gEfiSchedIntfGuid  = EFI_SCHEDULER_INTERFACE_GUID;
STATIC GUID gEfiInfoBlkHobGuid = EFI_INFORMATION_BLOCK_GUID;

VOID PlatformSchedulerInit()
{
  UINT32 SchedulingLibVersion = pSchedIntf->GetLibVersion();
  UINT32 MajorVersion         = (SchedulingLibVersion >> 16) & 0xFFFF;
  UINT32 MinorVersion         = SchedulingLibVersion & 0xFFFF;

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD, "Scheduling Library Version %d.%d\n",
       MajorVersion, MinorVersion));

  UINT32 MaxCpuCount   = pSchedIntf->MpCpu->MpcoreGetMaxCpuCount();
  UINT32 AvailCpuCount = pSchedIntf->MpCpu->MpcoreGetAvailCpuCount();

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Maximum CPU Count: %d\n", MaxCpuCount));
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Available CPU Count: %d\n", AvailCpuCount));

  for (UINT32 i = 0; i < MaxCpuCount; i++) {
    if (pSchedIntf->MpCpu->MpcoreIsCpuActive(i)) {
      DEBUG((EFI_D_INFO | EFI_D_LOAD, "CPU %d is online.\n", i));

      if (i != 0) {
        DEBUG((EFI_D_INFO | EFI_D_LOAD, "Shutting down CPU %d...\n", i));
        pSchedIntf->MpCpu->MpcorePowerOffCpu(1 << i);
      }
    }
    else {
      DEBUG((EFI_D_INFO | EFI_D_LOAD, "CPU %d is offline.\n", i));
    }
  }
}

VOID InstallPlatformHob()
{
  static int initialized = 0;

  if (!initialized) {
    PlatformSchedulerInit();

    UINTN Data  = (UINTN)&ShLib;
    UINTN Data2 = (UINTN)pSchedIntf;
    UINTN Data3 = 0x9FFFF000;

    BuildMemHobForFv(EFI_HOB_TYPE_FV2);
    BuildGuidDataHob(&gEfiShLibHobGuid, &Data, sizeof(Data));
    BuildGuidDataHob(&gEfiSchedIntfGuid, &Data2, sizeof(Data2));
    BuildGuidDataHob(&gEfiInfoBlkHobGuid, &Data3, sizeof(Data3));

    initialized = 1;
  }
}
