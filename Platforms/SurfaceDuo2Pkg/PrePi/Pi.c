// Pi.c: Entry point for SEC(Security).

#include "Pi.h"

#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <PiDxe.h>
#include <PiPei.h>

#include <Guid/LzmaDecompress.h>
#include <Ppi/GuidedSectionExtraction.h>

#include <Library/ArmLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugAgentLib.h>
#include <Library/DebugLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/HobLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PeCoffGetEntryPointLib.h>
#include <Library/PerformanceLib.h>
#include <Library/PrePiHobListPointerLib.h>
#include <Library/PrePiLib.h>
#include <Library/SerialPortLib.h>

#include <IndustryStandard/ArmStdSmc.h>
#include <Library/ArmHvcLib.h>
#include <Library/ArmSmcLib.h>

#include <Library/PlatformHobLib.h>
#include <Configuration/DeviceMemoryMap.h>
#include <Protocol/EFIKernelInterface.h>

#define TLMM_ADDR 0x0F100000

#define TLMM_ADDR_OFFSET_FOR_PIN(x) (0x1000 * x)

#define TLMM_PIN_CONTROL_REGISTER 0
#define TLMM_PIN_IO_REGISTER 4
#define TLMM_PIN_INTERRUPT_CONFIG_REGISTER 8
#define TLMM_PIN_INTERRUPT_STATUS_REGISTER 0xC
#define TLMM_PIN_INTERRUPT_TARGET_REGISTER TLMM_PIN_INTERRUPT_CONFIG_REGISTER

#define LID0_GPIO38_STATUS_ADDR (TLMM_ADDR + TLMM_ADDR_OFFSET_FOR_PIN(38) + TLMM_PIN_IO_REGISTER)

#define LINUX_KERNEL_ARCH_MAGIC_OFFSET 0x38
#define LINUX_KERNEL_AARCH64_MAGIC 0x644D5241

typedef VOID (*LITTLE_KERNEL_CALLBACK) (UINT64 Argument);

typedef VOID (*LITTLE_KERNEL) (LITTLE_KERNEL_CALLBACK PrimaryEntry,
                               UINT64 Argument, 
                               UINT64 HeapBase,
                               UINT64 HeapSize,
                               UINT64 SecondaryEntry,
                               UINT32 MaxCoreCount,
                               UINT32 CoreCount);

extern VOID* _ModuleEntryPoint;

typedef VOID (*LINUX_KERNEL) (UINT64 ParametersBase,
                              UINT64 Reserved0,
                              UINT64 Reserved1,
                              UINT64 Reserved2);

VOID EFIAPI ProcessLibraryConstructorList(VOID);

PARM_MEMORY_REGION_DESCRIPTOR_EX PStoreMemoryRegion = NULL;

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

STATIC VOID UartInit(VOID)
{
  SerialPortInitialize();
  InitializeFb();
  ResetFb();

  DEBUG((EFI_D_INFO, "\nProjectMu on Duo 2 (AArch64)\n"));
  DEBUG(
      (EFI_D_INFO, "Firmware version %s built %a %a\n\n",
       (CHAR16 *)PcdGetPtr(PcdFirmwareVersionString), __TIME__, __DATE__));
}

BOOLEAN IsLinuxAvailable(IN VOID *KernelLoadAddress)
{
  VOID *LinuxKernelAddr = KernelLoadAddress + PcdGet32(PcdFdSize);
  UINT32 *LinuxKernelMagic = (UINT32*)(LinuxKernelAddr + LINUX_KERNEL_ARCH_MAGIC_OFFSET);
  return *LinuxKernelMagic == LINUX_KERNEL_AARCH64_MAGIC;
}

