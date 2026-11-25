/** @file
  Best-effort SM8150 DSI/PHY initialization for PrePi.

  Register programming and command packet framing follow the modified
  Qualcomm XBL HALDSILib implementation. Panel command bytes are taken from
  the LG_QHD sequences embedded in DisplayDxe.efi. The panel reset GPIOs are
  deliberately not touched.

  SPDX-License-Identifier: MIT
**/

#include <PiPei.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/QcomUefiCfgMemoryMapHelperLib.h>
#include <Library/TimerLib.h>

#include "DisplayResumeConfig.h"
#include "DisplayResumeScanout.h"
#include "DisplayResumeScanoutRoute.h"

#define DISPLAY_RESUME_MDSS_BASE                  0x0AE00000U
#define DISPLAY_RESUME_DSI0_BASE                 (DISPLAY_RESUME_MDSS_BASE + 0x00094000U)
#define DISPLAY_RESUME_INTF1_BASE                (DISPLAY_RESUME_MDSS_BASE + 0x0006B800U)
#define DISPLAY_RESUME_INTF1_TEAR_BASE           (DISPLAY_RESUME_MDSS_BASE + 0x0006BA80U)
#define DISPLAY_RESUME_DISP_CC_BASE              (DISPLAY_RESUME_MDSS_BASE + 0x00100000U)
#define DISPLAY_RESUME_DSI1_BASE                 (DISPLAY_RESUME_MDSS_BASE + 0x00096000U)
#define DISPLAY_RESUME_INTF2_BASE                (DISPLAY_RESUME_MDSS_BASE + 0x0006C000U)
#define DISPLAY_RESUME_INTF2_TEAR_BASE           (DISPLAY_RESUME_MDSS_BASE + 0x0006C280U)

#define DSI_REG_CTRL                              0x004U
#define DSI_REG_STATUS                            0x008U
#define DSI_REG_COMMAND_MODE_DMA_CTRL             0x03CU
#define DSI_REG_COMMAND_MODE_MDP_CTRL             0x040U
#define DSI_REG_COMMAND_MODE_MDP_DCS_CMD_CTRL     0x044U
#define DSI_REG_DMA_CMD_OFFSET                    0x048U
#define DSI_REG_DMA_CMD_LENGTH                    0x04CU
#define DSI_REG_COMMAND_MODE_MDP_STREAM0_CTRL     0x058U
#define DSI_REG_COMMAND_MODE_MDP_STREAM0_TOTAL    0x05CU
#define DSI_REG_TRIG_CTRL                         0x084U
#define DSI_REG_CMD_MODE_DMA_SW_TRIGGER           0x090U
#define DSI_REG_LANE_CTRL                         0x0ACU
#define DSI_REG_HS_TIMER_CTRL                     0x0BCU
#define DSI_REG_EOT_PACKET_CTRL                   0x0CCU
#define DSI_REG_INT_CTRL                          0x110U
#define DSI_REG_SOFT_RESET                        0x118U
#define DSI_REG_CLK_CTRL                          0x11CU
#define DSI_REG_PHY_SW_RESET                      0x12CU
#define DSI_REG_TEST_PATTERN_GEN_CTRL             0x15CU
#define DSI_REG_TEST_PATTERN_GEN_CMD_DMA_INIT_VAL 0x17CU
#define DSI_REG_COMMAND_MODE_MDP_IDLE_CTRL        0x194U
#define DSI_REG_TPG_DMA_FIFO_RESET                0x1ECU
#define DSI_REG_COMMAND_MODE_MDP_CTRL2            0x1B8U
#define DSI_REG_COMMAND_COMPRESSION_MODE_CTRL     0x2A8U
#define DSI_REG_COMMAND_COMPRESSION_MODE_CTRL2    0x2ACU
#define DSI_REG_COMMAND_COMPRESSION_MODE_CTRL3    0x2B0U
#define DSI_REG_COMMAND_MODE_NULL_INSERTION_CTRL  0x2B4U

#define DSI_PANEL_WIDTH                           1350U
#define DSI_PANEL_HEIGHT                          1800U
#define DSI_DSC_SLICE_WIDTH                       675U
#define DSI_DSC_BPP                               8U
#define DSI_DSC_SLICES_PER_LINE                   (DSI_PANEL_WIDTH / DSI_DSC_SLICE_WIDTH)
#define DSI_DSC_SLICES_PER_PACKET                 DSI_DSC_SLICES_PER_LINE
#define DSI_DSC_BYTES_PER_SLICE                   ((DSI_DSC_SLICE_WIDTH * DSI_DSC_BPP + 7U) / 8U)
#define DSI_DSC_LINE_BYTES                        (DSI_DSC_BYTES_PER_SLICE * DSI_DSC_SLICES_PER_LINE)
#define DSI_DSC_WIDTH_TOTAL                       ((DSI_DSC_LINE_BYTES + 2U) / 3U)
#define DSI_DSC_WORD_COUNT                        (DSI_DSC_SLICES_PER_PACKET * DSI_DSC_SLICE_WIDTH + 1U)
#define DSI_DSC_PACKETS_PER_LINE                  ((DSI_DSC_SLICES_PER_LINE / DSI_DSC_SLICES_PER_PACKET) / 2U)
#define DSI_DSC_EOL_BYTES                         (DSI_DSC_LINE_BYTES % 3U)

#define DISP_CC_MDSS_PCLK0_CBCR                    (DISPLAY_RESUME_DISP_CC_BASE + 0x2004U)
#define DISP_CC_MDSS_BYTE0_CBCR                    (DISPLAY_RESUME_DISP_CC_BASE + 0x2028U)
#define DISP_CC_MDSS_BYTE0_INTF_CBCR               (DISPLAY_RESUME_DISP_CC_BASE + 0x202CU)
#define DISP_CC_MDSS_ESC0_CBCR                     (DISPLAY_RESUME_DISP_CC_BASE + 0x2038U)
#define DISP_CC_MDSS_PCLK0_CMD_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x2098U)
#define DISP_CC_MDSS_PCLK0_CFG_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x209CU)
#define DISP_CC_MDSS_BYTE0_CMD_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x2110U)
#define DISP_CC_MDSS_BYTE0_CFG_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x2114U)
#define DISP_CC_MDSS_BYTE0_DIV_CDIVR               (DISPLAY_RESUME_DISP_CC_BASE + 0x2128U)
#define DISP_CC_MDSS_ESC0_CMD_RCGR                 (DISPLAY_RESUME_DISP_CC_BASE + 0x2148U)
#define DISP_CC_MDSS_ESC0_CFG_RCGR                 (DISPLAY_RESUME_DISP_CC_BASE + 0x214CU)
#define DISP_CC_MDSS_PCLK1_CBCR                    (DISPLAY_RESUME_DISP_CC_BASE + 0x2008U)
#define DISP_CC_MDSS_BYTE1_CBCR                    (DISPLAY_RESUME_DISP_CC_BASE + 0x2030U)
#define DISP_CC_MDSS_BYTE1_INTF_CBCR               (DISPLAY_RESUME_DISP_CC_BASE + 0x2034U)
#define DISP_CC_MDSS_ESC1_CBCR                     (DISPLAY_RESUME_DISP_CC_BASE + 0x203CU)
#define DISP_CC_MDSS_PCLK1_CMD_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x20B0U)
#define DISP_CC_MDSS_PCLK1_CFG_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x20B4U)
#define DISP_CC_MDSS_BYTE1_CMD_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x212CU)
#define DISP_CC_MDSS_BYTE1_CFG_RCGR                (DISPLAY_RESUME_DISP_CC_BASE + 0x2130U)
#define DISP_CC_MDSS_BYTE1_DIV_CDIVR               (DISPLAY_RESUME_DISP_CC_BASE + 0x2144U)
#define DISP_CC_MDSS_ESC1_CMD_RCGR                 (DISPLAY_RESUME_DISP_CC_BASE + 0x2160U)
#define DISP_CC_MDSS_ESC1_CFG_RCGR                 (DISPLAY_RESUME_DISP_CC_BASE + 0x2164U)

