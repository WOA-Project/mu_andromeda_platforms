#include <Library/ArmGicLib.h>
#include <Library/ArmLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmSmcLib.h>
#include <Library/PlatformPrePiLib.h>

#include "PlatformUtils.h"
#include <Configuration/DeviceMemoryMap.h>

BOOLEAN IsLinuxBootRequested()
{
  return (MmioRead32(LID0_GPIO38_STATUS_ADDR) & 1) == 0;
}

EFI_STATUS
EFIAPI
SerialPortLocateArea(PARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryDescriptor)
{
  PARM_MEMORY_REGION_DESCRIPTOR_EX MemoryDescriptorEx =
      gDeviceMemoryDescriptorEx;

  // Run through each memory descriptor
  while (MemoryDescriptorEx->Length != 0) {
    if (AsciiStriCmp("PStore", MemoryDescriptorEx->Name) == 0) {
      *MemoryDescriptor = MemoryDescriptorEx;
      return EFI_SUCCESS;
    }
    MemoryDescriptorEx++;
  }

  return EFI_NOT_FOUND;
}

VOID InitializeSharedUartBuffers(VOID)
{
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  PARM_MEMORY_REGION_DESCRIPTOR_EX PStoreMemoryRegion = NULL;
#endif

  INTN *pFbConPosition =
      (INTN
           *)(FixedPcdGet32(PcdMipiFrameBufferAddress) + (FixedPcdGet32(PcdMipiFrameBufferWidth) * FixedPcdGet32(PcdMipiFrameBufferHeight) * FixedPcdGet32(PcdMipiFrameBufferPixelBpp) / 8));

  *(pFbConPosition + 0) = 0;
  *(pFbConPosition + 1) = 0;

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  // Clear PStore area
  SerialPortLocateArea(&PStoreMemoryRegion);
  UINT8 *base = (UINT8 *)PStoreMemoryRegion->Address;
  for (UINTN i = 0; i < PStoreMemoryRegion->Length; i++) {
    base[i] = 0;
  }
#endif
}

