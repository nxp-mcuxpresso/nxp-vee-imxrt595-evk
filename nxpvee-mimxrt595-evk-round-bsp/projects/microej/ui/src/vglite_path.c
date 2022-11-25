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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <math.h>

#include "ui_drawing.h"
#include "dw_drawing.h"

#include "display_impl.h"
#include "display_vglite.h"
#include "vglite_path.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * Precomputation of tangente for rounded caps
 */
#define CAP_TAN     		1.3333333333333333
#define QUARTER_TAN   		0.5522847498307933

#define COS_0       		1.0
#define COS_90      		0.0
#define COS_180     		-1.0
#define COS_270     		0.0

#define SIN_0       		0.0
#define SIN_90      		1.0
#define SIN_180     		0.0
#define SIN_270     		-1.0

/*
 * Constant used to compute the ellipse path
 */

#define CIRCLE_RADIUS		200		// radius
#define CIRCLE_CP			111		// control point

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief enumeration that identifies the direction of a quarter curve
 */
typedef enum {
	__horizontal = 0,	// The quarter curve starts horizontaly
	__vertical,			// The quarter curve starts verticaly
} __axis_t;

/*
 * @brief data to draw a quarter curve
 */
typedef struct {
	int x;
	int y;
} __quarter_curve_data_t;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/*
 * @brief drawing context
 */
static struct {
	uint8_t *path;			// Current path buffer
	int offset;				// Offset in the path buffer
	int length;				// Length of the path buffer
	int x;					// Current horizontal coordinate
	int y;					// Current vertical coordinate
	float32_t sin;				// Last computed sinus
	float32_t cos;				// Last computed cosinus
	float32_t tangent;			// Last computed tangent
	int center_x;			// horizontal coordinate of the current ellipse center
	int center_y;			// vertical coordinate of the current ellipse center
	float32_t control_x;		// horizontal coordinate of the last control point
	float32_t control_y;		// vertical coordinate of the last control point
	__axis_t tangent_axis;	// Last tangent (if last command was a quarter curve)
} __ctxt;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

/*
 * Circle path
 * This path can be used to draw a circle of anysize anywhere with the proper
 * transformation matrix
 */
static const int16_t __circle_s16[] = {
		VGLITE_MOVE_CMD, // move to (r, 0)
		CIRCLE_RADIUS, 0,

		VGLITE_CUBIC_CMD, // cubic to (r, -4/3 * tan(pi/8), 4/3 * tan(pi/8), -r, 0, -r)
		CIRCLE_RADIUS, -CIRCLE_CP, CIRCLE_CP, -CIRCLE_RADIUS, 0, -CIRCLE_RADIUS,

		VGLITE_CUBIC_CMD, // cubic to (-4/3 * tan(pi/8), -r, -r, -4/3 * tan(pi/8), -r, 0)
		-CIRCLE_CP, -CIRCLE_RADIUS, -CIRCLE_RADIUS, -CIRCLE_CP, -CIRCLE_RADIUS, 0,

		VGLITE_CUBIC_CMD, // cubic to (-r, 4/3 * tan(pi/8), -4/3 * tan(pi/8), r, 0, r)
		-CIRCLE_RADIUS, CIRCLE_CP, -CIRCLE_CP, CIRCLE_RADIUS, 0, CIRCLE_RADIUS,

		VGLITE_CUBIC_CMD, // cubic to (4/3 * tan(pi/8), r, r, 4/3 * tan(pi/8), r, 0)
		CIRCLE_CP, CIRCLE_RADIUS, CIRCLE_RADIUS, CIRCLE_CP, CIRCLE_RADIUS, 0,

		VGLITE_END_CMD, // end
};

/*
 * @brief data to draw all the quarter curves
 */
static __quarter_curve_data_t __quarter_curve_data[4] = {
		{ SIN_0, COS_0, },
		{ SIN_90, COS_90, },
		{ SIN_180, COS_180, },
		{ SIN_270, COS_270, },
};

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Sets the center for ellipse drawing
 *
 * @param[out]: x: the horizontal coordinate of the point to compute
 * @param[out]: y: the vertical coordinate of the point to compute
 */
static inline void __set_center(int x, int y) {
	__ctxt.center_x = x;
	__ctxt.center_y = y;
}

/*
 * @brief Computes the control point to approximate an ellipse arc
 *
 * @param[out]: x: the horizontal coordinate of the point to compute
 * @param[out]: y: the vertical coordinate of the point to compute
 * @param[in]: point_angle: the angle of the point to control in radians
 * @param[in]: section_angle: the section angle in radians
 */
static inline void __compute_control_point(
		float32_t *x,
		float32_t *y,
		float32_t point_angle,
		float32_t section_angle) {

	__ctxt.tangent = 4 * mej_tan_f32(section_angle/4) / 3;

	__ctxt.control_x = __ctxt.tangent * arm_cos_f32(point_angle);
	__ctxt.control_y = __ctxt.tangent * arm_sin_f32(point_angle);
	*x = __ctxt.control_x;
	*y = __ctxt.control_y;
}

/*
 * @brief Initializes the drawing context
 *
 * @param[in] path: vg_lite path to use for drawing
 * @param[in] offset: offset in the vg_lite path
 */
static void __init_drawing(vg_lite_path_t *path, int offset);

/*
 * @brief Computes a "move_to" VGLite command
 *
 * @param[in] x: x coordinate of the destination
 * @param[in] y: y coordinate of the destination
 *
 * @return: -1 if there is not enough memory in the buffer, otherwise the new path_offset
 */
static int __move_to(int16_t x, int16_t y);

/*
 * @brief Computes a "line_to" VGLite command
 *
 * @param[in] x: horizontal coordinate of the destination
 * @param[in] y: vertical coordinate of the destination
 *
 * @return: -1 if there is not enough memory in the buffer, otherwise the new path_offset
 */
