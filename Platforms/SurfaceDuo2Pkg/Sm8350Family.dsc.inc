## @file
#
#  Copyright (c) 2011-2015, ARM Limited. All rights reserved.
#  Copyright (c) 2014, Linaro Limited. All rights reserved.
#  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.
#  Copyright (c) 2018 - 2019, Bingxing Wang. All rights reserved.
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
  DEFINE SILICON_PLATFORM = 8350

[PcdsFixedAtBuild.common]
  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # Starting address
  gArmTokenSpaceGuid.PcdCpuVectorBaseAddress|0x9f58c000
  gArmTokenSpaceGuid.PcdArmArchTimerFreqInHz|19200000
  gArmTokenSpaceGuid.PcdArmArchTimerSecIntrNum|29
  gArmTokenSpaceGuid.PcdArmArchTimerIntrNum|30
  gArmTokenSpaceGuid.PcdGicDistributorBase|0x17A00000
  gArmTokenSpaceGuid.PcdGicRedistributorsBase|0x17B40000

  gEfiMdeModulePkgTokenSpaceGuid.PcdAcpiDefaultOemRevision|0x00008280
  gEmbeddedTokenSpaceGuid.PcdPrePiStackBase|0x9f590000
  gEmbeddedTokenSpaceGuid.PcdPrePiStackSize|0x00040000      # 256K stack
  gEmbeddedTokenSpaceGuid.PcdInterruptBaseAddress|0x09BC0000
  gEmbeddedTokenSpaceGuid.PcdPrePiCpuIoSize|44

  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdUefiMemPoolSize|0x10C00000
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdPreAllocatedMemorySize|0x0E400000  # Start here

  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosProcessorModel|"Snapdragon (TM) 888 @ 2.84 GHz"
  gSurfaceDuoFamilyPkgTokenSpaceGuid.PcdSmbiosProcessorRetailModel|"SM8350"

  gArmTokenSpaceGuid.PcdSystemMemoryBase|0x80000000         # 2GB Base
  gArmPlatformTokenSpaceGuid.PcdCoreCount|8
  gArmPlatformTokenSpaceGuid.PcdClusterCount|3

[LibraryClasses.common]
  MemoryInitPeiLib|SurfaceDuo2Pkg/Library/MemoryInitPeiLib/PeiMemoryAllocationLib.inf
  PlatformPeiLib|SurfaceDuo2Pkg/Library/PlatformPeiLib/PlatformPeiLib.inf
  PlatformPrePiLib|SurfaceDuo2Pkg/Library/PlatformPrePiLib/PlatformPrePiLib.inf
  AslUpdateLib|SurfaceDuoFamilyPkg/Library/DxeAslUpdateLib/DxeAslUpdateLib.inf

  MsPlatformDevicesLib|SurfaceDuo2Pkg/Library/MsPlatformDevicesLib/MsPlatformDevicesLib.inf
  PlatformThemeLib|SurfaceDuo2Pkg/Library/PlatformThemeLib/PlatformThemeLib.inf

[Components.common]
  # Graphics Driver
  SurfaceDuoFamilyPkg/Driver/SimpleFbDxe/SimpleFbDxe.inf

  # ACPI and SMBIOS
  # We use pre-built ACPI tables, so only core components are loaded
  SurfaceDuo2Pkg/AcpiTables/SurfaceDuo2AcpiTables.inf

  MdeModulePkg/Universal/TimestampDxe/TimestampDxe.inf

  SurfaceDuo2Pkg/Driver/GpioButtons/GpioButtons.inf

  SurfaceDuo2Pkg/Driver/KernelErrataPatcher/KernelErrataPatcher.inf