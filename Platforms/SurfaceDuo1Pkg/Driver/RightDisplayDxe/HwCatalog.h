#ifndef RIGHT_DISPLAY_DXE_HWCATALOG_H_
#define RIGHT_DISPLAY_DXE_HWCATALOG_H_

///////////////////////////////////////////////////////////////////////////////
// Driver configuration
///////////////////////////////////////////////////////////////////////////////

#define RIGHT_FB_BYTES_PER_PIXEL 4
#define RIGHT_FB_ALIGNMENT       SIZE_2MB

#define RIGHT_DISPLAY_VENDOR_GUID \
  {0x5e3b7a14, 0x9c2d, 0x4f6e, {0xa1, 0x58, 0x0d, 0x73, 0xb4, 0x2c, 0x96, 0xe8}}

// Dump hardware state to the debug console at the end of bring-up and hold it
// on screen for RIGHT_DISPLAY_DIAG_HOLD_MS.
#ifndef RIGHT_DISPLAY_DIAG
#define RIGHT_DISPLAY_DIAG 0
#endif

#ifndef RIGHT_DISPLAY_DIAG_HOLD_MS
#define RIGHT_DISPLAY_DIAG_HOLD_MS 20000
#endif

#define MS(x) ((x) * 1000)

#define Be16Pps(p) (((UINT32)(p)[0] << 8) | (p)[1])

///////////////////////////////////////////////////////////////////////////////
// Panel (Duo 1 right panel, DSI controller 1, 1350x1800 command mode, DSC 1.1)
///////////////////////////////////////////////////////////////////////////////

#define PANEL_WIDTH      1350
#define PANEL_HEIGHT     1800
#define PANEL_BITCLK_HZ  500000000ULL

#define PANEL_RESET_GPIO       6
#define PANEL_TE_GPIO          9
#define PANEL_TE_GPIO_FUNC     1 // mdp_vsync

///////////////////////////////////////////////////////////////////////////////
// SoC register blocks (SM8150)
///////////////////////////////////////////////////////////////////////////////

#define GCC_BASE       0x00100000ULL
#define MDP_BASE       0x0AE01000ULL
#define DSI0_CTRL_BASE 0x0AE94000ULL
#define DSI0_PHY_BASE  0x0AE94400ULL
#define DSI1_CTRL_BASE 0x0AE96000ULL
#define DSI1_PHY_BASE  0x0AE96400ULL
#define DISP_CC_BASE   0x0AF00000ULL

#define TLMM_WEST_BASE   0x03100000ULL
#define TLMM_EAST_BASE   0x03500000ULL
#define TLMM_NORTH_BASE  0x03900000ULL
#define TLMM_SOUTH_BASE  0x03D00000ULL

///////////////////////////////////////////////////////////////////////////////
// TLMM (the pin controller is split in tiles, each with its own register map)
///////////////////////////////////////////////////////////////////////////////

#define TLMM_CFG_GPIO_OE BIT9
#define TLMM_IN_OUT_OUT  BIT1

///////////////////////////////////////////////////////////////////////////////
// DISP_CC clocks (offsets from DISP_CC_BASE)
///////////////////////////////////////////////////////////////////////////////

#define DISPCC_MDSS_MDP_CBC    0x200C
#define DISPCC_MDSS_ROT_CBC    0x2014
#define DISPCC_MDSS_VSYNC_CBC  0x2024
#define DISPCC_MDSS_AHB_CBC    0x2080
#define DISPCC_MDSS_CORE_GDSCR 0x3000

#define DISPCC_PCLK1_CBC       0x2008
#define DISPCC_BYTE1_CBC       0x2030
#define DISPCC_BYTE1_INTF_CBC  0x2034
#define DISPCC_ESC0_CBC        0x2038
#define DISPCC_ESC1_CBC        0x203C

#define DISPCC_PCLK1_CMD       0x20B0
#define DISPCC_BYTE1_CMD       0x212C
#define DISPCC_BYTE1_DIV       0x2144
#define DISPCC_ESC1_CMD        0x2160

#define RCG_SRC_SHIFT  8
#define RCG_UPDATE     BIT0

// DISP_CC RCG parent selection (BYTE and PCLK RCGs)
#define DISPCC_SRC_TCXO     0
#define DISPCC_SRC_DSI1_PLL 2

///////////////////////////////////////////////////////////////////////////////
// MDP pipeline map: VIG_2 -> LM_2 -> PP_2 -> DSC_2 -> INTF_2, driven by CTL_1
///////////////////////////////////////////////////////////////////////////////

#define MDP(off) ((UINTN)(MDP_BASE + (off)))

// Block offsets (from MDP_BASE)
#define RD_SSPP_OFF       0x8000
#define RD_CTL_OFF        0x1200
#define INTF_BASE         0x6B000
#define RD_LM_OFF         0x46000
#define RD_PP_OFF         0x71000
#define RD_DSC_OFF        0x80800
#define RD_DSC_CTL_OFF    0x81800

