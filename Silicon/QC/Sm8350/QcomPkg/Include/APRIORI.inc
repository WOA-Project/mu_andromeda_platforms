APRIORI DXE {

    # First, install the PCD driver and call DxeInit
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/EnvDxe/EnvDxe.inf

    # INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    # INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

    # Next, install cpu protocol and enable the interrupt controller
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

    # Core Drivers
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf

    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SmemDxe/SmemDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/DALSYSDxe/DALSYSDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/HWIODxe/HWIODxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ChipInfoDxe/ChipInfoDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PlatformInfoDxe/PlatformInfoDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/HALIOMMUDxe/HALIOMMUDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ULogDxe/ULogDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/CmdDbDxe/CmdDbDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PwrUtilsDxe/PwrUtilsDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/NpaDxe/NpaDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/RpmhDxe/RpmhDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/VcsDxe/VcsDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ClockDxe/ClockDxe.inf
    INF QcomPkg/PatchedBinaries/ClockDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ICBDxe/ICBDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ShmBridgeDxe/ShmBridgeDxeLA.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/TzDxe/ScmDxeLA.inf

    FILE DRIVER = 4b4973ee-401b-4f36-a6a9-533dfccdfc33 {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/TzDxe/ScmDxeLA.efi
            SECTION UI = "ScmDxeLA"
    }

    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/TLMMDxe/TLMMDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/I2CDxe/I2CDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SPMIDxe/SPMIDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PmicDxe/PmicDxeLa.inf
    INF QcomPkg/PatchedBinaries/PmicDxeLa.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/Sm8350FamilyPkg/Drivers/ExtFgDxe/ExtFgDxe.inf
    # INF MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf

    FILE DRIVER = 6b38f7b4-ad98-40e9-9093-aca2b5a253c4 {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.efi
            SECTION UI = "DiskIoDxe"
    }

    INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
    INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
    INF MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SdccDxe/SdccDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UFSDxe/UFSDxe.inf
    INF QcomPkg/PatchedBinaries/UFSDxe.inf
    INF MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
    INF FatPkg/EnhancedFatDxe/Fat.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/TzDxe/TzDxeLA.inf

    FILE DRIVER = cf6dfc66-14ab-4e13-84db-9c02912d1404 {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/TzDxe/TzDxeLA.efi
            SECTION UI = "TzDxeLA"
    }

    INF QcomPkg/PatchedBinaries/MsAbstractLayer.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/VariableDxe/VariableDxe.inf
    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/QcomWDogDxe/QcomWDogDxe.inf
    INF QcomPkg/PatchedBinaries/QcomWDogDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/DDRInfoDxe/DDRInfoDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ResetRuntimeDxe/ResetRuntimeDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/FeatureEnablerDxe/FeatureEnablerDxe.inf

    FILE DRIVER = 1c04f529-0612-4319-9315-b2e75c16810a {
            SECTION PE32 = QcomPkg/PatchedBinaries/FeatureEnablerDxe.efi
            SECTION UI = "FeatureEnablerDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/DisplayDxe/DisplayDxe.inf
    INF SurfaceDuoFamilyPkg/Driver/SimpleFbDxe/SimpleFbDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/FvUtilsDxe/FvUtilsDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PILProxyDxe/PILProxyDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PILDxe/PILDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SPSSDxe/SPSSDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/IPCCDxe/IPCCDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/GLinkDxe/GLinkDxe.inf
    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
    INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/VerifiedBootDxe/VerifiedBootDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SecRSADxe/SecRSADxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ASN1X509Dxe/ASN1X509Dxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/FontDxe/FontDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/QcomBds/QcomBds.inf
    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SPIDxe/SPIDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/PmicGlinkDxe/PmicGlinkDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbPwrCtrlDxe/UsbPwrCtrlDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/QcomChargerDxe/QcomChargerDxeLA.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ChargerExDxe/ChargerExDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbfnDwc3Dxe/UsbfnDwc3Dxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/XhciPciEmulationDxe/XhciPciEmulationDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/XhciDxe/XhciDxe.inf

    FILE DRIVER = b7f50e91-a759-412c-ade4-dcd03e7f7c28 {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/XhciDxe/XhciDxe.efi
            SECTION UI = "XhciDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.inf

    FILE DRIVER = 240612b7-a063-11d4-9a3a-0090273fc14d {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbBusDxe/UsbBusDxe.efi
            SECTION UI = "UsbBusDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.inf

    FILE DRIVER = 2d2e62cf-9ecf-43b7-8219-94e7fc713dfe {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbKbDxe/UsbKbDxe.efi
            SECTION UI = "UsbKbDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.inf

    FILE DRIVER = 9fb4b4a7-42c0-4bcd-8540-9bcc6711f83e {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbMassStorageDxe/UsbMassStorageDxe.efi
            SECTION UI = "UsbMassStorageDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UsbConfigDxe/UsbConfigDxe.inf
    INF QcomPkg/PatchedBinaries/UsbConfigDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/ButtonsDxe/ButtonsDxe.inf
    INF QcomPkg/PatchedBinaries/ButtonsDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/CPRDxe/CPRDxe.inf
    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/GpiDxe/GpiDxe.inf
    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UCDxe/UCDxe.inf

    FILE DRIVER = 5f0937cd-73f9-445e-b320-bb6fd5d3cbde {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/UCDxe/UCDxe.efi
            SECTION UI = "UCDxe"
    }

    # INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/RNGDxe/RngDxe.inf

    FILE DRIVER = b0d3689e-11f8-43c6-8ece-023a29cec35b {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/RNGDxe/RngDxe.efi
            SECTION UI = "RngDxe"
    }

    INF Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/QcomPkg/Drivers/SimpleTextInOutSerialDxe/SimpleTextInOutSerial.inf

    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf

    # INF MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.inf

    FILE DRIVER = cccb0c28-4b24-11d5-9a5a-0090273fc14d {
            SECTION PE32 = Zeta/BOOT.MXF.1.0-00893-LAHAINA-1/MdeModulePkg/Universal/Console/GraphicsConsoleDxe/GraphicsConsoleDxe.efi
            SECTION UI = "GraphicsConsoleDxe"
    }
}