VOID BootLinux(IN VOID *KernelLoadAddress, IN VOID *DeviceTreeLoadAddress)
{
  VOID *LinuxKernelAddr = KernelLoadAddress + PcdGet32(PcdFdSize);
  LINUX_KERNEL LinuxKernel = (LINUX_KERNEL) LinuxKernelAddr;

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "Kernel Load Address = 0x%llx, Device Tree Load Address = 0x%llx\n",
       LinuxKernelAddr, DeviceTreeLoadAddress));

  // Jump to linux
  LinuxKernel ((UINT64)DeviceTreeLoadAddress, 0, 0, 0);

  // We should never reach here
  CpuDeadLoop();
}

VOID DisableWatchDogTimer()
{
  ARM_SMC_ARGS         StubArgsSmc;
  StubArgsSmc.Arg0 = 0x86000005;
  StubArgsSmc.Arg1 = 2;
  ArmCallSmc(&StubArgsSmc);
  if (StubArgsSmc.Arg0 != 0) {
    DEBUG((EFI_D_ERROR, "Disabling Qualcomm Watchdog Reboot timer failed! Status=%d\n", StubArgsSmc.Arg0));
  }
}

VOID ContinueBoot(IN UINTN StackSize)
{

  EFI_HOB_HANDOFF_INFO_TABLE *HobList;
  EFI_STATUS                  Status;

  VOID* StackBase = NULL;
  UINTN MemoryBase     = 0;
  UINTN MemorySize     = 0;
  UINTN UefiMemoryBase = 0;
  UINTN UefiMemorySize = 0;

  // Architecture-specific initialization
  // Enable Floating Point
  ArmEnableVFP();

  /* Enable program flow prediction, if supported */
  ArmEnableBranchPrediction();

  // Initialize (fake) UART.
  UartInit();

  // Declare UEFI region
  MemoryBase     = FixedPcdGet32(PcdSystemMemoryBase);
  MemorySize     = FixedPcdGet32(PcdSystemMemorySize);
  UefiMemoryBase = MemoryBase + FixedPcdGet32(PcdPreAllocatedMemorySize);
  UefiMemorySize = FixedPcdGet32(PcdUefiMemPoolSize);
  StackBase      = (VOID *)(UefiMemoryBase + UefiMemorySize - StackSize);

  DEBUG(
      (EFI_D_INFO | EFI_D_LOAD,
       "UEFI Memory Base = 0x%llx, Size = 0x%llx, Stack Base = 0x%llx, Stack "
       "Size = 0x%llx\n",
       UefiMemoryBase, UefiMemorySize, StackBase, StackSize));

  // Set up HOB
  HobList = HobConstructor(
      (VOID *)UefiMemoryBase, UefiMemorySize, (VOID *)UefiMemoryBase,
      StackBase);

  PrePeiSetHobList (HobList);

  // Invalidate cache
  InvalidateDataCacheRange(
      (VOID *)(UINTN)PcdGet64(PcdFdBaseAddress), PcdGet32(PcdFdSize));

  // Initialize MMU
  Status = MemoryPeim(UefiMemoryBase, UefiMemorySize);

  if (EFI_ERROR(Status)) {
    DEBUG((EFI_D_ERROR, "Failed to configure MMU\n"));
    CpuDeadLoop();
  }

  DEBUG((EFI_D_LOAD | EFI_D_INFO, "MMU configured from device config\n"));

  // Add HOBs
  BuildStackHob ((UINTN)StackBase, StackSize);

  // TODO: Call CpuPei as a library
  BuildCpuHob (ArmGetPhysicalAddressBits (), PcdGet8 (PcdPrePiCpuIoSize));

  // Set the Boot Mode
  SetBootMode (BOOT_WITH_DEFAULT_SETTINGS);

  // Initialize Platform HOBs (CpuHob and FvHob)
  Status = PlatformPeim();
  ASSERT_EFI_ERROR(Status);

  // Install SoC driver HOBs
  InstallPlatformHob();

  // Now, the HOB List has been initialized, we can register performance information
  // PERF_START (NULL, "PEI", NULL, StartTimeStamp);

  // SEC phase needs to run library constructors by hand.
  ProcessLibraryConstructorList();

  // Assume the FV that contains the PI (our code) also contains a
  // compressed FV.
  Status = DecompressFirstFv();
  ASSERT_EFI_ERROR(Status);

  // Load the DXE Core and transfer control to it
  Status = LoadDxeCoreFromFv(NULL, 0);
  ASSERT_EFI_ERROR(Status);

  // We should never reach here
  CpuDeadLoop();
}