#define DSI_STATUS_COMMAND_DMA_BUSY                BIT1
#define DSI_COMMAND_DMA_DONE                      BIT0
#define DSI_COMMAND_DMA_DONE_ACK                  BIT0
#define DSI_CTRL_ENABLE_CMD                       0x000001F5U
#define DSI_CMD_DMA_EMBEDDED_LP                   (BIT28 | BIT26)
#define DSI_TPG_CMD_DMA_FIFO                      0x00030006U
#define DSI_TPG_CMD_DMA_MASK                      0x00030006U
#define DSI_DMA_FIFO_SIZE                         64U
#define DSI_DMA_BUFFER_SIZE                       EFI_PAGE_SIZE
#define DSI_EMBEDDED_SHORT_LAST                   0x80U
#define DSI_EMBEDDED_LONG_LAST                    0xC0U
#define DSI_COMMAND_TIMEOUT_US                    100000U
#define DSI_COMMAND_DMA_TIMEOUT_US                16670U
#define DSI_COMMAND_DMA_POLL_US                   100U
#define DSI_CLOCK_TIMEOUT_US                       1000U
#define DSI_PLL_TIMEOUT_US                        15000U
#define DSI_REFGEN_TIMEOUT_US                     5000U

#define DISP_CC_CLOCK_ENABLE                      BIT0
#define DISP_CC_CLOCK_HW_CONTROL                  BIT1
#define DISP_CC_CLOCK_OFF                         BIT31
#define DISP_CC_ROOT_UPDATE                       BIT0
#define DISP_CC_ROOT_ENABLE                       BIT1
#define DISP_CC_ROOT_OFF                          BIT31
#define DISP_CC_CFG_SOURCE_MASK                   0x00000700U
#define DISP_CC_CFG_SOURCE_SHIFT                  8
#define DISP_CC_CFG_DIVIDER_MASK                  0x0000001FU
#define DISP_CC_CFG_MODE_MASK                     0x00003000U
#define DISP_CC_CFG_HW_CLK_CONTROL                BIT20
#define DSI_INTERNAL_CLOCKS_ON                    0x0000023FU

#define DISPLAY_TLMM_BASE                         0x03000000U
#define DISPLAY_TLMM_GPIO8_OFFSET                 0x8000U
#define DISPLAY_TLMM_GPIO9_OFFSET                 0x9000U
#define DISPLAY_TLMM_GPIO_ID_STATUS               0x10U
#define DISPLAY_TLMM_EGPIO_PRESENT                BIT11
#define DISPLAY_TLMM_EGPIO_ENABLE                 BIT12
#define DISPLAY_TLMM_GPIO_CFG_MASK                0x17FFU
#define DISPLAY_TLMM_TE_CONFIG                    0x4U

#define DSI_PHY_CMN_CTRL_0                        0x024U
#define DSI_PHY_CMN_CTRL_2                        0x02CU
#define DSI_PHY_CMN_CTRL_3                        0x030U
#define DSI_PHY_CMN_PLL_CTRL                      0x03CU
#define DSI_PHY_CMN_RBUF_CTRL                     0x01CU
#define DSI_PHY_CMN_VREG_CTRL_0                   0x020U
#define DSI_PHY_CMN_VREG_CTRL_1                   0x110U
#define DSI_PHY_CMN_CLK_CFG_0                     0x010U
#define DSI_PHY_CMN_CLK_CFG_1                     0x014U
#define DSI_PHY_CMN_LANE_CTRL_0                   0x0A0U
#define DSI_PHY_CMN_LANE_CTRL_1                   0x0A4U
#define DSI_PHY_CMN_PHY_STATUS                    0x140U
#define DSI_PHY_CMN_LANE_CFG_0                    0x034U
#define DSI_PHY_CMN_LANE_CFG_1                    0x038U
#define DSI_PHY_CMN_TIMING_CTRL_0                 0x0B4U

#define DSI_PHY_PLL_CORE_INPUT_OVERRIDE           0x0BCU
#define DSI_PHY_PLL_CORE_OVERRIDE                 0x0B8U
#define DSI_PHY_PLL_SYSTEM_MUXES                  0x028U
#define DSI_PHY_PLL_ANALOG_CONTROLS_TWO           0x004U
#define DSI_PHY_PLL_ANALOG_CONTROLS_THREE         0x010U
#define DSI_PHY_PLL_ANALOG_CONTROLS_FIVE          0x018U
#define DSI_PHY_PLL_DSM_DIVIDER                   0x020U
#define DSI_PHY_PLL_FEEDBACK_DIVIDER              0x024U
#define DSI_PHY_PLL_CALIBRATION_SETTINGS          0x044U
#define DSI_PHY_PLL_BAND_SEL_CAL_SETTINGS_THREE   0x068U
#define DSI_PHY_PLL_FREQ_DETECT_SETTINGS_ONE      0x078U
#define DSI_PHY_PLL_PFILT                         0x090U
#define DSI_PHY_PLL_IFILT                         0x094U
#define DSI_PHY_PLL_OUTDIV                        0x0A8U
#define DSI_PHY_PLL_PLL_DIGITAL_TIMERS_TWO        0x0C8U
#define DSI_PHY_PLL_DECIMAL_DIV_START             0x0E0U
#define DSI_PHY_PLL_FRAC_DIV_START_LOW            0x0E4U
#define DSI_PHY_PLL_FRAC_DIV_START_MID            0x0E8U
#define DSI_PHY_PLL_FRAC_DIV_START_HIGH           0x0ECU
#define DSI_PHY_PLL_COMMON_STATUS_ONE             0x1B0U
#define DSI_PHY_PLL_PLL_OUTDIV_RATE                0x154U
#define DSI_PHY_PLL_PLL_LOCKDET_RATE_1             0x158U
#define DSI_PHY_PLL_PLL_LOCK_DELAY                 0x194U
#define DSI_PHY_PLL_PLL_PROP_GAIN_RATE_1           0x160U
#define DSI_PHY_PLL_PLL_BAND_SEL_RATE_1             0x168U
#define DSI_PHY_PLL_PLL_INT_GAIN_IFILT_BAND_1      0x16CU
#define DSI_PHY_PLL_PLL_FL_INT_GAIN_PFILT_BAND_1   0x170U
#define DSI_PHY_PLL_PLL_LOCK_OVERRIDE              0x180U
#define DSI_PHY_PLL_GEAR_BAND_SELECT_CONTROLS     0x234U
#define DSI_PHY_PLL_VCO_CONFIG_1                  0x240U
#define DSI_PHY_PLL_CLOCK_INVERTERS_1             0x248U
#define DSI_PHY_PLL_CMODE_1                       0x250U
#define DSI_PHY_PLL_ANALOG_CONTROLS_FIVE_1        0x258U

typedef struct {
  UINT8  Type;
  UINT8  Length;
  UINT8  Payload[128];
} DSI_PANEL_PACKET;

typedef struct {
  UINT8  Controller;
  UINT8  Interface;
  UINTN  DsiBase;
  UINTN  InterfaceBase;
  UINTN  TearBase;
} DSI_PANEL_ROUTE;

