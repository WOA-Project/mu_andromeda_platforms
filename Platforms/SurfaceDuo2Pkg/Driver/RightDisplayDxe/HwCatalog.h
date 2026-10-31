#ifndef RIGHT_DISPLAY_DXE_HWCATALOG_H_
#define RIGHT_DISPLAY_DXE_HWCATALOG_H_

#define RIGHT_FB_BYTES_PER_PIXEL 4
#define RIGHT_FB_ALIGNMENT       SIZE_2MB

#define RIGHT_DISPLAY_VENDOR_GUID \
  {0x5e3b7a14, 0x9c2d, 0x4f6e, {0xa1, 0x58, 0x0d, 0x73, 0xb4, 0x2c, 0x96, 0xe8}}

///////////////////////////////////////////////////////////////////////////////

// Command packets sent by memory DMA (buffer in the last 1 KiB of the display
// reservation, DMA_CMD_OFFSET / DMA_FIFO_CTRL) instead of the embedded FIFO
#ifndef RIGHT_DISPLAY_EXP0_MEM_DMA
#define RIGHT_DISPLAY_EXP0_MEM_DMA 1
#endif

// Bisection: the DPU fetches the left display's framebuffer instead of ours.
// The right panel then mirrors the left one. If it does, our own framebuffer
// memory (allocation, cache or SMMU mapping) is the fault; if it stays black,
// the fault is downstream of the framebuffer fetch.
#ifndef RIGHT_DISPLAY_MIRROR_LEFT
#define RIGHT_DISPLAY_MIRROR_LEFT 0
#endif

// As XBL does with DSIForceCmdInVideoHS: reset the panel, start the DPU pipe
// and autorefresh, and only then send the INIT sequence and PPS.
#ifndef RIGHT_DISPLAY_INIT_AFTER_PIPE
#define RIGHT_DISPLAY_INIT_AFTER_PIPE 0
#endif

// Dump hardware state to the debug console at the end of bring-up and hold it
// on screen for RIGHT_DISPLAY_DIAG_HOLD_MS.
#ifndef RIGHT_DISPLAY_DIAG
#define RIGHT_DISPLAY_DIAG 0
#endif

#ifndef RIGHT_DISPLAY_DIAG_HOLD_MS
#define RIGHT_DISPLAY_DIAG_HOLD_MS 20000
#endif

///////////////////////////////////////////////////////////////////////////////

//
// Panel (Elgin right panel, surface_elgin_dsi1_r2_cmd_mp.dtsi, 60Hz timing)
//
#define PANEL_WIDTH      1344
#define PANEL_HEIGHT     1892
#define PANEL_BITCLK_HZ  760000000ULL

#define PANEL_RESET_GPIO      51
#define PANEL_TE_GPIO         83
#define PANEL_TE_GPIO_FUNC    1 // mdp_vsync (SM8350 TLMM: gpio83 f1)
#define PANEL_TE_VSYNC_SOURCE 1 // qcom,panel-te-source = <1> for the DSI1 display (lahaina-sde-display.dtsi)
#define PANEL_T_CLK_POST      0x0B
#define PANEL_T_CLK_PRE       0x17

//
// SoC register blocks (SM8350)
//
#define DSI0_CTRL_BASE 0x0AE94000ULL
#define DSI0_PHY_BASE  0x0AE94400ULL
#define DSI1_CTRL_BASE 0x0AE96000ULL
#define DSI1_PHY_BASE  0x0AE96400ULL
#define DISP_CC_BASE   0x0AF00000ULL
#define TLMM_BASE      0x0F100000ULL

#define MDP_BASE       0x0AE01000ULL

#define MS(x) ((x)*1000)

