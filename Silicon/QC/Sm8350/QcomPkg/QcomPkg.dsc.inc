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

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address, 2GB Base
  gAndromedaPkgTokenSpaceGuid.PcdCpuVectorBaseAddress|0x9FF8C000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x17A00000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x17A60000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008350
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x9FF90000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x09BC0000
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gAndromedaPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 888 @ 2.84 GHz"
  gAndromedaPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8350"

  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3

  # UART
  gAndromedaPkgTokenSpaceGuid.PcdUartSerialBase|0x0098C000

  # Keep no Stream IDs attached by default
  gAndromedaPkgTokenSpaceGuid.PcdSMMUSIDsToKeep|{0x0}

[PcdsDynamicDefault.common.DEFAULT]
  # Zeta Product ID: 68
  # gMsSurfaceCorePkgTokenSpaceGuid.PcdMfgModeRequiredEKU|"1.3.6.1.4.1.311.76.9.14.68"
  # gMsSurfaceCorePkgTokenSpaceGuid.PcdPkRequiredEKU|"1.3.6.1.4.1.311.76.9.14.68"
  gQcomPkgTokenSpaceGuid.PcdTestMfgModeRequiredEKU|"1.3.6.1.4.1.311.76.9.17"
  gQcomPkgTokenSpaceGuid.PcdTkRequiredEKU|"1.3.6.1.4.1.311.76.9.17"

[LibraryClasses.common]
  PlatformMemoryMapLib|QcomPkg/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  PlatformConfigurationMapLib|QcomPkg/Library/PlatformConfigurationMapLib/PlatformConfigurationMapLib.inf
  PlatformPeiLib|QcomPkg/PlatformPei/PlatformPeiLib.inf
  PlatformPrePiLib|QcomPkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  AslUpdateLib|AndromedaPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf
  RFSProtectionLib|QcomPkg/Library/RFSProtectionLib/RFSProtectionLib.inf

  AcpiPlatformUpdateLib|QcomPkg/Library/AcpiPlatformUpdateLib/AcpiPlatformUpdateLib.inf
  PlatformThemeLib|QcomPkg/Library/PlatformThemeLib/PlatformThemeLib.inf

  PlatformHobLib|QcomPkg/Library/PlatformHobLib/PlatformHobLib.inf

  QcomMmuDetachHelperLib|AndromedaPkg/Library/QcomMmuDetachHelperLib/QcomMmuDetachHelperLib.inf

[Components.common]
  Arcata/BOOT.MXF.1.1.c1-00127-MAKENA-2/ArcataPkg/GpioButtonsDxe/GpioButtons.inf
  QcomPkg/PatchedBinaries/ClockDxe.inf
  QcomPkg/PatchedBinaries/MinidumpTADxe.inf
  QcomPkg/PatchedBinaries/MsAbstractLayer.inf
  QcomPkg/PatchedBinaries/MsManufacturingMode.inf
  QcomPkg/PatchedBinaries/PmicDxeLa.inf
  QcomPkg/PatchedBinaries/QcomWDogDxe.inf
  QcomPkg/PatchedBinaries/SurfaceTouchHidDxeV2.inf
  QcomPkg/PatchedBinaries/UsbConfigDxe.inf
  AndromedaPkg/Driver/GpioButtons/GpioButtons.inf
  AndromedaPkg/Driver/SimpleFbDxe/SimpleFbDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/MsCommonPkg/Drivers/MsEnvDxe/MsEnvDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/MsCommonPkg/Drivers/MsTouchPointDxe/MsTouchPoint.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/MsCommonPkg/Drivers/TouchControllerInitDxe/TouchControllerInitDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/MsCommonPkg/Drivers/TouchUtilsDxe/TouchUtilsDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/FontDxe/FontDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/FvUtilsDxe/FvUtilsDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ICBDxe/ICBDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/IPCCDxe/IPCCDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ParserDxe/ParserDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/PILDxe/PILDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/PmicGlinkDxe/PmicGlinkDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SerialPortDxe/SerialPortDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/SPSSDxe/SPSSDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/TsensDxe/TsensDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UFSDxe/UFSDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/VcsDxe/VcsDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/ExtFgDxe/ExtFgDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/MsDebugDxe/MsDebugDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/MsDfciDxe/MsDfciDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/MsFRUDxe/MsFRUDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/MsPmicPonInfoDxe/MsPmicPonInfoDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/MsXblHlosHobDxe/MsXblHlosHobDxe.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/SfpdUtils/SfpdUtils.inf
  Zeta/BOOT.MXF.1.0-00893-LAHAINA-1.7786.2/Sm8350FamilyPkg/Drivers/TimestampDxe/TimestampDxe.inf