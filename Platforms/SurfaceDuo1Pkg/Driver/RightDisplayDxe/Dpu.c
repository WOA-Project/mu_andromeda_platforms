/** @file
  DPU pipeline for the right display: VIG_2 -> LM_2 -> PP_2 -> DSC_2 -> INTF_2
  (DSI1), command mode with DSC 1.1, driven by CTL_1.

  The blocks chosen here are not in use by the left display pipeline
  configured by XBL. Not hardware-verified.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

STATIC UINT64  mFbBase;

STATIC
VOID
MDPMmioWrite32 (
  IN UINT32 Offset,
  IN UINT32 Value
  )
{
  MmioWrite32 (MDP (Offset), Value);
}

STATIC
UINT32
MDPMmioRead32 (
  IN UINT32 Offset
  )
{
  return MmioRead32 (MDP (Offset));
}

STATIC
UINT32
Be16 (
  IN CONST UINT8 *P
  )
{
  return ((UINT32)P[0] << 8) | P[1];
}

//
// Program the DSC encoder from the PPS blob sent to the panel, so both sides
// of the link are guaranteed to agree.
//
STATIC
VOID
DscSetup (
  IN CONST UINT8 *Pps
  )
{
  UINT32 Bpp, PicH, PicW, SliceH, SliceW, Chunk, Xmit, Dec, Scale, Incr, Decr;
  UINT32 FirstLineBpg, Nfl, SliceBpg, InitOff, FinalOff, FlatMin, FlatMax;
  UINT32 Model, Edge, Qil0, Qil1, TgtHi, TgtLo, Bpc, LineBuf;
  UINT32 SoftSlices, InitialLines, LastGroup, Val;
  UINT32 i;

  Bpc     = Pps[3] >> 4;
  LineBuf = Pps[3] & 0xF;
  Bpp     = ((Pps[4] & 3) << 8) | Pps[5];
  PicH    = Be16 (&Pps[6]);
  PicW    = Be16 (&Pps[8]);
  SliceH  = Be16 (&Pps[10]);
  SliceW  = Be16 (&Pps[12]);
  Chunk   = Be16 (&Pps[14]);
  Xmit    = Be16 (&Pps[16]) & 0x3FF;
  Dec     = Be16 (&Pps[18]);
  Scale   = Pps[21] & 0x3F;
  Incr    = Be16 (&Pps[22]);
  Decr    = Be16 (&Pps[24]) & 0xFFF;
  FirstLineBpg = Pps[27] & 0x1F;
  Nfl      = Be16 (&Pps[28]);
  SliceBpg = Be16 (&Pps[30]);
  InitOff  = Be16 (&Pps[32]);
  FinalOff = Be16 (&Pps[34]);
  FlatMin  = Pps[36] & 0x1F;
  FlatMax  = Pps[37] & 0x1F;
  Model    = Be16 (&Pps[38]);
  Edge     = Pps[40] & 0xF;
  Qil0     = Pps[41] & 0x1F;
  Qil1     = Pps[42] & 0x1F;
  TgtHi    = Pps[43] >> 4;
  TgtLo    = Pps[43] & 0xF;

  SoftSlices = PicW / SliceW;

  // Initial lines: slice pipeline latency in whole lines, plus one when the
  // output FIFO would otherwise be underfilled
  {
    UINT32 SsmDelay, Total, FifoSize, T0, T1, T2;

    SsmDelay     = 3 * ((Bpc < 10) ? 84 : 92);
    Total        = SoftSlices * SsmDelay + Xmit + 47;
    FifoSize     = (SoftSlices == 2) ? 5610 : 11610;
    InitialLines = (Total + SliceW - 1) / SliceW;
    T0           = Xmit + (FirstLineBpg * PicW) / 24;
    T1           = (UINT32)(((2 * 4 + 3 * SoftSlices) * Chunk) / 4);
    T2           = (Total % SliceW) * ((Bpp >> 4) / 8);
    if (T0 + T1 - T2 < FifoSize) {
      InitialLines++;
    }
  }

  // Valid pixels in the last group of each slice: 0 = 1, 1 = 2, 2 = 3
  LastGroup = (SliceW + 2) % 3;

  // One DSC encoder: no split panel, no hard slice mux, command mode
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_COMMON_MODE), 0);

  // INPUT_10BITS | CONVERT_RGB (always) | LINEBUF_DEPTH | BLOCK_PRED | BPP |
  // SLICE_LAST_GROUP_SIZE | INITIAL_LINES
  Val = ((Bpc == 10) ? BIT0 : 0) | BIT1 | (LineBuf << 3) |
        (((Pps[4] & BIT5) ? 1U : 0U) << 7) | (Bpp << 8) |
        (LastGroup << 18) | (InitialLines << 20);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_ENC), Val);

  MmioWrite32 (MDP (RD_DSC_OFF + DSC_PICTURE), (PicW << 16) | PicH);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_SLICE), (SliceW << 16) | SliceH);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_CHUNK_SIZE), Chunk << 16);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_DELAY), (Dec << 16) | Xmit);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_SCALE_INITIAL), Scale);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_SCALE_DEC_INT), Decr);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_SCALE_INC_INT), Incr);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_FIRST_LINE_BPG), FirstLineBpg);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_BPG_OFFSET), (Nfl << 16) | SliceBpg);
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_DSC_OFFSET), (InitOff << 16) | FinalOff);
  MmioWrite32 (
    MDP (RD_DSC_OFF + DSC_FLATNESS),
    FlatMin | (FlatMax << 5) | ((2U << (Bpc - 8)) << 10));
  MmioWrite32 (MDP (RD_DSC_OFF + DSC_RC_MODEL_SIZE), Model);
  MmioWrite32 (
    MDP (RD_DSC_OFF + DSC_RC),
    Edge | (Qil0 << 4) | (Qil1 << 9) | (TgtHi << 14) | (TgtLo << 18));

  // Thresholds are stored in the PPS already shifted right by 6, one byte each
  for (i = 0; i < 14; i++) {
    MmioWrite32 (MDP (RD_DSC_OFF + DSC_RC_BUF_THRESH (i)), Pps[44 + i]);
  }

  for (i = 0; i < 15; i++) {
    UINT32 Rc = Be16 (&Pps[58 + i * 2]);
    MmioWrite32 (MDP (RD_DSC_OFF + DSC_RANGE_MIN_QP (i)), (Rc >> 11) & 0x1F);
    MmioWrite32 (MDP (RD_DSC_OFF + DSC_RANGE_MAX_QP (i)), (Rc >> 6) & 0x1F);
    MmioWrite32 (MDP (RD_DSC_OFF + DSC_RANGE_BPG (i)), Rc & 0x3F);
  }

  // Ping-pong feeds the encoder: byte order 0..5, big endian, encoder enabled
  MmioWrite32 (
    MDP (PP_BASE + PP_DCE_DATA_OUT_SWAP),
    BIT18 | (1U << 3) | (2U << 6) | (3U << 9) | (4U << 12) | (5U << 15));
  MmioWrite32 (MDP (PP_BASE + PP_DSC_MODE), 1);

  // PP to DSC crossbar: DSC_2 takes its input from PP_2
  MmioWrite32 (MDP (DSC_CTL + DSC_CTL_DSC (DSC_INDEX)), PP_INDEX);
}

STATIC
VOID
PipelineSetup (VOID)
{
  UINT32 Stride = PANEL_WIDTH * 4;
  UINT32 Size   = (PANEL_HEIGHT << 16) | PANEL_WIDTH;
  UINT32 Retry;
  UINT32 Stage;

  // Reset this CTL path
  MDPMmioWrite32 (CTL_SW_RESET, 1);
  for (Retry = 0; Retry < 2000 && (MDPMmioRead32 (CTL_SW_RESET) & BIT0); Retry++) {
    gBS->Stall (1);
  }

  // SSPP: VIG_2 fetches the framebuffer. RGB source, so the VIG CSC is bypassed
  MDPMmioWrite32 (SSPP_BASE + SSPP_CSC1_OP_MODE, MDPMmioRead32 (SSPP_BASE + SSPP_CSC1_OP_MODE) & ~BIT0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_SIZE, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_XY, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_OUT_SIZE, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_OUT_XY, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC0_ADDR, (UINT32)mFbBase);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC1_ADDR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC2_ADDR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC3_ADDR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_YSTRIDE0, Stride);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_YSTRIDE1, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_FORMAT, SSPP_FORMAT_XRGB8888);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_UNPACK, SSPP_UNPACK_XRGB8888);
  // Unity scaling: QSEED3 scaler disabled
  MDPMmioWrite32 (SSPP_BASE + SSPP_QSEED3_OP_MODE, 0);
  // Pixel extension: no over-fetch or repeat (unscaled), required pixels = size
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C0_LR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C0_TB, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C0_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C1C2_LR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C1C2_TB, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C1C2_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C3_LR, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C3_TB, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C3_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_OP_MODE, SSPP_OP_PE_OVERRIDE);
  MDPMmioWrite32 (SSPP_BASE + SSPP_DANGER_LUT, 0x0003FFFF);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SAFE_LUT, 0x0000FE00);
  MDPMmioWrite32 (SSPP_BASE + SSPP_CREQ_LUT_0, 0x67777777);
  MDPMmioWrite32 (SSPP_BASE + SSPP_CREQ_LUT_1, 0x00112236);

  // Layer mixer: opaque blending on stages 0..5 (BG = inverse FG constant
  // alpha, FG constant alpha 0xFF), black border
  MDPMmioWrite32 (LM_BASE + LM_OP_MODE, 0);
  MDPMmioWrite32 (LM_BASE + LM_OUT_SIZE, Size);
  MDPMmioWrite32 (LM_BASE + 0x08, 0);
  MDPMmioWrite32 (LM_BASE + 0x0C, 0);
  for (Stage = 0; Stage < 6; Stage++) {
    MDPMmioWrite32 (LM_BASE + LM_BLEND0_OP + 0x18 * Stage, BIT10);
    MDPMmioWrite32 (LM_BASE + LM_BLEND0_FG + 0x18 * Stage, 0xFFU << 16);
  }

  // CTL: VIG_2 (bits 8:6) on z-order 4, border enabled
  MDPMmioWrite32 (CTL_LAYER (LM_INDEX), BIT24 | (4U << RD_STAGE_SHIFT));
  MDPMmioWrite32 (CTL_LAYER_EXT (LM_INDEX), 0);
  MDPMmioWrite32 (CTL_LAYER_EXT2 (LM_INDEX), 0);
  MDPMmioWrite32 (CTL_LAYER_EXT3 (LM_INDEX), 0);

  // DSC encoder and ping-pong
  DscSetup (gPanelPps);

  // INTF_2: compressed data, bound to PP_2, tearcheck driven by the panel TE pin
  // (no timing block or panel format is needed in command mode)
  MDPMmioWrite32 (INTF_BASE + INTF_MUX, PP_INDEX);
  // The panel XML sets DSITEvSyncSelect=1 (TE on the mdp_vsync pin)
  MDPMmioWrite32 (INTF_BASE + INTF_TEAR_MDP_VSYNC_SEL, 1);

  // The HW-generated TE is effectively disabled by a near-maximum sync height.
  // VSYNC_COUNT = CXO (19.2 MHz) / (height * refresh), counter enabled
  // (BIT19), VSYNC_IN_EN (BIT20, dedicated TE pin) set at setup
  MDPMmioWrite32 (
    INTF_BASE + INTF_SYNC_CONFIG_VSYNC,
    (19200000U / (PANEL_HEIGHT * 60U)) | BIT19 | BIT20);
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_CONFIG_HEIGHT, 0xFFFF);
  MDPMmioWrite32 (INTF_BASE + INTF_VSYNC_INIT_VAL, PANEL_HEIGHT);
  MDPMmioWrite32 (INTF_BASE + INTF_RD_PTR_IRQ, PANEL_HEIGHT + 1);
  MDPMmioWrite32 (INTF_BASE + INTF_WR_PTR_IRQ, PANEL_HEIGHT);
  MDPMmioWrite32 (INTF_BASE + INTF_START_POS, PANEL_HEIGHT);
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_THRESH, 4 | (5 << 16));
  // Autorefresh (enable, frame count 1) is armed before any panel traffic
  MDPMmioWrite32 (INTF_BASE + INTF_AUTOREFRESH, BIT31 | 1);
  MDPMmioWrite32 (INTF_BASE + INTF_TEAR_CHECK_EN, 1);

  // CTL: command mode, which blocks are active
  MDPMmioWrite32 (CTL_TOP, CTL_TOP_MODE_CMD);
  MDPMmioWrite32 (CTL_INTF_MASTER, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_MERGE_3D_ACTIVE, 0);
  MDPMmioWrite32 (CTL_WB_ACTIVE, 0);
  MDPMmioWrite32 (CTL_CDM_ACTIVE, 0);
  MDPMmioWrite32 (CTL_INTF_ACTIVE, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_DSC_ACTIVE, 1U << DSC_INDEX);

  // Flush: sub-block registers first, then the main flush
  MDPMmioWrite32 (CTL_INTF_FLUSH, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_DSC_FLUSH, 1U << DSC_INDEX);
  MDPMmioWrite32 (CTL_PERIPH_FLUSH, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_FLUSH, CTL_FLUSH_VIG2 | CTL_FLUSH_LM2 | CTL_FLUSH_CTL | CTL_FLUSH_DSC | CTL_FLUSH_INTF | CTL_FLUSH_PERIPH);
}

EFI_STATUS
DpuStart (
  IN EFI_PHYSICAL_ADDRESS FbBase
  )
{
  if (FbBase > 0xFFFFFFFFULL) {
    return EFI_UNSUPPORTED;
  }

  mFbBase = FbBase;
  PipelineSetup ();

  // XBL forces the first frame in command mode with CTL_START after the flush
  MDPMmioWrite32 (CTL_START, 1);
  return EFI_SUCCESS;
}