STATIC CONST DSI_PANEL_ROUTE mPrimaryPanelRouteByRevision[2] = {
  {
    0,
    1,
    DISPLAY_RESUME_DSI0_BASE,
    DISPLAY_RESUME_INTF1_BASE,
    DISPLAY_RESUME_INTF1_TEAR_BASE
  },
  {
    0,
    1,
    DISPLAY_RESUME_DSI0_BASE,
    DISPLAY_RESUME_INTF1_BASE,
    DISPLAY_RESUME_INTF1_TEAR_BASE
  }
};

STATIC CONST DSI_PANEL_ROUTE mSecondaryPanelRoute = {
  1,
  2,
  DISPLAY_RESUME_DSI1_BASE,
  DISPLAY_RESUME_INTF2_BASE,
  DISPLAY_RESUME_INTF2_TEAR_BASE
};

//
// Both LG_QHD profiles use four D-PHY data lanes, RGB888, and a 500 MHz bit
// clock. The PPS packet is panel-side DSC setup only.
//
STATIC CONST DSI_PANEL_PACKET mPanelInit[] = {
  { 0x07, 2, { 0x11, 0x00 } },
  { 0xFF, 1, { 0x78 } },
  { 0x0A, 128, {
    0x11, 0x00, 0x00, 0x89, 0x30, 0x80, 0x07, 0x08, 0x05, 0x46, 0x03, 0x84, 0x02, 0xA3, 0x02, 0xA3,
    0x02, 0x00, 0x02, 0x51, 0x00, 0x20, 0x5F, 0x4B, 0x00, 0x09, 0x00, 0x0C, 0x00, 0x1C, 0x00, 0x18,
    0x18, 0x00, 0x10, 0xF0, 0x03, 0x0C, 0x20, 0x00, 0x06, 0x0B, 0x0B, 0x33, 0x0E, 0x1C, 0x2A, 0x38,
    0x46, 0x54, 0x62, 0x69, 0x70, 0x77, 0x79, 0x7B, 0x7D, 0x7E, 0x01, 0x02, 0x01, 0x00, 0x09, 0x40,
    0x09, 0xBE, 0x19, 0xFC, 0x19, 0xFA, 0x19, 0xF8, 0x1A, 0x38, 0x1A, 0x78, 0x1A, 0xB6, 0x2A, 0xF6,
    0x2B, 0x34, 0x2B, 0x74, 0x3B, 0x74, 0x6B, 0xF4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
  } },
  { 0x15, 2, { 0xB0, 0xAC } },
  { 0x39, 5, { 0x2A, 0x00, 0x00, 0x05, 0x45 } },
  { 0x39, 5, { 0x2B, 0x00, 0x00, 0x07, 0x07 } },
  { 0x15, 2, { 0x26, 0x02 } },
  { 0x39, 3, { 0x44, 0x03, 0x84 } },
  { 0x05, 1, { 0x35 } },
  { 0x39, 3, { 0x51, 0x01, 0x47 } },
  { 0x39, 3, { 0x53, 0x0C, 0x30 } },
  { 0x39, 7, { 0x55, 0x04, 0x70, 0xDB, 0x00, 0x78, 0xDB } },
  { 0x15, 2, { 0xEE, 0x24 } },
  { 0x15, 2, { 0xFB, 0xAC } },
  { 0x15, 2, { 0xB0, 0xCA } },
  { 0x05, 1, { 0x11 } },
  { 0xFF, 1, { 0x82 } },
  { 0x05, 1, { 0x29 } },
  { 0xFF, 1, { 0x82 } }
};

STATIC CONST DSI_PANEL_PACKET mPanelOldRevisionPacket = {
  0x15, 2, { 0x57, 0x10 }
};

STATIC CONST DSI_PANEL_PACKET mPanelOff[] = {
  { 0x05, 1, { 0x28 } },
  { 0x05, 1, { 0x10 } }
};

DISPLAY_RESUME_DSI_ROUTE  gDisplayResumeDsiRoute;
STATIC UINT8              *mDsiDmaBuffer;
STATIC BOOLEAN            mDsiCommandPending;
STATIC UINT32             mPanelRevision;
STATIC BOOLEAN            mDsiInitialized;
STATIC BOOLEAN            mSecondaryDsi;
STATIC BOOLEAN            mDsiHostReady[2];
STATIC BOOLEAN            mDsiPhyTouched[2];
STATIC UINT32             mDsiRootsTouched[2];
STATIC UINT32             mDsiBranchesTouched[2];
STATIC BOOLEAN            mPanelSequenceStarted[2];
STATIC UINTN              mTeAddress[2];
STATIC UINT32             mTeOriginalConfig[2];
STATIC UINTN              mDsiBase;
STATIC UINTN              mPhyCommonBase;
STATIC UINTN              mPhyPllBase;
STATIC UINTN              mPhyLaneBase;

BOOLEAN
DisplayResumeDsiCommandPending (
  VOID
  )
{
  return mDsiCommandPending;
}

STATIC
UINT32
DsiRead (
  IN UINTN  Address
  )
{
  return MmioRead32 (Address);
}

STATIC
VOID
DsiWrite (
  IN UINTN   Address,
  IN UINT32  Value
  )
{
  MmioWrite32 (Address, Value);
}

STATIC
EFI_STATUS
DsiConfigureTeGpio (
  VOID
  )
{
  STATIC CONST UINT32  TileOffsets[] = { 0x900000U, 0xD00000U, 0x500000U, 0x100000U };
  UINTN                Index;
  UINTN                Address;
  UINT32               Id;
  UINT32               Value;
  UINTN                Controller;

  // XBL finds the GPIO tile by its ID status. Both TE pins use function 1,
  // input, no pull, 2 mA.
  for (Index = 0; Index < ARRAY_SIZE (TileOffsets); Index++) {
    Address = DISPLAY_TLMM_BASE + TileOffsets[Index] +
              (mSecondaryDsi ? DISPLAY_TLMM_GPIO9_OFFSET : DISPLAY_TLMM_GPIO8_OFFSET);
    Id = MmioRead32 (Address + DISPLAY_TLMM_GPIO_ID_STATUS);
    if ((Id != MAX_UINT32) && ((Id & BIT0) != 0)) {
      Controller = mSecondaryDsi ? 1 : 0;
      mTeAddress[Controller] = Address;
      mTeOriginalConfig[Controller] = MmioRead32 (Address);
      Value = DISPLAY_TLMM_TE_CONFIG;
      if ((mTeOriginalConfig[Controller] & DISPLAY_TLMM_EGPIO_PRESENT) != 0) {
        Value |= DISPLAY_TLMM_EGPIO_ENABLE;
      }

      MmioWrite32 (Address, Value);
      MemoryFence ();
      if ((MmioRead32 (Address) & DISPLAY_TLMM_GPIO_CFG_MASK) != Value) {
        return EFI_DEVICE_ERROR;
      }

      return EFI_SUCCESS;
    }
  }

  return EFI_NOT_FOUND;
}

