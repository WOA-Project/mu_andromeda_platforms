/** @file
  Internal read-only partition and FAT interfaces.
  SPDX-License-Identifier: MIT
**/
#ifndef PRE_PI_BROWSER_DISK_H_
#define PRE_PI_BROWSER_DISK_H_

#include <Uefi.h>
#include <Library/PrePiUfsReadLib.h>

typedef struct {
  UINT64 Start;
  UINT64 Size;
  UINT32 Index;
  CHAR8  Name[80];
} BROWSER_PARTITION;

typedef struct {
  PRE_PI_UFS_LUN Lun;
  UINT64         TableOffset;
  UINT64         FirstUsable;
  UINT64         LastUsable;
  UINT64         ExtendedStart;
  UINT64         ExtendedEnd;
  UINT32         EntryCount;
  UINT32         EntrySize;
  BOOLEAN        Gpt;
} BROWSER_DISK;

typedef struct {
  BROWSER_DISK      *Disk;
  BROWSER_PARTITION Partition;
  UINT32            SectorSize;
  UINT32            ClusterSectors;
  UINT32            ClusterCount;
  UINT32            FatSectors;
  UINT32            FatStart;
  UINT32            DataStart;
  UINT32            RootStart;
  UINT32            RootEntries;
  UINT32            RootCluster;
  UINT8             FatBits;
} BROWSER_FAT;

typedef struct {
  BROWSER_FAT *Fat;
  UINT32      Cluster;
  UINT32      ClusterSteps;
  UINT64      Offset;
  BOOLEAN     FixedRoot;
} BROWSER_DIRECTORY;

typedef struct {
  CHAR16  Name[256];
  UINT32  Cluster;
  UINT32  Size;
  BOOLEAN Directory;
} BROWSER_FILE;

EFI_STATUS BrowserDiskOpen (IN CONST PRE_PI_UFS_LUN *Lun, OUT BROWSER_DISK *Disk);
EFI_STATUS BrowserPartitionGet (IN BROWSER_DISK *Disk, IN UINT32 Index, OUT BROWSER_PARTITION *Partition);
EFI_STATUS BrowserFatOpen (IN BROWSER_DISK *Disk, IN CONST BROWSER_PARTITION *Partition, OUT BROWSER_FAT *Fat);
VOID BrowserDirectoryOpen (IN BROWSER_FAT *Fat, IN UINT32 Cluster, OUT BROWSER_DIRECTORY *Directory);
EFI_STATUS BrowserDirectoryNext (IN OUT BROWSER_DIRECTORY *Directory, OUT BROWSER_FILE *File);

#endif
