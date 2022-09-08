#include "Pi.h"

#define TLMM_WEST 0x03100000
#define TLMM_EAST 0x03500000
#define TLMM_NORTH 0x03900000
#define TLMM_SOUTH 0x03D00000

#define TLMM_ADDR_OFFSET_FOR_PIN(x) (0x1000 * x)

#define TLMM_PIN_CONTROL_REGISTER 0
#define TLMM_PIN_IO_REGISTER 4
#define TLMM_PIN_INTERRUPT_CONFIG_REGISTER 8
#define TLMM_PIN_INTERRUPT_STATUS_REGISTER 0xC
#define TLMM_PIN_INTERRUPT_TARGET_REGISTER TLMM_PIN_INTERRUPT_CONFIG_REGISTER

#define LID0_GPIO121_STATUS_ADDR                                               \
  (TLMM_SOUTH + TLMM_ADDR_OFFSET_FOR_PIN(121) + TLMM_PIN_IO_REGISTER)

#define MDSS_DSI0 0xAE94000
#define MDSS_DSI1 0xAE96000
#define DSI_CTRL 4

BOOLEAN IsLinuxBootRequested()
{
  return (MmioRead32(LID0_GPIO121_STATUS_ADDR) & 1) == 1;
}

VOID PlatformInitialize()
{
  // Reset MDSS DSI0 Controller
  MmioWrite32(MDSS_DSI0 + DSI_CTRL, MmioRead32(MDSS_DSI0 + DSI_CTRL) & ~0x7);

  // Reset MDSS DSI1 Controller
  MmioWrite32(MDSS_DSI1 + DSI_CTRL, MmioRead32(MDSS_DSI1 + DSI_CTRL) & ~0x7);

  // Windows requires Cache Coherency for the UFS to work at its best
  // The UFS device is currently attached to the main IOMMU on Context Bank 1
  // (Previous firmware) But said configuration is non cache coherent compliant,
  // fix it.
  MmioWrite32(0x15081000, 0x9F00E0);
  MmioWrite32(0x15081020, 0x0);
  MmioWrite32(0x15081024, 0x0);
  MmioWrite32(0x15081028, 0x0);
  MmioWrite32(0x1508102C, 0x0);
  MmioWrite32(0x15081038, 0x0);
  MmioWrite32(0x1508103C, 0x0);
  MmioWrite32(0x15081030, 0x0);

  // Disable WatchDog Timer
  MmioWrite32(0x17C10008, 0x00000000);

  // Initialize GIC
  if (!FixedPcdGetBool(PcdIsLkBuild)) {
    if (EFI_ERROR(QGicPeim())) {
      DEBUG((EFI_D_ERROR, "Failed to configure GIC\n"));
      CpuDeadLoop();
    }
  }
}