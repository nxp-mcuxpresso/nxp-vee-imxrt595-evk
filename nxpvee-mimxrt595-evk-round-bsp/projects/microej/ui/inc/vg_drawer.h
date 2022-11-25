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

#if !defined VG_DRAWER_H
#define VG_DRAWER_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_DISPLAY.h>

#include "display_configuration.h"
#include "mej_math.h"

#include "vg_lite.h"

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief Function to update a color according to the drawer implementation.
 *
 * @param[in/out]: color: pointer to the color to update
 * @param[in]: blend: the blending mode to apply
 *
 * @return: nothing
 */
typedef void (* VG_DRAWER_update_color_t) (
		vg_lite_color_t* color,
		vg_lite_blend_t blend);

/*
 * @brief Function to update a gradient according to the drawer implementation.
 *
 * @param[in]: gradient: pointer to the gradient to update
 * @param[in]: blend: the blending mode to apply
 *
 * @return: nothing
 */
typedef void (* VG_DRAWER_update_gradient_t) (
		vg_lite_linear_gradient_t* gradient,
		vg_lite_blend_t blend);

/*
 * @brief Function to draw a path according to the drawer implementation.
 *
 * @see vg_lite_draw()
 *
 * @param[in]: target: the target used by the drawer implementation
 *
 * @return: the vglite error code
 */
typedef vg_lite_error_t (* VG_DRAWER_draw_path_t) (
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color);

/*
 * @brief Function to draw a path with gradient according to the drawer implementation.
 *
 * @see vg_lite_draw_gradient()
 *
 * @param[in]: target: the target used by the drawer implementation
 *
 * @return: the vglite error code
 */
typedef vg_lite_error_t (* VG_DRAWER_draw_gradient_t)(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend);

/*
 * @brief Function to blit a part of the source according to the drawer implementation.
 *
 * @see vg_lite_blit_rect()
 *
 * @param[in]: target: the target used by the drawer implementation
 *
 * @return: the vglite error code
 */
typedef vg_lite_error_t (* VG_DRAWER_blit_rect_t)(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter);

/*
 * @brief Function to blit the source according to the drawer implementation.
 *
 * @see vg_lite_blit()
 *
 * @param[in]: target: the target used by the drawer implementation
 *
 * @return: the vglite error code
 */
typedef vg_lite_error_t (* VG_DRAWER_blit_t)(
		void* target,
		vg_lite_buffer_t * source,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color,
		vg_lite_filter_t filter);

/*
 * @brief Function to clear a rectangle according to the drawer implementation.
 *
 * @see vg_lite_clear()
 *
 * @param[in]: target: the target used by the drawer implementation
 *
 * @return: the vglite error code
 */
typedef vg_lite_error_t (* VG_DRAWER_clear_t)(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color);

/*
 * @brief Operation to perform after a drawing operation.
 *
 * @param[in] vg_lite_error: the drawing operation's return code.
 *
 * @return: the MicroUI status according to the drawing operation status.
 *
 */
typedef DRAWING_Status (* VG_DRAWER_post_operation_t)(vg_lite_error_t vg_lite_error) ;

/*
 * @brief Defines a drawer
 */
typedef struct VG_DRAWER_drawer {

	/*
	 * @brief Draws a path using the drawer.
	 */
	VG_DRAWER_draw_path_t draw_path;

	/*
	 * @brief Draws a path with gradient using the drawer.
	 */
	VG_DRAWER_draw_gradient_t draw_gradient;

	/*
	 * @brief Blits a rectangle using the drawer.
	 */
	VG_DRAWER_blit_rect_t blit_rect;

	/*
	 * @brief Blits an image using the drawer.
	 */
	VG_DRAWER_blit_t blit;

	/*
	 * @brief Clears a rectangle using the drawer.
	 */
	VG_DRAWER_clear_t clear;

	/*
	 * @brief Updates a color according to the drawer implementation. Useful to update
	 * the color only once for several calls to drawing operations.
	 */
	VG_DRAWER_update_color_t update_color;

	/*
	 * @brief Updates a gradient according to the drawer implementation. Useful to update
	 * the gradient only once for several calls to drawing operations.
	 */
	VG_DRAWER_update_gradient_t update_gradient;

	/*
	 * @brief Performs a post operation after any drawings.
	 */
	VG_DRAWER_post_operation_t post_operation;

	/*
	 * @brief Used by the drawer to identify the destination (depends on drawer).
	 */
	void* target;

} VG_DRAWER_drawer_t;

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Gets the default drawer to use. The default drawer is the vglite drawer
 * (see VGLITE_PATH_get_vglite_drawer()). This drawer may be overrided by the
 * platform implementation.
 *
 * @see VGLITE_PATH_get_vglite_drawer()
 *
 * @return a drawer
 */
VG_DRAWER_drawer_t* VG_DRAWER_get_drawer(MICROUI_GraphicsContext* gc);

/*
 * @brief Configures and returns the target according to the MicroUI destination.
 * The target can be the destination buffer or a drawer that will execute the
 * drawings.
 *
 * @return: the target (a buffer or a drawer)
 */
void* VG_DRAWER_configure_target(MICROUI_GraphicsContext* gc) ;

/*
 * @brief Function to update a color to be compatible with the target.
 *
 * @param[in/out]: color: pointer to the color to update
 * @param[in]: blend: the blending mode to apply
 *
 * @return: nothing
 */
void VG_DRAWER_update_color(void* target, vg_lite_color_t* color, vg_lite_blend_t blend) ;

/*
 * @brief Function to update a gradient to be compatible with the target.
 *
 * @param[in/out]: color: pointer to the color to update
 * @param[in]: blend: the blending mode to apply
 *
 * @return: nothing
 */
void VG_DRAWER_update_gradient(void* target, vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) ;

/*
 * @brief Draws a path in the target or using the target as drawer.
 *
 * @param[in] target the destination or a drawer
 * other parameters: @see vg_lite_draw
 *
 * @return the VG-Lite error code.
 */
vg_lite_error_t VG_DRAWER_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color);

/*
 * @brief Draws a path with gradient in the target or using the target as drawer.
 *
 * @param[in] target the destination or a drawer
 * other parameters: @see vg_lite_draw_gradient
 *
 * @return the VG-Lite error code.
 */
vg_lite_error_t VG_DRAWER_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend);

/*
 * @brief Blits a rectangle in the target or using the target as drawer.
 *
 * @param[in] target the destination or a drawer
 * other parameters: @see vg_lite_blit_rect
 *
 * @return the VG-Lite error code.
 */
vg_lite_error_t VG_DRAWER_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter);

/*
 * @brief Blits an image in the target or using the target as drawer.
 *
 * @param[in] target the destination or a drawer
 * other parameters: @see vg_lite_blit
 *
 * @return the VG-Lite error code.
 */
vg_lite_error_t VG_DRAWER_blit(
		void* target,
		vg_lite_buffer_t * source,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color,
		vg_lite_filter_t filter);

/*
 * @brief Clears a rectangle in the target or using the target as drawer.
 *
 * @param[in] target the destination or a drawer
 * other parameters: @see vg_lite_clear
 *
 * @return the VG-Lite error code.
 */
vg_lite_error_t VG_DRAWER_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color);

/*
 * @brief Operation to perform after a drawing operation.
 *
 * @param[in] target: the destination or a drawer
 * @param[in] vg_lite_error: the drawing operation's return code.
 *
 * @return: the MicroUI status according to the drawing operation status.
 */
DRAWING_Status VG_DRAWER_post_operation(void* target, vg_lite_error_t vg_lite_error) ;

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined VG_DRAWER_H
