/** @file
  Optional bring-up diagnostics, only used when RIGHT_DISPLAY_DIAG is set.

  SPDX-License-Identifier: MIT
**/
#include "RightDisplayDxe.h"

#include <Library/FrameBufferSerialPortLib.h>
#include <Library/PcdLib.h>
#include <Library/PlatformHwioLib.h>

//
// Color bars on the top two thirds, grayscale ramp below, white 4px border.
//
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
// The left display console (FrameBufferSerialPortLib) keeps its cursor in
// memory right after the left framebuffer. Clearing the framebuffer and zeroing
// that cursor makes the RD: log start at the top of an empty screen.
//
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
VOID
DiagDump (
  IN BOOLEAN     PanelUp,
  IN EFI_STATUS  LastStatus
  )
{
  UINT32 Frame0, Frame1;

  DEBUG ((DEBUG_ERROR, "RD: panelup=%u last=%r\n", PanelUp, LastStatus));
  DEBUG ((
    DEBUG_ERROR, "RD: pll lock=%08x phy_status=%08x\n",
    MmioRead32 (PLL (0x1B0)),
    MmioRead32 (PHY (CMN_PHY_STATUS))));
  DEBUG ((
    DEBUG_ERROR, "RD: dsi ctrl=%08x clk=%08x intr=%08x mdp=%08x lane=%08x\n",
    MmioRead32 (DSI (DSI_CTRL)),
    MmioRead32 (DSI (DSI_CLK_CTRL)),
    MmioRead32 (DSI (DSI_INT_CTRL)),
    MmioRead32 (DSI (DSI_MDP_CTRL)),
    MmioRead32 (DSI (DSI_LANE_STATUS))));
  DEBUG ((
    DEBUG_ERROR, "RD: ctl top=%08x flush=%08x intf_act=%08x dsc_act=%08x\n",
    MmioRead32 (MDP (CTL_TOP)),
    MmioRead32 (MDP (CTL_FLUSH)),
    MmioRead32 (MDP (CTL_INTF_ACTIVE)),
    MmioRead32 (MDP (CTL_DSC_ACTIVE))));
  DEBUG ((
    DEBUG_ERROR, "RD: intf tear=%08x mux=%08x\n",
    MmioRead32 (MDP (INTF_BASE + INTF_TEAR_CHECK_EN)),
    MmioRead32 (MDP (INTF_BASE + INTF_MUX))));

  // INTF frame counter should advance if frames are being pushed
  Frame0 = MmioRead32 (MDP (INTF_BASE + 0xAC));
  gBS->Stall (MS (100));
  Frame1 = MmioRead32 (MDP (INTF_BASE + 0xAC));
  DEBUG ((DEBUG_ERROR, "RD: intf frame cnt %08x -> %08x\n", Frame0, Frame1));

  gBS->Stall (MS (RIGHT_DISPLAY_DIAG_HOLD_MS));
}