#define RD_CTL_INDEX      1
#define RD_CTL_OFF        0x16000
#define RD_LM_OFF         0x45000
#define RD_SSPP_OFF       0x6000
#define RD_PP_OFF         0x6A000
#define RD_DSC_CORE_OFF   0x80000
#define RD_DSC_ENC_OFF    0x80200
#define RD_DSC_CTL_OFF    0x80F80
#define RD_STAGE_SHIFT    3
#define RD_FETCH_BIT      BIT17
#define RD_FLUSH_SSPP     BIT1
#define RD_FLUSH_LM       BIT7

///////////////////////////////////////////////////////////////////////////////

#define GCC_BASE          0x00100000ULL
#define GCC_DISP_AHB_CBC  0x27004
#define GCC_DISP_HF_AXI_CBC 0x2700C
#define GCC_DISP_SF_AXI_CBC 0x27014

#define DISPCC_BYTE1_CMD  0x2128
#define DISPCC_ESC1_CMD   0x215C
#define DISPCC_PCLK1_CMD  0x20AC
#define DISPCC_BYTE1_DIV  0x2140

#define DISPCC_MDSS_AHB_CBC   0x207C
#define DISPCC_MDSS_MDP_CBC   0x200C
#define DISPCC_MDSS_VSYNC_CBC 0x2024
#define DISPCC_ESC0_CBC       0x2038
#define DISPCC_BYTE1_CBC      0x2030
#define DISPCC_BYTE1_INTF_CBC 0x2034
#define DISPCC_ESC1_CBC       0x203C
#define DISPCC_PCLK1_CBC      0x2008

#define DISPCC_MDSS_CORE_GDSCR 0x3000
#define DISPCC_MDSS_ROT_CBC    0x2014
#define DISPCC_RSCC_VSYNC_CBC  0x4008
#define DISPCC_RSCC_AHB_CBC    0x400C

#define RCG_SRC_SHIFT 8
#define RCG_UPDATE    BIT0

///////////////////////////////////////////////////////////////////////////////


#define MDP(off) ((UINTN)(MDP_BASE + (off)))

#define CTL_BASE  RD_CTL_OFF
#define LM_BASE   RD_LM_OFF
#define LM_LEFT_BASE 0x44000 // LM_0, programmed by XBL for the left display
#define SSPP_BASE RD_SSPP_OFF
#define PP_BASE   RD_PP_OFF
#define INTF_BASE 0x36000
#define INTF_LEFT_BASE 0x35000
#define DSC_BASE  RD_DSC_CORE_OFF
#define DSC_ENC   RD_DSC_ENC_OFF
#define DSC_CTL   RD_DSC_CTL_OFF

#define CTL_INDEX RD_CTL_INDEX
#define LM_INDEX  RD_CTL_INDEX
#define PP_INDEX  RD_CTL_INDEX
#define DSC_INDEX RD_CTL_INDEX
#define INTF_INDEX 2

// CTL
#define CTL_LAYER(n)         (CTL_BASE + 0x00 + (n)*4)
#define CTL_LAYER_EXT2(n)    (CTL_BASE + 0x70 + (n)*4)
#define CTL_TOP              (CTL_BASE + 0x14)
#define CTL_FLUSH            (CTL_BASE + 0x18)
#define CTL_START            (CTL_BASE + 0x1C)
#define CTL_SW_RESET         (CTL_BASE + 0x30)
#define CTL_LAYER_EXT(n)     (CTL_BASE + 0x40 + (n)*4)
#define CTL_LAYER_EXT3(n)    (CTL_BASE + 0xA0 + (n)*4)
#define CTL_MERGE_3D_ACTIVE  (CTL_BASE + 0xE4)
#define CTL_DSC_ACTIVE       (CTL_BASE + 0xE8)
#define CTL_WB_ACTIVE        (CTL_BASE + 0xEC)
#define CTL_INTF_ACTIVE      (CTL_BASE + 0xF4)
#define CTL_CDM_ACTIVE       (CTL_BASE + 0xF8)
#define CTL_FETCH_PIPE_ACTIVE (CTL_BASE + 0xFC)
#define CTL_DSC_FLUSH        (CTL_BASE + 0x104)
#define CTL_INTF_FLUSH       (CTL_BASE + 0x110)
#define CTL_PERIPH_FLUSH     (CTL_BASE + 0x128)
#define CTL_INTF_MASTER      (CTL_BASE + 0x134)
#define CTL_FLUSH_PERIPH     BIT30