#define SSPP_BASE  RD_SSPP_OFF
#define CTL_BASE   RD_CTL_OFF
#define LM_BASE    RD_LM_OFF
#define PP_BASE    RD_PP_OFF
#define DSC_BASE   RD_DSC_OFF
#define DSC_CTL    RD_DSC_CTL_OFF

// Block indices
#define RD_CTL_INDEX  1
#define LM_INDEX      2
#define PP_INDEX      2
#define DSC_INDEX     2
#define INTF_INDEX    2

// CTL layer / flush bit positions for the blocks above
#define RD_STAGE_SHIFT  6   // VIG_2 field (bits 8:6) of CTL_LAYER_2
#define RD_FLUSH_SSPP   BIT2
#define RD_FLUSH_LM     BIT8

// CTL
#define CTL_LAYER(n)           (CTL_BASE + (n) * 4)
#define CTL_TOP                (CTL_BASE + 0x14)
#define CTL_FLUSH              (CTL_BASE + 0x18)
#define CTL_START              (CTL_BASE + 0x1C)
#define CTL_SW_RESET           (CTL_BASE + 0x30)
#define CTL_LAYER_EXT(n)       (CTL_BASE + 0x40 + (n) * 4)
#define CTL_LAYER_EXT2(n)      (CTL_BASE + 0x70 + (n) * 4)
#define CTL_LAYER_EXT3(n)      (CTL_BASE + 0xA0 + (n) * 4)
#define CTL_MERGE_3D_ACTIVE    (CTL_BASE + 0xE4)
#define CTL_DSC_ACTIVE         (CTL_BASE + 0xE8)
#define CTL_WB_ACTIVE          (CTL_BASE + 0xEC)
#define CTL_INTF_ACTIVE        (CTL_BASE + 0xF4)
#define CTL_CDM_ACTIVE         (CTL_BASE + 0xF8)
#define CTL_DSC_FLUSH          (CTL_BASE + 0x104)
#define CTL_INTF_FLUSH         (CTL_BASE + 0x110)
#define CTL_PERIPH_FLUSH       (CTL_BASE + 0x128)
#define CTL_INTF_MASTER        (CTL_BASE + 0x134)

// CTL_TOP
#define CTL_TOP_MODE_CMD  BIT17

// CTL_FLUSH bits
#define CTL_FLUSH_VIG2    RD_FLUSH_SSPP
#define CTL_FLUSH_LM2     RD_FLUSH_LM
#define CTL_FLUSH_CTL     BIT17
#define CTL_FLUSH_DSC     BIT22
#define CTL_FLUSH_PERIPH  BIT30
#define CTL_FLUSH_INTF    BIT31

// SSPP (VIG_2)
#define SSPP_SRC_SIZE        0x00
#define SSPP_SRC_XY          0x08
#define SSPP_OUT_SIZE        0x0C
#define SSPP_OUT_XY          0x10
#define SSPP_SRC0_ADDR       0x14
#define SSPP_SRC1_ADDR       0x18
#define SSPP_SRC2_ADDR       0x1C
#define SSPP_SRC3_ADDR       0x20
#define SSPP_SRC_YSTRIDE0    0x24
#define SSPP_SRC_YSTRIDE1    0x28
#define SSPP_SRC_FORMAT      0x30
#define SSPP_SRC_UNPACK      0x34
#define SSPP_SRC_OP_MODE     0x38
#define SSPP_DANGER_LUT      0x60
#define SSPP_SAFE_LUT        0x64
#define SSPP_CREQ_LUT_0      0x74
#define SSPP_CREQ_LUT_1      0x78
#define SSPP_PE_C0_LR        0x100
#define SSPP_PE_C0_TB        0x104
#define SSPP_PE_C0_REQ       0x108
#define SSPP_PE_C1C2_LR      0x110
#define SSPP_PE_C1C2_TB      0x114
#define SSPP_PE_C1C2_REQ     0x118
#define SSPP_PE_C3_LR        0x120
#define SSPP_PE_C3_TB        0x124
#define SSPP_PE_C3_REQ       0x128
#define SSPP_QSEED3_OP_MODE  0xA04
#define SSPP_CSC1_OP_MODE    0x1A00

// XRGB8888: B,G,R,X in memory order, 8 bits each, tightly packed, 4 bpp
#define SSPP_FORMAT_XRGB8888  (0xFF | BIT8 | (3 << 12) | (3 << 9) | BIT17)
#define SSPP_UNPACK_XRGB8888  ((3U << 24) | (2 << 16) | (0 << 8) | 1)
#define SSPP_OP_PE_OVERRIDE   BIT31

// Layer mixer
#define LM_OP_MODE    0x00
#define LM_OUT_SIZE   0x04
#define LM_BLEND0_OP  0x20
#define LM_BLEND0_FG  0x24

// Ping-pong
#define PP_DSC_MODE          0xA0
#define PP_DCE_DATA_OUT_SWAP 0xC8