STATIC
VOID
DsiConfigureCommandStream (
  VOID
  )
{
  UINT32  Value;

  // XBL selects MDP trigger NONE (MDP drives the stream), DMA trigger SW,
  // stream 0 and no DSI-side external TE; the DPU handles the TE pin.
  DsiWrite (mDsiBase + DSI_REG_TRIG_CTRL, 4U);

  // Profile 6 defaults to two 675-pixel slices per packet at 8 bpp. The
  // command stream carries 1350 compressed bytes per line in 450 RGB slots;
  // the extra word-count byte is the inserted DCS memory-write command.
  DsiWrite (
    mDsiBase + DSI_REG_COMMAND_MODE_MDP_STREAM0_TOTAL,
    (DSI_PANEL_HEIGHT << 16) | DSI_DSC_WIDTH_TOTAL
    );
  DsiWrite (
    mDsiBase + DSI_REG_COMMAND_MODE_MDP_STREAM0_CTRL,
    (DSI_DSC_WORD_COUNT << 16) | 0x39U
    );
  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_MDP_DCS_CMD_CTRL, 0x13C2CU);
  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_MDP_CTRL, (1U << 20) | 8U);

  Value = DsiRead (mDsiBase + DSI_REG_COMMAND_MODE_MDP_CTRL2);
  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_MDP_CTRL2, Value | BIT16);
  Value = DsiRead (mDsiBase + DSI_REG_EOT_PACKET_CTRL);
  DsiWrite (mDsiBase + DSI_REG_EOT_PACKET_CTRL, Value | BIT0);
  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_MDP_IDLE_CTRL, 0);
  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_NULL_INSERTION_CTRL, 0);

  DsiWrite (
    mDsiBase + DSI_REG_COMMAND_COMPRESSION_MODE_CTRL,
    (0x39U << 8) | (DSI_DSC_PACKETS_PER_LINE << 6) | (DSI_DSC_EOL_BYTES << 4) | BIT0
    );
  DsiWrite (
    mDsiBase + DSI_REG_COMMAND_COMPRESSION_MODE_CTRL2,
    DSI_DSC_BYTES_PER_SLICE
    );
  DsiWrite (mDsiBase + DSI_REG_COMMAND_COMPRESSION_MODE_CTRL3, 0);
  MemoryFence ();
}

