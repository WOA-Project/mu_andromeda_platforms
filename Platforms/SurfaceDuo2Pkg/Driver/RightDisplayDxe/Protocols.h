#ifndef RIGHT_DISPLAY_DXE_PROTOCOLS_H_
#define RIGHT_DISPLAY_DXE_PROTOCOLS_H_

extern EFI_GUID gQcomPmicVregProtocolGuid;
extern EFI_GUID gQcomPmicClkBuffProtocolGuid;

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
  EFI_STATUS (EFIAPI *VregGetLevel)(UINT32 Pmic, UINT32 VregId, UINT32 *Mv);
  EFI_STATUS (EFIAPI *VregSetPwrMode)(UINT32 Pmic, UINT32 VregId, UINT32 Mode);
  EFI_STATUS (EFIAPI *VregMultiphaseCtrl)(UINT32 Pmic, UINT32 VregId, UINT32 Phases);
  EFI_STATUS (EFIAPI *VregGetStatus)(UINT32 Pmic, UINT32 VregId, PM_VREG_STATUS *Status);
  EFI_STATUS (EFIAPI *VregSetLevelUv)(UINT32 Pmic, UINT32 VregId, UINT32 Uv);
} PM_VREG_PROTOCOL;

typedef struct {
  UINT32  DrvStrength;
  BOOLEAN SwEnable;
  BOOLEAN PinCtrled;
  BOOLEAN PullDown;
  UINT32  OutDiv;
} PM_CLK_STATUS;

typedef struct {
  UINT64 Revision;
  EFI_STATUS (EFIAPI *ClkBuffEnable)(UINT32 Pmic, UINT32 ClkType, BOOLEAN Enable);
  EFI_STATUS (EFIAPI *SetOutDrvStrength)(UINT32 Pmic, UINT32 ClkType, UINT32 Strength);
  EFI_STATUS (EFIAPI *SetOutClkDiv)(UINT32 Pmic, UINT32 ClkType, UINT32 Div);
  EFI_STATUS (EFIAPI *GetStatus)(UINT32 Pmic, UINT32 ClkType, PM_CLK_STATUS *Status);
} PM_CLKBUFF_PROTOCOL;

#endif
