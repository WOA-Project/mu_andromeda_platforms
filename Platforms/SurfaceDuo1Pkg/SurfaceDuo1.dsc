#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018, Bingxing Wang. All rights reserved.
#
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution.  The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.php
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
#

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

  DEFINE SECURE_BOOT_ENABLE           = TRUE
  DEFINE USE_SCREEN_FOR_SERIAL_OUTPUT = 0
  DEFINE USE_MEMORY_FOR_SERIAL_OUTPUT = 0

[BuildOptions.common]
!if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8150 -DUSE_MEMORY_FOR_SERIAL_OUTPUT=1
!else
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8150
!endif
  
[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # 2GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x180000000        # 6GB Size

  # SMBIOS
  gSurfacePkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 855 @ 2.84 GHz"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8150"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemModel|"Surface Duo"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"1930"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Surface_Duo_1930"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosBoardModel|"Surface Duo"

  # Simple FrameBuffer
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferAddress|0x80600000
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1350
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1800
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32

  # PStore
  gSurfacePkgTokenSpaceGuid.PcdPStoreBufferAddress|0x17FE00000
  gSurfacePkgTokenSpaceGuid.PcdPStoreBufferSize|0x00200000
  
!include SurfaceDuo1Pkg/Shared.dsc.inc
!include SurfacePkg/FrontpageDsc.inc