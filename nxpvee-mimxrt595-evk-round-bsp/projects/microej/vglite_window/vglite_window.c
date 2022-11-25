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
 * See vglite_windows.h
 */


#include "vglite_window.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/

static int fb_idx = 0;

/*******************************************************************************
 * Code
 ******************************************************************************/
static vg_lite_buffer_format_t video_format_to_vglite(video_pixel_format_t format)
{
    vg_lite_buffer_format_t fmt;
    switch (format)
    {
        case kVIDEO_PixelFormatRGB565:
            fmt = VG_LITE_BGR565;
            break;

        case kVIDEO_PixelFormatBGR565:
            fmt = VG_LITE_RGB565;
            break;

        case kVIDEO_PixelFormatXRGB8888:
            fmt = VG_LITE_BGRX8888;
            break;

        default:
            fmt = VG_LITE_RGB565;
            break;
    }

    return fmt;
}

vg_lite_error_t VGLITE_CreateDisplay(vg_lite_display_t *display)
{
    if (!display)
        return VG_LITE_INVALID_ARGUMENT;

    BOARD_PrepareDisplayController();
    FBDEV_Open(&display->g_fbdev, &g_dc, 0);

    return VG_LITE_SUCCESS;
}

// added by MicroEJ
void ** VGLITE_GetBuffers(void)
{
    return (void **) &s_frameBufferAddress[0];
}

vg_lite_error_t VGLITE_CreateWindow(vg_lite_display_t *display, vg_lite_window_t *window)
{
    vg_lite_error_t ret = VG_LITE_SUCCESS;
    status_t status;
    void *buffer;
    vg_lite_buffer_t *vg_buffer;
    fbdev_t *g_fbdev          = &(display->g_fbdev);
    fbdev_fb_info_t *g_fbInfo = &(display->g_fbInfo);

    window->bufferCount = APP_BUFFER_COUNT;
    window->display     = display;
    window->width       = BUFFER_WIDTH;
    window->height      = BUFFER_HEIGHT;
    window->current     = -1;
    FBDEV_GetFrameBufferInfo(g_fbdev, g_fbInfo);

    g_fbInfo->bufInfo.pixelFormat = BUFFER_PIXEL_FORMAT;
    g_fbInfo->bufInfo.startX      = BUFFER_START_X;
    g_fbInfo->bufInfo.startY      = BUFFER_START_Y;
    g_fbInfo->bufInfo.width       = window->width;
    g_fbInfo->bufInfo.height      = window->height;
    g_fbInfo->bufInfo.strideBytes = BUFFER_STRIDE_BYTE;

    g_fbInfo->bufferCount = window->bufferCount;
    for (uint8_t i = 0; i < window->bufferCount; i++)
    {
        vg_buffer            = &(window->buffers[i]);
        g_fbInfo->buffers[i] = (void *)s_frameBufferAddress[i];
        vg_buffer->memory    = g_fbInfo->buffers[i];
        vg_buffer->address   = (uint32_t) s_frameBufferAddress[i];
        vg_buffer->width     = g_fbInfo->bufInfo.width;
        vg_buffer->height    = g_fbInfo->bufInfo.height;
        vg_buffer->stride    = g_fbInfo->bufInfo.strideBytes;
        vg_buffer->format    = video_format_to_vglite(BUFFER_PIXEL_FORMAT);
    }

    status = FBDEV_SetFrameBufferInfo(g_fbdev, g_fbInfo);
    if (status != kStatus_Success)
    {
        while (1)
            ;
    }

    buffer = FBDEV_GetFrameBuffer(g_fbdev, 0);

    assert(buffer != NULL);

    memset(buffer, 0, g_fbInfo->bufInfo.height * g_fbInfo->bufInfo.strideBytes);

    FBDEV_SetFrameBuffer(g_fbdev, buffer, 0);

    FBDEV_Enable(g_fbdev);

    return ret;
}

vg_lite_error_t VGLITE_DestoryWindow(void)
{
    return VG_LITE_SUCCESS;
}

vg_lite_error_t VGLITE_DestroyDisplay(void)
{
    return VG_LITE_SUCCESS;
}

// added by MicroEJ
vg_lite_buffer_t *VGLITE_GetNextBuffer(vg_lite_window_t *window) {
    return &window->buffers[(fb_idx + 1) % APP_BUFFER_COUNT];
}

// modified by MicroEJ
vg_lite_buffer_t *VGLITE_GetRenderTarget(vg_lite_window_t *window)
{
    return &window->buffers[fb_idx];
}

// modified by MicroEJ
void VGLITE_SwapBuffers(vg_lite_window_t *window)
{
    vg_lite_finish();

    FBDEV_SetFrameBuffer(&window->display->g_fbdev, window->buffers[fb_idx].memory, 0);

    fb_idx++;
    fb_idx %= APP_BUFFER_COUNT;

#if APP_BUFFER_COUNT == 1
    // In case of single buffering an extra synchronization is needed;
    void *b = FBDEV_GetFrameBuffer(&window->display->g_fbdev, 0);
#endif
}
