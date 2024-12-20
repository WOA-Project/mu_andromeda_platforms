/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#include <Library/BaseLib.h>
#include <Library/PlatformConfigurationMapLib.h>

static CONFIGURATION_DESCRIPTOR_EX gDeviceConfigurationDescriptorEx[] = {
    {"AllowNonPersistentVarsInRetail", 0x1},
    {"APPS_HOB_ADDRESS", 0xE3400000},
    {"APPS_HOB_SIZE", 0x00001000},
    {"DDRInfoNotifyFlag", 0x0},
    {"DetectRetailUserAttentionHotkey", 0x00},
    {"DetectRetailUserAttentionHotkeyCode", 0x17},
    {"DloadCookieAddr", 0x01FD3000},
    {"DloadCookieValue", 0x10},
    {"EarlyInitCoreCnt", 2},
    {"EnableACPIFallback", 0x0},
    {"EnableDisplayImageFv", 0x0},
    {"EnableDisplayThread", 0x1},
    {"EnableLogFsSyncInRetail", 0x1},
    {"EnableMultiCoreFvDecompression", 1},
    {"EnableMultiThreading", 1},
    {"EnableOEMSetupAppInRetail", 0x0},
    {"EnableSDHCSwitch", 0x1},
    {"EnableShell", 0x1},
    {"EnableUefiSecAppDebugLogDump", 0x0},
    {"EnableUfsIOC", 1},
    {"EnableVariablePolicyEngine", 0},
    {"EUDEnableAddr", 0x88E2000},
    {"InitialPagePoolCount", 0x900},
    {"MaxCoreCnt", 8},
    {"MaxLogFileSize", 0x400000},
    {"MinidumpTALoadingCfg", 0x0},
    {"MS_PON_HOB_ADDR", 0xE3401000},
    {"MS_PON_HOB_SIZE", 0x00001000},
    {"NumActiveCores", 8},
    {"NumCpus", 8},
    {"PwrBtnShutdownFlag", 0x0},
    {"Sdc1GpioConfigOff", 0xA00},
    {"Sdc1GpioConfigOn", 0x1E92},
    {"Sdc2GpioConfigOff", 0xA00},
    {"Sdc2GpioConfigOn", 0x1E92},
    {"SecurityFlag", 0x1C4},
    {"SharedIMEMBaseAddr", 0x146BF000},
    {"ShmBridgememSize", 0xA00000},
    {"SmcInvokeConfig", 0x20040100},
    {"UefiMemUseThreshold", 0xE1},
    {"UfsSmmuConfigForOtherBootDev", 1},
    {"UsbFnIoRevNum", 0x00010001},
    {"USBHS1_Config", 0x0},
    /* Terminator */
    {"Terminator", 0xFFFFFFFF}};

CONFIGURATION_DESCRIPTOR_EX *GetPlatformConfigurationMap()
{
  return gDeviceConfigurationDescriptorEx;
}