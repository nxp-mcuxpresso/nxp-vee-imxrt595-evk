/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined MICROVG_VGLITE_HELPER_H
#define MICROVG_VGLITE_HELPER_H

/**
* @file
* @brief MicroVG library low level API
* @author MicroEJ Developer Team
* @version 3.0.0
*/

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLVG_GRADIENT_impl.h>
#include <LLUI_DISPLAY.h>

#include "vg_lite.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief The vglite matrix can be mapped on a float array
 */
#define MAP_VGLITE_MATRIX(h) ((float*)(((vg_lite_matrix_t*)(h))->m))

/*
 * @brief The vglite gradient matrix can be mapped on a float array
 */
#define MAP_VGLITE_GRADIENT_MATRIX(h) MAP_VGLITE_MATRIX(&(((vg_lite_linear_gradient_t*)(h))->matrix))

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Converts the MicroVG blend in a VG-Lite blend.
 *
 * @param[in] blend: the drawing's blending
 *
 * @return the VG-Lite blend
 */
vg_lite_blend_t MICROVG_VGLITE_HELPER_get_blend(jint blend);

/*
 * @brief Converts the MicroVG fill rule in a VG-Lite rule.
 *
 * @param[in] blend: the drawing's fill rule
 *
 * @return the VG-Lite fill rule
 */
vg_lite_fill_t MICROVG_VGLITE_HELPER_get_fill_rule(jint fill_type);

/*
 * @brief Converts a MicroVG LinearGradient in a VG-Lite gradient according to
 * the drawing parameters. After this call, the gradient is ready to be updated
 * by calling vg_lite_update_grad()
 *
 * @param[in] gradient: the gradient destination (VG-Lite gradient)
 * @param[in] gradientData: the gradient source
 * @param[in] matrix: the gradient source's matrix
 * @param[in] globalMatrix: the drawing's matrix
 * @param[in] globalAlpha: the drawing's opacity
 *
 * @return a VG-Lite error code
 */
vg_lite_error_t MICROVG_VGLITE_HELPER_to_vg_lite_gradient(vg_lite_linear_gradient_t* gradient, jint* gradientData, jfloat* matrix, jfloat* globalMatrix, jint globalAlpha);

/*
 * @brief Configures the vg_lite's scissor according to the MicroUI clip.
 *
 * If the clip is empty, the drawing is useless. In that case the LLUI_DISPLAY's
 * drawing status is updated and this function returns false.
 *
 * Otherwise the VG-Lite scissor is configured to match the MicroUI clip.
 *
 * @param[in] gc: the destination where performing the drawing (MicroUI Graphics Context)
 *
 * @return false when the drawing is useless (bounds out of clip)
 */
bool MICROVG_VGLITE_HELPER_enable_vg_lite_scissor(MICROUI_GraphicsContext* gc);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_VGLITE_HELPER_H
