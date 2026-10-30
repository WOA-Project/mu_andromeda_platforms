/** @file
  RightDisplayDxe: bring-up of the Surface Duo 2 right display (DSI1).

  Register programming sequences are derived from the open-source Surface Duo
  kernel (surface-duo-oss-kernel.msm-5.4 and the vendor display driver) and
  from the mainline Linux msm/dsi drivers.

  SPDX-License-Identifier: MIT
**/
#ifndef RIGHT_DISPLAY_DXE_H_
#define RIGHT_DISPLAY_DXE_H_

#include <PiDxe.h>
#include <Uefi.h>

#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/IoLib.h>
#include <Library/UefiBootServicesTableLib.h>

//
// Panel (Elgin right panel, surface_elgin_dsi1_r2_cmd_mp.dtsi, 60Hz timing)
//
#define PANEL_WIDTH      1344
#define PANEL_HEIGHT     1892
#define PANEL_BITCLK_HZ  760000000ULL

// DSIPacketTransferHS=True in the vendor panel XML: init commands go out in HS
// with the clock lane forced to HS beforehand. Set to 0 to fall back to LP.
#define PANEL_INIT_HS    0
#define PANEL_RESET_GPIO 51
#define PANEL_TE_GPIO         83
#define PANEL_TE_GPIO_FUNC    1 // mdp_vsync (SM8350 TLMM: gpio83 f1)
#define PANEL_TE_VSYNC_SOURCE 1 // qcom,panel-te-source = <1> for the DSI1 display (lahaina-sde-display.dtsi)
#define PANEL_T_CLK_POST 0x0B
#define PANEL_T_CLK_PRE  0x17

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

//
// Bring-up depth, for bisecting hangs: 0 = GOP only (no hardware access),
// 1 = + PMIC panel supplies, 2 = + ext-trigger clock, 3 = + MDSS clocks and
// DSI PHY/PLL/clocks, 4 = + DSI controller and panel init, 5 = + DPU pipeline
// and display on.
//
#ifndef RIGHT_DISPLAY_STAGE
#define RIGHT_DISPLAY_STAGE 5
#endif

// Allow the driver to program PMIC rails/clock buffer that read as disabled.
// Off by default: XBL already powers the shared panel rails.
#ifndef RIGHT_DISPLAY_FORCE_PMIC
#define RIGHT_DISPLAY_FORCE_PMIC 0
#endif

// Bisection: replace the DMA2 layer with a solid white layer-mixer border.
// White on the right panel means the framebuffer fetch is at fault; black
// means the fault is after the mixer (DSC, INTF, DSI or panel).
#ifndef RIGHT_DISPLAY_BORDER_TEST
#define RIGHT_DISPLAY_BORDER_TEST 0
#endif

// Bisection: after bring-up, make the DSI controller's own test pattern
// generator drive command stream 0. Anything (noise, colours) appearing on the
// right panel proves the DSI link and panel pixel path work, and the fault is
// upstream in the DPU (DSC/INTF); a black panel points at DSI or the panel.
#ifndef RIGHT_DISPLAY_DSI_TPG
#define RIGHT_DISPLAY_DSI_TPG 0
#endif

// Bisection: the DPU fetches the left display's framebuffer instead of ours.
// The right panel then mirrors the left one. If it does, our own framebuffer
// memory (allocation, cache or SMMU mapping) is the fault; if it stays black,
// the fault is downstream of the framebuffer fetch.
#ifndef RIGHT_DISPLAY_MIRROR_LEFT
#define RIGHT_DISPLAY_MIRROR_LEFT 1
#endif

// Bisection: send DCS 0x23 (all pixels on) after display-on. A white panel
// proves the panel and its supplies emit light and the fault is in the pixel
// path; a black panel points at panel power (e.g. OLED supplies).
#ifndef RIGHT_DISPLAY_ALL_PIXELS_ON
#define RIGHT_DISPLAY_ALL_PIXELS_ON 0
#endif

