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
  DEFINE SEND_HEARTBEAT_TO_SERIAL     = 1

[BuildOptions.common]
!if $(USE_MEMORY_FOR_SERIAL_OUTPUT) == 1
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8350 -DUSE_MEMORY_FOR_SERIAL_OUTPUT=1 -DSEND_HEARTBEAT_TO_SERIAL=1
!else
  GCC:*_*_AARCH64_CC_FLAGS = -DSILICON_PLATFORM=8350 -DSEND_HEARTBEAT_TO_SERIAL=1
!endif
  
[PcdsFixedAtBuild.common]
  # Platform-specific
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # 2GB Base
  gArmTokenSpaceGuid.PcdSystemMemorySize|0x200000000        # 8GB Size
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3

  # SMBIOS
  gSurfacePkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 888 @ 2.84 GHz"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8350"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemModel|"Surface Duo 2"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemRetailModel|"1995"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosSystemRetailSku|"Surface_Duo_2_1995"
  gSurfacePkgTokenSpaceGuid.PcdSmbiosBoardModel|"Surface Duo 2"

  # Simple FrameBuffer
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferAddress|0xe1000000
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferWidth|1344
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferHeight|1892
  gSurfacePkgTokenSpaceGuid.PcdMipiFrameBufferPixelBpp|32

  # PStore
  gSurfacePkgTokenSpaceGuid.PcdPStoreBufferAddress|0x1BFE00000
  gSurfacePkgTokenSpaceGuid.PcdPStoreBufferSize|0x00200000
  
!include SurfaceDuo2Pkg/Shared.dsc.inc
!include SurfacePkg/FrontpageDsc.inc