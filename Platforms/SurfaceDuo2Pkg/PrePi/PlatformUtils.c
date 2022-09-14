#include "Pi.h"

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#include "PlatformUtils.h"
#include "EarlyQGic/EarlyQGic.h"
#include <Configuration/DeviceMemoryMap.h>

BOOLEAN IsLinuxBootRequested()
{
  return (MmioRead32(LID0_GPIO38_STATUS_ADDR) & 1) == 0;
}

EFI_STATUS
EFIAPI
SerialPortLocateArea(PARM_MEMORY_REGION_DESCRIPTOR_EX* MemoryDescriptor)
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

  INTN* pFbConPosition = (INTN*)(FixedPcdGet32(PcdMipiFrameBufferAddress) + (FixedPcdGet32(PcdMipiFrameBufferWidth) * 
                                                                              FixedPcdGet32(PcdMipiFrameBufferHeight) * 
                                                                              FixedPcdGet32(PcdMipiFrameBufferPixelBpp) / 8));

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

VOID SetWatchdogState(BOOLEAN Enable)
{
  if (!Enable) {
    ARM_SMC_ARGS StubArgsSmc;
    StubArgsSmc.Arg0 = 0x86000005;
    StubArgsSmc.Arg1 = 2;
    ArmCallSmc(&StubArgsSmc);
    if (StubArgsSmc.Arg0 != 0) {
      DEBUG(
          (EFI_D_ERROR,
           "Disabling Qualcomm Watchdog Reboot timer failed! Status=%d\n",
           StubArgsSmc.Arg0));
    }
  }
}

VOID PlatformInitialize()
{
  UartInit();

  // Disable WatchDog Timer
  // SetWatchdogState(FALSE);

  // PlatformSchedulerInit();

  // Initialize GIC
  /*if (!FixedPcdGetBool(PcdIsLkBuild)) {
    if (EFI_ERROR(QGicPeim())) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }*/
}