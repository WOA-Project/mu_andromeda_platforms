#include "RightDisplayDxe.h"

#include <Library/FrameBufferBltLib.h>
#include <Library/FrameBufferSerialPortLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformHwioLib.h>
#include <Library/UefiLib.h>

#include <Protocol/DevicePath.h>
#include <Protocol/GraphicsOutput.h>

//
// Diagnostic DCS read: short read packet with the BTA flag set, sent in LP.
// Header flags: last (BIT31), BTA (BIT29). Returns the first readback word and
// the response data type, plus the controller error/ack status.
//
EFI_STATUS
DsiCtrlReadDcs (
  IN  UINT8   Command,
  OUT UINT32  *Data,
  OUT UINT32  *RdbkType,
  OUT UINT32  *AckErr
  )
{
  UINT32 i;
  UINT32 Done;

  MmioWrite32 (DSI (DSI_INT_CTRL), DSI_DMA_DONE | BIT20);
  MmioWrite32 (DSI (DSI_TPG_CTRL), BIT1 | BIT2 | (3 << 16));
  MmioWrite32 (DSI (DSI_TPG_DMA_INIT_VAL), BIT31 | BIT29 | (0x06 << 16) | Command);
  MmioWrite32 (DSI (DSI_TPG_DMA_INIT_VAL), 0);
  MmioWrite32 (DSI (DSI_DMA_CTRL), BIT28 | BIT26);
  MmioWrite32 (DSI (DSI_DMA_CMD_LENGTH), 4);
  MemoryFence ();
  MmioWrite32 (DSI (DSI_DMA_SW_TRIGGER), 1);

  // DMA done (BIT0) and BTA done (BIT20) in INT_CTRL
  Done = BIT0 | BIT20;
  for (i = 0; i < 10000; i++) {
    if ((MmioRead32 (DSI (DSI_INT_CTRL)) & Done) == Done) {
      break;
    }
    gBS->Stall (10);
  }

  *Data     = MmioRead32 (DSI (0x6C));
  *RdbkType = MmioRead32 (DSI (0x7C));
  *AckErr   = MmioRead32 (DSI (0x68));

  MmioWrite32 (DSI (DSI_INT_CTRL), DSI_DMA_DONE | BIT20);
  MmioWrite32 (DSI (DSI_TPG_CTRL), 0);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 1);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 0);

  return (i == 10000) ? EFI_TIMEOUT : EFI_SUCCESS;
}

//
// Reads back power mode (0x0A), pixel format (0x0C) and the ID bytes (0xDA) so
// the log shows whether the panel is alive and answering over BTA.
//
EFI_STATUS
PanelProbe (VOID)
{
  // power mode, pixel format, signal mode, self diagnostic, DSI error count,
  // brightness, ID
  STATIC CONST UINT8 Cmds[] = {0x0A, 0x0C, 0x0E, 0x0F, 0x05, 0x52, 0xDA};
  UINTN      i;
  UINT32     Data;
  UINT32     Type;
  UINT32     Ack;
  EFI_STATUS Status;

  for (i = 0; i < ARRAY_SIZE (Cmds); i++) {
    Data = Type = Ack = 0;
    Status = DsiCtrlReadDcs (Cmds[i], &Data, &Type, &Ack);
    DEBUG ((
      DEBUG_ERROR, "RD: read %02x: %r data=%08x type=%08x ack=%08x\n",
      Cmds[i], Status, Data, Type, Ack));
  }

  return EFI_SUCCESS;
}

//
// Color bars on the top two thirds, grayscale ramp below, white 4px border.
//
STATIC
VOID
DrawTestPattern (
  IN UINT32  *Fb
  )
{
  STATIC CONST UINT32 Bars[8] = {
    0xFFFFFF, 0xFFFF00, 0x00FFFF, 0x00FF00, 0xFF00FF, 0xFF0000, 0x0000FF, 0x000000
  };
  UINT32 X, Y, Color, Gray;

  for (Y = 0; Y < PANEL_HEIGHT; Y++) {
    for (X = 0; X < PANEL_WIDTH; X++) {
      if (Y < (PANEL_HEIGHT * 2 / 3)) {
        Color = Bars[X * 8 / PANEL_WIDTH];
      } else {
        Gray  = X * 255 / (PANEL_WIDTH - 1);
        Color = (Gray << 16) | (Gray << 8) | Gray;
      }

      if (X < 4 || Y < 4 || X >= PANEL_WIDTH - 4 || Y >= PANEL_HEIGHT - 4) {
        Color = 0xFFFFFF;
      }

      Fb[Y * PANEL_WIDTH + X] = Color;
    }
  }
}

