/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/
#ifndef _QCOM_CONFIG_PARAMETERS_HELPER_LIB_H_
#define _QCOM_CONFIG_PARAMETERS_HELPER_LIB_H_

#include <PiPei.h>
#include <Library/ArmLib.h>

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT32ByName(
    CHAR8  *ConfigurationMapUINT32Name,
    UINT32 *ConfigurationDescriptor);

EFI_STATUS EFIAPI QcomLocateConfigurationMapUINT64ByName(
    CHAR8  *ConfigurationMapUINT64Name,
    UINT64 *ConfigurationDescriptor);

EFI_STATUS EFIAPI QcomLocateConfigurationMapStringByName (
    CHAR8  *ConfigurationMapStringName,
    CHAR8  *ConfigurationDescriptor,
    UINTN  *ConfigurationDescriptorSize);

#endif /* _QCOM_CONFIG_PARAMETERS_HELPER_LIB_H_ */