/** @file
  Polling access to the Qualcomm APPS SPMI observer.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/PrePiSpmiLib.h>
#include <Library/TimerLib.h>

#define SPMI_CORE          0x0C440000U
#define SPMI_OBSERVER      0x0E600000U
#define SPMI_CONFIG        0x0C40A000U
#define SPMI_CHANNEL_COUNT 512U
#define SPMI_STRIDE        0x80U
#define SPMI_PPID_MASK     0x0FFFU
#define SPMI_STATUS_DONE   BIT0
#define SPMI_STATUS_ERROR  (BIT1 | BIT2 | BIT3)

EFI_STATUS
EFIAPI
PrePiSpmiInitialize (
  VOID
  )
{
  UINT32 Version;

  Version = MmioRead32 (SPMI_CORE);
  if ((Version >> 28) != 5U) {
    DEBUG ((DEBUG_ERROR, "PrePiSpmi: unsupported arbiter version 0x%x\n", Version));
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
PrePiSpmiFindChannel (
  IN UINT32 Peripheral,
  OUT UINTN *Channel
  )
{
  UINTN Index;
  UINT32 Mapping;
  UINTN Fallback;

  if ((Channel == NULL) || (Peripheral > SPMI_PPID_MASK)) {
    return EFI_INVALID_PARAMETER;
  }

  Fallback = MAX_UINTN;
  for (Index = 0; Index < SPMI_CHANNEL_COUNT; Index++) {
    Mapping = MmioRead32 (SPMI_CORE + 0x900U + 4U * Index);
    if ((Mapping == 0) || (((Mapping >> 8) & SPMI_PPID_MASK) != Peripheral)) {
      continue;
    }

#ifdef PREPI_SPMI_SM8350
    if ((MmioRead32 (SPMI_CONFIG + 0x700U + 4U * Index) & 0xFU) == 0) {
#else
    if ((MmioRead32 (SPMI_CONFIG + 0x700U + 4U * Index) & 7U) == 0) {
#endif
      *Channel = SPMI_OBSERVER + SPMI_STRIDE * Index;
      return EFI_SUCCESS;
    }

    if (Fallback == MAX_UINTN) {
      Fallback = Index;
    }
  }

  if (Fallback != MAX_UINTN) {
    *Channel = SPMI_OBSERVER + SPMI_STRIDE * Fallback;
    return EFI_SUCCESS;
  }
  DEBUG ((DEBUG_ERROR, "PrePiSpmi: no accessible channel for peripheral 0x%x\n", Peripheral));
  return EFI_NOT_FOUND;
}

EFI_STATUS
EFIAPI
PrePiSpmiRead (
  IN UINTN Channel,
  IN UINT8 Register,
  OUT UINT8 *Value
  )
{
  UINT32 Status;
  UINTN Attempt;

  if ((Value == NULL)
      || (Channel < SPMI_OBSERVER)
      || (Channel >= SPMI_OBSERVER + SPMI_STRIDE * SPMI_CHANNEL_COUNT)
      || (((Channel - SPMI_OBSERVER) % SPMI_STRIDE) != 0)
      ) {
    return EFI_INVALID_PARAMETER;
  }

  MmioWrite32 (Channel, BIT27 | ((UINT32)Register << 4));
  MemoryFence ();
  for (Attempt = 0; Attempt < 1000; Attempt++) {
    Status = MmioRead32 (Channel + 8);
    if ((Status & BIT3) != 0) {
      DEBUG ((DEBUG_ERROR, "PrePiSpmi: transaction dropped: 0x%x\n", Status));
      return EFI_DEVICE_ERROR;
    }

    if ((Status & SPMI_STATUS_DONE) != 0) {
      if ((Status & SPMI_STATUS_ERROR) != 0) {
        DEBUG ((DEBUG_ERROR, "PrePiSpmi: transaction status 0x%x\n", Status));
        return EFI_DEVICE_ERROR;
      }

      *Value = (UINT8)MmioRead32 (Channel + 0x18);
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  DEBUG ((DEBUG_ERROR, "PrePiSpmi: read timed out\n"));
  return EFI_TIMEOUT;
}
