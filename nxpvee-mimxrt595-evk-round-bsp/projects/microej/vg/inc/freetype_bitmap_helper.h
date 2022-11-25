/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief Freetype bitmap helper implementation header for VectorGraphics Low Level API.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */
 
#ifndef FREETYPE_BITMAP_HELPER_H_
#define FREETYPE_BITMAP_HELPER_H_

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <freetype/internal/ftobjs.h>

#include <LLVG_FONT_PAINTER_impl.h>
#include <LLVG_FONT_impl.h>

/* Macros ------------------------------------------------------------------*/

/**
 * The divider coefficient used to pass from 1/64th of pixels unit to pixels unit.
 */
#define METRICS_DIVISOR	6

#define FT_HELPER_X_MIN 0 	/*!< The min start x coordinate to draw a glyph. */
#define FT_HELPER_Y_MIN 0 	/*!< The min start y coordinate to draw a glyph. */

#define FREETYPE_OK	0 					/*!< Error code returned when function execution is successful. */
#define FREETYPE_INTERNAL_ERROR	-1		/*!< Error code returned when an error occurred after a call of a freetype function. */
#define FREETYPE_NOT_IMPLEMENTED -2		/*!< Error code returned when the function is not implemented. */

#define TRANSFORM_MATRIX_POS_X 2	/*!< Position of the x coordinate inside the transform matrix when in 1D-array form. */
#define TRANSFORM_MATRIX_POS_Y 5	/*!< Position of the y coordinate inside the transform matrix when in 1D-array form. */

/* Shift for the pixel red component for argb format. */
#define FT_RED_SHIFT 16
/* Shift for the pixel green component for argb format. */
#define FT_GREEN_SHIFT 8
/* Shift for the pixel blue component for argb format. */
#define FT_BLUE_SHIFT 0


/* Private Type Definitions ---------------------------------------------- */

/**
 * @brief Data structure for pack all the variables required by freetype
 * handler.
 */
typedef struct{
	FT_Library library;
	FT_Face face;
	FT_Error error;
	FT_GlyphSlot  slot;
	FT_UInt glyph_index;
	FT_Renderer renderer;
	FT_Vector pen;
}Freetype_context_type;

typedef struct transform_matrix {
        float m[3][3];    /*! The 3x3 matrix itself, in [row][column] order. */
} transform_matrix_t;


/* Public Function Prototypes ------------------------------------------------ */

/*
 * @brief Returns the highest value between two integers.
 *
 * @param[in] X first integer to compare.
 * @param[in] Y second integer to compare.
 *
 * @return X if it has the highest value, Y is returned otherwise.
 */
#define max(X, Y) (((X) > (Y)) ? (X) : (Y))

/*
 * @brief Returns the lowest value between two integers.
 *
 * @param[in] X the first integer to compare.
 * @param[in] Y the second integer to compare.
 *
 * @return X if it has the lowest value, Y is returned otherwise.
 */
#define min(X, Y) (((X) < (Y)) ? (X) : (Y))

/**
 * @brief Prints a string in a buffer respecting the clipping area of the MicroUI Graphics Context.
 *
 * @param[in] gc Pointer to MicroUI GraphicsContext.
 * @param[in] freetype_context The Freetype context with font face data.
 * @param[in] string Pointer of the string.
 * @param[in] s_size Lenght of the string in characters.
 * @param[in] x The X coordinate in the frame-buffer pointer.
 * @param[in] y The Y coordinate in the frame-buffer pointer.
 * @param[in] color The 32 bits ARGB color of the string.
 * @param[in] alpha The opacity coefficient.
 * @param[in] size The character size.
 * @param[in] blend the blend mode to use
 * @param[in] letterSpacing the extra letter spacing to use
 *
 * @return FREETYPE_OK code if success, otherwise there is an error.
 */
int ft_helper_print_jstring_clipped(MICROUI_GraphicsContext* gc, Freetype_context_type *freetype_context, jchar* string, jint s_size, jint x, jint y, jint color, jint alpha, jfloat size, jint blend, jfloat letterSpacing);

/**
 * @brief Writes the current rendered glyph stored inside the Freetype context into the frame-buffer respecting the clipping area of the MicroUI Graphics Context.
 *
 * @param[in] gc Pointer to MicroUI GraphicsContext.
 * @param[in] freetype_context The Freetype context with glyph data.
 * @param[in] x The X coordinate in the frame-buffer pointer.
 * @param[in] y The Y coordinate in the frame-buffer pointer.
 * @param[in] color The 32 bits ARGB color of the glyph.
 * @param[in] alpha The opacity coefficient.
 */
static void ft_helper_write_to_framebuffer_clipped(MICROUI_GraphicsContext* gc, Freetype_context_type *freetype_context, jint x, jint y, jint color, jint alpha);


/**
 * @brief Frees all Freetype data context.
 *
 * @param[in] Freetype context of the job.
 */
void ft_helper_free(Freetype_context_type *freetype_context);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
	}
#endif

#endif // FREETYPE_BITMAP_HELPER_H_
