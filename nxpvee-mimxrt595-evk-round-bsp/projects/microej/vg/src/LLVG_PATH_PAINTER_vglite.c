/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * Copyright 2023 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: implementation over VG-Lite.
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <math.h>

#include <LLVG_PATH_PAINTER_impl.h>
#include <LLVG_MATRIX_impl.h>
#include <LLVG_PATH_impl.h>

#include "microvg_path.h"
#include "microvg_helper.h"
#include "microvg_vglite_helper.h"
#include "vg_lite.h"
#include "color.h"
#include "display_vglite.h"
#include "vglite_path.h"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

#ifndef VG_FEATURE_PATH
#error "This implementation is only compatible when VG_FEATURE_PATH is set"
#endif

/*
 * @brief Specific error code when VG-Lite library throws an error.
 */
#define RET_ERROR_VGLITE -1

/*
 * @brief Specific error code when gradient data is invalid.
 */
#define RET_ERROR_GRADIENT -2

/*
 * @brief Fills a VGLite path with RAW path data
 */
#define PATH_TO_VGLITEPATH(p) \
		{ \
	{ \
			(vg_lite_float_t)(((MICROVG_PATH_HEADER_t*)(p))->bounds_xmin), (vg_lite_float_t)(((MICROVG_PATH_HEADER_t*)(p))->bounds_ymin),  \
			(vg_lite_float_t)(((MICROVG_PATH_HEADER_t*)(p))->bounds_xmax), (vg_lite_float_t)(((MICROVG_PATH_HEADER_t*)(p))->bounds_ymax)  \
	}, \
	VG_LITE_UPPER,  \
	(vg_lite_format_t)(((MICROVG_PATH_HEADER_t*)(p))->format),  \
	{0},  \
	(int32_t)(((MICROVG_PATH_HEADER_t*)(p))->data_size),  \
	(void*)&(((uint8_t*)(p))[((MICROVG_PATH_HEADER_t*)(p))->data_offset]),  \
	1 \
		}

// -----------------------------------------------------------------------------
// microvg_path.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
uint8_t MICROVG_PATH_get_path_encoder_format(void) {
	return VG_LITE_FP32;
}

// See the header file for the function documentation
uint32_t MICROVG_PATH_convert_path_command(jint command) {
	uint32_t ret;
	switch (command) {
	default: // unknown -> close (should not occur)
	case LLVG_PATH_CMD_CLOSE:
		ret = VLC_OP_END;
		break;
	case LLVG_PATH_CMD_MOVE:
		ret = VLC_OP_MOVE;
		break;
	case LLVG_PATH_CMD_MOVE_REL:
		ret = VLC_OP_MOVE_REL;
		break;
	case LLVG_PATH_CMD_LINE:
		ret = VLC_OP_LINE;
		break;
	case LLVG_PATH_CMD_LINE_REL:
		ret = VLC_OP_LINE_REL;
		break;
	case LLVG_PATH_CMD_QUAD:
		ret = VLC_OP_QUAD;
		break;
	case LLVG_PATH_CMD_QUAD_REL:
		ret = VLC_OP_QUAD_REL;
		break;
	case LLVG_PATH_CMD_CUBIC:
		ret = VLC_OP_CUBIC;
		break;
	case LLVG_PATH_CMD_CUBIC_REL:
		ret = VLC_OP_CUBIC_REL;
		break;
	}
	return ret;
}

// See the header file for the function documentation
uint32_t MICROVG_PATH_get_command_parameter_number(jint command) {
	uint32_t ret;
	switch (command) {
	default: // unknown -> close (should not occur)
	case LLVG_PATH_CMD_CLOSE:
		ret = 0;
		break;
	case LLVG_PATH_CMD_MOVE:
	case LLVG_PATH_CMD_MOVE_REL:
	case LLVG_PATH_CMD_LINE:
	case LLVG_PATH_CMD_LINE_REL:
		ret = 2;
		break;
	case LLVG_PATH_CMD_QUAD:
	case LLVG_PATH_CMD_QUAD_REL:
		ret = 4;
		break;
	case LLVG_PATH_CMD_CUBIC:
	case LLVG_PATH_CMD_CUBIC_REL:
		ret = 6;
		break;
	}
	return ret;
}

