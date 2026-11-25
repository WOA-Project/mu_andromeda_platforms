/** @file
  Polling PrePi SPMI reads.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_SPMI_LIB_H_
#define PRE_PI_SPMI_LIB_H_

#include <Uefi.h>
EFI_STATUS EFIAPI PrePiSpmiInitialize (VOID);
EFI_STATUS EFIAPI PrePiSpmiFindChannel (IN UINT32 Peripheral, OUT UINTN *Channel);
EFI_STATUS EFIAPI PrePiSpmiRead (IN UINTN Channel, IN UINT8 Register, OUT UINT8 *Value);

#endif
