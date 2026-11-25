/** @file
  SM8150 display diagnostic capture, before PrePi disables DSI or detaches SMMU.

  SPDX-License-Identifier: MIT
**/
#include <PiPei.h>

#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PrePiDisplayDiagnosticLib.h>
#include <Library/QcomUefiCfgMemoryMapHelperLib.h>

#define DISP_CC_BASE          0x0AF00000
#define MDSS_BASE             0x0AE00000
#define GCC_DISP_AHB_CBCR     0x0010B00C
#define XBL_DISPLAY_BASE      0x9C000000
#define MDSS_CORE_GDSCR       (DISP_CC_BASE + 0x3000)
#define MDSS_AHB_CBCR         (DISP_CC_BASE + 0x2080)
#define CLOCK_RUNNING(Value)  (((Value) & (BIT31 | BIT2 | BIT0)) == BIT0)

STATIC DISPLAY_RESUME_DIAGNOSTIC mDisplayDiagnostic;

STATIC UINT32 CaptureRegister(UINTN Address, BOOLEAN Accessible)
{
  DISPLAY_DIAGNOSTIC_REGISTER *Register;

  if (mDisplayDiagnostic.RegisterCount == DISPLAY_DIAGNOSTIC_MAX_REGISTERS) {
    mDisplayDiagnostic.CaptureStatus = EFI_BUFFER_TOO_SMALL;
    return 0;
  }

  Register = &mDisplayDiagnostic.Registers[mDisplayDiagnostic.RegisterCount++];
  Register->Address = Address;
  Register->Read = Accessible;
  if (Accessible) {
    Register->Value = MmioRead32(Address);
  }
  return Register->Value;
}

STATIC VOID CaptureRegisters(
    UINTN Base, CONST UINT32 *Offsets, UINTN Count, BOOLEAN Accessible)
{
  UINTN Index;

  for (Index = 0; Index < Count; Index++) {
    CaptureRegister(Base + Offsets[Index], Accessible);
  }
}

VOID RecordDisplayDsiControl(UINTN Address, UINT32 Value)
{
  UINTN Index;

  for (Index = 0; Index < mDisplayDiagnostic.RegisterCount; Index++) {
    if (mDisplayDiagnostic.Registers[Index].Address == Address) {
      mDisplayDiagnostic.Registers[Index].Value = Value;
      mDisplayDiagnostic.Registers[Index].Read = TRUE;
      WriteBackDataCacheRange(&mDisplayDiagnostic, sizeof(mDisplayDiagnostic));
      return;
    }
  }

  if (!EFI_ERROR(mDisplayDiagnostic.CaptureStatus)) {
    mDisplayDiagnostic.CaptureStatus = EFI_NOT_FOUND;
  }
  WriteBackDataCacheRange(&mDisplayDiagnostic, sizeof(mDisplayDiagnostic));
}

STATIC VOID CaptureMemorySamples(
    volatile CONST UINT32 *Words, UINT64 WordCount, DISPLAY_MEMORY_SAMPLES *Samples)
{
  UINT32 Index;
  UINT32 Value;
  UINT32 Previous;
  UINT64 LastWord;
  UINT64 Offset;

  LastWord = WordCount - 1;
  Previous = 0;
  for (Index = 0; Index < DISPLAY_FRAMEBUFFER_SAMPLES; Index++) {
    Offset = (LastWord / (DISPLAY_FRAMEBUFFER_SAMPLES - 1)) * Index +
             (LastWord % (DISPLAY_FRAMEBUFFER_SAMPLES - 1)) * Index /
             (DISPLAY_FRAMEBUFFER_SAMPLES - 1);
    Value = Words[Offset];
    if (Value != 0) {
      Samples->NonzeroSamples++;
    }
    if (Index != 0 && Value != Previous) {
      Samples->Transitions++;
    }
    if (Index == 0) {
      Samples->FirstValue = Value;
    }
    Samples->Checksum = (Samples->Checksum * 33) ^ Value;
    Previous = Value;
  }
  Samples->LastValue = Previous;
  Samples->SampleCount = DISPLAY_FRAMEBUFFER_SAMPLES;
}

STATIC VOID CaptureXblDisplaySummary(VOID)
{
  UINTN Index;
  UINTN Address;

  // The Epsilon XBL uefiplat.cfg reservation, not the UEFI display reservation.
  mDisplayDiagnostic.XblDisplayAddress = XBL_DISPLAY_BASE;
  for (Index = 0; Index < DISPLAY_XBL_CHUNKS; Index++) {
    Address = XBL_DISPLAY_BASE + Index * DISPLAY_XBL_CHUNK_SIZE;
    CaptureMemorySamples(
        (volatile CONST UINT32 *)Address,
        DISPLAY_XBL_CHUNK_SIZE / sizeof(UINT32),
        &mDisplayDiagnostic.XblDisplaySamples[Index]);
  }
}