VOID LKTrampoline(IN UINTN StackSize)
{
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "About to jump...!\n"));

  EFI_KERNEL_PROTOCOL* pSchedIntf = (EFI_KERNEL_PROTOCOL*)0x9FC37980;

  UINT32 SchedulingLibVersion = pSchedIntf->GetLibVersion();
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Scheduling Library Version %d.%d\n", (SchedulingLibVersion >> 16) & 0xFFFF, SchedulingLibVersion & 0xFFFF));

  //LITTLE_KERNEL LittleKernel = (LITTLE_KERNEL) 0x9FC05548;
  //LittleKernel(ContinueBoot, StackSize, 0x9FB00000, 0x00400000, (UINT64)&_ModuleEntryPoint, 8, 2);

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Jump failed!\n"));

  // In case of failure, jump to ContinueBoot anyway.
  ContinueBoot(StackSize);
}

VOID Main(IN VOID *StackBase, IN UINTN StackSize, IN VOID *KernelLoadAddress, IN VOID *DeviceTreeLoadAddress)
{
  EFI_STATUS                  Status;

  UINT32 Lid0Status    = 0;

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  SerialPortLocateArea(&PStoreMemoryRegion);

  // Clear PStore area
  UINT8 *base = (UINT8 *)PStoreMemoryRegion->Address;
  for (UINTN i = 0; i < PStoreMemoryRegion->Length; i++) {
    base[i] = 0;
  }
#endif

  // Initialize (fake) UART.
  UartInit();

  // Which EL?
  if (ArmReadCurrentEL() == AARCH64_EL2) {
    DEBUG((EFI_D_ERROR, "Running at EL2 \n"));
  }
  else {
    DEBUG((EFI_D_ERROR, "Running at EL1 \n"));
  }

  if (IsLinuxAvailable(KernelLoadAddress)) {
    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD,
        "Kernel Load Address = 0x%llx, Device Tree Load Address = 0x%llx\n",
        KernelLoadAddress, DeviceTreeLoadAddress));

    Lid0Status = MmioRead32(LID0_GPIO38_STATUS_ADDR) & 1;

    DEBUG((EFI_D_INFO | EFI_D_LOAD, "Lid Status = 0x%llx\n", Lid0Status));

    if (Lid0Status == 0) {
      BootLinux(KernelLoadAddress, DeviceTreeLoadAddress);

      // We should never reach here
      CpuDeadLoop();
    }
  }

  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Disabling Qualcomm Watchdog Reboot timer\n"));
  DisableWatchDogTimer();
  DEBUG((EFI_D_INFO | EFI_D_LOAD, "Qualcomm Watchdog Reboot timer disabled\n"));

  // Initialize GIC
  if (!FixedPcdGetBool(PcdIsLkBuild)) {
    Status = QGicPeim();

    if (EFI_ERROR(Status)) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }

  // Immediately launch all CPUs, 7 CPUs hold
  DEBUG((EFI_D_LOAD | EFI_D_INFO, "Launching CPUs\n"));

  // Launch all CPUs, hold and jump
  LKTrampoline(StackSize);

  // We should never reach here
  CpuDeadLoop();
}

VOID CEntryPoint(IN VOID *StackBase, IN UINTN StackSize, IN VOID *KernelLoadAddress, IN VOID *DeviceTreeLoadAddress)
{
  Main(StackBase, StackSize, KernelLoadAddress, DeviceTreeLoadAddress);
}

VOID SecondaryCEntryPoint(IN UINT64 Argument)
{
  LITTLE_KERNEL_CALLBACK CallBack = (LITTLE_KERNEL_CALLBACK)0x9FC05298;
  CallBack(Argument);
}