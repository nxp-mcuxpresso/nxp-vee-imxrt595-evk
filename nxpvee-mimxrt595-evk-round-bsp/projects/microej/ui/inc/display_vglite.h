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

#if !defined DISPLAY_VGLITE_H
#define DISPLAY_VGLITE_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_PAINTER_impl.h>

#include "vglite_window.h"

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Flush the GPU operations.
 *
 * @param[in] wakeup_graphics_engine: true to wakeup the graphics engine as soon as
 * the drawing is performed, false to perform an active waiting until the GPU interrupt
 * is thrown
 */
void DISPLAY_VGLITE_start_operation(bool wakeup_graphics_engine);

/*
 * @brief Enables hardware rendering
 * @see VGLITE_OPTION_TOGGLE_GPU
 */
void DISPLAY_VGLITE_enable_hardware_rendering(void);

/*
 * @brief Disables hardware rendering
 * @see VGLITE_OPTION_TOGGLE_GPU
 */
void DISPLAY_VGLITE_disable_hardware_rendering(void);

/*
 * @brief Checks if hardware rendering is enabled
 *
 * @return: true if hardware rendering is enabled, false otherwise
 * @see VGLITE_OPTION_TOGGLE_GPU
 */
bool DISPLAY_VGLITE_is_hardware_rendering_enabled(void);

/*
 * @brief Toggles hardware rendering
 * @see VGLITE_OPTION_TOGGLE_GPU
 */
void DISPLAY_VGLITE_toggle_hardware_rendering(void);

/*
 * @brief Create display, window contexes and initialize the VGLite library
 */
void DISPLAY_VGLITE_init(void);

/*
 * @brief Get the main VGLite window
 */
vg_lite_window_t* DISPLAY_VGLITE_get_window(void);

/*
 * @brief Get the next graphics buffer to use
 */
vg_lite_buffer_t *DISPLAY_VGLITE_get_next_graphics_buffer(void);

/*
 * Because of VGLite library implementation,
 * a buffer located at the same address should not be modified and reused
 * as the VGLite library check if the buffer address match with the one
 * stored in its context. If matching is successfull, render target will not
 * be updated.
 * This function rotate over an array of vg_lite_buffers to give a different
 * buffer address at each call and configures it from a MICROUI_Image.
 * The caller MUST send it to the vg_lite library to update its context.
 *
 * @param[in] gc: graphics context of destination
 *
 * @return a vg_lite buffer that should not match vg_lite library render context
 */
vg_lite_buffer_t* DISPLAY_VGLITE_configure_destination(MICROUI_GraphicsContext* gc);

/*
 * This function configures a source buffer from a MICROUI_Image.
 *
 * @param[in] buffer: source buffer
 * @param[in] image: source image
 *
 * @return false if source image format is not supported by vg_lite, true on success
 */
bool DISPLAY_VGLITE_configure_source(vg_lite_buffer_t *buffer, MICROUI_Image* image);

/*
 * @brief RT595 Porter-Duff operators seems to not be functional
 * When using transparency, the colors passed to the RT595
 * need to be modifed to display the correct colors.
 * This modification also has an impact on the front-panel
 * that needs to be fixed.
 *
 * Note: This has been tested on SRC_OVER only.
 *
 * @param[in] color: the color in ARGB8888 format to convert
 * @return the converted color int ARGB8888 format
 */
uint32_t DISPLAY_VGLITE_porter_duff_workaround_ARGB8888(uint32_t color);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined DISPLAY_VGLITE_H
