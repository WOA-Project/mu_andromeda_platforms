/** @file
  DSI1 host controller setup and FIFO-mode command transmission.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#define DSI(off) ((UINTN)(DSI1_CTRL_BASE + (off)))

#define DSI_CTRL                0x04
#define DSI_DMA_CTRL            0x3C
#define DSI_MDP_CTRL            0x40
#define DSI_MDP_DCS_CMD_CTRL    0x44
#define DSI_DMA_CMD_LENGTH      0x4C
#define DSI_TRIG_CTRL           0x84
#define DSI_DMA_SW_TRIGGER      0x90
#define DSI_LANE_SWAP_CTRL      0xB0
#define DSI_HS_TIMER_CTRL       0xBC
#define DSI_CLKOUT_TIMING_CTRL  0xC4
#define DSI_LANE_CTRL           0xAC
#define DSI_NULL_INSERTION      0x2B4
#define DSI_EOT_PACKET_CTRL     0xCC
#define DSI_INT_CTRL            0x110
#define DSI_SOFT_RESET          0x118
#define DSI_LANE_STATUS         0xA8
#define DSI_CLK_CTRL            0x11C
#define DSI_PHY_SW_RESET        0x12C
#define DSI_TPG_CTRL            0x15C
#define DSI_TPG_DMA_INIT_VAL    0x17C
#define DSI_TPG_DMA_FIFO_RESET  0x1EC
#define DSI_MDP_CTRL2           0x1B8
#define DSI_MDP_STREAM0_CTRL    0x58
#define DSI_MDP_STREAM0_TOTAL   0x5C
#define DSI_MDP_STREAM1_CTRL    0x60
#define DSI_MDP_STREAM1_TOTAL   0x64
#define DSI_CMD_COMPRESSION_CTRL  0x2A8
#define DSI_CMD_COMPRESSION_CTRL2 0x2AC

#define Be16Pps(p) (((UINT32)(p)[0] << 8) | (p)[1])

#define DSI_DMA_DONE BIT0

EFI_STATUS
DsiCtrlInit (VOID)
{
  UINT32 Val;

  // The PHY reset is done in DsiPhyEnable, before the PLL is programmed:
  // resetting here would wipe the locked PLL and the lane setup.
  // Controller disable + soft reset, then clear interrupts (as XBL HAL_DSI_Init)
  MmioWrite32 (DSI (DSI_CTRL), 0);
  MmioWrite32 (DSI (DSI_CLK_CTRL), 0x23F);
  MmioWrite32 (DSI (DSI_SOFT_RESET), BIT0);
  gBS->Stall (10);
  MmioWrite32 (DSI (DSI_SOFT_RESET), 0);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 1);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 0);

  // DMA trigger: software (4); MDP trigger (bits 6:4): none. BIT31 (DSI-level
  // TE gating) is left clear, matching the live DSI0 controller; the MDP
  // tearcheck handles TE.
  MmioWrite32 (DSI (DSI_TRIG_CTRL), 0x4);
  // Copy both timers from the live DSI0 controller as XBL left it
  MmioWrite32 (DSI (DSI_CLKOUT_TIMING_CTRL), MmioRead32 (DSI0_CTRL_BASE + DSI_CLKOUT_TIMING_CTRL));
  // BIT0: append EOT to HS packets (live DSI0 reads 1)
  MmioWrite32 (DSI (DSI_EOT_PACKET_CTRL), BIT0);
  MmioWrite32 (DSI (DSI_HS_TIMER_CTRL), MmioRead32 (DSI0_CTRL_BASE + DSI_HS_TIMER_CTRL));
  MmioWrite32 (DSI (DSI_LANE_SWAP_CTRL), 0);

  MmioWrite32 (DSI (DSI_CLK_CTRL), 0x23F);

  // Clock lane + four data lanes
  Val = MmioRead32 (DSI (DSI_CTRL));
  Val |= BIT8 | BIT7 | BIT6 | BIT5 | BIT4;
  MmioWrite32 (DSI (DSI_CTRL), Val);

#if PANEL_INIT_HS
  // DSIClockHSForceRequest: clock lane in HS before the init sequence
  MmioWrite32 (DSI (DSI_LANE_CTRL), BIT28);
#endif

  // DSINullpacketInsertionBytes=1: EN, VC 0, WC 0
  MmioWrite32 (DSI (DSI_NULL_INSERTION), BIT0);

  // Command engine: RGB888, DCS write_memory_start / continue
  // DST_FORMAT = RGB888, INTERLEAVE_MAX = 1
  MmioWrite32 (DSI (DSI_MDP_CTRL), 0x8 | (1 << 20));
  // XBL writes the MDP_CTRL value into MDP_CTRL2 with BURST_MODE (BIT16) set;
  // the live DSI0 reads 0x00110008
  MmioWrite32 (DSI (DSI_MDP_CTRL2), 0x8 | (1 << 20) | BIT16);
  MmioWrite32 (DSI (DSI_MDP_DCS_CMD_CTRL), 0x2C | (0x3C << 8) | BIT16);

  MmioWrite32 (DSI (DSI_CLK_CTRL), 0x23F | BIT8 | BIT9 | BIT11 | BIT21);
  MmioWrite32 (DSI (DSI_CTRL), Val | BIT0);
  MmioWrite32 (DSI (DSI_CTRL), Val | BIT0 | BIT2);
  MmioWrite32 (DSI (DSI_CLK_CTRL), 0x23F);

  return EFI_SUCCESS;
}

//
// MDP stream: DSC-compressed data as DCS long writes, one packet per line
// carrying both slices (slice-per-pkt = 2, so PKT_PER_LINE stays 0). Mainline
// dsi_host.c hardcodes one slice per packet (WC = chunk + 1); here the word
// count scales with the slices per packet. With DATABUS_WIDEN the stream
// H_TOTAL counts six bytes per pclk.
//
VOID
DsiCtrlSetupStream (VOID)
{
  UINT32 Chunk   = Be16Pps (&gPanelPps[14]);
  UINT32 Slices  = Be16Pps (&gPanelPps[8]) / Be16Pps (&gPanelPps[12]);
  UINT32 Bytes   = Chunk * Slices;
  UINT32 Word    = (Bytes + 1);

  MmioWrite32 (DSI (DSI_MDP_STREAM0_CTRL), 0x39 | (Word << 16));
  MmioWrite32 (DSI (DSI_MDP_STREAM0_TOTAL), ((Bytes + 5) / 6) | (PANEL_HEIGHT << 16));
  // Stream 1 stays cleared, as on the live left controller (0x60/0x64 read 0)
  MmioWrite32 (DSI (DSI_CMD_COMPRESSION_CTRL), BIT0 | ((Bytes % 3) << 4) | (0x39 << 8));
  MmioWrite32 (DSI (DSI_CMD_COMPRESSION_CTRL2), Chunk);
}

EFI_STATUS
DsiCtrlSendPacket (
  IN UINT8        DataType,
  IN CONST UINT8  *Payload,
  IN UINT32       PayloadLength,
  IN BOOLEAN      LowPower
  )
{
  UINT32 Words[80];
  UINT32 Header;
  UINT32 Count;
  UINT32 Bytes;
  UINT32 i;
  UINT32 Val;
  BOOLEAN Long = (DataType == 0x39 || DataType == 0x29 || DataType == 0x0A);

  if (PayloadLength > 256 - 8) {
    return EFI_BAD_BUFFER_SIZE;
  }

  // Embedded header: bytes 0-1 = word count (long) or the two data bytes
  // (short), byte 2 = VC[7:6] | DT[5:0], byte 3 = last | long | BTA
  if (Long) {
    Header = (DataType << 16) | (PayloadLength & 0xFFFF);
  } else {
    Header = (DataType << 16);
    if (PayloadLength > 0) {
      Header |= (UINT32)Payload[0];
    }
    if (PayloadLength > 1) {
      Header |= (UINT32)Payload[1] << 8;
    }
  }
  Header |= BIT31; // last command
  if (Long) {
    Header |= BIT30;
  }

  ZeroMem (Words, sizeof (Words));
  Words[0] = Header;
  Count = 1;
  if (Long) {
    CopyMem (&Words[1], Payload, PayloadLength);
    Count += (PayloadLength + 3) / 4;
  }
  Bytes = Count * 4;
  if (Count & 1) {
    Count++; // FIFO is written in pairs
  }

  MmioWrite32 (DSI (DSI_TPG_CTRL), BIT1 | BIT2 | (3 << 16));
  for (i = 0; i < Count; i++) {
    MmioWrite32 (DSI (DSI_TPG_DMA_INIT_VAL), Words[i]);
  }

  Val = BIT28;
  if (LowPower) {
    Val |= BIT26;
  }
  MmioWrite32 (DSI (DSI_DMA_CTRL), Val);
  MmioWrite32 (DSI (DSI_DMA_CMD_LENGTH), Bytes);
  MemoryFence ();
  MmioWrite32 (DSI (DSI_DMA_SW_TRIGGER), 1);

  for (i = 0; i < 10000; i++) {
    if (MmioRead32 (DSI (DSI_INT_CTRL)) & DSI_DMA_DONE) {
      break;
    }
    gBS->Stall (10);
  }

  if (i == 10000) {
    DEBUG ((
      DEBUG_ERROR,
      "RD: dma timeout lane=%08x int=%08x dmactl=%08x tpg=%08x ctrl=%08x\n",
      MmioRead32 (DSI (DSI_LANE_STATUS)),
      MmioRead32 (DSI (DSI_INT_CTRL)),
      MmioRead32 (DSI (DSI_DMA_CTRL)),
      MmioRead32 (DSI (DSI_TPG_CTRL)),
      MmioRead32 (DSI (DSI_CTRL))));
  }

  MmioWrite32 (DSI (DSI_INT_CTRL), DSI_DMA_DONE);
  MmioWrite32 (DSI (DSI_TPG_CTRL), 0);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 1);
  MmioWrite32 (DSI (DSI_TPG_DMA_FIFO_RESET), 0);

  return (i == 10000) ? EFI_TIMEOUT : EFI_SUCCESS;
}

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
