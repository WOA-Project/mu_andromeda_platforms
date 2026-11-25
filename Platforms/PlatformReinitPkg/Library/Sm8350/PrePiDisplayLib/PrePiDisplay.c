/** @file
  SM8350 inherited single-framebuffer display.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformHwioLib.h>
#include <Library/PrePiDisplayLib.h>

STATIC UINT32   *mFramebuffer;
STATIC UINTN    mWidth;
STATIC UINTN    mHeight;
STATIC UINTN    mFramebufferSize;

EFI_STATUS
PlatformStartDisplay (
  VOID
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX  Region;
  EFI_STATUS                       Status;
  UINTN                            Width;
  UINTN                            Height;

  if (mFramebuffer != NULL) {
    return EFI_ALREADY_STARTED;
  }

  Status = GetPlatformDisplayFrameBuffer (&Region);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Width = FixedPcdGet32 (PcdMipiFrameBufferWidth);
  Height = FixedPcdGet32 (PcdMipiFrameBufferHeight);
  if ((Region.Address == 0) || (Region.Address > MAX_UINTN) ||
      ((Region.Address & (sizeof (UINT32) - 1)) != 0) ||
      (Width == 0) || (Height == 0) ||
      (FixedPcdGet32 (PcdMipiFrameBufferPixelBpp) != 32) ||
      (Width > MAX_UINTN / Height / sizeof (UINT32)) ||
      (Region.Length < Width * Height * sizeof (UINT32)) ||
      (Region.Address > MAX_UINTN - Width * Height * sizeof (UINT32))) {
    return EFI_UNSUPPORTED;
  }

  mFramebuffer = (UINT32 *)(UINTN)Region.Address;
  mWidth = Width;
  mHeight = Height;
  mFramebufferSize = Width * Height * sizeof (UINT32);
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformDisplayTestImage (
  VOID
  )
{
  STATIC CONST UINT32 Colors[] = {
    0x00FFFFFF, 0x00FFFF00, 0x0000FFFF, 0x0000FF00,
    0x00FF00FF, 0x00FF0000, 0x000000FF, 0x00000000
  };
  UINTN X;
  UINTN Y;

  if (mFramebuffer == NULL) {
    return EFI_NOT_STARTED;
  }

  for (Y = 0; Y < mHeight; Y++) {
    for (X = 0; X < mWidth; X++) {
      mFramebuffer[Y * mWidth + X] = Colors[(X * ARRAY_SIZE (Colors)) / mWidth];
    }
  }

  WriteBackDataCacheRange (mFramebuffer, mFramebufferSize);
  MemoryFence ();
  return EFI_SUCCESS;
}

EFI_STATUS
PlatformStopDisplay (
  VOID
  )
{
  if (mFramebuffer == NULL) {
    return EFI_NOT_STARTED;
  }

  mFramebuffer = NULL;
  return EFI_SUCCESS;
}
