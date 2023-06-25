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
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x9ff8c000
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|19200000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|17
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|18
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x17a00000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x17a60000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008150
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x9ffB0000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00020000      # 128K stack
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x09bc0000
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 855 @ 2.84 GHz"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8150"

  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # 2GB Base
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3

  # UART
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdUartSerialBase|0x00A90000

[LibraryClasses.common]
  PlatformMemoryMapLib|QcomPkg/Library/PlatformMemoryMapLib/PlatformMemoryMapLib.inf
  PlatformPeiLib|QcomPkg/PlatformPei/PlatformPeiLib.inf
  PlatformPrePiLib|QcomPkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  AslUpdateLib|SurfaceDuoFamilyPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf
  RFSProtectionLib|QcomPkg/Library/RFSProtectionLib/RFSProtectionLib.inf

  AcpiPlatformUpdateLib|QcomPkg/Library/AcpiPlatformUpdateLib/AcpiPlatformUpdateLib.inf
  PlatformThemeLib|QcomPkg/Library/PlatformThemeLib/PlatformThemeLib.inf

  PlatformHobLib|QcomPkg/Library/PlatformHobLib/PlatformHobLib.inf
  ConsoleMsgLib|QcomPkg/Library/ConsoleMsgLib/ConsoleMsgLib.inf

[Components.common]
  # Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/PciHostBridgeDxe/PciHostBridgeDxe.inf
  # Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf
  # Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf
  # Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf
  # Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/XhciDxe/XhciDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/FmpDevicePkg/FmpDxe/05472C14-ACB9-4458-A354-A68357AC3BF0.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/DisplayEngineDxe/DisplayEngineDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/MsUiTheme/Dxe/MsUiThemeProtocol.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/OnScreenKeyboardDxe/OnScreenKeyboardDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/RenderingEngineDxe/RenderingEngineDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/SimpleWindowManagerDxe/SimpleWindowManagerDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CipherDxe/CipherDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HashDxe/HashDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/MpPowerDxe/MpPowerDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomBds/QcomBds.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RNGDxe/RngDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UFSDxe/UFSDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/VariableDxe/VariableDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/ColorbarsDxe/ColorbarsDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/UfsSecurityDxe/UfsSecurityDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf
  # Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
  # QcomPkg/PatchedBinaries/ColorbarsDxe.inf
  # QcomPkg/PatchedBinaries/PciHostBridgeDxe.inf
  # SharedCryptoPkg/Driver/SharedCryptoDxe.inf
  Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
  Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxe.inf
  Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf
  Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/EpsilonPkg/GpioButtonsDxe/GpioButtons.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsSurfaceCorePkg/SurfaceTouchHidDxe/SurfaceTouchHidDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Application/QcomChargerApp/QcomChargerApp.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/AdcDxe/AdcDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ClockDxe/ClockDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/EnvDxe/EnvDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/FontDxe/FontDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PdcDxe/PdcDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PILDxe/PILDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomMpmTimerDxe/QcomMpmTimerDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TsensDxe/TsensDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TzDxe/ScmDxeLA.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TzDxe/TzDxeLA.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/SharedCryptoDriverDxeMu/SharedCryptoDriverDxeMu.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/ExtFgDxe/ExtFgDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/MsEnvDxe/MsEnvDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/MsManufacturingModeDxe/MsManufacturingMode.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/SfpdUtils/SfpdUtils.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/TouchControllerInitDxe/TouchControllerInitDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/VarLockNullDxe/VarLockNullDxe.inf
  Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Variable/UefiVariablePolicy/Dxe/VariablePolicyNullDxe.inf
  QcomPkg/PatchedBinaries/05472C14-ACB9-4458-A354-A68357AC3BF0.inf
  QcomPkg/PatchedBinaries/DisplayDxe.inf
  QcomPkg/PatchedBinaries/HALIOMMUDxe.inf
  QcomPkg/PatchedBinaries/UFSDxe.inf
  QcomPkg/PatchedBinaries/UsbConfigDxe.inf