VOID UartInit(VOID)
{
  SerialPortInitialize();
  InitializeSharedUartBuffers();

  DEBUG((EFI_D_INFO, "\nProjectMu on Duo 2 (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

VOID DoSmcCall(ARM_SMC_ARGS *SmcArgs)
{
  ARM_SMC_ARGS StubArgsSmc;
  UINTN        InterruptId;
  UINTN        AcknowledgeInterrupt;

  UINT32 AttemptCount = 0;

  // Enable gic cpu interface
  ArmGicV3EnableInterruptInterface();

  // Enable gic distributor
  ArmGicEnableDistributor(PcdGet64(PcdGicDistributorBase));

  // Enable interrupts
  ArmEnableInterrupts();

  do {
    CopyMem(&StubArgsSmc, SmcArgs, sizeof(ARM_SMC_ARGS));
    ArmCallSmc(&StubArgsSmc);
    AttemptCount++;

    AcknowledgeInterrupt = ArmGicAcknowledgeInterrupt(
        FixedPcdGet64(PcdGicInterruptInterfaceBase), &InterruptId);
    if (InterruptId <
        ArmGicGetMaxNumInterrupts(FixedPcdGet64(PcdGicDistributorBase))) {
      // Got a valid SGI number hence signal End of Interrupt
      ArmGicEndOfInterrupt(
          FixedPcdGet64(PcdGicInterruptInterfaceBase), AcknowledgeInterrupt);
    }
  } while (StubArgsSmc.Arg0 == 1 &&
           AttemptCount < 1000); // Interrupted error code

  // Disable interrupts
  ArmDisableInterrupts();

  // Disable Gic Interface
  ArmGicV3DisableInterruptInterface();

  // Disable Gic Distributor
  ArmGicDisableDistributor(PcdGet64(PcdGicDistributorBase));

  CopyMem(SmcArgs, &StubArgsSmc, sizeof(ARM_SMC_ARGS));
}

VOID SetWatchdogState(BOOLEAN Enable)
{
  ARM_SMC_ARGS StubArgsSmc;

  StubArgsSmc.Arg0 = QHEE_SMC_VENDOR_HYP_WDOG_CONTROL;
  StubArgsSmc.Arg1 = Enable ? 3 : 2;
  StubArgsSmc.Arg2 = 0;
  StubArgsSmc.Arg3 = 0;
  StubArgsSmc.Arg4 = 0;
  StubArgsSmc.Arg5 = 0;
  StubArgsSmc.Arg6 = 0;
  StubArgsSmc.Arg7 = 0;
  DoSmcCall(&StubArgsSmc);

  if (StubArgsSmc.Arg0 == 1) {
    DEBUG(
        (EFI_D_ERROR,
         "Qualcomm Watchdog Reboot timer could not be disabled due to SMC call "
         "processing being interrupted by another CPU.\n"));
  }
  else if (StubArgsSmc.Arg0 != 0) {
    DEBUG(
        (EFI_D_ERROR,
         "Disabling Qualcomm Watchdog Reboot timer failed! Status=%d\n",
         StubArgsSmc.Arg0));
  }
}

VOID SetHypervisorUartState(BOOLEAN Enable)
{
  ARM_SMC_ARGS StubArgsSmc;

  StubArgsSmc.Arg0 = Enable ? QHEE_SMC_VENDOR_HYP_UART_ENABLE
                            : QHEE_SMC_VENDOR_HYP_UART_DISABLE;
  StubArgsSmc.Arg1 = 0;
  StubArgsSmc.Arg2 = 0;
  StubArgsSmc.Arg3 = 0;
  StubArgsSmc.Arg4 = 0;
  StubArgsSmc.Arg5 = 0;
  StubArgsSmc.Arg6 = 0;
  StubArgsSmc.Arg7 = 0;
  DoSmcCall(&StubArgsSmc);

  if (StubArgsSmc.Arg0 == 1) {
    DEBUG(
        (EFI_D_ERROR,
         "Qualcomm Hypervisor UART Logging could not be toggled due to SMC "
         "call processing being interrupted by another CPU.\n"));
  }
  else if (StubArgsSmc.Arg0 != 0) {
    DEBUG(
        (EFI_D_ERROR,
         "Toggling Qualcomm Hypervisor UART Logging failed! Status=%d\n",
         StubArgsSmc.Arg0));
  }
}

EFI_STATUS
EFIAPI
QGicEarlyConfiguration(VOID)
{
  // Enable gic distributor
  // ArmGicEnableDistributor(PcdGet64(PcdGicDistributorBase));

  for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    // Wake up GIC Redistributor for this CPU
    MmioWrite32(
        PcdGet64(PcdGicRedistributorsBase) + CpuId * GICR_SIZE + GICR_WAKER, 0);

    /*// Deactivate Interrupts for this CPU
    MmioWrite32(
        PcdGet64(PcdGicRedistributorsBase) + CpuId * GICR_SIZE + GICR_SGI +
            GICR_ICENABLER0,
        0);

    // Clear Pending Interrupts for this CPU
    MmioWrite32(
        PcdGet64(PcdGicRedistributorsBase) + CpuId * GICR_SIZE + GICR_SGI +
            GICR_ICPENDR0,
        0x10000000);*/
  }

  // Disable Gic Distributor
  // ArmGicDisableDistributor(PcdGet64(PcdGicDistributorBase));

  return EFI_SUCCESS;
}

VOID PlatformInitialize()
{
  // Initialize UART Serial
  UartInit();

  // Configure Qualcomm GIC Early
  QGicEarlyConfiguration();

#if PREFER_MPPARK_OVER_SMC_PSCI == 1
  // Launch all 8 CPUs for Multi Processor Parking Protocol
  LaunchAllCPUs();
#endif

  // Enable Hypervisor UART
  // SetHypervisorUartState(TRUE);

  // Disable WatchDog Timer
  // SetWatchdogState(FALSE);
}

VOID SecondaryPlatformInitialize(UINTN MpIdr)
{
#if PREFER_MPPARK_OVER_SMC_PSCI == 1
  // Initialize Secondary CPU via MpPark
  MpParkMain(MpIdr);
#endif
}