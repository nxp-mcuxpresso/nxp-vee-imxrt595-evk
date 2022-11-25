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

#if !defined VGLITE_PATH_H
#define VGLITE_PATH_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <arm_math.h>

#include "mej_math.h"

#include "vg_lite.h"
#include "vg_drawer.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define VGLITE_END_CMD     0
#define VGLITE_MOVE_CMD    2
#define VGLITE_LINE_CMD    4
#define VGLITE_CUBIC_CMD   8

/*
 * @brief Length of a VGLite line path
 */
#define MEJ_VGLITE_PATH_LINE_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)	/* move to command */ \
		+ 6 * MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)	/* line have 6 sides */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)		/* end command */ \
	)

/*
 * @brief Length of a VGLite thick shape line path
 */
#define MEJ_VGLITE_PATH_THICK_SHAPE_LINE_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)	/* move to command */ \
		+ 2 * MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)	/* line have 6 sides */ \
		+ 2 * MEJ_MAX(	MEJ_VGLITE_PATH_ROUNDED_CAP_LENGTH(t), \
						MEJ_VGLITE_PATH_NO_CAP_LENGTH(t))		/* CAPS */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)		/* end command */ \
	)

/*
 * @brief Length of a VGLite ellipse path
 */
#define MEJ_VGLITE_PATH_CIRCLE_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)		/* move to command */ \
		+ 4 * MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t)	/* 4 sections per ellipse */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)			/* end command */ \
	)

/*
 * @brief Length of a VGLite ellipse path
 */
#define MEJ_VGLITE_PATH_CIRCLE_ARC_OUTLINE_MAX_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)		/* move to command */ \
		+ 2 * MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)		/* 2 ends */ \
		+ 8 * MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t)	/* 4 sections per ellipse */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)			/* end command */ \
	)

/*
 * @brief Length of a VGLite filled rectangle path
 */
#define MEJ_VGLITE_PATH_RECT_LENGTH(t) \
    (0 \
        + 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)        /* move to command */ \
        + 4 * MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)        /* 4 sides */ \
        + 1 * MEJ_VGLITE_PATH_END_LENGTH(t)            /* end command */ \
    )

/*
 * @brief Length of a VGLite filled rounded rectangle path
 */
#define MEJ_VGLITE_PATH_ROUND_RECT_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)		/* move to command */ \
		+ 4 * MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)		/* 4 sides */ \
		+ 4 * MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t)	/* 4 corners */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)			/* end command */ \
	)

/*
 * @brief Maximum length of a VGLite filled ellipse arc path
 */
#define MEJ_VGLITE_PATH_CIRCLE_ARC_MAX_LENGTH(t) \
	(0 \
		+ 1 * MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)				/* move to command */ \
		+ 2 * MEJ_MAX(	MEJ_VGLITE_PATH_ROUNDED_CAP_LENGTH(t), \
						MEJ_VGLITE_PATH_NO_CAP_LENGTH(t))		/* CAPS */ \
		+ 8 * MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t)			/* Up to 4 sections per curve in and out */ \
		+ 1 * MEJ_VGLITE_PATH_END_LENGTH(t)					/* end command */ \
	)

/*
 * @brief Size of a rounded cap: 2 cubic curves
 */
#define MEJ_VGLITE_PATH_ROUNDED_CAP_LENGTH(t)       (2 * MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t))

/*
 * @brief Size of a "no cap": 1 line
 */
#define MEJ_VGLITE_PATH_NO_CAP_LENGTH(t)            MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)

/*
 * @brief Size of a move_to command
 */
#define MEJ_VGLITE_PATH_MOVE_TO_LENGTH(t)           (sizeof(path_move_to_ ## t))

/*
 * @brief Size of a line_to command
 */
#define MEJ_VGLITE_PATH_LINE_TO_LENGTH(t)           (sizeof(path_line_to_ ## t))

/*
 * @brief Size of a cubic_to command
 */
#define MEJ_VGLITE_PATH_CUBIC_TO_LENGTH(t)          (sizeof(path_cubic_to_ ## t))

/*
 * @brief Size of a end command
 */
#define MEJ_VGLITE_PATH_END_LENGTH(t)               (sizeof(path_end_ ## t))

/*
 * @brief Position of the starting cap in the caps bitfield
 */
