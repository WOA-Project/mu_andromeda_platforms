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
  PLATFORM_NAME                  = SurfaceDuo1
  PLATFORM_GUID                  = b6325ac2-9f3f-4b1d-b129-ac7b35ddde60
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/SurfaceDuo1-$(ARCH)
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SurfaceDuo1Pkg/SurfaceDuo1.fdf
  SECURE_BOOT_ENABLE             = 1
  USE_PHYSICAL_TIMER             = 1
  USE_SCREEN_FOR_SERIAL_OUTPUT   = 0
  USE_MEMORY_FOR_SERIAL_OUTPUT   = 1
  SEND_HEARTBEAT_TO_SERIAL       = 0

[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x180000000        # 6GB Size

  # SMBIOS
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemModel|"Surface Duo"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"1930"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Surface_Duo_1930"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosBoardModel|"Surface Duo"

  # Simple FrameBuffer
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1350
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1800
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32

[PcdsDynamicDefault.common]
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoHorizontalResolution|1350
  gEfiMdeModulePkgTokenSpaceGuid.PcdVideoVerticalResolution|1800
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoHorizontalResolution|1350
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupVideoVerticalResolution|1800
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|94 # 94.73
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|168 # 168.75
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|94 # 94.73
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|168 # 168.75

!include QcomPkg/QcomPkg.dsc.inc
!include SurfaceDuoFamilyPkg/SurfaceDuoFamily.dsc.inc
!include SurfaceDuoFamilyPkg/Frontpage.dsc.inc