// Bisection: after display-on, set the maximum brightness (0x51 = 0x0FFF)
#ifndef RIGHT_DISPLAY_MAX_BRIGHTNESS
#define RIGHT_DISPLAY_MAX_BRIGHTNESS 1
#endif

// Which block chain the right display uses:
//  1: CTL_1 -> VIG1 -> LM_1 -> PP_1 -> DSC_1 -> INTF_2
//  0: CTL_2 -> VIG2 -> LM_2 -> PP_2 -> DSC_2 -> INTF_2 (the previous working-but-black chain)
#ifndef RIGHT_DISPLAY_CHAIN1
#define RIGHT_DISPLAY_CHAIN1 1
#endif

#if RIGHT_DISPLAY_CHAIN1
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
#else
#define RD_CTL_INDEX      2
#define RD_CTL_OFF        0x17000
#define RD_LM_OFF         0x46000
#define RD_SSPP_OFF       0x8000
#define RD_PP_OFF         0x6B000
#define RD_DSC_CORE_OFF   0x81000
#define RD_DSC_ENC_OFF    0x81100
#define RD_DSC_CTL_OFF    0x81F00
#define RD_STAGE_SHIFT    6
#define RD_FETCH_BIT      BIT18
#define RD_FLUSH_SSPP     BIT2
#define RD_FLUSH_LM       BIT8
#endif

// As XBL does with DSIForceCmdInVideoHS: reset the panel, start the DPU pipe
// and autorefresh, and only then send the INIT sequence and PPS.
#ifndef RIGHT_DISPLAY_INIT_AFTER_PIPE
#define RIGHT_DISPLAY_INIT_AFTER_PIPE 0
#endif

// Dump hardware state to the debug console at the end of bring-up and hold it
// on screen for RIGHT_DISPLAY_DIAG_HOLD_MS.
#ifndef RIGHT_DISPLAY_DIAG
#define RIGHT_DISPLAY_DIAG 1
#endif

#ifndef RIGHT_DISPLAY_DIAG_HOLD_MS
#define RIGHT_DISPLAY_DIAG_HOLD_MS 20000
#endif

//
// Tlmm.c
//
VOID
TlmmConfigGpioOutput (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  );

VOID
TlmmSetGpio (
  IN UINT32  Gpio,
  IN BOOLEAN Level
  );

VOID
TlmmConfigFunction (
  IN UINT32 Gpio,
  IN UINT32 Function,
  IN UINT32 Pull,
  IN UINT32 DriveStrength
  );

//
// DsiPhy.c
//
EFI_STATUS
DsiPhyEnable (VOID);

EFI_STATUS
DsiPllEnable (VOID);

//
// Power.c
VOID
PowerEnablePanelSupplies (VOID);

VOID
PowerEnableExtTrigClock (VOID);

// DispCc.c
//
EFI_STATUS
DispCcEnableMdssClocks (VOID);

EFI_STATUS
DispCcEnableDsi1Clocks (VOID);

//
// DsiCtrl.c
//
EFI_STATUS
DsiCtrlInit (VOID);

EFI_STATUS
DsiCtrlSendPacket (
  IN UINT8        DataType,
  IN CONST UINT8  *Payload,
  IN UINT32       PayloadLength,
  IN BOOLEAN      LowPower
  );

VOID
DsiCtrlSetupStream (VOID);

//
// Panel.c
//
extern CONST UINT8 gPanelPps[128];

EFI_STATUS
DsiCtrlReadDcs (
  IN  UINT8   Command,
  OUT UINT32  *Data,
  OUT UINT32  *RdbkType,
  OUT UINT32  *AckErr
  );

EFI_STATUS
PanelProbe (VOID);

EFI_STATUS
PanelPowerOnAndInit (VOID);

EFI_STATUS
PanelReset (VOID);

EFI_STATUS
PanelSendInit (VOID);

EFI_STATUS
PanelDisplayOn (VOID);

//
// Dpu.c
//
EFI_STATUS
DpuStart (
  IN EFI_PHYSICAL_ADDRESS FbBase
  );

VOID
DpuEnableAutoRefresh (VOID);

#endif