STATIC VOID CaptureFramebufferSummary(VOID)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX Region;
  UINT64 Bytes;

  mDisplayDiagnostic.FramebufferWidth = FixedPcdGet32(PcdMipiFrameBufferWidth);
  mDisplayDiagnostic.FramebufferHeight = FixedPcdGet32(PcdMipiFrameBufferHeight);
  mDisplayDiagnostic.FramebufferBitsPerPixel = FixedPcdGet32(PcdMipiFrameBufferPixelBpp);
  mDisplayDiagnostic.FramebufferStatus =
      QcomLocateMemoryMapAreaByName("Display Reserved", &Region);
  if (EFI_ERROR(mDisplayDiagnostic.FramebufferStatus)) {
    return;
  }
  mDisplayDiagnostic.FramebufferAddress = Region.Address;
  mDisplayDiagnostic.FramebufferRegionSize = Region.Length;
  Bytes = (UINT64)mDisplayDiagnostic.FramebufferWidth *
          mDisplayDiagnostic.FramebufferHeight;
  if (mDisplayDiagnostic.FramebufferBitsPerPixel != 32 ||
      Bytes == 0 || Bytes > Region.Length / sizeof(UINT32) ||
      (Region.Address & (sizeof(UINT32) - 1)) != 0) {
    mDisplayDiagnostic.FramebufferStatus = EFI_UNSUPPORTED;
    return;
  }

  CaptureMemorySamples(
      (volatile CONST UINT32 *)(UINTN)Region.Address, Bytes,
      &mDisplayDiagnostic.FramebufferSamples);
}

