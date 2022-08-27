
    # First, install the PCD driver and call DxeInit
    INF MdeModulePkg/Core/Dxe/DxeMain.inf
    INF MdeModulePkg/Universal/PCD/Dxe/Pcd.inf

    INF MdeModulePkg/Universal/ReportStatusCodeRouter/RuntimeDxe/ReportStatusCodeRouterRuntimeDxe.inf
    INF MdeModulePkg/Universal/StatusCodeHandler/RuntimeDxe/StatusCodeHandlerRuntimeDxe.inf
    INF MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf

    # Next, install cpu protocol and enable the interrupt controller
    INF ArmPkg/Drivers/CpuDxe/CpuDxe.inf
    INF ArmPkg/Drivers/ArmGic/ArmGicDxe.inf

    # Core Drivers
    INF EmbeddedPkg/MetronomeDxe/MetronomeDxe.inf

    FILE DRIVER = 49ea041e-6752-42ca-b0b1-7344fe2546b7 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-49ea041e-6752-42ca-b0b1-7344fe2546b7/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-49ea041e-6752-42ca-b0b1-7344fe2546b7/section1.pe
            SECTION UI = "ArmTimerDxe"
    }

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
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-4db5dea6-5302-4d1a-8a82-677a683b0d29/section1.pe
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

    # FILE DRIVER = 0d35cd8e-97ea-4f9a-96af-0f0d89f76567 {
    #         SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0d35cd8e-97ea-4f9a-96af-0f0d89f76567/section0.dxe.depex
    #         SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-0d35cd8e-97ea-4f9a-96af-0f0d89f76567/section1.pe
    #         SECTION UI = "UFSDxe"
    # }

    INF MdeModulePkg/Bus/Pci/PciBusDxe/PciBusDxe.inf
    INF FatPkg/EnhancedFatDxe/Fat.inf

    FILE DRIVER = cf6dfc66-14ab-4e13-84db-9c02912d1404 {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-cf6dfc66-14ab-4e13-84db-9c02912d1404/section0.pe
            SECTION UI = "TzDxeLA"
    }

    INF MdeModulePkg/Universal/Variable/RuntimeDxe/VariableRuntimeDxe.inf

    FILE DRIVER = 615676d8-f930-40dd-aff2-f1fae384337f {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-615676d8-f930-40dd-aff2-f1fae384337f/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-615676d8-f930-40dd-aff2-f1fae384337f/section1.pe
            SECTION UI = "MsEnvDxe"
    }

    FILE DRIVER = 040e1e61-0afb-411b-9ec9-00f3fc59cc13 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-040e1e61-0afb-411b-9ec9-00f3fc59cc13/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-040e1e61-0afb-411b-9ec9-00f3fc59cc13/section1.pe
            SECTION UI = "QcomWDogDxe"
    }

    FILE DRIVER = 8681cc5a-0df6-441e-b4b8-e915c538f067 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8681cc5a-0df6-441e-b4b8-e915c538f067/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8681cc5a-0df6-441e-b4b8-e915c538f067/section1.pe
            SECTION UI = "DALTLMM"
    }

    FILE DRIVER = 2a7b4bef-80cd-49e1-b473-374ba4d673fc {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2a7b4bef-80cd-49e1-b473-374ba4d673fc/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-2a7b4bef-80cd-49e1-b473-374ba4d673fc/section1.pe
            SECTION UI = "SPMI"
    }

    FILE DRIVER = 7db0793a-4402-4be1-906e-d0fabad2707e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7db0793a-4402-4be1-906e-d0fabad2707e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7db0793a-4402-4be1-906e-d0fabad2707e/section1.pe
            SECTION UI = "DDRInfoDxe"
    }

    FILE DRIVER = 5776232e-082d-4b75-9a0e-fe1d13f7a5d9 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5776232e-082d-4b75-9a0e-fe1d13f7a5d9/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5776232e-082d-4b75-9a0e-fe1d13f7a5d9/section1.pe
            SECTION UI = "PmicDxe"
    }

    FILE DRIVER = 1c04f529-0612-4319-9315-b2e75c16810a {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1c04f529-0612-4319-9315-b2e75c16810a/section0.pe
            SECTION UI = "FeatureEnablerDxe"
    }

    INF SurfaceDuo2Pkg/Driver/SimpleFbDxe/SimpleFbDxe.inf

    FILE DRIVER = 1d317734-4083-47c6-83ea-e3a42f2a6bff {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1d317734-4083-47c6-83ea-e3a42f2a6bff/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1d317734-4083-47c6-83ea-e3a42f2a6bff/section1.pe
            SECTION UI = "PILDxe"
    }

    FILE DRIVER = 5f0937cd-73f9-445e-b320-bb6fd5d3cbde {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5f0937cd-73f9-445e-b320-bb6fd5d3cbde/section0.pe
            SECTION UI = "UCDxe"
    }

    INF MdeModulePkg/Universal/WatchdogTimerDxe/WatchdogTimer.inf

    INF EmbeddedPkg/EmbeddedMonotonicCounter/EmbeddedMonotonicCounter.inf
    INF MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
    INF EmbeddedPkg/RealTimeClockRuntimeDxe/RealTimeClockRuntimeDxe.inf

    INF MdeModulePkg/Universal/PrintDxe/PrintDxe.inf
    INF MdeModulePkg/Universal/DevicePathDxe/DevicePathDxe.inf

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
    
    FILE DRIVER = 1bfbafda-52eb-4550-b952-d663e0aa97a2 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1bfbafda-52eb-4550-b952-d663e0aa97a2/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1bfbafda-52eb-4550-b952-d663e0aa97a2/section1.pe
            SECTION UI = "TouchControllerInitDxe"
    }

    INF MdeModulePkg/Universal/HiiDatabaseDxe/HiiDatabaseDxe.inf

    FILE DRIVER = ed15ffd5-bc96-4107-9118-95366600188e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-ed15ffd5-bc96-4107-9118-95366600188e/section1.pe
            SECTION UI = "FontDxe"
    }

    INF MdeModulePkg/Universal/BdsDxe/BdsDxe.inf

    FILE DRIVER = 04de8591-d2b3-4077-bbbe-b12070094eb6 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-04de8591-d2b3-4077-bbbe-b12070094eb6/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-04de8591-d2b3-4077-bbbe-b12070094eb6/section1.pe
            SECTION UI = "I2C"
    }

    FILE DRIVER = 6201ace5-5f41-4eb5-bfad-6980ea341d63 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6201ace5-5f41-4eb5-bfad-6980ea341d63/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6201ace5-5f41-4eb5-bfad-6980ea341d63/section1.pe
            SECTION UI = "EXTFG"
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

    FILE DRIVER = cd823a4d-7dec-4531-ae5d-4134fa4127b8 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-cd823a4d-7dec-4531-ae5d-4134fa4127b8/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-cd823a4d-7dec-4531-ae5d-4134fa4127b8/section1.pe
            SECTION UI = "UsbConfigDxe"
    }

    FILE DRIVER = 5bd181db-0487-4f1a-ae73-820e165611b3 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5bd181db-0487-4f1a-ae73-820e165611b3/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5bd181db-0487-4f1a-ae73-820e165611b3/section1.pe
            SECTION UI = "ButtonsDxe"
    }

    FILE DRIVER = f09ad92e-485c-9cdb-3e05-3385c38f94cc {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-f09ad92e-485c-9cdb-3e05-3385c38f94cc/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-f09ad92e-485c-9cdb-3e05-3385c38f94cc/section1.pe
            SECTION UI = "TsensDxe"
    }

    FILE DRIVER = 569ea0de-b557-4043-84cf-01103fe516e5 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-569ea0de-b557-4043-84cf-01103fe516e5/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-569ea0de-b557-4043-84cf-01103fe516e5/section1.pe
            SECTION UI = "GpiDxe"
    }

    FILE DRIVER = b0d3689e-11f8-43c6-8ece-023a29cec35b {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-b0d3689e-11f8-43c6-8ece-023a29cec35b/section0.pe
            SECTION UI = "RngDxe"
    }

    FILE DRIVER = 505a1007-32e0-4d76-9c82-3223048bc00b {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-505a1007-32e0-4d76-9c82-3223048bc00b/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-505a1007-32e0-4d76-9c82-3223048bc00b/section1.pe
            SECTION UI = "ManufacturingModeDxe"
    }

    FILE DRIVER = 1b52138d-3fa3-4e50-b958-20887353f809 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1b52138d-3fa3-4e50-b958-20887353f809/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-1b52138d-3fa3-4e50-b958-20887353f809/section1.pe
            SECTION UI = "CPRDxe"
    }

    FILE DRIVER = 7942a7eb-b7d5-4e2e-b908-831e4de55b58 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7942a7eb-b7d5-4e2e-b908-831e4de55b58/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7942a7eb-b7d5-4e2e-b908-831e4de55b58/section1.pe
            SECTION UI = "GlinkDxe"
    }

    FILE DRIVER = 6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6d4a9cf7-cc0c-42b0-aa89-f6a6a8d99273/section1.pe
            SECTION UI = "PmicGlinkDxe"
    }

    INF EmbeddedPkg/SimpleTextInOutSerial/SimpleTextInOutSerial.inf
    INF MdeModulePkg/Universal/Console/ConPlatformDxe/ConPlatformDxe.inf
    INF MdeModulePkg/Universal/Console/ConSplitterDxe/ConSplitterDxe.inf

    FILE DRIVER = cccb0c28-4b24-11d5-9a5a-0090273fc14d {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-cccb0c28-4b24-11d5-9a5a-0090273fc14d/section0.pe
            SECTION UI = "GraphicsConsoleDxe"
    }

    INF MdeModulePkg/Universal/CapsuleRuntimeDxe/CapsuleRuntimeDxe.inf
    INF MdeModulePkg/Universal/SecurityStubDxe/SecurityStubDxe.inf
    INF MdeModulePkg/Universal/Acpi/BootGraphicsResourceTableDxe/BootGraphicsResourceTableDxe.inf

    FILE DRIVER = 7a32bd23-f735-4f57-aa1a-447d2fe3be0d {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a32bd23-f735-4f57-aa1a-447d2fe3be0d/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a32bd23-f735-4f57-aa1a-447d2fe3be0d/section1.pe
            SECTION UI = "SPI"
    }

    FILE DRIVER = 5af77f10-90df-4e7e-8325-a17ec09d5443 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5af77f10-90df-4e7e-8325-a17ec09d5443/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5af77f10-90df-4e7e-8325-a17ec09d5443/section1.pe
            SECTION UI = "UsbMsdDxe"
    }

    FILE DRIVER = 3299a266-15f0-4346-8318-716336736d3e {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-3299a266-15f0-4346-8318-716336736d3e/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-3299a266-15f0-4346-8318-716336736d3e/section1.pe
            SECTION UI = "UsbDeviceDxe"
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

    INF MdeModulePkg/Bus/Usb/UsbMouseAbsolutePointerDxe/UsbMouseAbsolutePointerDxe.inf

    FILE DRIVER = 9fb4b4a7-42c0-4bcd-8540-9bcc6711f83e {
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-9fb4b4a7-42c0-4bcd-8540-9bcc6711f83e/section0.pe
            SECTION UI = "UsbMassStorageDxe"
    }

    FILE DRIVER = 0a134f0e-075e-40b3-9c63-3b3906804663 {
            SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-0a134f0e-075e-40b3-9c63-3b3906804663/section0.dxe.depex
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-0a134f0e-075e-40b3-9c63-3b3906804663/section1.pe
            SECTION UI = "UsbInitDxe"
    }

    # FILE DRIVER = 7a26d1bd-0557-4b78-8ca1-1e40c7ad9f4d {
    #         SECTION DXE_DEPEX = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a26d1bd-0557-4b78-8ca1-1e40c7ad9f4d/section0.dxe.depex
    #         SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7a26d1bd-0557-4b78-8ca1-1e40c7ad9f4d/section1.pe
    #         SECTION UI = "ColorbarsDxe"
    # }

    FILE APPLICATION = eee9c2b1-16ca-4f34-87ea-2e6d1e160cc4 {
            SECTION UI = "QcomChargerApp"
            SECTION PE32 = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-eee9c2b1-16ca-4f34-87ea-2e6d1e160cc4/section1.pe
    }
    FILE FREEFORM = a91d838e-a5fa-4138-825d-455e2303079e {
            SECTION UI = "BDS_Menu.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-a91d838e-a5fa-4138-825d-455e2303079e/section1.raw
    }

    FILE FREEFORM = a91d838e-a5fa-4138-825d-455e23030794 {
            SECTION UI = "logo1.bmp"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-a91d838e-a5fa-4138-825d-455e23030794/section1.raw
    }

    FILE FREEFORM = 5d3485f0-3c38-4b8b-b1ef-d5d7057708d2 {
            SECTION UI = "uilogo.bmp"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-5d3485f0-3c38-4b8b-b1ef-d5d7057708d2/section1.raw
    }

    FILE FREEFORM = 8cae2c55-fc5a-410b-812b-8ca22f572f36 {
            SECTION UI = "Panel_truly_wqxga_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8cae2c55-fc5a-410b-812b-8ca22f572f36/section1.raw
    }

    FILE FREEFORM = 13ac9dc9-0092-4374-ae1a-06ff35950fd0 {
            SECTION UI = "Panel_truly_wqxga_dsc_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-13ac9dc9-0092-4374-ae1a-06ff35950fd0/section1.raw
    }

    FILE FREEFORM = fc6315f9-f8fe-4222-a1e3-226df55d7592 {
            SECTION UI = "Panel_truly_wqxga_dual_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-fc6315f9-f8fe-4222-a1e3-226df55d7592/section1.raw
    }

    FILE FREEFORM = 95e7b040-26a2-4511-9abb-1d95d6da7082 {
            SECTION UI = "Panel_truly_wqxga_dual_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-95e7b040-26a2-4511-9abb-1d95d6da7082/section1.raw
    }

    FILE FREEFORM = 8bc595b7-391a-4a9a-8256-5b9983b30392 {
            SECTION UI = "Panel_sharp_4k_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8bc595b7-391a-4a9a-8256-5b9983b30392/section1.raw
    }

    FILE FREEFORM = 19b58acd-b1f8-4078-b9a5-a33584b680e2 {
            SECTION UI = "Panel_sharp_4k_dsc_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-19b58acd-b1f8-4078-b9a5-a33584b680e2/section1.raw
    }

    FILE FREEFORM = 0ae323d6-2ebe-11e8-8349-9ba636a0d80c {
            SECTION UI = "Panel_sharp_1080p_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-0ae323d6-2ebe-11e8-8349-9ba636a0d80c/section1.raw
    }

    FILE FREEFORM = 87273b42-311d-11e8-9324-dfb2dadfc3d1 {
            SECTION UI = "Panel_truly_1080p_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-87273b42-311d-11e8-9324-dfb2dadfc3d1/section1.raw
    }

    FILE FREEFORM = bafd3200-311d-11e8-8651-0fb3cacf38fb {
            SECTION UI = "Panel_truly_1080p_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-bafd3200-311d-11e8-8651-0fb3cacf38fb/section1.raw
    }

    FILE FREEFORM = 88ca5abf-d0ac-4093-a68c-0cfae1ef9635 {
            SECTION UI = "Panel_boe_amoled_wqhd_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-88ca5abf-d0ac-4093-a68c-0cfae1ef9635/section1.raw
    }

    FILE FREEFORM = 6523a5c9-91cb-45c0-a2dc-25997cf23369 {
            SECTION UI = "Panel_boe_amoled_wqhd_dsc_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6523a5c9-91cb-45c0-a2dc-25997cf23369/section1.raw
    }

    FILE FREEFORM = d7170685-5965-4b44-9305-37cdb199b9be {
            SECTION UI = "Panel_boe_amoled_fhd_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-d7170685-5965-4b44-9305-37cdb199b9be/section1.raw
    }

    FILE FREEFORM = 8f276ee4-e75e-4c3b-8bcc-c37771e26478 {
            SECTION UI = "Panel_nt36860_wqhd_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8f276ee4-e75e-4c3b-8bcc-c37771e26478/section1.raw
    }

    FILE FREEFORM = 6f2d4884-d8a0-11e9-af7a-d77b1fe990ec {
            SECTION UI = "Panel_nt36860_wqhd_dsc_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-6f2d4884-d8a0-11e9-af7a-d77b1fe990ec/section1.raw
    }

    FILE FREEFORM = 81ce7295-491d-4e3f-aaa7-ab43fef2f615 {
            SECTION UI = "Panel_r66451_60hz_fhd_plus_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-81ce7295-491d-4e3f-aaa7-ab43fef2f615/section1.raw
    }

    FILE FREEFORM = 81ce7295-491d-4e3f-aaa7-ab43fef2f692 {
            SECTION UI = "Panel_r66451_fhd_plus_60hz_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-81ce7295-491d-4e3f-aaa7-ab43fef2f692/section1.raw
    }

    FILE FREEFORM = 8297c793-9861-422c-9f67-d447af1ea812 {
            SECTION UI = "Panel_r66451_120hz_fhd_plus_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8297c793-9861-422c-9f67-d447af1ea812/section1.raw
    }

    FILE FREEFORM = 61156380-eb8a-4618-940f-ab9bdea1eec2 {
            SECTION UI = "Panel_r66451_120hz_fhd_plus_dsc_cmd_hdk.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-61156380-eb8a-4618-940f-ab9bdea1eec2/section1.raw
    }

    FILE FREEFORM = 44b09226-c2b4-45ed-aa6f-6686db103978 {
            SECTION UI = "Panel_r66451_120hz_fhd_plus_dsc_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-44b09226-c2b4-45ed-aa6f-6686db103978/section1.raw
    }

    FILE FREEFORM = d3f8580c-ccb8-4804-bb03-776e7b542fc2 {
            SECTION UI = "Panel_secondary_truly_1080p_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-d3f8580c-ccb8-4804-bb03-776e7b542fc2/section1.raw
    }

    FILE FREEFORM = d192fee9-6a04-4be8-abfe-5534e2b457ec {
            SECTION UI = "Panel_secondary_truly_1080p_vid.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-d192fee9-6a04-4be8-abfe-5534e2b457ec/section1.raw
    }

    FILE FREEFORM = 13594e78-635e-49c9-80c7-f2a3ef8d959b {
            SECTION UI = "Panel_r66451_60hz_fhd_plus_dsc_cmd_cphy.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-13594e78-635e-49c9-80c7-f2a3ef8d959b/section1.raw
    }

    FILE FREEFORM = 40c19f7e-c171-4c05-9c56-de6e30aed8c9 {
            SECTION UI = "Panel_r66451_90hz_fhd_plus_dsc_vid_cphy.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-40c19f7e-c171-4c05-9c56-de6e30aed8c9/section1.raw
    }

    FILE FREEFORM = 7246c37a-04e7-4d15-8db6-638d8b59155e {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi0.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-7246c37a-04e7-4d15-8db6-638d8b59155e/section1.raw
    }

    FILE FREEFORM = 8ca934cc-4385-4d4e-bc8b-37e8ac0b0960 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi1.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-8ca934cc-4385-4d4e-bc8b-37e8ac0b0960/section1.raw
    }

    FILE FREEFORM = 4365fdd2-2a06-11eb-adc1-0242ac120002 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi0_ev2.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-4365fdd2-2a06-11eb-adc1-0242ac120002/section1.raw
    }

    FILE FREEFORM = 54de8cdc-2a06-11eb-adc1-0242ac120002 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi1_ev2.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-54de8cdc-2a06-11eb-adc1-0242ac120002/section1.raw
    }

    FILE FREEFORM = edc40719-989a-45d4-9442-1e35af98d8a6 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi0_ev3.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-edc40719-989a-45d4-9442-1e35af98d8a6/section1.raw
    }

    FILE FREEFORM = d0eec6e6-ee21-438a-a7fb-8252f1f0f25d {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi1_ev3.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-d0eec6e6-ee21-438a-a7fb-8252f1f0f25d/section1.raw
    }

    FILE FREEFORM = ba2f91c8-278e-447e-9121-373b0d812651 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi0_dv.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-ba2f91c8-278e-447e-9121-373b0d812651/section1.raw
    }

    FILE FREEFORM = b374f91e-464c-4325-9f9b-657035b9a9b6 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi1_dv.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-b374f91e-464c-4325-9f9b-657035b9a9b6/section1.raw
    }

    FILE FREEFORM = a21f6348-4f65-4bf0-bcdd-b8de64d9ab5a {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi0_mp.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-a21f6348-4f65-4bf0-bcdd-b8de64d9ab5a/section1.raw
    }

    FILE FREEFORM = 84267c04-7399-498b-b294-0ceb2b369169 {
            SECTION UI = "Surface_Elgin_Panel_cmd_dsc_dsi1_mp.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-84267c04-7399-498b-b294-0ceb2b369169/section1.raw
    }

    FILE FREEFORM = c32cb97a-d116-4d64-83b9-6ba1236057ad {
            SECTION UI = "Panel_boe_amoled_wqhd_dsc_cmd_demura.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-c32cb97a-d116-4d64-83b9-6ba1236057ad/section1.raw
    }

    FILE FREEFORM = 453d4ceb-2ec9-49a6-8eec-b233eb679591 {
            SECTION UI = "Panel_boe_amoled_wqhd_dsc_vid_demura.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-453d4ceb-2ec9-49a6-8eec-b233eb679591/section1.raw
    }

    FILE FREEFORM = 602cffcd-8a5e-438e-b432-bf1a656a2561 {
            SECTION UI = "Panel_secondary_r66451_fhd_plus_dsc_cmd.xml"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-602cffcd-8a5e-438e-b432-bf1a656a2561/section1.raw
    }

    FILE FREEFORM = a1e235de-e825-4591-9623-c43175811826 {
            SECTION UI = "SecParti.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-1/section0/section1/volume-0/file-a1e235de-e825-4591-9623-c43175811826/section1.raw
    }

    FILE FREEFORM = 21e9bdd9-6c3f-4f10-84a5-bbec322741f1 {
            SECTION UI = "uefipil.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-21e9bdd9-6c3f-4f10-84a5-bbec322741f1/section1.raw
    }

    FILE FREEFORM = 45fe4b7c-150c-45da-a021-4beb2048ec6f {
            SECTION UI = "QcomChargerCfg.cfg"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-45fe4b7c-150c-45da-a021-4beb2048ec6f/section1.raw
    }

    FILE FREEFORM = f780c779-dd7c-47cd-bd1a-5eb414c51704 {
            SECTION UI = "BATTERY.PROVISION"
            SECTION RAW = SurfaceDuoBinaries/BOOT.MXF.1.0-00872-LAHAINA-2/volume-0/file-2/section0/section1/volume-0/file-f780c779-dd7c-47cd-bd1a-5eb414c51704/section1.raw
    }