STATIC
EFI_STATUS
DsiWaitForBits (
  IN UINTN   Address,
  IN UINT32  Mask,
  IN UINT32  Expected,
  IN UINT32  TimeoutUs
  )
{
  UINT32  Attempt;

  for (Attempt = 0; Attempt < TimeoutUs; Attempt++) {
    if ((DsiRead (Address) & Mask) == Expected) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
DsiConfigurePhy (
  VOID
  )
{
  UINTN       Common;
  UINTN       Pll;
  UINTN       Lane;
  UINT32      Index;
  EFI_STATUS  Status;

  Common = mPhyCommonBase;
  Pll    = mPhyPllBase;

  //
  // Follow HAL_DSI_Phy_ResetPhy() and HAL_DSI_Phy_PowerUp(). Required clocks
  // and retained panel rails are supplied by the caller.
  //
  DsiWrite (mDsiBase + DSI_REG_PHY_SW_RESET, 1);
  DsiWrite (mDsiBase + DSI_REG_PHY_SW_RESET, 0);
  Status = DsiWaitForBits (Common + DSI_PHY_CMN_PHY_STATUS, BIT0, BIT0, DSI_REFGEN_TIMEOUT_US);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DsiWrite (Common + DSI_PHY_CMN_CTRL_3, 0);
  DsiWrite (Common + DSI_PHY_CMN_CTRL_0, 0x20);
  DsiWrite (Common + DSI_PHY_CMN_CTRL_0, 0x60);
  DsiWrite (Common + DSI_PHY_CMN_PLL_CTRL, 0);
  DsiWrite (Common + DSI_PHY_CMN_RBUF_CTRL, 0);
  DsiWrite (Common + DSI_PHY_CMN_VREG_CTRL_0, 0x59);
  DsiWrite (Common + DSI_PHY_CMN_VREG_CTRL_1, 0x19);
  DsiWrite (Common + DSI_PHY_CMN_CTRL_3, 0);
  DsiWrite (Common + 0x10CU, 0);
  DsiWrite (Common + 0x0ECU, 0x88);
  DsiWrite (Common + 0x104U, 0);
  DsiWrite (Common + 0x0F4U, 0);
  DsiWrite (Common + 0x0F8U, 0);
  DsiWrite (Common + 0x100U, 0x55);
  DsiWrite (Common + DSI_PHY_CMN_CTRL_0, 0x7F);
  DsiWrite (Common + DSI_PHY_CMN_LANE_CTRL_0, 0x1F);

  //
  // Identity lane map, four data lanes plus clock lane; lane programming
  // follows HAL_DSI_Phy_SetupLanePhy() for the 7 nm D-PHY.
  //
  DsiWrite (Common + DSI_PHY_CMN_LANE_CFG_0, 0x21);
  DsiWrite (Common + DSI_PHY_CMN_LANE_CFG_1, 0x84);
  for (Index = 0; Index < 5; Index++) {
    Lane = mPhyLaneBase + (Index * 0x80U);
    DsiWrite (Lane + 0x14U, 0);
    DsiWrite (Lane + 0x14U, (Index == 0) ? 3 : 0);
    DsiWrite (Lane + 0x10U, 0);
    DsiWrite (Lane + 0x00U, 0);
    DsiWrite (Lane + 0x04U, 0);
    DsiWrite (Lane + 0x08U, ((Index == 4) ? BIT7 : 0) | 0x0AU);
    DsiWrite (Lane + 0x18U, (Index == 3) ? 4 : ((Index == 4) ? 1 : 0));
  }

  //
  // XBL PHY timing algorithm evaluated at 500 MHz (UI=2 ns), with no OEM
  // timing overrides enabled in this profile. Registers 1..13 correspond to
  // tCLK_ZERO, tCLK_PREPARE, tCLK_TRAIL, tHS_EXIT, tHS_ZERO, tHS_PREPARE,
  // tHS_TRAIL, tHS_REQUEST, TA, tTA_GET, TRIG3, tCLK_PRE, tCLK_POST.
  //
  {
    STATIC CONST UINT8  Timing[] = {
      19, 4, 4, 31, 30, 5, 5, 3, 0x02, 4, 0, 18, 21
    };

    DsiWrite (Common + DSI_PHY_CMN_TIMING_CTRL_0, 0);
    for (Index = 0; Index < ARRAY_SIZE (Timing); Index++) {
      DsiWrite (Common + DSI_PHY_CMN_TIMING_CTRL_0 + ((Index + 1) * 4U), Timing[Index]);
    }
  }

  //
  // 500 MHz D-PHY: source LUT chooses PHY divider 1 and PLL divider 2;
  // VCO=1 GHz, 38.4 MHz reference with prescaler 2, N=26 + 0x2AAB/2^18.
  //
  DsiWrite (Common + DSI_PHY_CMN_CLK_CFG_0, 0x31);
  DsiWrite (Common + DSI_PHY_CMN_CLK_CFG_1, 1);
  DsiWrite (Common + DSI_PHY_CMN_VREG_CTRL_0, 0x5B);
  DsiWrite (Common + 0x10CU, 0x03);
  DsiWrite (Common + 0x0ECU, 0x66);

  DsiWrite (Pll + DSI_PHY_PLL_CORE_INPUT_OVERRIDE, 0x10);
  DsiWrite (Pll + DSI_PHY_PLL_CORE_INPUT_OVERRIDE, 0x12);
  DsiWrite (Pll + DSI_PHY_PLL_DECIMAL_DIV_START, 26);
  DsiWrite (Pll + DSI_PHY_PLL_FRAC_DIV_START_LOW, 0x0B);
  DsiWrite (Pll + DSI_PHY_PLL_FRAC_DIV_START_MID, 0x2A);
  DsiWrite (Pll + DSI_PHY_PLL_FRAC_DIV_START_HIGH, 0);
  DsiWrite (Pll + DSI_PHY_PLL_SYSTEM_MUXES, 0xC0);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_LOCKDET_RATE_1, 0x40);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_OUTDIV_RATE, 1);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_LOCK_DELAY, 6);
  DsiWrite (Pll + DSI_PHY_PLL_ANALOG_CONTROLS_FIVE_1, 1);
  DsiWrite (Pll + DSI_PHY_PLL_ANALOG_CONTROLS_FIVE, 1);
  DsiWrite (Pll + DSI_PHY_PLL_VCO_CONFIG_1, 0);
  DsiWrite (Pll + DSI_PHY_PLL_CLOCK_INVERTERS_1, 0x10);
  DsiWrite (Pll + DSI_PHY_PLL_CMODE_1, 0x10);
  DsiWrite (Pll + DSI_PHY_PLL_GEAR_BAND_SELECT_CONTROLS, 0x21);
  DsiWrite (Pll + DSI_PHY_PLL_ANALOG_CONTROLS_TWO, 3);
  DsiWrite (Pll + DSI_PHY_PLL_ANALOG_CONTROLS_THREE, 0);
  DsiWrite (Pll + DSI_PHY_PLL_DSM_DIVIDER, 0);
  DsiWrite (Pll + DSI_PHY_PLL_FEEDBACK_DIVIDER, 0x4E);
  DsiWrite (Pll + DSI_PHY_PLL_CALIBRATION_SETTINGS, 0x40);
  DsiWrite (Pll + DSI_PHY_PLL_BAND_SEL_CAL_SETTINGS_THREE, 0xBA);
  DsiWrite (Pll + DSI_PHY_PLL_FREQ_DETECT_SETTINGS_ONE, 0x0C);
  DsiWrite (Pll + DSI_PHY_PLL_OUTDIV, 0);
  DsiWrite (Pll + DSI_PHY_PLL_CORE_OVERRIDE, 0);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_DIGITAL_TIMERS_TWO, 8);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_PROP_GAIN_RATE_1, 8);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_BAND_SEL_RATE_1, 0xC0);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_INT_GAIN_IFILT_BAND_1, 0x82);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_FL_INT_GAIN_PFILT_BAND_1, 0x4C);
  DsiWrite (Pll + DSI_PHY_PLL_PLL_LOCK_OVERRIDE, 0x80);
  DsiWrite (Pll + DSI_PHY_PLL_PFILT, 0x2F);
  DsiWrite (Pll + DSI_PHY_PLL_IFILT, 0x3F);

  DsiWrite (Common + DSI_PHY_CMN_PLL_CTRL, 1);
  Status = DsiWaitForBits (Pll + DSI_PHY_PLL_COMMON_STATUS_ONE, BIT0, BIT0, DSI_PLL_TIMEOUT_US);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  DsiWrite (Common + DSI_PHY_CMN_CLK_CFG_1, BIT5 | 1);
  DsiWrite (Common + DSI_PHY_CMN_CTRL_2, 0x40);
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DsiConfigureClockRoot (
  IN UINTN   CommandAddress,
  IN UINTN   ConfigAddress,
  IN UINT32  Source,
  IN UINT32  Divider
  )
{
  UINT32      Value;
  EFI_STATUS  Status;

  //
  // XBL HAL_clk_GenericConfigMuxOffs clears the source, divider, MND mode,
  // and HW-control fields before installing the mux selection. The OEM DSI
  // profile uses direct (undivided) DSI PLL roots and no PCLK MND ratio.
  //
  Value  = DsiRead (ConfigAddress);
  Value &= ~(DISP_CC_CFG_SOURCE_MASK | DISP_CC_CFG_DIVIDER_MASK |
             DISP_CC_CFG_MODE_MASK | DISP_CC_CFG_HW_CLK_CONTROL);
  Value |= ((Source << DISP_CC_CFG_SOURCE_SHIFT) & DISP_CC_CFG_SOURCE_MASK);
  Value |= (Divider & DISP_CC_CFG_DIVIDER_MASK);
  DsiWrite (ConfigAddress, Value);

  //
  // Match HAL_clk_GenericConfigMux(): write the CFG, trigger UPDATE, and
  // wait until the command bit self-clears before enabling the root.
  //
  Value = DsiRead (CommandAddress) | DISP_CC_ROOT_UPDATE;
  DsiWrite (CommandAddress, Value);
  MemoryFence ();
  Status = DsiWaitForBits (
             CommandAddress,
             DISP_CC_ROOT_UPDATE,
             0,
             DSI_CLOCK_TIMEOUT_US
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Match HAL_clk_GenericUpdateRootEnable() and require the root-off status
  // to clear before its branch clock is enabled.
  //
  Value = DsiRead (CommandAddress);
  if ((Value & DISP_CC_ROOT_ENABLE) == 0) {
    DsiWrite (CommandAddress, Value | DISP_CC_ROOT_ENABLE);
  }

  return DsiWaitForBits (
           CommandAddress,
           DISP_CC_ROOT_OFF,
           0,
           DSI_CLOCK_TIMEOUT_US
           );
}

STATIC
EFI_STATUS
DsiEnableBranchClock (
  IN UINTN  Address
  )
{
  UINT32  Value;
  UINT32  Attempt;

  Value = DsiRead (Address);
  if ((Value & DISP_CC_CLOCK_ENABLE) == 0) {
    DsiWrite (Address, Value | DISP_CC_CLOCK_ENABLE);
    MemoryFence ();
  }

  for (Attempt = 0; Attempt < DSI_CLOCK_TIMEOUT_US; Attempt++) {
    Value = DsiRead (Address);
    if (((Value & DISP_CC_CLOCK_ENABLE) != 0) &&
        (((Value & DISP_CC_CLOCK_HW_CONTROL) != 0) ||
         ((Value & DISP_CC_CLOCK_OFF) == 0))) {
      return EFI_SUCCESS;
    }

    MicroSecondDelay (1);
  }

  DEBUG ((DEBUG_ERROR, "DisplayResumeDsiInit: clock 0x%lx timed out (0x%x)\n",
          (UINT64)Address, Value));
  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
DsiConfigureExternalClocks (
  VOID
  )
{
  UINT32      Value;
  EFI_STATUS  Status;
  UINTN       Controller;

  Controller = mSecondaryDsi ? 1 : 0;
  //
  // XBL uses independent PLLs for these single-controller panels: ESC uses
  // XO, while BYTE and PCLK select DSI0 PLL (1) or DSI1 PLL (2).
  //
  Status = DsiConfigureClockRoot (
             mSecondaryDsi ? DISP_CC_MDSS_ESC1_CMD_RCGR : DISP_CC_MDSS_ESC0_CMD_RCGR,
             mSecondaryDsi ? DISP_CC_MDSS_ESC1_CFG_RCGR : DISP_CC_MDSS_ESC0_CFG_RCGR,
             0,
             0
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDsiBranchesTouched[Controller] |= BIT3;
  Status = DsiEnableBranchClock (mSecondaryDsi ? DISP_CC_MDSS_ESC1_CBCR : DISP_CC_MDSS_ESC0_CBCR);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDsiRootsTouched[Controller] |= BIT1;
  Status = DsiConfigureClockRoot (
             mSecondaryDsi ? DISP_CC_MDSS_BYTE1_CMD_RCGR : DISP_CC_MDSS_BYTE0_CMD_RCGR,
             mSecondaryDsi ? DISP_CC_MDSS_BYTE1_CFG_RCGR : DISP_CC_MDSS_BYTE0_CFG_RCGR,
             mSecondaryDsi ? 2 : 1,
             0
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDsiBranchesTouched[Controller] |= BIT2;
  Status = DsiEnableBranchClock (mSecondaryDsi ? DISP_CC_MDSS_BYTE1_CBCR : DISP_CC_MDSS_BYTE0_CBCR);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // XBL sets nSourceDiv=2 for D-PHY byte_intf; HAL's branch divider uses
  // nDiv-1 in the two-bit CDIVR field, so the encoded value is one.
  //
  Value = DsiRead (mSecondaryDsi ? DISP_CC_MDSS_BYTE1_DIV_CDIVR : DISP_CC_MDSS_BYTE0_DIV_CDIVR);
  DsiWrite (
    mSecondaryDsi ? DISP_CC_MDSS_BYTE1_DIV_CDIVR : DISP_CC_MDSS_BYTE0_DIV_CDIVR,
    (Value & ~0x3U) | 1U
    );
  MemoryFence ();
  mDsiBranchesTouched[Controller] |= BIT1;
  Status = DsiEnableBranchClock (mSecondaryDsi ? DISP_CC_MDSS_BYTE1_INTF_CBCR : DISP_CC_MDSS_BYTE0_INTF_CBCR);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  mDsiRootsTouched[Controller] |= BIT0;
  Status = DsiConfigureClockRoot (
             mSecondaryDsi ? DISP_CC_MDSS_PCLK1_CMD_RCGR : DISP_CC_MDSS_PCLK0_CMD_RCGR,
             mSecondaryDsi ? DISP_CC_MDSS_PCLK1_CFG_RCGR : DISP_CC_MDSS_PCLK0_CFG_RCGR,
             mSecondaryDsi ? 2 : 1,
             0
             );
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // For RGB888 on four D-PHY lanes, HAL_DSI_Phy_MND_Divider() returns 1/1;
  // XBL therefore leaves PCLK M/N/2D at the bypass configuration.
  //
  mDsiBranchesTouched[Controller] |= BIT0;
  Status = DsiEnableBranchClock (mSecondaryDsi ? DISP_CC_MDSS_PCLK1_CBCR : DISP_CC_MDSS_PCLK0_CBCR);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // HAL_DSI_ClockConfig(AHBM_SCLK | PCLK | MAINCLK) enables AHBM, PCLK,
  // AHBS, DSI, byte, and escape clocks plus the AHBM dynamic-clock override.
  //
  DsiWrite (mDsiBase + DSI_REG_CLK_CTRL, DSI_INTERNAL_CLOCKS_ON);
  MemoryFence ();
  return EFI_SUCCESS;
}

STATIC
EFI_STATUS
DsiWaitForCommandDmaDone (
  VOID
  )
{
  UINT32  Attempt;
  UINT32  InterruptStatus;

  for (Attempt = 0;
       Attempt < (DSI_COMMAND_DMA_TIMEOUT_US / DSI_COMMAND_DMA_POLL_US);
       Attempt++) {
    MicroSecondDelay (DSI_COMMAND_DMA_POLL_US);
    InterruptStatus = DsiRead (mDsiBase + DSI_REG_INT_CTRL);
    if ((InterruptStatus & DSI_COMMAND_DMA_DONE) != 0) {
      //
      // Match HAL_DSI_ClrInterruptStatus()'s write-one-to-clear acknowledge.
      //
      DsiWrite (
        mDsiBase + DSI_REG_INT_CTRL,
        InterruptStatus | DSI_COMMAND_DMA_DONE_ACK
        );
      MemoryFence ();
      return EFI_SUCCESS;
    }
  }

  return EFI_TIMEOUT;
}

STATIC
EFI_STATUS
DsiSendPacket (
  IN CONST DSI_PANEL_PACKET  *Packet
  )
{
  UINT8       Buffer[136];
  UINT32      BufferLength;
  UINT32      WordCount;
  UINT32      Index;
  UINT32      Word;
  UINT32      StatusValue;
  BOOLEAN     LongPacket;
  BOOLEAN     UseDma;
  EFI_STATUS  Status;

  if ((Packet == NULL) || (Packet->Length > sizeof (Packet->Payload))) {
    return EFI_INVALID_PARAMETER;
  }

  if (mDsiCommandPending) {
    return EFI_NOT_READY;
  }

  LongPacket = ((Packet->Type & 0x0FU) == 0x09U) ||
               ((Packet->Type & 0x0FU) == 0x0AU);
  if (LongPacket) {
    BufferLength = (4U + Packet->Length + 3U) & ~3U;
    if ((BufferLength > sizeof (Buffer)) || (BufferLength > DSI_DMA_BUFFER_SIZE)) {
      return EFI_BAD_BUFFER_SIZE;
    }

    ZeroMem (Buffer, BufferLength);
    Buffer[0] = Packet->Length & 0xFF;
    Buffer[1] = (Packet->Length >> 8) & 0xFF;
    Buffer[2] = Packet->Type;
    Buffer[3] = DSI_EMBEDDED_LONG_LAST;
    CopyMem (&Buffer[4], Packet->Payload, Packet->Length);
  } else {
    if (Packet->Length > 2) {
      return EFI_INVALID_PARAMETER;
    }

    BufferLength = 4;
    ZeroMem (Buffer, sizeof (Buffer));
    if (Packet->Length > 0) {
      Buffer[0] = Packet->Payload[0];
    }

    if (Packet->Length > 1) {
      Buffer[1] = Packet->Payload[1];
    }

    Buffer[2] = Packet->Type;
    Buffer[3] = DSI_EMBEDDED_SHORT_LAST;
  }

  UseDma = (BufferLength > DSI_DMA_FIFO_SIZE);
  if (UseDma) {
    if (mDsiDmaBuffer == NULL) {
      // Reserved pages remain owned even if a timed-out DMA completes later.
      // PrePi's memory map is identity-mapped, and this register is 32-bit.
      mDsiDmaBuffer = AllocateReservedPages (1);
      if (mDsiDmaBuffer == NULL) {
        return EFI_OUT_OF_RESOURCES;
      }
    }

    if (((UINTN)mDsiDmaBuffer > (MAX_UINT32 - EFI_PAGE_SIZE + 1U)) ||
        (((UINTN)mDsiDmaBuffer & (EFI_PAGE_SIZE - 1U)) != 0)) {
      return EFI_UNSUPPORTED;
    }

    CopyMem (mDsiDmaBuffer, Buffer, BufferLength);
    WriteBackDataCacheRange (mDsiDmaBuffer, BufferLength);
    MemoryFence ();
  }

  Status = DsiWaitForBits (
             mDsiBase + DSI_REG_STATUS,
             DSI_STATUS_COMMAND_DMA_BUSY,
             0,
             DSI_COMMAND_TIMEOUT_US
             );
  if (EFI_ERROR (Status)) {
    mDsiCommandPending = TRUE;
    return Status;
  }

  DsiWrite (mDsiBase + DSI_REG_COMMAND_MODE_DMA_CTRL, DSI_CMD_DMA_EMBEDDED_LP);
  DsiWrite (mDsiBase + DSI_REG_DMA_CMD_LENGTH, BufferLength);
  StatusValue = DsiRead (mDsiBase + DSI_REG_TEST_PATTERN_GEN_CTRL);
  StatusValue &= ~DSI_TPG_CMD_DMA_MASK;
  if (UseDma) {
    DsiWrite (mDsiBase + DSI_REG_DMA_CMD_OFFSET, (UINT32)(UINTN)mDsiDmaBuffer);
    DsiWrite (mDsiBase + DSI_REG_TEST_PATTERN_GEN_CTRL, StatusValue);
  } else {
    DsiWrite (mDsiBase + DSI_REG_TEST_PATTERN_GEN_CTRL, StatusValue | DSI_TPG_CMD_DMA_FIFO);
    WordCount = (BufferLength + 3U) >> 2;
    for (Index = 0; Index < WordCount; Index++) {
      CopyMem (&Word, &Buffer[Index * sizeof (UINT32)], sizeof (Word));
      DsiWrite (mDsiBase + DSI_REG_TEST_PATTERN_GEN_CMD_DMA_INIT_VAL, Word);
    }

    if ((WordCount & 1U) != 0) {
      DsiWrite (mDsiBase + DSI_REG_TEST_PATTERN_GEN_CMD_DMA_INIT_VAL, 0);
    }
  }

  //
  // Clear a stale completion status before triggering. XBL's
  // dsiShared_ClientWrite() waits on CMDMODE_DMA_DONE, rather than treating
  // an immediately-idle STATUS register as proof that this packet finished.
  //
  StatusValue = DsiRead (mDsiBase + DSI_REG_INT_CTRL);
  DsiWrite (
    mDsiBase + DSI_REG_INT_CTRL,
    StatusValue | DSI_COMMAND_DMA_DONE_ACK
    );
  MemoryFence ();
  Status = DsiWaitForBits (
             mDsiBase + DSI_REG_INT_CTRL,
             DSI_COMMAND_DMA_DONE,
             0,
             DSI_COMMAND_TIMEOUT_US
             );
  if (EFI_ERROR (Status)) {
    mDsiCommandPending = TRUE;
    return Status;
  }

  mDsiCommandPending = TRUE;
  DsiWrite (mDsiBase + DSI_REG_CMD_MODE_DMA_SW_TRIGGER, 1);
  Status = DsiWaitForCommandDmaDone ();
  if (!EFI_ERROR (Status)) {
    mDsiCommandPending = FALSE;
  }

  if (EFI_ERROR (Status)) {
    StatusValue = DsiRead (mDsiBase + DSI_REG_STATUS);
    DEBUG ((
      DEBUG_ERROR,
      "DisplayResumeDsiInit: packet 0x%02x DMA completion timeout, STATUS=0x%08x INT=0x%08x\n",
      Packet->Type,
      StatusValue,
      DsiRead (mDsiBase + DSI_REG_INT_CTRL)
      ));
  }

  return Status;
}

STATIC
EFI_STATUS
DsiSendXmlSequence (
  IN UINT32  PanelRevision
  )
{
  UINT32      Index;
  EFI_STATUS  Status;
  BOOLEAN     SendOldRevisionCommand;

  SendOldRevisionCommand = FALSE;
  for (Index = 0; Index < ARRAY_SIZE (mPanelInit); Index++) {
    if (mPanelInit[Index].Type == 0xFF) {
      MicroSecondDelay ((UINTN)mPanelInit[Index].Payload[0] * 1000U);
      if (SendOldRevisionCommand) {
        Status = DsiSendPacket (&mPanelOldRevisionPacket);
        if (EFI_ERROR (Status)) {
          return Status;
        }

        SendOldRevisionCommand = FALSE;
      }

      continue;
    }

    Status = DsiSendPacket (&mPanelInit[Index]);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // OEMPlatformLib selects this command only for revisions 6 through 15;
    // its source expression treats values below six as the newer profile.
    //
    if ((mPanelInit[Index].Type == 0x05) &&
        (mPanelInit[Index].Payload[0] == 0x11) &&
        ((PanelRevision - 6U) <= 9U)) {
      SendOldRevisionCommand = TRUE;
    }
  }

  return EFI_SUCCESS;
}

/**
  Initialize the selected LG_QHD DSI command stream. Call after DPU
  configuration and before starting DPU refresh.

  The caller must provide MDSS power and core clocks, retain the panel rails,
  and install the temporary display SMMU bypass before calling. It must remain
  installed throughout any pending command and scanout DMA. Long commands use
  a reserved, identity-mapped page below 4 GB and require the bypass to cover
  DSI command DMA as well as framebuffer reads. This function configures
  GPIO 8 or 9 as the TE input, but does not clear framebuffer memory or pulse panel
  reset GPIOs. The PHY, clocks, controller, and command DMA may be partially
  initialized on failure. A command timeout prevents re-entry; its reserved
  DMA page is never reused or released and the mapping must remain installed.

  @retval EFI_SUCCESS       The DSI command stream is configured.
  @retval EFI_TIMEOUT       Refgen or PLL lock did not complete.
**/
EFI_STATUS
DisplayResumeDsiInit (
  IN BOOLEAN  Secondary
  )
{
  UINT32      RouteIndex;
  EFI_STATUS  Status;
  CONST DSI_PANEL_ROUTE  *SelectedRoute;

  if (mDsiCommandPending) {
    return EFI_NOT_READY;
  }

  mPanelRevision = MmioRead8 (0x146BFA94U);
  RouteIndex = ((mPanelRevision - 6U) <= 9U) ? 0U : 1U;
  SelectedRoute = Secondary ? &mSecondaryPanelRoute : &mPrimaryPanelRouteByRevision[RouteIndex];
  mSecondaryDsi = Secondary;
  mDsiInitialized = FALSE;
  mDsiBase = SelectedRoute->DsiBase;
  mPhyCommonBase = mDsiBase + 0x400U;
  mPhyLaneBase = mDsiBase + 0x600U;
  mPhyPllBase = mDsiBase + 0x900U;

  // Record the route selected for this controller.
  gDisplayResumeDsiRoute.Valid         = TRUE;
  gDisplayResumeDsiRoute.Controller    = SelectedRoute->Controller;
  gDisplayResumeDsiRoute.Interface     = SelectedRoute->Interface;
  gDisplayResumeDsiRoute.DsiBase       = SelectedRoute->DsiBase;
  gDisplayResumeDsiRoute.InterfaceBase = SelectedRoute->InterfaceBase;
  gDisplayResumeDsiRoute.TearBase      = SelectedRoute->TearBase;

  Status = DsiConfigureTeGpio ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "DisplayResumeDsiInit: TE GPIO setup failed: %r\n", Status));
    return Status;
  }

  mDsiPhyTouched[Secondary ? 1 : 0] = TRUE;
  Status = DsiConfigurePhy ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "DisplayResumeDsiInit: PHY/PLL setup failed: %r\n", Status));
    return Status;
  }

  Status = DsiConfigureExternalClocks ();
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "DisplayResumeDsiInit: DSI external clocks failed: %r\n", Status));
    return Status;
  }

  //
  // Match HAL_DSI_DeviceConfig() for four data lanes, clock lane, command
  // engine, and controller; no video engine or panel reset is enabled.
  //
  DsiWrite (mDsiBase + DSI_REG_SOFT_RESET, 1);
  DsiWrite (mDsiBase + DSI_REG_SOFT_RESET, 0);
  DsiConfigureCommandStream ();
  DsiWrite (mDsiBase + DSI_REG_CTRL, DSI_CTRL_ENABLE_CMD);
  MmioOr32 (mDsiBase + DSI_REG_LANE_CTRL, BIT28);
  MmioOr32 (
    mPhyCommonBase + DSI_PHY_CMN_LANE_CTRL_1,
    BIT6 | BIT5
    );

  mDsiInitialized = TRUE;
  mDsiHostReady[Secondary ? 1 : 0] = TRUE;
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDsiPanelInit (
  VOID
  )
{
  EFI_STATUS  Status;

  if (!mDsiInitialized) {
    return EFI_NOT_READY;
  }

  mPanelSequenceStarted[mSecondaryDsi ? 1 : 0] = TRUE;
  Status = DsiSendXmlSequence (mPanelRevision);
  if (!EFI_ERROR (Status)) {
    // XBL sends its generated PPS after the XML; this profile uses the same
    // DSC parameters as the PPS already embedded in that sequence.
    Status = DsiSendPacket (&mPanelInit[2]);
  }

  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "DisplayResumeDsiPanelInit: panel sequence failed: %r\n", Status));
    return Status;
  }

  DEBUG ((
    DEBUG_INFO,
    "DisplayResumeDsiPanelInit: DSI%u ready, panel revision %u\n",
    mSecondaryDsi ? 1U : 0U,
    mPanelRevision
    ));
  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDsiPanelShutdown (
  VOID
  )
{
  UINTN       Controller;
  UINTN       Index;
  EFI_STATUS  Status;

  if (mDsiCommandPending) {
    return EFI_NOT_READY;
  }

  for (Controller = 0; Controller < ARRAY_SIZE (mPanelSequenceStarted); Controller++) {
    if (!mPanelSequenceStarted[Controller]) {
      continue;
    }

    mSecondaryDsi = (Controller == 1);
    mDsiBase = mSecondaryDsi ? DISPLAY_RESUME_DSI1_BASE : DISPLAY_RESUME_DSI0_BASE;
    for (Index = 0; Index < ARRAY_SIZE (mPanelOff); Index++) {
      Status = DsiSendPacket (&mPanelOff[Index]);
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "DisplayResume: DSI%u panel shutdown failed: %r\n",
                (UINT32)Controller, Status));
        return Status;
      }
    }

    MicroSecondDelay (192000);
    mPanelSequenceStarted[Controller] = FALSE;
  }

  return EFI_SUCCESS;
}

