/** @file
  Polling UFS partition and FAT file browser.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/PrePiMenuLib.h>
#include <Library/PrePiUfsBrowserLib.h>
#include <Library/PrePiUfsReadLib.h>
#include <Library/PrintLib.h>
#include "BrowserDisk.h"

#define PAGE_ENTRIES 24U
#define MAX_DEPTH    32U
#define MAX_LUNS     8U

STATIC PRE_PI_UFS_LUN  mLuns[MAX_LUNS];
STATIC BROWSER_FILE   mFiles[PAGE_ENTRIES];
STATIC BROWSER_FILE   mParents[MAX_DEPTH];
STATIC BROWSER_PARTITION mPartitions[PAGE_ENTRIES];
STATIC CHAR8          mLabels[PAGE_ENTRIES + 3][280];
STATIC CONST CHAR8    *mChoices[PAGE_ENTRIES + 3];

STATIC
EFI_STATUS
ShowMessage (
  IN CONST CHAR8 *Message
  )
{
  CONST CHAR8 *Choice[1];
  UINTN Selection;

  Choice[0] = "Back";
  Selection = 0;
  return PrePiMenuShow (Message, Choice, 1, &Selection);
}

STATIC
VOID
DisplayName (
  OUT CHAR8 *Label, IN UINTN Capacity, IN CONST CHAR16 *Name, IN BOOLEAN Directory
  )
{
  UINTN Position;
  UINTN Index;

  Position = 0;
  if (Directory) {
    AsciiSPrint (Label, Capacity, "[DIR] ");
    Position = AsciiStrLen (Label);
  }

  for (Index = 0; (Name[Index] != 0) && (Position + 1 < Capacity); Index++) {
    Label[Position++] = ((Name[Index] >= 32) && (Name[Index] <= 126)) ?
                        (CHAR8)Name[Index] : '?';
  }

  Label[Position] = 0;
}

STATIC
EFI_STATUS
SelectLun (
  IN UINTN Count, OUT UINTN *LunIndex
  )
{
  UINTN Index;
  UINTN Selection;
  EFI_STATUS Status;

  mChoices[0] = "Back";
  for (Index = 0; Index < Count; Index++) {
    AsciiSPrint (mLabels[Index + 1], sizeof (mLabels[0]), "LUN %u (%u-byte blocks)",
                 mLuns[Index].Lun, mLuns[Index].BlockSize);
    mChoices[Index + 1] = mLabels[Index + 1];
  }

  Selection = 0;
  Status = PrePiMenuShow ("Select UFS LUN", mChoices, Count + 1, &Selection);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Selection == 0) {
    return EFI_ABORTED;
  }

  *LunIndex = Selection - 1;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SelectPartition (
  IN BROWSER_DISK *Disk, OUT BROWSER_PARTITION *Partition
  )
{
  EFI_STATUS Status;
  UINT32 Index;
  UINTN Page;
  UINTN Seen;
  UINTN Count;
  UINTN ChoiceCount;
  UINTN Selection;
  UINTN First;
  BOOLEAN More;
  BROWSER_PARTITION Item;

  Page = 0;
  for (;;) {
    Seen = 0;
    Count = 0;
    More = FALSE;
    for (Index = 0; Index < Disk->EntryCount; Index++) {
      Status = BrowserPartitionGet (Disk, Index, &Item);
      if (Status == EFI_NOT_FOUND) {
        continue;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Seen++ < Page * PAGE_ENTRIES) {
        continue;
      }

      if (Count == PAGE_ENTRIES) {
        More = TRUE;
        break;
      }

      mPartitions[Count++] = Item;
    }

    ChoiceCount = 0;
    mChoices[ChoiceCount++] = "Back";
    if (Page != 0) {
      mChoices[ChoiceCount++] = "Previous page";
    }

    First = ChoiceCount;
    for (Index = 0; Index < Count; Index++) {
      mChoices[ChoiceCount++] = mPartitions[Index].Name;
    }

    if (More) {
      mChoices[ChoiceCount++] = "Next page";
    }

    Selection = 0;
    Status = PrePiMenuShow ("Select partition", mChoices, ChoiceCount, &Selection);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (Selection == 0) {
      return EFI_ABORTED;
    }

    if ((Page != 0) && (Selection == 1)) {
      Page--;
    } else if (Selection < First + Count) {
      *Partition = mPartitions[Selection - First];
      return EFI_SUCCESS;
    } else {
      Page++;
    }
  }
}

STATIC
EFI_STATUS
SelectFile (
  IN BROWSER_FAT *Fat, IN OUT UINTN *PathChars, OUT CHAR16 *Path
  )
{
  EFI_STATUS Status;
  BROWSER_DIRECTORY Directory;
  BROWSER_FILE Item;
  UINTN Depth;
  UINTN Page;
  UINTN Seen;
  UINTN Count;
  UINTN ChoiceCount;
  UINTN Selection;
  UINTN First;
  UINTN Index;
  UINTN Need;
  UINTN Position;
  BOOLEAN More;

  Depth = 0;
  Page = 0;
  for (;;) {
    BrowserDirectoryOpen (Fat, (Depth == 0) ? 0 : mParents[Depth - 1].Cluster, &Directory);
    Seen = 0;
    Count = 0;
    More = FALSE;
    for (;;) {
      Status = BrowserDirectoryNext (&Directory, &Item);
      if (Status == EFI_NOT_FOUND) {
        break;
      }

      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (Seen++ < Page * PAGE_ENTRIES) {
        continue;
      }

      if (Count == PAGE_ENTRIES) {
        More = TRUE;
        break;
      }

      mFiles[Count++] = Item;
    }

    ChoiceCount = 0;
    mChoices[ChoiceCount++] = (Depth == 0) ? "Back to partitions" : "Parent directory";
    if (Page != 0) {
      mChoices[ChoiceCount++] = "Previous page";
    }

    First = ChoiceCount;
    for (Index = 0; Index < Count; Index++) {
      DisplayName (mLabels[ChoiceCount], sizeof (mLabels[0]),
                   mFiles[Index].Name, mFiles[Index].Directory);
      mChoices[ChoiceCount] = mLabels[ChoiceCount];
      ChoiceCount++;
    }

    if (More) {
      mChoices[ChoiceCount++] = "Next page";
    }

    Selection = 0;
    Status = PrePiMenuShow ("Browse FAT files", mChoices, ChoiceCount, &Selection);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (Selection == 0) {
      if (Depth == 0) {
        return EFI_ABORTED;
      }

      Depth--;
      Page = 0;
    } else if ((Page != 0) && (Selection == 1)) {
      Page--;
    } else if (Selection >= First + Count) {
      Page++;
    } else {
      Item = mFiles[Selection - First];
      if (Item.Directory) {
        if (Depth == MAX_DEPTH) {
          Status = ShowMessage ("Maximum directory depth reached");
          if (EFI_ERROR (Status)) {
            return Status;
          }
        } else {
          mParents[Depth++] = Item;
          Page = 0;
        }

        continue;
      }

      Need = 2 + StrLen (Item.Name);
      for (Index = 0; Index < Depth; Index++) {
        Need += StrLen (mParents[Index].Name) + 1;
      }

      if (*PathChars < Need) {
        *PathChars = Need;
        return EFI_BUFFER_TOO_SMALL;
      }

      Position = 0;
      Path[Position++] = L'\\';
      for (Index = 0; Index < Depth; Index++) {
        CopyMem (Path + Position, mParents[Index].Name, StrLen (mParents[Index].Name) * sizeof (CHAR16));
        Position += StrLen (mParents[Index].Name);
        Path[Position++] = L'\\';
      }

      CopyMem (Path + Position, Item.Name, (StrLen (Item.Name) + 1) * sizeof (CHAR16));
      *PathChars = Need;
      return EFI_SUCCESS;
    }
  }
}

EFI_STATUS
EFIAPI
PrePiUfsBrowse (
  OUT UINT8 *Lun, OUT UINT32 *Partition, IN OUT UINTN *PathChars, OUT CHAR16 *Path
  )
{
  EFI_STATUS Status;
  EFI_STATUS CloseStatus;
  UINTN Count;
  UINTN LunIndex;
  BROWSER_DISK Disk;
  BROWSER_PARTITION Part;
  BROWSER_FAT Fat;

  if ((Lun == NULL) || (Partition == NULL) || (PathChars == NULL) || (Path == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = PrePiUfsReadOpen ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Count = MAX_LUNS;
  Status = PrePiUfsReadGetLuns (&Count, mLuns);
  while (!EFI_ERROR (Status)) {
    Status = SelectLun (Count, &LunIndex);
    if (EFI_ERROR (Status)) {
      break;
    }

    Status = BrowserDiskOpen (&mLuns[LunIndex], &Disk);
    if (Status == EFI_UNSUPPORTED) {
      Status = ShowMessage ("No supported partition table");
      continue;
    }

    if (EFI_ERROR (Status)) {
      break;
    }

    for (;;) {
      Status = SelectPartition (&Disk, &Part);
      if (Status == EFI_ABORTED) {
        Status = EFI_SUCCESS;
        break;
      }

      if (EFI_ERROR (Status)) {
        break;
      }

      Status = BrowserFatOpen (&Disk, &Part, &Fat);
      if (Status == EFI_UNSUPPORTED) {
        Status = ShowMessage ("Partition is not a supported FAT volume");
        if (EFI_ERROR (Status)) {
          break;
        }

        continue;
      }

      if (EFI_ERROR (Status)) {
        break;
      }

      Status = SelectFile (&Fat, PathChars, Path);
      if (Status == EFI_ABORTED) {
        Status = EFI_SUCCESS;
        continue;
      }

      if (!EFI_ERROR (Status)) {
        *Lun = mLuns[LunIndex].Lun;
        *Partition = Part.Index;
      }

      goto Done;
    }
  }

Done:
  CloseStatus = PrePiUfsReadClose ();
  if (EFI_ERROR (CloseStatus)) {
    DEBUG ((DEBUG_ERROR, "PrePiUfsBrowser: DMA close failed: %r\n", CloseStatus));
    CpuDeadLoop ();
  }

  return Status;
}
