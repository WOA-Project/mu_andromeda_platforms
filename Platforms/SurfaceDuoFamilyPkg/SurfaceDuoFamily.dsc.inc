## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

[BuildOptions.common]
  *_CLANG38_AARCH64_CC_FLAGS = -Wno-unused-const-variable -Wno-sometimes-uninitialized -Wno-unaligned-access -D CLANG
  *_*_*_CC_FLAGS = -march=armv8.2-a+crypto+rcpc -DUSE_MEMORY_FOR_SERIAL_OUTPUT=$(USE_MEMORY_FOR_SERIAL_OUTPUT) -DUSE_SCREEN_FOR_SERIAL_OUTPUT=$(USE_SCREEN_FOR_SERIAL_OUTPUT) -DSECURE_BOOT_ENABLE=$(SECURE_BOOT_ENABLE)

[BuildOptions.common.EDKII.DXE_CORE,BuildOptions.common.EDKII.DXE_DRIVER,BuildOptions.common.EDKII.UEFI_DRIVER,BuildOptions.common.EDKII.UEFI_APPLICATION]
  *_*_*_DLINK_FLAGS = -z common-page-size=0x1000

[BuildOptions.common.EDKII.DXE_RUNTIME_DRIVER]
  *_*_ARM_DLINK_FLAGS = -z common-page-size=0x1000
  *_*_AARCH64_DLINK_FLAGS = -z common-page-size=0x10000

[PcdsFeatureFlag.common]
  gArmTokenSpaceGuid.PcdCpuDxeProduceDebugSupport|FALSE

  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdTurnOffUsbLegacySupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutGopSupport|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|TRUE

  gEmbeddedTokenSpaceGuid.PcdPrePiProduceMemoryTypeInformationHob|TRUE

  gEfiMdeModulePkgTokenSpaceGuid.PcdPciBusHotplugDeviceSupport|FALSE

  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|TRUE

