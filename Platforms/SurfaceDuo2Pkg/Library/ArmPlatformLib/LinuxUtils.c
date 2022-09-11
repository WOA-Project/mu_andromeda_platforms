#include "Pi.h"

typedef VOID (*LINUX_KERNEL) (UINT64 ParametersBase,
                              UINT64 Reserved0,
                              UINT64 Reserved1,
                              UINT64 Reserved2);

#define LINUX_KERNEL_ARCH_MAGIC_OFFSET 0x38
#define LINUX_KERNEL_AARCH64_MAGIC 0x644D5241

BOOLEAN IsLinuxAvailable(IN VOID *KernelLoadAddress)
{
  VOID *LinuxKernelAddr = KernelLoadAddress + PcdGet32(PcdFdSize);
  UINT32 *LinuxKernelMagic = (UINT32*)(LinuxKernelAddr + LINUX_KERNEL_ARCH_MAGIC_OFFSET);
  return *LinuxKernelMagic == LINUX_KERNEL_AARCH64_MAGIC;
}

VOID BootLinux(IN VOID *DeviceTreeLoadAddress, IN VOID *KernelLoadAddress)
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

VOID ContinueToLinuxIfAllowed(IN VOID *DeviceTreeLoadAddress, IN VOID *KernelLoadAddress)
{
  // Early Test..
  PlatformInitialize();
  
  if (IsLinuxAvailable(KernelLoadAddress)) {
    DEBUG(
        (EFI_D_INFO | EFI_D_LOAD,
        "Kernel Load Address = 0x%llx, Device Tree Load Address = 0x%llx\n",
        KernelLoadAddress, DeviceTreeLoadAddress));

    if (IsLinuxBootRequested()) {
      BootLinux(DeviceTreeLoadAddress, KernelLoadAddress);
    }
  }
}