//
// Prints every register where the right pipe differs from the left one that XBL
// brought up: DSI controller, D-PHY, and INTF. Differences in things like
// strengths, lane config or panel format are candidates for what we got wrong.
//
//
// Compact register diff of one MDP block pair (left XBL block vs ours),
// three diffs per line, at most MaxDiffs shown.
//
STATIC
VOID
DiffBlock (
  IN CONST CHAR8  *Tag,
  IN UINTN        LeftBase,
  IN UINTN        RightBase,
  IN UINTN        End,
  IN UINTN        MaxDiffs
  )
{
  UINTN  Off, Shown = 0, Total = 0, NPend = 0, k;
  UINT32 P[3][3];
  UINT32 A, B;

  for (Off = 0; Off < End; Off += 4) {
    A = MmioRead32 (LeftBase + Off);
    B = MmioRead32 (RightBase + Off);
    if (A == B) {
      continue;
    }
    Total++;
    if (Shown >= MaxDiffs) {
      continue;
    }
    Shown++;
    P[NPend][0] = (UINT32)Off;
    P[NPend][1] = A;
    P[NPend][2] = B;
    if (++NPend == 3) {
      DEBUG ((DEBUG_ERROR, "RD: %a %03x %08x %08x|%03x %08x %08x|%03x %08x %08x\n", Tag,
        P[0][0], P[0][1], P[0][2], P[1][0], P[1][1], P[1][2], P[2][0], P[2][1], P[2][2]));
      NPend = 0;
    }
  }
  for (k = 0; k < NPend; k++) {
    DEBUG ((DEBUG_ERROR, "RD: %a %03x %08x %08x\n", Tag, P[k][0], P[k][1], P[k][2]));
  }
  DEBUG ((DEBUG_ERROR, "RD: %a diffs total %d\n", Tag, (UINT32)Total));
}

