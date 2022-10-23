APRIORI DXE {

    # First, install the PCD driver and call DxeInit
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

    # INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    # INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

    # Next, install cpu protocol and enable the interrupt controller
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

    # Core Drivers
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf
    INF ArmPkg/Drivers/TimerDxe/TimerDxe.inf

    FILE DRIVER = f541d663-4a48-40aa-aabf-ff158ccae34c {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-f541d663-4a48-40aa-aabf-ff158ccae34c/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-f541d663-4a48-40aa-aabf-ff158ccae34c/section1.pe
            SECTION UI = "SmemDxe"
    }

    FILE DRIVER = 8e9bd160-b184-11df-94e2-0800200c9a66 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8e9bd160-b184-11df-94e2-0800200c9a66/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8e9bd160-b184-11df-94e2-0800200c9a66/section1.pe
            SECTION UI = "DALSys"
    }

    FILE DRIVER = af9763a2-033b-4109-8e17-56a98d380c92 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-af9763a2-033b-4109-8e17-56a98d380c92/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-af9763a2-033b-4109-8e17-56a98d380c92/section1.pe
            SECTION UI = "HWIODxeDriver"
    }

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

    FILE DRIVER = 9a00771f-36d4-4dd5-8916-c48ed9b16b86 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-9a00771f-36d4-4dd5-8916-c48ed9b16b86/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-9a00771f-36d4-4dd5-8916-c48ed9b16b86/section1.pe
            SECTION UI = "HALIOMMU"
    }

    FILE DRIVER = e43128a8-8692-42b6-8afa-676158578d18 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-e43128a8-8692-42b6-8afa-676158578d18/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-e43128a8-8692-42b6-8afa-676158578d18/section1.pe
            SECTION UI = "ULogDxe"
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

    FILE DRIVER = cb29f4d1-7f37-4692-a416-93e82e219711 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219711/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219711/section1.pe
            SECTION UI = "NpaDxe"
    }

    FILE DRIVER = cb29f4d1-7f37-4692-a416-93e82e219766 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219766/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cb29f4d1-7f37-4692-a416-93e82e219766/section1.pe
            SECTION UI = "RpmhDxe"
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

    FILE DRIVER = 8430c46d-ab1c-4f82-896b-33e156931fb3 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8430c46d-ab1c-4f82-896b-33e156931fb3/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-8430c46d-ab1c-4f82-896b-33e156931fb3/section1.pe
            SECTION UI = "ShmBridgeDxeLA"
    }

    FILE DRIVER = 4b4973ee-401b-4f36-a6a9-533dfccdfc33 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-4b4973ee-401b-4f36-a6a9-533dfccdfc33/section0.pe
            SECTION UI = "ScmDxeLA"
    }

    FILE DRIVER = 8681cc5a-0df6-441e-b4b8-e915c538f067 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8681cc5a-0df6-441e-b4b8-e915c538f067/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8681cc5a-0df6-441e-b4b8-e915c538f067/section1.pe
            SECTION UI = "DALTLMM"
    }

    FILE DRIVER = 04de8591-d2b3-4077-bbbe-b12070094eb6 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-04de8591-d2b3-4077-bbbe-b12070094eb6/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-04de8591-d2b3-4077-bbbe-b12070094eb6/section1.pe
            SECTION UI = "I2C"
    }

    FILE DRIVER = 2a7b4bef-80cd-49e1-b473-374ba4d673fc {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2a7b4bef-80cd-49e1-b473-374ba4d673fc/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2a7b4bef-80cd-49e1-b473-374ba4d673fc/section1.pe
            SECTION UI = "SPMI"
    }

    FILE DRIVER = 5776232e-082d-4b75-9a0e-fe1d13f7a5d9 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5776232e-082d-4b75-9a0e-fe1d13f7a5d9/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/PmicDxe.efi
            SECTION UI = "PmicDxe"
    }

    FILE DRIVER = 6201ace5-5f41-4eb5-bfad-6980ea341d63 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6201ace5-5f41-4eb5-bfad-6980ea341d63/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6201ace5-5f41-4eb5-bfad-6980ea341d63/section1.pe
            SECTION UI = "EXTFG"
    }

    FILE DRIVER = 6b38f7b4-ad98-40e9-9093-aca2b5a253c4 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-6b38f7b4-ad98-40e9-9093-aca2b5a253c4/section0.pe
            SECTION UI = "DiskIoDxe"
    }

    INF MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf
    INF MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf
    INF MdeModulePkg/Universal/FvSimpleFileSystemDxe/FvSimpleFileSystemDxe.inf

    FILE DRIVER = f10f76db-42c1-533f-34a8-69be24653110 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-f10f76db-42c1-533f-34a8-69be24653110/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-f10f76db-42c1-533f-34a8-69be24653110/section1.pe
            SECTION UI = "SdccDxe"
    }

    FILE DRIVER = 0d35cd8e-97ea-4f9a-96af-0f0d89f76567 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0d35cd8e-97ea-4f9a-96af-0f0d89f76567/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/UFSDxe.efi
            SECTION UI = "UFSDxe"
    }

    INF MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
    INF FatPkg/EnhancedFatDxe/Fat.inf

    FILE DRIVER = cf6dfc66-14ab-4e13-84db-9c02912d1404 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cf6dfc66-14ab-4e13-84db-9c02912d1404/section0.pe
            SECTION UI = "TzDxeLA"
    }

    FILE DRIVER = bd97f704-ce48-11eb-916c-db1f9c585103 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-bd97f704-ce48-11eb-916c-db1f9c585103/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/MsAbstractLayerDxe.efi
            SECTION UI = "MsAbstractLayerDxe"
    }

    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

    FILE DRIVER = 040e1e61-0afb-411b-9ec9-00f3fc59cc13 {
          SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-040e1e61-0afb-411b-9ec9-00f3fc59cc13/section0.dxe.depex
          SECTION PE32 = QcomPkg/PatchedBinaries/QcomWDogDxe.efi
          SECTION UI = "QcomWDogDxe"
    }

    FILE DRIVER = 7db0793a-4402-4be1-906e-d0fabad2707e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7db0793a-4402-4be1-906e-d0fabad2707e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7db0793a-4402-4be1-906e-d0fabad2707e/section1.pe
            SECTION UI = "DDRInfoDxe"
    }

    FILE DRIVER = 3ae17db7-3cc5-4b89-9304-9549211057ef {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-3ae17db7-3cc5-4b89-9304-9549211057ef/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-3ae17db7-3cc5-4b89-9304-9549211057ef/section1.pe
            SECTION UI = "ResetRuntimeDxe"
    }

    FILE DRIVER = 1c04f529-0612-4319-9315-b2e75c16810a {
            SECTION PE32 = QcomPkg/PatchedBinaries/FeatureEnablerDxe.efi
            SECTION UI = "FeatureEnablerDxe"
    }

    INF SurfaceDuoFamilyPkg/Driver/SimpleFbDxe/SimpleFbDxe.inf

    FILE DRIVER = e2534a24-1d9a-4a54-aaec-be8836d0f45d {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-e2534a24-1d9a-4a54-aaec-be8836d0f45d/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-e2534a24-1d9a-4a54-aaec-be8836d0f45d/section1.pe
            SECTION UI = "FvDxe"
    }

    FILE DRIVER = 8cf9c920-d5e6-4dac-bef9-6e6a4eec7add {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8cf9c920-d5e6-4dac-bef9-6e6a4eec7add/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8cf9c920-d5e6-4dac-bef9-6e6a4eec7add/section1.pe
            SECTION UI = "PILProxyDxe"
    }

    FILE DRIVER = 1d317734-4083-47c6-83ea-e3a42f2a6bff {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1d317734-4083-47c6-83ea-e3a42f2a6bff/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1d317734-4083-47c6-83ea-e3a42f2a6bff/section1.pe
            SECTION UI = "PILDxe"
    }

    FILE DRIVER = 5d787b6c-6254-43f5-a7a8-cb25329cf97e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5d787b6c-6254-43f5-a7a8-cb25329cf97e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5d787b6c-6254-43f5-a7a8-cb25329cf97e/section1.pe
            SECTION UI = "SPSSDxe"
    }

    FILE DRIVER = 42031f68-1f0b-429b-93f1-0eb0cf89ddbf {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-42031f68-1f0b-429b-93f1-0eb0cf89ddbf/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-42031f68-1f0b-429b-93f1-0eb0cf89ddbf/section1.pe
            SECTION UI = "IPCCDxe"
    }

    FILE DRIVER = 7942a7eb-b7d5-4e2e-b908-831e4de55b58 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7942a7eb-b7d5-4e2e-b908-831e4de55b58/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7942a7eb-b7d5-4e2e-b908-831e4de55b58/section1.pe
            SECTION UI = "GlinkDxe"
    }

    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf
    INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
    INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf
    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf
    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

    FILE DRIVER = ed15ffd5-bc96-4107-9118-95366600188e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section1.pe
            SECTION UI = "FontDxe"
    }

    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

    FILE DRIVER = 7a32bd23-f735-4f57-aa1a-447d2fe3be0d {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a32bd23-f735-4f57-aa1a-447d2fe3be0d/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a32bd23-f735-4f57-aa1a-447d2fe3be0d/section1.pe
            SECTION UI = "SPI"
    }

    FILE DRIVER = 6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273/section1.pe
            SECTION UI = "PmicGlinkDxe"
    }

    FILE DRIVER = 11faed4c-b21f-4d88-8e48-c4c28a1e50df {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-11faed4c-b21f-4d88-8e48-c4c28a1e50df/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-11faed4c-b21f-4d88-8e48-c4c28a1e50df/section1.pe
            SECTION UI = "UsbPwrCtrlDxe"
    }

    FILE DRIVER = 4bce7f36-428e-4393-99e3-7e0844404dba {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-4bce7f36-428e-4393-99e3-7e0844404dba/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-4bce7f36-428e-4393-99e3-7e0844404dba/section1.pe
            SECTION UI = "QcomChargerDxeLA"
    }

    FILE DRIVER = 2e3042f3-2bb9-463f-8866-56d75b7c4ab0 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2e3042f3-2bb9-463f-8866-56d75b7c4ab0/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2e3042f3-2bb9-463f-8866-56d75b7c4ab0/section1.pe
            SECTION UI = "ChargerExDxe"
    }

    FILE DRIVER = 94f8a6a7-dc34-4101-88c1-99179cceae83 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-94f8a6a7-dc34-4101-88c1-99179cceae83/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-94f8a6a7-dc34-4101-88c1-99179cceae83/section1.pe
            SECTION UI = "UsbfnDwc3Dxe"
    }

    FILE DRIVER = beb12bee-f6e1-11e1-9fb8-6c626de4aeb1 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-beb12bee-f6e1-11e1-9fb8-6c626de4aeb1/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-beb12bee-f6e1-11e1-9fb8-6c626de4aeb1/section1.pe
            SECTION UI = "XhciPciEmulation"
    }

    FILE DRIVER = b7f50e91-a759-412c-ade4-dcd03e7f7c28 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-b7f50e91-a759-412c-ade4-dcd03e7f7c28/section0.pe
            SECTION UI = "XhciDxe"
    }

    FILE DRIVER = 240612b7-a063-11d4-9a3a-0090273fc14d {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-240612b7-a063-11d4-9a3a-0090273fc14d/section0.pe
            SECTION UI = "UsbBusDxe"
    }

    FILE DRIVER = 2d2e62cf-9ecf-43b7-8219-94e7fc713dfe {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2d2e62cf-9ecf-43b7-8219-94e7fc713dfe/section0.pe
            SECTION UI = "UsbKbDxe"
    }

    FILE DRIVER = 9fb4b4a7-42c0-4bcd-8540-9bcc6711f83e {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-9fb4b4a7-42c0-4bcd-8540-9bcc6711f83e/section0.pe
            SECTION UI = "UsbMassStorageDxe"
    }

    FILE DRIVER = cd823a4d-7dec-4531-ae5d-4134fa4127b8 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-cd823a4d-7dec-4531-ae5d-4134fa4127b8/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/UsbConfigDxe.efi
            SECTION UI = "UsbConfigDxe"
    }

    FILE DRIVER = 5bd181db-0487-4f1a-ae73-820e165611b3 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5bd181db-0487-4f1a-ae73-820e165611b3/section0.dxe.depex
            SECTION PE32 = QcomPkg/PatchedBinaries/ButtonsDxe.efi
            SECTION UI = "ButtonsDxe"
    }

    # FILE DRIVER = 1b52138d-3fa3-4e50-b958-20887353f809 {
    #         SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1b52138d-3fa3-4e50-b958-20887353f809/section0.dxe.depex
    #         SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1b52138d-3fa3-4e50-b958-20887353f809/section1.pe
    #         SECTION UI = "CPRDxe"
    # }

    FILE DRIVER = 569ea0de-b557-4043-84cf-01103fe516e5 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-569ea0de-b557-4043-84cf-01103fe516e5/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-569ea0de-b557-4043-84cf-01103fe516e5/section1.pe
            SECTION UI = "GpiDxe"
    }

    FILE DRIVER = 5f0937cd-73f9-445e-b320-bb6fd5d3cbde {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5f0937cd-73f9-445e-b320-bb6fd5d3cbde/section0.pe
            SECTION UI = "UCDxe"
    }

    FILE DRIVER = b0d3689e-11f8-43c6-8ece-023a29cec35b {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-b0d3689e-11f8-43c6-8ece-023a29cec35b/section0.pe
            SECTION UI = "RngDxe"
    }

    INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf

    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf

    FILE DRIVER = cccb0c28-4b24-11d5-9a5a-0090273fc14d {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-cccb0c28-4b24-11d5-9a5a-0090273fc14d/section0.pe
            SECTION UI = "GraphicsConsoleDxe"
    }
}