[PcdsFixedAtBuild.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|L"DuoWoA authors"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVersionString|L"2303.83"
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareRevision|0x01230383
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemId|"QCOM  "
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemTableId|0x324B44454D4F4351
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorId|0x4D4F4351
  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultCreatorRevision|0x00000001
  gEfiMdePkgTokenSpaceGuid.PcdMaximumUnicodeStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumAsciiStringLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdMaximumLinkedListLength|1000000
  gEfiMdePkgTokenSpaceGuid.PcdSpinLockTimeout|10000000
  gEfiMdePkgTokenSpaceGuid.PcdDebugClearMemoryValue|0xAF
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|1
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|320
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0xff
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0xffffffff
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0xff
  gEfiMdeModulePkgTokenSpaceGuid.PcdResetOnMemoryTypeInformationChange|FALSE

  #
  # Make VariableRuntimeDxe work at emulated non-volatile variable mode.
  #
  gEfiMdeModulePkgTokenSpaceGuid.PcdEmuVariableNvModeEnable|TRUE

  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIReclaimMemory|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiACPIMemoryNVS|0
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiReservedMemoryType|0
!if $(SECURE_BOOT_ENABLE) == 1
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|600
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|400
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1500
!else
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesData|300
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiRuntimeServicesCode|150
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesCode|1000
!endif
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiBootServicesData|800
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderCode|10
  gEmbeddedTokenSpaceGuid.PcdMemoryTypeEfiLoaderData|0

  ## Default Terminal Type
  ## 0-PCANSI, 1-VT100, 2-VT00+, 3-UTF8, 4-TTYTERM
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|4

  # Boot Manager
  gEfiMdePkgTokenSpaceGuid.PcdPlatformBootTimeOut|2

  # PCI Express
  gEfiMdeModulePkgTokenSpaceGuid.PcdSrIovSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdAriSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdMrIovSupport|FALSE
  gArmTokenSpaceGuid.PcdPciBusMin|0x0
  gArmTokenSpaceGuid.PcdPciBusMax|0xff

!if $(SECURE_BOOT_ENABLE) == 1
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxAuthVariableSize|0x2800
  gEfiSecurityPkgTokenSpaceGuid.PcdUserPhysicalPresence|TRUE
!endif

  # a PCD that controls the enumeration and connection of ConIn's. When true, ConIn is only connected once a console input is requests
  gEfiMdeModulePkgTokenSpaceGuid.PcdConInConnectOnDemand|FALSE

  # Enable SHELL to build instead of just taking the binary
  gEfiMdePkgTokenSpaceGuid.PcdUefiLibMaxPrintBufferSize|16000
  gEfiShellPkgTokenSpaceGuid.PcdShellProfileMask|0x1f    # All profiles

  gMsGraphicsPkgTokenSpaceGuid.PcdUiThemeInDxe|TRUE
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerInBootOrder|TRUE

  # The GUID of Frontpage.inf from MU_OEM_SAMPLE
  # 4042708A-0F2D-4823-AC60-0D77B3111889
  gEfiMdeModulePkgTokenSpaceGuid.PcdBootManagerMenuFile|{ 0x8A, 0x70, 0x42, 0x40, 0x2D, 0x0F, 0x23, 0x48, 0xAC, 0x60, 0x0D, 0x77, 0xB3, 0x11, 0x18, 0x89 }

  gOemPkgTokenSpaceGuid.PcdUefiVersionNumber|0x1230383 # 1.23.03.83
  gOemPkgTokenSpaceGuid.PcdUefiBuildDate|0x20232603 # 03/26/2023

  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000

[LibraryClasses.common]
  DebugLib                     |MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseLib                      |MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib                |MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib          |MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib                      |MdePkg/Library/UefiLib/UefiLib.inf
  PrintLib                     |MdePkg/Library/BasePrintLib/BasePrintLib.inf
  TimerLib                     |MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  PcdLib                       |MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DevicePathLib                |MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  UefiRuntimeServicesTableLib  |MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesLib               |MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  BaseBinSecurityLib           |MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
  HiiLib                       |MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf

  # Services tables/Entry points
  UefiBootServicesTableLib    |MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib |MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  UefiDriverEntryPoint        |MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint   |MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  DxeServicesLib              |MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib         |MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  UefiHiiServicesLib          |MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  RegisterFilterLib           |MdePkg/Library/RegisterFilterLibNull/RegisterFilterLibNull.inf

  # Math Libraries
  FltUsedLib |MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MathLib    |MsCorePkg/Library/MathLib/MathLib.inf
  SafeIntLib |MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  # Libraries used for test modules.
  UnitTestLib|UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestPersistenceLib|UnitTestFrameworkPkg/Library/UnitTestPersistenceLibNull/UnitTestPersistenceLibNull.inf
  UnitTestResultReportLib|UnitTestFrameworkPkg/Library/UnitTestResultReportLib/UnitTestResultReportLibDebugLib.inf

  # PCI libraries
  PciCf8Lib           |MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  PciExpressLib       |MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  PciLib              |MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  PciSegmentLib       |MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf

  # IO Libraries
  IoLib         |MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
  SerialPortLib |PcAtChipsetPkg/Library/SerialIoLib/SerialIoLib.inf
  TdxLib        |MdePkg/Library/TdxLib/TdxLib.inf
  CcProbeLib    |MdePkg/Library/CcProbeLibNull/CcProbeLibNull.inf

  # Sorter helper Libraries
  SortLib              |MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  OrderedCollectionLib |MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  # Shell Libraries
  ShellLib         |ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCommandLib  |ShellPkg/Library/UefiShellCommandLib/UefiShellCommandLib.inf
  ShellCEntryLib   |ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  HandleParsingLib |ShellPkg/Library/UefiHandleParsingLib/UefiHandleParsingLib.inf
  BcfgCommandLib   |ShellPkg/Library/UefiShellBcfgCommandLib/UefiShellBcfgCommandLib.inf

  # File Libraries
  FileExplorerLib   |MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  FileHandleLib     |MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  UefiDecompressLib |MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf

  # TPM Libraries
  OemTpm2InitLib          |SecurityPkg/Library/OemTpm2InitLibNull/OemTpm2InitLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  Tpm12CommandLib         |SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib          |SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib           |SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  Tcg2PpVendorLib         |SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
  TpmMeasurementLib       |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf

[LibraryClasses.common.PEIM]
  Tpm12DeviceLib             |SecurityPkg/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  Tpm2DeviceLib              |SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  SourceDebugEnabledLib      |SourceLevelDebugPkg/Library/SourceDebugEnabled/SourceDebugEnabledLib.inf

!if $(TOOL_CHAIN_TAG) == VS2019 or $(TOOL_CHAIN_TAG) == VS2022
[LibraryClasses.X64]
  # Provide StackCookie support lib so that we can link to /GS exports for VS builds
  RngLib|MdePkg/Library/BaseRngLib/BaseRngLib.inf
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
  NULL|MdePkg/Library/BaseBinSecurityLibRng/BaseBinSecurityLibRng.inf
!endif

[LibraryClasses.ARM, LibraryClasses.AARCH64]
  NULL|MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicArmVirt.inf

[LibraryClasses.common.PEIM]
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.DXE_DRIVER]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf

!include NetworkPkg/NetworkDefines.dsc.inc

!include MdePkg/MdeLibs.dsc.inc

[LibraryClasses.common]
!if $(TARGET) == RELEASE
  # DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!else
  DebugLib|MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
!endif
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf

  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  BmpSupportLib|MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicArmVirt.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf

  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLibDevicePathProtocol/UefiDevicePathLibDevicePathProtocol.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf

  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  OrderedCollectionLib|MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf

  #
  # Ramdisk Requirements
  #
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf

  # Allow dynamic PCDs
  #
  PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf

  # use the accelerated BaseMemoryLibOptDxe by default, overrides for SEC/PEI below
  BaseMemoryLib|MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf

  # Networking Requirements
!include NetworkPkg/NetworkLibs.dsc.inc
!if $(NETWORK_TLS_ENABLE) == TRUE
  TlsLib|CryptoPkg/Library/TlsLib/TlsLib.inf
!endif

  NonDiscoverableDeviceRegistrationLib|MdeModulePkg/Library/NonDiscoverableDeviceRegistrationLib/NonDiscoverableDeviceRegistrationLib.inf

  #
  # It is not possible to prevent the ARM compiler from inserting calls to intrinsic functions.
  # This library provides the intrinsic functions such a compiler may generate calls to.
  #
  # NULL|ArmPkg/Library/CompilerIntrinsicsLib/CompilerIntrinsicsLib.inf   # MU_CHANGE
  NULL|MdePkg/Library/CompilerIntrinsicsLib/ArmCompilerIntrinsicsLib.inf  # MU_CHANGE

  # Add support for GCC stack protector
  NULL|MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

  # ARM Architectural Libraries
  CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  DefaultExceptionHandlerLib|ArmPkg/Library/DefaultExceptionHandlerLib/DefaultExceptionHandlerLib.inf
  CpuExceptionHandlerLib|ArmPkg/Library/ArmExceptionLib/ArmExceptionLib.inf
  ArmDisassemblerLib|ArmPkg/Library/ArmDisassemblerLib/ArmDisassemblerLib.inf
  ArmGicLib|ArmPkg/Drivers/ArmGic/ArmGicLib.inf
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  ArmSmcLib|ArmPkg/Library/ArmSmcLib/ArmSmcLib.inf
  ArmHvcLib|ArmPkg/Library/ArmHvcLib/ArmHvcLib.inf
!if $(USE_PHYSICAL_TIMER) == 1
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerPhyCounterLib/ArmGenericTimerPhyCounterLib.inf
!else
  ArmGenericTimerCounterLib|ArmPkg/Library/ArmGenericTimerVirtCounterLib/ArmGenericTimerVirtCounterLib.inf
!endif

  PlatformPeiLib|ArmPlatformPkg/PlatformPei/PlatformPeiLib.inf
  MemoryInitPeiLib|SurfaceDuoFamilyPkg/MemoryInitPei/MemoryInitPeiLib.inf

  RealTimeClockLib|EmbeddedPkg/Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
  TimeBaseLib|EmbeddedPkg/Library/TimeBaseLib/TimeBaseLib.inf
!if $(USE_SCREEN_FOR_SERIAL_OUTPUT) == 1
!if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
  SerialPortLib|SurfaceDuoFamilyPkg/Library/FrameBufferInMemorySerialPortLib/FrameBufferInMemorySerialPortLib.inf
!else
  SerialPortLib|SurfaceDuoFamilyPkg/Library/FrameBufferSerialPortLib/FrameBufferSerialPortLib.inf
!endif
!else
!if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
  SerialPortLib|SurfaceDuoFamilyPkg/Library/InMemorySerialPortLib/InMemorySerialPortLib.inf
!else
  SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
