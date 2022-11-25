/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: helper to implement library natives
* methods.
* @author MicroEJ Developer Team
* @version 2.0.0
*/

#if !defined MICROVG_HELPER_H
#define MICROVG_HELPER_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------


// cppcheck-suppress [misra-c2012-21.6] required to use "printf"
#include <stdio.h>

#include <sni.h>

#include "mej_log.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#if defined MEJ_LOG_INFO_LEVEL && defined MEJ_LOG_MICROVG
#define MEJ_LOG_INFO_MICROVG(fmt, ...) MEJ_LOG(INFO,MICROVG,fmt, ##__VA_ARGS__ )
#else
#define MEJ_LOG_INFO_MICROVG(fmt, ...)
#endif

// Errors should always be printed
#define MEJ_LOG_ERROR_MICROVG(fmt, ...) MEJ_LOG(ERROR,MICROVG,fmt, ##__VA_ARGS__ )

/**
 * @brief Set this define to monitor freetype heap evolution.
 *        It needs MEJ_LOG_MICROVG  and MEJ_LOG_INFO_LEVEL defines
 *        to print the heap logs.
 */
//#define MICROVG_MONITOR_HEAP

/**
 * @brief NULL Gradient value
 */
#define MICROVG_HELPER_NULL_GRADIENT 0

/**
 * @brief Freetype supplementary flag for complex layout
 *        Uses a free bit in freetype face flags to convey the complex layout mode
 *        information with the freetype face.
 *        freetype.h must be checked on freetype update to ensure that this bit is
 *        still free.
 */
#define FT_FACE_FLAG_COMPLEX_LAYOUT  (((uint32_t)1) << 31)

#ifndef M_PI
#define M_PI 3.1415926535
#endif

#define RAD_TO_DEG(r) ((r) * (180.0f / M_PI))
#define DEG_TO_RAD(d) (((d) * M_PI) / 180.0f)

#define JFLOAT_TO_UINT32_t(f) (*(uint32_t*)&(f))
#define UINT32_t_TO_JFLOAT(i) (*(float*)&(i))

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Initializes the helper
 */
void MICROVG_HELPER_initialize(void);

/**
 * @brief Gets the next UTF character from a text buffer.
 *
 * @param[in] text: text buffer encoded in UTF16 where to read UTF character.
 * @param[in] length: lenght of the text buffer.
 * @param[in/out] offset: offset in the text buffer where to read UTF character. Updated
 *    to the next character position.
 *
 * @return The decoded UTF character.
 */
int MICROVG_HELPER_get_utf(unsigned short *text, int length, int *offset);


/*
 * @brief Configures the font layouter with a font and a text
 *
 * @param[in] faceHandle: handle on font face.
 * @param[in] text: text buffer encoded in UTF16 where to read UTF character.
 * @param[in] length: text buffer length.
 *
 */
void MICROVG_HELPER_layout_configure(int faceHandle, unsigned short *text, int length);

/*
 * @brief Loads the next layouted glyph and gets index and positions.
 *
 * @param[out] glyph_idx: next glyph index.
 * @param[out] x_advance: the horizontal advance to add to the cursor position after drawing the glyph.
 * @param[out] y_advance: the vertical advance to add to thecursor after drawing the glyph.
 * @param[out] x_offset: the hozizontal offset of the glyph, does not affect the cursor position.
 * @param[out] y_offset: the vertical offset of the glyph, does not affect the cursor position.
 *
 * @return true if a glyph is available otherwise false.
 */
bool MICROVG_HELPER_layout_load_glyph(int *glyph_idx, int *x_advance, int *y_advance, int *x_offset, int *y_offset);

/*
 * @brief Checks if the matrix is null. In that case, returns an identity matrix.
 * This allows to prevent to make some checks on the matrix in the algorithms.
 *
 * The identity matrix can be used several times by the same algorithm. The caller
 * must not modify it (read-only matrix).
 *
 * @param[in] the matrix to check
 *
 * @return the matrix or an identity matrix
 */
jfloat* MICROVG_HELPER_check_matrix(jfloat* matrix);

/*
 * @brief Applies the global opacity on given color.
 *
 * @param[in] color: the 32-bit color.
 * @param[in] alpha: the opacity.
 *
 * @return the new color.
 */
uint32_t MICROVG_HELPER_apply_alpha(uint32_t color, uint32_t alpha) ;

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_HELPER_H
