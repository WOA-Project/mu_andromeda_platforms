/** @file
  Reader for INI key/value and record-oriented runtime boot configuration data.
**/

#include <Guid/AndromedaBootConfig.h>
#include <Pi/PiBootMode.h>
#include <Pi/PiHob.h>
#include <Library/AndromedaBootConfigLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>

CONST CHAR8  * volatile  gAndromedaBootConfigBuffer     = NULL;
volatile UINTN            gAndromedaBootConfigBufferSize = 0;

STATIC
BOOLEAN
IsConfigSpace (
  IN  CHAR8  Character
  )
{
  return (BOOLEAN)(
           (Character == ' ') ||
           (Character == '\t') ||
           (Character == '\r')
           );
}

STATIC
CHAR8
ToLowerAscii (
  IN  CHAR8  Character
  )
{
  if ((Character >= 'A') && (Character <= 'Z')) {
    return (CHAR8)(Character - 'A' + 'a');
  }

  return Character;
}

STATIC
BOOLEAN
RangeEqualsString (
  IN  CONST CHAR8  *Data,
  IN  UINTN         DataSize,
  IN  CONST CHAR8  *String
  )
{
  UINTN  Index;

  for (Index = 0; Index < DataSize; Index++) {
    if (String[Index] == '\0') {
      return FALSE;
    }

    if (ToLowerAscii (Data[Index]) != ToLowerAscii (String[Index])) {
      return FALSE;
    }
  }

  return (BOOLEAN)(String[DataSize] == '\0');
}

EFI_STATUS
EFIAPI
SetAndromedaBootConfigBuffer (
  IN  CONST CHAR8  *Config,
  IN  UINTN         ConfigSize
  )
{
  UINTN  Index;

  gAndromedaBootConfigBuffer     = NULL;
  gAndromedaBootConfigBufferSize = 0;
  if ((Config == NULL) || (ConfigSize == 0)) {
    return EFI_INVALID_PARAMETER;
  }

  if (ConfigSize > ANDROMEDA_BOOT_CONFIG_MAX_SIZE) {
    return EFI_COMPROMISED_DATA;
  }

  for (Index = 0; Index < ConfigSize; Index++) {
    if (Config[Index] == '\0') {
      return EFI_COMPROMISED_DATA;
    }
  }

  gAndromedaBootConfigBuffer     = Config;
  gAndromedaBootConfigBufferSize = ConfigSize;
  return EFI_SUCCESS;
}

STATIC
VOID
TrimRange (
  IN      CONST CHAR8  *Data,
  IN OUT  UINTN        *Start,
  IN OUT  UINTN        *End
  )
{
  while ((*Start < *End) && IsConfigSpace (Data[*Start])) {
    (*Start)++;
  }

  while ((*End > *Start) && IsConfigSpace (Data[*End - 1])) {
    (*End)--;
  }
}

STATIC
VOID
TrimInlineComment (
  IN      CONST CHAR8  *Data,
  IN      UINTN         Start,
  IN OUT  UINTN        *End
  )
{
  UINTN    Index;
  BOOLEAN  InQuotes;
  BOOLEAN  Escaped;

  InQuotes = FALSE;
  Escaped  = FALSE;
  for (Index = Start; Index < *End; Index++) {
    if (InQuotes && (Data[Index] == '\\') && !Escaped) {
      Escaped = TRUE;
      continue;
    }

    if (Data[Index] == '"') {
      if (!Escaped) {
        InQuotes = !InQuotes;
      }

      Escaped = FALSE;
      continue;
    }

    if (!InQuotes &&
        ((Data[Index] == '#') || (Data[Index] == ';')) &&
        ((Index == Start) || IsConfigSpace (Data[Index - 1]))) {
      *End = Index;
      TrimRange (Data, &Start, End);
      return;
    }

    Escaped = FALSE;
  }
}