static int __line_to(int16_t x, int16_t y);

/*
 * @brief Computes a "cubic_to" VGLite command
 *
 * @param[in] cy1: y coordinate of the first control point
 * @param[in] cx2: x coordinate of the second control point
 * @param[in] cy2: y coordinate of the second control point
 * @param[in] x: x coordinate of the destination
 * @param[in] y: y coordinate of the destination
 *
 * @return: -1 if there is not enough memory in the buffer, otherwise the new path_offset
 */
static int __cubic_to(
		int16_t cx1,
		int16_t cy1,
		int16_t cx2,
		int16_t cy2,
		int16_t x,
		int16_t y);

/*
 * @brief Computes an "end" VGLite command
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __end(void);

/*
 * @brief Updates a path structure
 *
 * @param[in,out] path: pointer to the path to update.
 * @param[in] first_path:
 * - if true: the bounding box is set, the offset is set to zero.
 * - otherwise: the bounding box is increased.
 * @param[in] last_path:
 * - if true: the path is closed calling __end
 *            and the path_length will be set to the current offset.
 * @param[in] left: left most coordinate of the shape.
 * @param[in] top: top most coordinate of the shape.
 * @param[in] right: right most coordinate of the shape.
 * @param[in] bottom: bottom most coordinate of the shape.
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __update_path(vg_lite_path_t *path,
		bool first_path,
		bool last_path,
		int left,
		int top,
		int right,
		int bottom);

/*
 * @brief Computes an approximation of an ellipse arc
 * This computation uses multiple calls to __approximate_ellipse_arc_fragment_to
 * The center point stored in the context must be set before calling this function.
 *
 * @param[in] radius_w: the width radius of the ellipse, sign indicates the direction:
 * - if negative, drawing to the left of current point.
 * - if positive, drawing to the righ of the current point.
 * @param[in] radius_h: the height radius of the ellipse, sign indicates the direction:
 * - if negative, drawing to the top of the current point.
 * - if positive, drawing to the bottom of the current point.
 * @param[in] start_angle: the angle of the start point in radians.
 * @param[in] end_angle: the angle of the end point in radians.
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __approximate_ellipse_arc_to(
		int radius_w,
		int radius_h,
		float32_t start_angle,
		float32_t end_angle);

/*
 * @brief Computes an approximation of an ellipse arc fragment
 * This computation uses one VGLite cubic_to command * (bezier curves of 3rd order)
 * This algorithm is implemented from the following source:
 * https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%c3%a9zier-curves
 * The approximation is acceptable if the angle is less than PI/2.
 * For ellipse arcs with angles bigger than PI/2, multiple fragments should be computed
 * A is the starting point
 * B is the ending point
 * C and D are the control points
 * The computed arc will start from start_angle until end_angle
 * The center point stored in the context must be set before calling this function.
 *
 * @param[in] radius_w: the width radius of the ellipse, sign indicates the direction:
 * - if negative, drawing to the left of current point.
 * - if positive, drawing to the righ of the current point.
 * @param[in] radius_h: the height radius of the ellipse, sign indicates the direction:
 * - if negative, drawing to the top of the current point.
 * - if positive, drawing to the bottom of the current point.
 * @param[in] start_angle: the angle of the start point in radians.
 * @param[in] end_angle: the angle of the end point in radians.
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __approximate_ellipse_arc_fragment_to(
		int radius_w,
		int radius_h,
		float32_t start_angle,
		float32_t end_angle);

/*
 * @brief Computes an approximation of a ellipse arc fragment
 * This computation uses one VGLite cubic_to command * (bezier curves of 3rd order)
 * This algorithm is implemented from the following source:
 * https://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%c3%a9zier-curves
 * The approximation is acceptable if the angle is less than 90 degrees.
 * For ellipse arcs with angles bigger than 90 degrees, multiple fragments should be computed
 * A is the starting point
 * B is the ending point
 * C and D are the control points
 *
 * @param[in] center_x: the horizontal coordinate of the center of the ellipse
 * @param[in] center_y: the vetical coordinate of the center of the ellipse
 * @param[in] radius_w: the horizontal radius of the ellipse
 * @param[in] radius_h: the vertical radius of the ellipse
 * @param[in] start_angle_rad: the angle of the beginning of the ellipse in radians
 * @param[in] end_angle_rad: the angle of the end of the ellipse in radians
 * @param[in] tangent: the tangent of 1/4 th of the angle (end_angle_rad - start_angle_rad) as described
 * 		by the algorithm (see source mentionned previously), this must be computed by the caller
 * 		as the computation is heavy and can be reused.
 * @param[in] revert:
 * 		- 0: angle is drawn from start_angle_rad to end_angle_rad
 * 		- 1: angle is drawn from end_angle_rad to start_angle_rad
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __approximate_ellipse_arc_fragment(
		uint16_t center_x,
		uint16_t center_y,
		int radius_w,
		int radius_h,
		float32_t start_angle_rad,
		float32_t end_angle_rad,
		float32_t tangent,
		int revert);

/*
 * @brief Sets the tangent for the quarter curve algorithm.
 *
 * @param tangent: the orientation (tangent) of the beginning of the curve
 */
static inline void __set_quarter_curve_tangent_axis(__axis_t tangent_axis)
{
	__ctxt.tangent_axis = tangent_axis;
}

