/** @file
  Read-only UFS partition and FAT directory access for PrePi.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/PrintLib.h>
#include <Library/PrePiUfsReadLib.h>
#include "BrowserDisk.h"

#define MAX_GPT_ENTRIES  4096U
#define MAX_GPT_ENTRY    4096U
#define MAX_EBR_ENTRIES  4096U

STATIC UINT8   mBlock[65536];
STATIC UINT8   mScratch[4096];
STATIC UINT64  mCachedLba;
STATIC UINT8   mCachedLun;
STATIC BOOLEAN mCached;
STATIC UINT32  mCrcTable[256];
STATIC BOOLEAN mCrcReady;
STATIC UINT64  mEbrLbas[MAX_EBR_ENTRIES];

STATIC UINT16 Le16 (CONST UINT8 *P) { return (UINT16)(P[0] | ((UINT16)P[1] << 8)); }
STATIC UINT32 Le32 (CONST UINT8 *P) { return (UINT32)Le16 (P) | ((UINT32)Le16 (P + 2) << 16); }
STATIC UINT64 Le64 (CONST UINT8 *P) { return (UINT64)Le32 (P) | ((UINT64)Le32 (P + 4) << 32); }

STATIC
EFI_STATUS
ReadDisk (
  IN BROWSER_DISK *Disk, IN UINT64 Offset, IN UINTN Length, OUT VOID *Buffer
  )
{
  UINT64 Lba;
  UINTN InBlock;
  UINTN Chunk;
  UINT8 *Out;
  EFI_STATUS Status;

  if ((Offset > Disk->Lun.BlockCount * Disk->Lun.BlockSize) ||
      (Length > Disk->Lun.BlockCount * Disk->Lun.BlockSize - Offset)) {
    return EFI_VOLUME_CORRUPTED;
  }

  Out = Buffer;
  while (Length != 0) {
    Lba = Offset / Disk->Lun.BlockSize;
    InBlock = (UINTN)(Offset % Disk->Lun.BlockSize);
    if (!mCached || (mCachedLun != Disk->Lun.Lun) || (mCachedLba != Lba)) {
      Status = PrePiUfsReadBlocks (Disk->Lun.Lun, Lba, 1, sizeof (mBlock), mBlock);
      if (EFI_ERROR (Status)) {
        mCached = FALSE;
        return Status;
      }

      mCached = TRUE;
      mCachedLun = Disk->Lun.Lun;
      mCachedLba = Lba;
    }

    Chunk = Disk->Lun.BlockSize - InBlock;
    if (Chunk > Length) {
      Chunk = Length;
    }

    CopyMem (Out, mBlock + InBlock, Chunk);
    Out += Chunk;
    Offset += Chunk;
    Length -= Chunk;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
ReadPartition (
  IN BROWSER_FAT *Fat, IN UINT64 Offset, IN UINTN Length, OUT VOID *Buffer
  )
{
  if ((Offset > Fat->Partition.Size) || (Length > Fat->Partition.Size - Offset)) {
    return EFI_VOLUME_CORRUPTED;
  }

  return ReadDisk (Fat->Disk, Fat->Partition.Start + Offset, Length, Buffer);
}

STATIC
UINT32
CrcUpdate (
  UINT32 Crc, CONST UINT8 *Buffer, UINTN Length
  )
{
  UINTN Index;
  UINTN Bit;

  if (!mCrcReady) {
    for (Index = 0; Index < 256; Index++) {
      mCrcTable[Index] = (UINT32)Index;
      for (Bit = 0; Bit < 8; Bit++) {
        mCrcTable[Index] = (mCrcTable[Index] >> 1) ^
                           ((mCrcTable[Index] & 1U) ? 0xEDB88320U : 0U);
      }
    }

    mCrcReady = TRUE;
  }

  while (Length-- != 0) {
    Crc = (Crc >> 8) ^ mCrcTable[(Crc ^ *Buffer++) & 0xFFU];
  }

  return Crc;
}

STATIC
EFI_STATUS
ReadGpt (
  IN OUT BROWSER_DISK *Disk, IN UINT64 HeaderLba
  )
{
  EFI_STATUS Status;
  UINT32 HeaderSize;
  UINT32 HeaderCrc;
  UINT32 ArrayCrc;
  UINT32 Crc;
  UINT64 TableLba;
  UINT64 TableBytes;
  UINT64 Position;
  UINTN Chunk;

  Status = ReadDisk (Disk, HeaderLba * Disk->Lun.BlockSize, 92, mScratch);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (CompareMem (mScratch, "EFI PART", 8) != 0) {
    return EFI_VOLUME_CORRUPTED;
  }

  HeaderSize = Le32 (mScratch + 12);
  if ((HeaderSize < 92) || (HeaderSize > Disk->Lun.BlockSize) ||
      (HeaderSize > sizeof (mScratch))) {
    return EFI_VOLUME_CORRUPTED;
  }

  Status = ReadDisk (Disk, HeaderLba * Disk->Lun.BlockSize, HeaderSize, mScratch);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  HeaderCrc = Le32 (mScratch + 16);
  SetMem (mScratch + 16, 4, 0);
  if (CalculateCrc32 (mScratch, HeaderSize) != HeaderCrc) {
    return EFI_CRC_ERROR;
  }

  Disk->FirstUsable = Le64 (mScratch + 40);
  Disk->LastUsable = Le64 (mScratch + 48);
  TableLba = Le64 (mScratch + 72);
  Disk->EntryCount = Le32 (mScratch + 80);
  Disk->EntrySize = Le32 (mScratch + 84);
  ArrayCrc = Le32 (mScratch + 88);
  if ((Disk->EntryCount > MAX_GPT_ENTRIES) || (Disk->EntrySize > MAX_GPT_ENTRY)) {
    return EFI_UNSUPPORTED;
  }

  if ((Le64 (mScratch + 24) != HeaderLba) ||
      (Le64 (mScratch + 32) >= Disk->Lun.BlockCount) ||
      (Disk->FirstUsable > Disk->LastUsable) ||
      (Disk->LastUsable >= Disk->Lun.BlockCount) ||
      (Disk->EntryCount == 0) || (Disk->EntrySize < 128) ||
      ((Disk->EntrySize & (Disk->EntrySize - 1)) != 0) ||
      (TableLba >= Disk->Lun.BlockCount)) {
    return EFI_VOLUME_CORRUPTED;
  }

  TableBytes = (UINT64)Disk->EntryCount * Disk->EntrySize;
  Disk->TableOffset = TableLba * Disk->Lun.BlockSize;
  if (TableBytes > Disk->Lun.BlockCount * Disk->Lun.BlockSize - Disk->TableOffset) {
    return EFI_VOLUME_CORRUPTED;
  }

  Crc = MAX_UINT32;
  for (Position = 0; Position < TableBytes; Position += Chunk) {
    Chunk = (UINTN)((TableBytes - Position > sizeof (mScratch)) ?
                    sizeof (mScratch) : TableBytes - Position);
    Status = ReadDisk (Disk, Disk->TableOffset + Position, Chunk, mScratch);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Crc = CrcUpdate (Crc, mScratch, Chunk);
  }

  if ((Crc ^ MAX_UINT32) != ArrayCrc) {
    return EFI_CRC_ERROR;
  }

  Disk->Gpt = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
BrowserDiskOpen (
  IN CONST PRE_PI_UFS_LUN *Lun, OUT BROWSER_DISK *Disk
  )
{
  EFI_STATUS Status;
  UINTN Index;
  UINTN Seen;
  BOOLEAN Protective;
  UINT8 Mbr[512];
  UINT8 Entry[32];
  UINT8 Type;
  UINT64 Current;
  UINT64 Next;
  UINT64 Length;

  if ((Lun->BlockSize < 512) || (Lun->BlockSize > sizeof (mBlock)) ||
      (Lun->BlockCount < 2) || (Lun->BlockCount > MAX_UINT64 / Lun->BlockSize)) {
    return EFI_UNSUPPORTED;
  }

  ZeroMem (Disk, sizeof (*Disk));
  Disk->Lun = *Lun;
  mCached = FALSE;
  Status = ReadDisk (Disk, 0, 512, mScratch);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((mScratch[510] != 0x55) || (mScratch[511] != 0xAA)) {
    return EFI_UNSUPPORTED;
  }

  CopyMem (Mbr, mScratch, sizeof (Mbr));
  Protective = FALSE;
  for (Index = 0; Index < 4; Index++) {
    if (Mbr[446 + Index * 16 + 4] == 0xEE) {
      Protective = TRUE;
    }
  }

  if (Protective) {
    Status = ReadGpt (Disk, 1);
    if ((Status == EFI_CRC_ERROR) || (Status == EFI_VOLUME_CORRUPTED)) {
      Status = ReadGpt (Disk, Lun->BlockCount - 1);
    }

    return Status;
  }

  if (Lun->BlockSize != 512) {
    return EFI_UNSUPPORTED;
  }

  Disk->EntryCount = 4;
  for (Index = 0; Index < 4; Index++) {
    Type = Mbr[446 + Index * 16 + 4];
    if ((Type != 0x05) && (Type != 0x0F) && (Type != 0x85)) {
      continue;
    }

    if (Disk->ExtendedStart != 0) {
      return EFI_UNSUPPORTED;
    }

    Disk->ExtendedStart = Le32 (Mbr + 446 + Index * 16 + 8);
    Length = Le32 (Mbr + 446 + Index * 16 + 12);
    Disk->ExtendedEnd = Disk->ExtendedStart + Length;
    if ((Disk->ExtendedStart == 0) || (Length == 0) ||
        (Disk->ExtendedEnd > Disk->Lun.BlockCount)) {
      return EFI_VOLUME_CORRUPTED;
    }
  }

  if (Disk->ExtendedStart == 0) {
    return EFI_SUCCESS;
  }

  Current = Disk->ExtendedStart;
  for (;;) {
    if (Disk->EntryCount - 4 >= MAX_EBR_ENTRIES) {
      return EFI_UNSUPPORTED;
    }

    for (Seen = 0; Seen < Disk->EntryCount - 4; Seen++) {
      if (mEbrLbas[Seen] == Current) {
        return EFI_VOLUME_CORRUPTED;
      }
    }

    Status = ReadDisk (Disk, Current * 512, 512, mScratch);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if ((mScratch[510] != 0x55) || (mScratch[511] != 0xAA)) {
      return EFI_VOLUME_CORRUPTED;
    }

    CopyMem (Entry, mScratch + 446, sizeof (Entry));
    if ((Entry[4] == 0) || (Le32 (Entry + 8) == 0) || (Le32 (Entry + 12) == 0)) {
      return EFI_VOLUME_CORRUPTED;
    }

    mEbrLbas[Disk->EntryCount++ - 4] = Current;
    Type = Entry[20];
    if (Type == 0) {
      break;
    }

    if ((Type != 0x05) && (Type != 0x0F) && (Type != 0x85)) {
      return EFI_VOLUME_CORRUPTED;
    }

    Next = Disk->ExtendedStart + Le32 (Entry + 24);
    Length = Le32 (Entry + 28);
    if ((Length == 0) || (Next < Disk->ExtendedStart) ||
        (Next >= Disk->ExtendedEnd) || (Length > Disk->ExtendedEnd - Next)) {
      return EFI_VOLUME_CORRUPTED;
    }

    Current = Next;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
BrowserPartitionGet (
  IN BROWSER_DISK *Disk, IN UINT32 Index, OUT BROWSER_PARTITION *Partition
  )
{
  EFI_STATUS Status;
  UINT8 Entry[128];
  UINT64 Start;
  UINT64 End;
  UINTN Byte;
  UINTN Name;
  UINT16 Ch;

  if (Index >= Disk->EntryCount) {
    return EFI_INVALID_PARAMETER;
  }

  if (Disk->Gpt) {
    Status = ReadDisk (Disk, Disk->TableOffset + (UINT64)Index * Disk->EntrySize,
                       sizeof (Entry), Entry);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    for (Byte = 0; (Byte < 16) && (Entry[Byte] == 0); Byte++) {
    }

    if (Byte == 16) {
      return EFI_NOT_FOUND;
    }

    Start = Le64 (Entry + 32);
    End = Le64 (Entry + 40);
    if ((Start < Disk->FirstUsable) || (End > Disk->LastUsable) || (Start > End)) {
      return EFI_VOLUME_CORRUPTED;
    }

    Partition->Start = Start * Disk->Lun.BlockSize;
    Partition->Size = (End - Start + 1) * Disk->Lun.BlockSize;
    Partition->Index = Index + 1;
    AsciiSPrint (Partition->Name, sizeof (Partition->Name), "Partition %u: ", Index + 1);
    Name = AsciiStrLen (Partition->Name);
    for (Byte = 0; Byte < 36; Byte++) {
      Ch = Le16 (Entry + 56 + Byte * 2);
      if (Ch == 0) {
        break;
      }

      Partition->Name[Name++] = (Ch >= 32 && Ch <= 126) ? (CHAR8)Ch : '?';
    }

    Partition->Name[Name] = '\0';
    return EFI_SUCCESS;
  }

  if (Index < 4) {
    Status = ReadDisk (Disk, 446 + Index * 16, 16, Entry);
    Start = 0;
  } else {
    Start = mEbrLbas[Index - 4];
    Status = ReadDisk (Disk, Start * 512 + 446, 16, Entry);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Entry[4] == 0) || (Le32 (Entry + 12) == 0)) {
    return EFI_NOT_FOUND;
  }

  if ((Entry[4] == 0x05) || (Entry[4] == 0x0F) || (Entry[4] == 0x85)) {
    return (Index < 4) ? EFI_NOT_FOUND : EFI_VOLUME_CORRUPTED;
  }

  Start += Le32 (Entry + 8);
  End = Start + Le32 (Entry + 12);
  if ((Start == 0) || (End > Disk->Lun.BlockCount) ||
      ((Index >= 4) && ((Start <= mEbrLbas[Index - 4]) ||
                        (Start < Disk->ExtendedStart) || (End > Disk->ExtendedEnd)))) {
    return EFI_VOLUME_CORRUPTED;
  }

  Partition->Start = Start * 512;
  Partition->Size = (End - Start) * 512;
  Partition->Index = Index + 1;
  AsciiSPrint (Partition->Name, sizeof (Partition->Name), "Partition %u (type %02x)",
               Index + 1, Entry[4]);
  return EFI_SUCCESS;
}

EFI_STATUS
BrowserFatOpen (
  IN BROWSER_DISK *Disk, IN CONST BROWSER_PARTITION *Partition, OUT BROWSER_FAT *Fat
  )
{
  EFI_STATUS Status;
  UINT8 Boot[512];
  UINT32 SectorSize;
  UINT32 ClusterSectors;
  UINT32 Reserved;
  UINT32 Fats;
  UINT32 RootEntries;
  UINT32 FatSectors;
  UINT32 TotalSectors;
  UINT32 RootSectors;
  UINT32 ActiveFat;
  UINT64 RootStart;
  UINT64 DataStart;
  UINT64 Clusters;
  UINT64 FatBytes;
  UINT16 Flags;

  ZeroMem (Fat, sizeof (*Fat));
  Fat->Disk = Disk;
  Fat->Partition = *Partition;
  if (Partition->Size < sizeof (Boot)) {
    return EFI_UNSUPPORTED;
  }

  Status = ReadPartition (Fat, 0, sizeof (Boot), Boot);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Boot[510] != 0x55) || (Boot[511] != 0xAA) ||
      ((Boot[0] != 0xEB) && (Boot[0] != 0xE9))) {
    return EFI_UNSUPPORTED;
  }

  SectorSize = Le16 (Boot + 11);
  ClusterSectors = Boot[13];
  Reserved = Le16 (Boot + 14);
  Fats = Boot[16];
  RootEntries = Le16 (Boot + 17);
  TotalSectors = Le16 (Boot + 19);
  if (TotalSectors == 0) {
    TotalSectors = Le32 (Boot + 32);
  }

  FatSectors = Le16 (Boot + 22);
  if (FatSectors == 0) {
    FatSectors = Le32 (Boot + 36);
  }

  if ((SectorSize < 512) || (SectorSize > 4096) ||
      ((SectorSize & (SectorSize - 1)) != 0) ||
      (ClusterSectors == 0) || (ClusterSectors > 128) ||
      ((ClusterSectors & (ClusterSectors - 1)) != 0) ||
      (Reserved == 0) || (Fats == 0) || (FatSectors == 0) ||
      (TotalSectors == 0) || ((UINT64)TotalSectors * SectorSize > Partition->Size)) {
    return EFI_UNSUPPORTED;
  }

  RootSectors = (RootEntries * 32U + SectorSize - 1) / SectorSize;
  RootStart = (UINT64)Reserved + (UINT64)Fats * FatSectors;
  DataStart = RootStart + RootSectors;
  if (DataStart >= TotalSectors) {
    return EFI_VOLUME_CORRUPTED;
  }

  Clusters = (TotalSectors - DataStart) / ClusterSectors;
  if ((Clusters == 0) || (Clusters > 0x0FFFFFF5U)) {
    return EFI_VOLUME_CORRUPTED;
  }

  Fat->FatBits = (Clusters < 4085) ? 12 : ((Clusters < 65525) ? 16 : 32);
  if (((Fat->FatBits == 32) && ((RootEntries != 0) || (Le16 (Boot + 22) != 0) ||
                                 (Le16 (Boot + 42) != 0))) ||
      ((Fat->FatBits != 32) && ((RootEntries == 0) || (Le16 (Boot + 22) == 0)))) {
    return EFI_VOLUME_CORRUPTED;
  }

  FatBytes = ((Clusters + 2) * Fat->FatBits + 7) / 8;
  if (FatBytes > (UINT64)FatSectors * SectorSize) {
    return EFI_VOLUME_CORRUPTED;
  }

  ActiveFat = 0;
  if (Fat->FatBits == 32) {
    Flags = Le16 (Boot + 40);
    if ((Flags & BIT7) != 0) {
      ActiveFat = Flags & 0x0F;
      if (ActiveFat >= Fats) {
        return EFI_VOLUME_CORRUPTED;
      }
    }

    Fat->RootCluster = Le32 (Boot + 44) & 0x0FFFFFFFU;
    if ((Fat->RootCluster < 2) || (Fat->RootCluster >= Clusters + 2)) {
      return EFI_VOLUME_CORRUPTED;
    }
  }

  Fat->SectorSize = SectorSize;
  Fat->ClusterSectors = ClusterSectors;
  Fat->ClusterCount = (UINT32)Clusters;
  Fat->FatSectors = FatSectors;
  Fat->FatStart = Reserved + ActiveFat * FatSectors;
  Fat->RootStart = (UINT32)RootStart;
  Fat->DataStart = (UINT32)DataStart;
  Fat->RootEntries = RootEntries;
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
NextCluster (
  IN BROWSER_FAT *Fat, IN UINT32 Cluster, OUT UINT32 *Next
  )
{
  UINT64 Offset;
  UINT32 Value;
  UINT32 Last;
  UINT8 Bytes[4];
  UINTN Size;
  EFI_STATUS Status;

  if ((Cluster < 2) || (Cluster >= Fat->ClusterCount + 2)) {
    return EFI_VOLUME_CORRUPTED;
  }

  Offset = (Fat->FatBits == 12) ? Cluster + Cluster / 2 :
           ((UINT64)Cluster * (Fat->FatBits / 8));
  Size = (Fat->FatBits == 32) ? 4 : 2;
  if (Offset + Size > (UINT64)Fat->FatSectors * Fat->SectorSize) {
    return EFI_VOLUME_CORRUPTED;
  }

  Status = ReadPartition (Fat, (UINT64)Fat->FatStart * Fat->SectorSize + Offset,
                          Size, Bytes);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (Fat->FatBits == 12) {
    Value = Le16 (Bytes);
    Value = (Cluster & 1) ? (Value >> 4) : (Value & 0xFFF);
    Last = 0xFF8;
  } else if (Fat->FatBits == 16) {
    Value = Le16 (Bytes);
    Last = 0xFFF8;
  } else {
    Value = Le32 (Bytes) & 0x0FFFFFFF;
    Last = 0x0FFFFFF8;
  }

  if (Value >= Last) {
    return EFI_NOT_FOUND;
  }

  if ((Value < 2) || (Value >= Fat->ClusterCount + 2) ||
      (Value >= Last - 8)) {
    return EFI_VOLUME_CORRUPTED;
  }

  *Next = Value;
  return EFI_SUCCESS;
}

VOID
BrowserDirectoryOpen (
  IN BROWSER_FAT *Fat, IN UINT32 Cluster, OUT BROWSER_DIRECTORY *Directory
  )
{
  ZeroMem (Directory, sizeof (*Directory));
  Directory->Fat = Fat;
  Directory->FixedRoot = (Cluster == 0) && (Fat->FatBits != 32);
  Directory->Cluster = (Cluster == 0) ? Fat->RootCluster : Cluster;
}

STATIC
EFI_STATUS
ReadDirectoryEntry (
  IN OUT BROWSER_DIRECTORY *Directory, OUT UINT8 *Entry
  )
{
  BROWSER_FAT *Fat;
  UINT64 ClusterBytes;
  UINT64 Offset;
  UINT32 Next;
  EFI_STATUS Status;

  Fat = Directory->Fat;
  if (Directory->FixedRoot) {
    if (Directory->Offset >= (UINT64)Fat->RootEntries * 32) {
      return EFI_NOT_FOUND;
    }

    Offset = (UINT64)Fat->RootStart * Fat->SectorSize + Directory->Offset;
  } else {
    ClusterBytes = (UINT64)Fat->ClusterSectors * Fat->SectorSize;
    if (Directory->Offset == ClusterBytes) {
      Status = NextCluster (Fat, Directory->Cluster, &Next);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (++Directory->ClusterSteps >= Fat->ClusterCount) {
        return EFI_VOLUME_CORRUPTED;
      }

      Directory->Cluster = Next;
      Directory->Offset = 0;
    }

    if ((Directory->Cluster < 2) || (Directory->Cluster >= Fat->ClusterCount + 2)) {
      return EFI_VOLUME_CORRUPTED;
    }

    Offset = ((UINT64)Fat->DataStart +
              (UINT64)(Directory->Cluster - 2) * Fat->ClusterSectors) *
             Fat->SectorSize + Directory->Offset;
  }

  Status = ReadPartition (Fat, Offset, 32, Entry);
  if (!EFI_ERROR (Status)) {
    Directory->Offset += 32;
  }

  return Status;
}

STATIC
UINT8
ShortNameChecksum (
  IN CONST UINT8 *Name
  )
{
  UINT8 Sum;
  UINTN Index;

  Sum = 0;
  for (Index = 0; Index < 11; Index++) {
    Sum = (UINT8)(((Sum & 1) << 7) + (Sum >> 1) + Name[Index]);
  }

  return Sum;
}

STATIC
VOID
ShortName (
  IN CONST UINT8 *Entry, OUT CHAR16 *Name
  )
{
  UINTN Index;
  UINTN Pos;
  UINT8 Ch;

  Pos = 0;
  for (Index = 0; (Index < 8) && (Entry[Index] != ' '); Index++) {
    Ch = Entry[Index];
    if ((Index == 0) && (Ch == 0x05)) {
      Ch = 0xE5;
    }

    if (((Entry[12] & BIT3) != 0) && (Ch >= 'A') && (Ch <= 'Z')) {
      Ch = (UINT8)(Ch + ('a' - 'A'));
    }

    Name[Pos++] = Ch;
  }

  if (Entry[8] != ' ') {
    Name[Pos++] = '.';
    for (Index = 8; (Index < 11) && (Entry[Index] != ' '); Index++) {
      Ch = Entry[Index];
      if (((Entry[12] & BIT4) != 0) && (Ch >= 'A') && (Ch <= 'Z')) {
        Ch = (UINT8)(Ch + ('a' - 'A'));
      }

      Name[Pos++] = Ch;
    }
  }

  Name[Pos] = 0;
}

EFI_STATUS
BrowserDirectoryNext (
  IN OUT BROWSER_DIRECTORY *Directory, OUT BROWSER_FILE *File
  )
{
  STATIC CONST UINT8 Positions[13] = { 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
  UINT8 Entry[32];
  CHAR16 LongName[256];
  UINT8 Expected;
  UINT8 Checksum;
  UINT8 Ordinal;
  UINTN Index;
  UINTN Position;
  UINT16 Ch;
  BOOLEAN Valid;
  BOOLEAN LongValid;
  EFI_STATUS Status;

  Expected = 0;
  Checksum = 0;
  LongValid = FALSE;
  ZeroMem (LongName, sizeof (LongName));
  for (;;) {
    Status = ReadDirectoryEntry (Directory, Entry);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (Entry[0] == 0) {
      return EFI_NOT_FOUND;
    }

    if (Entry[0] == 0xE5) {
      Expected = 0;
      LongValid = FALSE;
      continue;
    }

    if (Entry[11] == 0x0F) {
      Ordinal = Entry[0] & 0x1F;
      if ((Entry[0] & 0x40) != 0) {
        ZeroMem (LongName, sizeof (LongName));
        Expected = Ordinal;
        Checksum = Entry[13];
        LongValid = TRUE;
      }

      if ((Ordinal == 0) || (Ordinal > 20) || (Ordinal != Expected) ||
          ((Entry[0] & 0xA0) != 0) || (Entry[12] != 0) ||
          (Le16 (Entry + 26) != 0) || (Entry[13] != Checksum)) {
        Expected = 0;
        LongValid = FALSE;
        continue;
      }

      for (Index = 0; Index < 13; Index++) {
        Position = (Ordinal - 1U) * 13U + Index;
        Ch = Le16 (Entry + Positions[Index]);
        if (Position < 255) {
          LongName[Position] = Ch;
        } else if ((Ch != 0) && (Ch != 0xFFFF)) {
          Expected = 0;
          LongValid = FALSE;
          break;
        }
      }

      if (Expected != 0) {
        Expected--;
      }

      continue;
    }

    if ((Entry[11] & BIT3) != 0) {
      Expected = 0;
      LongValid = FALSE;
      continue;
    }

    ZeroMem (File, sizeof (*File));
    Valid = LongValid && (Expected == 0) &&
            (Checksum == ShortNameChecksum (Entry)) &&
            (LongName[0] != 0) && (LongName[0] != 0xFFFF);
    for (Index = 0; Valid && (Index < 255) && (LongName[Index] != 0) &&
         (LongName[Index] != 0xFFFF); Index++) {
      if ((LongName[Index] < 32) || (LongName[Index] == '\\') || (LongName[Index] == '/')) {
        Valid = FALSE;
      }
    }

    if (Valid) {
      CopyMem (File->Name, LongName, Index * sizeof (CHAR16));
      File->Name[Index] = 0;
    } else {
      ShortName (Entry, File->Name);
    }

    Expected = 0;
    Checksum = 0;
    LongValid = FALSE;
    ZeroMem (LongName, sizeof (LongName));
    if ((File->Name[0] == '.') &&
        ((File->Name[1] == 0) || ((File->Name[1] == '.') && (File->Name[2] == 0)))) {
      continue;
    }

    File->Directory = (Entry[11] & BIT4) != 0;
    File->Cluster = Le16 (Entry + 26);
    if (Directory->Fat->FatBits == 32) {
      File->Cluster |= (UINT32)Le16 (Entry + 20) << 16;
      File->Cluster &= 0x0FFFFFFF;
    }

    File->Size = Le32 (Entry + 28);
    if ((File->Directory || (File->Size != 0)) &&
        ((File->Cluster < 2) || (File->Cluster >= Directory->Fat->ClusterCount + 2))) {
      return EFI_VOLUME_CORRUPTED;
    }

    return EFI_SUCCESS;
  }
}

