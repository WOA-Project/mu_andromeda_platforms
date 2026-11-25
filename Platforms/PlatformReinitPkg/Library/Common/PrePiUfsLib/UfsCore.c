/** @file
  Polling-only Qualcomm UFS initialization and transport for SEC/PrePi.

  The controller/device reset, PHY calibration, link startup and device
  initialization follow the supplied Linux ufshcd/ufs-qcom initialization path.
  No DXE services or interrupts are used.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PrePiUfsLib.h>
#include <Library/TimerLib.h>

#ifdef PREPI_UFS_SM8350
#include <Library/ArmMmuLib.h>
#endif

#define UFS_BASE          0x01D84000
#define PHY_BASE          0x01D87000
#define GCC_BASE          0x00100000
#define UFS_FATAL_STATUS  (BIT11 | BIT16 | BIT17 | BIT18)
#define UFS_UIC_COMPLETE  BIT10
#define UFS_QUERY_SIZE    255

#ifdef PREPI_UFS_SM8350
STATIC CONST UINT32 mUfsClockBranches[] = {
  0x77010, 0x770CC, 0x77018, 0x7705C, 0x77064, 0x7709C
};
#else
STATIC CONST UINT32 mUfsClockBranches[] = {
  0x77010, 0x770C0, 0x77014, 0x77058, 0x7705C, 0x77090
};
#endif

#define UFS_TRY(Expression)             \
  do {                                \
    EFI_STATUS TryStatus = (Expression); \
    if (EFI_ERROR (TryStatus)) {       \
      return TryStatus;               \
    }                                 \
  } while (0)

typedef struct {
  UINT8   *Dma;
  UINT32  Revision;
  UINT32  UniProVersion;
  UINT32  CoreCyclesPerUs;
  UINT32  UniProCyclesPerUs;
  UINT16  DeviceVersion;
  UINT16  Manufacturer;
  CHAR8   Model[32];
  BOOLEAN Gear4;
} UFS_INIT_CONTEXT;

STATIC UINT8             *mInitializedDma;
STATIC UFS_INIT_CONTEXT  mIoContext;
STATIC BOOLEAN           mIoOpen;
STATIC BOOLEAN           mIoFailed;

STATIC
UINT64
ElapsedUs (
  UINT64 Start
  )
{
  return GetTimeInNanoSecond (GetPerformanceCounter () - Start) / 1000;
}

STATIC
EFI_STATUS
WaitRegister (
  UINTN  Address,
  UINT32 Mask,
  UINT32 Value,
  UINT32 TimeoutUs,
  UINT32 IntervalUs
  )
{
  UINT64 Start = GetPerformanceCounter ();

  do {
    if ((MmioRead32 (Address) & Mask) == Value) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (IntervalUs);
  } while (ElapsedUs (Start) < TimeoutUs);

  DEBUG ((DEBUG_ERROR, "UFS: timeout at 0x%lx, value 0x%x, mask 0x%x\n",
          (UINT64)Address, MmioRead32 (Address), Mask));
  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
UicCommand (
  UINT32 Command,
  UINT32 Argument1,
  UINT32 Argument3,
  UINT32 *Result
  )
{
  UFS_TRY (WaitRegister (UFS_BASE + 0x30, BIT3, BIT3, 500000, 10));
  MmioWrite32 (UFS_BASE + 0x20, UFS_UIC_COMPLETE);
  MmioWrite32 (UFS_BASE + 0x94, Argument1);
  MmioWrite32 (UFS_BASE + 0x98, 0);
  MmioWrite32 (UFS_BASE + 0x9C, Argument3);
  MemoryFence ();
  MmioWrite32 (UFS_BASE + 0x90, Command);
  UFS_TRY (WaitRegister (UFS_BASE + 0x20, UFS_UIC_COMPLETE,
                        UFS_UIC_COMPLETE, 500000, 10));
  MmioWrite32 (UFS_BASE + 0x20, UFS_UIC_COMPLETE);
  if ((MmioRead32 (UFS_BASE + 0x98) & 0xFF) != 0 ||
      (MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS: UIC command 0x%x, attribute 0x%x failed: 0x%x\n",
            Command, Argument1, MmioRead32 (UFS_BASE + 0x98)));
    return EFI_DEVICE_ERROR;
  }

  if (Result != NULL) {
    *Result = MmioRead32 (UFS_BASE + 0x9C);
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DmeGet (
  UINT16 Attribute,
  UINT16 Selector,
  BOOLEAN Peer,
  UINT32 *Value
  )
{
  return UicCommand (Peer ? 3 : 1,
                     ((UINT32)Attribute << 16) | Selector, 0, Value);
}

STATIC
EFI_STATUS
DmeSet (
  UINT16 Attribute,
  UINT32 Value
  )
{
  return UicCommand (2, (UINT32)Attribute << 16, Value, NULL);
}

STATIC
EFI_STATUS
DmeUpdate (
  UINT16 Attribute,
  UINT32 Mask,
  UINT32 Value
  )
{
  UINT32 Current;

  UFS_TRY (DmeGet (Attribute, 0, FALSE, &Current));
  return DmeSet (Attribute, (Current & ~Mask) | Value);
}

STATIC
EFI_STATUS
ClockRate (
  UINT32 Root,
  UINT32 *Rate
  )
{
  UINT32 Config;
  UINT32 Parent;
  UINT32 Divider;

  UFS_TRY (WaitRegister (GCC_BASE + Root, BIT0, 0, 1000, 1));
  Config = MmioRead32 (GCC_BASE + Root + 4);
  Divider = Config & 0x1F;

  switch ((Config >> 8) & 7) {
    case 1: Parent = 600000000; break;
    case 6: Parent = 300000000; break;
    default:
      DEBUG ((DEBUG_ERROR, "UFS: unsupported boot clock source at 0x%x: 0x%x\n",
              Root, Config));
      return EFI_UNSUPPORTED;
  }

  if ((Config & 0x3000) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS: unexpected fractional clock mode at 0x%x\n", Root));
    return EFI_UNSUPPORTED;
  }

  *Rate = (Divider == 0) ? Parent : (2 * Parent) / (Divider + 1);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
EnableClocks (
  UFS_INIT_CONTEXT *Context
  )
{
#ifdef PREPI_UFS_SM8350
  STATIC CONST UINT32 Roots[] = {0x77024, 0x77084, 0x7706C, 0x770A0};
#else
  STATIC CONST UINT32 Roots[] = {0x77020, 0x77078, 0x77060, 0x77094};
#endif
  UINTN Index;
  UINT32 CoreRate;
  UINT32 UniProRate;
  UINT32 AuxConfig;

  // Do not raise CX voltage or reprogram shared PLLs without an RPMh driver.
#ifdef PREPI_UFS_SM8350
  UFS_TRY (ClockRate (0x77024, &CoreRate));
  UFS_TRY (ClockRate (0x77084, &UniProRate));
  if ((CoreRate != 25000000 && CoreRate != 75000000 &&
       CoreRate != 150000000 && CoreRate != 300000000) ||
      (UniProRate != 150000000 && UniProRate != 300000000)) {
#else
  UFS_TRY (ClockRate (0x77020, &CoreRate));
  UFS_TRY (ClockRate (0x77078, &UniProRate));
  if ((CoreRate != 37500000 && CoreRate != 75000000 &&
       CoreRate != 150000000 && CoreRate != 300000000) ||
      (UniProRate != 150000000 && UniProRate != 300000000)) {
#endif
    DEBUG ((DEBUG_ERROR, "UFS: unsupported boot AXI/UniPro clocks %u/%u Hz\n",
            CoreRate, UniProRate));
    return EFI_UNSUPPORTED;
  }

  Context->CoreCyclesPerUs = CoreRate / 1000000;
  Context->UniProCyclesPerUs = UniProRate / 1000000;
  DEBUG ((DEBUG_INFO, "UFS: retaining AXI/UniPro clocks %u/%u Hz\n",
          CoreRate, UniProRate));

#ifdef PREPI_UFS_SM8350
  AuxConfig = MmioRead32 (GCC_BASE + 0x770A4);
#else
  AuxConfig = MmioRead32 (GCC_BASE + 0x77098);
#endif
  if ((AuxConfig & 0x3700) != 0 || (AuxConfig & 0x1F) > 1) {
    DEBUG ((DEBUG_ERROR, "UFS: PHY auxiliary clock is not 19.2 MHz XO\n"));
    return EFI_UNSUPPORTED;
  }

  UFS_TRY (WaitRegister (GCC_BASE + 0x77004, BIT31, BIT31, 1000, 1));
  for (Index = 0; Index < ARRAY_SIZE (Roots); Index++) {
    UFS_TRY (WaitRegister (GCC_BASE + Roots[Index], BIT0, 0, 1000, 1));
    MmioOr32 (GCC_BASE + Roots[Index], BIT1);
    UFS_TRY (WaitRegister (GCC_BASE + Roots[Index], BIT31, 0, 1000, 1));
  }

  for (Index = 0; Index < ARRAY_SIZE (mUfsClockBranches); Index++) {
#ifdef PREPI_UFS_SM8350
    MmioAndThenOr32 (GCC_BASE + mUfsClockBranches[Index],
                     ~(UINT32)BIT1, BIT0);
#else
    // Match Linux's memory/peripheral retention flags; retain ICE memory only.
    MmioAndThenOr32 (GCC_BASE + mUfsClockBranches[Index],
                     ~(UINT32)(BIT1 | BIT12 | BIT13 | BIT14),
                     BIT0 | ((mUfsClockBranches[Index] == 0x7705C) ? BIT14 : 0));
#endif
    UFS_TRY (WaitRegister (GCC_BASE + mUfsClockBranches[Index], BIT31, 0, 1000, 1));
  }

  MmioOr32 (GCC_BASE + 0x8C000, BIT0);
  UFS_TRY (WaitRegister (GCC_BASE + 0x8C000, BIT31, 0, 1000, 1));
  return EFI_SUCCESS;
}

STATIC
VOID
ResetControllerAndDevice (
  VOID
  )
{
  MmioOr32 (GCC_BASE + 0x77000, BIT0);
  MicroSecondDelay (200);
  MmioAnd32 (GCC_BASE + 0x77000, ~(UINT32)BIT0);

#ifdef PREPI_UFS_SM8350
  MicroSecondDelay (1000);
  MmioAndThenOr32 (0x0F1D8000, ~(UINT32)(BIT4 | BIT3), BIT3);
  MmioAndThenOr32 (0x0F1D8000, ~(UINT32)7, 3);
  MmioAnd32 (0x0F1D8004, ~(UINT32)BIT0);
  MicroSecondDelay (10);
  MmioOr32 (0x0F1D8004, BIT0);
  MicroSecondDelay (10);
#else
  // UFS_RESET is a dedicated TLMM pad, not a normal GPIO.
  MmioAndThenOr32 (0x03EB6000, ~(UINT32)(BIT4 | BIT3), BIT3);
  MmioAndThenOr32 (0x03EB6000, ~(UINT32)7, 3);
  MmioAnd32 (0x03EB6004, ~(UINT32)BIT0);
  MmioOr32 (0x03EB6000, BIT31);
  MicroSecondDelay (10);
  MmioOr32 (0x03EB6004, BIT0);
  MicroSecondDelay (10);
#endif
}

STATIC
VOID
WritePhyTable (
  CONST UFS_PHY_SETTING *Table,
#ifdef PREPI_UFS_SM8350
  UINTN Count,
  UINTN LaneOffset,
  BOOLEAN Gear4
#else
  UINTN Count
#endif
  )
{
  UINTN Index;

  for (Index = 0; Index < Count; Index++) {
#ifdef PREPI_UFS_SM8350
    UINT16 Value = Gear4 ? Table[Index].Gear4 : Table[Index].Gear3;
    if (Value != UFS_PHY_SKIP) {
      MmioWrite32 (PHY_BASE + Table[Index].Offset + LaneOffset, Value);
    }
#else
    MmioWrite32 (PHY_BASE + Table[Index].Offset, Table[Index].Value);
#endif
  }
}

STATIC
EFI_STATUS
EnableHost (
  UFS_INIT_CONTEXT *Context
  )
{
  UINT32 State;
  UINT64 Start;
  UINTN  Index;
#ifdef PREPI_UFS_SM8350
  STATIC CONST UINT32 LaneClocks[] = {0x7701C, 0x77020, 0x770B8};
  STATIC CONST UINT32 LaneMuxes[] = {0x77048, 0x77058, 0x770C8};
#else
  STATIC CONST UINT32 LaneClocks[] = {0x7701C, 0x77018, 0x770AC};
#endif

  MmioWrite32 (UFS_BASE + 0x24, 0);
  MmioWrite32 (UFS_BASE + 0x34, 0);
  UFS_TRY (WaitRegister (UFS_BASE + 0x34, BIT0, 0, 1000, 10));
  MmioWrite32 (UFS_BASE + 0x20, MmioRead32 (UFS_BASE + 0x20));

  MmioOr32 (UFS_BASE + 0xDC, BIT1);
  MicroSecondDelay (1000);
#ifdef PREPI_UFS_SM8350
  MmioAnd32 (PHY_BASE + 0x734, ~(UINT32)BIT5);
  MmioAnd32 (PHY_BASE + 0xB34, ~(UINT32)BIT5);
  MmioWrite32 (PHY_BASE + 0xC04, 1);
  MemoryFence ();
#endif
  MmioWrite32 (PHY_BASE + 0xC08, 1);
  MemoryFence ();
#ifdef PREPI_UFS_SM8350
  MmioWrite32 (PHY_BASE + 0xC04, 1);
  WritePhyTable (mPhyCommon, ARRAY_SIZE (mPhyCommon), 0, Context->Gear4);
  WritePhyTable (mPhyLane, ARRAY_SIZE (mPhyLane), 0, Context->Gear4);
  WritePhyTable (mPhyLane, ARRAY_SIZE (mPhyLane), 0x400, Context->Gear4);
  WritePhyTable (mPhyPcs, ARRAY_SIZE (mPhyPcs), 0, Context->Gear4);
  MmioWrite32 (PHY_BASE + 0xDE0, 2);
  if (!Context->Gear4) {
    MmioWrite32 (PHY_BASE + 0x10C, 6);
  }
#else
  WritePhyTable (mPhyRateA, ARRAY_SIZE (mPhyRateA));
  if (Context->Gear4) {
    WritePhyTable (mPhyGear4, ARRAY_SIZE (mPhyGear4));
  } else {
    WritePhyTable (mPhyGear3, ARRAY_SIZE (mPhyGear3));
  }

  if (Context->Revision == 0x40010000) {
    if (Context->Gear4) {
      WritePhyTable (mPhyV2Gear4, ARRAY_SIZE (mPhyV2Gear4));
    } else {
      WritePhyTable (mPhyV2Gear3, ARRAY_SIZE (mPhyV2Gear3));
    }
  }

  WritePhyTable (mPhyLane1, ARRAY_SIZE (mPhyLane1));
  if (Context->Revision == 0x40010000) {
    if (Context->Gear4) {
      WritePhyTable (mPhyLane1V2Gear4, ARRAY_SIZE (mPhyLane1V2Gear4));
    } else {
      WritePhyTable (mPhyLane1V2Gear3, ARRAY_SIZE (mPhyLane1V2Gear3));
    }
  }

  if (!Context->Gear4) {
    MmioWrite32 (PHY_BASE + 0x10C, 6);
  }

  if (Context->Revision == 0x40000000) {
    MmioWrite32 (PHY_BASE + 0x668, 1);
    MmioWrite32 (PHY_BASE + 0x678, 1);
    MmioWrite32 (PHY_BASE + 0xA68, 1);
    MmioWrite32 (PHY_BASE + 0xA78, 1);
  }
#endif

  MmioWrite32 (PHY_BASE + 0xC08, 0);
  MemoryFence ();
  MmioAnd32 (UFS_BASE + 0xDC, ~(UINT32)BIT1);
  MicroSecondDelay (1000);
  MmioOr32 (PHY_BASE + 0xC00, BIT0);
  UFS_TRY (WaitRegister (PHY_BASE + 0xD80, BIT0, BIT0, 1000000, 10));
  MmioOr32 (UFS_BASE + 0xDC, BIT0);

  for (Index = 0; Index < ARRAY_SIZE (LaneClocks); Index++) {
#ifdef PREPI_UFS_SM8350
    MmioAnd32 (GCC_BASE + LaneMuxes[Index], ~(UINT32)3);
#endif
    MmioOr32 (GCC_BASE + LaneClocks[Index], BIT0);
    MicroSecondDelay (500);
  }

  MmioWrite32 (UFS_BASE + 0x34, 1);
  MicroSecondDelay (1000);
  UFS_TRY (WaitRegister (UFS_BASE + 0x34, BIT0, BIT0, 50000, 5000));
  Start = GetPerformanceCounter ();
  do {
    UFS_TRY (DmeGet (0x41, 0, FALSE, &State));
    if (State == 1) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (100);
  } while (ElapsedUs (Start) < 100000);

  DEBUG ((DEBUG_ERROR, "UFS: PHY did not enter Hibern8, TX FSM 0x%x\n", State));
  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
StartLink (
  UFS_INIT_CONTEXT *Context
  )
{
  EFI_STATUS Status;
  UINT32 Value;
  UINTN Attempt;
  UINTN Index;

  for (Attempt = 0; Attempt < 4; Attempt++) {
    MmioWrite32 (UFS_BASE + 0xC0, Context->CoreCyclesPerUs);
    MmioAnd32 (PHY_BASE + 0xD48, ~(UINT32)BIT1);
    // Preserve Linux's v4 DME core-clock encoding and divider handling.
    UFS_TRY (DmeGet (0xD002, 0, FALSE, &Value));
    Value = ((Value & ~0xFFFU) | Context->UniProCyclesPerUs) << 16;
    UFS_TRY (DmeSet (0xD002, Value & ~(UINT32)BIT8));
    UFS_TRY (DmeUpdate (0x9007, 0xF, Context->UniProCyclesPerUs / 25));
    MmioOr32 (UFS_BASE + 0xE0, 0xFF);
    UFS_TRY (DmeUpdate (0xA00B, 0x3FF, 0x3FF));
    UFS_TRY (DmeUpdate (0x9004, 0x1FF, 0x1FF));
    if (Context->Revision != 0x40000000) {
      UFS_TRY (DmeUpdate (0xD002, BIT9, BIT9));
    }

    UFS_TRY (DmeGet (0x15A9, 0, FALSE, &Context->UniProVersion));
    Context->UniProVersion &= 0xF;
    if (Context->UniProVersion != 2) {
      UFS_TRY (DmeSet (0x155E, 0));
    }

    UFS_TRY (DmeUpdate (0x9000, 0x6000, 0x6000));
    Status = UicCommand (0x16, 0, 0, NULL);
    if (!EFI_ERROR (Status)) {
      if ((MmioRead32 (UFS_BASE + 0x30) & BIT0) == 0) {
        DEBUG ((DEBUG_ERROR, "UFS: link startup completed without a device\n"));
        return EFI_NOT_FOUND;
      }

      UFS_TRY (DmeGet (0x1561, 0, FALSE, &Value));
      if (Value == 0 || Value > 2) {
        DEBUG ((DEBUG_ERROR, "UFS: invalid connected TX lanes %u\n", Value));
        return EFI_DEVICE_ERROR;
      }

      if (Context->UniProVersion == 2) {
        MmioOr32 (PHY_BASE + 0xD48, BIT1);
      }

      if ((MmioRead32 (UFS_BASE) & BIT23) != 0) {
        for (Index = 0; Index < ARRAY_SIZE (mUfsClockBranches); Index++) {
          MmioOr32 (GCC_BASE + mUfsClockBranches[Index], BIT1);
        }

        MmioOr32 (UFS_BASE + 0xFC, 0x3C);
      }

      return EFI_SUCCESS;
    }

    DEBUG ((DEBUG_WARN, "UFS: retrying link startup (%u): %r\n",
            (UINT32)Attempt + 1, Status));
    UFS_TRY (EnableHost (Context));
  }

  return Status;
}

STATIC
EFI_STATUS
MapDma (
  UFS_INIT_CONTEXT *Context
  )
{
  MmioWrite32 (UFS_BASE + 0x4C, 0);
  MmioWrite32 (UFS_BASE + 0x50, (UINT32)(UINTN)Context->Dma);
  MmioWrite32 (UFS_BASE + 0x54, (UINT32)((UINT64)(UINTN)Context->Dma >> 32));
  MmioWrite32 (UFS_BASE + 0x70, (UINT32)(UINTN)(Context->Dma + 1024));
  MmioWrite32 (UFS_BASE + 0x74, (UINT32)((UINT64)(UINTN)(Context->Dma + 1024) >> 32));
  UFS_TRY (WaitRegister (UFS_BASE + 0x30, BIT1 | BIT2 | BIT3,
                        BIT1 | BIT2 | BIT3, 100000, 10));
  MemoryFence ();
  MmioWrite32 (UFS_BASE + 0x80, 1);
  MmioWrite32 (UFS_BASE + 0x60, 1);
  return EFI_SUCCESS;
}

STATIC
UINT16
ReadBe16 (
  CONST UINT8 *Buffer
  )
{
  return (UINT16)(((UINT16)Buffer[0] << 8) | Buffer[1]);
}

STATIC
UINT32
ReadBe32 (
  CONST UINT8 *Buffer
  )
{
  return ((UINT32)Buffer[0] << 24) | ((UINT32)Buffer[1] << 16) |
         ((UINT32)Buffer[2] << 8) | Buffer[3];
}

STATIC
VOID
WriteBe32 (
  UINT8 *Buffer,
  UINT32 Value
  )
{
  Buffer[0] = (UINT8)(Value >> 24);
  Buffer[1] = (UINT8)(Value >> 16);
  Buffer[2] = (UINT8)(Value >> 8);
  Buffer[3] = (UINT8)Value;
}

STATIC
EFI_STATUS
DeviceCommand (
  UFS_INIT_CONTEXT *Context,
  UINT8 Opcode,
  UINT8 Idn,
  UINT8 Index,
  UINT32 *Value,
  UINT8 *Descriptor
  )
{
  UINT32 *Trd = (UINT32 *)Context->Dma;
  UINT8 *Request = Context->Dma + 2048;
  UINT8 *Response = Context->Dma + 2560;
  UINT64 Start;
  UINT32 TimeoutUs = (Opcode == 0) ? 30000 : 1500000;
  UINTN Length;

  ZeroMem (Context->Dma, EFI_PAGE_SIZE);
  // UFSHCI 2.0+: storage command type, no PRDT, one management request.
  Trd[0] = 0x11000000;
  Trd[2] = 0xF;
  Trd[4] = (UINT32)(UINTN)Request;
  Trd[5] = (UINT32)((UINT64)(UINTN)Request >> 32);
  Trd[6] = (128U << 16) | 128;
  Trd[7] = 256U << 16;
  if (Opcode != 0) {
    Request[0] = 0x16;
    Request[5] = (Opcode == 4 || Opcode == 6 || Opcode == 7) ? 0x81 : 1;
    Request[12] = Opcode;
    Request[13] = Idn;
    Request[14] = Index;
    if (Opcode == 1) {
      Request[19] = UFS_QUERY_SIZE;
    }

    if (Opcode == 4) {
      WriteBe32 (Request + 20, *Value);
    }
  }

  WriteBackInvalidateDataCacheRange (Context->Dma, EFI_PAGE_SIZE);
  MmioWrite32 (UFS_BASE + 0x20, BIT0);
  MemoryFence ();
  MmioWrite32 (UFS_BASE + 0x58, 1);
  Start = GetPerformanceCounter ();
  while ((MmioRead32 (UFS_BASE + 0x58) & BIT0) != 0) {
    if ((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) {
      DEBUG ((DEBUG_ERROR, "UFS: fatal transfer status 0x%x\n",
              MmioRead32 (UFS_BASE + 0x20)));
      return EFI_DEVICE_ERROR;
    }

    if (ElapsedUs (Start) >= TimeoutUs) {
      MmioWrite32 (UFS_BASE + 0x5C, ~(UINT32)BIT0);
      UFS_TRY (WaitRegister (UFS_BASE + 0x58, BIT0, 0, 1000000, 10));
      DEBUG ((DEBUG_ERROR, "UFS: device command 0x%x/0x%x timed out\n", Opcode, Idn));
      return EFI_NOT_READY;
    }

    MicroSecondDelay (10);
  }

  MemoryFence ();
  InvalidateDataCacheRange (Context->Dma, EFI_PAGE_SIZE);
  if ((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0 ||
      (Trd[2] & 0xF) != 0 || Response[3] != 0 ||
      Response[0] != ((Opcode == 0) ? 0x20 : 0x36)) {
    DEBUG ((DEBUG_ERROR, "UFS: invalid response, OCS 0x%x, type 0x%x, tag 0x%x\n",
            Trd[2], Response[0], Response[3]));
    return EFI_DEVICE_ERROR;
  }

  if (Opcode != 0) {
    if ((Opcode == 1) && (Idn == 2) && (Response[6] == 0xFC) &&
        (Response[12] == Opcode) && (Response[13] == Idn) &&
        (Response[14] == Index) && (Response[15] == 0)) {
      return EFI_NOT_FOUND;
    }

    if (Response[6] != 0 || Response[12] != Opcode ||
        Response[13] != Idn || Response[14] != Index || Response[15] != 0) {
      DEBUG ((DEBUG_ERROR, "UFS: query 0x%x/0x%x rejected, result 0x%x\n",
              Opcode, Idn, Response[6]));
      return EFI_DEVICE_ERROR;
    }

    if (Value != NULL) {
      *Value = ReadBe32 (Response + 20);
    }

    if (Descriptor != NULL) {
      Length = ReadBe16 (Response + 18);
      if (Length < 2 || Length > UFS_QUERY_SIZE ||
          ReadBe16 (Response + 10) != Length ||
          Response[32] != Length || Response[33] != Idn ||
          (Idn == 5 && (Length & 1) != 0)) {
        DEBUG ((DEBUG_ERROR, "UFS: malformed descriptor response, length %u\n",
                (UINT32)Length));
        return EFI_DEVICE_ERROR;
      }

      ZeroMem (Descriptor, UFS_QUERY_SIZE);
      CopyMem (Descriptor, Response + 32, Length);
    }
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
Query (
  UFS_INIT_CONTEXT *Context,
  UINT8 Opcode,
  UINT8 Idn,
  UINT8 Index,
  UINT32 *Value,
  UINT8 *Descriptor
  )
{
  EFI_STATUS Status;
  UINTN Attempt;

  for (Attempt = 0; Attempt < 3; Attempt++) {
    Status = DeviceCommand (Context, Opcode, Idn, Index, Value, Descriptor);
    if (!EFI_ERROR (Status)) {
      return Status;
    }

    if (Status == EFI_NOT_FOUND) {
      return Status;
    }

    if ((MmioRead32 (UFS_BASE + 0x58) & BIT0) != 0 ||
        (MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) {
      break;
    }
  }

  return Status;
}

STATIC
EFI_STATUS
InitializeDevice (
  UFS_INIT_CONTEXT *Context
  )
{
  UINT8 Descriptor[UFS_QUERY_SIZE];
  EFI_STATUS Status;
  UINTN Attempt;
  UINTN Index;
  UINT32 Flag;
  UINT64 Start;

  for (Attempt = 0; Attempt < 10; Attempt++) {
    Status = DeviceCommand (Context, 0, 0, 0, NULL, NULL);
    if (!EFI_ERROR (Status) || Status == EFI_TIMEOUT ||
        (MmioRead32 (UFS_BASE + 0x58) & BIT0) != 0 ||
        (MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) {
      break;
    }
  }

  UFS_TRY (Status);
  UFS_TRY (Query (Context, 6, 1, 0, NULL, NULL));
  Start = GetPerformanceCounter ();
  Index = 0;
  do {
    UFS_TRY (Query (Context, 5, 1, 0, &Flag, NULL));
    if ((Flag & 1) == 0) {
      break;
    }

    if (Index++ >= 1000) {
      MicroSecondDelay (20000);
    }
  } while (ElapsedUs (Start) < 5000000);

  if ((Flag & 1) != 0) {
    DEBUG ((DEBUG_ERROR, "UFS: device did not clear fDeviceInit within 5 seconds\n"));
    return EFI_TIMEOUT;
  }

  UFS_TRY (Query (Context, 1, 0, 0, NULL, Descriptor));
  if (Descriptor[0] < 0x1A) {
    DEBUG ((DEBUG_ERROR, "UFS: device descriptor is too short\n"));
    return EFI_DEVICE_ERROR;
  }

  Context->DeviceVersion = ReadBe16 (Descriptor + 0x10);
  Context->Manufacturer = ReadBe16 (Descriptor + 0x18);
  ZeroMem (Context->Model, sizeof (Context->Model));
  UFS_TRY (Query (Context, 1, 5, Descriptor[0x15], NULL, Descriptor));
  for (Index = 0; Index < sizeof (Context->Model) - 1 &&
                  3 + 2 * Index < Descriptor[0]; Index++) {
    Context->Model[Index] = (Descriptor[2 + 2 * Index] == 0) ?
                           (CHAR8)Descriptor[3 + 2 * Index] : '?';
  }

  DEBUG ((DEBUG_INFO, "UFS: device version 0x%x, manufacturer 0x%x, model %a\n",
          Context->DeviceVersion, Context->Manufacturer, Context->Model));
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
TuneDevice (
  UFS_INIT_CONTEXT *Context
  )
{
  STATIC CONST UINT32 GranularityUs[] = {1, 4, 8, 16, 32, 100};
  UINT32 HostGranularity;
  UINT32 PeerGranularity;
  UINT32 HostActivate;
  UINT32 PeerActivate;
  UINT32 Local;
  UINT32 Peer;

  if (Context->UniProVersion < 3) {
    UFS_TRY (DmeGet (0x8F, 4, TRUE, &Peer));
    UFS_TRY (DmeSet (0x15A8, Peer * 10));
    UFS_TRY (DmeGet (0xF, 0, FALSE, &Local));
    UFS_TRY (DmeGet (0x92, 4, TRUE, &Peer));
    UFS_TRY (DmeSet (0x15A7, MAX (Local, Peer)));
  }

  if (Context->Manufacturer == 0x198 &&
      (AsciiStrnCmp (Context->Model, "THGLF2G9C8KBADG", 15) == 0 ||
       AsciiStrnCmp (Context->Model, "THGLF2G9D8KBADG", 15) == 0)) {
    UFS_TRY (DmeSet (0x15A8, 10));
  }

  if (Context->Manufacturer == 0x1CE) {
    UFS_TRY (DmeGet (0x15AA, 0, FALSE, &HostGranularity));
    UFS_TRY (DmeGet (0x15AA, 0, TRUE, &PeerGranularity));
    if (HostGranularity < 1 || HostGranularity > 6 ||
        PeerGranularity < 1 || PeerGranularity > 6) {
      DEBUG ((DEBUG_ERROR, "UFS: invalid PA granularity %u/%u\n",
              HostGranularity, PeerGranularity));
      return EFI_DEVICE_ERROR;
    }

    UFS_TRY (DmeGet (0x15A8, 0, FALSE, &HostActivate));
    UFS_TRY (DmeGet (0x15A8, 0, TRUE, &PeerActivate));
    HostActivate *= GranularityUs[HostGranularity - 1];
    PeerActivate *= GranularityUs[PeerGranularity - 1];
    if (HostActivate > PeerActivate) {
      UFS_TRY (UicCommand (4, 0x15A80000,
                           HostActivate / GranularityUs[PeerGranularity - 1] + 1,
                           NULL));
    }
  }

  if (Context->Manufacturer == 0x1AD) {
    UFS_TRY (DmeUpdate (0x9000, BIT12, BIT12));
  }

  UFS_TRY (Query (Context, 3, 0xD, 0, &Local, NULL));
  Local |= BIT2;
  UFS_TRY (Query (Context, 4, 0xD, 0, &Local, NULL));
  UFS_TRY (Query (Context, 7, 4, 0, NULL, NULL));
  UFS_TRY (Query (Context, 3, 0xA, 0, &Local, NULL));
  if (Local > 3) {
    DEBUG ((DEBUG_ERROR, "UFS: invalid device reference clock code %u\n", Local));
    return EFI_DEVICE_ERROR;
  }

  if (Local != 0) {
    Local = 0;
    UFS_TRY (Query (Context, 4, 0xA, 0, &Local, NULL));
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetPowerMode (
  UFS_INIT_CONTEXT *Context
  )
{
  UINT32 RxLanes;
  UINT32 TxLanes;
  UINT32 RxGear;
  UINT32 TxGear;
  UINT32 Adapt;
  UINT32 Gear;

  UFS_TRY (DmeGet (0x1581, 0, FALSE, &RxLanes));
  UFS_TRY (DmeGet (0x1561, 0, FALSE, &TxLanes));
  UFS_TRY (DmeGet (0x1587, 0, FALSE, &RxGear));
  UFS_TRY (DmeGet (0x1587, 0, TRUE, &TxGear));
  if (RxLanes == 0 || TxLanes == 0 || RxGear == 0 || TxGear == 0) {
    DEBUG ((DEBUG_ERROR, "UFS: invalid HS capabilities lanes %u/%u, gears %u/%u\n",
            RxLanes, TxLanes, RxGear, TxGear));
    return EFI_UNSUPPORTED;
  }

  Gear = MIN (4U, MIN (RxGear, TxGear));
  RxLanes = MIN (2U, RxLanes);
  TxLanes = MIN (2U, TxLanes);
  MmioOr32 (UFS_BASE + 0xDC, BIT26);
  MicroSecondDelay ((Context->Manufacturer == 0x1AD) ? 50 : 1);
  UFS_TRY (DmeSet (0x1583, Gear));
  UFS_TRY (DmeSet (0x1580, RxLanes));
  UFS_TRY (DmeSet (0x1584, 1));
  UFS_TRY (DmeSet (0x1568, Gear));
  UFS_TRY (DmeSet (0x1560, TxLanes));
  UFS_TRY (DmeSet (0x1569, 1));
  UFS_TRY (DmeSet (0x156A, Context->Gear4 ? 1 : 2));
  if (Gear == 4) {
    UFS_TRY (DmeGet (0x9F, 4, TRUE, &Adapt));
    UFS_TRY (DmeSet (0x15D3, Adapt));
    UFS_TRY (DmeSet (0x15D4, 1));
  } else if (MmioRead32 (UFS_BASE + 8) >= 0x300) {
    UFS_TRY (DmeSet (0x15D4, 3));
  }

  UFS_TRY (DmeSet (0x15B0, 8191));
  UFS_TRY (DmeSet (0x15B1, 65535));
  UFS_TRY (DmeSet (0x15B2, 32767));
  UFS_TRY (DmeSet (0xD041, 8191));
  UFS_TRY (DmeSet (0xD042, 65535));
  UFS_TRY (DmeSet (0xD043, 32767));
  MmioWrite32 (UFS_BASE + 0x20, BIT4);
  UFS_TRY (DmeSet (0x1571, 0x11));
  UFS_TRY (WaitRegister (UFS_BASE + 0x20, BIT4, BIT4, 500000, 10));
  MmioWrite32 (UFS_BASE + 0x20, BIT4);
  if (((MmioRead32 (UFS_BASE + 0x30) >> 8) & 7) != 1) {
    DEBUG ((DEBUG_ERROR, "UFS: power-mode change failed, HCS 0x%x\n",
            MmioRead32 (UFS_BASE + 0x30)));
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((DEBUG_INFO, "UFS: HS-G%u rate %a, RX/TX lanes %u/%u\n",
          Gear, Context->Gear4 ? "A" : "B", RxLanes, TxLanes));
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
SetActiveCurrent (
  UFS_INIT_CONTEXT *Context
  )
{
  UINT8 Descriptor[UFS_QUERY_SIZE];
#ifdef PREPI_UFS_SM8350
  STATIC CONST UINT32 MaxCurrentUa[] = {800000, 900000};
#else
  STATIC CONST UINT32 MaxCurrentUa[] = {750000, 700000, 750000};
#endif
  UINT32 Level = 15;
  UINT32 Current;
  UINT16 Entry;
  UINTN Rail;

  UFS_TRY (Query (Context, 1, 8, 0, NULL, Descriptor));
  if (Descriptor[0] < 0x62) {
    DEBUG ((DEBUG_ERROR, "UFS: power descriptor is too short\n"));
    return EFI_DEVICE_ERROR;
  }

  for (Rail = 0; Rail < ARRAY_SIZE (MaxCurrentUa); Rail++) {
    if (Rail == 1 && Context->DeviceVersion < 0x300 &&
                     (Context->Manufacturer == 0x1CE ||
                      Context->Manufacturer == 0x1AD)) {
      continue;
    }

    for (;;) {
      Entry = ReadBe16 (Descriptor + 2 + Rail * 32 + Level * 2);
      Current = Entry & 0x3FF;
      switch (Entry >> 14) {
        case 0: Current /= 1000; break;
        case 2: Current *= 1000; break;
        case 3: Current *= 1000000; break;
        default: break;
      }

      if (Current <= MaxCurrentUa[Rail]) {
        break;
      }

      if (Level == 0) {
        DEBUG ((DEBUG_ERROR, "UFS: no supported active current level for rail %u\n",
                (UINT32)Rail));
        return EFI_UNSUPPORTED;
      }

      Level--;
    }
  }

  return Query (Context, 4, 3, 0, &Level, NULL);
}

STATIC
EFI_STATUS
StopDma (
  UFS_INIT_CONTEXT *Context
  )
{
  EFI_STATUS Status;

  MmioWrite32 (UFS_BASE + 0x24, 0);
  MmioWrite32 (UFS_BASE + 0x60, 0);
  MmioWrite32 (UFS_BASE + 0x80, 0);
  Status = WaitRegister (UFS_BASE + 0x60, BIT0, 0, 1000000, 10);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  UFS_TRY (WaitRegister (UFS_BASE + 0x80, BIT0, 0, 1000000, 10));
  UFS_TRY (WaitRegister (UFS_BASE + 0x58, MAX_UINT32, 0, 1000000, 10));
  UFS_TRY (WaitRegister (UFS_BASE + 0x78, MAX_UINT32, 0, 1000000, 10));
  MmioWrite32 (UFS_BASE + 0x50, 0);
  MmioWrite32 (UFS_BASE + 0x54, 0);
  MmioWrite32 (UFS_BASE + 0x70, 0);
  MmioWrite32 (UFS_BASE + 0x74, 0);
  MemoryFence ();

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
AllocateDmaPage (
  UFS_INIT_CONTEXT *Context
  )
{
  if (mInitializedDma != NULL) {
    Context->Dma = mInitializedDma;
    return EFI_SUCCESS;
  }

  Context->Dma = AllocatePages (1);
  if (Context->Dma == NULL) {
    DEBUG ((DEBUG_ERROR, "UFS: cannot allocate a DMA page\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  if ((MmioRead32 (UFS_BASE) & BIT24) == 0 &&
      (UINT64)(UINTN)Context->Dma + EFI_PAGE_SIZE > 0x100000000ULL) {
    DEBUG ((DEBUG_ERROR, "UFS: DMA page exceeds the controller's address width\n"));
    return EFI_UNSUPPORTED;
  }

  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
InitializeUfs (
  BOOLEAN Reinitialize
  )
{
  UFS_INIT_CONTEXT Context;
  EFI_STATUS Status;
  EFI_STATUS StopStatus;
  CONST CHAR8 *Stage = "clocks";
  UINTN Pass;

  if (mIoOpen) {
    DEBUG ((DEBUG_ERROR, "UFS: cannot initialize while a session is open\n"));
    return EFI_ACCESS_DENIED;
  }

  ZeroMem (&Context, sizeof (Context));
#ifdef PREPI_UFS_SM8350
  Status = ArmSetMemoryAttributes (UFS_BASE, 4 * EFI_PAGE_SIZE,
                                   EFI_MEMORY_UC | EFI_MEMORY_XP, 0);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS: cannot map Lahaina controller registers: %r\n", Status));
    return Status;
  }

  if (Reinitialize) {
    Status = ArmSetMemoryAttributes (0x00177000, EFI_PAGE_SIZE,
                                     EFI_MEMORY_UC | EFI_MEMORY_XP, 0);
    if (!EFI_ERROR (Status)) {
      Status = ArmSetMemoryAttributes (0x0018C000, EFI_PAGE_SIZE,
                                       EFI_MEMORY_UC | EFI_MEMORY_XP, 0);
    }
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "UFS: cannot map Lahaina clock registers: %r\n", Status));
      return Status;
    }
  }
#endif
  DEBUG ((DEBUG_INFO,
          "UFS: inherited HCE 0x%x HCS 0x%x IS 0x%x revision 0x%x\n",
          MmioRead32 (UFS_BASE + 0x34), MmioRead32 (UFS_BASE + 0x30),
          MmioRead32 (UFS_BASE + 0x20), MmioRead32 (UFS_BASE + 0xE4)));
  if (!Reinitialize) {
    if (((MmioRead32 (UFS_BASE + 0x34) & BIT0) == 0) ||
        ((MmioRead32 (UFS_BASE + 0x30) & BIT0) == 0) ||
        ((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0)) {
      DEBUG ((DEBUG_ERROR, "UFS: inherited controller or device is not ready\n"));
      return EFI_NOT_READY;
    }

    UFS_TRY (WaitRegister (UFS_BASE + 0x58, MAX_UINT32, 0, 1000000, 10));
    UFS_TRY (WaitRegister (UFS_BASE + 0x78, MAX_UINT32, 0, 1000000, 10));
    UFS_TRY (StopDma (&Context));
    MmioWrite32 (UFS_BASE + 0x4C, 0);
    MmioWrite32 (UFS_BASE + 0x20, BIT0);
    UFS_TRY (AllocateDmaPage (&Context));
    mInitializedDma = Context.Dma;
    DEBUG ((DEBUG_INFO, "PrePi: using inherited UFS link without reinitialization\n"));
    return EFI_SUCCESS;
  }

  Status = EnableClocks (&Context);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  Context.Revision = MmioRead32 (UFS_BASE + 0xE4);
#ifdef PREPI_UFS_SM8350
  if ((Context.Revision >> 28) < 4 || (Context.Revision >> 28) > 5) {
    DEBUG ((DEBUG_ERROR, "UFS: unsupported SM8350 controller revision 0x%x\n",
            Context.Revision));
#else
  if (Context.Revision != 0x40000000 && Context.Revision != 0x40010000) {
    DEBUG ((DEBUG_ERROR, "UFS: unsupported SM8150 controller revision 0x%x\n",
            Context.Revision));
#endif
    return EFI_UNSUPPORTED;
  }

  UFS_TRY (AllocateDmaPage (&Context));

#ifdef PREPI_UFS_SM8350
  DEBUG ((DEBUG_INFO, "PrePi: initializing SM8350 UFS (revision 0x%x)\n",
          Context.Revision));
#else
  DEBUG ((DEBUG_INFO, "PrePi: initializing SM8150 UFS (revision 0x%x)\n",
          Context.Revision));
#endif
  for (Pass = 0; Pass < 2; Pass++) {
    DEBUG ((DEBUG_INFO, "UFS: starting %a PHY and link initialization\n",
            Context.Gear4 ? "G4 rate-A" : "G3 rate-B"));
    ResetControllerAndDevice ();
    Stage = "PHY/host enable";
    Status = EnableHost (&Context);
    if (EFI_ERROR (Status)) {
      break;
    }

    Stage = "link startup";
    Status = StartLink (&Context);
    if (EFI_ERROR (Status)) {
      break;
    }

    Stage = "DMA setup";
    Status = MapDma (&Context);
    if (EFI_ERROR (Status)) {
      break;
    }

    Stage = "device initialization";
    Status = InitializeDevice (&Context);
    if (EFI_ERROR (Status) || Context.DeviceVersion < 0x300 || Context.Gear4) {
      break;
    }

    // Linux repeats reset/probe with G4 rate-A calibration for UFS 3.x.
    Stage = "G4 reinitialization cleanup";
    Status = StopDma (&Context);
    if (EFI_ERROR (Status)) {
      break;
    }

    Context.Gear4 = TRUE;
  }

  if (!EFI_ERROR (Status)) {
    Stage = "device tuning";
    Status = TuneDevice (&Context);
  }

  if (!EFI_ERROR (Status)) {
    Stage = "power-mode negotiation";
    Status = SetPowerMode (&Context);
  }

  if (!EFI_ERROR (Status)) {
    Stage = "active current";
    Status = SetActiveCurrent (&Context);
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS: %a failed: %r\n", Stage, Status));
    // Stop the controller before removing DMA mappings on an error path.
    MmioWrite32 (UFS_BASE + 0x34, 0);
    StopStatus = WaitRegister (UFS_BASE + 0x34, BIT0, 0, 1000000, 10);
    if (EFI_ERROR (StopStatus)) {
      return StopStatus;
    }
  }

  StopStatus = StopDma (&Context);
  if (EFI_ERROR (StopStatus)) {
    DEBUG ((DEBUG_ERROR, "UFS: DMA teardown failed: %r\n", StopStatus));
    return StopStatus;
  }

  if (!EFI_ERROR (Status)) {
    MmioWrite32 (UFS_BASE + 0x20, BIT0);
    mInitializedDma = Context.Dma;
    DEBUG ((DEBUG_INFO, "PrePi: UFS initialization complete; DMA stopped for DXE\n"));
  }

  return Status;
}

EFI_STATUS
PlatformPrepareInheritedUfs (
  VOID
  )
{
  return InitializeUfs (FALSE);
}

EFI_STATUS
PlatformReinitializeUfs (
  VOID
  )
{
  return InitializeUfs (TRUE);
}

EFI_STATUS
PrePiUfsTransportOpen (
  VOID
  )
{
  EFI_STATUS  Status;
  EFI_STATUS  CleanupStatus;

  if (mIoOpen) {
    return EFI_ALREADY_STARTED;
  }

  if ((mInitializedDma == NULL) || ((MmioRead32 (UFS_BASE + 0x34) & BIT0) == 0)) {
    return EFI_NOT_READY;
  }

  ZeroMem (&mIoContext, sizeof (mIoContext));
  mIoContext.Dma = mInitializedDma;
  Status = WaitRegister (UFS_BASE + 0x58, MAX_UINT32, 0, 1000000, 10);
  if (!EFI_ERROR (Status)) {
    Status = WaitRegister (UFS_BASE + 0x78, MAX_UINT32, 0, 1000000, 10);
  }
  if (!EFI_ERROR (Status)) {
    Status = StopDma (&mIoContext);
  }
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS: request lists cannot be quiesced: %r\n", Status));
    CpuDeadLoop ();
  }

  Status = MapDma (&mIoContext);
  if (EFI_ERROR (Status)) {
    CleanupStatus = StopDma (&mIoContext);
    if (EFI_ERROR (CleanupStatus)) {
      DEBUG ((DEBUG_ERROR, "UFS: cannot clean up failed I/O session: %r\n", CleanupStatus));
      CpuDeadLoop ();
    }

    return Status;
  }

  mIoOpen = TRUE;
  mIoFailed = FALSE;
  return EFI_SUCCESS;
}

EFI_STATUS
PrePiUfsTransportClose (
  VOID
  )
{
  EFI_STATUS  Status;
  BOOLEAN     IoFailed;

  if (!mIoOpen) {
    return EFI_NOT_STARTED;
  }

  IoFailed = mIoFailed;
  if (IoFailed) {
    MmioWrite32 (UFS_BASE + 0x34, 0);
    Status = WaitRegister (UFS_BASE + 0x34, BIT0, 0, 1000000, 10);
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_ERROR, "UFS: cannot stop controller after failed I/O: %r\n", Status));
      return Status;
    }
  }

  Status = StopDma (&mIoContext);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "UFS: cannot close I/O session safely: %r\n", Status));
    return Status;
  }

  mIoOpen = FALSE;
  mIoFailed = FALSE;
  MmioWrite32 (UFS_BASE + 0x20, BIT0);
  if (IoFailed) {
    DEBUG ((DEBUG_ERROR, "UFS: failed I/O left the controller disabled\n"));
    return EFI_DEVICE_ERROR;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PlatformFinishUfs (
  VOID
  )
{
  EFI_STATUS Status;

  if (mIoOpen) {
    Status = PrePiUfsTransportClose ();
  } else {
    Status = StopDma (&mIoContext);
  }

  if (EFI_ERROR (Status)) {
    return Status;
  }

  if (((MmioRead32 (UFS_BASE + 0x34) & BIT0) == 0) ||
      ((MmioRead32 (UFS_BASE + 0x30) & BIT0) == 0) ||
      ((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0)) {
    DEBUG ((DEBUG_ERROR, "UFS: controller or device is not ready for DXE\n"));
    return EFI_NOT_READY;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
PrePiUfsTransportDescriptor (
  IN UINT8   Idn,
  IN UINT8   Index,
  OUT UINT8  *Descriptor
  )
{
  EFI_STATUS  Status;

  if (!mIoOpen) {
    return EFI_NOT_STARTED;
  }

  if (mIoFailed) {
    return EFI_DEVICE_ERROR;
  }

  if (Descriptor == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  Status = Query (&mIoContext, 1, Idn, Index, NULL, Descriptor);
  if (EFI_ERROR (Status) && (Status != EFI_NOT_FOUND)) {
    mIoFailed = TRUE;
  }

  return Status;
}

EFI_STATUS
PrePiUfsTransportReadBlock (
  IN UINT8   Lun,
  IN UINT64  Lba,
  IN UINT32  BlockSize,
  OUT VOID   *DmaBuffer
  )
{
  UINT32      *Trd;
  UINT32      *Prd;
  UINT8       *Request;
  UINT8       *Response;
  UINT64      Address;
  UINT64      Start;
  EFI_STATUS  Status;
  UINTN       Index;

  if (!mIoOpen) {
    return EFI_NOT_STARTED;
  }

  if (mIoFailed) {
    return EFI_DEVICE_ERROR;
  }

  if ((DmaBuffer == NULL) || (((UINTN)DmaBuffer & 3U) != 0) ||
      (BlockSize < 4) || (BlockSize > 0x40000U) || ((BlockSize & 3U) != 0)) {
    return EFI_INVALID_PARAMETER;
  }

  Address = (UINT64)(UINTN)DmaBuffer;
  if (((MmioRead32 (UFS_BASE) & BIT24) == 0) &&
      ((Address > MAX_UINT32) || (BlockSize - 1U > MAX_UINT32 - Address))) {
    return EFI_UNSUPPORTED;
  }

  Trd = (UINT32 *)mIoContext.Dma;
  // Do not reuse the command page or bounce buffer after a failed transfer.
  mIoFailed = TRUE;
  Request = mIoContext.Dma + 2048;
  Response = mIoContext.Dma + 2560;
  Prd = (UINT32 *)(mIoContext.Dma + 3072);
  ZeroMem (mIoContext.Dma, EFI_PAGE_SIZE);
  Trd[0] = 0x15000000U;
  Trd[2] = 0xFU;
  Trd[4] = (UINT32)(UINTN)Request;
  Trd[5] = (UINT32)((UINT64)(UINTN)Request >> 32);
  Trd[6] = (128U << 16) | 128U;
  Trd[7] = (256U << 16) | 1U;
  Request[0] = 0x01;
  Request[1] = 0x41;
  Request[2] = Lun;
  WriteBe32 (Request + 12, BlockSize);
  Request[16] = 0x88;
  for (Index = 0; Index < 8; Index++) {
    Request[18 + Index] = (UINT8)(Lba >> (56U - 8U * Index));
  }

  Request[29] = 1;
  Prd[0] = (UINT32)Address;
  Prd[1] = (UINT32)(Address >> 32);
  Prd[3] = BlockSize - 1U;
  WriteBackInvalidateDataCacheRange (DmaBuffer, BlockSize);
  WriteBackInvalidateDataCacheRange (mIoContext.Dma, EFI_PAGE_SIZE);
  MmioWrite32 (UFS_BASE + 0x20, BIT0);
  MemoryFence ();
  MmioWrite32 (UFS_BASE + 0x58, 1);
  Start = GetPerformanceCounter ();
  while ((MmioRead32 (UFS_BASE + 0x58) & BIT0) != 0) {
    if ((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) {
      DEBUG ((DEBUG_ERROR, "UFS: fatal read transfer error\n"));
      return EFI_DEVICE_ERROR;
    }

    if (ElapsedUs (Start) >= 3000000U) {
      MmioWrite32 (UFS_BASE + 0x5C, ~(UINT32)BIT0);
      Status = WaitRegister (UFS_BASE + 0x58, BIT0, 0, 1000000, 10);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "UFS: timed-out read remains in flight\n"));
        return Status;
      }

      DEBUG ((DEBUG_ERROR, "UFS: read timed out\n"));
      return EFI_TIMEOUT;
    }

    MicroSecondDelay (10);
  }

  MemoryFence ();
  InvalidateDataCacheRange (mIoContext.Dma, EFI_PAGE_SIZE);
  InvalidateDataCacheRange (DmaBuffer, BlockSize);
  if (((MmioRead32 (UFS_BASE + 0x20) & UFS_FATAL_STATUS) != 0) ||
      ((Trd[2] & 0xFFU) != 0) || (Response[0] != 0x21) ||
      (Response[2] != Lun) || (Response[3] != 0) || (Response[6] != 0) ||
      (Response[7] != 0) || ((Response[1] & (BIT5 | BIT6)) != 0) ||
      (ReadBe32 (Response + 12) != 0)) {
    DEBUG ((DEBUG_ERROR, "UFS: read failed, OCS 0x%x response 0x%x status 0x%x\n",
            Trd[2] & 0xFFU, Response[6], Response[7]));
    return EFI_DEVICE_ERROR;
  }

  mIoFailed = FALSE;
  return EFI_SUCCESS;
}
