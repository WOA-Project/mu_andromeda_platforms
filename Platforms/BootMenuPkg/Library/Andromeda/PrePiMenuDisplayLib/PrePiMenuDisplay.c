/** @file
  Andromeda PrePi framebuffer access.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformHwioLib.h>
#include <Library/PrePiMenuDisplayLib.h>

EFI_STATUS
EFIAPI
PrePiMenuDisplayGetSurface (
  OUT PRE_PI_MENU_SURFACE  *Surface
  )
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX  Region;
  EFI_STATUS                       Status;
  UINTN                            Width;
  UINTN                            Height;

  if (Surface == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Width = FixedPcdGet32 (PcdMipiFrameBufferWidth);
  Height = FixedPcdGet32 (PcdMipiFrameBufferHeight);
  Status = GetPlatformDisplayFrameBuffer (&Region);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Width < 480) || (Height < 300) ||
      (FixedPcdGet32 (PcdMipiFrameBufferPixelBpp) != 32) ||
      (Width > MAX_UINTN / Height / sizeof (UINT32)) ||
      (Region.Length < Width * Height * sizeof (UINT32))) {
    return EFI_UNSUPPORTED;
  }

  Surface->Pixels = (UINT32 *)(UINTN)Region.Address;
  Surface->Width = Width;
  Surface->Height = Height;
  Surface->Stride = Width;
  return EFI_SUCCESS;
}

VOID
EFIAPI
PrePiMenuDisplayFlush (
  IN CONST PRE_PI_MENU_SURFACE  *Surface
  )
{
  WriteBackDataCacheRange (Surface->Pixels, Surface->Stride * Surface->Height * sizeof (UINT32));
  MemoryFence ();
}

CONST CHAR8 *
EFIAPI
PrePiMenuDisplayGetHeading (
  VOID
  )
{
  return "Surface Duo";
}