/*
 * @brief Computes a path fragment for a quarter curve from current context position to
 * the given destination coordinates
 *
 * @param x: the horizontal coordinate of the destination point
 * @param y: the vertical coordinate of the destination point
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __quarter_curve_to(int x, int y);

/*
 * @brief Computes a path fragment for multiple quarter curve from current context position.
 * This function approximates ellipse quarters with bezier curves.
 *
 * @param radius_w: the width radius of the curves, the sign indicates the direction to draw:
 * - negative, draw to the left of the current point.
 * - positive, draw to the right of the current point.
 * @param radius_h: the height radius of the curves, the sign indeicates the direction to draw:
 * - negative, draw to the top of the current point.
 * - positive, draw to the bottom of the current point.
 * @param start: the first quarter to draw (between 1 to 4).
 * - 0: from 0 to 90 deg
 * - 1: from 90 to 180 deg
 * - 2: from 180 to 270 deg
 * - 3: from 270 to 360 deg
 * @param number: the number of quarters to draw
 * - positive: clockwise
 * - negative: counter clockwise
 *
 * @return:
 * - 1: not enough memory in the path buffer.
 * - otherwise: the updated path offset
 */
static int __multiple_quarter_curve_to(
		int radius_w,
		int radius_h,
		int start,
		int number);

/*
 * @brief Computes an approximation of an ellipse arc
 *
 * This computation is based on the function __approximate_ellipse_arc_fragment
 *
 * The computed path is always centered to (0, 0), the caller can use matrix
 * to position the ellipse arc
 *
 * @param[in]: radius_out_w: the horizontal radius of the outer edge of the ellipse arc
 * @param[in]: radius_out_h: the vertical radius of the outer edge of the ellipse arc
 * @param[in]: radius_in_w: the horizontal radius of the inner edge of the ellipse arc
 * @param[in]: radius_in_h: the vertical radius of the inner edge of the ellipse arc
 * @param[in]: start_angle_rad: the angle of the beginning of the ellipse in degrees
 * @param[in]: end_angle_rad: the angle of the end of the ellipse in degrees
 * @param[in]: caps: the cap representation of the start and the end of the ellipse arc
 * 		This is a bit field that can be manipulated with macros MEJ_VGLITE_PATH_CAPS_XXX,
 * 		MEJ_VGLITE_PATH_SET_CAPS_XXX and MEJ_VGLITE_PATH_GET_CAPS_XXX,
 * 		bits[0-1] represent the cap of the start of the shape,
 * 		bits[2-3] represent the cap of the end of the shape,
 * 		See DRAWING_ENDOFLINE_XXX for caps values
 *
 * @return: -1 if there is not enough memory in the buffer, otherwise the new path_offset
 */
static int __approximate_ellipse_arc(
		int radius_out_w,
		int radius_out_h,
		int radius_in_w,
		int radius_in_h,
		float32_t start_angle_rad,
		float32_t arc_angle_rad,
		int caps);

/*
 * @brief Normalize an angle so that it is in the range [0, 360[
 *
 * @param[in] angle_deg: the angle to normalize in degrees
 *
 * @return: the normalized angle
 */
static float32_t __normalize_angle(float32_t angle_deg);

/*
 * @brief Indirection to draw a path using VG-Lite library.
 *
 * @see vg_lite_draw
 *
 * @return: the VG-Lite error code
 */
static vg_lite_error_t __drawer_vglite_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color) ;

/*
 * @brief Indirection to draw a path with gradient using VG-Lite library.
 *
 * @see vg_lite_draw_gradient
 *
 * @return: the VG-Lite error code
 */
static vg_lite_error_t __drawer_vglite_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend) ;

/*
 * @brief Indirection to blit a part of the source using VG-Lite library.
 *
 * @see vg_lite_blit_rect
 *
 * @return: the VG-Lite error code
 */
static vg_lite_error_t __drawer_vglite_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) ;
/*
 * @brief Indirection to blit the source using VG-Lite library.
 *
 * @see vg_lite_blit
 *
 * @return: the VG-Lite error code
 */
static vg_lite_error_t __drawer_vglite_blit(
		void* target,
		vg_lite_buffer_t *source,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) ;
/*
 *
 * @brief Indirection to clear a rectangle using VG-Lite library.
 *
 * @see vg_lite_clear
 *
 * @return: the VG-Lite error code
 */
static vg_lite_error_t __drawer_vglite_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color) ;

// -----------------------------------------------------------------------------
// Fields
// -----------------------------------------------------------------------------

/*
 * @brief Drawer that makes an indirection to the VG-Lite library to draw paths.
 */
static VG_DRAWER_drawer_t vglite_drawer = {

		__drawer_vglite_draw_path,
		__drawer_vglite_draw_gradient,
		__drawer_vglite_blit_rect,
		__drawer_vglite_blit,
		__drawer_vglite_clear,

		VGLITE_PATH_update_color,
		VGLITE_PATH_update_gradient,

		VGLITE_PATH_post_operation,
};