!endif
!endif

  #
  # Uncomment (and comment out the next line) For RealView Debugger. The Standard IO window
  # in the debugger will show load and unload commands for symbols. You can cut and paste this
  # into the command window to load symbols. We should be able to use a script to do this, but
  # the version of RVD I have does not support scripts accessing system memory.
  #
  #PeCoffExtraActionLib|ArmPkg/Library/RvdPeCoffExtraActionLib/RvdPeCoffExtraActionLib.inf
  PeCoffExtraActionLib|ArmPkg/Library/DebugPeCoffExtraActionLib/DebugPeCoffExtraActionLib.inf
  #PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf

  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  DebugAgentTimerLib|EmbeddedPkg/Library/DebugAgentTimerLibNull/DebugAgentTimerLibNull.inf

  # Flattened Device Tree (FDT) access library
  FdtLib|EmbeddedPkg/Library/FdtLib/FdtLib.inf

  # PCI Libraries
  PciLib|MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf

  # USB Libraries
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf

  #
  # CryptoPkg libraries needed by multiple firmware features
  #
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
!if $(NETWORK_TLS_ENABLE) == TRUE
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
!else
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLibCrypto.inf
!endif
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf
  RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  Hash2CryptoLib|SecurityPkg/Library/BaseHash2CryptoLibNull/BaseHash2CryptoLibNull.inf

  #
  # Secure Boot dependencies
  #
  AuthVariableLib|SecurityPkg/Library/AuthVariableLib/AuthVariableLib.inf
  SecureBootVariableLib|SecurityPkg/Library/SecureBootVariableLib/SecureBootVariableLib.inf
  SecureBootVariableProvisionLib|SecurityPkg/Library/SecureBootVariableProvisionLib/SecureBootVariableProvisionLib.inf
  # re-use the UserPhysicalPresent() dummy implementation from the ovmf tree
  PlatformSecureLib|SurfaceDuoFamilyPkg/Library/PlatformSecureLib/PlatformSecureLib.inf

  # Security Libraries
  LockBoxLib            |MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  PasswordStoreLib      |OemPkg/Library/PasswordStoreLib/PasswordStoreLib.inf
  PasswordPolicyLib     |OemPkg/Library/PasswordPolicyLib/PasswordPolicyLib.inf
  SecureBootKeyStoreLib |SurfaceDuoFamilyPkg/Library/SecureBootKeyStoreLib/SecureBootKeyStoreLib.inf
  PlatformPKProtectionLib|SecurityPkg/Library/PlatformPKProtectionLibVarPolicy/PlatformPKProtectionLibVarPolicy.inf
  MuSecureBootKeySelectorLib|MsCorePkg/Library/MuSecureBootKeySelectorLib/MuSecureBootKeySelectorLib.inf

  # Variable Libraries
  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLib.inf
  VariablePolicyHelperLib|MdeModulePkg/Library/VariablePolicyHelperLib/VariablePolicyHelperLib.inf

  # Whea Libraries
  MsWheaEarlyStorageLib |SurfaceDuoFamilyPkg/Library/MsWheaEarlyStorageLib/MsWheaEarlyStorageLib.inf
  MuTelemetryHelperLib  |MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf

  UefiBootManagerLib|MdeModulePkg/Library/UefiBootManagerLib/UefiBootManagerLib.inf
  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

  # Unit Test Libs
  UnitTestLib             |UnitTestFrameworkPkg/Library/UnitTestLib/UnitTestLib.inf
  UnitTestBootLib         |UnitTestFrameworkPkg/Library/UnitTestBootLibNull/UnitTestBootLibNull.inf
  UnitTestPersistenceLib  |UnitTestFrameworkPkg/Library/UnitTestPersistenceLibSimpleFileSystem/UnitTestPersistenceLibSimpleFileSystem.inf
  UnitTestResultReportLib |XmlSupportPkg/Library/UnitTestResultReportJUnitFormatLib/UnitTestResultReportLib.inf

  # Base ARM libraries
  ArmLib|ArmPkg/Library/ArmLib/ArmBaseLib.inf
  ArmMmuLib|ArmPkg/Library/ArmMmuLib/ArmMmuBaseLib.inf
  # MmuLib|ArmPkg/Library/MmuLib/BaseMmuLib.inf
  ArmSvcLib|ArmPkg/Library/ArmSvcLib/ArmSvcLib.inf

  ArmPlatformLib|SurfaceDuoFamilyPkg/Library/ArmPlatformLib/ArmPlatformLib.inf

  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf

  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
  BootLogoLib|MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  PlatformBootManagerLib|MsCorePkg/Library/PlatformBootManagerLib/PlatformBootManagerLib.inf
  CustomizedDisplayLib|MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  FrameBufferBltLib|MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  FileExplorerLib|MdeModulePkg/Library/FileExplorerLib/FileExplorerLib.inf
  PciSegmentLib|MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf

  MemoryTypeInformationChangeLib|MsCorePkg/Library/MemoryTypeInformationChangeLib/MemoryTypeInformationChangeLib.inf
  BaseBinSecurityLib|MdePkg/Library/BaseBinSecurityLibNull/BaseBinSecurityLibNull.inf
  DxeMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLib/DxeMemoryProtectionHobLib.inf
  MmMemoryProtectionHobLib|MdeModulePkg/Library/MemoryProtectionHobLibNull/MmMemoryProtectionHobLibNull.inf
  VariableFlashInfoLib|MdeModulePkg/Library/BaseVariableFlashInfoLib/BaseVariableFlashInfoLib.inf
  MemoryTypeInfoSecVarCheckLib|MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf
  # ExceptionPersistenceLib|MsCorePkg/Library/ExceptionPersistenceLibCmos/ExceptionPersistenceLibCmos.inf
  SecurityLockAuditLib|MdeModulePkg/Library/SecurityLockAuditDebugMessageLib/SecurityLockAuditDebugMessageLib.inf
  ResetUtilityLib|MdeModulePkg/Library/ResetUtilityLib/ResetUtilityLib.inf
  HwResetSystemLib|ArmPkg/Library/ArmSmcPsciResetSystemLib/ArmSmcPsciResetSystemLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf
  DeviceBootManagerLib|OemPkg/Library/DeviceBootManagerLib/DeviceBootManagerLib.inf
  MsPlatformDevicesLib|SurfaceDuoFamilyPkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf
  MsNetworkDependencyLib|PcBdsPkg/Library/MsNetworkDependencyLib/MsNetworkDependencyLib.inf
  MsBootOptionsLib|SurfaceDuoFamilyPkg/Library/MsBootOptionsLib/MsBootOptionsLib.inf
  ConsoleMsgLib|PcBdsPkg/Library/ConsoleMsgLibNull/ConsoleMsgLibNull.inf
  MsBootPolicyLib|OemPkg/Library/MsBootPolicyLib/MsBootPolicyLib.inf
  MsBootManagerSettingsLib|OemPkg/Library/MsBootManagerSettingsDxeLib/MsBootManagerSettingsDxeLib.inf
  MsPlatformPowerCheckLib|PcBdsPkg/Library/MsPlatformPowerCheckLibNull/MsPlatformPowerCheckLibNull.inf
  ThermalServicesLib|PcBdsPkg/Library/ThermalServicesLibNull/ThermalServicesLibNull.inf
  PowerServicesLib|SurfaceDuoFamilyPkg/Library/PowerServicesLib/PowerServicesLib.inf
  MuUefiVersionLib|OemPkg/Library/MuUefiVersionLib/MuUefiVersionLib.inf
  MsNVBootReasonLib|OemPkg/Library/MsNVBootReasonLib/MsNVBootReasonLib.inf
  MuTelemetryHelperLib|MsWheaPkg/Library/MuTelemetryHelperLib/MuTelemetryHelperLib.inf
  UiRectangleLib|MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf
  MmUnblockMemoryLib|MdePkg/Library/MmUnblockMemoryLib/MmUnblockMemoryLibNull.inf
  ResetSystemLib|MdeModulePkg/Library/DxeResetSystemLib/DxeResetSystemLib.inf

  # Math Libraries
  FltUsedLib |MdePkg/Library/FltUsedLib/FltUsedLib.inf
  MathLib    |MsCorePkg/Library/MathLib/MathLib.inf
  SafeIntLib |MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  # UI and graphics
  BmpSupportLib            |MdeModulePkg/Library/BaseBmpSupportLib/BaseBmpSupportLib.inf
  BootLogoLib              |MdeModulePkg/Library/BootLogoLib/BootLogoLib.inf
  BootGraphicsProviderLib  |OemPkg/Library/BootGraphicsProviderLib/BootGraphicsProviderLib.inf #  uses PCDs and raw files in the firmware volumes to get Pcd
  CustomizedDisplayLib     |MdeModulePkg/Library/CustomizedDisplayLib/CustomizedDisplayLib.inf
  FrameBufferBltLib        |MdeModulePkg/Library/FrameBufferBltLib/FrameBufferBltLib.inf
  FrameBufferMemDrawLib    |MsGraphicsPkg/Library/FrameBufferMemDrawLib/FrameBufferMemDrawLib.inf
  BootGraphicsLib          |MsGraphicsPkg/Library/BootGraphicsLib/BootGraphicsLib.inf
  GraphicsConsoleHelperLib |PcBdsPkg/Library/GraphicsConsoleHelperLib/GraphicsConsoleHelper.inf
  DisplayDeviceStateLib    |MsGraphicsPkg/Library/ColorBarDisplayDeviceStateLib/ColorBarDisplayDeviceStateLib.inf # Display the On screen notifications for the platform using color bars
  UIToolKitLib             |MsGraphicsPkg/Library/SimpleUIToolKit/SimpleUIToolKit.inf
  MsColorTableLib          |MsGraphicsPkg/Library/MsColorTableLib/MsColorTableLib.inf
  SwmDialogsLib            |MsGraphicsPkg/Library/SwmDialogsLib/SwmDialogs.inf #  Dialog Boxes in a Simple Window Manager environment.
  DeviceStateLib           |MdeModulePkg/Library/DeviceStateLib/DeviceStateLib.inf
  UiRectangleLib           |MsGraphicsPkg/Library/BaseUiRectangleLib/BaseUiRectangleLib.inf
  MsUiThemeCopyLib         |MsGraphicsPkg/Library/MsUiThemeCopyLib/MsUiThemeCopyLib.inf # handles copying the theme
  MsAltBootLib             |OemPkg/Library/MsAltBootLib/MsAltBootLib.inf
  MsPlatformEarlyGraphicsLib|SurfaceDuoFamilyPkg/Library/MsPlatformEarlyGraphicsLib/MsEarlyGraphicsLib.inf

  # Setup variable libraries
  ConfigVariableListLib     |SetupDataPkg/Library/ConfigVariableListLib/ConfigVariableListLib.inf
  OemMfciLib|OemPkg/Library/OemMfciLib/OemMfciLibDxe.inf
  SvdXmlSettingSchemaSupportLib |SetupDataPkg/Library/SvdXmlSettingSchemaSupportLib/SvdXmlSettingSchemaSupportLib.inf

  # DFCI / XML / JSON Libraries
  DfciUiSupportLib                  |OemPkg/Library/DfciUiSupportLib/DfciUiSupportLib.inf # Supports DFCI Groups.
  DfciV1SupportLib                  |DfciPkg/Library/DfciV1SupportLibNull/DfciV1SupportLibNull.inf # Backwards compatibility with DFCI V1 functions.
  DfciDeviceIdSupportLib            |OemPkg/Library/DfciDeviceIdSupportLib/DfciDeviceIdSupportLib.inf
  DfciGroupLib                      |OemPkg/Library/DfciGroupLib/DfciGroups.inf
  DfciRecoveryLib                   |DfciPkg/Library/DfciRecoveryLib/DfciRecoveryLib.inf
   # Zero Touch is part of DFCI
  ZeroTouchSettingsLib              |ZeroTouchPkg/Library/ZeroTouchSettings/ZeroTouchSettings.inf
   # Libraries that understands the MsXml Settings Schema and providers helper functions
  DfciXmlIdentitySchemaSupportLib   |DfciPkg/Library/DfciXmlIdentitySchemaSupportLib/DfciXmlIdentitySchemaSupportLib.inf
  DfciXmlDeviceIdSchemaSupportLib   |DfciPkg/Library/DfciXmlDeviceIdSchemaSupportLib/DfciXmlDeviceIdSchemaSupportLib.inf
  DfciXmlSettingSchemaSupportLib    |DfciPkg/Library/DfciXmlSettingSchemaSupportLib/DfciXmlSettingSchemaSupportLib.inf
  DfciXmlPermissionSchemaSupportLib |DfciPkg/Library/DfciXmlPermissionSchemaSupportLib/DfciXmlPermissionSchemaSupportLib.inf
  DfciSettingChangedNotificationLib |DfciPkg/Library/DfciSettingChangedNotificationLib/DfciSettingChangedNotificationLibNull.inf

   #XML libraries
  XmlTreeQueryLib                   |XmlSupportPkg/Library/XmlTreeQueryLib/XmlTreeQueryLib.inf
  XmlTreeLib                        |XmlSupportPkg/Library/XmlTreeLib/XmlTreeLib.inf
   # Json parser
  JsonLiteParserLib |MsCorePkg/Library/JsonLiteParser/JsonLiteParser.inf

  # TPM Libraries
  OemTpm2InitLib          |SecurityPkg/Library/OemTpm2InitLibNull/OemTpm2InitLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibNull.inf
  Tpm12CommandLib         |SecurityPkg/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib          |SecurityPkg/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tpm2DeviceLib           |SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  Tcg2PpVendorLib         |SecurityPkg/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf
  TpmMeasurementLib       |MdeModulePkg/Library/TpmMeasurementLibNull/TpmMeasurementLibNull.inf