#define MEJ_VGLITE_PATH_CAPS_START_SHIFT            0

/*
 * @brief Position of the ending cap in the caps bitfield
 */
#define MEJ_VGLITE_PATH_CAPS_END_SHIFT              2

/*
 * @brief Mask of a cap in the caps bitfield
 */
#define MEJ_VGLITE_PATH_CAPS_MASK                   0x03

/*
 * @brief Sets the starting cap in the bitfield
 */
#define MEJ_VGLITE_PATH_SET_CAPS_START(x)           (((x) & MEJ_VGLITE_PATH_CAPS_MASK) << MEJ_VGLITE_PATH_CAPS_START_SHIFT)

/*
 * @brief Sets the ending cap in the bitfield
 */
#define MEJ_VGLITE_PATH_SET_CAPS_END(x)             (((x) & MEJ_VGLITE_PATH_CAPS_MASK) << MEJ_VGLITE_PATH_CAPS_END_SHIFT)

/*
 * @brief Gets the starting cap from the bitfield
 */
#define MEJ_VGLITE_PATH_GET_CAPS_START(x)           (((x) >> MEJ_VGLITE_PATH_CAPS_START_SHIFT) & MEJ_VGLITE_PATH_CAPS_MASK)

/*
 * @brief Gets the ending cap from the bitfield
 */
#define MEJ_VGLITE_PATH_GET_CAPS_END(x)             (((x) >> MEJ_VGLITE_PATH_CAPS_END_SHIFT) & MEJ_VGLITE_PATH_CAPS_MASK)

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief VGLite S16 move to command
 */
typedef struct path_move_to_s16 {
    int16_t cmd;	// @brief Command
    int16_t x;		// @brief Horizontal coordinate of the position to move to
    int16_t y;		// @brief Vertical coordinate of the position to move to
} path_move_to_s16_t;

/*
 * @brief VGLite S16 line to command
 */
typedef struct path_line_s16 {
    int16_t cmd;	// @brief Command
    int16_t x;		// @brief Horizontal coordinate of the position to move to
    int16_t y;		// @brief Vertical coordinate of the position to move to
} path_line_to_s16_t;

/*
 * @brief VGLite S16 cubic to command
 */
typedef struct path_cubic_s16 {
    int16_t cmd;	// @brief command
    int16_t cx1;	// @brief Horizontal coordinate of the first control point
    int16_t cy1;	// @brief Vertical coordinate of the first control point
    int16_t cx2;	// @brief Horizontal coordinate of the second control point
    int16_t cy2;	// @brief Vertical coordinate of the second control point
    int16_t x;		// @brief Horizontal coordinate of the end of the curve
    int16_t y;		// @brief Vertical coordinate of the end of the curve
} path_cubic_to_s16_t;

/*
 * @brief VGLite S16 end command
 */
typedef struct path_end_s16 {
    int16_t cmd;	// @brief command
} path_end_s16_t;

/*
 * @brief Fixed array size to store a line path
 */