// -----------------------------------------------------------------------------
// LLVG_PAINTER_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_PATH_PAINTER_IMPL_drawPath(MICROUI_GraphicsContext* gc, jbyte* pathData, jint x, jint y, jfloat* matrix, jint fillRule, jint blend, jint color) {
	jint ret = LLVG_SUCCESS;
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_PATH_PAINTER_IMPL_drawPath)) && MICROVG_VGLITE_HELPER_enable_vg_lite_scissor(gc)) {

		vg_lite_path_t path = PATH_TO_VGLITEPATH(pathData);
		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_blend_t vg_lite_blend = MICROVG_VGLITE_HELPER_get_blend(blend);
		vg_lite_color_t vg_lite_color = (vg_lite_color_t)color;
		VG_DRAWER_update_color(target, &vg_lite_color, vg_lite_blend);

		vg_lite_matrix_t vg_lite_matrix;
		jfloat* mapped_matrix = MAP_VGLITE_MATRIX(&vg_lite_matrix);

		if((0 != x) || (0 != y)) {
			// Create translate matrix for initial x,y translation from graphicscontext.
			LLVG_MATRIX_IMPL_setTranslate(mapped_matrix, x, y);
			LLVG_MATRIX_IMPL_concatenate(mapped_matrix, matrix);
		}
		else {
			// use original matrix
			LLVG_MATRIX_IMPL_copy(mapped_matrix, matrix);
		}

		vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
				target,
				&path, MICROVG_VGLITE_HELPER_get_fill_rule(fillRule), &vg_lite_matrix,
				vg_lite_blend, vg_lite_color);


		if (VG_LITE_SUCCESS == vg_lite_error) {
			LLUI_DISPLAY_setDrawingStatus(VG_DRAWER_post_operation(target, vg_lite_error));
		}
		else {
			ret = RET_ERROR_VGLITE;
		}
	}
	return ret;
}

// See the header file for the function documentation
jint LLVG_PATH_PAINTER_IMPL_drawGradient(MICROUI_GraphicsContext* gc, jbyte* pathData, jint x, jint y, jfloat* matrix, jint fillRule, jint alpha, jint blend, jint* gradientData, jfloat* gradientMatrix) {

	jint ret = LLVG_SUCCESS;
	if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_PATH_PAINTER_IMPL_drawGradient)) && MICROVG_VGLITE_HELPER_enable_vg_lite_scissor(gc)) {

		vg_lite_path_t path = PATH_TO_VGLITEPATH(pathData);
		void* target = VG_DRAWER_configure_target(gc);

		vg_lite_linear_gradient_t gradient = {0};
		jfloat* local_gradient_matrix = MICROVG_HELPER_check_matrix(gradientMatrix);
		vg_lite_error_t vg_lite_error = MICROVG_VGLITE_HELPER_to_vg_lite_gradient(&gradient, gradientData, local_gradient_matrix, matrix, alpha);

		if (VG_LITE_SUCCESS == vg_lite_error) {

			vg_lite_matrix_t vg_lite_matrix;
			jfloat* mapped_matrix = MAP_VGLITE_MATRIX(&vg_lite_matrix);

			vg_lite_blend_t vglite_blend = MICROVG_VGLITE_HELPER_get_blend(blend);
			vg_lite_fill_t vglite_fill = MICROVG_VGLITE_HELPER_get_fill_rule(fillRule);
			VG_DRAWER_update_gradient(target, &gradient, vglite_blend);

			if((0 != x) || (0 != y)) {
				// Create translate matrix for initial x,y translation from graphicscontext.

				LLVG_MATRIX_IMPL_setTranslate(mapped_matrix, x, y);
				LLVG_MATRIX_IMPL_concatenate(mapped_matrix, matrix);

				vg_lite_linear_gradient_t translated_gradient;
				(void)memcpy(&translated_gradient, &gradient, sizeof(vg_lite_linear_gradient_t));
				jfloat* mapped_translated_gradient_matrix = MAP_VGLITE_GRADIENT_MATRIX(&translated_gradient);
				jfloat* mapped_gradient_matrix = MAP_VGLITE_GRADIENT_MATRIX(&gradient);
				LLVG_MATRIX_IMPL_setTranslate(mapped_translated_gradient_matrix, x, y);
				LLVG_MATRIX_IMPL_concatenate(mapped_translated_gradient_matrix, mapped_gradient_matrix);

				vg_lite_error = VG_DRAWER_draw_gradient(
						target,
						&path,
						vglite_fill,
						&vg_lite_matrix, &translated_gradient,
						vglite_blend);
			}
			else {
				// use original matrix and gradient

				LLVG_MATRIX_IMPL_copy(mapped_matrix, matrix);

				vg_lite_error = VG_DRAWER_draw_gradient(
						target,
						&path,
						vglite_fill,
						&vg_lite_matrix, &gradient,
						vglite_blend);
			}

			if (VG_LITE_SUCCESS == vg_lite_error) {
				LLUI_DISPLAY_setDrawingStatus(VG_DRAWER_post_operation(target, vg_lite_error));
			}
			else {
				ret = RET_ERROR_VGLITE;
			}
		}
		else {
			ret = RET_ERROR_GRADIENT;
		}

		// vg_lite_init_grad allocates a buffer in VGLite buffer, we must free it.
		// No error even if init_grad is never called because vg_lite_clear_grad
		// checks the allocation.
		vg_lite_clear_grad(&gradient);
	}
	return ret;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
