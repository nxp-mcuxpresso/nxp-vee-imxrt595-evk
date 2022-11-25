/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ VectorGraphics library low level API
* @author MicroEJ Developer Team
* @version 2.0.0
*/

#include "microvg_configuration.h"

#if defined (VG_FEATURE_FONT) && defined (VG_FEATURE_FONT_FREETYPE_BITMAP) && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <math.h>
#include "ft2build.h"

#include <LLVG_impl.h>
#include <LLVG_PATH_impl.h>
#include <LLVG_GRADIENT_impl.h>
#include <LLVG_MATRIX_impl.h>
#include <LLVG_FONT_impl.h>
#include <LLVG_FONT_PAINTER_impl.h>
#include <sni.h>

#include "microvg_font_freetype.h"
#include "microvg_helper.h"
#include "freetype_bitmap_helper.h"
#include "bsp_util.h"

// -----------------------------------------------------------------------------
// Macros
// ---------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// Extern variables
// -----------------------------------------------------------------------------

extern FT_Library library;
extern FT_Renderer renderer;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/**
 * @brief Sub-function used to draw a string in the frame buffer.
 *
 * @param[in] gc Pointer to MicroUI GraphicsContext.
 * @param[in] face The font face reference.
 * @param[in] size The height of the text in pixels.
 * @param[in] text The pointer of the string to draw.
 * @param[in] length The number of characters to draw from the array.
 * @param[in] x The x coordinate of the anchor top-left point.
 * @param[in] y The y coordinate of the anchor top-left point.
 * @param[in] alpha The opacity coefficient.
 * @param[in] blend the blend mode to use
 * @param[in] letterSpacing the extra letter spacing to use
 * @param[in] callback A function pointer that must points to the function that called this one.
 *
 *
 */
void __draw_string(MICROUI_GraphicsContext* gc, FT_Face face, jfloat size, jchar* text, jint length, jint x, jint y, jint alpha, jint blend, jfloat letterSpacing, SNI_callback callback);


// -----------------------------------------------------------------------------
// LLVG_FONT_PAINTER API functions
// -----------------------------------------------------------------------------


jint LLVG_FONT_PAINTER_IMPL_draw_string(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing){
	FT_Face face = (FT_Face) faceHandle;
	int length = SNI_getArrayLength(text);

	// add matrix translation to x,y variables
	int local_x = (int) (x + matrix[2]);
	int local_y = (int) (y + matrix[5]);

    __draw_string(gc, face, size, text, length, (jint)local_x, (jint)local_y, alpha, blend, letterSpacing, (SNI_callback)&LLVG_FONT_PAINTER_IMPL_draw_string);

    return (jint)LLVG_SUCCESS;
}


jint LLVG_FONT_PAINTER_IMPL_draw_string_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jint *gradientData, jfloat *gradientMatrix){
	SNI_throwNativeException(FREETYPE_NOT_IMPLEMENTED, "Native not implemented");

	// Avoid unusued parameter in misra cppcheck
	(void)(gc);
	(void)(text);
	(void)(faceHandle);
	(void)(size);
	(void)(x);
	(void)(y);
	(void)(matrix);
	(void)(alpha);
	(void)(blend);
	(void)(letterSpacing);
	(void)(gradientData);
	(void)(gradientMatrix);

	 return (jint)LLVG_DATA_INVALID;
}

jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction){
	SNI_throwNativeException(FREETYPE_NOT_IMPLEMENTED, "Native not implemented");

	// Avoid unusued parameter in misra cppcheck
	(void)(gc);
	(void)(text);
	(void)(faceHandle);
	(void)(size);
	(void)(x);
	(void)(y);
	(void)(matrix);
	(void)(alpha);
	(void)(blend);
	(void)(letterSpacing);
	(void)(radius);
	(void)(direction);

	return (jint)LLVG_DATA_INVALID;
}

jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction, jint *gradientData, jfloat *gradientMatrix){
	SNI_throwNativeException(FREETYPE_NOT_IMPLEMENTED, "Native not implemented");

	// Avoid unusued parameter in misra cppcheck
	(void)(gc);
	(void)(text);
	(void)(faceHandle);
	(void)(size);
	(void)(x);
	(void)(y);
	(void)(matrix);
	(void)(alpha);
	(void)(blend);
	(void)(letterSpacing);
	(void)(radius);
	(void)(direction);
	(void)(gradientData);
	(void)(gradientMatrix);

	return (jint)LLVG_DATA_INVALID;
}


// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

void __draw_string(MICROUI_GraphicsContext* gc, FT_Face face, jfloat size, jchar* text, jint length, jint x, jint y, jint alpha, jint blend, jfloat letterSpacing, SNI_callback callback){
	Freetype_context_type local_freetype_context;

	 if(!LLUI_DISPLAY_requestDrawing(gc, callback)){
	   	MEJ_LOG_INFO_MICROVG("request drawing declined \n");

	} else {

		local_freetype_context.library = library;
		local_freetype_context.renderer = renderer;
		local_freetype_context.face = face;

		int char_height = (int)size;
		local_freetype_context.error = FT_Set_Pixel_Sizes( local_freetype_context.face, 0, char_height);
		if(FT_ERR(Ok) != local_freetype_context.error){
			MEJ_LOG_INFO_MICROVG("error while setting font face size: %d\n", local_freetype_context.error);
		}

		// Update the y coordinate to match the wanted start position with the top-left of the string
		jint y_adapt = (jint) LLVG_FONT_IMPL_get_baseline_position((jint)local_freetype_context.face, size);
		MEJ_LOG_INFO_MICROVG("y_adapt = %d \n", y_adapt);

		jint font_color;
		font_color = (gc->foreground_color & 0x00FFFFFF) + (alpha << 24);

		(void)ft_helper_print_jstring_clipped(gc, &local_freetype_context, text, length, x, y + y_adapt, font_color, alpha, size, blend, letterSpacing);
		if(!LLUI_DISPLAY_setDrawingLimits(x, y, gc->clip_x2, y+char_height)){
			MEJ_LOG_INFO_MICROVG("Warning, drawing area out of the given graphics context!\n");
		}
		LLUI_DISPLAY_setDrawingStatus(DRAWING_DONE);
	}
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // #if (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)
