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
  OUTPUT_DIRECTORY               = Build/SurfaceDuo1Pkg
  SUPPORTED_ARCHITECTURES        = AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT
  FLASH_DEFINITION               = SurfaceDuo1Pkg/SurfaceDuo1.fdf
  SECURE_BOOT_ENABLE             = TRUE
  USE_PHYSICAL_TIMER             = 1
  USE_SCREEN_FOR_SERIAL_OUTPUT   = 0
  USE_MEMORY_FOR_SERIAL_OUTPUT   = 0
  SEND_HEARTBEAT_TO_SERIAL       = 0

  DEFAULT_KEYS                   = TRUE
  PK_DEFAULT_FILE                = SurfaceDuoFamilyPkg/Include/Resources/pk.bin.p7
  KEK_DEFAULT_FILE1              = SurfaceDuoFamilyPkg/Include/Resources/kek.bin.p7
  DB_DEFAULT_FILE1               = SurfaceDuoFamilyPkg/Include/Resources/db.bin.p7
  DBX_DEFAULT_FILE1              = SurfaceDuoFamilyPkg/Include/Resources/dbx.bin

  DXE_CRYPTO_SERVICES = STANDARD
  DXE_CRYPTO_ARCH = AARCH64
  PEI_CRYPTO_SERVICES = NONE 
  SMM_CRYPTO_SERVICES = NONE

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
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutColumn|168 # 168.75 = 1350 / EFI_GLYPH_WIDTH(8)
  gEfiMdeModulePkgTokenSpaceGuid.PcdSetupConOutRow|94 # 94.73 = 1800 / EFI_GLYPH_HEIGHT(19)
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutColumn|168 # 168.75 = 1350 / EFI_GLYPH_WIDTH(8)
  gEfiMdeModulePkgTokenSpaceGuid.PcdConOutRow|94 # 94.73 = 1800 / EFI_GLYPH_HEIGHT(19)

!include QcomPkg/QcomPkg.dsc.inc
!include SurfaceDuoFamilyPkg/SurfaceDuoFamily.dsc.inc
!include SurfaceDuoFamilyPkg/Frontpage.dsc.inc

[Components.common]
  SurfaceDuo1Pkg/AcpiTables/AcpiTables.inf
