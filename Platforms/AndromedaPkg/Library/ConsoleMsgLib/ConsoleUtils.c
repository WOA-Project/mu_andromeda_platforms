/**@file Console Message Library

Copyright (C) Microsoft Corporation. All rights reserved.
SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <Uefi.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/MsUiThemeLib.h>
#include <Library/PcdLib.h>
#include <Library/PrintLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/GraphicsOutput.h>
#include <Protocol/HiiFont.h>

GLOBAL_REMOVE_IF_UNREFERENCED EFI_GRAPHICS_OUTPUT_BLT_PIXEL
    mConsoleEfiColors[16] = {
        {0x00, 0x00, 0x00, 0x00}, {0x98, 0x00, 0x00, 0x00},
        {0x00, 0x98, 0x00, 0x00}, {0x98, 0x98, 0x00, 0x00},
        {0x00, 0x00, 0x98, 0x00}, {0x98, 0x00, 0x98, 0x00},
        {0x00, 0x98, 0x98, 0x00}, {0x98, 0x98, 0x98, 0x00},
        {0x10, 0x10, 0x10, 0x00}, {0xff, 0x10, 0x10, 0x00},
        {0x10, 0xff, 0x10, 0x00}, {0xff, 0xff, 0x10, 0x00},
        {0x10, 0x10, 0xff, 0x00}, {0xf0, 0x10, 0xff, 0x00},
        {0x10, 0xff, 0xff, 0x00}, {0xff, 0xff, 0xff, 0x00}};

VOID ConsoleInternalPrintGraphic(
    IN UINTN PointX, IN UINTN PointY, IN CHAR16 *Buffer)
{
  EFI_STATUS                       Status;
  UINT32                           HorizontalResolution;
  UINT32                           VerticalResolution;
  EFI_HII_FONT_PROTOCOL           *HiiFont;
  EFI_IMAGE_OUTPUT                *Blt;
  EFI_FONT_DISPLAY_INFO            FontInfo;
  EFI_GRAPHICS_OUTPUT_PROTOCOL    *GraphicsOutput;
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *Sto;
  EFI_HANDLE                       ConsoleHandle;

  HorizontalResolution = 0;
  VerticalResolution   = 0;
  Blt                  = NULL;

  ConsoleHandle = gST->ConsoleOutHandle;

  ASSERT(ConsoleHandle != NULL);

  Status = gBS->HandleProtocol(
      ConsoleHandle, &gEfiGraphicsOutputProtocolGuid, (VOID **)&GraphicsOutput);

  if (EFI_ERROR(Status)) {
    goto Error;
  }

  Status = gBS->HandleProtocol(
      ConsoleHandle, &gEfiSimpleTextOutProtocolGuid, (VOID **)&Sto);

  if (EFI_ERROR(Status)) {
    goto Error;
  }

  if (GraphicsOutput != NULL) {
    HorizontalResolution = GraphicsOutput->Mode->Info->HorizontalResolution;
    VerticalResolution   = GraphicsOutput->Mode->Info->VerticalResolution;
  }
  else {
    goto Error;
  }

  ASSERT((HorizontalResolution != 0) && (VerticalResolution != 0));

  Status =
      gBS->LocateProtocol(&gEfiHiiFontProtocolGuid, NULL, (VOID **)&HiiFont);
  if (EFI_ERROR(Status)) {
    goto Error;
  }

  Blt = (EFI_IMAGE_OUTPUT *)AllocateZeroPool(sizeof(EFI_IMAGE_OUTPUT));
  if (Blt == NULL) {
    ASSERT(Blt != NULL);
    goto Error;
  }

  Blt->Width  = (UINT16)(HorizontalResolution);
  Blt->Height = (UINT16)(VerticalResolution);

  ZeroMem(&FontInfo, sizeof(EFI_FONT_DISPLAY_INFO));

  CopyMem(
      &FontInfo.ForegroundColor,
      &mConsoleEfiColors[Sto->Mode->Attribute & 0x0f],
      sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  CopyMem(
      &FontInfo.BackgroundColor, &mConsoleEfiColors[Sto->Mode->Attribute >> 4],
      sizeof(EFI_GRAPHICS_OUTPUT_BLT_PIXEL));

  FontInfo.FontInfoMask       = EFI_FONT_INFO_ANY_FONT;
  FontInfo.FontInfo.FontSize  = MsUiGetFixedFontHeight();
  FontInfo.FontInfo.FontStyle = EFI_HII_FONT_STYLE_NORMAL;

  if (GraphicsOutput != NULL) {
    Blt->Image.Screen = GraphicsOutput;

    Status = HiiFont->StringToImage(
        HiiFont,
        EFI_HII_IGNORE_IF_NO_GLYPH | EFI_HII_OUT_FLAG_CLIP |
            EFI_HII_OUT_FLAG_CLIP_CLEAN_X | EFI_HII_OUT_FLAG_CLIP_CLEAN_Y |
            EFI_HII_IGNORE_LINE_BREAK | EFI_HII_DIRECT_TO_SCREEN,
        Buffer, &FontInfo, &Blt, PointX, PointY, NULL, NULL, NULL);
    if (EFI_ERROR(Status)) {
      goto Error;
    }
  }
  else {
    goto Error;
  }

  FreePool(Blt);
  return;

Error:
  if (Blt != NULL) {
    FreePool(Blt);
  }

  return;
}

UINTN LineCounter = 0;

VOID EFIAPI ConsolePrint(IN CONST CHAR16 *Format, ...)
{
  VA_LIST Marker;
  CHAR16 *Buffer;
  UINTN   BufferSize;

  ASSERT(Format != NULL);
  ASSERT(((UINTN)Format & BIT0) == 0);

  VA_START(Marker, Format);

  BufferSize = (PcdGet32(PcdUefiLibMaxPrintBufferSize) + 1) * sizeof(CHAR16);

  Buffer = (CHAR16 *)AllocatePool(BufferSize);
  if (Buffer == NULL) {
    ASSERT(Buffer != NULL);
    return;
  }

  UnicodeVSPrint(Buffer, BufferSize, Format, Marker);

  VA_END(Marker);

  ConsoleInternalPrintGraphic(0, LineCounter, Buffer);
  LineCounter += MsUiGetFixedFontHeight();

  FreePool(Buffer);

  return;
}