EFI_STATUS
DisplayResumeDsiClose (
  VOID
  )
{
  UINTN       Controller;
  UINTN       Base;
  UINTN       Common;
  UINTN       Branches[4];
  UINTN       Index;
  EFI_STATUS  Status;

  if (mDsiCommandPending) {
    return EFI_NOT_READY;
  }

  for (Controller = 0; Controller < ARRAY_SIZE (mDsiHostReady); Controller++) {
    Base = (Controller == 0) ? DISPLAY_RESUME_DSI0_BASE : DISPLAY_RESUME_DSI1_BASE;
    Common = Base + 0x400U;
    if (mDsiHostReady[Controller]) {
      MmioAnd32 (Base + DSI_REG_CTRL, ~BIT0);
      DsiWrite (Base + DSI_REG_SOFT_RESET, 1);
      DsiWrite (Base + DSI_REG_SOFT_RESET, 0);
      // XBL reset restores a 50 ms timeout at the 19.2 MHz escape clock.
      MmioAndThenOr32 (Base + DSI_REG_HS_TIMER_CTRL, ~0x000FFFFFU, (4U << 16) | 60000U);
      DsiWrite (Base + DSI_REG_TPG_DMA_FIFO_RESET, 1);
      DsiWrite (Base + DSI_REG_TPG_DMA_FIFO_RESET, 0);
      DsiWrite (Base + DSI_REG_CTRL, 0);
      DsiWrite (Base + DSI_REG_CLK_CTRL, 0);
      MemoryFence ();
      mDsiHostReady[Controller] = FALSE;
    }

    if ((mDsiRootsTouched[Controller] & BIT0) != 0) {
      Status = DsiConfigureClockRoot (
                 (Controller == 0) ? DISP_CC_MDSS_PCLK0_CMD_RCGR : DISP_CC_MDSS_PCLK1_CMD_RCGR,
                 (Controller == 0) ? DISP_CC_MDSS_PCLK0_CFG_RCGR : DISP_CC_MDSS_PCLK1_CFG_RCGR,
                 0,
                 1
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "DisplayResume: DSI%u pixel clock shutdown failed: %r\n",
                (UINT32)Controller, Status));
        return Status;
      }

      mDsiRootsTouched[Controller] &= ~BIT0;
    }

    if ((mDsiRootsTouched[Controller] & BIT1) != 0) {
      Status = DsiConfigureClockRoot (
                 (Controller == 0) ? DISP_CC_MDSS_BYTE0_CMD_RCGR : DISP_CC_MDSS_BYTE1_CMD_RCGR,
                 (Controller == 0) ? DISP_CC_MDSS_BYTE0_CFG_RCGR : DISP_CC_MDSS_BYTE1_CFG_RCGR,
                 0,
                 1
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((DEBUG_ERROR, "DisplayResume: DSI%u byte clock shutdown failed: %r\n",
                (UINT32)Controller, Status));
        return Status;
      }

      mDsiRootsTouched[Controller] &= ~BIT1;
    }

    Branches[0] = (Controller == 0) ? DISP_CC_MDSS_PCLK0_CBCR : DISP_CC_MDSS_PCLK1_CBCR;
    Branches[1] = (Controller == 0) ? DISP_CC_MDSS_BYTE0_INTF_CBCR : DISP_CC_MDSS_BYTE1_INTF_CBCR;
    Branches[2] = (Controller == 0) ? DISP_CC_MDSS_BYTE0_CBCR : DISP_CC_MDSS_BYTE1_CBCR;
    Branches[3] = (Controller == 0) ? DISP_CC_MDSS_ESC0_CBCR : DISP_CC_MDSS_ESC1_CBCR;
    for (Index = 0; Index < ARRAY_SIZE (Branches); Index++) {
      if ((mDsiBranchesTouched[Controller] & (1U << Index)) == 0) {
        continue;
      }

      MmioAnd32 (Branches[Index], ~DISP_CC_CLOCK_ENABLE);
      MemoryFence ();
      if ((DsiRead (Branches[Index]) & DISP_CC_CLOCK_ENABLE) != 0) {
        DEBUG ((DEBUG_ERROR, "DisplayResume: DSI%u clock 0x%lx did not disable\n",
                (UINT32)Controller, (UINT64)Branches[Index]));
        return EFI_DEVICE_ERROR;
      }

      mDsiBranchesTouched[Controller] &= ~(1U << Index);
    }

    if (mDsiPhyTouched[Controller]) {
      DsiWrite (Common + DSI_PHY_CMN_PLL_CTRL, 0);
      DsiWrite (Common + DSI_PHY_CMN_CTRL_0, 0);
      MemoryFence ();
      mDsiPhyTouched[Controller] = FALSE;
    }

    if (mTeAddress[Controller] != 0) {
      MmioAndThenOr32 (
        mTeAddress[Controller],
        ~DISPLAY_TLMM_GPIO_CFG_MASK,
        mTeOriginalConfig[Controller] & DISPLAY_TLMM_GPIO_CFG_MASK
        );
      MemoryFence ();
      if ((MmioRead32 (mTeAddress[Controller]) & DISPLAY_TLMM_GPIO_CFG_MASK) !=
          (mTeOriginalConfig[Controller] & DISPLAY_TLMM_GPIO_CFG_MASK)) {
        DEBUG ((DEBUG_ERROR, "DisplayResume: DSI%u TE GPIO restore failed\n", (UINT32)Controller));
        return EFI_DEVICE_ERROR;
      }

      mTeAddress[Controller] = 0;
    }
  }

  mDsiInitialized = FALSE;
  return EFI_SUCCESS;
}
