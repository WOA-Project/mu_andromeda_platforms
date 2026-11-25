/** @file
  PrePi boot menu with a read-only UFS file browser.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrePiBootMenuLib.h>
#include <Library/PrePiMenuDisplayLib.h>
#include <Library/PrePiMenuLib.h>
#include <Library/PrePiUfsBrowserLib.h>
#include <Library/PrintLib.h>

STATIC CHAR16 mPath[32U * 256U + 257U];
STATIC VOID   *mSavedFramebuffer;
STATIC UINTN  mSavedFramebufferSize;

STATIC
EFI_STATUS
RunBootMenu (
  VOID
  )
{
  CONST CHAR8 *Entries[2];
  CONST CHAR8 *Back[1];
  CHAR8 Title[256];
  UINTN Selection;
  UINTN PathChars;
  UINTN Index;
  UINTN Position;
  UINT8 Lun;
  UINT32 Partition;
  EFI_STATUS Status;

  Entries[0] = "Continue boot";
  Entries[1] = "File browser";
  for (;;) {
    Selection = 0;
    Status = PrePiMenuShow ("PrePi boot menu", Entries, 2, &Selection);
    if (EFI_ERROR (Status) || (Selection == 0)) {
      return Status;
    }

    PathChars = sizeof (mPath) / sizeof (mPath[0]);
    Status = PrePiUfsBrowse (&Lun, &Partition, &PathChars, mPath);
    if (Status == EFI_ABORTED) {
      continue;
    }

    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "PrePi: UFS browser failed: %r\n", Status));
      AsciiSPrint (Title, sizeof (Title), "UFS browser failed: %r", Status);
      Back[0] = "Back to boot menu";
      Selection = 0;
      Status = PrePiMenuShow (Title, Back, 1, &Selection);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      continue;
    }

    DEBUG ((DEBUG_INFO, "PrePi: selected LUN %u partition %u: %s\n", Lun, Partition, mPath));
    AsciiSPrint (Title, sizeof (Title), "Selected (not executed): ");
    Position = AsciiStrLen (Title);
    for (Index = 0; (mPath[Index] != 0) && (Position + 1 < sizeof (Title)); Index++) {
      Title[Position++] = ((mPath[Index] >= 32) && (mPath[Index] <= 126)) ?
                          (CHAR8)mPath[Index] : '?';
    }

    Title[Position] = 0;
    Back[0] = "Back to boot menu";
    Selection = 0;
    Status = PrePiMenuShow (Title, Back, 1, &Selection);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }
}

EFI_STATUS
EFIAPI
PrePiBootMenuShow (
  VOID
  )
{
  PRE_PI_MENU_SURFACE Surface;
  EFI_STATUS          Status;
  UINTN               Size;

  Status = PrePiMenuDisplayGetSurface (&Surface);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Surface.Pixels == NULL) || (Surface.Height == 0) ||
      (Surface.Stride < Surface.Width) ||
      (Surface.Stride > MAX_UINTN / Surface.Height / sizeof (UINT32))) {
    return EFI_UNSUPPORTED;
  }

  Size = Surface.Stride * Surface.Height * sizeof (UINT32);
  if ((Size == 0) || (Size > MAX_UINTN - EFI_PAGE_MASK) ||
      ((UINTN)Surface.Pixels > MAX_UINTN - Size)) {
    return EFI_UNSUPPORTED;
  }

  if (mSavedFramebuffer == NULL) {
    mSavedFramebuffer = AllocatePages (EFI_SIZE_TO_PAGES (Size));
    if (mSavedFramebuffer == NULL) {
      DEBUG ((DEBUG_ERROR, "PrePi: cannot preserve the menu framebuffer\n"));
      return EFI_OUT_OF_RESOURCES;
    }

    mSavedFramebufferSize = Size;
  } else if (Size > mSavedFramebufferSize) {
    DEBUG ((DEBUG_ERROR, "PrePi: menu framebuffer grew beyond its saved buffer\n"));
    return EFI_BAD_BUFFER_SIZE;
  }

  CopyMem (mSavedFramebuffer, Surface.Pixels, Size);
  Status = RunBootMenu ();
  CopyMem (Surface.Pixels, mSavedFramebuffer, Size);
  PrePiMenuDisplayFlush (&Surface);
  return Status;
}