STATIC
EFI_STATUS
GetBootConfig (
  OUT  CONST CHAR8  **Config,
  OUT  UINTN        *ConfigSize
  )
{
  EFI_HOB_GUID_TYPE  *ConfigHob;

  if (gAndromedaBootConfigBuffer != NULL) {
    if ((gAndromedaBootConfigBufferSize == 0) ||
        (gAndromedaBootConfigBufferSize > ANDROMEDA_BOOT_CONFIG_MAX_SIZE)) {
      return EFI_COMPROMISED_DATA;
    }

    *Config     = gAndromedaBootConfigBuffer;
    *ConfigSize = gAndromedaBootConfigBufferSize;
    return EFI_SUCCESS;
  }

  ConfigHob = GetFirstGuidHob (&gAndromedaBootConfigHobGuid);
  if (ConfigHob == NULL) {
    return EFI_NOT_FOUND;
  }

  *ConfigSize = GET_GUID_HOB_DATA_SIZE (ConfigHob);
  if ((*ConfigSize == 0) || (*ConfigSize > ANDROMEDA_BOOT_CONFIG_MAX_SIZE)) {
    return EFI_COMPROMISED_DATA;
  }

  *Config = GET_GUID_HOB_DATA (ConfigHob);
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetAndromedaBootConfigValue (
  IN      CONST CHAR8  *Section,
  IN      CONST CHAR8  *Key,
  OUT     CHAR8        *Value,
  IN OUT  UINTN        *ValueSize
  )
{
  CONST CHAR8        *Config;
  UINTN              ConfigSize;
  UINTN              Capacity;
  UINTN              LineStart;
  UINTN              LineEnd;
  UINTN              ContentStart;
  UINTN              ContentEnd;
  UINTN              SectionStart;
  UINTN              SectionEnd;
  UINTN              SectionClose;
  UINTN              KeyStart;
  UINTN              KeyEnd;
  UINTN              ValueStart;
  UINTN              ValueEnd;
  UINTN              MatchedValueStart;
  UINTN              MatchedValueEnd;
  UINTN              CommentStart;
  UINTN              RequiredSize;
  BOOLEAN            InRequestedSection;
  BOOLEAN            Found;
  EFI_STATUS         Status;

  if ((Section == NULL) || (Key == NULL) || (Key[0] == '\0') || (ValueSize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Capacity = *ValueSize;
  if ((Capacity > 0) && (Value == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetBootConfig (&Config, &ConfigSize);
  if (EFI_ERROR (Status)) {
    *ValueSize = 0;
    return Status;
  }

  for (LineStart = 0; LineStart < ConfigSize; LineStart++) {
    if (Config[LineStart] == '\0') {
      *ValueSize = 0;
      return EFI_COMPROMISED_DATA;
    }
  }

  InRequestedSection = (BOOLEAN)(Section[0] == '\0');
  Found = FALSE;
  MatchedValueStart = 0;
  MatchedValueEnd = 0;
  LineStart = 0;
  while (LineStart < ConfigSize) {
    LineEnd = LineStart;
    while ((LineEnd < ConfigSize) && (Config[LineEnd] != '\n')) {
      LineEnd++;
    }

    ContentStart = LineStart;
    ContentEnd = LineEnd;
    TrimRange (Config, &ContentStart, &ContentEnd);

    if (ContentStart < ContentEnd) {
      if ((Config[ContentStart] != '#') && (Config[ContentStart] != ';')) {
        if (Config[ContentStart] == '[') {
          SectionStart = ContentStart + 1;
          SectionEnd = SectionStart;
          while ((SectionEnd < ContentEnd) && (Config[SectionEnd] != ']')) {
            SectionEnd++;
          }

          if (SectionEnd == ContentEnd) {
            *ValueSize = 0;
            return EFI_COMPROMISED_DATA;
          }

          SectionClose = SectionEnd;
          TrimRange (Config, &SectionStart, &SectionEnd);
          CommentStart = SectionClose + 1;
          while ((CommentStart < ContentEnd) && IsConfigSpace (Config[CommentStart])) {
            CommentStart++;
          }

          if ((CommentStart < ContentEnd) &&
              (Config[CommentStart] != '#') &&
              (Config[CommentStart] != ';')) {
            *ValueSize = 0;
            return EFI_COMPROMISED_DATA;
          }

          InRequestedSection = RangeEqualsString (
                                 &Config[SectionStart],
                                 SectionEnd - SectionStart,
                                 Section
                                 );
        } else {
          KeyStart = ContentStart;
          KeyEnd = KeyStart;
          while ((KeyEnd < ContentEnd) && (Config[KeyEnd] != '=')) {
            KeyEnd++;
          }

          if (KeyEnd == ContentEnd) {
            LineStart = (LineEnd < ConfigSize) ? LineEnd + 1 : ConfigSize;
            continue;
          }

          ValueStart = KeyEnd + 1;
          ValueEnd = ContentEnd;
          TrimRange (Config, &KeyStart, &KeyEnd);
          if (KeyStart == KeyEnd) {
            *ValueSize = 0;
            return EFI_COMPROMISED_DATA;
          }

          TrimRange (Config, &ValueStart, &ValueEnd);

          CommentStart = ValueEnd;
          TrimInlineComment (Config, ValueStart, &CommentStart);
          ValueEnd = CommentStart;
          TrimRange (Config, &ValueStart, &ValueEnd);

          if (InRequestedSection &&
              RangeEqualsString (&Config[KeyStart], KeyEnd - KeyStart, Key)) {
            if (Found) {
              *ValueSize = 0;
              return EFI_COMPROMISED_DATA;
            }

            Found = TRUE;
            MatchedValueStart = ValueStart;
            MatchedValueEnd = ValueEnd;
          }
        }
      }
    }

    LineStart = (LineEnd < ConfigSize) ? LineEnd + 1 : ConfigSize;
  }

  if (!Found) {
    *ValueSize = 0;
    return EFI_NOT_FOUND;
  }

  RequiredSize = MatchedValueEnd - MatchedValueStart + 1;
  if (Capacity < RequiredSize) {
    *ValueSize = RequiredSize;
    return EFI_BUFFER_TOO_SMALL;
  }

  CopyMem (Value, &Config[MatchedValueStart], RequiredSize - 1);
  Value[RequiredSize - 1] = '\0';
  *ValueSize = RequiredSize;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetAndromedaBootConfigUint64 (
  IN  CONST CHAR8  *Section,
  IN  CONST CHAR8  *Key,
  OUT UINT64       *Value
  )
{
  EFI_STATUS  Status;
  UINTN       ValueSize;
  UINTN       Index;
  UINTN       Start;
  UINT8       Radix;
  UINT8       Digit;
  CHAR8       StringValue[32];
  UINT64      ParsedValue;

  if (Value == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  ValueSize = sizeof (StringValue);
  Status = GetAndromedaBootConfigValue (
             Section,
             Key,
             StringValue,
             &ValueSize
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Start = 0;
  Radix = 10;
  if ((StringValue[0] == '0') &&
      ((StringValue[1] == 'x') || (StringValue[1] == 'X'))) {
    Start = 2;
    Radix = 16;
  }

  if (StringValue[Start] == '\0') {
    return EFI_COMPROMISED_DATA;
  }

  ParsedValue = 0;
  for (Index = Start; StringValue[Index] != '\0'; Index++) {
    if ((StringValue[Index] >= '0') && (StringValue[Index] <= '9')) {
      Digit = (UINT8)(StringValue[Index] - '0');
    } else if ((Radix == 16) &&
               (StringValue[Index] >= 'a') &&
               (StringValue[Index] <= 'f')) {
      Digit = (UINT8)(StringValue[Index] - 'a' + 10);
    } else if ((Radix == 16) &&
               (StringValue[Index] >= 'A') &&
               (StringValue[Index] <= 'F')) {
      Digit = (UINT8)(StringValue[Index] - 'A' + 10);
    } else {
      return EFI_COMPROMISED_DATA;
    }

    if ((Digit >= Radix) ||
        (ParsedValue > (MAX_UINT64 - Digit) / Radix)) {
      return EFI_COMPROMISED_DATA;
    }

    ParsedValue = (ParsedValue * Radix) + Digit;
  }

  *Value = ParsedValue;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetAndromedaBootConfigUint32 (
  IN  CONST CHAR8  *Section,
  IN  CONST CHAR8  *Key,
  OUT UINT32       *Value
  )
{
  EFI_STATUS  Status;
  UINT64      ParsedValue;

  if (Value == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetAndromedaBootConfigUint64 (Section, Key, &ParsedValue);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (ParsedValue > MAX_UINT32) {
    return EFI_COMPROMISED_DATA;
  }

  *Value = (UINT32)ParsedValue;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
GetAndromedaBootConfigSectionEntry (
  IN      CONST CHAR8  *Section,
  IN      UINTN         EntryIndex,
  OUT     CHAR8        *Entry,
  IN OUT  UINTN        *EntrySize
  )
{
  CONST CHAR8  *Config;
  UINTN        ConfigSize;
  UINTN        Capacity;
  UINTN        LineStart;
  UINTN        LineEnd;
  UINTN        ContentStart;
  UINTN        ContentEnd;
  UINTN        SectionStart;
  UINTN        SectionEnd;
  UINTN        SectionClose;
  UINTN        CommentStart;
  UINTN        RequiredSize;
  UINTN        CurrentIndex;
  BOOLEAN      InRequestedSection;
  EFI_STATUS   Status;

  if ((Section == NULL) || (EntrySize == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Capacity = *EntrySize;
  if ((Capacity > 0) && (Entry == NULL)) {
    return EFI_INVALID_PARAMETER;
  }

  Status = GetBootConfig (&Config, &ConfigSize);
  if (EFI_ERROR (Status)) {
    *EntrySize = 0;
    return Status;
  }

  for (LineStart = 0; LineStart < ConfigSize; LineStart++) {
    if (Config[LineStart] == '\0') {
      *EntrySize = 0;
      return EFI_COMPROMISED_DATA;
    }
  }

  InRequestedSection = (BOOLEAN)(Section[0] == '\0');
  CurrentIndex = 0;
  LineStart = 0;
  while (LineStart < ConfigSize) {
    LineEnd = LineStart;
    while ((LineEnd < ConfigSize) && (Config[LineEnd] != '\n')) {
      LineEnd++;
    }

    ContentStart = LineStart;
    ContentEnd = LineEnd;
    TrimRange (Config, &ContentStart, &ContentEnd);

    if (ContentStart < ContentEnd) {
      if ((Config[ContentStart] != '#') && (Config[ContentStart] != ';')) {
        if (Config[ContentStart] == '[') {
          SectionStart = ContentStart + 1;
          SectionEnd = SectionStart;
          while ((SectionEnd < ContentEnd) && (Config[SectionEnd] != ']')) {
            SectionEnd++;
          }

          if (SectionEnd == ContentEnd) {
            *EntrySize = 0;
            return EFI_COMPROMISED_DATA;
          }

          SectionClose = SectionEnd;
          TrimRange (Config, &SectionStart, &SectionEnd);
          CommentStart = SectionClose + 1;
          while ((CommentStart < ContentEnd) && IsConfigSpace (Config[CommentStart])) {
            CommentStart++;
          }

          if ((CommentStart < ContentEnd) &&
              (Config[CommentStart] != '#') &&
              (Config[CommentStart] != ';')) {
            *EntrySize = 0;
            return EFI_COMPROMISED_DATA;
          }

          InRequestedSection = RangeEqualsString (
                                 &Config[SectionStart],
                                 SectionEnd - SectionStart,
                                 Section
                                 );
        } else if (InRequestedSection) {
          TrimInlineComment (Config, ContentStart, &ContentEnd);
          TrimRange (Config, &ContentStart, &ContentEnd);
          if (ContentStart < ContentEnd) {
            if (CurrentIndex == EntryIndex) {
              RequiredSize = ContentEnd - ContentStart + 1;
              if (Capacity < RequiredSize) {
                *EntrySize = RequiredSize;
                return EFI_BUFFER_TOO_SMALL;
              }

              CopyMem (Entry, &Config[ContentStart], RequiredSize - 1);
              Entry[RequiredSize - 1] = '\0';
              *EntrySize = RequiredSize;
              return EFI_SUCCESS;
            }

            CurrentIndex++;
          }
        }
      }
    }

    LineStart = (LineEnd < ConfigSize) ? LineEnd + 1 : ConfigSize;
  }

  *EntrySize = 0;
  return EFI_NOT_FOUND;
}
