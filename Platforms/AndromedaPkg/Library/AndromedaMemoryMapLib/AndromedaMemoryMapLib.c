/** @file

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT

**/

#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/AndromedaMemoryMapLib.h>
#include <Library/PlatformMemoryMapLib.h>
#include <Library/PlatformDdrBankMapLib.h>

PARM_MEMORY_REGION_DESCRIPTOR_EX gDeviceMemoryDescriptorEx;

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

#if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
EFI_STATUS
MemoryMapConfigLoadPStoreDescriptor (
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor
  )
{
  EFI_STATUS  Status;
  UINT64      Address;

  if (Descriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  SetMem (Descriptor, sizeof (*Descriptor), 0);
  Status = GetAndromedaBootConfigUint64 (
             "AndromedaUefi",
             "PStoreAddress",
             &Address
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Descriptor->Address = (EFI_PHYSICAL_ADDRESS)Address;
  if ((UINT64)Descriptor->Address != Address) {
    return EFI_COMPROMISED_DATA;
  }

  Status = GetAndromedaBootConfigUint64 (
             "AndromedaUefi",
             "PStoreSize",
             &Descriptor->Length
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((Descriptor->Length == 0) ||
      ((UINT64)Descriptor->Address > (~(UINT64)0 - Descriptor->Length))) {
    return EFI_COMPROMISED_DATA;
  }

  CopyMem (Descriptor->Name, "PStore", sizeof ("PStore"));
  Descriptor->HobOption          = AddMem;
  Descriptor->ResourceType       = MEM_RES;
  Descriptor->ResourceAttribute  = SYS_MEM_CAP;
  Descriptor->MemoryType         = Reserv;
  Descriptor->ArmAttributes      = WRITE_THROUGH_XN;
  return EFI_SUCCESS;
}

EFI_STATUS
MemoryMapConfigOverlayDescriptor (
  IN OUT  ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptors,
  IN OUT  UINTN                            *DescriptorCount,
  IN      UINTN                            DescriptorCapacity,
  IN      CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Overlay,
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX  *Workspace
  )
{
  UINTN                                Index;
  UINTN                                OutputCount;
  EFI_PHYSICAL_ADDRESS                 OverlayEnd;
  EFI_PHYSICAL_ADDRESS                 DescriptorEnd;
  ARM_MEMORY_REGION_DESCRIPTOR_EX      Descriptor;
  ARM_MEMORY_REGION_DESCRIPTOR_EX      Fragment;
  BOOLEAN                              OverlayInserted;

  if ((Descriptors == NULL) || (DescriptorCount == NULL) ||
      (Overlay == NULL) || (Workspace == NULL) ||
      (Descriptors == Workspace) ||
      (*DescriptorCount > DescriptorCapacity)) {
    return EFI_INVALID_PARAMETER;
  }

  if ((UINT64)Overlay->Address > (~(UINT64)0 - Overlay->Length)) {
    return EFI_COMPROMISED_DATA;
  }

  OverlayEnd      = Overlay->Address + Overlay->Length;
  OutputCount     = 0;
  OverlayInserted = FALSE;
  for (Index = 0; Index < *DescriptorCount; Index++) {
    Descriptor    = Descriptors[Index];
    DescriptorEnd = Descriptor.Address + Descriptor.Length;

    if (!OverlayInserted && (Descriptor.Address >= Overlay->Address)) {
      if (OutputCount >= DescriptorCapacity) {
        return EFI_OUT_OF_RESOURCES;
      }

      Workspace[OutputCount++] = *Overlay;
      OverlayInserted = TRUE;
    }

    if ((Descriptor.Address < OverlayEnd) &&
        (Overlay->Address < DescriptorEnd)) {
      if (Descriptor.Address < Overlay->Address) {
        Fragment        = Descriptor;
        Fragment.Length = Overlay->Address - Descriptor.Address;
        if (OutputCount >= DescriptorCapacity) {
          return EFI_OUT_OF_RESOURCES;
        }

        Workspace[OutputCount++] = Fragment;
      }

      if (!OverlayInserted) {
        if (OutputCount >= DescriptorCapacity) {
          return EFI_OUT_OF_RESOURCES;
        }

        Workspace[OutputCount++] = *Overlay;
        OverlayInserted = TRUE;
      }

      if (DescriptorEnd > OverlayEnd) {
        Fragment         = Descriptor;
        Fragment.Address = OverlayEnd;
        Fragment.Length  = DescriptorEnd - OverlayEnd;
        if (OutputCount >= DescriptorCapacity) {
          return EFI_OUT_OF_RESOURCES;
        }

        Workspace[OutputCount++] = Fragment;
      }
    } else {
      if (OutputCount >= DescriptorCapacity) {
        return EFI_OUT_OF_RESOURCES;
      }

      Workspace[OutputCount++] = Descriptor;
    }
  }

  if (!OverlayInserted) {
    if (OutputCount >= DescriptorCapacity) {
      return EFI_OUT_OF_RESOURCES;
    }

    Workspace[OutputCount++] = *Overlay;
  }

  CopyMem (Descriptors, Workspace, OutputCount * sizeof (*Descriptors));
  *DescriptorCount = OutputCount;
  return EFI_SUCCESS;
}

EFI_STATUS
MemoryMapConfigValidatePStorePlacement (
  IN      CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptors,
  IN      UINTN                                  DescriptorCount,
  IN      CONST PLATFORM_DDR_BANK_REGION         *DdrBankRegions,
  IN      UINTN                                  DdrBankRegionCount
  )
{
  UINTN                                  DescriptorIndex;
  UINTN                                  RegionIndex;
  UINTN                                  PStoreCount;
  EFI_PHYSICAL_ADDRESS                   PStoreEnd;
  BOOLEAN                                PStoreInDdrBank;
  CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *PStore;

  if ((Descriptors == NULL) || (DdrBankRegions == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  PStore       = NULL;
  PStoreCount  = 0;
  for (DescriptorIndex = 0;
       DescriptorIndex < DescriptorCount;
       DescriptorIndex++) {
    if (AsciiStriCmp (Descriptors[DescriptorIndex].Name, "PStore") == 0) {
      PStore = &Descriptors[DescriptorIndex];
      PStoreCount++;
    }
  }

  if (PStoreCount != 1) {
    return EFI_COMPROMISED_DATA;
  }

  PStoreEnd       = PStore->Address + PStore->Length;
  PStoreInDdrBank = FALSE;
  for (RegionIndex = 0; RegionIndex < DdrBankRegionCount; RegionIndex++) {
    EFI_PHYSICAL_ADDRESS  RegionEnd;

    RegionEnd = DdrBankRegions[RegionIndex].Base +
                DdrBankRegions[RegionIndex].Length;
    if ((PStore->Address >= DdrBankRegions[RegionIndex].Base) &&
        (PStoreEnd <= RegionEnd)) {
      PStoreInDdrBank = TRUE;
      break;
    }
  }

  if (!PStoreInDdrBank) {
    return EFI_COMPROMISED_DATA;
  }

  for (DescriptorIndex = 0;
       DescriptorIndex < DescriptorCount;
       DescriptorIndex++) {
    CONST ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor;

    Descriptor = &Descriptors[DescriptorIndex];
    if ((Descriptor != PStore) &&
        (Descriptor->Address < PStoreEnd) &&
        (PStore->Address < Descriptor->Address + Descriptor->Length)) {
      return EFI_COMPROMISED_DATA;
    }
  }

  return EFI_SUCCESS;
}

STATIC EFI_STATUS
LoadPStoreArea(
  VOID
)
{
  UINTN       ConfiguredCount;
  EFI_STATUS  Status;

  ARM_MEMORY_REGION_DESCRIPTOR_EX  PStoreDescriptor;
  CONST PLATFORM_DDR_BANK_REGION   *DdrBankRegions;
  UINTN                            DdrBankRegionCount;

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

  gDeviceMemoryDescriptorEx[ConfiguredCount] = gMemoryMapTerminator;

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
}
#endif

STATIC EFI_STATUS
EnsurePlatformMemoryMapLoaded (
  VOID
  )
{
  if (!gMemoryMapLoaded) {
    gDeviceMemoryDescriptorEx = LoadPlatformMemoryMap ();
    if (gDeviceMemoryDescriptorEx != NULL) {
      gMemoryMapLoaded = TRUE;
    }

  #if USE_MEMORY_FOR_SERIAL_OUTPUT == 1
    LoadPStoreArea();
  #endif
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
