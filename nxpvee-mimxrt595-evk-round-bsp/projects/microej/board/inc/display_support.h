/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 * 1. Setup RM67162 panel
 * 2. Disable fixed address for RM67162
 * 3. Remove irrelevant code
 * 4. Redirect DEMO_BUFFER_XXX to FRAME_BUFFER_XXX defined in display_framebuffer.h
 */

#ifndef _DISPLAY_SUPPORT_H_
#define _DISPLAY_SUPPORT_H_

#include "fsl_dc_fb.h"
#include "display_framebuffer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*
 * @brief Defines used by VGLite library and FB drivers.
 */
#define DEMO_PANEL_WIDTH DISPLAY_PANEL_WIDTH
#define DEMO_PANEL_HEIGHT DISPLAY_PANEL_HEIGHT
#define BUFFER_PIXEL_FORMAT kVIDEO_PixelFormatRGB565
#define BUFFER_START_X ((DISPLAY_PANEL_WIDTH - FRAME_BUFFER_WIDTH) / 2)
#define BUFFER_START_Y ((DISPLAY_PANEL_HEIGHT - FRAME_BUFFER_HEIGHT) / 2)
#define BUFFER_BYTE_PER_PIXEL FRAME_BUFFER_BYTE_PER_PIXEL
#define BUFFER_WIDTH FRAME_BUFFER_WIDTH
#define BUFFER_HEIGHT FRAME_BUFFER_HEIGHT
#define BUFFER_STRIDE_BYTE FRAME_BUFFER_STRIDE_BYTE

#define DEMO_RM67162_USE_DSI_SMARTDMA 1

extern const dc_fb_t g_dc;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

status_t BOARD_PrepareDisplayController(void);
void BOARD_DisplayTEPinHandler(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _DISPLAY_SUPPORT_H_ */
