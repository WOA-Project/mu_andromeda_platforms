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
  for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    UINT32 GICRAddr = 0x17A60000 + CpuId * 0x20000;
    
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "CpuId: %d\n", CpuId));

    UINT32 off1 = MmioRead32(GICRAddr + 0x10280);
    UINT32 off2 = MmioRead32(GICRAddr + 0x10180);
    UINT32 off3 = MmioRead32(GICRAddr + 0x0014);

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 1: %d\n", off1));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 2: %d\n", off2));
    DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicR off 3: %d\n", off3));
  }

  UINT32 GICDAddr = 0x17A00000;
  UINT32 off4 = MmioRead32(GICDAddr);
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GicD off 4: %d\n", off4));
}

VOID GICv3SetRegisters()
{
  UINT32 GICD_BASE = 0x17A00000;

  /*for (UINT32 CpuId = 0; CpuId < 8; CpuId++) {
    UINT32 GICRAddr = 0x17A60000 + CpuId * 0x20000;

    MmioWrite32(GICRAddr + 0x10280, 0x10000000);
    MmioWrite32(GICRAddr + 0x10180, 0);
    MmioWrite32(GICRAddr + 0x00014, 0);
  }

  UINT32 GICDAddr = 0x17A00000;
  MmioWrite32(GICDAddr, 0x10);*/

  UINT32 GICD_TYPER = MmioRead32(GICD_BASE + 0x04);

  UINT32 Spis = ((GICD_TYPER & 0x1F) == 0x1F) ? 1020 : 32 * ((GICD_TYPER & 0x1F) + 1);

  GICD_TYPER = (GICD_TYPER & 0x100) ? GICD_TYPER >> 0x1B : 0;
  UINT32 ESpis = ((GICD_TYPER & 0x1F) == 0x1F) ? 1020 : 32 * ((GICD_TYPER & 0x1F) + 1);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "GIC: Spis=%d, ESpis=%d\n", Spis, ESpis));

  //UINT32 GICD_CTRL = MmioRead32(GICD_BASE);
  //GICD_CTRL &= ~0x12;
  MmioWrite32(GICD_BASE, 0);

  while (MmioRead32(GICD_BASE) & 0x80000000) 
    ;

  for (UINT32 i = 32; i < Spis; i += 32) {
    MmioWrite32(GICD_BASE + 0x0080 + 4 * (i / 32), 0);
  }

  for (UINT32 i = 0; i < ESpis; i += 32) {
    MmioWrite32(GICD_BASE + 0x1400 + 4 * (i / 32), 0xFFFFFFFF);
    MmioWrite32(GICD_BASE + 0x1C00 + 4 * (i / 32), 0xFFFFFFFF);
  }

  for (UINT32 i = 0; i < ESpis; i += 16) {
    MmioWrite32(GICD_BASE + 0x1000 + 4 * (i / 32), 0xFFFFFFFF);
  }

  for (UINT32 i = 0; i < ESpis; i += 16) {
    MmioWrite32(GICD_BASE + 0x3000 + 4 * (i / 16), 0);
  }

  for (UINT32 i = 0; i < ESpis; i += 4) {
    MmioWrite32(GICD_BASE + 0x2000 + 4 * (i / 16), 0xA0A0A0A0);
  }



  for (UINT32 i = 32; i < Spis; i += 16) {
    MmioWrite32(GICD_BASE + 0x0C00 + 4 * (i / 16), 0);
  }

  for (UINT32 i = 32; i < Spis; i += 4) {
    MmioWrite32(GICD_BASE + 0x0400 + 4 * (i / 4), 0xA0A0A0A0);
  }

  for (UINT32 i = 32; i < Spis; i += 32) {
    MmioWrite32(GICD_BASE + 0x0380 + 4 * (i / 32), 0xFFFFFFFF);
    MmioWrite32(GICD_BASE + 0x0180 + 4 * (i / 32), 0xFFFFFFFF);
  }

  while (MmioRead32(GICD_BASE) & 0x80000000) 
    ;
}

VOID PlatformInitialize()
{
  UartInit();

  GICv3DumpRegisters();
  GICv3SetRegisters();
  GICv3DumpRegisters();

  // Hang here for debugging
  ASSERT(FALSE);

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