#define CTL_FLUSH_DMA2  RD_FLUSH_SSPP
#define CTL_FLUSH_LM2   RD_FLUSH_LM
#define CTL_FLUSH_CTL   BIT17
#define CTL_FLUSH_DSC   BIT22
#define CTL_FLUSH_INTF  BIT31

// SSPP (DMA2)
#define SSPP_SRC_SIZE         0x00
#define SSPP_SRC_XY           0x08
#define SSPP_OUT_SIZE         0x0C
#define SSPP_OUT_XY           0x10
#define SSPP_SRC0_ADDR        0x14
#define SSPP_SRC_YSTRIDE0     0x24
#define SSPP_SRC_YSTRIDE1     0x28
#define SSPP_SRC_FORMAT       0x30
#define SSPP_SRC_UNPACK       0x34
#define SSPP_SRC_OP_MODE      0x38
#define SSPP_MULTIRECT_OPMODE 0x170
#define SSPP_DANGER_LUT       0x60
#define SSPP_SAFE_LUT         0x64
#define SSPP_CREQ_LUT_0       0x74
#define SSPP_CREQ_LUT_1       0x78
#define SSPP_PE_C0_REQ        0x108
#define SSPP_PE_C1C2_REQ      0x118
#define SSPP_PE_C3_REQ        0x128

// XRGB8888: B,G,R,X in memory order, 8 bits each, tightly packed, 4 bpp
// BIT8 (alpha enable) is set on the left pipe as programmed by XBL
#define SSPP_FORMAT_XRGB8888   (0xFF | BIT8 | (3 << 12) | (3 << 9) | BIT17)
#define SSPP_UNPACK_XRGB8888   ((3U << 24) | (2 << 16) | (0 << 8) | 1)
#define SSPP_OP_PE_OVERRIDE    BIT31

// Layer mixer
#define LM_OP_MODE     0x00
#define LM_OUT_SIZE    0x04
#define LM_BLEND0_OP   0x20
#define LM_BLEND0_FG   0x24
#define LM_BLEND0_BG   0x28

// Ping-pong
#define PP_DSC_MODE           0xA0
#define PP_DCE_DATA_OUT_SWAP  0xC8

// INTF
#define INTF_CONFIG2            0x60
#define INTF_MUX                0x25C
#define INTF_TEAR_MDP_VSYNC_SEL 0x280
#define INTF_TEAR_CHECK_EN      0x284
#define INTF_SYNC_CONFIG_VSYNC  0x288
#define INTF_SYNC_CONFIG_HEIGHT 0x28C
#define INTF_SYNC_WRCOUNT       0x290
#define INTF_VSYNC_INIT_VAL     0x294
#define INTF_SYNC_THRESH        0x29C
#define INTF_START_POS          0x2A0
#define INTF_RD_PTR_IRQ         0x2A4
#define INTF_WR_PTR_IRQ         0x2A8
#define INTF_AUTOREFRESH        0x2B4

