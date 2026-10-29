/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/PlatformMemoryMapLib.h>
#include <Library/PlatformMemoryMapConfigLib.h>

STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX
    gDeviceMemoryDescriptorEx[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX
    gPStoreWorkspace[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
#endif
STATIC BOOLEAN    gMemoryMapLoaded;
STATIC EFI_STATUS gMemoryMapLoadStatus = EFI_NOT_READY;

STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX
    gMergedMemoryDescriptorEx[MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT];
STATIC CONST ARM_MEMORY_REGION_DESCRIPTOR_EX gRamPartitionTemplate = {
    "RAM Partition", 0, 0, AddMem, SYS_MEM, SYS_MEM_CAP, Conv, WRITE_BACK_XN};
STATIC ARM_MEMORY_REGION_DESCRIPTOR_EX gMemoryMapTerminator = {
    "Terminator", 0, 0, 0, 0, 0, 0, 0};

STATIC EFI_STATUS
LoadPlatformMemoryMap (
  VOID
  )
{
  UINTN       ConfiguredCount;
  EFI_STATUS  Status;
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  ARM_MEMORY_REGION_DESCRIPTOR_EX  PStoreDescriptor;
  CONST PLATFORM_DDR_BANK_REGION   *DdrBankRegions;
  UINTN                            DdrBankRegionCount;
#endif

  ConfiguredCount = 0;
  Status = MemoryMapConfigLoadDescriptorSection (
             "MemoryMap",
             gDeviceMemoryDescriptorEx,
             MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1,
             &ConfiguredCount
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  Status = MemoryMapConfigLoadPStoreDescriptor (&PStoreDescriptor);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MemoryMapConfigOverlayDescriptor (
             gDeviceMemoryDescriptorEx,
             &ConfiguredCount,
             MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1,
             &PStoreDescriptor,
             gPStoreWorkspace
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }
#endif

  Status = MemoryMapConfigLoadDescriptorSection (
             "RegisterMap",
             gDeviceMemoryDescriptorEx,
             MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1,
             &ConfiguredCount
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (ConfiguredCount == 0) {
    return EFI_NOT_FOUND;
  }

  gDeviceMemoryDescriptorEx[ConfiguredCount] = gMemoryMapTerminator;
#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
  Status = GetPlatformDdrBankRegions (&DdrBankRegions, &DdrBankRegionCount);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  return MemoryMapConfigValidatePStorePlacement (
           gDeviceMemoryDescriptorEx,
           ConfiguredCount,
           DdrBankRegions,
           DdrBankRegionCount
           );
#else
  return EFI_SUCCESS;
#endif
}

STATIC EFI_STATUS
EnsurePlatformMemoryMapLoaded (
  VOID
  )
{
  if (!gMemoryMapLoaded) {
    gMemoryMapLoadStatus = LoadPlatformMemoryMap ();
    if (gMemoryMapLoadStatus != EFI_NOT_FOUND) {
      gMemoryMapLoaded = TRUE;
    }
  }

  return gMemoryMapLoadStatus;
}

STATIC BOOLEAN
DescriptorOverlapsRegion (
  CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor,
  CONST PLATFORM_DDR_BANK_REGION         *Region
  )
{
  return Descriptor->Address < Region->Base + Region->Length &&
         Region->Base < Descriptor->Address + Descriptor->Length;
}

STATIC EFI_STATUS
AppendRamPartition (
  IN OUT UINTN                *Count,
  IN     EFI_PHYSICAL_ADDRESS  Base,
  IN     UINT64                Length
  )
{
  if (*Count >= MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1) {
    return EFI_OUT_OF_RESOURCES;
  }

  gMergedMemoryDescriptorEx[*Count] = gRamPartitionTemplate;
  gMergedMemoryDescriptorEx[*Count].Address = Base;
  gMergedMemoryDescriptorEx[*Count].Length = Length;
  (*Count)++;
  return EFI_SUCCESS;
}

STATIC EFI_STATUS
AppendDescriptor (
  IN OUT UINTN                                  *Count,
  IN     CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor
  )
{
  if (*Count >= MAX_ARM_MEMORY_REGION_DESCRIPTOR_COUNT - 1) {
    return EFI_OUT_OF_RESOURCES;
  }

  gMergedMemoryDescriptorEx[*Count] = *Descriptor;
  (*Count)++;
  return EFI_SUCCESS;
}

STATIC EFI_STATUS
MergeDdrBankRegions (
  IN  CONST PLATFORM_DDR_BANK_REGION  *DdrBankRegions,
  IN  UINTN                           DdrBankRegionCount,
  OUT ARM_MEMORY_REGION_DESCRIPTOR_EX  **MergedMemoryMap
  )
{
  UINTN       Index;
  UINTN       RegionIndex;
  UINTN       Count;
  EFI_STATUS  Status;

  Count = 0;
  for (RegionIndex = 0; RegionIndex < DdrBankRegionCount; RegionIndex++) {
    CONST PLATFORM_DDR_BANK_REGION  *Region;
    EFI_PHYSICAL_ADDRESS            Cursor;

    Region = &DdrBankRegions[RegionIndex];
    Cursor = Region->Base;
    for (Index = 0; gDeviceMemoryDescriptorEx[Index].Length != 0; Index++) {
      CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor;
      EFI_PHYSICAL_ADDRESS                    DescriptorEnd;

      Descriptor    = &gDeviceMemoryDescriptorEx[Index];
      DescriptorEnd = Descriptor->Address + Descriptor->Length;
      if (!DescriptorOverlapsRegion (Descriptor, Region)) {
        continue;
      }

      if (Descriptor->Address > Cursor) {
        Status = AppendRamPartition (
                   &Count,
                   Cursor,
                   Descriptor->Address - Cursor
                   );
        if (EFI_ERROR (Status)) {
          return Status;
        }
      }

      Status = AppendDescriptor (&Count, Descriptor);
      if (EFI_ERROR (Status)) {
        return Status;
      }

      if (DescriptorEnd > Cursor) {
        Cursor = DescriptorEnd;
      }
    }

    if (Cursor < Region->Base + Region->Length) {
      Status = AppendRamPartition (
                 &Count,
                 Cursor,
                 Region->Base + Region->Length - Cursor
                 );
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  for (Index = 0; gDeviceMemoryDescriptorEx[Index].Length != 0; Index++) {
    BOOLEAN  OverlapsDdrBank;

    OverlapsDdrBank = FALSE;
    for (RegionIndex = 0; RegionIndex < DdrBankRegionCount; RegionIndex++) {
      if (DescriptorOverlapsRegion (
            &gDeviceMemoryDescriptorEx[Index],
            &DdrBankRegions[RegionIndex]
            )) {
        OverlapsDdrBank = TRUE;
        break;
      }
    }

    if (!OverlapsDdrBank) {
      Status = AppendDescriptor (&Count, &gDeviceMemoryDescriptorEx[Index]);
      if (EFI_ERROR (Status)) {
        return Status;
      }
    }
  }

  gMergedMemoryDescriptorEx[Count] = gMemoryMapTerminator;
  *MergedMemoryMap = gMergedMemoryDescriptorEx;
  return EFI_SUCCESS;
}

ARM_MEMORY_REGION_DESCRIPTOR_EX *
GetPlatformMemoryMap (
  VOID
  )
{
  CONST PLATFORM_DDR_BANK_REGION  *DdrBankRegions;
  UINTN                           DdrBankRegionCount;
  EFI_STATUS                      Status;
  ARM_MEMORY_REGION_DESCRIPTOR_EX *MemoryMap;

  Status = EnsurePlatformMemoryMapLoaded ();
  if (!EFI_ERROR (Status)) {
    Status = GetPlatformDdrBankRegions (&DdrBankRegions, &DdrBankRegionCount);
  }

  if (!EFI_ERROR (Status)) {
    Status = MergeDdrBankRegions (
               DdrBankRegions,
               DdrBankRegionCount,
               &MemoryMap
               );
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Failed to load platform memory map from boot config: %r\n",
      Status
      ));
    ASSERT_EFI_ERROR (Status);
    return &gMemoryMapTerminator;
  }

  return MemoryMap;
}