!if $(TPM_ENABLE) == TRUE
  TpmMeasurementLib       |SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  Tpm2DebugLib            |SecurityPkg/Library/Tpm2DebugLib/Tpm2DebugLibSimple.inf
!endif

  # Common Standalone MM libraries
  MemLib|StandaloneMmPkg/Library/StandaloneMmMemLib/StandaloneMmMemLib.inf

  AdvancedLoggerAccessLib |AdvLoggerPkg/Library/AdvancedLoggerAccessLib/AdvancedLoggerAccessLib.inf
  FmpDependencyLib|FmpDevicePkg/Library/FmpDependencyLib/FmpDependencyLib.inf
  ExceptionPersistenceLib|MdeModulePkg/Library/BaseExceptionPersistenceLibNull/BaseExceptionPersistenceLibNull.inf

  MemoryMapHelperLib|SurfaceDuoFamilyPkg/Library/MemoryMapHelperLib/MemoryMapHelperLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  LzmaDecompressLib|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  PrePiLib|EmbeddedPkg/Library/PrePiLib/PrePiLib.inf
  ArmPlatformStackLib|ArmPlatformPkg/Library/ArmPlatformStackLib/ArmPlatformStackLib.inf
  DtPlatformDtbLoaderLib|EmbeddedPkg/Library/DxeDtPlatformDtbLoaderLibDefault/DxeDtPlatformDtbLoaderLibDefault.inf

