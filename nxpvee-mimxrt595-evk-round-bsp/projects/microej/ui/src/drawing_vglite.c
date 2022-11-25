/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 *  @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_DISPLAY.h>

#include "ui_drawing_soft.h"
#include "dw_drawing_soft.h"
#include "ui_drawing.h"
#include "dw_drawing.h"

#include "display_configuration.h"
#include "display_impl.h"
#include "display_vglite.h"
#include "vglite_path.h"

#include "vg_lite.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Scale factor to improve drawing quality in vector mode
 */
#define DRAWING_SCALE_FACTOR      (2)
#define DRAWING_SCALE_DIV         (1.0 / (vg_lite_float_t)DRAWING_SCALE_FACTOR)

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/*
 * @brief Variable to store the vglite pathes during the GPU computation.
 *
 * MicroUI design must ensure that only one computation is done at a time.
 */
static union {
	vglite_path_line_t line; // line
	vglite_path_thick_shape_line_t thick_shape_line; // thick shape line
	vglite_path_ellipse_t ellipse[2]; // ellipse (in & out)
	vglite_path_ellipse_arc_t ellipse_arc; // ellipse arc
	vglite_path_rounded_rectangle_t rounded_rectangle[2]; // round rect (in & out)
	vglite_path_thick_ellipse_arc_t thick_ellipse_arc; // anti aliased ellipse arc
} __shape_paths;

/*
 * @brief Variable to target a vglite source buffer (there is only one source at any time)
 */
static vg_lite_buffer_t source_buffer;

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Checks MicroUI clip and updates the graphical engine's drawing limits on demand. This
 * update is useless when calling vg_lite draw_path function because the callback already updates
 * the graphical engine drawing limits. See vg_lite_draw_notify_render_area() in display_vglite.c.
 *
 * @param[in] gc: Graphics context where to draw
 * @param[in] x1 the top-left pixel X coordinate.
 * @param[in] y1 the top-left pixel Y coordinate.
 * @param[in] x2 the bottom-right pixel X coordinate.
 * @param[in] y2 the top-right pixel Y coordinate.
 * @param[in] update_drawing_limits true to update drawing limits
 *
 * @return false when the limits are fully outside the clip, true when at least
 * one pixel fits the clip.
 */
static bool __check_clip(
		MICROUI_GraphicsContext* gc,
		int x1,
		int y1,
		int x2,
		int y2,
		bool update_drawing_limits);

/*
 * @brief Draws an ellipse arc covering the square specified by its diameter.
 * See DRAWING_impl.h for more information
 *
 * @param[in] gc: the MicroUI GraphicsContext target.
 * @param[in] x: the x coordinate of the upper-left corner of the square where the ellipse arc is drawn
 * @param[in] y: the y coordinate of the upper-left corner of the square where the ellipse arc is drawn
 * @param[in] diameter_w: the horizontal diameter of the ellipse arc to draw
 * @param[in] diameter_h: the vertical diameter of the ellipse arc to draw
 * @param[in] start_angle_deg: the beginning angle of the arc to draw
 * @param[in] arc_angle_deg: the angular extent of the arc from start_angle_deg
 * @param[in] fill: the flag indicating if the ellipse arc will be filled
 * - true: the ellipse arc will be filled
 * - false: the ellipse arc will not be filled
 *
 * @return the drawing status.
 */
static DRAWING_Status __draw_ellipse_arc(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_w,
		int diameter_h,
		float start_angle,
		float arc_angle,
		bool fill);

/*
 * @brief Draws a ellipse covering the square specified by its diameter.
 * See DRAWING_impl.h for more information
 *
 * @param[in] gc: the MicroUI GraphicsContext target.
 * @param[in] x: the x coordinate of the upper-left corner of the square where the ellipse is drawn
 * @param[in] y: the y coordinate of the upper-left corner of the square where the ellipse is drawn
 * @param[in] diameter_out_w: the horizontal outer diameter of the ellipse to draw
 * @param[in] diameter_out_h: the vertical outer diameter of the ellipse to draw
 * @param[in] diameter_in_w: the horizontal inner diameter of the ellipse to draw
 * @param[in] diameter_in_h: the vertical inner diameter of the ellipse to draw
 *
 * @return the drawing status.
 */
static DRAWING_Status __draw_ellipse(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_out_w,
		int diameter_out_h,
		int diameter_in_w,
		int diameter_in_h);

/*
 * @brief Fills a ellipse covering the square specified by its diameter.
 * See DRAWING_impl.h for more information
 *
 * @param[in] gc: the MicroUI GraphicsContext target.
 * @param[in] x: the x coordinate of the upper-left corner of the square where the ellipse is drawn
 * @param[in] y: the y coordinate of the upper-left corner of the square where the ellipse is drawn
 * @param[in] diameter_w: the horizontal diameter of the ellipse to draw
 * @param[in] diameter_h: the vertical diameter of the ellipse to draw
 *
 * @return the drawing status.
 */
static DRAWING_Status __fill_ellipse(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_w,
		int diameter_h);

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
/*
 * @brief Draws a line between two points
 * See DRAWING_impl.h for more information
 *
 * @param[in] gc the MicroUI GraphicsContext target.
 * @param[in] x1: the horizontal coordinate of the start of the line
 * @param[in] y1: the vertical coordinate of start of the line
 * @param[in] x2: the horizontal coordinate of the end of the line
 * @param[in] y2: the vertical coordinate of end of the line
 *
 * @return the drawing status.
 */
static DRAWING_Status __draw_line(
		MICROUI_GraphicsContext* gc,
		int x1,
		int y1,
		int x2,
		int y2);
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

/*
 * @brief Draws a thick line between two points
 * See DRAWING_impl.h for more information
 *
 * @param[in] gc the MicroUI GraphicsContext target.
 * @param[in] x1: the horizontal coordinate of the start of the line
 * @param[in] y1: the vertical coordinate of start of the line
 * @param[in] x2: the horizontal coordinate of the end of the line
 * @param[in] y2: the vertical coordinate of end of the line
 * @param[in] thickness: the line thickness.
 * @param[in] start cap representation of start of the line
 * @param[in] end cap representation of end of the line
 *
 * @return the drawing status.
 */
static DRAWING_Status __thick_line(
		MICROUI_GraphicsContext* gc,
		int x1,
		int y1,
		int x2,
		int y2,
		int thickness,
		DRAWING_Cap start,
		DRAWING_Cap end);

/*
 * @brief Draws a thick arc covering the square specified by its diameter.
 * See dw_drawing.h for more information
 *
 * @param[in] gc: the MicroUI GraphicsContext target.
 * @param[in] x: the x coordinate of the upper-left corner of the square where the arc is drawn
 * @param[in] y: the y coordinate of the upper-left corner of the square where the arc is drawn
 * @param[in] diameter_w: the horizontal diameter of the ellipse arc to draw
 * @param[in] diameter_h: the vertical diameter of the ellipse arc to draw
 * @param[in] start_angle_deg: the beginning angle of the arc to draw
 * @param[in] arc_angle_deg: the angular extent of the arc from start_angle_deg
 * @param[in] thickness: the arc thickness.
 * @param[in] start: cap representation of start of shape
 * @param[in] end: cap representation of end of shape
 *
 * @return the drawing status.
 */