VOID CaptureDisplayDiagnostic(VOID)
{
  STATIC CONST UINT32 DispCcOffsets[] = {
    0x0000, 0x0004, 0x0008, 0x000C, 0x0010,
    0x2004, 0x2008, 0x200C, 0x2014, 0x2024,
    0x2028, 0x202C, 0x2030, 0x2034, 0x2038, 0x203C,
    0x2098, 0x209C, 0x20A0, 0x20A4, 0x20A8,
    0x20B0, 0x20B4, 0x20B8, 0x20BC, 0x20C0,
    0x20C8, 0x20CC, 0x20E0, 0x20E4, 0x20F8, 0x20FC,
    0x2110, 0x2114, 0x2128, 0x212C, 0x2130, 0x2144,
    0x2148, 0x214C, 0x2160, 0x2164, 0x22BC, 0x22C0
  };
  STATIC CONST UINT32 DsiOffsets[] = {
    0x000, 0x004, 0x008, 0x010, 0x03C, 0x040, 0x044,
    0x058, 0x05C, 0x068, 0x080, 0x084, 0x0A8, 0x0AC,
    0x0BC, 0x0CC, 0x110, 0x114, 0x118, 0x11C, 0x120,
    0x12C, 0x194, 0x1F8
  };
  STATIC CONST UINT32 PhyOffsets[] = {
    0x010, 0x014, 0x018, 0x01C, 0x020, 0x024, 0x028,
    0x02C, 0x030, 0x034, 0x03C, 0x060, 0x064, 0x068,
    0x06C, 0x070, 0x074, 0x078, 0x07C, 0x080, 0x084,
    0x088, 0x08C, 0x090, 0x094, 0x098,
    0x500 + 0x0E0, 0x500 + 0x0E4, 0x500 + 0x0E8,
    0x500 + 0x0EC, 0x500 + 0x154, 0x500 + 0x1B0
  };
  STATIC CONST UINT32 CtlOffsets[] = {
    0x000, 0x004, 0x008, 0x014, 0x018, 0x030,
    0x0E8, 0x0F4, 0x104, 0x110, 0x134
  };
  STATIC CONST UINT32 SsppOffsets[] = {
    0x000, 0x00C, 0x014, 0x024, 0x030, 0x034, 0x038
  };
  STATIC CONST UINT32 InterfaceOffsets[] = {
    0x000, 0x004, 0x25C, 0x280, 0x284, 0x288,
    0x28C, 0x290, 0x2A0, 0x2A4, 0x2AC
  };
  STATIC CONST UINT32 DscOffsets[] = {
    0x000, 0x004, 0x008, 0x00C, 0x010, 0x014, 0x018
  };
  STATIC CONST UINT32 Gpios[] = {5, 6, 8, 9};
  STATIC CONST UINT32 Tiles[] = {
    0x03900000, 0x03D00000, 0x03500000, 0x03100000
  };
  UINT32 GccAhb;
  UINT32 Gdsc;
  UINT32 MdssAhb;
  BOOLEAN DispCcAccessible;
  BOOLEAN MdssAccessible;
  UINTN Index;
  UINTN Tile;
  UINTN Base;

  ZeroMem(&mDisplayDiagnostic, sizeof(mDisplayDiagnostic));
  mDisplayDiagnostic.Signature = DISPLAY_DIAGNOSTIC_SIGNATURE;
  mDisplayDiagnostic.Version = DISPLAY_DIAGNOSTIC_VERSION;

  CaptureXblDisplaySummary();

  GccAhb = CaptureRegister(GCC_DISP_AHB_CBCR, TRUE);
  CaptureRegister(0x0010B000, TRUE);
  CaptureRegister(0x0010B038, TRUE);
  CaptureRegister(0x0010B03C, TRUE);
  CaptureRegister(0x00105058, TRUE);
  CaptureRegister(0x0010505C, TRUE);
  DispCcAccessible = CLOCK_RUNNING(GccAhb);
  Gdsc = CaptureRegister(MDSS_CORE_GDSCR, DispCcAccessible);
  MdssAhb = CaptureRegister(MDSS_AHB_CBCR, DispCcAccessible);
  CaptureRegisters(
      DISP_CC_BASE, DispCcOffsets, ARRAY_SIZE(DispCcOffsets), DispCcAccessible);

  // Avoid MDSS while collapse/reset, clock disable or I/O clamp is asserted.
  MdssAccessible = DispCcAccessible && CLOCK_RUNNING(MdssAhb) &&
                   ((Gdsc & (BIT31 | BIT5 | BIT4 | BIT3 | BIT0)) == BIT31);
  mDisplayDiagnostic.MdssAccessible = MdssAccessible;
  CaptureRegister(MDSS_BASE, MdssAccessible);

  for (Index = 0; Index < 2; Index++) {
    Base = MDSS_BASE + 0x94000 + Index * 0x2000;
    CaptureRegisters(Base, DsiOffsets, ARRAY_SIZE(DsiOffsets), MdssAccessible);
    CaptureRegisters(
        Base + 0x400, PhyOffsets, ARRAY_SIZE(PhyOffsets), MdssAccessible);
    CaptureRegisters(
        MDSS_BASE + 0x2000 + Index * 0x200,
        CtlOffsets, ARRAY_SIZE(CtlOffsets), MdssAccessible);
    CaptureRegisters(
        MDSS_BASE + 0x6B800 + Index * 0x800,
        InterfaceOffsets, ARRAY_SIZE(InterfaceOffsets), MdssAccessible);
    CaptureRegisters(
        MDSS_BASE + 0x81000 + Index * 0x400,
        DscOffsets, ARRAY_SIZE(DscOffsets), MdssAccessible);
  }

  for (Index = 0; Index < 4; Index++) {
    CaptureRegisters(
        MDSS_BASE + 0x5000 + Index * 0x2000,
        SsppOffsets, ARRAY_SIZE(SsppOffsets), MdssAccessible);
    CaptureRegisters(
        MDSS_BASE + 0x25000 + Index * 0x2000,
        SsppOffsets, ARRAY_SIZE(SsppOffsets), MdssAccessible);
  }

  for (Index = 0; Index < ARRAY_SIZE(Gpios); Index++) {
    for (Tile = 0; Tile < ARRAY_SIZE(Tiles); Tile++) {
      Base = Tiles[Tile] + Gpios[Index] * 0x1000;
      // Match the original TLMM driver's GPIO_ID_STATUS tile discovery.
      if ((CaptureRegister(Base + 0x10, TRUE) & BIT0) != 0) {
        CaptureRegister(Base, TRUE);
        CaptureRegister(Base + 4, TRUE);
        break;
      }
    }
    if (Tile == ARRAY_SIZE(Tiles)) {
      if (!EFI_ERROR(mDisplayDiagnostic.CaptureStatus)) {
        mDisplayDiagnostic.CaptureStatus = EFI_NOT_FOUND;
      }
    }
  }

  CaptureFramebufferSummary();

  // CEntryPoint later invalidates the firmware region after disabling caches.
  WriteBackDataCacheRange(&mDisplayDiagnostic, sizeof(mDisplayDiagnostic));
}

CONST DISPLAY_RESUME_DIAGNOSTIC *GetDisplayDiagnostic(VOID)
{
  return &mDisplayDiagnostic;
}
