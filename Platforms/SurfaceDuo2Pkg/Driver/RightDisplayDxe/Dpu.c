/** @file
  DPU pipeline for the right display: DMA2 -> LM_2 -> PP_2 -> DSC_2 -> INTF_2
  (DSI1), command mode with DSC 1.1, driven by CTL_2.

  Block offsets and register layouts follow the mainline msm/dpu1 driver
  (SM8350 catalog). The blocks chosen here are not expected to be in use by the
  XBL-configured left display pipeline.

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
  UINT32 Model, Edge, Qil0, Qil1, TgtHi, TgtLo, Bpc, LineBuf, Minor;
  UINT32 SliceCount, InitialLines, Val;
  UINT32 Min[3], Max[3], Bpg[3];
  UINT32 i;

  Minor   = Pps[0] & 0xF;
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

  SliceCount = PicW / SliceW;

  // Encoder pipeline latency model: SSM delay + output buffer latency + extra
  // output-rate budget; matches XBL for 10bpc, 2 slices, 8 bpp (gives 3)
  {
    UINT32 BppG, SsmDelay, InLat, ObufLat, ExtraBits, Extra, Total;

    BppG      = Bpp >> 4;
    SsmDelay  = (10 + 1) * 4 + 48 - 1;
    InLat     = 28 + 3 * (SsmDelay + 2) * SliceCount;
    ObufLat   = ((9 * 64 + 48) + BppG - 1) / BppG + 1;
    ExtraBits = (SliceCount > 1) ? Chunk * 8 : ((64 - BppG) * Chunk * 8 + 63) / 64;
    Extra     = (ExtraBits + BppG - 1) / BppG;
    Total     = Xmit + InLat + ObufLat + Extra;

    InitialLines = (Total + SliceW - 1) / SliceW;
  }

  // Same output buffer depth as the live left DSC (0x12BC0003 -> 2400/slices-1)
  // The DSC core common register is shared with the live left DSC_0 in chain 1
  MmioWrite32 (
    MDP (DSC_ENC + ENC_DF_CTRL),
    (InitialLines & 0xFF) | (((2400 / SliceCount) - 1) << 18));

  // PPS byte 4: bit5 = block_pred_enable, bit4 = convert_rgb
  Val = (Minor << 28) | (Bpp << 10) | (((Pps[4] & BIT5) ? 1U : 0U) << 20) |
        (((Pps[4] & BIT4) ? 1U : 0U) << 4) | (LineBuf << 6) | Bpc;
  MmioWrite32 (MDP (DSC_ENC + ENC_MAIN_CONF), Val);

  MmioWrite32 (MDP (DSC_ENC + ENC_PICTURE_SIZE), PicW | (PicH << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_SLICE_SIZE), SliceW | (SliceH << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_MISC_SIZE), Chunk);
  MmioWrite32 (MDP (DSC_ENC + ENC_HRD_DELAYS), Xmit | (Dec << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_SCALE), Scale);
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_SCALE_INC_DEC), Incr | (Decr << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_OFFSETS_1), FirstLineBpg);
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_OFFSETS_2), Nfl | (SliceBpg << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_OFFSETS_3), InitOff | (FinalOff << 16));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_OFFSETS_4), 0);
  MmioWrite32 (
    MDP (DSC_ENC + ENC_FLATNESS_QP),
    FlatMin | (FlatMax << 5) | ((2U << (Bpc - 8)) << 10));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_MODEL_SIZE), Model);
  MmioWrite32 (
    MDP (DSC_ENC + ENC_RC_CONFIG),
    Edge | (Qil0 << 8) | (Qil1 << 13) | (TgtHi << 20) | (TgtLo << 24));

  for (i = 0; i < 4; i++) {
    CONST UINT8 *T = &Pps[44 + i * 4];
    Val = T[0] | (T[1] << 8);
    if (i < 3) {
      Val |= ((UINT32)T[2] << 16) | ((UINT32)T[3] << 24);
    }
    MmioWrite32 (MDP (DSC_ENC + ENC_RC_BUF_THRESH (i)), Val);
  }

  for (i = 0; i < 3; i++) {
    UINT32 j;
    Min[i] = Max[i] = Bpg[i] = 0;
    for (j = 0; j < 5; j++) {
      UINT32 Rc = Be16 (&Pps[58 + (i * 5 + j) * 2]);
      Min[i] |= ((Rc >> 11) & 0x1F) << (j * 5);
      Max[i] |= ((Rc >> 6) & 0x1F) << (j * 5);
      Bpg[i] |= (Rc & 0x3F) << (j * 6);
    }
    MmioWrite32 (MDP (DSC_ENC + ENC_RC_MIN_QP (i)), Min[i]);
    MmioWrite32 (MDP (DSC_ENC + ENC_RC_MAX_QP (i)), Max[i]);
    MmioWrite32 (MDP (DSC_ENC + ENC_RC_RANGE_BPG (i)), Bpg[i]);
  }

  // XBL's HAL derives these two from its own model and they differ by one
  // step from the PPS-derived value; mirror the live left encoder
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_SCALE_INC_DEC), MDPMmioRead32 (0x80000 + 0x100 + ENC_RC_SCALE_INC_DEC));
  MmioWrite32 (MDP (DSC_ENC + ENC_RC_OFFSETS_2), MDPMmioRead32 (0x80000 + 0x100 + ENC_RC_OFFSETS_2));

  // enable | 8bpc | cmd mode (not video)
  MmioWrite32 (MDP (DSC_CTL + DSC_CTL_CFG), BIT0 | BIT11 | BIT17);
  MmioWrite32 (MDP (DSC_CTL + DSC_CTL_PP_MUX), PP_INDEX);
  // DATA_IN_SWAP: take what XBL programmed on the live left DSC
  MmioWrite32 (MDP (DSC_CTL + 0x08), MDPMmioRead32 (0x80000 + 0xF08));
}

STATIC
VOID
PipelineSetup (VOID)
{
  UINT32 Stride = PANEL_WIDTH * 4;
  UINT32 Size   = (PANEL_HEIGHT << 16) | PANEL_WIDTH;
  UINT32 Retry;

  // Reset this CTL path
  MDPMmioWrite32 (CTL_SW_RESET, 1);
  for (Retry = 0; Retry < 2000 && (MDPMmioRead32 (CTL_SW_RESET) & BIT0); Retry++) {
    gBS->Stall (1);
  }

  // SSPP: DMA2 fetches the framebuffer
  MDPMmioWrite32 (SSPP_BASE + SSPP_MULTIRECT_OPMODE, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_SIZE, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_XY, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_OUT_SIZE, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_OUT_XY, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC0_ADDR, (UINT32)mFbBase);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_YSTRIDE0, Stride);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_YSTRIDE1, 0);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_FORMAT, SSPP_FORMAT_XRGB8888);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_UNPACK, SSPP_UNPACK_XRGB8888);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C0_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C1C2_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_PE_C3_REQ, Size);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SRC_OP_MODE, SSPP_OP_PE_OVERRIDE);
  MDPMmioWrite32 (SSPP_BASE + SSPP_DANGER_LUT, 0x0003FFFF);
  MDPMmioWrite32 (SSPP_BASE + SSPP_SAFE_LUT, 0x0000FE00);
  MDPMmioWrite32 (SSPP_BASE + SSPP_CREQ_LUT_0, 0x67777777);
  MDPMmioWrite32 (SSPP_BASE + SSPP_CREQ_LUT_1, 0x00112236);

  // Layer mixer: mirror the left mixer (stage registers have a 0x18 stride,
  // the left values are the ones known to show a framebuffer)
  {
    UINT32 Off;
    for (Off = 0; Off < 0x80; Off += 4) {
      MDPMmioWrite32 (LM_BASE + Off, MDPMmioRead32 (LM_LEFT_BASE + Off));
    }
    MDPMmioWrite32 (LM_BASE + LM_OUT_SIZE, Size);
  }

  // CTL: VIG2 (bits 8:6) at the same stage value the left uses for VIG0 (5), border enabled
  MDPMmioWrite32 (CTL_LAYER (LM_INDEX), BIT24 | (5U << RD_STAGE_SHIFT));
  MDPMmioWrite32 (CTL_LAYER_EXT (LM_INDEX), 0);
  MDPMmioWrite32 (CTL_LAYER_EXT2 (LM_INDEX), 0);
  MDPMmioWrite32 (CTL_LAYER_EXT3 (LM_INDEX), 0);

  // DSC encoder and ping-pong
  DscSetup (gPanelPps);
  // Lahaina's PPB blocks only hold dither registers; XBL programs nothing there

  // INTF_2: start from what XBL programmed on INTF_1 (timing block, command
  // mode trigger, panel format and its swap), then apply our overrides
  {
    UINT32 Off;
    for (Off = 0x04; Off <= 0x68; Off += 4) {
      MDPMmioWrite32 (INTF_BASE + Off, MDPMmioRead32 (INTF_LEFT_BASE + Off));
    }
    MDPMmioWrite32 (INTF_BASE + 0x84, MDPMmioRead32 (INTF_LEFT_BASE + 0x84));
    MDPMmioWrite32 (INTF_BASE + 0x90, MDPMmioRead32 (INTF_LEFT_BASE + 0x90));
    MDPMmioWrite32 (INTF_BASE + 0x94, MDPMmioRead32 (INTF_LEFT_BASE + 0x94));
  }

  // INTF_2: compressed data, bound to PP_2, tearcheck driven by the panel TE pin
  MDPMmioWrite32 (INTF_BASE + INTF_CONFIG2, MDPMmioRead32 (INTF_BASE + INTF_CONFIG2) | BIT0 | BIT12);
  // SPLIT_SEL (bits 19:16) = 0xF means no split mux, as XBL programs it
  MDPMmioWrite32 (INTF_BASE + INTF_MUX, PP_INDEX | (0xFU << 16));
  // The secondary panel XML sets DSITEvSyncSelect=1 (TE on gpio83, mdp_vsync_s)
  MDPMmioWrite32 (INTF_BASE + INTF_TEAR_MDP_VSYNC_SEL, 1);

  // As in the vendor driver: external TE (BIT20) is connected only after the
  // first kickoff, and the HW-generated TE is effectively disabled by a
  // near-maximum sync height.
  MDPMmioWrite32 (INTF_BASE + INTF_TEAR_CHECK_EN, 0);
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_CONFIG_VSYNC, (MDPMmioRead32 (INTF_LEFT_BASE + INTF_SYNC_CONFIG_VSYNC) & 0xFFFFF) | BIT19);
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_CONFIG_HEIGHT, MDPMmioRead32 (INTF_LEFT_BASE + INTF_SYNC_CONFIG_HEIGHT));
  MDPMmioWrite32 (INTF_BASE + INTF_VSYNC_INIT_VAL, PANEL_HEIGHT);
  MDPMmioWrite32 (INTF_BASE + INTF_RD_PTR_IRQ, PANEL_HEIGHT + 1);
  MDPMmioWrite32 (INTF_BASE + INTF_WR_PTR_IRQ, 1);
  MDPMmioWrite32 (INTF_BASE + INTF_START_POS, PANEL_HEIGHT);
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_THRESH, 4 | (4 << 16));
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_WRCOUNT, MDPMmioRead32 (INTF_LEFT_BASE + INTF_SYNC_WRCOUNT));
  MDPMmioWrite32 (INTF_BASE + INTF_AUTOREFRESH, 0);
  MDPMmioWrite32 (INTF_BASE + 0x298, MDPMmioRead32 (INTF_LEFT_BASE + 0x298));
  MDPMmioWrite32 (INTF_BASE + 0x2B8, MDPMmioRead32 (INTF_LEFT_BASE + 0x2B8));
  MDPMmioWrite32 (INTF_BASE + 0x2BC, MDPMmioRead32 (INTF_LEFT_BASE + 0x2BC));
  MDPMmioWrite32 (INTF_BASE + INTF_TEAR_CHECK_EN, 1);

  // CTL: command mode, which blocks are active
  MDPMmioWrite32 (CTL_TOP, BIT17 | (0xFU << 28));
  MDPMmioWrite32 (CTL_INTF_MASTER, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_MERGE_3D_ACTIVE, 0);
  MDPMmioWrite32 (CTL_WB_ACTIVE, 0);
  MDPMmioWrite32 (CTL_CDM_ACTIVE, 0);
  MDPMmioWrite32 (CTL_FETCH_PIPE_ACTIVE, RD_FETCH_BIT);
  MDPMmioWrite32 (CTL_INTF_ACTIVE, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_DSC_ACTIVE, 1U << DSC_INDEX);

  // Flush: sub-block registers first, then the main flush
  MDPMmioWrite32 (CTL_INTF_FLUSH, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_DSC_FLUSH, 1U << DSC_INDEX);
  MDPMmioWrite32 (CTL_PERIPH_FLUSH, 1U << INTF_INDEX);
  MDPMmioWrite32 (CTL_FLUSH, CTL_FLUSH_DMA2 | CTL_FLUSH_LM2 | CTL_FLUSH_CTL | CTL_FLUSH_DSC | CTL_FLUSH_INTF | CTL_FLUSH_PERIPH);
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

  // One kickoff only; hardware autorefresh is enabled by DpuEnableAutoRefresh
  // once the panel has been lit and probed on a quiet link.
  MDPMmioWrite32 (INTF_BASE + INTF_SYNC_CONFIG_VSYNC, MDPMmioRead32 (INTF_BASE + INTF_SYNC_CONFIG_VSYNC) | BIT20);
  MDPMmioWrite32 (CTL_START, 1);
  
  return EFI_SUCCESS;
}

VOID
DpuEnableAutoRefresh (VOID)
{
  // Tearcheck autorefresh (BIT31 enable, frame count 1 as on the live left
  // INTF): the hardware re-triggers every TE, no periodic CTL_START needed.
  MDPMmioWrite32 (INTF_BASE + INTF_AUTOREFRESH, BIT31 | 1);
}
