/** @file
  Platform-independent polling menu.
  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>
#include <Library/PrePiMenuLib.h>
#include <Library/PrePiMenuButtonsLib.h>
#include <Library/PrePiMenuDisplayLib.h>
#include <Library/TimerLib.h>

#define font5x12 mPrePiMenuFont
#include <Resources/font5x12.h>

#define MENU_MAX_SCALE  6U

STATIC PRE_PI_MENU_SURFACE  mSurface;
STATIC UINTN                mScale;

STATIC
UINTN
StringLength (
  IN CONST CHAR8 *Text
  )
{
  UINTN Length;

  Length = 0;
  while (Text[Length] != '\0') {
    Length++;
  }

  return Length;
}

STATIC
UINTN
GetTextColumns (
  IN UINTN X, IN UINTN MaxX
  )
{
  UINTN AvailableWidth;
  UINTN GlyphWidth;
  UINTN CharacterWidth;

  GlyphWidth = FONT_WIDTH * mScale;
  CharacterWidth = (FONT_WIDTH + 1U) * mScale;
  AvailableWidth = (MaxX >= X) ? MaxX - X : 0U;
  if (AvailableWidth < GlyphWidth) {
    return 1U;
  }

  return ((AvailableWidth - GlyphWidth) / CharacterWidth) + 1U;
}

STATIC
UINTN
GetBadgeWidth (
  IN CONST CHAR8 *Text, IN UINTN MaxWidth, IN UINTN Padding
  )
{
  UINTN Columns;
  UINTN Length;
  UINTN TextWidth;

  if (Padding >= MaxWidth / 2U) {
    return MaxWidth;
  }

  Columns = GetTextColumns (Padding, MaxWidth - Padding);
  Length = StringLength (Text);
  if (Length > Columns) {
    Length = Columns;
  }

  if (Length == 0U) {
    return (FONT_WIDTH * mScale) + (2U * Padding);
  }

  TextWidth = ((Length - 1U) * (FONT_WIDTH + 1U) * mScale) +
              (FONT_WIDTH * mScale);
  if (TextWidth > MaxWidth - (2U * Padding)) {
    return MaxWidth;
  }

  return TextWidth + (2U * Padding);
}

STATIC
VOID
FillRect (
  IN UINTN X, IN UINTN Y, IN UINTN Width, IN UINTN Height, IN UINT32 Color
  )
{
  UINTN Row;
  UINTN Column;
  for (Row = Y; Row < Y + Height; Row++) {
    for (Column = X; Column < X + Width; Column++) {
      mSurface.Pixels[Row * mSurface.Stride + Column] = Color;
    }
  }
}

STATIC
VOID
DrawTextSpan (
  IN UINTN X, IN UINTN Y, IN UINTN MaxX,
  IN CONST CHAR8 *Text, IN UINTN Length, IN UINT32 Color
  )
{
  UINTN Row;
  UINTN Column;
  UINTN Index;
  UINTN RowsAvailable;
  UINTN GlyphWidth;
  UINT32 Bits;
  UINT8 Character;

  if ((Y >= mSurface.Height) || (mScale > mSurface.Height - Y)) {
    return;
  }

  RowsAvailable = (mSurface.Height - Y) / mScale;
  GlyphWidth = FONT_WIDTH * mScale;
  for (Index = 0; (Index < Length) && (Text[Index] != '\0') &&
       (X <= MaxX) && (GlyphWidth <= MaxX - X); Index++) {
    Character = (UINT8)Text[Index];
    if ((Character < 32) || (Character > 126)) {
      Character = '?';
    }

    for (Row = 0; (Row < FONT_HEIGHT) && (Row < RowsAvailable); Row++) {
      Bits = mPrePiMenuFont[(Character - 32U) * 2U + Row / 6U];
      Bits >>= (Row % 6U) * FONT_WIDTH;
      for (Column = 0; Column < FONT_WIDTH; Column++) {
        if ((Bits & (1U << Column)) != 0) {
          FillRect (X + Column * mScale, Y + Row * mScale,
                    mScale, mScale, Color);
        }
      }
    }

    if (Index + 1U < Length) {
      X += (FONT_WIDTH + 1U) * mScale;
    }
  }
}

STATIC
VOID
DrawText (
  IN UINTN X, IN UINTN Y, IN UINTN MaxX,
  IN CONST CHAR8 *Text, IN UINT32 Color
  )
{
  DrawTextSpan (X, Y, MaxX, Text, StringLength (Text), Color);
}

STATIC
UINTN
GetWrappedLineCount (
  IN CONST CHAR8 *Text, IN UINTN Columns
  )
{
  UINTN Length;

  Length = StringLength (Text);
  return (Length == 0U) ? 1U :
         (Length / Columns) + ((Length % Columns) != 0U);
}

STATIC
UINTN
DrawWrappedText (
  IN UINTN X, IN UINTN Y, IN UINTN MaxX,
  IN CONST CHAR8 *Text, IN UINTN Columns,
  IN UINTN LineHeight, IN UINTN MaxLines, IN UINT32 Color
  )
{
  UINTN Length;
  UINTN Lines;
  UINTN Offset;
  UINTN Line;
  UINTN SegmentLength;

  Length = StringLength (Text);
  Lines = GetWrappedLineCount (Text, Columns);
  if (Lines > MaxLines) {
    Lines = MaxLines;
  }

  Offset = 0;
  for (Line = 0; Line < Lines; Line++) {
    SegmentLength = (Length - Offset < Columns) ? Length - Offset : Columns;
    DrawTextSpan (X, Y + Line * LineHeight, MaxX,
                  Text + Offset, SegmentLength, Color);
    Offset += SegmentLength;
  }

  return Lines;
}

STATIC
VOID
DrawBadge (
  IN UINTN X, IN UINTN Y, IN UINTN Width, IN UINTN Height,
  IN UINTN Inset, IN CONST CHAR8 *Text,
  IN UINT32 Background, IN UINT32 Foreground
  )
{
  UINTN TextHeight;

  FillRect (X, Y, Width, Height, Background);
  TextHeight = FONT_HEIGHT * mScale;
  DrawText (X + Inset, Y + ((Height - TextHeight) / 2U),
            X + Width - Inset, Text, Foreground);
}

STATIC
VOID
DrawMenu (
  IN CONST CHAR8 *Title, IN CONST CHAR8 *CONST *Entries,
  IN UINTN Count, IN UINTN Selected
  )
{
  UINTN PanelWidth;
  UINTN Inset;
  UINTN Top;
  UINTN LineHeight;
  UINTN RowHeight;
  UINTN FooterHeight;
  UINTN TitleHeight;
  UINTN SectionY;
  UINTN TitleY;
  UINTN ListY;
  UINTN AvailableHeight;
  UINTN TitleColumns;
  UINTN EntryColumns;
  UINTN TitleLines;
  UINTN MaxTitleLines;
  UINTN MaxEntryLines;
  UINTN EntryLines;
  UINTN EntryHeight;
  UINTN UsedHeight;
  UINTN VisibleCount;
  UINTN First;
  UINTN Index;
  UINTN Y;
  UINTN FooterY;
  UINTN BadgeWidth;
  UINTN HeadingBadgeWidth;
  UINTN SectionBadgeWidth;
  UINTN EntryX;
  CONST CHAR8 *Heading;

  PanelWidth = mSurface.Width;
  Inset = PanelWidth / 16U;
  if (Inset < 8U) {
    Inset = 8U;
  }

  mScale = PanelWidth / 220U;
  if (mScale < 1U) {
    mScale = 1U;
  } else if (mScale > MENU_MAX_SCALE) {
    mScale = MENU_MAX_SCALE;
  }

  if (mScale > mSurface.Height / 240U) {
    mScale = mSurface.Height / 240U;
  }

  LineHeight = (FONT_HEIGHT + 4U) * mScale;
  RowHeight = LineHeight + (2U * mScale);
  FooterHeight = (2U * LineHeight) + (2U * mScale);
  TitleHeight = LineHeight + (2U * mScale);
  Top = mSurface.Height / 28U;
  BadgeWidth = PanelWidth - (2U * Inset);
  Heading = PrePiMenuDisplayGetHeading ();
  HeadingBadgeWidth = ((Heading == NULL) || (Heading[0] == '\0')) ?
                      0U : GetBadgeWidth (Heading, BadgeWidth, 2U * mScale);
  SectionBadgeWidth = GetBadgeWidth ("Boot Select", BadgeWidth, 2U * mScale);
  SectionY = Top + TitleHeight + (4U * mScale);
  TitleY = SectionY + TitleHeight + (4U * mScale);
  TitleColumns = GetTextColumns (Inset, mSurface.Width - Inset);
  MaxTitleLines = (mSurface.Height - TitleY - FooterHeight - RowHeight -
                   (10U * mScale)) / LineHeight;
  if (MaxTitleLines == 0U) {
    MaxTitleLines = 1U;
  }

  TitleLines = GetWrappedLineCount (Title, TitleColumns);
  if (TitleLines > MaxTitleLines) {
    TitleLines = MaxTitleLines;
  }

  EntryX = Inset + (2U * (FONT_WIDTH + 1U) * mScale);
  EntryColumns = GetTextColumns (EntryX, mSurface.Width - Inset);
  ListY = TitleY + (TitleLines * LineHeight) + (6U * mScale);
  AvailableHeight = mSurface.Height - ListY - FooterHeight - (2U * mScale);
  MaxEntryLines = (AvailableHeight - (2U * mScale)) / LineHeight;
  if (MaxEntryLines == 0U) {
    MaxEntryLines = 1U;
  }

  First = Selected;
  VisibleCount = 1U;
  EntryLines = GetWrappedLineCount (Entries[Selected], EntryColumns);
  if (EntryLines > MaxEntryLines) {
    EntryLines = MaxEntryLines;
  }

  UsedHeight = (EntryLines * LineHeight) + (2U * mScale);
  while (First > 0U) {
    EntryLines = GetWrappedLineCount (Entries[First - 1U], EntryColumns);
    if (EntryLines > MaxEntryLines) {
      EntryLines = MaxEntryLines;
    }

    EntryHeight = (EntryLines * LineHeight) + (2U * mScale);
    if (EntryHeight > AvailableHeight - UsedHeight) {
      break;
    }

    First--;
    VisibleCount++;
    UsedHeight += EntryHeight;
  }

  while ((First + VisibleCount < Count) &&
         (UsedHeight < AvailableHeight)) {
    EntryLines = GetWrappedLineCount (Entries[First + VisibleCount], EntryColumns);
    if (EntryLines > MaxEntryLines) {
      EntryLines = MaxEntryLines;
    }

    EntryHeight = (EntryLines * LineHeight) + (2U * mScale);
    if (EntryHeight > AvailableHeight - UsedHeight) {
      break;
    }

    VisibleCount++;
    UsedHeight += EntryHeight;
  }

  FillRect (0, 0, mSurface.Width, mSurface.Height, 0x00000000U);

  if (HeadingBadgeWidth != 0U) {
    FillRect (Inset + (4U * mScale), Top + (4U * mScale),
              HeadingBadgeWidth, TitleHeight, 0x008B84A6U);
    FillRect (Inset + (2U * mScale), Top + (2U * mScale),
              HeadingBadgeWidth, TitleHeight, 0x005D5685U);
    DrawBadge (Inset, Top, HeadingBadgeWidth, TitleHeight, 2U * mScale, Heading,
               0x001D2347U, 0x00FFFFFFU);
  }
  FillRect (Inset + (2U * mScale), SectionY + (2U * mScale),
            SectionBadgeWidth, TitleHeight, 0x000078B8U);
  DrawBadge (Inset, SectionY, SectionBadgeWidth, TitleHeight, 2U * mScale,
             "Boot Select", 0x0000AEEF, 0x00001830U);
  DrawWrappedText (Inset, TitleY, mSurface.Width - Inset, Title, TitleColumns,
                   LineHeight, TitleLines, 0x00E8E8ECU);

  Y = ListY;
  for (Index = First; Index < First + VisibleCount; Index++) {
    EntryLines = GetWrappedLineCount (Entries[Index], EntryColumns);
    if (EntryLines > MaxEntryLines) {
      EntryLines = MaxEntryLines;
    }

    if (Index == Selected) {
      DrawText (Inset, Y, mSurface.Width - Inset, "-", 0x0000CFFFU);
    }

    DrawWrappedText (EntryX, Y, mSurface.Width - Inset, Entries[Index],
                     EntryColumns, LineHeight, EntryLines,
                     (Index == Selected) ? 0x0000CFFFU : 0x00E8E8ECU);
    
    if (Index == Selected) {
      DrawText (EntryX + StringLength (Entries[Index]) * 
                (FONT_WIDTH + 1U) * mScale, Y,
                mSurface.Width - Inset, "-", 0x0000CFFFU);
    }
                    
    Y += (EntryLines * LineHeight) + (2U * mScale);
  }

  FooterY = ListY + UsedHeight + (2U * mScale);
  DrawText (Inset, FooterY, mSurface.Width - Inset,
            "Press Volume Up or Down button to change selection",
            0x00C5C5CEU);
  DrawText (Inset, FooterY + LineHeight, mSurface.Width - Inset,
            "Press Power button to select", 0x00C5C5CEU);
  PrePiMenuDisplayFlush (&mSurface);
}

EFI_STATUS
EFIAPI
PrePiMenuShow (
  IN CONST CHAR8 *Title, IN CONST CHAR8 *CONST *Entries,
  IN UINTN EntryCount, IN OUT UINTN *Selection
  )
{
  EFI_STATUS Status;
  UINTN Index;
  UINT32 Raw;
  UINT32 Previous;
  UINT32 Samples;
  BOOLEAN Armed;

  if ((Title == NULL) || (Entries == NULL) || (Selection == NULL) ||
      (EntryCount == 0) || (*Selection >= EntryCount)) {
    return EFI_INVALID_PARAMETER;
  }

  for (Index = 0; Index < EntryCount; Index++) {
    if (Entries[Index] == NULL) {
      return EFI_INVALID_PARAMETER;
    }
  }

  Status = PrePiMenuDisplayGetSurface (&mSurface);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  if ((mSurface.Pixels == NULL) || (mSurface.Width < 480) ||
      (mSurface.Height < 300) || (mSurface.Stride < mSurface.Width) ||
      (mSurface.Stride > MAX_UINTN / mSurface.Height / sizeof (UINT32))) {
    return EFI_UNSUPPORTED;
  }

  Status = PrePiMenuButtonsInitialize ();
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DrawMenu (Title, Entries, EntryCount, *Selection);
  Previous = MAX_UINT32;
  Samples = 0;
  Armed = FALSE;
  for (;;) {
    Status = PrePiMenuButtonsRead (&Raw);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    Samples = (Raw == Previous) ? (Samples < 2 ? Samples + 1 : 2) : 1;
    Previous = Raw;
    if (Samples >= 2) {
      if (Raw == 0) {
        Armed = TRUE;
      } else if (Armed) {
        Armed = FALSE;
        if (Raw == PRE_PI_MENU_BUTTON_SELECT) {
          return EFI_SUCCESS;
        }

        if ((Raw == PRE_PI_MENU_BUTTON_UP) && (*Selection > 0)) {
          --*Selection;
        } else if ((Raw == PRE_PI_MENU_BUTTON_DOWN) && (*Selection < EntryCount - 1)) {
          ++*Selection;
        }

        DrawMenu (Title, Entries, EntryCount, *Selection);
      }
    }

    MicroSecondDelay (20000);
  }
}
