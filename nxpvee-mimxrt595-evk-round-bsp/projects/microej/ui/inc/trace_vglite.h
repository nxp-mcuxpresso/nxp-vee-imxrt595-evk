/*
 * C
 *
 * Copyright 2020-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined TRACE_VGLITE_H
#define TRACE_VGLITE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "trace_platform.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Logs an init gpu start event
 *
 * @param[in] operation: The VGLite operation sent to the GPU
 */
#define TRACE_VGLITE_INIT_GPU_START(operation) \
	TRACE_PLATFORM_START_U32( \
		VGLITE, \
		TRACE_VGLITE_INIT_GPU, \
		TRACE_VGLITE_OP_ ## operation \
			);

/*
 * @brief Logs an init gpu end event
 */
#define TRACE_VGLITE_INIT_GPU_END() \
	TRACE_PLATFORM_END_VOID( \
		VGLITE, \
		TRACE_VGLITE_INIT_GPU \
			);

/*
 * @brief INIT_GPU event identifier
 */
#define TRACE_VGLITE_INIT_GPU 1

/*
 * @brief VGLite operation CLEAR identifier
 * This event is expected to be used to trace a call to vg_lite_clear
 */
#define TRACE_VGLITE_OP_CLEAR 1

/*
 * @brief VGLite operation DRAW (draw path) identifier
 * This event is expected to be used to trace a call to vg_lite_draw
 */
#define TRACE_VGLITE_OP_DRAW 2

/*
 * @brief VGLite operation DRAW_GRAD (draw path with gradient) identifier
 * This event is expected to be used to trace a call to vg_lite_draw_grad
 */
#define TRACE_VGLITE_OP_DRAW_GRAD 3

/*
 * @brief VGLite operation BLIT (draw image) identifier
 * This event is expected to be used to trace a call to vg_lite_blit
 */
#define TRACE_VGLITE_OP_BLIT 4

/*
 * @brief VGLite operation BLIT_RECT (draw image area) identifier
 * This event is expected to be used to trace a call to vg_lite_blit_rect
 */
#define TRACE_VGLITE_OP_BLIT_RECT 5

/*
 * Trace start macros
 *
 * param[in] event: LLVGLITE Event to trace
 */
#define __TRACE_START(event) \
		TRACE_PLATFORM_START_VOID(LLVGLITE, LLVGLITE_TRACE_ ## event)

/*
 * Trace end macros
 *
 * param[in] event: LLVGLITE Event to trace
 */
#define __TRACE_END(event) \
		TRACE_PLATFORM_END_VOID(LLVGLITE, LLVGLITE_TRACE_ ## event);

/*
 * @brief Low Level VGLite API DRAW (draw path) identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_draw
 */
#define LLVGLITE_TRACE_DRAW (10 + TRACE_VGLITE_OP_DRAW)

/*
 * @brief VLow Level VGLite API DRAW_GRAD (draw path with gradient) identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_draw_grad
 */
#define LLVGLITE_TRACE_DRAW_GRAD (10 + TRACE_VGLITE_OP_DRAW_GRAD)

/*
 * @brief VLow Level VGLite API MATRIX_IDENTITY (set matrix to identity)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_identity
 */
#define LLVGLITE_TRACE_MATRIX_IDENTITY 21

/*
 * @brief VLow Level VGLite API MATRIX_TRANSLATE (update matrix for translation)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_translate
 */
#define LLVGLITE_TRACE_MATRIX_TRANSLATE 22

/*
 * @brief VLow Level VGLite API MATRIX_TRANSLATE (update matrix for rotation)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_rotate
 */
#define LLVGLITE_TRACE_MATRIX_ROTATE 23

/*
 * @brief VLow Level VGLite API MATRIX_TRANSLATE (update matrix for scale)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_scale
 */
#define LLVGLITE_TRACE_MATRIX_SCALE 24

/*
 * @brief VLow Level VGLite API MATRIX_PERPECTIVE
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_perspective
 */
#define LLVGLITE_TRACE_MATRIX_PERSPECTIVE 25

/*
 * @brief VLow Level VGLite API MATRIX_COPY (copy matrix parameters from an other matrix)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_copy
 */
#define LLVGLITE_TRACE_MATRIX_COPY 26

/*
 * @brief VLow Level VGLite API MATRIX_COPY (copy matrix parameters from an other matrix)
 * identifier
 * This event is expected to be used to trace a call to the java API
 * vg_lite_matrix_copy
 */
#define LLVGLITE_TRACE_MATRIX_CONCATENATE 27

#endif // !defined TRACE_VGLITE_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------


