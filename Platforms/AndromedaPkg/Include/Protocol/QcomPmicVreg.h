/** @file
  Qualcomm PMIC voltage regulator protocol.

  SPDX-License-Identifier: MIT
**/
#ifndef QCOM_PMIC_VREG_H_
#define QCOM_PMIC_VREG_H_

#include <Uefi.h>

extern EFI_GUID gQcomPmicVregProtocolGuid;

typedef struct {
  UINT32  PullDown;
  UINT32  SwMode;
  UINT32  PinCtrled;
  UINT32  SwEnable;
  BOOLEAN VregOk;
} PM_VREG_STATUS;

typedef struct {
  UINT64 Revision;
  EFI_STATUS (EFIAPI *VregControl)(UINT32 Pmic, UINT32 VregId, BOOLEAN Enable);
  EFI_STATUS (EFIAPI *VregSetLevel)(UINT32 Pmic, UINT32 VregId, UINT32 Mv);
  EFI_STATUS (EFIAPI *VregGetLevel)(UINT32 Pmic, UINT32 VregId, UINT32 *Microvolts);
  EFI_STATUS (EFIAPI *VregSetPwrMode)(UINT32 Pmic, UINT32 VregId, UINT32 Mode);
  EFI_STATUS (EFIAPI *VregMultiphaseCtrl)(UINT32 Pmic, UINT32 VregId, UINT32 Phases);
  EFI_STATUS (EFIAPI *VregGetStatus)(UINT32 Pmic, UINT32 VregId, PM_VREG_STATUS *Status);
  EFI_STATUS (EFIAPI *VregSetLevelUv)(UINT32 Pmic, UINT32 VregId, UINT32 Uv);
} PM_VREG_PROTOCOL;

#endif
