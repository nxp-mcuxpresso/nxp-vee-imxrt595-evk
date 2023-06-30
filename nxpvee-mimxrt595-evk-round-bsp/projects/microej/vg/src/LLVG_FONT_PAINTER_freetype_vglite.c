/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * Copyright 2023 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: implementation over Freetype and VG-Lite.
* @author MicroEJ Developer Team
* @version 3.0.0
*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "microvg_configuration.h"

#if defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)

#include <math.h>

#include <freetype/internal/ftobjs.h>
#include <freetype/ftcolor.h>

#include <LLVG_FONT_PAINTER_impl.h>
#include <LLVG_FONT_impl.h>
#include <LLVG_PATH_impl.h>
#include <LLVG_GRADIENT_impl.h>
#include <LLVG_MATRIX_impl.h>
#include <sni.h>

#include "microvg_helper.h"
#include "microvg_font_freetype.h"
#include "microvg_vglite_helper.h"
#include "vg_lite.h"
#include "ftvector/ftvector.h"
#include "display_vglite.h"
#include "vglite_path.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define FT_COLOR_TO_INT(x) (*((int*) &(x)))
#define INT_TO_FT_COLOR(x) (*((FT_Color*) &(x)))

#define DIRECTION_CLOCK_WISE 0

// -----------------------------------------------------------------------------
// Extern Variables
// -----------------------------------------------------------------------------

extern FT_Library library;
extern FT_Renderer renderer;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------
static FT_Color *palette;

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Computes the scale to apply to the font.
 *
 * @param[in] size: the font size
 * @param[in] face: the face of the font
 */
static inline float __get_scale(jfloat size, FT_Face face);

/*
 * @brief Sets renderer parameters.
 *
 * @param[in] gc: the graphic context
 * @param[in] matrix: the transformation matrix
 * @param[in] alpha: the opacity of the string (from 0 to 255)
 * @param[in] gradient: the gradient to apply
 */
static void __set_renderer(MICROUI_GraphicsContext* gc, vg_lite_matrix_t* matrix, int color, vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend);

/*
 * @brief Sets renderer color parameter.
 *
 * @param[in] color: the color to use to render the glyph
 */
static void __set_color(int color);

/*
 * @brief Updates the angle to use for the next glyph when drawn on an arc.
 * When drawing on an arc, the glyph position is defined by its angle. We thus
 * convert the advance (distance to the next glyph) to an angle.
 */
static float __get_angle(float advance, float radius);

/**
 * @brief load and render the selected glyph. If the glyph is a multilayer glyph,
 * this function will retrieve the different layers glyphs with theirs colors and
 * update the renderer to draw the glyph with the correct color.
 *
 * @param[in] face: the face of the font.
 * @param[in] glyph: the glyph index.
 * @param[in] glyph: the original color of the string index.
 * @param[in,out] matrix: the transformation matrix.
 */
static int __render_glyph(FT_Face face, FT_UInt glyph_index, FT_Color color);

/*
 * @brief Draws a string along a circle, with a color or a linear gradient.
 *
 * @param[in] gc the MicroUI GraphicsContext target.
 * @param[in] text the array of characters to draw.
 * @param[in] faceHandle the font reference handle.
 * @param[in] size the height of the font in pixels.
 * @param[in] x the horizontal coordinate of the top/left of the first drawn character.
 * @param[in] y the vertical coordinate of the top/left of the first drawn character.
 * @param[in] vgGlobalMatrix: deformation matrix
 * @param[in] blend the blend mode to use
 * @param[in] color the color to apply if gradient is NULL
 * @param[in] gradient the gradient to apply or NULL
 * @param[in] letterSpacing the extra letter spacing to use
 * @param[in] radius the radius of the circle
 * @param[in] direction the direction of the text along the circle
 */
static void __draw_string(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* vgGlobalMatrix, jint blend, int color, vg_lite_linear_gradient_t *gradient, jfloat letterSpacing, jfloat radius, jint direction );