static DRAWING_Status __draw_thick_shape_ellipse_arc(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_w,
		int diameter_h,
		float start_angle_deg,
		float arc_angle,
		int thickness,
		DRAWING_Cap start,
		DRAWING_Cap end);

/*
 * @brief Rotates an image using the GPU
 *
 * @param[in] gc: Graphics context where to draw
 * @param[in] img: Source image to draw
 * @param[in] src: Source buffer
 * @param[in] x: Horizontal coordinate of the image reference anchor top-left point.
 * @param[in] y: Vertical coordinate of the image reference anchor top-left point.
 * @param[in] xRotation: Horizontal coordinate of the rotation center
 * @param[in] yRotation: Vertical coordinate of the rotation center
 * @param[in] angle: Angle that must be used to rotate the image
 * @param[in] alpha: The opacity level to apply while drawing the rotated image
 * @param[in] filter: Quality of drawing.
 *
 * @return The drawing status.
 */
static DRAWING_Status __rotate_image(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		vg_lite_buffer_t* src,
		int x,
		int y,
		int xRotation,
		int yRotation,
		float angle_deg,
		int alpha,
		vg_lite_filter_t filter);

/*
 * @brief Scales an image using the GPU
 *
 * @param[in] gc: Graphics context where to draw.
 * @param[in] img: Source image to draw.
 * @param[in] src: Source buffer
 * @param[in] x: Horizontal coordinate of the image reference anchor top-left point.
 * @param[in] y: Vertical coordinate of the image reference anchor top-left point.
 * @param[in] factorX: Horizontal scaling factor.
 * @param[in] yRotation: Vertical scaling factor.
 * @param[in] alpha: The opacity level to apply while drawing the image
 * @param[in] filter: Quality of drawing.
 *
 * @return The drawing status.
 */
static DRAWING_Status __scale_image(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		vg_lite_buffer_t* src,
		int x,
		int y,
		float factorX,
		float factorY,
		int alpha,
		vg_lite_filter_t filter);

/*
 * @brief Get color to give to vglite library according to destination's foreground
 * color, image format and opacity level.
 *
 * @param[in] gc: the MicroUI GraphicsContext of destination
 * @param[in] img: the MicroUI Image of source
 * @param[in] alpha: the application opacity
 *
 * @return The vg_lite color.
 */
static vg_lite_color_t __get_vglite_color(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint alpha);

/*
 * @brief Configures vg_lite_buffer_t according to the MicroUI Image.
 *
 * @param[in] buffer: the vg_lite_buffer_t to configure.
 * @param[in] image: the MicroUI Image.
 *
 * @return true when the vg_lite_buffer_t has been configured, false when GPU cannot
 * be used to render the image for any reason.
 */
static bool __configure_source(vg_lite_buffer_t *buffer, MICROUI_Image* image);

/*
 * Tells if the source and destination are GPU compatible. If yes, configure the given target, matrix, blit rectangle and color.
 *
 * @param[in] gc ... alpha: see UI_DRAWING_drawImage()
 * @param[out] target: pointer on the target to configure
 * @param[out] color: pointer on the color to configure
 * @param[out] matrix: pointer on the matrix to configure
 * @param[out] blit_rect: pointer on the blit rectangle to configure
 *
 * @return true if the GPU can be used, false to draw the image in software.
 */
static bool __prepare_gpu_draw_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha, void** target, vg_lite_color_t* color, vg_lite_matrix_t* matrix, uint32_t* blit_rect);

/*
 * Draws the image by using the Graphics Engine' software algorithm
 *
 * @param[in] gc ... alpha: see UI_DRAWING_drawImage()
 */
static void __soft_draw_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha);

/*
 * Draws a region of an image at another position by using the GPU.
 *
 * This function draws block per block to prevent the overlapping.
 *
 * @param[in] target ... rect: see __prepare_gpu_draw_image()
 * @param[in] element_index: the matrix and rectangle offset (0 to target X/width and 1 to target Y/height)
 *
 * @return DRAWING_RUNNING on success
 */
static DRAWING_Status __draw_region_with_overlap(void* target, vg_lite_color_t color, vg_lite_matrix_t* matrix, uint32_t* rect, uint32_t element_index) ;

