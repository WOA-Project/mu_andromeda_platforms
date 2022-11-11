
    # First, install the PCD driver and call DxeInit
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/EnvDxe/EnvDxe.inf

    INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

    # Next, install cpu protocol and enable the interrupt controller
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

    # Core Drivers
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf

    FILE DRIVER = 9a00771f-36d4-4dd5-8916-c48ed9b16b86 {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/HALIOMMU.efi
            SECTION UI = "HALIOMMU"
    }

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PdcDxe/PdcDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ClockDxe/ClockDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TzDxe/ScmDxeLA.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf

    FILE DRIVER = 6b38f7b4-ad98-40e9-9093-aca2b5a253c4 {
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.efi
            SECTION UI = "DiskIoDxe"
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf

    FILE DRIVER = 1fa1f39e-feff-4aae-bd7b-38a070a3b609 {
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.efi
            SECTION UI = "PartitionDxe"
    }

    INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
    INF MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UFSDxe/UFSDxe.inf

    FILE DRIVER = 0d35cd8e-97ea-4f9a-96af-0f0d89f76567 {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UFSDxe/UFSDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/UFSDxe.efi
            SECTION UI = "UFSDxe"
    }

    INF FatPkg/EnhancedFatDxe/Fat.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TzDxe/TzDxeLA.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/VariableDxe/VariableDxe.inf
    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/MsEnvDxe/MsEnvDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf

    FILE DRIVER = 4138022f-06c7-4f79-9c94-7e33b511a4e7 {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/DisplayDxe/DisplayDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/DisplayDxe.efi
            SECTION UI = "DisplayDxe"
    }

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/PILDxe/PILDxe.inf
    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/SharedCryptoDriverDxeMu/SharedCryptoDriverDxeMu.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf

    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/TouchControllerInitDxe/TouchControllerInitDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/SfpdUtils/SfpdUtils.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/FontDxe/FontDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomBds/QcomBds.inf
    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/ExtFgDxe/ExtFgDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/AdcDxe/AdcDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf

    FILE DRIVER = cd823a4d-7dec-4531-ae5d-4134fa4127b8 {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/UsbConfigDxe.efi
            SECTION UI = "UsbConfigDxe"
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf

    FILE DRIVER = 5bd181db-0487-4f1a-ae73-820e165611b3 {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/ButtonsDxe.efi
            SECTION UI = "ButtonsDxe"
    }

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/TsensDxe/TsensDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/LimitsDxe/LimitsDxe.inf

    FILE DRIVER = f09ad92e-4e91-490f-ab99-e69939b840b2 {
            SECTION UI = "LimitsDxe"
            SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/LimitsDxe/LimitsDxe.raw
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/LimitsDxe/LimitsDxe.efi
    }

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/MpPowerDxe/MpPowerDxe.inf

    FILE DRIVER = c62f4b20-681e-11df-8f0d-0002a5d5c51b {
            SECTION DXE_DEPEX = Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/PciHostBridgeDxe/PciHostBridgeDxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/PciHostBridgeDxe.efi
            SECTION UI = "PciHostBridge"
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HashDxe/HashDxe.inf

    FILE DRIVER = 3adf8dda-1850-44c5-8c63-bb991849bc6f {
            SECTION UI = "HashDxe"
            SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HashDxe/HashDxe.raw
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/HashDxe/HashDxe.efi
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CipherDxe/CipherDxe.inf

    FILE DRIVER = 5f0937cd-73f9-445e-b320-bb6fd5d3cbde {
            SECTION UI = "CipherDxe"
            SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CipherDxe/CipherDxe.raw
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/CipherDxe/CipherDxe.efi
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RNGDxe/RngDxe.inf

    FILE DRIVER = b0d3689e-11f8-43c6-8ece-023a29cec35b {
            SECTION UI = "RngDxe"
            SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RNGDxe/RngDxe.raw
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/RNGDxe/RngDxe.efi
    }

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/VarLockNullDxe/VarLockNullDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Variable/UefiVariablePolicy/Dxe/VariablePolicyNullDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/MsManufacturingModeDxe/MsManufacturingMode.inf

    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/AdapterInformationDxe/AdapterInformationDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/OSConfigDxe/OSConfigDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/PmicGlinkDxe/PmicGlinkDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf
    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

    FILE DRIVER = cccb0c28-4b24-11d5-9a5a-0090273fc14d {
            SECTION PE32 = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Override/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.efi
            SECTION UI = "GraphicsConsoleDxe"
    }

    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
    INF MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/QcomMpmTimerDxe/QcomMpmTimerDxe.inf
    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbMsdDxe/UsbMsdDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbDeviceDxe/UsbDeviceDxe.inf
    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
    # INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/XhciDxe/XhciDxe.inf

    FILE DRIVER = B7F50E91-A759-412C-ADE4-DCD03E7F7C28 {
            SECTION PE32 = Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/XhciDxe/XhciDxe.efi
            SECTION UI = "XhciDxe"
    }

    # INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf

    FILE DRIVER = 240612B7-A063-11D4-9A3A-0090273FC14D {
            SECTION PE32 = Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.efi
            SECTION UI = "UsbBusDxe"
    }

    # INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf

    FILE DRIVER = 2D2E62CF-9ECF-43B7-8219-94E7FC713DFE {
            SECTION PE32 = Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.efi
            SECTION UI = "UsbKbDxe"
    }

    INF MdeModulePkg/Bus/Usb/UsbMouseAbsolutePointerDxe/UsbMouseAbsolutePointerDxe.inf
    # INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf

    FILE DRIVER = 9FB4B4A7-42C0-4BCD-8540-9BCC6711F83E {
            SECTION PE32 = Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.efi
            SECTION UI = "UsbMassStorageDxe"
    }

    INF Cambria/BOOT.XF.3.0.1-00243-SC8180XWZB-2/QcomPkg/Drivers/UsbInitDxe/UsbInitDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsSurfaceCorePkg/SurfaceTouchHidDxe/SurfaceTouchHidDxe.inf
    # INF SharedCryptoPkg/Driver/SharedCryptoDxe.inf
    INF CryptoPkg/Driver/CryptoDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/FmpDevicePkg/FmpDxe/05472C14-ACB9-4458-A354-A68357AC3BF0.inf

    FILE DRIVER = 05472c14-acb9-4458-a354-a68357ac3bf0  {
            SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/FmpDevicePkg/FmpDxe/05472C14-ACB9-4458-A354-A68357AC3BF0.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/FmpDxe.efi
            SECTION UI = "FmpDxe"
    }

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/ColorbarsDxe/ColorbarsDxe.inf

    # FILE DRIVER = 7a26d1bd-0557-4b78-8ca1-1e40c7ad9f4d  {
    #         SECTION DXE_DEPEX = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/ColorbarsDxe/ColorbarsDxe.depex
    #         SECTION PE32 = QcomPkg/PatchedBinaries/ColorbarsDxe.efi
    #         SECTION UI = "ColorbarsDxe"
    # }

    INF SurfaceDuoFamilyPkg/Driver/ColorbarsDxe/ColorbarsDxe.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/QcomPkg/Application/QcomChargerApp/QcomChargerApp.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/EpsilonPkg/GpioButtonsDxe/GpioButtons.inf

    INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/MsEarlyGraphics/Dxe/MsEarlyGraphics.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/RenderingEngineDxe/RenderingEngineDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/SimpleWindowManagerDxe/SimpleWindowManagerDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/OnScreenKeyboardDxe/OnScreenKeyboardDxe.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/MsUiTheme/Dxe/MsUiThemeProtocol.inf
    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/MsGraphicsPkg/DisplayEngineDxe/DisplayEngineDxe.inf

    # INF Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/Sm8150FamilyPkg/Drivers/UfsSecurityDxe/UfsSecurityDxe.inf

    FILE FREEFORM = A91D838E-A5FA-4138-825D-455E2303079E {
        SECTION UI = "BDS_Menu.cfg"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/BDS_Menu.cfg
    }

    FILE FREEFORM = A91D838E-A5FA-4138-825D-455E23030794 {
        SECTION UI = "logo1.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/logo1.bmp
    }

    FILE FREEFORM = 8CAE2C55-FC5A-410B-812B-8CA22F572F36 {
        SECTION UI = "Panel_truly_wqxga_dsc_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_wqxga_dsc_cmd.xml
    }

    FILE FREEFORM = 13AC9DC9-0092-4374-AE1A-06FF35950FD0 {
        SECTION UI = "Panel_truly_wqxga_dsc_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_wqxga_dsc_vid.xml
    }

    FILE FREEFORM = FC6315F9-F8FE-4222-A1E3-226DF55D7592 {
        SECTION UI = "Panel_truly_wqxga_dual_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_wqxga_dual_cmd.xml
    }

    FILE FREEFORM = 95E7B040-26A2-4511-9ABB-1D95D6DA7082 {
        SECTION UI = "Panel_truly_wqxga_dual_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_wqxga_dual_vid.xml
    }

    FILE FREEFORM = 8BC595B7-391A-4A9A-8256-5B9983B30392 {
        SECTION UI = "Panel_sharp_4k_dsc_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sharp_4k_dsc_cmd.xml
    }

    FILE FREEFORM = 19B58ACD-B1F8-4078-B9A5-A33584B680E2 {
        SECTION UI = "Panel_sharp_4k_dsc_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sharp_4k_dsc_vid.xml
    }

    FILE FREEFORM = 0AE323D6-2EBE-11E8-8349-9BA636A0D80C {
        SECTION UI = "Panel_sharp_1080p_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sharp_1080p_cmd.xml
    }

    FILE FREEFORM = 87273B42-311D-11E8-9324-DFB2DADFC3D1 {
        SECTION UI = "Panel_truly_1080p_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_1080p_cmd.xml
    }

    FILE FREEFORM = BAFD3200-311D-11E8-8651-0FB3CACF38FB {
        SECTION UI = "Panel_truly_1080p_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_truly_1080p_vid.xml
    }

    FILE FREEFORM = 88CA5ABF-D0AC-4093-A68C-0CFAE1EF9635 {
        SECTION UI = "Panel_boe_amoled_wqhd_dsc_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_boe_amoled_wqhd_dsc_cmd.xml
    }

    FILE FREEFORM = 45A8FDA2-4FAD-4234-BCDA-D1778611CE13 {
        SECTION UI = "Panel_boe_amoled_wqhd_dsc_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_boe_amoled_wqhd_dsc_vid.xml
    }

    FILE FREEFORM = 2BCA3B86-E175-4609-AC7F-207908B89C75 {
        SECTION UI = "Panel_primary_sharp_1080p_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_primary_sharp_1080p_cmd.xml
    }

    FILE FREEFORM = A3607731-F28D-496C-83D3-FB182F885F8C {
        SECTION UI = "Panel_secondary_truly_1080p_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_secondary_truly_1080p_cmd.xml
    }

    FILE FREEFORM = 32C7620B-9746-4BB6-8166-605682A2C58A {
        SECTION UI = "Panel_nt36850_truly_wqhd_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_nt36850_truly_wqhd_cmd.xml
    }

    FILE FREEFORM = 2FD70D55-C89A-47C8-A960-5A3AAA10426C {
        SECTION UI = "Panel_sdx55_truly_wqxga_dual_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sdx55_truly_wqxga_dual_cmd.xml
    }

    FILE FREEFORM = 13E35E77-F1E0-4639-9021-70CBA62FB897 {
        SECTION UI = "Panel_sdx55_truly_wqxga_dual_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sdx55_truly_wqxga_dual_vid.xml
    }

    FILE FREEFORM = 07777D7D-677F-47F4-A1FB-3BB1E08E607A {
        SECTION UI = "Panel_sdx55_sharp_4k_dsc_cmd.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sdx55_sharp_4k_dsc_cmd.xml
    }

    FILE FREEFORM = 151F4DFB-643B-418D-ACB3-A66BC8D7A4EF {
        SECTION UI = "Panel_sdx55_sharp_4k_dsc_vid.xml"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Panel_sdx55_sharp_4k_dsc_vid.xml
    }

    FILE FREEFORM = 3E5584ED-05D4-4267-9048-0D47F76F4248 {
        SECTION UI = "battery_plugged_in_low.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_plugged_in_low.bmp
    }

    FILE FREEFORM = 4753E815-DDD8-402D-BF69-9B8C4EB7573E {
        SECTION UI = "battery_error.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_error.bmp
    }

    FILE FREEFORM = 03DED53E-BECD-428F-9F79-5ABA64C58445 {
        SECTION UI = "battery_symbol_Nocharger.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_symbol_Nocharger.bmp
    }

    FILE FREEFORM = 8B86CD38-C772-4FCF-85AA-345B2B3C1AB4 {
        SECTION UI = "battery_plugged_in.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_plugged_in.bmp
    }

    FILE FREEFORM = 3FD97907-93F1-4349-AF3C-3B68B0A5E626 {
        SECTION UI = "low_battery.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/low_battery.bmp
    }

    FILE FREEFORM = C72FB1A9-5527-4E13-BAC4-F671D0330145 {
        SECTION UI = "temperature_warning.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/temperature_warning.bmp
    }

    FILE FREEFORM = B0F8D1BE-5353-4812-B1F6-07E7768204CC {
        SECTION UI = "battery_symbol_DebugBoot.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_symbol_DebugBoot.bmp
    }

    FILE FREEFORM = F9E8F683-E065-4E09-B4F9-0230D7CECD08 {
        SECTION UI = "battery_symbol_DebugStay.bmp"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/battery_symbol_DebugStay.bmp
    }

    FILE FREEFORM = A1E235DE-E825-4591-9623-C43175811826 {
        SECTION UI = "SecParti.cfg"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/SecParti.cfg
    }

    FILE FREEFORM = 21E9BDD9-6C3F-4F10-84A5-BBEC322741F1 {
        SECTION UI = "uefipil.cfg"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/uefipil.cfg
    }

    FILE FREEFORM = 45FE4B7C-150C-45DA-A021-4BEB2048EC6F {
        SECTION UI = "QcomChargerCfg.cfg"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/QcomChargerCfg.cfg
    }

    FILE FREEFORM = F780C779-DD7C-47CD-BD1A-5EB414C51704 {
        SECTION UI = "BATTERY.PROVISION"
        SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/BATTERY.PROVISION
    }

    # FILE FREEFORM = 7BB28B99-61BB-11D5-9A5D-0090273FC14D {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Logo
    #     SECTION UI = "Logo"
    # }

    # FILE FREEFORM = D2AF039E-9F1F-4616-97AF-BE4509A365EA {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Low_Battery
    #     SECTION UI = "Low Battery"
    # }

    # FILE FREEFORM = 7C3A8851-A6D2-445B-A53B-155F867AB4DA {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/Too_Hot
    #     SECTION UI = "Too Hot"
    # }

    # FILE FREEFORM = 4FFEE6D5-B396-40F9-9246-7A8E4A821720 {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/FirmwareSettingsIndicator
    #     SECTION UI = "FirmwareSettingsIndicator"
    # }

    # FILE FREEFORM = F1E7A352-46AE-4912-92AF-36AB51781D8E {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/BootFailIndicator
    #     SECTION UI = "BootFailIndicator"
    # }

    # FILE FREEFORM = AE2D011C-4128-4960-A536-B424CCC3D1EA {
    #     SECTION RAW = Epsilon/BOOT.XF.3.0-00527-SM8150LZB-1/RawFiles/VolumeUpIndicator
    #     SECTION UI = "VolumeUpIndicator"
    # }