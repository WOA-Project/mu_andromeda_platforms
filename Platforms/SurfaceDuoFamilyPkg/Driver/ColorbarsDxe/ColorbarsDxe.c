#include <Guid/EventGroup.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/DevicePathLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrintLib.h>
#include <Library/SerialPortLib.h>
#include <Library/TimerLib.h>
#include <Library/UefiBootManagerLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/Cpu.h>
#include <Protocol/DevicePathFromText.h>
#include <Protocol/EmbeddedGpio.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/PlatformBootManager.h>

#include <Library/DeviceStateLib.h>

EFI_STATUS
EFIAPI
InitializeColorbars(IN EFI_HANDLE ImageHandle, IN EFI_SYSTEM_TABLE *SystemTable)
{
  DEVICE_STATE State;

  State = 0;

#if SECURE_BOOT_ENABLE == 0
  State |= DEVICE_STATE_SECUREBOOT_OFF;
#endif

  State |= DEVICE_STATE_DEVELOPMENT_BUILD_ENABLED;

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1 || USE_SCREEN_FOR_SERIAL_OUTPUT == 1
  State |= DEVICE_STATE_UNIT_TEST_MODE;
#endif

  AddDeviceState(State);

  return EFI_SUCCESS;
}