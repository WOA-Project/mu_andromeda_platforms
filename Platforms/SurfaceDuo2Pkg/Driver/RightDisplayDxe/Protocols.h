#ifndef RIGHT_DISPLAY_DXE_PROTOCOLS_H_
#define RIGHT_DISPLAY_DXE_PROTOCOLS_H_

#include <Protocol/QcomPmicVreg.h>

extern EFI_GUID gQcomPmicClkBuffProtocolGuid;

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