[LibraryClasses.common.DXE_CORE, LibraryClasses.common.DXE_RUNTIME_DRIVER, LibraryClasses.common.UEFI_DRIVER, LibraryClasses.common.DXE_DRIVER, LibraryClasses.common.UEFI_APPLICATION]
  MsUiThemeLib                  |MsGraphicsPkg/Library/MsUiThemeLib/Dxe/MsUiThemeLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  CheckHwErrRecHeaderLib|MsWheaPkg/Library/CheckHwErrRecHeaderLib/CheckHwErrRecHeaderLib.inf

[LibraryClasses.common.SEC]
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf

  ArmGicArchLib|ArmPkg/Library/ArmGicArchSecLib/ArmGicArchSecLib.inf
  # DebugAgentLib|ArmPkg/Library/DebugAgentSymbolsBaseLib/DebugAgentSymbolsBaseLib.inf
  HobLib|EmbeddedPkg/Library/PrePiHobLib/PrePiHobLib.inf
  MemoryAllocationLib|EmbeddedPkg/Library/PrePiMemoryAllocationLib/PrePiMemoryAllocationLib.inf
  PrePiHobListPointerLib|ArmPlatformPkg/Library/PrePiHobListPointerLib/PrePiHobListPointerLib.inf

[LibraryClasses.common.PEI_CORE]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeiCoreEntryPoint|MdePkg/Library/PeiCoreEntryPoint/PeiCoreEntryPoint.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf

  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf

[LibraryClasses.common.PEIM]
  PcdLib|MdePkg/Library/PeiPcdLib/PeiPcdLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PerformanceLib|MdeModulePkg/Library/PeiPerformanceLib/PeiPerformanceLib.inf
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  ResetSystemLib|MdeModulePkg/Library/PeiResetSystemLib/PeiResetSystemLib.inf

  PeiServicesTablePointerLib|ArmPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PcdDatabaseLoaderLib|MdeModulePkg/Library/PcdDatabaseLoaderLib/Pei/PcdDatabaseLoaderLibPei.inf

  MsPlatformEarlyGraphicsLib |MsGraphicsPkg/Library/MsEarlyGraphicsLibNull/Pei/MsEarlyGraphicsLibNull.inf
  MsUiThemeLib               |MsGraphicsPkg/Library/MsUiThemeLib/Pei/MsUiThemeLib.inf
  ArmPlatformLib             |SurfaceDuoFamilyPkg/Library/ArmPlatformLib/ArmPlatformLib.inf
  OemMfciLib                 |OemPkg/Library/OemMfciLib/OemMfciLibPei.inf
  ConfigKnobShimLib          |SetupDataPkg/Library/ConfigKnobShimLib/ConfigKnobShimPeiLib/ConfigKnobShimPeiLib.inf