// -----------------------------------------------------------------------------
// vglite_path.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
int VGLITE_PATH_compute_line(vg_lite_path_t *line_shape, int x, int y, int xi, int yi) {
	// Initialize the drawing context
	__init_drawing(line_shape, 0);

	// Move to beginning
	(void)__move_to(0, 0);
	(void)__move_to(xi, 0);

	// Compute edges of the line
	(void)__line_to(x + xi, y);
	(void)__line_to(x + xi, y + yi);
	(void)__line_to(x, y + yi);
	(void)__line_to(0, yi);
	(void)__line_to(0, 0);

	return __update_path(line_shape, true, true, 0, 0, x + xi, y + yi);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_rectangle(
		vg_lite_path_t *rectangle_shape,
		int path_offset,
		int x, int y,
		int width, int height,
		bool end_path) {
	bool first_path = (path_offset == 0);

	// Precompute all necessary coordinates
	int left = x;
	int right = left + width;
	int top = y;
	int bottom = top + height;

	// Initialize the drawing context
	__init_drawing(rectangle_shape, path_offset);

	// Move to beginning (beginning of top side)
	(void)__move_to(left, top);

	// Compute top side
	(void)__line_to(right, top);

	// Compute right side
	(void)__line_to(right, bottom);

	// Compute bottom side
	(void)__line_to(left, bottom);

	// Compute left side
	(void)__line_to(left, top);

	return __update_path(rectangle_shape, first_path, end_path,
			left, top, right, bottom);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_rounded_rectangle(
		vg_lite_path_t *rounded_rectangle_shape,
		int path_offset,
		int x, int y,
		int width, int height,
		int arc_width, int arc_height,
		bool end_path) {
	bool first_path = (path_offset == 0);

	// Convert diameter to radius
	int radius_width = arc_width / 2;
	int radius_height = arc_height / 2;

	// Precompute all necessary coordinates
	int left = x;
	int right = left + width;
	int top = y;
	int bottom = top + height;
	int left_radius = left + radius_width;
	int right_radius = right - radius_width;
	int top_radius = top + radius_height;
	int bottom_radius = bottom - radius_height;

	// Initialize the drawing context
	__init_drawing(rounded_rectangle_shape, path_offset);

	// Move to beginning (beginning of top side)
	(void)__move_to(left_radius, top);

	// Compute top side
	(void)__line_to(right_radius, top);

	// Compute top right corner
	__set_quarter_curve_tangent_axis(__horizontal);
	(void)__quarter_curve_to(right, top_radius);

	// Compute right side
	(void)__line_to(right, bottom_radius);

	// Compute bottom right corner
	(void)__quarter_curve_to(right_radius, bottom);

	// Compute bottom side
	(void)__line_to(left_radius, bottom);

	// Compute bottom left corner
	(void)__quarter_curve_to(left, bottom_radius);

	// Compute left side
	(void)__line_to(left, top_radius);

	// Compute top left corner
	(void)__quarter_curve_to(left_radius, top);

	return __update_path(rounded_rectangle_shape, first_path, end_path,
			left, top, right, bottom);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_ellipse_arc(
		vg_lite_path_t *ellipse_arc,
		int radius_out_w,
		int radius_out_h,
		int radius_in_w,
		int radius_in_h,
		float32_t start_angle_degree,
		float32_t arc_angle_degree,
		bool fill) {
	float32_t start_angle;
	float32_t end_angle;
	float32_t arc_angle;

	arc_angle = MEJ_DEG2RAD(arc_angle_degree);

	start_angle = MEJ_DEG2RAD(start_angle_degree);
	end_angle = start_angle + arc_angle;

	// Initialize the drawing context
	__init_drawing(ellipse_arc, 0);

	// Move to start
	__set_center(0, 0);
	__ctxt.x = (int) (radius_out_w * arm_sin_f32(start_angle));
	__ctxt.y = (int) (-radius_out_h * arm_cos_f32(start_angle));

	(void)__move_to(__ctxt.x, __ctxt.y);

	// Draw outer ellipse
	(void)__approximate_ellipse_arc_to(radius_out_w, radius_out_h, start_angle, end_angle);

	if (fill == false) {
		// Line to inner ellipse start
		(void)__line_to(
				(int) (__ctxt.center_x + (__ctxt.sin * radius_in_w)),
				(int) (__ctxt.center_y - (__ctxt.cos * radius_in_h)));

		// Draw inner ellipse
		(void)__approximate_ellipse_arc_to(radius_in_w, radius_in_h, end_angle, start_angle);
	} else {
		(void)__line_to(0, 0);
	}

	(void)__end();

	return __update_path(ellipse_arc, true, true,
			-radius_out_w, -radius_out_h, radius_out_w, radius_out_h);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_ellipse(
		vg_lite_path_t *ellipse_shape,
		int path_offset,
		int radius_w,
		int radius_h,
		bool end_path) {
	bool first_path = (path_offset == 0);

	// Initialize the drawing context
	__init_drawing(ellipse_shape, path_offset);

	// Move to top
	(void)__move_to(0, -radius_h);

	(void)__multiple_quarter_curve_to(radius_w, radius_h, 0, 4);

	return __update_path(ellipse_shape, first_path, end_path,
			-radius_w, -radius_h, radius_w, radius_h);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_filled_ellipse(
		vg_lite_path_t *point_shape,
		int radius_w,
		int radius_h,
		vg_lite_matrix_t *matrix) {

	// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
	point_shape->path = (void *) __circle_s16;
	vg_lite_float_t scale_w = (float32_t) radius_w / CIRCLE_RADIUS;
	vg_lite_float_t scale_h = (float32_t) radius_h / CIRCLE_RADIUS;

	vg_lite_scale(scale_w, scale_h, matrix);

	point_shape->path_length = sizeof(__circle_s16);

	return __update_path(point_shape, true, false,
			-CIRCLE_RADIUS,
			-CIRCLE_RADIUS,
			+CIRCLE_RADIUS,
			+CIRCLE_RADIUS);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_thick_shape_line(
		vg_lite_path_t *thick_line_shape,
		int x,
		int y,
		int thickness,
		int caps,
		vg_lite_matrix_t *matrix) {
	int length;
	int top;
	int bottom;
	int cap;

	// Compute the length of the line: sqrt(dx^2 + dy^2)
	length = (int) mej_sqrt_f32((x*x) + (y*y));

	// Compute line angle with x axis
	vg_lite_rotate(MEJ_RAD2DEG(atan2(y, x)), matrix);

	int half_thickness = thickness / 2;
	top = -half_thickness;
	bottom = thickness - half_thickness;

	// Initialize the drawing context
	__init_drawing(thick_line_shape, 0);

	// Move to top
	(void)__move_to(0, top);

	// Line to upper end side
	(void)__line_to(length, top);

	cap = MEJ_VGLITE_PATH_GET_CAPS_END(caps);
	switch(cap) {
	case DRAWING_ENDOFLINE_ROUNDED:
	{
		// Cubics for end cap
		__set_quarter_curve_tangent_axis(__horizontal);
		(void)__quarter_curve_to(length + half_thickness, 0);
		(void)__quarter_curve_to(length, bottom);
		break;
	}
	case DRAWING_ENDOFLINE_NONE:
	case DRAWING_ENDOFLINE_PERPENDICULAR:
	{
		// Compute line cap
		(void)__line_to(length, bottom);
		break;
	}
	default:
		DISPLAY_IMPL_error(false, "Unknown cap: %d", cap);
		break;
	}

	// Line to bottom origin
	(void)__line_to(0, bottom);

	// Cubics for start cap
	cap = MEJ_VGLITE_PATH_GET_CAPS_START(caps);
	switch(cap) {
	case DRAWING_ENDOFLINE_ROUNDED:
	{
		// Cubics for end cap
		__set_quarter_curve_tangent_axis(__horizontal);
		(void)__quarter_curve_to(-half_thickness, 0);
		(void)__quarter_curve_to(0, top);
		break;
	}
	case DRAWING_ENDOFLINE_NONE:
	case DRAWING_ENDOFLINE_PERPENDICULAR:
	{
		// Compute line cap
		(void)__line_to(0, top);
		break;
	}
	default:
		DISPLAY_IMPL_error(false, "Unknown cap: %d", cap);
		break;
	}

	return __update_path(thick_line_shape, true, true,
			-half_thickness, top, length + half_thickness, bottom);
}

// See the header file for the function documentation
int VGLITE_PATH_compute_thick_shape_ellipse_arc(
		vg_lite_path_t *thick_ellipse_arc_shape,
		int diameter_w,
		int diameter_h,
		int thickness,
		float32_t start_angle_deg,
		float32_t arc_angle_deg,
		int caps) {
	int path_length;

	float32_t l_arc_angle_deg = arc_angle_deg;
	float32_t l_start_angle_deg = start_angle_deg;
	int l_caps = caps;

	if (l_arc_angle_deg < 0) {
		// invert start angle and end angle
		l_start_angle_deg += l_arc_angle_deg;
		l_arc_angle_deg = -l_arc_angle_deg;
		int cap1 = MEJ_VGLITE_PATH_GET_CAPS_START(l_caps);
		int cap2 = MEJ_VGLITE_PATH_GET_CAPS_END(l_caps);
		l_caps = 0;
		l_caps |= MEJ_VGLITE_PATH_SET_CAPS_START(cap2);
		l_caps |= MEJ_VGLITE_PATH_SET_CAPS_END(cap1);
	}

	if (l_arc_angle_deg > 360) {
		l_arc_angle_deg = 360;
	}
	l_start_angle_deg = __normalize_angle(l_start_angle_deg);

	float32_t start_angle_rad = MEJ_DEG2RAD(l_start_angle_deg);
	float32_t arc_angle_rad = MEJ_DEG2RAD(l_arc_angle_deg);

	int radius_out_w = (diameter_w + thickness)/2;
	int radius_out_h = (diameter_h + thickness)/2;
	int radius_in_w = ((diameter_w - thickness)/2) + 1;
	int radius_in_h = ((diameter_h - thickness)/2) + 1;

	// Initialize the drawing context
	__init_drawing(thick_ellipse_arc_shape, 0);

	path_length = __approximate_ellipse_arc(
			radius_out_w,
			radius_out_h,
			radius_in_w,
			radius_in_h,
			start_angle_rad,
			arc_angle_rad,
			l_caps);

	thick_ellipse_arc_shape->path_length = path_length;

	return __update_path(thick_ellipse_arc_shape, true, true,
			-radius_out_w,
			-radius_out_h,
			+radius_out_w,
			+radius_out_h);
}

// See the header file for the function documentation
inline void VGLITE_PATH_update_color(vg_lite_color_t* color, vg_lite_blend_t blend) {
	if (VG_LITE_BLEND_SRC_OVER == blend){
		*color =  DISPLAY_VGLITE_porter_duff_workaround_ARGB8888(*color);
	}
	// else: nothing to do
}

// See the header file for the function documentation
void VGLITE_PATH_update_gradient(vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) {

	// convert colors to bypass porter duff limitation
	if (VG_LITE_BLEND_SRC_OVER == blend){
		uint32_t count = gradient->count;
		uint32_t* colors = gradient->colors;
		for (uint32_t i = 0; i < count; i++) {
			colors[i] = DISPLAY_VGLITE_porter_duff_workaround_ARGB8888(colors[i]);
		}
	}

	// update the VG-Lite internal image that represents the gradient
	(void)vg_lite_update_grad(gradient); // always success
}

// See the header file for the function documentation
DRAWING_Status VGLITE_PATH_post_operation(vg_lite_error_t vg_lite_error) {
	DRAWING_Status ret;
	if (VG_LITE_SUCCESS != vg_lite_error) {
		DISPLAY_IMPL_error(true, "vg_lite operation failed with error: %d\n", vg_lite_error);
		ret = DRAWING_DONE;
	} else {
		ret = DRAWING_RUNNING;
		// start GPU operation
		DISPLAY_VGLITE_start_operation(true);
	}
	return ret;
}

// See the header file for the function documentation
inline VG_DRAWER_drawer_t* VGLITE_PATH_get_vglite_drawer(MICROUI_GraphicsContext* gc) {
	vglite_drawer.target = (void*)DISPLAY_VGLITE_configure_destination(gc);
	return &vglite_drawer;
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __init_drawing(vg_lite_path_t *path, int offset) {
	__ctxt.path = path->path;
	__ctxt.offset = offset;
	__ctxt.length = path->path_length;
}

// See the section 'Internal function definitions' for the function documentation
static int __move_to(int16_t x, int16_t y) {
	uint8_t *path = __ctxt.path;

	// cppcheck-suppress [misra-c2012-11.3] cast to (path_move_to_s16_t *) is valid
	path_move_to_s16_t *move_to = (path_move_to_s16_t *) &path[__ctxt.offset];

	move_to->cmd = VGLITE_MOVE_CMD;
	move_to->x = x;
	move_to->y = y;

	__ctxt.x = x;
	__ctxt.y = y;
	__ctxt.offset += (int)MEJ_VGLITE_PATH_MOVE_TO_LENGTH(s16_t);

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static int __line_to(int16_t x, int16_t y)
{
	// cppcheck-suppress [misra-c2012-11.3] cast to (path_line_to_s16_t *) is valid
	path_line_to_s16_t *line_to = (path_line_to_s16_t *) &__ctxt.path[__ctxt.offset];

	line_to->cmd = VGLITE_LINE_CMD;
	line_to->x = x;
	line_to->y = y;

	__ctxt.x = x;
	__ctxt.y = y;
	__ctxt.offset += (int)MEJ_VGLITE_PATH_LINE_TO_LENGTH(s16_t);

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static int __quarter_curve_to(int x, int y) {
	int radius_h = x - __ctxt.x;
	int radius_v = y - __ctxt.y;
	int control_h = (int) (QUARTER_TAN * radius_h);
	int control_v = (int) (QUARTER_TAN * radius_v);
	int c1_x;
	int c1_y;
	int c2_x;
	int c2_y;

	if (__horizontal == __ctxt.tangent_axis) {
		c1_x = __ctxt.x + control_h;
		c1_y = __ctxt.y;

		c2_x = x;
		c2_y = y - control_v;

		__ctxt.tangent_axis = __vertical;
	} else {
		c1_x = __ctxt.x;
		c1_y = __ctxt.y + control_v;

		c2_x = x - control_h;
		c2_y = y;

		__ctxt.tangent_axis = __horizontal;
	}

	// Start tangeant to horizontal
	(void)__cubic_to(c1_x, c1_y, c2_x, c2_y, x, y);

	__ctxt.x = x;
	__ctxt.y = y;

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static int __multiple_quarter_curve_to(
		int radius_w,
		int radius_h,
		int start,
		int number) {
	int data_idx;

	data_idx = start;
	int l_number = number;
	int l_radius_w = radius_w;
	int l_radius_h = radius_h;

	if (l_number < 0) {
		l_number = -l_number;

		if (1 == (data_idx & 1)) {
			l_radius_h = -l_radius_h;
		} else {
			l_radius_w = -l_radius_w;
		}
	}

	if (1 == (data_idx & 1)) {
		__set_quarter_curve_tangent_axis(__vertical);
	} else {
		__set_quarter_curve_tangent_axis(__horizontal);
	}

	// Compute center
	__ctxt.center_x = __ctxt.x - (l_radius_w * __quarter_curve_data[data_idx & (4 - 1)].x);
	__ctxt.center_y = __ctxt.y + (l_radius_h * __quarter_curve_data[data_idx & (4 - 1)].y);

	while (0 != l_number) {
		data_idx = (data_idx + 1) & (4 - 1);

		int next_x = __ctxt.center_x + (l_radius_w * __quarter_curve_data[data_idx & (4 - 1)].x);
		int next_y = __ctxt.center_y - (l_radius_h * __quarter_curve_data[data_idx & (4 - 1)].y);

		(void)__quarter_curve_to(next_x, next_y);
		--l_number;
	}

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static int __approximate_ellipse_arc(
		int radius_out_w,
		int radius_out_h,
		int radius_in_w,
		int radius_in_h,
		float32_t start_angle_rad,
		float32_t arc_angle_rad,
		int caps) {

	// Number of sections per curve.
	// CircleArc approximation from Bezier curves is acceptable if the arc angle is less than 90 degrees.
	// If the caller require and arc angle bigger than 90 degrees,
	// the curve should be splited in sections of less than 90 degrees.
	int nb_sections = (int) ceil(arc_angle_rad / (PI / 2));

	// angle of one section
	float32_t section_angle = arc_angle_rad / nb_sections;

	float32_t cos_start_angle = arm_cos_f32(start_angle_rad);
	float32_t sin_start_angle = arm_sin_f32(start_angle_rad);
	float32_t cos_end_angle = arm_cos_f32(start_angle_rad + arc_angle_rad);
	float32_t sin_end_angle = arm_sin_f32(start_angle_rad + arc_angle_rad);

	float32_t tangent = 4 * mej_tan_f32(section_angle/4) / 3;

	// Compute outside start point
	int16_t out_start_x = (int16_t) (+radius_out_w * cos_start_angle);
	int16_t out_start_y = (int16_t) (-radius_out_h * sin_start_angle);

	// Compute inside start point
	int16_t in_start_x = (int16_t) (+radius_in_w * cos_start_angle);
	int16_t in_start_y = (int16_t) (-radius_in_h * sin_start_angle);

	// Compute outside end point
	int16_t out_end_x = (int16_t) (+radius_out_w * cos_end_angle);
	int16_t out_end_y = (int16_t) (-radius_out_h * sin_end_angle);

	// Compute inside end point
	int16_t in_end_x = (int16_t) (+radius_in_w * cos_end_angle);
	int16_t in_end_y = (int16_t) (-radius_in_h * sin_end_angle);

	// Move to beginning
	(void)__move_to(out_start_x, out_start_y);

	// Compute first (going) curve
	for (int i = 0; i < nb_sections; i++) {
		float32_t section_start_angle = start_angle_rad + (i * section_angle);
		float32_t section_end_angle = start_angle_rad + (i+1) * section_angle;
		(void)__approximate_ellipse_arc_fragment(
				0, 0,
				radius_out_w, radius_out_h,
				section_start_angle, section_end_angle, tangent, 0);
	}

	// Compute end cap
	int cap = MEJ_VGLITE_PATH_GET_CAPS_END(caps);
	switch(cap) {
	case DRAWING_ENDOFLINE_ROUNDED:
	{
		// Compute rounded cap
		float32_t cap_start_angle = start_angle_rad + arc_angle_rad;
		float32_t cap_end_angle = start_angle_rad + arc_angle_rad + PI;
		uint16_t cap_x = (in_end_x + out_end_x) / 2;
		uint16_t cap_y = (in_end_y + out_end_y) / 2;
		// FIXME radius should depend on position of start/end points
		uint16_t cap_radius = (uint16_t)((radius_out_w - radius_in_w) / 2);
		(void)__approximate_ellipse_arc_fragment(
				cap_x, cap_y,
				cap_radius, cap_radius,
				cap_start_angle, cap_end_angle, CAP_TAN, 0);
		break;
	}
	case DRAWING_ENDOFLINE_NONE:
	case DRAWING_ENDOFLINE_PERPENDICULAR:
	{
		// Compute line cap
		(void)__line_to(in_end_x, in_end_y);
		break;
	}
	default:
		DISPLAY_IMPL_error(false, "Unknown cap: %d", cap);
		break;
	}

	// Compute second (return) curve
	for (int i = 0; i < nb_sections; i++) {
		float32_t section_start_angle = start_angle_rad + (nb_sections-1-i) * section_angle;
		float32_t section_end_angle = start_angle_rad + (nb_sections-i) * section_angle;
		(void)__approximate_ellipse_arc_fragment(
				0, 0, radius_in_w, radius_in_h,
				section_start_angle, section_end_angle, tangent, 1);
	}

	// Compute start cap
	cap = MEJ_VGLITE_PATH_GET_CAPS_START(caps);
	switch(cap) {
	case DRAWING_ENDOFLINE_ROUNDED:
	{
		// Compute rounded cap
		float32_t cap_start_angle = start_angle_rad + PI;
		float32_t cap_end_angle = start_angle_rad;
		uint16_t cap_x = (in_start_x + out_start_x) / 2;
		uint16_t cap_y = (in_start_y + out_start_y) / 2;
		// FIXME radius should depend on position of start/end points
		uint16_t cap_radius = (uint16_t)((radius_out_w - radius_in_w) / 2);
		(void)__approximate_ellipse_arc_fragment(
				cap_x, cap_y,
				cap_radius, cap_radius,
				cap_start_angle, cap_end_angle, CAP_TAN, 0);
		break;
	}
	case DRAWING_ENDOFLINE_NONE:
	case DRAWING_ENDOFLINE_PERPENDICULAR:
		// Compute line cap
		(void)__line_to(out_start_x, out_start_y);
		break;
	default:
		DISPLAY_IMPL_error(false, "Unknown cap: %d", cap);
		break;
	}

	// End path
	return __end();
}

// See the section 'Internal function definitions' for the function documentation
static int __approximate_ellipse_arc_to(
		int radius_w,
		int radius_h,
		float32_t start_angle,
		float32_t end_angle) {
	float32_t start_angle_temp;
	float32_t end_angle_temp;
	float32_t arc_angle;
	float32_t arc_angle_temp;
	float32_t arc_angle_abs;

	arc_angle = end_angle - start_angle;

	start_angle_temp = start_angle;

	arc_angle_abs = MEJ_ABS(arc_angle);

	while (arc_angle_abs > 0) {
		arc_angle_temp = end_angle - start_angle_temp;
		arc_angle_temp = MEJ_BOUNDS(arc_angle_temp, -(PI/2), +(PI/2));
		end_angle_temp = start_angle_temp + arc_angle_temp;

		(void)__approximate_ellipse_arc_fragment_to(
				radius_w, radius_h,
				start_angle_temp,
				end_angle_temp);

		start_angle_temp += arc_angle_temp;

		arc_angle_abs -= PI/2;
	}

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static int __approximate_ellipse_arc_fragment_to(
		int radius_w,
		int radius_h,
		float32_t start_angle,
		float32_t end_angle) {
	// End point angle
	float32_t section_angle;
	float32_t temp_x;
	float32_t temp_y;

	// End point coordinates
	int end_x;
	int end_y;

	// Control point 1 coordinates
	int c1_x;
	int c1_y;

	// Control point 2 coordinates
	int c2_x;
	int c2_y;

	section_angle = end_angle - start_angle;

	// Compute start point
	__compute_control_point(
			&temp_x, &temp_y,
			start_angle,
			section_angle);

	c1_x = __ctxt.x + (int) (temp_x * radius_w);
	c1_y = __ctxt.y + (int) (temp_y * radius_h);

	// Compute end point
	__ctxt.sin = arm_sin_f32(end_angle);
	__ctxt.cos = arm_cos_f32(end_angle);

	end_x = (int) (__ctxt.center_x + (radius_w * __ctxt.sin));
	end_y = (int) (__ctxt.center_y + (radius_h * -__ctxt.cos));

	__compute_control_point(
			&temp_x, &temp_y,
			end_angle,
			-section_angle);

	c2_x = end_x + (int) (temp_x * radius_w);
	c2_y = end_y + (int) (temp_y * radius_h);

	return __cubic_to(c1_x, c1_y, c2_x, c2_y, end_x, end_y);
}

// See the section 'Internal function definitions' for the function documentation
static int __approximate_ellipse_arc_fragment(
		uint16_t center_x, uint16_t center_y,
		int radius_w, int radius_h,
		float32_t start_angle_rad,
		float32_t end_angle_rad,
		float32_t tangent,
		int revert) {
	float32_t tx;
	float32_t ty;

	// Compute start point A1 & A2
	int16_t Ax = (int16_t) (radius_w * arm_cos_f32(start_angle_rad));
	int16_t Ay = (int16_t) (radius_h * -arm_sin_f32(start_angle_rad)); // minus because y axis is inverted (0,0 is top left)

	// Compute end point B1 & B2
	int16_t Bx = (int16_t) (radius_w * arm_cos_f32(end_angle_rad));
	int16_t By = (int16_t) (radius_h * -arm_sin_f32(end_angle_rad));

	// Compute control point C
	tx = tangent * arm_cos_f32(start_angle_rad + (PI/2));
	ty = tangent * -arm_sin_f32(start_angle_rad + (PI/2));

	int16_t Cx = (int16_t) (Ax + (radius_w * tx));
	int16_t Cy = (int16_t) (Ay + (radius_h * ty));

	// Compute control point D
	tx = tangent * arm_cos_f32(end_angle_rad - (PI/2));
	ty = tangent * -arm_sin_f32(end_angle_rad - (PI/2));

	int16_t Dx = (int16_t) (Bx + (radius_w * tx));
	int16_t Dy = (int16_t) (By + (radius_h * ty));

	Ax += (int16_t)center_x;
	Ay += (int16_t)center_y;
	Bx += (int16_t)center_x;
	By += (int16_t)center_y;
	Cx += (int16_t)center_x;
	Cy += (int16_t)center_y;
	Dx += (int16_t)center_x;
	Dy += (int16_t)center_y;

	// Compute path
	return (revert == 0) ?
			__cubic_to(Cx, Cy, Dx, Dy, Bx, By) :
			__cubic_to(Dx, Dy, Cx, Cy, Ax, Ay);
}

// See the section 'Internal function definitions' for the function documentation
int __cubic_to(
		int16_t cx1,
		int16_t cy1,
		int16_t cx2,
		int16_t cy2,
		int16_t x,
		int16_t y)
{
	// cppcheck-suppress [misra-c2012-11.3] cast to (path_cubic_to_s16_t *) is valid
	path_cubic_to_s16_t *cubic_to = (path_cubic_to_s16_t *) &__ctxt.path[__ctxt.offset];

	cubic_to->cmd = VGLITE_CUBIC_CMD;
	cubic_to->cx1 = cx1; // control point 1
	cubic_to->cy1 = cy1; // control point 1
	cubic_to->cx2 = cx2; // control point 2
	cubic_to->cy2 = cy2; // control point 2
	cubic_to->x = x; // curve end
	cubic_to->y = y; // curve end

	__ctxt.x = x;
	__ctxt.y = y;
	__ctxt.offset += (int)MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(s16_t);

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
int __end(void)
{
	// cppcheck-suppress [misra-c2012-11.3] cast to (path_end_s16_t *) is valid
	path_end_s16_t *end = (path_end_s16_t *) &__ctxt.path[__ctxt.offset];
	end->cmd = VGLITE_END_CMD;

	__ctxt.offset += (int)MEJ_VGLITE_PATH_END_LENGTH(s16_t);

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
int __update_path(vg_lite_path_t *path,
		bool first_path,
		bool last_path,
		int left,
		int top,
		int right,
		int bottom) {

	if (!first_path) {
		path->bounding_box[0] = (int) MEJ_MIN(path->bounding_box[0], left);
		path->bounding_box[1] = (int) MEJ_MIN(path->bounding_box[1], top);
		path->bounding_box[2] = (int) MEJ_MAX(path->bounding_box[2], right);
		path->bounding_box[3] = (int) MEJ_MAX(path->bounding_box[3], bottom);
	}
	else {
		path->bounding_box[0] = left;
		path->bounding_box[1] = top;
		path->bounding_box[2] = right;
		path->bounding_box[3] = bottom;
	}

	path->quality = VG_LITE_HIGH;
	path->format = VG_LITE_S16;
	(void)memset(&path->uploaded, 0,
			sizeof(path->uploaded));
	path->path_changed = 1;
	path->path_type = VG_LITE_DRAW_FILL_PATH;
	path->pdata_internal = 0;

	if (last_path) {
		(void)__end();
		path->path_length = __ctxt.offset;
	}

	return __ctxt.offset;
}

// See the section 'Internal function definitions' for the function documentation
static float32_t __normalize_angle(float32_t angle) {
	float32_t l_angle = fmod(angle, 360);
	return (l_angle >= 0) ? l_angle : (l_angle + 360);
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_error_t __drawer_vglite_draw_path(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_blend_t blend,
		vg_lite_color_t color) {
	return vg_lite_draw((vg_lite_buffer_t*)target, path, fill_rule, matrix, blend, color);
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_error_t __drawer_vglite_draw_gradient(
		void* target,
		vg_lite_path_t * path,
		vg_lite_fill_t fill_rule,
		vg_lite_matrix_t * matrix,
		vg_lite_linear_gradient_t * grad,
		vg_lite_blend_t blend) {
	return vg_lite_draw_gradient((vg_lite_buffer_t*)target, path, fill_rule, matrix, grad, blend);
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_error_t __drawer_vglite_blit_rect(
		void* target,
		vg_lite_buffer_t *source,
		uint32_t         *rect,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) {
	return vg_lite_blit_rect((vg_lite_buffer_t*)target, source, rect, matrix, blend, color, filter);
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_error_t __drawer_vglite_blit(
		void* target,
		vg_lite_buffer_t *source,
		vg_lite_matrix_t *matrix,
		vg_lite_blend_t   blend,
		vg_lite_color_t   color,
		vg_lite_filter_t  filter) {
	return vg_lite_blit((vg_lite_buffer_t*)target, source, matrix, blend, color, filter);
}

// See the section 'Internal function definitions' for the function documentation
static vg_lite_error_t __drawer_vglite_clear(
		void* target,
		vg_lite_rectangle_t *rectangle,
		vg_lite_color_t color) {
	return vg_lite_clear((vg_lite_buffer_t*)target, rectangle, color);
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