// DSC encoder (relative to DSC_BASE)
#define DSC_COMMON_MODE       0x00
#define DSC_ENC               0x04
#define DSC_PICTURE           0x08
#define DSC_SLICE             0x0C
#define DSC_CHUNK_SIZE        0x10
#define DSC_DELAY             0x14
#define DSC_SCALE_INITIAL     0x18
#define DSC_SCALE_DEC_INT     0x1C
#define DSC_SCALE_INC_INT     0x20
#define DSC_FIRST_LINE_BPG    0x24
#define DSC_BPG_OFFSET        0x28
#define DSC_DSC_OFFSET        0x2C
#define DSC_FLATNESS          0x30
#define DSC_RC_MODEL_SIZE     0x34
#define DSC_RC                0x38
#define DSC_RC_BUF_THRESH(n)  (0x3C + (n) * 4)
#define DSC_RANGE_MIN_QP(n)   (0x74 + (n) * 4)
#define DSC_RANGE_MAX_QP(n)   (0xB0 + (n) * 4)
#define DSC_RANGE_BPG(n)      (0xEC + (n) * 4)

// PP to DSC crossbar: one register per DSC block, holding the PP index
#define DSC_CTL_DSC(n)  ((n) * 4)

// INTF
#define INTF_MUX                 0x25C
#define INTF_TEAR_MDP_VSYNC_SEL  0x280
#define INTF_TEAR_CHECK_EN       0x284
#define INTF_SYNC_CONFIG_VSYNC   0x288
#define INTF_SYNC_CONFIG_HEIGHT  0x28C
#define INTF_VSYNC_INIT_VAL      0x294
#define INTF_SYNC_THRESH         0x29C
#define INTF_START_POS           0x2A0
#define INTF_RD_PTR_IRQ          0x2A4
#define INTF_WR_PTR_IRQ          0x2A8
#define INTF_AUTOREFRESH         0x2B4

///////////////////////////////////////////////////////////////////////////////
// DSI1 controller (relative to DSI1_CTRL_BASE)
///////////////////////////////////////////////////////////////////////////////

#define DSI(off) ((UINTN)(DSI1_CTRL_BASE + (off)))

#define DSI_CTRL                   0x04
#define DSI_DMA_CTRL               0x3C
#define DSI_MDP_CTRL               0x40
#define DSI_MDP_DCS_CMD_CTRL       0x44
#define DSI_DMA_CMD_LENGTH         0x4C
#define DSI_MDP_STREAM0_CTRL       0x58
#define DSI_MDP_STREAM0_TOTAL      0x5C
#define DSI_TRIG_CTRL              0x84
#define DSI_DMA_SW_TRIGGER         0x90
#define DSI_LANE_STATUS            0xA8
#define DSI_LANE_CTRL              0xAC
#define DSI_HS_TIMER_CTRL          0xBC
#define DSI_EOT_PACKET_CTRL        0xCC
#define DSI_INT_CTRL               0x110
#define DSI_SOFT_RESET             0x118
#define DSI_CLK_CTRL               0x11C
#define DSI_PHY_SW_RESET           0x12C
#define DSI_TPG_CTRL               0x15C
#define DSI_TPG_DMA_INIT_VAL       0x17C
#define DSI_MDP_IDLE_CTRL          0x194
#define DSI_MDP_CTRL2              0x1B8
#define DSI_TPG_DMA_FIFO_RESET     0x1EC
#define DSI_CMD_COMPRESSION_CTRL   0x2A8
#define DSI_CMD_COMPRESSION_CTRL2  0x2AC
#define DSI_CMD_COMPRESSION_CTRL3  0x2B0
#define DSI_NULL_INSERTION         0x2B4

#define DSI_DMA_DONE  BIT0

///////////////////////////////////////////////////////////////////////////////
// DSI1 D-PHY / PLL (7nm, relative to DSI1_PHY_BASE)
///////////////////////////////////////////////////////////////////////////////

#define PHY(off) ((UINTN)(DSI1_PHY_BASE + (off)))
#define PLL(off) ((UINTN)(DSI1_PHY_BASE + 0x500 + (off)))

#define PLL_XTAL_HZ     19200000ULL
#define PLL_PRESCALER   2
// Bit clock 500MHz falls in the divider table row PLL post divider 2, PHY post
// divider 1, giving a 1GHz VCO
#define PLL_POST_DIV    2

// Common block
#define CMN_CLK_CFG0         0x10
#define CMN_CLK_CFG1         0x14
#define CMN_RBUF_CTRL        0x1C
#define CMN_VREG_CTRL_0      0x20
#define CMN_CTRL_0           0x24
#define CMN_CTRL_2           0x2C
#define CMN_CTRL_3           0x30
#define CMN_PLL_CNTRL        0x3C
#define CMN_LANE_CTRL0       0xA0
#define CMN_TIMING_CTRL(n)   (0xB4 + 4 * (n))
#define CMN_HSTX_STR         0xEC
#define CMN_RESCODE_TOP      0xF4
#define CMN_RESCODE_BOT      0xF8
#define CMN_LPTX_STR         0x100
#define CMN_PEMPH_0          0x104
#define CMN_STR_SWI_CAL      0x10C
#define CMN_VREG_CTRL_1      0x110
#define CMN_PHY_STATUS       0x140

// Per-lane block (lanes 0..3 data, 4 clock)
#define LANE(n, off)  (0x200 + 0x80 * (n) + (off))

#endif