// -----------------------------------------------------------------------------
// LLVG_FONT_PAINTER_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing){

	jint ret;
	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jint)LLVG_RESOURCE_CLOSED;
	}
	else {
		if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_FONT_PAINTER_IMPL_draw_string))){
			int color = (gc->foreground_color & 0x00FFFFFF) + (int)(((unsigned int) alpha) << 24);
			jfloat* local_matrix = MICROVG_HELPER_check_matrix(matrix);
			__draw_string(gc, text, faceHandle, size, x, y, local_matrix, blend, color, MICROVG_HELPER_NULL_GRADIENT, letterSpacing, 0, 0);
		}
		ret = (jint)LLVG_SUCCESS;
	}
	return ret;
}


// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jint *gradientData, jfloat *gradientMatrix){

	jint ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jint)LLVG_RESOURCE_CLOSED;
	}
	else {
		if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_FONT_PAINTER_IMPL_draw_string_gradient))){

			vg_lite_linear_gradient_t gradient = {0};
			jfloat* local_gradient_matrix = MICROVG_HELPER_check_matrix(gradientMatrix);

			vg_lite_matrix_t local_matrix;
			jfloat* mapped_local_matrix = MAP_VGLITE_MATRIX(&local_matrix);
			LLVG_MATRIX_IMPL_setTranslate(mapped_local_matrix, x, y);
			LLVG_MATRIX_IMPL_concatenate(mapped_local_matrix, matrix);

			MICROVG_VGLITE_HELPER_to_vg_lite_gradient(&gradient, gradientData, local_gradient_matrix, mapped_local_matrix, alpha);

			__draw_string(gc, text, faceHandle, size, x, y, matrix, blend, 0, &gradient, letterSpacing, 0, 0);

			// vg_lite_init_grad allocates a buffer in VGLite buffer, we must free it.
			// No error even if init_grad is never called because vg_lite_clear_grad
			// checks the allocation.
			vg_lite_clear_grad(&gradient);
		}
		ret = (jint)LLVG_SUCCESS;
	}

	return ret;
}


// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction){

	jint ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jint)LLVG_RESOURCE_CLOSED;
	}
	else {
		if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_FONT_PAINTER_IMPL_draw_string_on_circle))){
			int color = (gc->foreground_color & 0x00FFFFFF) + (int)(((unsigned int) alpha) << 24);
			__draw_string(gc, text, faceHandle, size, x, y, matrix, blend, color, MICROVG_HELPER_NULL_GRADIENT, letterSpacing, radius, direction);
		}
		ret = (jint)LLVG_SUCCESS;
	}

	return ret;
}

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction, jint *gradientData, jfloat *gradientMatrix){

	jint ret;

	if (LLVG_FONT_UNLOADED == faceHandle) {
		ret = (jint)LLVG_RESOURCE_CLOSED;
	}
	else {
		if (LLUI_DISPLAY_requestDrawing(gc, (SNI_callback)&(LLVG_FONT_PAINTER_IMPL_draw_string_on_circle_gradient))){

			vg_lite_linear_gradient_t gradient = {0};
			jfloat* local_gradient_matrix = MICROVG_HELPER_check_matrix(gradientMatrix);

			vg_lite_matrix_t local_matrix;
			jfloat* mapped_local_matrix = MAP_VGLITE_MATRIX(&local_matrix);
			LLVG_MATRIX_IMPL_setTranslate(mapped_local_matrix, x, y);
			LLVG_MATRIX_IMPL_concatenate(mapped_local_matrix, matrix);

			MICROVG_VGLITE_HELPER_to_vg_lite_gradient(&gradient, gradientData, local_gradient_matrix, mapped_local_matrix, alpha);

			__draw_string(gc, text, faceHandle, size, x, y, matrix, blend, 0, &gradient, letterSpacing, radius, direction);

			// vg_lite_init_grad allocates a buffer in VGLite buffer, we must free it.
			// No error even if init_grad is never called because vg_lite_clear_grad
			// checks the allocation.
			vg_lite_clear_grad(&gradient);
		}
		ret = (jint)LLVG_SUCCESS;
	}

	return ret;
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

static inline float __get_scale(jfloat size, FT_Face face){
	return size / face->units_per_EM;
}

static void __set_renderer(MICROUI_GraphicsContext* gc, vg_lite_matrix_t* matrix, int color, vg_lite_linear_gradient_t* gradient, vg_lite_blend_t blend){
	FT_Parameter vglite_params[7];

	vglite_params[0].tag = FT_PARAM_TAG_VGLITE_DESTINATION;
	vglite_params[0].data = VG_DRAWER_configure_target(gc);

	vglite_params[1].tag = FT_PARAM_TAG_VGLITE_MATRIX;
	vglite_params[1].data = matrix;

	vglite_params[2].tag = FT_PARAM_TAG_VGLITE_QUALITY;
	vglite_params[2].data = (void *) VG_LITE_HIGH;

	vglite_params[3].tag = FT_PARAM_TAG_VGLITE_FORMAT;
	vglite_params[3].data = (void *) VG_LITE_S16;

	vglite_params[4].tag = FT_PARAM_TAG_VGLITE_BLEND;
	vglite_params[4].data = (void *) blend;

	vglite_params[5].tag = FT_PARAM_TAG_VGLITE_COLOR;
	// cppcheck-suppress [misra-c2012-11.6] pointer conversion to pass color
	vglite_params[5].data = (void *) color;

	vglite_params[6].tag = FT_PARAM_TAG_VGLITE_GRADIENT;
	vglite_params[6].data = (void *) gradient;
	FT_Set_Renderer(library, renderer, 7, &vglite_params[0]);
}

static void __set_color(int color) {

	FT_Renderer renderer = FT_Get_Renderer(library, FT_GLYPH_FORMAT_OUTLINE);
	FT_Renderer_SetModeFunc set_mode = renderer->clazz->set_mode;

	// cppcheck-suppress [misra-c2012-11.6] pointer conversion to pass color
	set_mode(renderer, FT_PARAM_TAG_VGLITE_COLOR, (void *)color);
}

static float __get_angle(float advance, float radius){
	return (advance/radius) * 180.0f / M_PI;
}

static int __render_glyph(FT_Face face, FT_UInt glyph_index, FT_Color color) {
	FT_LayerIterator iterator;

	FT_Bool have_layers;
	FT_UInt layer_glyph_index;
	FT_UInt layer_color_index;

	int error = 0;
	bool color_updated = false;

	iterator.p = NULL;
	have_layers = FT_Get_Color_Glyph_Layer(face, glyph_index,
			&layer_glyph_index, &layer_color_index, &iterator);

	do {
		if (palette && have_layers) {
			// Update renderer color with layer_color
			if (layer_color_index == 0xFFFF){
				__set_color(FT_COLOR_TO_INT(color));
			}
			else {
				__set_color(FT_COLOR_TO_INT(palette[layer_color_index]));
				color_updated = true;
			}
		}
		else {
			// Use main glyph_index as layer_glyph_index
			layer_glyph_index = glyph_index;
		}

		if(layer_glyph_index != glyph_index){
			error = FT_Load_Glyph(face, layer_glyph_index, FT_LOAD_NO_SCALE);
		}

		if (0 == error) {
			// convert to an anti-aliased bitmap
			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		}
		else {
			MEJ_LOG_ERROR_MICROVG("Error while loading glyphid %d: 0x%x, refer to fterrdef.h\n",layer_glyph_index, error);
		}
	}
	while ((layer_glyph_index != glyph_index) && (0 == error) && (0 != FT_Get_Color_Glyph_Layer(face, glyph_index,
			&layer_glyph_index, &layer_color_index, &iterator)));

	if ((0 == error) && color_updated) {
		// Revert renderer color to original color in case it has been modified.
		__set_color(FT_COLOR_TO_INT(color));
	}

	return error;
}

static void __draw_string(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* vgGlobalMatrix, jint blend, int color, vg_lite_linear_gradient_t *gradient, jfloat letterSpacing, jfloat radius, jint direction ){
	FT_Face face = (FT_Face) faceHandle;

	if (MICROVG_VGLITE_HELPER_enable_vg_lite_scissor(gc)){

		// Select palette
		if (0 != FT_Palette_Select(face, 0, &palette)){
			palette = NULL;
		}

		float scale = __get_scale(size, face);
		int advanceX = 0;
		int advanceY = 0;
		float letterSpacingScaled = letterSpacing / scale;
		float radiusScaled = radius / scale;

		// baselineposition
		short baselineposition = face->ascender;

		// Compute matrix to position the glyph
		vg_lite_matrix_t localMatrix;
		jfloat* vgLocalMatrix = MAP_VGLITE_MATRIX(&localMatrix);

		if((0.f != x) || (0.f != y))
		{
			// Create translate matrix for initial x,y translation from graphicscontext.
			LLVG_MATRIX_IMPL_setTranslate(vgLocalMatrix, x, y);
			LLVG_MATRIX_IMPL_concatenate(vgLocalMatrix, vgGlobalMatrix);
		}
		else
		{
			// use original matrix
			LLVG_MATRIX_IMPL_copy(vgLocalMatrix, vgGlobalMatrix);
		}

		LLVG_MATRIX_IMPL_scale(vgLocalMatrix, scale, scale);

		vg_lite_matrix_t localGlyphMatrix;
		jfloat* vglocalGlyphMatrix = MAP_VGLITE_MATRIX(&localGlyphMatrix);

		__set_renderer(gc, &localGlyphMatrix, color, gradient, MICROVG_VGLITE_HELPER_get_blend(blend));

		// Layout variables
		int glyph_index ; // current glyph index
		int previous_glyph_index = 0; // previous glyph index for kerning

		int advance_x;
		int advance_y;
		int offset_x;
		int offset_y;

		int length = (int)SNI_getArrayLength(text);
		MICROVG_HELPER_layout_configure(faceHandle, text, length);

		while(0 != MICROVG_HELPER_layout_load_glyph(&glyph_index, &advance_x, &advance_y, &offset_x, &offset_y)){
			// At that point the current glyph has been loaded by Freetype

			int charWidth = advance_x;

			if (0 == previous_glyph_index){
				// first glyph: remove the first blank line
				if( 0 == face->glyph->metrics.width) {
					advanceX -= charWidth;
				}
				else {
					advanceX -= face->glyph->metrics.horiBearingX;
				}
			}

			LLVG_MATRIX_IMPL_copy(vglocalGlyphMatrix, vgLocalMatrix);

			if(0.f == radius) {
				LLVG_MATRIX_IMPL_translate(vglocalGlyphMatrix, advanceX + offset_x, baselineposition + advanceY  + offset_y);
			} else {
				float sign = (DIRECTION_CLOCK_WISE != direction) ? -1.f : 1.f;

				// Space characters joining bboxes at baseline
				float angleDegrees = 90 + __get_angle(advanceX + offset_x, radiusScaled)	+ __get_angle(charWidth / 2, radiusScaled);

				// Rotate to angle
				LLVG_MATRIX_IMPL_rotate(vglocalGlyphMatrix, sign * angleDegrees);

				// Translate left to center of bbox
				// Translate baseline over circle
				LLVG_MATRIX_IMPL_translate(vglocalGlyphMatrix, -charWidth / 2, -sign * radiusScaled);
			}

			// Draw the glyph
			FT_Error error = __render_glyph(face, glyph_index, INT_TO_FT_COLOR(color));
			if (0 != error) {
				MEJ_LOG_ERROR_MICROVG("Error while rendering glyphid %d: 0x%x, refer to fterrdef.h\n",glyph_index, error);
				continue; // ignore errors
			}

			// Compute advance to next glyph
			advanceX += charWidth;
			advanceX += (int)letterSpacingScaled;

			advanceY += advance_y;

			previous_glyph_index = glyph_index;
		}

		DISPLAY_VGLITE_start_operation(true);

		LLUI_DISPLAY_setDrawingStatus(DRAWING_RUNNING);
	}
	// else empty clip, nothing to draw
}
#endif // defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)
// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
