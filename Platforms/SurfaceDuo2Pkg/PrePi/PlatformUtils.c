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

VOID GICv3DumpRegisters()
{
  UINT32 GICD_BASE = 0x17A00000;
  UINT32 GICR_BASE = 0x17A60000;

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC register status\n"));

  for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    UINT32 GICRAddr = GICR_BASE + CpuId * 0x20000;
    
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "CPU@%d\n", CpuId));

    UINT32 off1 = MmioRead32(GICRAddr + 0x10280);
    UINT32 off2 = MmioRead32(GICRAddr + 0x10180);
    UINT32 off3 = MmioRead32(GICRAddr + 0x0014);

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR + 0x10280: %d\n", off1));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR + 0x10180: %d\n", off2));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR + 0x00014: %d\n\n", off3));
  }

  UINT32 off4 = MmioRead32(GICD_BASE);
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICD: %d\n\n", off4));
}

#define GICD_TYPER_SPIS(GICD_TYPER) ((((GICD_TYPER) & 0x1F) + 1) * 32)

VOID GICv3SetupDistributor()
{
  UINT32 mGicDistributorBase = 0x17A00000;
  UINT32 GICD_TYPER = MmioRead32(mGicDistributorBase + 0x04);
  UINTN  Index;

  UINT32 mGicNumInterrupts = GICD_TYPER_SPIS(GICD_TYPER);
  if (mGicNumInterrupts > 1020) {
    mGicNumInterrupts = 1020;
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: SPIs=%d\n", mGicNumInterrupts));

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Disabling GIC Distributor\n"));
  MmioWrite32(mGicDistributorBase, 0);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Waiting for Sync\n"));
  while (MmioRead32(mGicDistributorBase) & 0x80000000) 
    ;

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Configure SPIs as NS Grp 1\n"));
  for (Index = 32; Index < mGicNumInterrupts; Index += 32) {
    MmioWrite32(mGicDistributorBase + 0x0080 + 4 * (Index / 32), 0);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Set all IRQs to be active low, level triggered\n"));
  for (Index = 32; Index < mGicNumInterrupts; Index += 16) {
    MmioWrite32(mGicDistributorBase + 0x0C00 + 4 * (Index / 16), 0);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Set all IRQs Priority Register\n"));
  for (Index = 32; Index < mGicNumInterrupts; Index += 4) {
    MmioWrite32(mGicDistributorBase + 0x0400 + Index, 0xa0a0a0a0);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Set all IRQs disable state\n"));
  for (Index = 32; Index < mGicNumInterrupts; Index += 32) {
    MmioWrite32(mGicDistributorBase + 0x0380 + 4 * (Index / 32), 0xffffffff);
    MmioWrite32(mGicDistributorBase + 0x0180 + 4 * (Index / 32), 0xffffffff);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Enabling GIC Distributor\n"));
  MmioWrite32(mGicDistributorBase, 0x13);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Waiting for Sync\n"));
  while (MmioRead32(mGicDistributorBase) & 0x80000000) 
    ;

	UINT64 affinity = ArmReadMpidr();
  affinity &= 0xFF00FFFFFF;
	for (Index = 32; Index < mGicNumInterrupts; Index++) {
		MmioWrite64(mGicDistributorBase + 0x6000 + Index * 8, affinity);
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Setting registers is done!\n"));
}

VOID GICv3SetupReDistributor()
{
  UINT32 mGicRedistributorsBase = 0x17A60000;
  UINTN  Index;

  UINT32 GICR_WAKER = MmioRead32(mGicRedistributorsBase + 0x014);
  GICR_WAKER &= ~0x2;
  MmioWrite32(mGicRedistributorsBase + 0x014, GICR_WAKER);

  for (Index = 0; Index < 1000000; Index++) {
    GICR_WAKER = MmioRead32(mGicRedistributorsBase + 0x014);
    if ((GICR_WAKER & 0x4) == 0) {
      break;
    }
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR: Looping til CPU 0 wakes up...\n"));
  }

  if (Index == 1000000) {
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR: CPU 0 failed to wake up!\n"));
  } else {
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR: CPU 0 woke up!\n"));
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GICR: Setting registers is done!\n"));
}

VOID GICv3SetRegisters()
{
  UINT32 GICD_BASE = 0x17A00000;
  UINT32 GICR_BASE = 0x17A60000;

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Setting registers\n"));

  for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    UINT32 GICRAddr = GICR_BASE + CpuId * 0x20000;

    MmioWrite32(GICRAddr + 0x10280, 0x10000000);
    MmioWrite32(GICRAddr + 0x10180, 0);
    MmioWrite32(GICRAddr + 0x00014, 0);
  }

  MmioWrite32(GICD_BASE, 0x10);
}

VOID PlatformInitialize()
{
  UartInit();

  GICv3DumpRegisters();
  // GICv3SetupDistributor();
  // GICv3SetupReDistributor();
  GICv3SetRegisters();
  GICv3DumpRegisters();

  // Hang here for debugging
  // ASSERT(FALSE);

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