// DSC encoder (relative to DSC_ENC)
#define DSC_CMN_MAIN_CNF         (DSC_BASE + 0x00)
#define ENC_DF_CTRL              0x00
#define ENC_MAIN_CONF            0x30
#define ENC_PICTURE_SIZE         0x34
#define ENC_SLICE_SIZE           0x38
#define ENC_MISC_SIZE            0x3C
#define ENC_HRD_DELAYS           0x40
#define ENC_RC_SCALE             0x44
#define ENC_RC_SCALE_INC_DEC     0x48
#define ENC_RC_OFFSETS_1         0x4C
#define ENC_RC_OFFSETS_2         0x50
#define ENC_RC_OFFSETS_3         0x54
#define ENC_RC_OFFSETS_4         0x58
#define ENC_FLATNESS_QP          0x5C
#define ENC_RC_MODEL_SIZE        0x60
#define ENC_RC_CONFIG            0x64
#define ENC_RC_BUF_THRESH(n)     (0x68 + (n)*4)
#define ENC_RC_MIN_QP(n)         (0x78 + (n)*4)
#define ENC_RC_MAX_QP(n)         (0x84 + (n)*4)
#define ENC_RC_RANGE_BPG(n)      (0x90 + (n)*4)
// DSC wrapper (relative to DSC_CTL)
#define DSC_CTL_PP_MUX           0x00
#define DSC_CTL_CFG              0x04

///////////////////////////////////////////////////////////////////////////////

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

///////////////////////////////////////////////////////////////////////////////

#define PHY(off) ((UINTN)(DSI1_PHY_BASE + (off)))
#define PLL(off) ((UINTN)(DSI1_PHY_BASE + 0x500 + (off)))

#define CMN_REVISION_ID0 0x00
#define CMN_CLK_CFG0     0x10
#define CMN_CLK_CFG1     0x14
#define CMN_RBUF_CTRL    0x1C
#define CMN_VREG_CTRL_0  0x20
#define CMN_CTRL_0       0x24
#define CMN_CTRL_2       0x2C
#define CMN_CTRL_3       0x30
#define CMN_LANE_CFG0    0x34
#define CMN_LANE_CFG1    0x38
#define CMN_PLL_CNTRL    0x3C
#define CMN_LANE_CTRL0   0xA0
#define CMN_TIMING_CTRL(n) (0xB4 + 4 * (n))
#define CMN_HSTX_STR     0xEC
#define CMN_RESCODE_TOP  0xF4
#define CMN_RESCODE_BOT  0xF8
#define CMN_LPTX_STR     0x100
#define CMN_PEMPH_0      0x104
#define CMN_STR_SWI_CAL  0x10C
#define CMN_VREG_CTRL_1  0x110
#define CMN_CTRL_4       0x114
#define CMN_DIGTOP_SPARE4 0x128
#define CMN_PHY_STATUS   0x140

#define LANE(n, off) (0x200 + 0x80 * (n) + (off))

#define PLL_XTAL_HZ 19200000ULL

///////////////////////////////////////////////////////////////////////////////

// PMIC device indices (XBL MDPPlatformLib): A = PMK8350, B = PM8350, C = PM8350C, D = PM8350B
#define PMIC_A  0
#define PMIC_B  1
#define PMIC_C  2
#define PMIC_D  3

// EFI_PM_LDO_n enumerators start at LDO_1 = 0
#define PM_LDO(n)  ((n) - 1)

#define PM_ON  1

//
// LN_BB_CLK3 (RPMH_LN_BB_CLK3, "ext_trig_clk") is enabled by the vendor driver
// right before the panel is enabled. PmicDxe casts the protocol clock type
// straight to its internal pm_clk_type, whose BB_3 (== LN_BB_CLK3) value is 9
// (SLEEP 0, XO 1, RF_1..5 2..6, LN_BB/BB_1 7, BB_2 8, BB_3 9), which does not
// match the EFI_PM_CLK_BUFF_TYPE enumerator names, so the number is used here.
//
#define PM_CLK_BB_3_RAW   9
#define EXT_TRIG_CLK_PMIC PMIC_A

///////////////////////////////////////////////////////////////////////////////

#define TLMM_GPIO_CFG(n)    (TLMM_BASE + 0x1000ULL * (n))
#define TLMM_GPIO_IN_OUT(n) (TLMM_BASE + 0x1000ULL * (n) + 0x4)

#define TLMM_CFG_GPIO_OE BIT9
#define TLMM_IN_OUT_OUT  BIT1

#endif