!if $(TPM2_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
!endif

[LibraryClasses.common.DXE_CORE]
  ArmGicArchLib|ArmPkg/Library/ArmGicArchLib/ArmGicArchLib.inf
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  UefiDecompressLib|MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  PerformanceLib|MdeModulePkg/Library/DxeCorePerformanceLib/DxeCorePerformanceLib.inf
  MemoryBinOverrideLib|MdeModulePkg/Library/MemoryBinOverrideLibNull/MemoryBinOverrideLibNull.inf

  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf

[LibraryClasses.common.DXE_DRIVER]
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  PcdDatabaseLoaderLib|MdeModulePkg/Library/PcdDatabaseLoaderLib/Dxe/PcdDatabaseLoaderLibDxe.inf
  UpdateFacsHardwareSignatureLib|OemPkg/Library/UpdateFacsHardwareSignatureLib/UpdateFacsHardwareSignatureLib.inf
  LockBoxLib|MdeModulePkg/Library/SmmLockBoxLib/SmmLockBoxDxeLib.inf

!if $(TPM2_ENABLE) == TRUE
  Tpm2DeviceLib|SecurityPkg/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
!endif

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  CapsuleLib|MdeModulePkg/Library/DxeCapsuleLibNull/DxeCapsuleLibNull.inf
!if $(TARGET) != RELEASE
  DebugLib|MdePkg/Library/DxeRuntimeDebugLibSerialPort/DxeRuntimeDebugLibSerialPort.inf
!endif
  DebugLib|MdePkg/Library/DxeRuntimeDebugLibSerialPort/DxeRuntimeDebugLibSerialPort.inf
  VariablePolicyLib|MdeModulePkg/Library/VariablePolicyLib/VariablePolicyLibRuntimeDxe.inf
  ResetSystemLib|MdeModulePkg/Library/RuntimeResetSystemLib/RuntimeResetSystemLib.inf

!if $(SECURE_BOOT_ENABLE) == TRUE
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/RuntimeCryptLib.inf
!endif

[LibraryClasses.common.MM_CORE_STANDALONE]
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  ExtractGuidedSectionLib|EmbeddedPkg/Library/PrePiExtractGuidedSectionLib/PrePiExtractGuidedSectionLib.inf
  FvLib|StandaloneMmPkg/Library/FvLib/FvLib.inf
  HobLib|StandaloneMmPkg/Library/StandaloneMmCoreHobLib/StandaloneMmCoreHobLib.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmCoreMemoryAllocationLib/StandaloneMmCoreMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  # ArmMmuLib|ArmPkg/Library/StandaloneMmMmuLib/ArmMmuStandaloneMmLib.inf
  StandaloneMmCoreEntryPoint|ArmPkg/Library/StandaloneMmCoreEntryPoint/StandaloneMmCoreEntryPoint.inf
  PeCoffExtraActionLib|StandaloneMmPkg/Library/StandaloneMmPeCoffExtraActionLib/StandaloneMmPeCoffExtraActionLib.inf

[LibraryClasses.common.MM_STANDALONE]
  StandaloneMmDriverEntryPoint|MdePkg/Library/StandaloneMmDriverEntryPoint/StandaloneMmDriverEntryPoint.inf

  HobLib|StandaloneMmPkg/Library/StandaloneMmHobLib/StandaloneMmHobLib.inf
  MmServicesTableLib|MdePkg/Library/StandaloneMmServicesTableLib/StandaloneMmServicesTableLib.inf
  MemoryAllocationLib|StandaloneMmPkg/Library/StandaloneMmMemoryAllocationLib/StandaloneMmMemoryAllocationLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/SmmCryptLib.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf
  RngLib|MdePkg/Library/BaseRngLibTimerLib/BaseRngLibTimerLib.inf
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  VarCheckLib|MdeModulePkg/Library/VarCheckLib/VarCheckLib.inf
  TimerLib|ArmPkg/Library/ArmArchTimerLib/ArmArchTimerLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf

  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  AdvLoggerAccessLib|AdvLoggerPkg/Library/AdvLoggerMmAccessLib/AdvLoggerMmAccessLib.inf
  MemoryTypeInfoSecVarCheckLib|MdeModulePkg/Library/MemoryTypeInfoSecVarCheckLib/MemoryTypeInfoSecVarCheckLib.inf
  FltUsedLib|MdePkg/Library/FltUsedLib/FltUsedLib.inf

[LibraryClasses.common.UEFI_DRIVER]
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf
  ExtractGuidedSectionLib|MdePkg/Library/DxeExtractGuidedSectionLib/DxeExtractGuidedSectionLib.inf
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf

[BuildOptions]
!include NetworkPkg/NetworkBuildOptions.dsc.inc

[Components.common]
  # PrePI
  SurfaceDuoFamilyPkg/PrePi/PeiUniCore.inf {
    <LibraryClasses>
    !if $(USE_SCREEN_FOR_SERIAL_OUTPUT) == 1
    !if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
      SerialPortLib|SurfaceDuoFamilyPkg/Library/FrameBufferInMemorySerialPortLib/FrameBufferInMemorySerialPortLib.inf
    !else
      SerialPortLib|SurfaceDuoFamilyPkg/Library/FrameBufferSerialPortLib/FrameBufferSerialPortLib.inf
    !endif
    !else
    !if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
      SerialPortLib|SurfaceDuoFamilyPkg/Library/InMemorySerialPortLib/InMemorySerialPortLib.inf
    !else
      SerialPortLib|MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
    !endif
    !endif
  }

  # DXE
  MdeModulePkg/Core/Dxe/DxeMain.inf {
    <LibraryClasses>
    NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
    NULL|MdeModulePkg/Library/DxeCrc32GuidedSectionExtractLib/DxeCrc32GuidedSectionExtractLib.inf
    NULL|MdeModulePkg/Library/LzmaCustomDecompressLib/LzmaCustomDecompressLib.inf
  }

  # PCD Database
  MdeModulePkg/Universal/PCD/Dxe/Pcd.inf {
    <LibraryClasses>
    PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  }

  # Processor Core Services
  ArmPkg/Drivers/CpuDxe/CpuDxe.inf
  ArmPkg/Drivers/TimerDxe/TimerDxe.inf
  ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
  ArmPkg/Drivers/ArmPsciMpServicesDxe/ArmPsciMpServicesDxe.inf
  MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
  EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
  EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf {
    <LibraryClasses>
	  RealTimeClockLib|SurfaceDuoFamilyPkg/Library/VirtualRealTimeClockLib/VirtualRealTimeClockLib.inf
  }

  # Debug heartbeat
!if $(SEND_HEARTBEAT_TO_SERIAL) == 1
  SurfaceDuoFamilyPkg/Driver/HeartbeatDxe/HeartbeatDxe.inf
!endif

  # Runtime Services
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
  EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf

  # Fake Variable Services
  MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

  # Security Stub
  MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf {
    <LibraryClasses>
!if $(SECURE_BOOT_ENABLE) == 1
      NULL|SecurityPkg/Library/DxeImageVerificationLib/DxeImageVerificationLib.inf
!endif
  }

!if $(SECURE_BOOT_ENABLE) == 1
  SecurityPkg/VariableAuthenticated/SecureBootConfigDxe/SecureBootConfigDxe.inf
  SurfaceDuoFamilyPkg/Driver/SecureBootProvisioningDxe/SecureBootProvisioningDxe.inf
!endif

  MsCorePkg/MuVarPolicyFoundationDxe/MuVarPolicyFoundationDxe.inf

  # BDS
  MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
  MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
  MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf
  MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf
  MdeModulePkg/Universal/SerialDxe/SerialDxe.inf
  MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
  MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
  MdeModulePkg/Universal/SmbiosDxe/SmbiosDxe.inf
  MdeModulePkg/Universal/SetupBrowserDxe/SetupBrowserDxe.inf
  MdeModulePkg/Universal/DriverHealthManagerDxe/DriverHealthManagerDxe.inf
  MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

  # HID Support
  HidPkg/HidKeyboardDxe/HidKeyboardDxe.inf
  HidPkg/HidMouseAbsolutePointerDxe/HidMouseAbsolutePointerDxe.inf

  # Disk IO
  MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
  MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
  FatPkg/EnhancedFatDxe/Fat.inf
  MdeModulePkg/Bus/Scsi/ScsiBusDxe/ScsiBusDxe.inf
  MdeModulePkg/Bus/Scsi/ScsiDiskDxe/ScsiDiskDxe.inf

  # USB
  MdeModulePkg/Bus/Usb/UsbMouseAbsolutePointerDxe/UsbMouseAbsolutePointerDxe.inf

  # DT
  EmbeddedPkg/Drivers/DtPlatformDxe/DtPlatformDxe.inf

  EmbeddedPkg/Drivers/VirtualKeyboardDxe/VirtualKeyboardDxe.inf

  # ACPI and SMBIOS
  # We use pre-built ACPI tables, so only core components are loaded
  SurfaceDuoFamilyPkg/Driver/SmBiosTableDxe/SmBiosTableDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiTableDxe/AcpiTableDxe.inf
  MdeModulePkg/Universal/Acpi/AcpiPlatformDxe/AcpiPlatformDxe.inf
  MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf
  MdeModulePkg/Universal/Acpi/FirmwarePerformanceDataTableDxe/FirmwarePerformanceDxe.inf

  # Pci
  MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
  MdeModulePkg/Bus/Pci/NvmExpressDxe/NvmExpressDxe.inf

  MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
  MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
  EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  MdeModulePkg/Universal/EsrtFmpDxe/EsrtFmpDxe.inf

  SurfaceDuoFamilyPkg/Driver/ColorbarsDxe/ColorbarsDxe.inf
  SurfaceDuoFamilyPkg/UFP/ufpdevicefw.inf
  SurfaceDuoFamilyPkg/Driver/KernelErrataPatcher/KernelErrataPatcher.inf

  AdvLoggerPkg/AdvancedFileLogger/AdvancedFileLogger.inf
  MdeModulePkg/Universal/MemoryTest/NullMemoryTestDxe/NullMemoryTestDxe.inf

  # NetworkPkg/ArpDxe/ArpDxe.inf
  # NetworkPkg/Dhcp4Dxe/Dhcp4Dxe.inf
  # NetworkPkg/Dhcp6Dxe/Dhcp6Dxe.inf
  # NetworkPkg/DnsDxe/DnsDxe.inf
  # NetworkPkg/DpcDxe/DpcDxe.inf
  # NetworkPkg/HttpDxe/HttpDxe.inf
  # NetworkPkg/HttpUtilitiesDxe/HttpUtilitiesDxe.inf
  # NetworkPkg/Ip4Dxe/Ip4Dxe.inf
  # NetworkPkg/Ip6Dxe/Ip6Dxe.inf
  # NetworkPkg/MnpDxe/MnpDxe.inf
  # NetworkPkg/Mtftp4Dxe/Mtftp4Dxe.inf
  # NetworkPkg/Mtftp6Dxe/Mtftp6Dxe.inf
  # NetworkPkg/SnpDxe/SnpDxe.inf
  # NetworkPkg/TcpDxe/TcpDxe.inf
  # NetworkPkg/TlsDxe/TlsDxe.inf
  # NetworkPkg/Udp4Dxe/Udp4Dxe.inf
  # NetworkPkg/Udp6Dxe/Udp6Dxe.inf
  # NetworkPkg/UefiPxeBcDxe/UefiPxeBcDxe.inf
  # NetworkPkg/VlanConfigDxe/VlanConfigDxe.inf

  CryptoPkg/Driver/CryptoDxe.inf
