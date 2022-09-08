#include "Pi.h"

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#define TLMM_ADDR 0x0F100000

#define TLMM_ADDR_OFFSET_FOR_PIN(x) (0x1000 * x)

#define TLMM_PIN_CONTROL_REGISTER 0
#define TLMM_PIN_IO_REGISTER 4
#define TLMM_PIN_INTERRUPT_CONFIG_REGISTER 8
#define TLMM_PIN_INTERRUPT_STATUS_REGISTER 0xC
#define TLMM_PIN_INTERRUPT_TARGET_REGISTER TLMM_PIN_INTERRUPT_CONFIG_REGISTER

#define LID0_GPIO38_STATUS_ADDR                                                \
  (TLMM_ADDR + TLMM_ADDR_OFFSET_FOR_PIN(38) + TLMM_PIN_IO_REGISTER)

BOOLEAN IsLinuxBootRequested()
{
  return (MmioRead32(LID0_GPIO38_STATUS_ADDR) & 1) == 0;
}

VOID PlatformInitialize()
{
  // ARM_SMC_ARGS StubArgsSmc;
  
  // Disable WatchDog Timer
  // StubArgsSmc.Arg0 = 0x86000005;
  // StubArgsSmc.Arg1 = 2;
  // ArmCallSmc(&StubArgsSmc);
  // if (StubArgsSmc.Arg0 != 0) {
  //   DEBUG(
  //       (EFI_D_ERROR,
  //        "Disabling Qualcomm Watchdog Reboot timer failed! Status=%d\n",
  //        StubArgsSmc.Arg0));
  // }

  // PlatformSchedulerInit();

  // Initialize GIC
  if (!FixedPcdGetBool(PcdIsLkBuild)) {
    if (EFI_ERROR(QGicPeim())) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }
}