STATIC
VOID
CompareWithLeft (VOID)
{
  STATIC CONST UINT16 Ctrl[] = {
    0x04, 0x08, 0x0C, 0x3C, 0x40, 0x44, 0x58, 0x5C, 0x60, 0x64, 0x68, 0x84, 0xA8, 0xAC, 0xB0, 0xB4, 0xB8,
    0xBC, 0xC0, 0xC4, 0xCC, 0x10C, 0x11C, 0x1B8, 0x2A8, 0x2AC, 0x2B4, 0x318,
  };
  STATIC CONST UINT16 Phy[] = {
    0x10, 0x14, 0x1C, 0x20, 0x24, 0x2C, 0x30, 0x34, 0x38, 0x3C, 0xA0,
    0xB4, 0xB8, 0xBC, 0xC0, 0xC4, 0xC8, 0xCC, 0xD0, 0xD4, 0xD8, 0xDC, 0xE0, 0xE4, 0xE8,
    0xEC, 0xF4, 0xF8, 0x100, 0x104, 0x10C, 0x110, 0x114,
  };
  STATIC CONST UINT16 LaneRegs[] = {0x00, 0x04, 0x08, 0x10, 0x14, 0x18};
  STATIC CONST UINT16 Intf[] = {
    0x04, 0x08, 0x0C, 0x10, 0x14, 0x18, 0x1C, 0x20, 0x24, 0x28, 0x2C, 0x30, 0x34,
    0x38, 0x3C, 0x40, 0x44, 0x48, 0x4C, 0x50, 0x54, 0x58, 0x5C, 0x60, 0x64, 0x68,
    0x84, 0x90, 0x94, 0x25C, 0x280, 0x284, 0x288, 0x28C, 0x290, 0x294, 0x298, 0x29C,
    0x2A0, 0x2A4, 0x2A8, 0x2B4, 0x2B8, 0x2BC, 0x2C0, 0x2AC, 0x2B0,
  };
  UINTN  i, n;
  UINT32 A, B;

  DEBUG ((DEBUG_ERROR, "RD: diff left(dsi0/intf1) vs right (offset left right)\n"));
  for (i = 0; i < ARRAY_SIZE (Ctrl); i++) {
    A = MmioRead32 ((UINTN)(DSI0_CTRL_BASE + Ctrl[i]));
    B = MmioRead32 ((UINTN)(DSI1_CTRL_BASE + Ctrl[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: C %03x %08x %08x\n", Ctrl[i], A, B));
    }
  }
  for (i = 0; i < ARRAY_SIZE (Phy); i++) {
    A = MmioRead32 ((UINTN)(DSI0_PHY_BASE + Phy[i]));
    B = MmioRead32 ((UINTN)(DSI1_PHY_BASE + Phy[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: P %03x %08x %08x\n", Phy[i], A, B));
    }
  }
  for (n = 0; n < 5; n++) {
    for (i = 0; i < ARRAY_SIZE (LaneRegs); i++) {
      UINTN Off = 0x200 + 0x80 * n + LaneRegs[i];
      A = MmioRead32 ((UINTN)(DSI0_PHY_BASE + Off));
      B = MmioRead32 ((UINTN)(DSI1_PHY_BASE + Off));
      if (A != B) {
        DEBUG ((DEBUG_ERROR, "RD: L %03x %08x %08x\n", Off, A, B));
      }
    }
  }
  for (i = 0; i < ARRAY_SIZE (Intf); i++) {
    A = MmioRead32 ((UINTN)(MDP_BASE + 0x35000 + Intf[i]));
    B = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + Intf[i]));
    if (A != B) {
      DEBUG ((DEBUG_ERROR, "RD: I %03x %08x %08x\n", Intf[i], A, B));
    }
  }

  DiffBlock ("CTL", (UINTN)MDP_BASE + 0x15000, (UINTN)MDP_BASE + RD_CTL_OFF, 0x140, 9);
  DiffBlock ("LM", (UINTN)MDP_BASE + 0x44000, (UINTN)MDP_BASE + RD_LM_OFF, 0x80, 6);
  DiffBlock ("SSPP", (UINTN)MDP_BASE + 0x4000, (UINTN)MDP_BASE + RD_SSPP_OFF, 0x180, 9);
  DiffBlock ("DSI", (UINTN)DSI0_CTRL_BASE, (UINTN)DSI1_CTRL_BASE, 0x400, 12);
  DiffBlock ("ITOP", (UINTN)MDP_BASE + 0x35000, (UINTN)MDP_BASE + 0x36000, 0x10, 3);
  // INTF status/counters (frame, line, underrun...) and the PHY PLL block
  DiffBlock ("IST", (UINTN)MDP_BASE + 0x35080, (UINTN)MDP_BASE + 0x36080, 0x80, 6);
  DiffBlock ("PLL", (UINTN)DSI0_PHY_BASE + 0x500, (UINTN)DSI1_PHY_BASE + 0x500, 0x200, 12);
  DEBUG ((
    DEBUG_ERROR, "RD: dsi intr L=%08x R=%08x (b8=mdp done, b24=err)\n",
    MmioRead32 ((UINTN)(DSI0_CTRL_BASE + 0x110)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x110))));

  // DISP_CC: CBCs (bit31 = branch off) and every non-zero RCG word
  {
    UINTN  Off, N = 0, Lines = 0;
    UINT32 V, Q[4][2];

    for (Off = 0x2000; Off < 0x2180 && Lines < 12; Off += 4) {
      V = MmioRead32 ((UINTN)(DISP_CC_BASE + Off));
      if (V == 0) {
        continue;
      }
      Q[N][0] = (UINT32)Off;
      Q[N][1] = V;
      if (++N == 4) {
        Lines++;
        DEBUG ((DEBUG_ERROR, "RD: CC %04x=%08x %04x=%08x %04x=%08x %04x=%08x\n",
          Q[0][0], Q[0][1], Q[1][0], Q[1][1], Q[2][0], Q[2][1], Q[3][0], Q[3][1]));
        N = 0;
      }
    }
    for (Off = 0; Off < N; Off++) {
      DEBUG ((DEBUG_ERROR, "RD: CC %04x=%08x\n", Q[Off][0], Q[Off][1]));
    }
  }

  // DSC core 0 (left, XBL) against our core, compact: three diffs per line,
  // at most 12 diffs, wrapper (0xF00) first because it is the most telling.
  {
    UINTN  Off, Shown = 0, Total = 0, Pass;
    UINT32 Pending[3][3];
    UINTN  NPend = 0, k;

    for (Pass = 0; Pass < 2; Pass++) {
      UINTN Start = (Pass == 0) ? 0xF00 : 0x100;
      UINTN End   = (Pass == 0) ? 0xF80 : 0x200;

      for (Off = Start; Off < End; Off += 4) {
        A = MmioRead32 ((UINTN)(MDP_BASE + 0x80000 + Off));
        B = MmioRead32 ((UINTN)(MDP_BASE + ((Pass == 0) ? RD_DSC_CTL_OFF - 0xF00 : RD_DSC_ENC_OFF - 0x100) + Off));
        if (A == B) {
          continue;
        }
        Total++;
        if (Shown >= 6) {
          continue;
        }
        Shown++;
        Pending[NPend][0] = (UINT32)Off;
        Pending[NPend][1] = A;
        Pending[NPend][2] = B;
        if (++NPend == 3) {
          DEBUG ((DEBUG_ERROR, "RD: D %03x %08x %08x|%03x %08x %08x|%03x %08x %08x\n",
            Pending[0][0], Pending[0][1], Pending[0][2],
            Pending[1][0], Pending[1][1], Pending[1][2],
            Pending[2][0], Pending[2][1], Pending[2][2]));
          NPend = 0;
        }
      }
    }
    for (k = 0; k < NPend; k++) {
      DEBUG ((DEBUG_ERROR, "RD: D %03x %08x %08x\n", Pending[k][0], Pending[k][1], Pending[k][2]));
    }
    DEBUG ((DEBUG_ERROR, "RD: dsc diffs total %d (left ours)\n", (UINT32)Total));
  }
}

//
// The left display console (FrameBufferSerialPortLib) keeps its cursor in
// memory right after the left framebuffer. Clearing the framebuffer and zeroing
// that cursor makes the RD: log start at the top of an empty screen.
//
STATIC
VOID
ResetLeftConsole (VOID)
{
  ARM_MEMORY_REGION_DESCRIPTOR_EX  Region;
  UINTN                            FbBytes;

  if (EFI_ERROR (GetPlatformDisplayFrameBuffer (&Region))) {
    return;
  }

  FbBytes = (UINTN)FixedPcdGet32 (PcdMipiFrameBufferWidth) *
            FixedPcdGet32 (PcdMipiFrameBufferHeight) *
            FixedPcdGet32 (PcdMipiFrameBufferPixelBpp) / 8;

  ZeroMem ((VOID *)(UINTN)Region.Address, FbBytes);
  ZeroMem ((VOID *)(UINTN)(Region.Address + FbBytes), sizeof (FBCON_POSITION));
}

//
// Hardware state snapshot for bring-up debugging, held on the left console.
//
STATIC
VOID
DiagDump (
  IN BOOLEAN     PanelUp,
  IN EFI_STATUS  LastStatus
  )
{
  STATIC CONST struct {
    UINT32      Off;
    CONST CHAR8 *Name;
  } Clks[] = {
    {0x207C, "ahb"}, {0x200C, "mdp"}, {0x2008, "pclk1"}, {0x2030, "byte1"},
    {0x2034, "byte1_intf"}, {0x203C, "esc1"}, {0x3000, "gdsc"},
  };
  UINTN i;
  UINT32 Frame0, Frame1;

  DEBUG ((DEBUG_ERROR, "RD: panelup=%u last=%r\n", PanelUp, LastStatus));
  (VOID)Clks;
  (VOID)i;
  DEBUG ((
    DEBUG_ERROR, "RD: pll lock=%08x phy_status=%08x\n",
    MmioRead32 ((UINTN)(DSI1_PHY_BASE + 0x500 + 0x1B0)),
    MmioRead32 ((UINTN)(DSI1_PHY_BASE + 0x140))));
  DEBUG ((
    DEBUG_ERROR, "RD: dsi ctrl=%08x clk=%08x intr=%08x mdp=%08x\n",
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x04)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x11C)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x110)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x40))));
  DEBUG ((
    DEBUG_ERROR, "RD: dsi st=%08x fifo=%08x ack=%08x lane=%08x phyerr=%08x/%08x tmo=%08x\n",
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x08)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x0C)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x68)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xA8)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xB4)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0x318)),
    MmioRead32 ((UINTN)(DSI1_CTRL_BASE + 0xC0))));
  DEBUG ((
    DEBUG_ERROR, "RD: ctl top=%08x flush=%08x intf_act=%08x\n",
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0x14)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0x18)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xF4))));
  DEBUG ((
    DEBUG_ERROR, "RD: ctl dsc=%08x/%08x(L) fetch=%08x/%08x(L) lay=%08x/%08x(L)\n",
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xE8)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0xE8)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + 0xFC)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0xFC)),
    MmioRead32 ((UINTN)(MDP_BASE + RD_CTL_OFF + RD_CTL_INDEX * 4)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x15000 + 0x00))));
  DEBUG ((
    DEBUG_ERROR, "RD: intf en=%08x cfg2=%08x tear=%08x\n",
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x0)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x60)),
    MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0x284))));

  // INTF frame counter should advance if frames are being pushed
  Frame0 = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0xAC));
  gBS->Stall (MS (100));
  Frame1 = MmioRead32 ((UINTN)(MDP_BASE + 0x36000 + 0xAC));
  DEBUG ((DEBUG_ERROR, "RD: intf frame cnt %08x -> %08x\n", Frame0, Frame1));

  CompareWithLeft ();

  gBS->Stall (MS (RIGHT_DISPLAY_DIAG_HOLD_MS));
}