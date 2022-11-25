/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

#if !defined DISPLAY_CONFIGURATION_H
# define DISPLAY_CONFIGURATION_H

#if defined __cplusplus
extern "C" {
#endif

//#error "This header must be customized with platform specific configuration. Remove this #error when done. This file is not modified when a new version of the CCO is installed."

/**
 * @brief Compatibility sanity check value.
 * This define value is checked in the implementation to validate that the version of this configuration
 * is compatible with the implementation.
 *
 * This value must not be changed by the user of the CCO.
 * This value must be incremented by the implementor of the CCO when a configuration define is added, deleted or modified.
 */
#define DISPLAY_CONFIGURATION_VERSION (1)

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Width of the display panel
 */
#define DISPLAY_PANEL_WIDTH (400)

/*
 * @brief Height of the display panel
 */
#define DISPLAY_PANEL_HEIGHT (392)

/*
 * @brief Width of the frame buffers
 */
#define FRAME_BUFFER_WIDTH (392)

/*
 * @brief Height of the frame buffers
 */
#define FRAME_BUFFER_HEIGHT (392)

/*
 * @brief Source image line alignment (in bytes), required by VGLite library
 * when an image is used as source: 32 bits for RGB565 format
 * (see _check_source_aligned() in vg_lite.c).
 */
#define VGLITE_IMAGE_LINE_ALIGN_BYTE (32)

/*
 * @brief Frame buffer line alignment (in bytes), required by VGLite library
 * when frame buffer is used as source.
 * Note: Set (1) to not use the frame buffer as source image.
 * Note 2: Require a patch in fsl_dc_fb_dsi_cmd.c
 */
#define FRAME_BUFFER_LINE_ALIGN_BYTE (1) /* 1 | VGLITE_IMAGE_LINE_ALIGN_BYTE */

/*
 * @brief Available number of frame buffers
 */
#define FRAME_BUFFER_COUNT (2)

/*
 * @brief GPU is less efficient than CPU to perform simple aliased drawings (line, rectangle etc.)
 *
 * This define forces to use the GPU. Comment it to use the software algorithms instead.
 */
//#define VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

/*
 * @brief GPU is less efficient than CPU to perform a simple "draw image": when the image to render has
 * the same pixel definition than the destination buffer and no alpha blending is required.
 *
 * This defines forces to use the GPU to draw RGB565 images. Comment it to use the software algorithms
 * instead.
 */
//#define VGLITE_USE_GPU_FOR_RGB565_IMAGES

/*
 * @brief GCNanoLite-V does not support MSAA. By consequence the "draw image" functions (with or without
 * transformation like rotation or scale) cannot use GPU when the image to render contains transparent
 * pixels.
 *
 * This defines forces to use the GPU to draw transparent images. Comment it to use the software algorithms
 * instead.
 */
//#define VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES

/*
 * @brief Option to enable and disable the use of the GPU (toggle) at runtime. When the option is disabled,
 * the calls to DISPLAY_VGLITE_xxx_hardware_rendering have no effect.
 *
 * This defines forces to enable the option. Comment it to disable the option.
 */
//#define VGLITE_OPTION_TOGGLE_GPU

/*
 * @brief The multiple drawers option allows to use a specific drawer according to the MicroUI destination format.
 * (see LLUI_DISPLAY_isCustomFormat()).
 *
 * When disabled, all drawings are performed in software (by the Graphics Engine) or by the VG-Lite library (GPU).
 * In that case, all drawings are directly mapped on VG-Lite APIs to prevent useless indirections and performance
 * loss.
 *
 * When enabled and the destination format is custom, the platform has the responsability to redirect the drawings
 * to the expected drawer by overriding the default implementation of VG_DRAWER_get_drawer(). Note that the Graphics
 * Engine software algorithms don't manage the custom formats (no check).
 *
 * The drawing's callers should call vg_drawer.h APIs that check this option and redirect or not the drawings to
 * the registered drawer(s).
 */
//#define VGLITE_USE_MULTIPLE_DRAWERS

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined DISPLAY_CONFIGURATION_H