// -----------------------------------------------------------------------------
// ui_drawing.h and dw_drawing.h functions
// -----------------------------------------------------------------------------

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawLine(
		MICROUI_GraphicsContext* gc,
		jint x1, jint y1,
		jint x2, jint y2) {

	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawLine(gc, x1, y1, x2, y2);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		// TODO try to crop region (x1 may lower than x2 and y1 may be lower than y2)
		if (__check_clip(
				gc,
				0, 0,
				gc->image.width, gc->image.height, false)) {
			ret = __draw_line(gc, x1, y1, x2, y2);
		}
		else {
			// nothing to draw
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawHorizontalLine(
		MICROUI_GraphicsContext* gc,
		jint x1, jint x2, jint y) {

	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawLine(gc, x1, y, x2, y);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		if (__check_clip(gc, x1, y, x2, y, false)) {
			ret = __draw_line(gc, x1, y, x2, y);
		}
		else {
			// nothing to draw
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawVerticalLine(
		MICROUI_GraphicsContext* gc,
		jint x, jint y1, jint y2) {

	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawLine(gc, x, y1, x, y2);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		if (__check_clip(gc, x, y1, x, y2, false)) {
			ret = __draw_line(gc, x, y1, x, y2);
		}
		else {
			// nothing to draw
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillRectangle(
		MICROUI_GraphicsContext* gc,
		jint x1, jint y1, jint x2, jint y2) {

	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillRectangle(gc, x1, y1, x2, y2);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		if (__check_clip(gc, x1, y1, x2, y2, true)) {

			vg_lite_rectangle_t rectangle;
			rectangle.x = x1;
			rectangle.y = y1;
			rectangle.width = (x2-x1+1);
			rectangle.height = (y2-y1+1);

			void* target = VG_DRAWER_configure_target(gc);
			vg_lite_error_t vg_lite_error = VG_DRAWER_clear(target, &rectangle, gc->foreground_color);
			ret = VG_DRAWER_post_operation(target, vg_lite_error);
		}
		else {
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawRoundedRectangle(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height,
		jint arc_width, jint arc_height) {

	int path_offset;
	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawRoundedRectangle(gc, x, y, width, height, arc_width,  arc_height);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		if (__check_clip(gc, x, y, x + width, y + height, true)) {

			vg_lite_identity(&matrix);
			// Compute the rounded rectangle shape path
			shape_vg_path.path = &__shape_paths.rounded_rectangle[0];
			shape_vg_path.path_length = sizeof(__shape_paths.rounded_rectangle);
			path_offset = 0;
			path_offset = VGLITE_PATH_compute_rounded_rectangle(
					&shape_vg_path, path_offset,
					x, y, width, height, arc_width, arc_height, false);

			path_offset = VGLITE_PATH_compute_rounded_rectangle(
					&shape_vg_path, path_offset,
					x + 1, y + 1,
					width - 2, height - 2,
					arc_width - 1, arc_height - 1,
					true);

			if (0 > path_offset) {
				DISPLAY_IMPL_error(false, "Error during path computation");
				ret = DRAWING_DONE;
			}
			else {
				// Draw the point with the GPU
				void* target = VG_DRAWER_configure_target(gc);
				vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
						target,
						&shape_vg_path,
						VG_LITE_FILL_EVEN_ODD,
						&matrix,
						VG_LITE_BLEND_SRC_OVER,
						gc->foreground_color
				);
				ret = VG_DRAWER_post_operation(target, vg_lite_error);
			}
		}
		else {
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillRoundedRectangle(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height,
		jint arc_width, jint arc_height) {

	int path_offset;
	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillRoundedRectangle(gc, x, y, width, height, arc_width,  arc_height);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		// Check if there is something to draw and clip drawing limits
		if (__check_clip(gc, x, y, x + width, y + height, true)) {

			vg_lite_identity(&matrix);
			// Compute the rounded rectangle shape path
			shape_vg_path.path = &__shape_paths.rounded_rectangle[0];
			shape_vg_path.path_length = sizeof(__shape_paths.rounded_rectangle);
			path_offset = VGLITE_PATH_compute_rounded_rectangle(&shape_vg_path, 0,
					x, y, width, height, arc_width, arc_height, true);

			if (0 > path_offset) {
				DISPLAY_IMPL_error(false, "Error during path computation");
				ret = DRAWING_DONE;
			}
			else {
				// Draw the point with the GPU
				void* target = VG_DRAWER_configure_target(gc);
				vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
						target,
						&shape_vg_path,
						VG_LITE_FILL_EVEN_ODD,
						&matrix,
						VG_LITE_BLEND_SRC_OVER,
						gc->foreground_color
				);
				ret = VG_DRAWER_post_operation(target, vg_lite_error);
			}
		}
		else {
			ret = DRAWING_DONE;
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawCircleArc(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint diameter,
		jfloat start_angle,
		jfloat arc_angle) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawCircleArc(gc, x, y, diameter, start_angle, arc_angle);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		if (MEJ_ABS(arc_angle) >= 360) {
			ret = UI_DRAWING_drawCircle(gc, x, y, diameter);
		}
		else {
			ret = __draw_ellipse_arc(
					gc,
					x, y,
					diameter, diameter,
					start_angle, arc_angle,
					false);
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawEllipseArc(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint width,
		jint height,
		jfloat start_angle,
		jfloat arc_angle) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawEllipseArc(
				gc,
				x, y,
				width, height,
				start_angle, arc_angle);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		if (MEJ_ABS(arc_angle) >= 360) {
			ret = UI_DRAWING_drawEllipse(gc, x, y, width, height);
		}
		else {
			ret = __draw_ellipse_arc(
					gc,
					x, y,
					width, height,
					start_angle, arc_angle,
					false);
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillCircleArc(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint diameter,
		jfloat start_angle, jfloat arc_angle) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillCircleArc(
				gc,
				x, y,
				diameter,
				start_angle, arc_angle);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		if (MEJ_ABS(arc_angle) >= 360) {
			ret = UI_DRAWING_fillCircle(gc, x, y, diameter);
		}
		else {
			ret = __draw_ellipse_arc(
					gc,
					x, y,
					diameter, diameter,
					start_angle, arc_angle,
					true);
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillEllipseArc(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height,
		jfloat start_angle, jfloat arc_angle) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillEllipseArc(
				gc,
				x, y,
				width, height,
				start_angle, arc_angle);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		if (MEJ_ABS(arc_angle) >= 360) {
			ret = UI_DRAWING_fillEllipse(
					gc,
					x, y,
					width, height);
		}
		else {
			ret = __draw_ellipse_arc(
					gc,
					x, y,
					width, height,
					start_angle, arc_angle,
					true);
		}

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawEllipse(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawEllipse(gc, x, y, width, height);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __draw_ellipse(gc, x, y, width, height, width - 2, height - 2);

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillEllipse(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillEllipse(gc, x, y, width, height);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __fill_ellipse(gc, x, y, width, height);

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawCircle(
		MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_drawCircle(gc, x, y, diameter);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __draw_ellipse(
				gc,
				x, y,
				diameter, diameter,
				diameter - 2, diameter - 2);

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_fillCircle(
		MICROUI_GraphicsContext* gc, jint x, jint y, jint diameter) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		UI_DRAWING_SOFT_fillCircle(gc, x, y, diameter);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __fill_ellipse(gc, x, y, diameter, diameter);

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickFadedPoint(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint thickness,
		jint fade) {

	DRAWING_Status ret;
	if ((fade > 1) // No radial gradient with hardware
#ifdef VGLITE_OPTION_TOGGLE_GPU
			|| !DISPLAY_VGLITE_is_hardware_rendering_enabled()
#endif // VGLITE_OPTION_TOGGLE_GPU
	) {
		DW_DRAWING_SOFT_drawThickFadedPoint(gc, x, y, thickness,fade);
		ret = DRAWING_DONE;
	}
	else{
		ret = __fill_ellipse(gc, x - (thickness/2), y - (thickness/2), thickness, thickness);
	}
	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickFadedLine(
		MICROUI_GraphicsContext* gc,
		jint x1,
		jint y1,
		jint x2,
		jint y2,
		jint thickness,
		jint fade,
		DRAWING_Cap start,
		DRAWING_Cap end) {
	DRAWING_Status ret;

	if ((fade > 1) // No radial gradient with hardware
#ifdef VGLITE_OPTION_TOGGLE_GPU
			|| !DISPLAY_VGLITE_is_hardware_rendering_enabled()
#endif // VGLITE_OPTION_TOGGLE_GPU
	) {
		DW_DRAWING_SOFT_drawThickFadedLine(
				gc, x1, y1, x2, y2, thickness, fade, start, end);
		ret = DRAWING_DONE;
	}
	else {
		ret = __thick_line(
				gc,
				x1,
				y1,
				x2,
				y2,
				thickness,
				start,
				end);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickFadedCircle(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint diameter, jint thickness, jint fade) {

	int diameter_out;
	int diameter_in;
	DRAWING_Status ret;

	if ((fade > 1) // No radial gradient with hardware
#ifdef VGLITE_OPTION_TOGGLE_GPU
			|| !DISPLAY_VGLITE_is_hardware_rendering_enabled()
#endif // VGLITE_OPTION_TOGGLE_GPU
	) {
		DW_DRAWING_SOFT_drawThickFadedCircle(
				gc, x, y, diameter, thickness, fade);
		ret = DRAWING_DONE;
	}
	else {

		diameter_out = diameter + thickness;
		diameter_in = diameter - thickness;

		ret = __draw_ellipse(
				gc,
				x - (thickness / 2), y - (thickness / 2),
				diameter_out, diameter_out,
				diameter_in, diameter_in);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickFadedCircleArc(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint diameter,
		jfloat start_angle_deg,
		jfloat arc_angle,
		jint thickness,
		jint fade,
		DRAWING_Cap start,
		DRAWING_Cap end) {
	DRAWING_Status ret;

	if ((fade > 1) // No radial gradient with hardware
#ifdef VGLITE_OPTION_TOGGLE_GPU
			|| !DISPLAY_VGLITE_is_hardware_rendering_enabled()
#endif // VGLITE_OPTION_TOGGLE_GPU
	) {
		DW_DRAWING_SOFT_drawThickFadedCircleArc(	gc,
				x,
				y,
				diameter,
				start_angle_deg,
				arc_angle,
				thickness,
				fade,
				start,
				end );
		ret = DRAWING_DONE;
	}
	else {
		ret = __draw_thick_shape_ellipse_arc(
				gc,
				x, y,
				diameter, diameter,
				start_angle_deg, arc_angle,
				thickness,
				start, end);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickFadedEllipse(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height,
		jint thickness, jint fade) {

	int diameter_out_w;
	int diameter_out_h;
	int diameter_in_w;
	int diameter_in_h;
	DRAWING_Status ret;

	if ((fade > 1) // No radial gradient with hardware
#ifdef VGLITE_OPTION_TOGGLE_GPU
			|| !DISPLAY_VGLITE_is_hardware_rendering_enabled()
#endif // VGLITE_OPTION_TOGGLE_GPU
	) {
		DW_DRAWING_SOFT_drawThickFadedEllipse(
				gc,
				x, y,
				width, height,
				thickness, fade);
		ret = DRAWING_DONE;
	}
	else {

		diameter_out_w = width + thickness;
		diameter_out_h = height + thickness;
		diameter_in_w = width - thickness;
		diameter_in_h = height - thickness;

		ret = __draw_ellipse(
				gc,
				x - (thickness / 2), y - (thickness / 2),
				diameter_out_w, diameter_out_h,
				diameter_in_w, diameter_in_h);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickLine(
		MICROUI_GraphicsContext* gc,
		jint x1,
		jint y1,
		jint x2,
		jint y2,
		jint thickness) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	// Check if rendering should be done in software
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		DW_DRAWING_SOFT_drawThickLine(gc, x1, y1, x2, y2, thickness);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __thick_line(	gc,
				x1,
				y1,
				x2,
				y2,
				thickness,
				DRAWING_ENDOFLINE_NONE,
				DRAWING_ENDOFLINE_NONE);
#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickCircle(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint diameter,
		jint thickness) {

	int diameter_out;
	int diameter_in;
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		DW_DRAWING_SOFT_drawThickCircle(gc, x, y, diameter, thickness);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		diameter_out = diameter + thickness;
		diameter_in = diameter - thickness;

		ret = __draw_ellipse(
				gc,
				x - (thickness / 2), y - (thickness / 2),
				diameter_out, diameter_out,
				diameter_in, diameter_in);
#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

DRAWING_Status DW_DRAWING_drawThickEllipse(
		MICROUI_GraphicsContext* gc,
		jint x, jint y,
		jint width, jint height,
		jint thickness) {

	int diameter_out_w;
	int diameter_out_h;
	int diameter_in_w;
	int diameter_in_h;
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		DW_DRAWING_SOFT_drawThickEllipse(gc, x, y, width, height, thickness);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU


		diameter_out_w = width + thickness;
		diameter_out_h = height + thickness;
		diameter_in_w = width - thickness;
		diameter_in_h = height - thickness;

		ret = __draw_ellipse(
				gc,
				x - (thickness / 2), y - (thickness / 2),
				diameter_out_w, diameter_out_h,
				diameter_in_w, diameter_in_h);
#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawThickCircleArc(
		MICROUI_GraphicsContext* gc,
		jint x,
		jint y,
		jint diameter,
		jfloat start_angle_deg,
		jfloat arc_angle,
		jint thickness) {
	DRAWING_Status ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		DW_DRAWING_SOFT_drawThickCircleArc(
				gc, x, y, diameter, start_angle_deg, arc_angle, thickness);
		ret = DRAWING_DONE;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

		ret = __draw_thick_shape_ellipse_arc(
				gc,
				x, y,
				diameter, diameter,
				start_angle_deg, arc_angle,
				thickness,
				DRAWING_ENDOFLINE_NONE,
				DRAWING_ENDOFLINE_NONE);
#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the header file for the function documentation
DRAWING_Status UI_DRAWING_drawImage(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint x_src,
		jint y_src,
		jint width,
		jint height,
		jint x_dest,
		jint y_dest,
		jint alpha) {

	DRAWING_Status ret;
	void* target;
	vg_lite_color_t color;
	vg_lite_matrix_t matrix;
	uint32_t blit_rect[4];

	if (__prepare_gpu_draw_image(gc, img, x_src, y_src, width, height, x_dest, y_dest, alpha, &target, &color, &matrix, blit_rect)){
		if (VG_LITE_SUCCESS == VG_DRAWER_blit_rect(
				target,
				&source_buffer,
				blit_rect,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				color,
				VG_LITE_FILTER_POINT)){
			// draw source on itself applying an opacity and without overlap
			DISPLAY_VGLITE_start_operation(true);
			ret = DRAWING_RUNNING;
		}
		else{
			DISPLAY_IMPL_error(false, "Error during draw image");
			ret = DRAWING_DONE;
		}
	}
	else {
		__soft_draw_image(gc, img, x_src, y_src, width, height, x_dest, y_dest, alpha);
		ret = DRAWING_DONE;
	}

	return ret;
}

DRAWING_Status UI_DRAWING_drawRegion(MICROUI_GraphicsContext* gc, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha){

	DRAWING_Status ret;
	void* target;
	vg_lite_color_t color;
	vg_lite_matrix_t matrix;
	uint32_t blit_rect[4];

	if (__prepare_gpu_draw_image(gc, &gc->image, x_src, y_src, width, height, x_dest, y_dest, alpha, &target, &color, &matrix, blit_rect)){

		if ((y_dest == y_src) && (x_dest > x_src) && (x_dest < (x_src + width))){
			// draw with overlap: cut the drawings in several widths
			ret = __draw_region_with_overlap(target, color, &matrix, blit_rect, 0);
		}
		else if ((y_dest > y_src) && (y_dest < (y_src + height))){
			// draw with overlap: cut the drawings in several heights
			ret = __draw_region_with_overlap(target, color, &matrix, blit_rect, 1);
		}
		else if (VG_LITE_SUCCESS == VG_DRAWER_blit_rect(
				target,
				&source_buffer,
				blit_rect,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				color,
				VG_LITE_FILTER_POINT)){
			// draw source on itself applying an opacity and without overlap
			DISPLAY_VGLITE_start_operation(true);
			ret = DRAWING_RUNNING;
		}
		else{
			DISPLAY_IMPL_error(false, "Error during draw region");
			ret = DRAWING_DONE;
		}
	}
	else {
		__soft_draw_image(gc, &gc->image, x_src, y_src, width, height, x_dest, y_dest, alpha);
		ret = DRAWING_DONE;
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawFlippedImage(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint region_x,
		jint region_y,
		jint width,
		jint height,
		jint x,
		jint y,
		DRAWING_Flip transformation,
		jint alpha) {

	DRAWING_Status ret;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
	uint32_t blit_rect[4];
#endif

	if(!__configure_source(&source_buffer, img)
			|| (region_x != 0)	// GPU Limitation: origin different
			|| (region_y != 0)	// than (0, 0) not supported

#ifndef VGLITE_USE_GPU_FOR_RGB565_IMAGES
			// CPU (memcpy) is faster than GPU
			|| ((MICROUI_IMAGE_FORMAT_RGB565 == img->format) && (DRAWING_FLIP_NONE == transformation) && (0xff == alpha))
#endif
	)
	{
		DW_DRAWING_SOFT_drawFlippedImage(
				gc, img,
				region_x, region_y,
				width, height,
				x, y,
				transformation, alpha);
		ret = DRAWING_DONE;
	}
	else {
		// Check if there is something to draw and clip drawing limits
		// TODO try to crop region
		if (__check_clip(
				gc,
				0, 0,
				gc->image.width, gc->image.height,
				true)) {

			// Exclude bottom right region
			source_buffer.width = width;
			source_buffer.height = height;

			vg_lite_matrix_t matrix;
			vg_lite_identity(&matrix);

			vg_lite_translate(x + (img->width/2), y + (img->height/2), &matrix);

			switch (transformation) {
			case DRAWING_FLIP_NONE:
				break;
			case DRAWING_FLIP_90:
				vg_lite_rotate(-90, &matrix);
				break;
			case DRAWING_FLIP_180:
				vg_lite_rotate(-180, &matrix);
				break;
			case DRAWING_FLIP_270:
				vg_lite_rotate(-270, &matrix);
				break;
			case DRAWING_FLIP_MIRROR:
				vg_lite_scale(-1, 1, &matrix);
				break;
			case DRAWING_FLIP_MIRROR_90:
				vg_lite_rotate(90, &matrix);
				vg_lite_scale(1, -1, &matrix);
				break;
			case DRAWING_FLIP_MIRROR_180:
				vg_lite_rotate(180, &matrix);
				vg_lite_scale(-1, 1, &matrix);
				break;
			case DRAWING_FLIP_MIRROR_270:
				vg_lite_rotate(270, &matrix);
				vg_lite_scale(1, -1, &matrix);
				break;
			default:
				DISPLAY_IMPL_error(false, "Unexpected transformation: %d\n", transformation);
				break;
			}

			vg_lite_translate(-img->width/2, -img->height/2, &matrix);
			void* target = VG_DRAWER_configure_target(gc);
			vg_lite_error_t vg_lite_error;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
			blit_rect[0] = 0;
			blit_rect[1] = 0;
			blit_rect[2] = img->width;
			blit_rect[3] = img->height;

			vg_lite_error = VG_DRAWER_blit_rect(
					target,
					&source_buffer,
					blit_rect,
					&matrix, VG_LITE_BLEND_SRC_OVER,
					__get_vglite_color(gc, img, alpha),
					VG_LITE_FILTER_BI_LINEAR);
#else
			vg_lite_error = VG_DRAWER_blit(
					target,
					&source_buffer,
					&matrix, VG_LITE_BLEND_SRC_OVER,
					__get_vglite_color(gc, img, alpha),
					VG_LITE_FILTER_BI_LINEAR);
#endif
			ret = VG_DRAWER_post_operation(target, vg_lite_error);
		}
		else {
			ret = DRAWING_DONE;
		}
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawRotatedImageNearestNeighbor(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint x,
		jint y,
		jint xRotation,
		jint yRotation,
		jfloat angle,
		jint alpha) {
	DRAWING_Status ret;

	if(!__configure_source(&source_buffer, img))  {
		DW_DRAWING_SOFT_drawRotatedImageNearestNeighbor(gc, img, x, y, xRotation, yRotation, angle, alpha);
		ret = DRAWING_DONE;
	}
	else {
		// The GCNanoLiteV limitation: Same rendering around 90 degrees (89.0, 89.1 ... 90.9, 91)
		// Render is acceptable since vglite 3.0.4-rev4
		ret = __rotate_image(gc, img, &source_buffer, x, y, xRotation, yRotation, angle, alpha, VG_LITE_FILTER_POINT);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawRotatedImageBilinear(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint x,
		jint y,
		jint xRotation,
		jint yRotation,
		jfloat angle,
		jint alpha) {
	DRAWING_Status ret;

	if(!__configure_source(&source_buffer, img)) {
		DW_DRAWING_SOFT_drawRotatedImageBilinear(gc, img, x, y, xRotation, yRotation, angle, alpha);
		ret = DRAWING_DONE;
	}
	else {
		// The GCNanoLiteV limitation: Same rendering around 90 degrees (89.0, 89.1 ... 90.9, 91)
		// Render is acceptable since vglite 3.0.4-rev4
		ret = __rotate_image(gc, img, &source_buffer, x, y, xRotation, yRotation, angle, alpha, VG_LITE_FILTER_BI_LINEAR);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawScaledImageNearestNeighbor(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint x,
		jint y,
		jfloat factorX,
		jfloat factorY,
		jint alpha) {
	DRAWING_Status ret;

	if(!__configure_source(&source_buffer, img)){
		DW_DRAWING_SOFT_drawScaledImageNearestNeighbor(gc, img, x, y, factorX, factorY, alpha);
		ret = DRAWING_DONE;
	}
	else {
		ret =__scale_image(gc, img, &source_buffer, x, y, factorX, factorY, alpha, VG_LITE_FILTER_POINT);
	}

	return ret;
}

// See the header file for the function documentation
DRAWING_Status DW_DRAWING_drawScaledImageBilinear(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		jint x,
		jint y,
		jfloat factorX,
		jfloat factorY,
		jint alpha) {
	DRAWING_Status ret;

	if(!__configure_source(&source_buffer, img)){
		DW_DRAWING_SOFT_drawScaledImageBilinear(gc, img, x, y, factorX, factorY, alpha);
		ret = DRAWING_DONE;
	}
	else {
		ret =__scale_image(gc, img, &source_buffer, x, y, factorX, factorY, alpha, VG_LITE_FILTER_BI_LINEAR);
	}

	return ret;
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static bool __check_clip(
		MICROUI_GraphicsContext* gc,
		int x1,
		int y1,
		int x2,
		int y2,
		bool update_drawing_limits) {
	bool ret;

	if (LLUI_DISPLAY_isClipEnabled(gc))	{

		if (LLUI_DISPLAY_clipRectangle(gc, &x1, &y1, &x2, &y2)) {
			// drawing fully or partially fits the clip:

			// enable scissor for next vglite drawing
			vg_lite_enable_scissor();
			vg_lite_set_scissor(gc->clip_x1, gc->clip_y1, gc->clip_x2 - gc->clip_x1 + 1, gc->clip_y2 - gc->clip_y1 + 1);

			if (update_drawing_limits) {
				// update flush limits (cropped to the clip)
				LLUI_DISPLAY_setDrawingLimits(x1, y1, x2, y2);
			}

			// perform drawing
			ret = true;
		}
		else {
			// drawing is fully outside the clip, nothing to draw
			ret = false;
		}
	}
	else {
		// clip is disabled :

		// disable scissor (vglite lib already crops to the buffer bounds)
		vg_lite_disable_scissor();

		if (update_drawing_limits) {
			// update flush limits
			LLUI_DISPLAY_setDrawingLimits(x1, y1, x2, y2);
		}

		// perform drawing
		ret = true;
	}

	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __draw_ellipse_arc(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_w,
		int diameter_h,
		float start_angle_deg,
		float arc_angle_deg,
		bool fill) {

	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	DRAWING_Status ret;

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(gc, x, y, x + diameter_w, y + diameter_h, false)) {

		vg_lite_identity(&matrix);

		int radius_out_w = diameter_w / 2;
		int radius_out_h = diameter_h / 2;
		int radius_in_w = radius_out_w - 1;
		int radius_in_h = radius_out_h - 1;

		vg_lite_translate(x + radius_out_w, y + radius_out_h, &matrix);

		radius_out_w *= DRAWING_SCALE_FACTOR;
		radius_out_h *= DRAWING_SCALE_FACTOR;
		radius_in_w *= DRAWING_SCALE_FACTOR;
		radius_in_h *= DRAWING_SCALE_FACTOR;

		vg_lite_scale(DRAWING_SCALE_DIV, DRAWING_SCALE_DIV, &matrix);

		shape_vg_path.path = &__shape_paths.ellipse_arc;
		shape_vg_path.path_length = sizeof(__shape_paths.ellipse_arc);
		int path_offset = VGLITE_PATH_compute_ellipse_arc(
				&shape_vg_path,
				radius_out_w, radius_out_h,
				radius_in_w, radius_in_h,

				// 0 degrees:
				// - MicroUI: 3 o'clock.
				// - RT595 VGLite implementation : 0 o'clock.
				90.f - start_angle_deg,

				-arc_angle_deg, fill);

		if (0 > path_offset) {
			DISPLAY_IMPL_error(false, "Error during path computation");
			ret = DRAWING_DONE;
		}
		else {

			// Draw the point with the GPU
			void* target = VG_DRAWER_configure_target(gc);
			vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
					target,
					&shape_vg_path,
					VG_LITE_FILL_EVEN_ODD,
					&matrix,
					VG_LITE_BLEND_SRC_OVER,
					gc->foreground_color
			);
			ret = VG_DRAWER_post_operation(target, vg_lite_error);
		}
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __draw_ellipse(
		MICROUI_GraphicsContext* gc,
		int x, int y,
		int diameter_out_w, int diameter_out_h,
		int diameter_in_w, int diameter_in_h) {

	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	DRAWING_Status ret;

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(gc, x, y, x + diameter_out_w, y + diameter_out_h, false)) {

		vg_lite_identity(&matrix);

		int radius_w = diameter_out_w / 2;
		int radius_h = diameter_out_h / 2;

		vg_lite_translate(x + radius_w, y + radius_h, &matrix);

		radius_w *= DRAWING_SCALE_FACTOR;
		radius_h *= DRAWING_SCALE_FACTOR;

		vg_lite_scale(DRAWING_SCALE_DIV, DRAWING_SCALE_DIV, &matrix);
		// Compute the rounded rectangle shape path
		shape_vg_path.path = &__shape_paths.ellipse[0];
		shape_vg_path.path_length = sizeof(__shape_paths.ellipse);
		int path_offset = VGLITE_PATH_compute_ellipse(
				&shape_vg_path, 0,
				radius_w, radius_h,
				false);

		radius_w = diameter_in_w / 2;
		radius_h = diameter_in_h / 2;

		radius_w *= DRAWING_SCALE_FACTOR;
		radius_h *= DRAWING_SCALE_FACTOR;

		path_offset = VGLITE_PATH_compute_ellipse(
				&shape_vg_path,
				path_offset,
				radius_w, radius_h,
				true);

		if (0 > path_offset) {
			DISPLAY_IMPL_error(false, "Error during path computation");
			ret = DRAWING_DONE;
		}
		else {
			// Draw the point with the GPU
			void* target = VG_DRAWER_configure_target(gc);
			vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
					target,
					&shape_vg_path,
					VG_LITE_FILL_EVEN_ODD,
					&matrix,
					VG_LITE_BLEND_SRC_OVER,
					gc->foreground_color
			);
			ret = VG_DRAWER_post_operation(target, vg_lite_error);
		}
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __fill_ellipse(
		MICROUI_GraphicsContext* gc,
		int x, int y,
		int diameter_w, int diameter_h) {

	vg_lite_matrix_t matrix;
	vg_lite_path_t shape_vg_path;
	DRAWING_Status ret;

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(	gc,
			x,
			y,
			x + diameter_w - 1,
			y + diameter_h - 1, 
			false)) {

		int radius_w = diameter_w / 2;
		int radius_h = diameter_h / 2;

		vg_lite_identity(&matrix);
		vg_lite_translate(x + radius_w, y + radius_h, &matrix);

		// Compute the thick shape path
		shape_vg_path.path = &__shape_paths.ellipse;
		shape_vg_path.path_length = sizeof(__shape_paths.ellipse);
		(void)VGLITE_PATH_compute_filled_ellipse(&shape_vg_path, radius_w, radius_h, &matrix);

		// Draw the point with the GPU
		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
				target,
				&shape_vg_path,
				VG_LITE_FILL_NON_ZERO,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				gc->foreground_color
		);
		ret = VG_DRAWER_post_operation(target, vg_lite_error);
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

#ifdef VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS
// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __draw_line(
		MICROUI_GraphicsContext* gc,
		int x1, int y1,
		int x2, int y2) {

	vg_lite_path_t shape_vg_path;

	vg_lite_matrix_t matrix;
	vg_lite_identity(&matrix);

	// adjust parameters according to steering coefficient
	int xi;
	int yi;
	int xs;
	int ys;
	int xe;
	int ye;
	if (x1 > x2){
		if (y1 > y2){
			xs = x2;
			ys = y2;
			xe = x1;
			ye = y1;
			xi = 1;
			yi = 1;
		}
		else {
			xs = x1;
			ys = y1;
			xe = x2;
			ye = y2;
			xi = -1;
			yi = 1;
		}
	}
	else {
		if (y1 > y2){
			xs = x1;
			ys = y1;
			xe = x2;
			ye = y2;
			xi = 1;
			yi = -1;
		}
		else {
			xs = x1;
			ys = y1;
			xe = x2;
			ye = y2;
			xi = 1;
			yi = 1;
		}
	}

	vg_lite_translate(xs, ys, &matrix);

	// Compute the thick shape path
	shape_vg_path.path = &__shape_paths.line;
	shape_vg_path.path_length = sizeof(__shape_paths.line);
	(void)VGLITE_PATH_compute_line(&shape_vg_path, xe - xs, ye - ys, xi, yi);

	// Draw the point with the GPU
	void* target = VG_DRAWER_configure_target(gc);
	vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
			target,
			&shape_vg_path,
			VG_LITE_FILL_NON_ZERO,
			&matrix,
			VG_LITE_BLEND_SRC_OVER,
			gc->foreground_color
	);
	return VG_DRAWER_post_operation(target, vg_lite_error);
}
#endif // VGLITE_USE_GPU_FOR_SIMPLE_DRAWINGS

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __thick_line(
		MICROUI_GraphicsContext* gc,
		int x1,
		int y1,
		int x2,
		int y2,
		int thickness,
		DRAWING_Cap start,
		DRAWING_Cap end) {

	vg_lite_path_t shape_vg_path;
	vg_lite_matrix_t matrix;
	DRAWING_Status ret;

	int min_x = MIN(x1, x2);
	int min_y = MIN(y1, y2);
	int max_x = MAX(x1, x2);
	int max_y = MAX(y1, y2);

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(
			gc,
			min_x - (thickness / 2),
			min_y - (thickness / 2),
			max_x + (thickness / 2),
			max_y + (thickness / 2),
			false)) {

		vg_lite_identity(&matrix);
		vg_lite_translate(x1, y1, &matrix);

		shape_vg_path.path = &__shape_paths.thick_shape_line;
		shape_vg_path.path_length = sizeof(__shape_paths.thick_shape_line);

		int caps = 0;
		caps |= MEJ_VGLITE_PATH_SET_CAPS_START(start);
		caps |= MEJ_VGLITE_PATH_SET_CAPS_END(end);

		(void)VGLITE_PATH_compute_thick_shape_line(
				&shape_vg_path, x2 - x1, y2 - y1, thickness, caps, &matrix);

		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
				target,
				&shape_vg_path,
				VG_LITE_FILL_NON_ZERO,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				gc->foreground_color);
		ret = VG_DRAWER_post_operation(target, vg_lite_error);
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __draw_thick_shape_ellipse_arc(
		MICROUI_GraphicsContext* gc,
		int x,
		int y,
		int diameter_w,
		int diameter_h,
		float start_angle_deg,
		float arc_angle,
		int thickness,
		DRAWING_Cap start,
		DRAWING_Cap end) {

	vg_lite_path_t shape_vg_path;
	DRAWING_Status ret;

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(
			gc,
			x - thickness,
			y - thickness,
			x + diameter_w + (2 * thickness) - 1,
			y + diameter_h + (2 * thickness) - 1,
			false)) {

		// Tunning for CircleArcs rendering like software algorithm
		int m_diameter_w = diameter_w + 1;
		int m_diameter_h = diameter_h + 1;
		int m_thickness = thickness + 1;
		int translate_x = x + (m_diameter_w / 2);
		int translate_y = y + (m_diameter_h / 2);

		vg_lite_matrix_t matrix;
		vg_lite_identity(&matrix);
		vg_lite_translate(translate_x, translate_y, &matrix);
		vg_lite_rotate(-start_angle_deg, &matrix);
		vg_lite_scale(DRAWING_SCALE_DIV, DRAWING_SCALE_DIV, &matrix);

		int caps = 0;
		caps |= MEJ_VGLITE_PATH_SET_CAPS_START(start);
		caps |= MEJ_VGLITE_PATH_SET_CAPS_END(end);

		// Compute the thick shape path
		shape_vg_path.path = &__shape_paths.thick_ellipse_arc;
		shape_vg_path.path_length = sizeof(__shape_paths.thick_ellipse_arc);

		(void)VGLITE_PATH_compute_thick_shape_ellipse_arc(
				&shape_vg_path,
				DRAWING_SCALE_FACTOR * m_diameter_w,
				DRAWING_SCALE_FACTOR * m_diameter_h,
				DRAWING_SCALE_FACTOR * m_thickness,
				0,
				arc_angle,
				caps);

		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_error_t vg_lite_error = VG_DRAWER_draw_path(
				target,
				&shape_vg_path,
				VG_LITE_FILL_NON_ZERO,
				&matrix,
				VG_LITE_BLEND_SRC_OVER,
				gc->foreground_color);
		ret = VG_DRAWER_post_operation(target, vg_lite_error);
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __rotate_image(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		vg_lite_buffer_t* src,
		int x,
		int y,
		int xRotation,
		int yRotation,
		float angle_deg,
		int alpha,
		vg_lite_filter_t filter) {

	DRAWING_Status ret;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
	uint32_t blit_rect[4];
#endif

	// Check if there is something to draw and clip drawing limits
	// TODO try to crop region
	if (__check_clip(
			gc,
			0, 0,
			gc->image.width, gc->image.height,
			true)) {

		vg_lite_matrix_t matrix;
		vg_lite_identity(&matrix);
		vg_lite_translate(xRotation, yRotation, &matrix);
		vg_lite_rotate(-angle_deg, &matrix);
		vg_lite_translate(x-xRotation, y-yRotation, &matrix);

		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_error_t vg_lite_error;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
		blit_rect[0] = 0;
		blit_rect[1] = 0;
		blit_rect[2] = img->width;
		blit_rect[3] = img->height;

		vg_lite_error = VG_DRAWER_blit_rect(
				target,
				src,
				blit_rect,
				&matrix, VG_LITE_BLEND_SRC_OVER,
				__get_vglite_color(gc, img, alpha),
				filter);
#else
		vg_lite_error = VG_DRAWER_blit(
				target,
				src,
				&matrix, VG_LITE_BLEND_SRC_OVER,
				__get_vglite_color(gc, img, alpha),
				filter);
#endif
		ret = VG_DRAWER_post_operation(target, vg_lite_error);
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __scale_image(
		MICROUI_GraphicsContext* gc,
		MICROUI_Image* img,
		vg_lite_buffer_t* src,
		int x,
		int y,
		float factorX,
		float factorY,
		int alpha,
		vg_lite_filter_t filter) {

	DRAWING_Status ret;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
	uint32_t blit_rect[4];
#endif

	// Check if there is something to draw and clip drawing limits
	if (__check_clip(
			gc, x, y,
			(int) (x + (factorX * img->width)),
			(int) (y + (factorY * img->height)),
			true)) {

		vg_lite_matrix_t matrix;
		vg_lite_identity(&matrix);
		vg_lite_translate(x, y, &matrix);
		vg_lite_scale(factorX, factorY, &matrix);

		void* target = VG_DRAWER_configure_target(gc);
		vg_lite_error_t vg_lite_error ;

#if defined (VG_BLIT_WORKAROUND) && (VG_BLIT_WORKAROUND == 1)
		blit_rect[0] = 0;
		blit_rect[1] = 0;
		blit_rect[2] = img->width;
		blit_rect[3] = img->height;

		vg_lite_error = VG_DRAWER_blit_rect(
				target,
				src,
				blit_rect,
				&matrix, VG_LITE_BLEND_SRC_OVER,
				__get_vglite_color(gc, img, alpha),
				filter);
#else
		vg_lite_error = VG_DRAWER_blit(
				target,
				src,
				&matrix, VG_LITE_BLEND_SRC_OVER,
				__get_vglite_color(gc, img, alpha),
				filter);
#endif
		ret = VG_DRAWER_post_operation(target, vg_lite_error);
	}
	else {
		ret = DRAWING_DONE;
	}
	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_color_t __get_vglite_color(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint alpha) {
	jint fc;

	switch(img->format) {
	case MICROUI_IMAGE_FORMAT_A4:
	case MICROUI_IMAGE_FORMAT_A8:
		if (0xff == alpha) {
			fc = gc->foreground_color | 0xff000000u;
		}
		else {
			fc = gc->foreground_color;
			jint red = alpha * ((fc >> 16) & 0xff) / 0xff;
			jint green = alpha * ((fc >> 8) & 0xff) / 0xff;
			jint blue = alpha * (fc & 0xff) / 0xff;
			fc = (alpha << 24) | (red << 16) | (green << 8) | blue;
		}
		break;
	default:
		fc = alpha * 0x01010101;
		break;
	}

	return (vg_lite_color_t)fc;
}

// See the section 'Internal function definitions' for the function documentation
static bool __configure_source(vg_lite_buffer_t *buffer, MICROUI_Image* img) {
	bool ret;

#ifdef VGLITE_OPTION_TOGGLE_GPU
	if (!DISPLAY_VGLITE_is_hardware_rendering_enabled()) {
		// GPU disabled
		ret = false;
	}
	else {
#endif // VGLITE_OPTION_TOGGLE_GPU

#ifndef VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES
		if (LLUI_DISPLAY_isTransparent(img) && (MICROUI_IMAGE_FORMAT_A8 != img->format) && (MICROUI_IMAGE_FORMAT_A4 != img->format)) {
			// No MSAA with hardware
			ret = false;
		}
		else {
#endif // VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES

			if (!DISPLAY_VGLITE_configure_source(buffer, img)) {
				// image format not supported
				ret = false;
			}
			else {
				ret = true;
			}

#ifndef VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES
		}
#endif // VGLITE_USE_GPU_FOR_TRANSPARENT_IMAGES

#ifdef VGLITE_OPTION_TOGGLE_GPU
	}
#endif // VGLITE_OPTION_TOGGLE_GPU

	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static DRAWING_Status __draw_region_with_overlap(void* target, vg_lite_color_t color, vg_lite_matrix_t* matrix, uint32_t* rect, uint32_t element_index) {

	DRAWING_Status ret = DRAWING_RUNNING;

	// rect[x,y,w,h]
	uint32_t rect_index = element_index + (uint32_t)2;

	// retrieve band's size (width or height)
	jint size = rect[rect_index];
	rect[rect_index] = (uint32_t)(matrix->m[element_index][2] - rect[element_index]);

	// go to x + band size
	rect[element_index] += size;
	matrix->m[element_index][2] = rect[element_index] + rect[rect_index];

	while (size > 0) {

		// adjust band's size
		if (size < rect[rect_index]){
			rect[rect_index] = size;
		}

		// adjust src & dest positions
		rect[element_index] -= rect[rect_index];
		matrix->m[element_index][2] -= rect[rect_index];

		size -= rect[rect_index];

		if (VG_LITE_SUCCESS != VG_DRAWER_blit_rect(
				target,
				&source_buffer,
				rect,
				matrix,
				VG_LITE_BLEND_SRC_OVER,
				color,
				VG_LITE_FILTER_POINT)){
			ret = DRAWING_DONE;
			size = 0; // stop the loop
			DISPLAY_IMPL_error(false, "Error during draw image with overlap");
		}
		else {
			// wakeup task only for the last iteration, otherwise waits the end of the drawing before continue
			DISPLAY_VGLITE_start_operation(size <= 0);
		}
	}

	return ret;
}

// See the section 'Internal function definitions' for the function documentation
static bool __prepare_gpu_draw_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha, void** target, vg_lite_color_t* color, vg_lite_matrix_t* matrix, uint32_t* blit_rect){

	bool gpu_compatible;

	if(__configure_source(&source_buffer, img)) {

		*target = VG_DRAWER_configure_target(gc);
		*color = __get_vglite_color(gc, img, alpha);

		vg_lite_identity(matrix);
		matrix->m[0][2] = x_dest;
		matrix->m[1][2] = y_dest;

		blit_rect[0] = x_src;
		blit_rect[1] = y_src;
		blit_rect[2] = width;
		blit_rect[3] = height;

		LLUI_DISPLAY_setDrawingLimits(x_dest, y_dest, x_dest + width - 1, y_dest + height - 1);
		gpu_compatible = true;
	}
	else{
		gpu_compatible = false;
	}
	return gpu_compatible;
}

// See the section 'Internal function definitions' for the function documentation
static inline void __soft_draw_image(MICROUI_GraphicsContext* gc, MICROUI_Image* img, jint x_src, jint y_src, jint width, jint height, jint x_dest, jint y_dest, jint alpha){

#ifdef VGLITE_USE_MULTIPLE_DRAWERS
	if (!LLUI_DISPLAY_isCustomFormat(gc->image.format)) {
		UI_DRAWING_SOFT_drawImage(gc, img, x_src, y_src, width, height, x_dest, y_dest, alpha);
	}
	// else: unsupported functionality, the drawing is abandoned!
#else // VGLITE_USE_MULTIPLE_DRAWERS
	UI_DRAWING_SOFT_drawImage(gc, img, x_src, y_src, width, height, x_dest, y_dest, alpha);
#endif // VGLITE_USE_MULTIPLE_DRAWERS
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
