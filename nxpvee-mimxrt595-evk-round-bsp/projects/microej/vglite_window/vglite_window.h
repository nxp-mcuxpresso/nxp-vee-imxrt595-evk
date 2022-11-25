/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2019-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 *
 * 1. Add VGLITE_GetBuffers and VGLITE_GetNextBuffer functions.
 * 2. Redirect APP_BUFFER_COUNT to FRAME_BUFFER_COUNT defined
 *    in display_framebuffer.h
 * 3. Adjust the implementation of VGLITE_SwapBuffers and VGLITE_GetNextBuffer to 
 *    match the LLUI_DISPLAY_IMPL.h implementation
 */

#ifndef VGLITE_WINDOW_H
#define VGLITE_WINDOW_H

#include "fsl_common.h"
#include "vg_lite.h"
#include "vglite_support.h"
#include "display_support.h"
#include "fsl_fbdev.h"

#include "display_framebuffer.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define APP_BUFFER_COUNT FRAME_BUFFER_COUNT

typedef struct vg_lite_display
{
    fbdev_t g_fbdev;
    fbdev_fb_info_t g_fbInfo;
} vg_lite_display_t;

typedef struct vg_lite_window
{
    vg_lite_display_t *display;
    vg_lite_buffer_t buffers[APP_BUFFER_COUNT];
    int width;
    int height;
    int bufferCount;
    int current;
} vg_lite_window_t;

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

// added by MicroEJ
void ** VGLITE_GetBuffers(void);

vg_lite_error_t VGLITE_CreateDisplay(vg_lite_display_t *display);

vg_lite_error_t VGLITE_CreateWindow(vg_lite_display_t *display, vg_lite_window_t *window);

vg_lite_error_t VGLITE_DestoryWindow(void);

vg_lite_error_t VGLITE_DestroyDisplay(void);

vg_lite_buffer_t *VGLITE_GetRenderTarget(vg_lite_window_t *window);

// added by MicroEJ
vg_lite_buffer_t *VGLITE_GetNextBuffer(vg_lite_window_t *window);

void VGLITE_SwapBuffers(vg_lite_window_t *window);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* VGLITE_WINDOW_H */
