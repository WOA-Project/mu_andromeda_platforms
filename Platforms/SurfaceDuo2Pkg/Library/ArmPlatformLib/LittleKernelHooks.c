#include "Pi.h"
#include <Protocol/EFIKernelInterface.h>

EFI_KERNEL_PROTOCOL *pSchedIntf = (EFI_KERNEL_PROTOCOL *)0x9FC37980;

INT32
MpcoreSleepCpu(UINT64 DurationMs)
{
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "MpcoreSleepCpu\n"));
  return 0;
}

EFI_STATUS
RegisterPwrTransitionNotify(PwrTxnNotifyFn CbFn, VOID *Arg)
{
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "RegisterPwrTransitionNotify\n"));
  return EFI_SUCCESS;
}

EFI_STATUS
UnRegisterPwrTransitionNotify(PwrTxnNotifyFn CbFn)
{
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "UnRegisterPwrTransitionNotify\n"));
  return EFI_SUCCESS;
}

VOID MpcoreSleepLoggingControl(UINT32 Disable)
{
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "MpcoreSleepLoggingControl\n"));
}

typedef VOID (*LITTLE_KERNEL_ARM_GIC_V3_INIT)(UINT32 level);
typedef VOID (*LITTLE_KERNEL_INIT_PRIMARY_CPU)();

VOID PlatformSchedulerInit()
{
  UINT32 SchedulingLibVersion = pSchedIntf->GetLibVersion();
  UINT32 MajorVersion         = (SchedulingLibVersion >> 16) & 0xFFFF;
  UINT32 MinorVersion         = SchedulingLibVersion & 0xFFFF;

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD, "Scheduling Library Version %d.%d\n",
       MajorVersion, MinorVersion));

  pSchedIntf->MpCpu->RegisterPwrTransitionNotify = &RegisterPwrTransitionNotify;
  pSchedIntf->MpCpu->UnRegisterPwrTransitionNotify =
      &UnRegisterPwrTransitionNotify;
  pSchedIntf->MpCpu->MpcoreSleepLoggingControl = &MpcoreSleepLoggingControl;

  UINT32 MaxCpuCount   = pSchedIntf->MpCpu->MpcoreGetMaxCpuCount();
  UINT32 AvailCpuCount = pSchedIntf->MpCpu->MpcoreGetAvailCpuCount();

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Maximum CPU Count: %d\n", MaxCpuCount));
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Available CPU Count: %d\n", AvailCpuCount));

  for (UINT32 i = 0; i < MaxCpuCount; i++) {
    if (pSchedIntf->MpCpu->MpcoreIsCpuActive(i)) {
      DEBUG((EFI_D_INFO | EFI_D_LOAD, "CPU %d is online.\n", i));

      if (i != 0) {
        DEBUG((EFI_D_INFO | EFI_D_LOAD, "Shutting down CPU %d...\n", i));
        pSchedIntf->MpCpu->MpcorePowerOffCpu(1 << i);
      }
    }
    else {
      DEBUG((EFI_D_INFO | EFI_D_LOAD, "CPU %d is offline.\n", i));
    }
  }

  LITTLE_KERNEL_ARM_GIC_V3_INIT arm_gic_v3_init =
      (LITTLE_KERNEL_ARM_GIC_V3_INIT)0x9FC06658;
  LITTLE_KERNEL_ARM_GIC_V3_INIT gic_init_percpu_early =
      (LITTLE_KERNEL_ARM_GIC_V3_INIT)0x9FC064E0;

  DEBUG((
      EFI_D_INFO | EFI_D_LOAD, "Reinitializing GICv3 using LittleKernel...\n"));

  // Reinitialize GICv3
  arm_gic_v3_init(0);
  gic_init_percpu_early(0);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICv3 successfully reinitialized.\n"));
}