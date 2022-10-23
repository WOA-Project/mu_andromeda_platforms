    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
    INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf

    FILE DRIVER = 8430c46d-ab1c-4f82-896b-33e156931fb3 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8430c46d-ab1c-4f82-896b-33e156931fb3/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8430c46d-ab1c-4f82-896b-33e156931fb3/section1.pe
            SECTION UI = "ShmBridgeDxeLA"
    }

    FILE DRIVER = 4b4973ee-401b-4f36-a6a9-533dfccdfc33 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-4b4973ee-401b-4f36-a6a9-533dfccdfc33/section0.pe
            SECTION UI = "ScmDxeLA"
    }

    FILE DRIVER = cf6dfc66-14ab-4e13-84db-9c02912d1404 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cf6dfc66-14ab-4e13-84db-9c02912d1404/section0.pe
            SECTION UI = "TzDxeLA"
    }

    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf
    INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf

    FILE DRIVER = 3ae17db7-3cc5-4b89-9304-9549211057ef {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-3ae17db7-3cc5-4b89-9304-9549211057ef/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-3ae17db7-3cc5-4b89-9304-9549211057ef/section1.pe
            SECTION UI = "ResetRuntimeDxe"
    }

    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

    FILE DRIVER = ed15ffd5-bc96-4107-9118-95366600188e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section1.pe
            SECTION UI = "FontDxe"
    }

    FILE DRIVER = 040e1e61-0afb-411b-9ec9-00f3fc59cc13 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-040e1e61-0afb-411b-9ec9-00f3fc59cc13/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/QcomWDogDxe.efi
            SECTION UI = "QcomWDogDxe"
    }

    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf

    FILE DRIVER = 10e193df-9966-44e7-b17c-59dd831e20fc {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-10e193df-9966-44e7-b17c-59dd831e20fc/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-10e193df-9966-44e7-b17c-59dd831e20fc/section1.pe
            SECTION UI = "ChipInfo"
    }

    FILE DRIVER = b105211b-bbbd-4add-a3b0-d1cf4a52154c {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-b105211b-bbbd-4add-a3b0-d1cf4a52154c/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-b105211b-bbbd-4add-a3b0-d1cf4a52154c/section1.pe
            SECTION UI = "PlatformInfoDxeDriver"
    }

    FILE DRIVER = 8e9bd160-b184-11df-94e2-0800200c9a66 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8e9bd160-b184-11df-94e2-0800200c9a66/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8e9bd160-b184-11df-94e2-0800200c9a66/section1.pe
            SECTION UI = "DALSys"
    }

    FILE DRIVER = 9a00771f-36d4-4dd5-8916-c48ed9b16b86 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-9a00771f-36d4-4dd5-8916-c48ed9b16b86/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-9a00771f-36d4-4dd5-8916-c48ed9b16b86/section1.pe
            SECTION UI = "HALIOMMU"
    }

    FILE DRIVER = af9763a2-033b-4109-8e17-56a98d380c92 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-af9763a2-033b-4109-8e17-56a98d380c92/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-af9763a2-033b-4109-8e17-56a98d380c92/section1.pe
            SECTION UI = "HWIODxeDriver"
    }

    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf

    FILE DRIVER = 6b38f7b4-ad98-40e9-9093-aca2b5a253c4 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-6b38f7b4-ad98-40e9-9093-aca2b5a253c4/section0.pe
            SECTION UI = "DiskIoDxe"
    }

    INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
    INF FatPkg/EnhancedFatDxe/Fat.inf
    INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
    INF MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf

    FILE DRIVER = 0d35cd8e-97ea-4f9a-96af-0f0d89f76567 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0d35cd8e-97ea-4f9a-96af-0f0d89f76567/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/UFSDxe.efi
            SECTION UI = "UFSDxe"
    }

    FILE DRIVER = aba01ff8-2ccb-4e12-8b2e-cd3f4a742993 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-aba01ff8-2ccb-4e12-8b2e-cd3f4a742993/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-aba01ff8-2ccb-4e12-8b2e-cd3f4a742993/section1.pe
            SECTION UI = "CmdDbDxe"
    }

    FILE DRIVER = 0401b830-bff9-44a2-a5f6-95734a33c017 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0401b830-bff9-44a2-a5f6-95734a33c017/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0401b830-bff9-44a2-a5f6-95734a33c017/section1.pe
            SECTION UI = "PwrUtilsDxe"
    }

    FILE DRIVER = cb29f4d1-7f37-4692-a416-93e82e219766 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219766/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219766/section1.pe
            SECTION UI = "RpmhDxe"
    }

    FILE DRIVER = cb29f4d1-7f37-4692-a416-93e82e219711 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219711/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219711/section1.pe
            SECTION UI = "NpaDxe"
    }

    FILE DRIVER = e43128a8-8692-42b6-8afa-676158578d18 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-e43128a8-8692-42b6-8afa-676158578d18/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-e43128a8-8692-42b6-8afa-676158578d18/section1.pe
            SECTION UI = "ULogDxe"
    }

    FILE DRIVER = 8bd3b475-401a-4b0b-9315-edee61a1eae5 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8bd3b475-401a-4b0b-9315-edee61a1eae5/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8bd3b475-401a-4b0b-9315-edee61a1eae5/section1.pe
            SECTION UI = "VcsDxe"
    }

    FILE DRIVER = 4db5dea6-5302-4d1a-8a82-677a683b0d29 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-4db5dea6-5302-4d1a-8a82-677a683b0d29/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/ClockDxe.efi
            SECTION UI = "ClockDxe"
    }

    FILE DRIVER = 5824f9de-17d2-4b1f-a5fe-5eb77fa53093 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-5824f9de-17d2-4b1f-a5fe-5eb77fa53093/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-5824f9de-17d2-4b1f-a5fe-5eb77fa53093/section1.pe
            SECTION UI = "ICBDxe"
    }

    FILE DRIVER = f541d663-4a48-40aa-aabf-ff158ccae34c {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-f541d663-4a48-40aa-aabf-ff158ccae34c/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-f541d663-4a48-40aa-aabf-ff158ccae34c/section1.pe
            SECTION UI = "SmemDxe"
    }

    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf
    INF MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

    FILE FREEFORM = a91d838e-a5fa-4138-825d-455e2303079e {
            SECTION UI = "BDS_Menu.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-a91d838e-a5fa-4138-825d-455e2303079e/section1.raw
    }

    FILE FREEFORM = a1e235de-e825-4591-9623-c43175811826 {
            SECTION UI = "SecParti.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-a1e235de-e825-4591-9623-c43175811826/section1.raw
    }

    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf