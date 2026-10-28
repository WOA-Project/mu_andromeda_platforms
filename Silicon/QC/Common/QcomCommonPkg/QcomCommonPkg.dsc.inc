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

[LibraryClasses.common]
  QcomUefiCfgConfigParametersHelperLib|QcomCommonPkg/Library/QcomUefiCfgConfigParametersHelperLib/QcomUefiCfgConfigParametersHelperLib.inf
  QcomUefiCfgMemoryMapHelperLib|QcomCommonPkg/Library/QcomUefiCfgMemoryMapHelperLib/QcomUefiCfgMemoryMapHelperLib.inf
  QcomUefiPlatMemoryMapLib|QcomCommonPkg/Library/QcomUefiPlatMemoryMapLib/QcomUefiPlatMemoryMapLib.inf
  ShLib|QcomCommonPkg/Library/ShLib/ShLib.inf
  UefiCfgLib|QcomCommonPkg/Library/UefiCfgLib/UefiCfgLib.inf
  UfsInfoLib|QcomCommonPkg/Library/UfsInfoLib/UfsInfoLib.inf

[LibraryClasses.common.SEC]
  QcomUefiCfgConfigParametersHelperLib|QcomCommonPkg/Library/QcomUefiPlatConfigParametersHelperLib/QcomUefiPlatConfigParametersHelperLib.inf
  QcomUefiCfgMemoryMapHelperLib|QcomCommonPkg/Library/QcomUefiPlatMemoryMapHelperLib/QcomUefiPlatMemoryMapHelperLib.inf

[Components.common]
  # ACPI and SMBIOS
  # We use pre-built ACPI tables, so only core components are loaded
  QcomCommonPkg/Driver/SmBiosTableDxe/SmBiosTableDxe.inf

  QcomModulePkg/Application/LinuxLoader/LinuxLoader.inf {
  	<LibraryClasses>
  		DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  		UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  		UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  		CacheMaintenanceLib|ArmPkg/Library/ArmCacheMaintenanceLib/ArmCacheMaintenanceLib.inf
  		ArmLib|MdePkg/Library/ArmLib/ArmBaseLib.inf
  		BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  		DebugLib|MdePkg/Library/UefiDebugLibConOut/UefiDebugLibConOut.inf
  		DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  		HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  		PerformanceLib|MdeModulePkg/Library/DxePerformanceLib/DxePerformanceLib.inf
  		DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  
  		FdtLib|MdePkg/Library/BaseFdtLib/BaseFdtLib.inf
  		ArmSmcLib|MdePkg/Library/ArmSmcLib/ArmSmcLib.inf
  
  		AvbLib|QcomModulePkg/Library/avb/AvbLib.inf
  		BootLib|QcomModulePkg/Library/BootLib/BootLib.inf
  		FastbootLib|QcomModulePkg/Library/FastbootLib/FastbootLib.inf
  		LibUfdt|QcomModulePkg/Library/LibUfdt/LibUfdt.inf
  		StackCanary|QcomModulePkg/Library/StackCanary/StackCanary.inf
  		UbsanLib|QcomModulePkg/Library/UbsanLib/UbsanLib.inf
  		Zlib|QcomModulePkg/Library/zlib/zlib.inf
  }