typedef uint8_t vglite_path_line_t[MEJ_VGLITE_PATH_LINE_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a thick shape line path
 */
typedef uint8_t vglite_path_thick_shape_line_t[MEJ_VGLITE_PATH_THICK_SHAPE_LINE_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a ellipse
 */
typedef uint8_t vglite_path_ellipse_t[MEJ_VGLITE_PATH_CIRCLE_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a ellipse arc
 */
typedef uint8_t vglite_path_ellipse_arc_t[MEJ_VGLITE_PATH_CIRCLE_ARC_OUTLINE_MAX_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a rectangle path
 */
typedef uint8_t vglite_path_rectangle_t[MEJ_VGLITE_PATH_RECT_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a line path
 */
typedef uint8_t vglite_path_rounded_rectangle_t[MEJ_VGLITE_PATH_ROUND_RECT_LENGTH(s16_t)];

/*
 * @brief Fixed array size to store a thick ellipse arc path
 */
typedef uint8_t vglite_path_thick_ellipse_arc_t[MEJ_VGLITE_PATH_CIRCLE_ARC_MAX_LENGTH(s16_t)];

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Computes a ellipse arc
 *
 * This function uses bezier curves to apporoximate the ellipse arcs
 * The path computed by this function is intended to be used with VGLite
 * The function supports the followin caps: DRAWING_ENDOFLINE_NONE and DRAWING_ENDOFLINE_ROUNDED.
 * DRAWING_ENDOFLINE_PERPENDICULAR is considered as DRAWING_ENDOFLINE_NONE.
 * @param[out] thick_shape_ellipse_arc: the vg_lite path to compute
 * @param[in]: radius_out_w: the horizontal inner radius of the ellipse arc
 * @param[in]: radius_out_h: the vertical inner radius of the ellipse arc
 * @param[in]: radius_in_w: the horizontal outer radius of the ellipse arc
 * @param[in]: radius_in_h: the vertical outer radius of the ellipse arc
 * @param[in]: start_angle_degree: the start angle of the ellipse arc in degrees
 * @param[in]: arc_angle_degree: the arc angle of the ellipse arc in degrees
 * @param[in] fill: the flag indicating if the ellipse arc will be filled
 * - true: the ellipse arc will be filled
 * - false: the ellipse arc will not be filled
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_ellipse_arc(
		vg_lite_path_t *thick_ellipse_arc_shape,
		int radius_out_w,
		int radius_out_h,
		int radius_in_w,
		int radius_in_h,
		float32_t start_angle_degree,
		float32_t arc_angle_degree,
		bool fill);

/*
 * @brief Computes a thick ellipse arc shape
 *
 * This function uses bezier curves to apporoximate the ellipse arcs
 * The path computed by this function is intended to be used with VGLite
 * The function supports the followin caps: DRAWING_ENDOFLINE_NONE and DRAWING_ENDOFLINE_ROUNDED.
 * DRAWING_ENDOFLINE_PERPENDICULAR is considered as DRAWING_ENDOFLINE_NONE.
 * @param[out] thick_shape_ellipse_arc: vg_lite path to compute
 * @param[in]: diameter_w: diameter of the ellipse arc
 * @param[in]: diameter_h: diameter of the ellipse arc
 * @param[in]: thickness: thickness of the ellipse arc
 * @param[in]: start_angle_degree: starting angle of the ellipse arc in degrees
 * @param[in]: arc_angle_degree: angle of the ellipse arc in degrees
 * @param[in]: caps of the start and end of the ellipse arc, bits [0:1] contains the starting cap, bits [2:3] contains the ending cap.
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_thick_shape_ellipse_arc(
		vg_lite_path_t *thick_ellipse_arc_shape,
		int diameter_w,
		int diameter_h,
		int thickness,
		float32_t start_angle_degree,
		float32_t arc_angle_degree,
		int caps);

/*
 * @brief Computes a ellipse
 *
 * This function uses bezier curves to apporoximate the ellipse
 * The path computed by this function is intended to be used with VGLite
 * @param[out] point_shape: vg_lite path to compute
 * @param[in] thickness: thickness (diameter) of the point
 * @param[in,out] matrix: transformation matrix (updated to scale the point)
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_ellipse(
		vg_lite_path_t *point_shape,
		int path_offset,
		int radius_w,
		int radius_h,
		bool end_path);

/*
 * @brief Computes a plain ellipse
 *
 * This function uses a hard coded path of a thircle of diameter 200.
 * The matrix is updated to scale the ellipse to match the diameter.
 *
 * This function uses bezier curves to apporoximate the ellipse
 * The path computed by this function is intended to be used with VGLite
 * @param[out] point_shape: vg_lite path to compute
 * @param[in] radius_w: the horizontal radius of the ellipse.
 * @param[in] radius_h: the vertical radius of the ellipse.
 * @param[in,out] matrix: transformation matrix (updated to scale the point)
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_filled_ellipse(
		vg_lite_path_t *point_shape,
		int radius_w,
		int radius_h,
		vg_lite_matrix_t *matrix);

/*
 * @brief Computes a rectangle
 *
 * The path computed by this function is intended to be used with VGLite
 * @param[out] rectangle_shape: vg_lite path to compute
 * @param[in] path_offset: offset in the path buffer
 * @param[in] x: horizontal coordinate of the left side of the shape
 * @param[in] y: vertical coordinate of the top side of the shape
 * @param[in] width: width of the rectangle
 * @param[in] height: height of the rectangle
 * @param[in] end_path: true to end the path, false to leave path open
 *
 * @return:
 *     -1: an error occured,
 *     otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_rectangle(
        vg_lite_path_t *rounded_rectangle_shape,
        int path_offset,
        int x,
        int y,
        int width,
        int height,
        bool end_path);

/*
 * @brief Computes a rounded rectangle
 *
 * This function uses bezier curves to apporoximate the corners
 * The path computed by this function is intended to be used with VGLite
 * @param[out] rounded_rectangle_shape: vg_lite path to compute
 * @param[in] path_offset: offset in the path buffer
 * @param[in] x: horizontal coordinate of the left side of the shape
 * @param[in] y: vertical coordinate of the top side of the shape
 * @param[in] height: height of the rounded rectangle
 * @param[in] width: width of the rounded rectangle
 * @param[in] height: height of the rounded rectangle
 * @param[in] arc_width: horizontal diameter of the rounded corner arc
 * @param[in] arc_height: vertical diameter of the rounded corner arc
 * @param[in] end_path: true to end the path, false to leave path open
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_rounded_rectangle(
		vg_lite_path_t *rounded_rectangle_shape,
		int path_offset,
		int x,
		int y,
		int width,
		int height,
		int arc_width,
		int arc_height,
		bool end_path);

/*
 * @brief Computes a line
 *
 * The path computed by this function is intended to be used with VGLite
 * The line will be computed from origin (0,0) to (x,y)
 * @param[out] line_shape: vg_lite path to compute
 * @param[in]: x: horizontal coordinate of the end point of the line
 * @param[in]: y: vertical coordinate of the end point of the line
 * @param[in]: xi: horizontal increment (-1 or 1) according to the line direction (left or right).
 * @param[in]: yi: vertical increment (-1 or 1) according to the line direction (top or bottom).
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_line(vg_lite_path_t *line_shape, int x, int y, int xi, int yi);

/*
 * @brief Computes a thick shape line
 *
 * The path computed by this function is intended to be used with VGLite
 * This function will compute an horizontal line and update a matrix to rotate it
 * The line will be computed from origin (0,0) to sqrt(x^2,y^2)
 * The matrix will be updated to rotate it by atan2(y, x)
 * @param[out] line_shape: vg_lite path to compute
 * @param[in]: x: horizontal coordinate of the end point of the line
 * @param[in]: y: vertical coordinate of the end point of the line
 * @param[in]: thickness: thickness (diameter) of the point
 * @param[in]: caps: caps of the start and end of the ellipse arc, bits [0:1] contains the starting cap, bits [2:3] contains the ending cap.
 * @param[out]: matrix: matrix to be updated with rotation atan2(y, x)
 *
 * @return:
 * 	-1: an error occured,
 * 	otherwise: number of bytes written in the path
 */
int VGLITE_PATH_compute_thick_shape_line(
		vg_lite_path_t *thick_line_shape,
		int x,
		int y,
		int thickness,
		int caps,
		vg_lite_matrix_t *matrix);

/*
 * @brief Function to update a color to be compatible with VG-Lite.
 *
 * @param[in/out] color: pointer to the color to update
 * @param[in] blend: the blending mode to apply
 *
 * @return: nothing
 */
void VGLITE_PATH_update_color(vg_lite_color_t* color, vg_lite_blend_t blend) ;

/*
 * @brief Function to update a gradient to be compatible with VG-Lite.
 *
 * @param[in/out] color: pointer to the color to update
 * @param[in] blend: the blending mode to apply
 *
 * @return: nothing
 */
void VGLITE_PATH_update_gradient(vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend) ;

/*
 * @brief Operation to perform after a VG-Lite drawing operation. On success, the GPU is
 * started.
 *
 * @param[in] vg_lite_error: the VGLite drawing operation's return code.
 *
 * @return: the MicroUI status according to the drawing operation status.
 */
DRAWING_Status VGLITE_PATH_post_operation(vg_lite_error_t vg_lite_error);

/*
 * @brief Gets the drawer that makes an indirection to the VG-Lite library.
 *
 * @param[in] gc: the destination
 *
 * @return the VG-Lite drawer indirection
 */
VG_DRAWER_drawer_t* VGLITE_PATH_get_vglite_drawer(MICROUI_GraphicsContext* gc);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined VGLITE_PATH_H
