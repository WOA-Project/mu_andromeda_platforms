## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#
#  SPDX-License-Identifier: BSD-2-Clause-Patent
#
##

################################################################################
#
# Defines Section - statements that will be processed to create a Makefile.
#
################################################################################
[Defines]
  PLATFORM_NAME                  = SurfaceDuo2
  PLATFORM_GUID                  = b6325ac2-9f3f-4b1d-b129-ac7b35ddde60
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SurfaceDuo2-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SurfaceDuo2Pkg/SurfaceDuo2.fdf

  DEFINE SECURE_BOOT_ENABLE           = TRUE
  DEFINE USE_SCREEN_FOR_SERIAL_OUTPUT = 1
  DEFINE USE_MEMORY_FOR_SERIAL_OUTPUT = 0
  DEFINE SEND_HEARTBEAT_TO_SERIAL     = 0
  DEFINE PREFER_MPPARK_OVER_SMC_PSCI  = 0

[BuildOptions.common]
  *_*_*_CC_FLAGS = -DPREFER_MPPARK_OVER_SMC_PSCI=$(PREFER_MPPARK_OVER_SMC_PSCI)

[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # 2GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000        # 8GB Size
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3

  # SMBIOS
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Surface Duo 2"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"1995"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Surface_Duo_2_1995"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosBoardModel|"Surface Duo 2"

  # Simple FrameBuffer
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferAddress|0xe1000000
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1344
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1892
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32

  # PStore
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdPStoreBufferAddress|0x1BFE00000
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdPStoreBufferSize|0x00200000

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1344
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1892
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1344
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1892
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|158
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|150

!include SurfaceDuo2Pkg/Sm8350Family.dsc.inc
!include SurfaceDuoFamilyPkg/SurfaceDuoFamily.dsc.inc
!include SurfaceDuoFamilyPkg/Frontpage.dsc.inc