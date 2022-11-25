/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief Indirection of vector drawings to VG-Lite library or to a specific drawer.
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "vg_drawer.h"
#include "display_vglite.h"
#include "vglite_path.h"

#include "bsp_util.h"

// -----------------------------------------------------------------------------
// vg_drawer.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
inline BSP_DECLARE_WEAK_FCNT VG_DRAWER_drawer_t* VG_DRAWER_get_drawer(MICROUI_GraphicsContext* gc){
	// spec: default implementation returns the vglite drawer indirection
	return VGLITE_PATH_get_vglite_drawer(gc);
}

#ifndef VGLITE_USE_MULTIPLE_DRAWERS

/*
 * All drawings (and pre/post operations) are redirected to the vglite_path.h functions.
 * This prevent to use the unique drawer (see VG_DRAWER_get_drawer()) that makes useless
 * indirections.
 */

// See the header file for the function documentation
inline void* VG_DRAWER_configure_target(MICROUI_GraphicsContext* gc) {
	return (void*)DISPLAY_VGLITE_configure_destination(gc);
}

// See the header file for the function documentation
inline void VG_DRAWER_update_color(void* target, vg_lite_color_t* color, vg_lite_blend_t blend) {
	(void)target;
	VGLITE_PATH_update_color(color, blend);
}

// See the header file for the function documentation
inline void VG_DRAWER_update_gradient(void* target, vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) {
	(void)target;
	VGLITE_PATH_update_gradient(gradient, blend);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color){
	return vg_lite_draw((vg_lite_buffer_t*)target, path, fill_rule, matrix, blend, color);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend){
	return vg_lite_draw_gradient((vg_lite_buffer_t*)target, path, fill_rule, matrix, grad, blend);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter){
	return vg_lite_blit_rect((vg_lite_buffer_t*)target, source, rect, matrix, blend, color, filter);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_blit(
		void* target,
		vg_lite_buffer_t * source,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color,
		vg_lite_filter_t filter){
	return vg_lite_blit((vg_lite_buffer_t*)target, source, matrix, blend, color, filter);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color){
	return vg_lite_clear((vg_lite_buffer_t*)target, rectangle, color);
}

// See the header file for the function documentation
inline DRAWING_Status VG_DRAWER_post_operation(void* target, vg_lite_error_t vg_lite_error) {
	(void)target;
	return VGLITE_PATH_post_operation(vg_lite_error);
}

#else // VGLITE_USE_MULTIPLE_DRAWERS

/*
 * All drawings (and pre/post operations) are redirected to a specific drawer according to
 * the MicroUI destination's nature (see LLUI_DISPLAY_isCustomFormat()).
 */

// See the header file for the function documentation
inline void* VG_DRAWER_configure_target(MICROUI_GraphicsContext* gc) {
	return (void*)VG_DRAWER_get_drawer(gc);
}

// See the header file for the function documentation
inline void VG_DRAWER_update_color(void* target, vg_lite_color_t* color, vg_lite_blend_t blend) {
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	drawer->update_color(color, blend);
}

// See the header file for the function documentation
inline void VG_DRAWER_update_gradient(void* target, vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) {
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	drawer->update_gradient(gradient, blend);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color){
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->draw_path(drawer->target, path, fill_rule, matrix, blend, color);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend){
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->draw_gradient(drawer->target, path, fill_rule, matrix, grad, blend);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter){
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->blit_rect(drawer->target, source, rect, matrix, blend, color, filter);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_blit(
		void* target,
		vg_lite_buffer_t * source,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color,
		vg_lite_filter_t filter){
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->blit(drawer->target, source, matrix, blend, color, filter);
}

// See the header file for the function documentation
inline vg_lite_error_t VG_DRAWER_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color){
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->clear(drawer->target, rectangle, color);
}

// See the header file for the function documentation
inline DRAWING_Status VG_DRAWER_post_operation(void* target, vg_lite_error_t vg_lite_error) {
	// cppcheck-suppress [misra-c2012-11.5] cast to (VG_DRAWER_drawer_t *) is valid
	VG_DRAWER_drawer_t* drawer = (VG_DRAWER_drawer_t*)target;
	return drawer->post_operation(vg_lite_error);
}

#endif // VGLITE_USE_MULTIPLE_DRAWERS

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

