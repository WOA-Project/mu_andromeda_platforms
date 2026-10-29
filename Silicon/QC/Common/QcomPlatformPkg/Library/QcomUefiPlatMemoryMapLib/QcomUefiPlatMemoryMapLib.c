/** @file
  Helpers for reading Qualcomm-style memory-map records from boot configuration.

  Copyright (c) 2022-2024 DuoWoA authors

  SPDX-License-Identifier: MIT
**/

#include <Library/AndromedaBootConfigLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/QcomUefiPlatMemoryMapLib.h>

STATIC
CHAR8 *
MemoryMapConfigTrimField (
  IN OUT  CHAR8  *Field
  )
{
  CHAR8  *End;

  while ((*Field == ' ') || (*Field == '\t')) {
    Field++;
  }

  End = Field + AsciiStrLen (Field);
  while ((End > Field) && ((End[-1] == ' ') || (End[-1] == '\t'))) {
    *--End = '\0';
  }

  return Field;
}

STATIC
EFI_STATUS
MemoryMapConfigSplitCsv (
  IN OUT  CHAR8  *Record,
  OUT     CHAR8  **Fields,
  IN      UINTN  ExpectedFieldCount
  )
{
  UINTN    FieldCount;
  UINTN    Index;
  BOOLEAN  InQuotes;

  FieldCount = 1;
  InQuotes   = FALSE;
  Fields[0]  = Record;
  for (Index = 0; Record[Index] != '\0'; Index++) {
    if (Record[Index] == '"') {
      if (InQuotes && (Record[Index + 1] == '"')) {
        Index++;
      } else {
        InQuotes = (BOOLEAN)!InQuotes;
      }
    } else if ((Record[Index] == ',') && !InQuotes) {
      Record[Index] = '\0';
      if (FieldCount >= ExpectedFieldCount) {
        return EFI_COMPROMISED_DATA;
      }

      Fields[FieldCount++] = &Record[Index + 1];
    }
  }

  if (InQuotes || (FieldCount != ExpectedFieldCount)) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < FieldCount; Index++) {
    Fields[Index] = MemoryMapConfigTrimField (Fields[Index]);
    if (Fields[Index][0] == '"') {
      UINTN  Length;
      CHAR8  *Source;
      CHAR8  *Destination;

      Length = AsciiStrLen (Fields[Index]);
      if ((Length < 2) || (Fields[Index][Length - 1] != '"')) {
        return EFI_COMPROMISED_DATA;
      }

      Source      = Fields[Index] + 1;
      Destination = Fields[Index];
      while (*Source != '\0') {
        if ((*Source == '"') && (Source[1] == '"')) {
          Source++;
        }

        *Destination++ = *Source++;
      }

      if (Destination == Fields[Index]) {
        return EFI_COMPROMISED_DATA;
      }

      Destination[-1] = '\0';
    } else if (AsciiStrStr (Fields[Index], "\"") != NULL) {
      return EFI_COMPROMISED_DATA;
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
MemoryMapConfigParseUint64 (
  IN      CONST CHAR8  *Text,
  OUT     UINT64       *Value
  )
{
  UINTN   Index;
  UINTN   Radix;
  UINT64  ParsedValue;
  UINTN   Digit;

  if ((Text == NULL) || (Value == NULL) || (Text[0] == '\0')) {
    return EFI_COMPROMISED_DATA;
  }

  Index       = 0;
  Radix       = 10;
  ParsedValue = 0;
  if ((Text[0] == '0') && ((Text[1] == 'x') || (Text[1] == 'X'))) {
    Radix = 16;
    Index = 2;
    if (Text[Index] == '\0') {
      return EFI_COMPROMISED_DATA;
    }
  }

  for (; Text[Index] != '\0'; Index++) {
    if ((Text[Index] >= '0') && (Text[Index] <= '9')) {
      Digit = (UINTN)(Text[Index] - '0');
    } else if ((Radix == 16) && (Text[Index] >= 'a') && (Text[Index] <= 'f')) {
      Digit = (UINTN)(Text[Index] - 'a' + 10);
    } else if ((Radix == 16) && (Text[Index] >= 'A') && (Text[Index] <= 'F')) {
      Digit = (UINTN)(Text[Index] - 'A' + 10);
    } else {
      return EFI_COMPROMISED_DATA;
    }

    if ((Digit >= Radix) ||
        (ParsedValue > ((~(UINT64)0 - Digit) / Radix))) {
      return EFI_COMPROMISED_DATA;
    }

    ParsedValue = (ParsedValue * Radix) + Digit;
  }

  *Value = ParsedValue;
  return EFI_SUCCESS;
}

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
#endif

STATIC
EFI_STATUS
MemoryMapConfigParseDescriptor (
  IN OUT  CHAR8                         *Record,
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX  *Descriptor
  )
{
  CHAR8        *Fields[8];
  EFI_STATUS   Status;
  UINT64       NumericValue;
  UINTN        NameLength;
  UINTN        Index;
  CONST CHAR8  *HobOptions[] = {
    "NoHob", "AddMem", "AddDev", "HobOnlyNoCacheSetting", "AllocOnly"
  };
  CONST CHAR8  *ResourceTypes[] = {
    "FIRMWARE_DEV", "IO", "IO_RES", "MEM_RES", "MMAP_IO", "MMAP_IO_PORT",
    "SYS_MEM"
  };
  CONST CHAR8  *ResourceAttributes[] = {
    "EXECUTION_PROTECTED", "INITIALIZED", "PRESENT", "PROTECTED", "SYS_MEM_CAP",
    "SYS_MEM_CAP_XN", "TESTED", "UNCACHEABLE", "WRITE_COMBINEABLE",
    "WRITE_PROTECTED"
  };
  CONST CHAR8  *MemoryTypes[] = {
    "ACPI_NVS", "ACPIReclaim", "BsCode", "BsData", "Conv", "LdCode", "LdData",
    "MaxMem", "MmIO", "MmIOPort", "PalCode", "Reserv", "RtCode", "RtData",
    "Unusable"
  };
  CONST CHAR8  *ArmAttributes[] = {
    "DEVICE", "NS_DEVICE", "NS_UNCACHED_UNBUFFERED", "NS_WRITE_BACK",
    "NS_WRITE_THROUGH", "UNCACHED_UNBUFFERED", "UNCACHED_UNBUFFERED_XN",
    "WRITE_BACK", "WRITE_BACK_XN", "WRITE_THROUGH", "WRITE_THROUGH_XN"
  };
  EFI_RESOURCE_TYPE  ParsedResourceType[] = {
    EFI_RESOURCE_FIRMWARE_DEVICE, EFI_RESOURCE_IO, EFI_RESOURCE_IO_RESERVED,
    EFI_RESOURCE_MEMORY_RESERVED, EFI_RESOURCE_MEMORY_MAPPED_IO,
    EFI_RESOURCE_MEMORY_MAPPED_IO_PORT, EFI_RESOURCE_SYSTEM_MEMORY
  };
  EFI_RESOURCE_ATTRIBUTE_TYPE  ParsedResourceAttributes[] = {
    EFI_RESOURCE_ATTRIBUTE_EXECUTION_PROTECTED, INITIALIZED, PRESENT,
    EFI_RESOURCE_ATTRIBUTE_READ_PROTECTED, SYS_MEM_CAP, SYS_MEM_CAP_XN,
    TESTED, UNCACHEABLE, WRITE_COMBINEABLE,
    EFI_RESOURCE_ATTRIBUTE_WRITE_PROTECTED
  };
  EFI_MEMORY_TYPE  ParsedMemoryTypes[] = {
    EfiACPIMemoryNVS, EfiACPIReclaimMemory, EfiBootServicesCode,
    EfiBootServicesData, EfiConventionalMemory, EfiLoaderCode,
    EfiLoaderData, EfiMaxMemoryType, EfiMemoryMappedIO,
    EfiMemoryMappedIOPortSpace, EfiPalCode, EfiReservedMemoryType,
    EfiRuntimeServicesCode, EfiRuntimeServicesData, EfiUnusableMemory
  };
  ARM_MEMORY_REGION_ATTRIBUTES  ParsedArmAttributes[] = {
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
    ARM_MEMORY_REGION_ATTRIBUTE_DEVICE,
    ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH,
    ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED,
    ARM_MEMORY_REGION_ATTRIBUTE_UNCACHED_UNBUFFERED,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_BACK,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH,
    ARM_MEMORY_REGION_ATTRIBUTE_WRITE_THROUGH
  };

  if ((Record == NULL) || (Descriptor == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = MemoryMapConfigSplitCsv (Record, Fields, 8);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  SetMem (Descriptor, sizeof (*Descriptor), 0);
  Fields[2] = MemoryMapConfigTrimField (Fields[2]);
  NameLength = AsciiStrLen (Fields[2]);
  if ((NameLength == 0) || (NameLength >= sizeof (Descriptor->Name))) {
    return EFI_COMPROMISED_DATA;
  }

  Status = MemoryMapConfigParseUint64 (Fields[0], &NumericValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Descriptor->Address = (EFI_PHYSICAL_ADDRESS)NumericValue;
  Status = MemoryMapConfigParseUint64 (Fields[1], &Descriptor->Length);
  if (EFI_ERROR (Status) || (Descriptor->Length == 0) ||
      ((UINT64)Descriptor->Address > (~(UINT64)0 - Descriptor->Length))) {
    return EFI_COMPROMISED_DATA;
  }

  CopyMem (Descriptor->Name, Fields[2], NameLength + 1);
  for (Index = 0; Index < ARRAY_SIZE (HobOptions); Index++) {
    if (AsciiStriCmp (Fields[3], HobOptions[Index]) == 0) {
      Descriptor->HobOption = (DeviceMemoryAddHob)Index;
      break;
    }
  }

  if (Index == ARRAY_SIZE (HobOptions)) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < ARRAY_SIZE (ResourceTypes); Index++) {
    if (AsciiStriCmp (Fields[4], ResourceTypes[Index]) == 0) {
      Descriptor->ResourceType = ParsedResourceType[Index];
      break;
    }
  }

  if (Index == ARRAY_SIZE (ResourceTypes)) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < ARRAY_SIZE (ResourceAttributes); Index++) {
    if (AsciiStriCmp (Fields[5], ResourceAttributes[Index]) == 0) {
      Descriptor->ResourceAttribute = ParsedResourceAttributes[Index];
      break;
    }
  }

  if (Index == ARRAY_SIZE (ResourceAttributes)) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < ARRAY_SIZE (MemoryTypes); Index++) {
    if (AsciiStriCmp (Fields[6], MemoryTypes[Index]) == 0) {
      Descriptor->MemoryType = ParsedMemoryTypes[Index];
      break;
    }
  }

  if (Index == ARRAY_SIZE (MemoryTypes)) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < ARRAY_SIZE (ArmAttributes); Index++) {
    if (AsciiStriCmp (Fields[7], ArmAttributes[Index]) == 0) {
      Descriptor->ArmAttributes = ParsedArmAttributes[Index];
      break;
    }
  }

  return (Index == ARRAY_SIZE (ArmAttributes)) ?
         EFI_COMPROMISED_DATA : EFI_SUCCESS;
}

STATIC
EFI_STATUS
MemoryMapConfigReadDescriptor (
  IN      CONST CHAR8                       *Section,
  IN      UINTN                             EntryIndex,
  OUT     ARM_MEMORY_REGION_DESCRIPTOR_EX   *Descriptor
  )
{
  CHAR8       Record[MEMORY_MAP_CONFIG_ENTRY_SIZE];
  UINTN       RecordSize;
  EFI_STATUS  Status;

  RecordSize = sizeof (Record);
  Status     = GetAndromedaBootConfigSectionEntry (
                 Section,
                 EntryIndex,
                 Record,
                 &RecordSize
                 );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Status = MemoryMapConfigParseDescriptor (Record, Descriptor);
  if (EFI_ERROR (Status)) {
    DEBUG ((
      DEBUG_ERROR,
      "Invalid %a record %u: %r\n",
      Section,
      (UINT32)EntryIndex,
      Status
      ));
  }

  return Status;
}

EFI_STATUS
MemoryMapConfigLoadDescriptorSection (
  IN      CONST CHAR8                       *Section,
  IN OUT  ARM_MEMORY_REGION_DESCRIPTOR_EX   *Descriptors,
  IN      UINTN                             DescriptorCapacity,
  IN OUT  UINTN                             *DescriptorCount
  )
{
  UINTN       EntryIndex;
  UINTN       InitialCount;
  EFI_STATUS  Status;

  if ((Section == NULL) || (Descriptors == NULL) || (DescriptorCount == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  InitialCount = *DescriptorCount;
  for (EntryIndex = 0; ; EntryIndex++) {
    ARM_MEMORY_REGION_DESCRIPTOR_EX  Descriptor;

    Status = MemoryMapConfigReadDescriptor (Section, EntryIndex, &Descriptor);
    if (Status == EFI_NOT_FOUND) {
      return (*DescriptorCount == InitialCount) ? EFI_NOT_FOUND : EFI_SUCCESS;
    }

    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (*DescriptorCount >= DescriptorCapacity) {
      return EFI_OUT_OF_RESOURCES;
    }

    Descriptors[(*DescriptorCount)++] = Descriptor;
  }
}
