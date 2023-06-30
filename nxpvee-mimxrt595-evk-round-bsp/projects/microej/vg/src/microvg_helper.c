/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: helper to implement library natives
* methods.
* @author MicroEJ Developer Team
* @version 2.0.0
*/

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLVG_MATRIX_impl.h>
#include <freetype/internal/ftobjs.h>

#include "microvg_helper.h"
#include "microvg_configuration.h"

// -----------------------------------------------------------------------------
// Configuration Sanity Check
// -----------------------------------------------------------------------------

/**
 * Sanity check between the expected version of the configuration and the actual version of
 * the configuration.
 * If an error is raised here, it means that a new version of the CCO has been installed and
 * the configuration microvg_configuration.h must be updated based on the one provided
 * by the new CCO version.
 */

#if !defined MICROVG_CONFIGURATION_VERSION
	#error "Undefined MICROVG_CONFIGURATION_VERSION, it must be defined in microvg_configuration.h"
#endif

#if defined MICROVG_CONFIGURATION_VERSION && MICROVG_CONFIGURATION_VERSION != 1
	#error "Version of the configuration file microvg_configuration.h is not compatible with this implementation."
#endif

#if defined VG_FEATURE_FONT_COMPLEX_LAYOUT
#include "hb.h"
#include "hb-ft.h"
#endif

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

#define MIN_HIGH_SURROGATE  ((unsigned short)0xD800)
#define MAX_HIGH_SURROGATE  ((unsigned short)0xDBFF)
#define MIN_LOW_SURROGATE   ((unsigned short)0xDC00)
#define MAX_LOW_SURROGATE   ((unsigned short)0xDFFF)
#define MIN_SUPPLEMENTARY_CODE_POINT 0x010000

#define GET_NEXT_CHARACTER(t,l,o) ((o) >= (l) ? (unsigned short)0 : (t)[o])

#ifdef VG_FEATURE_FONT_COMPLEX_LAYOUT
#define IS_SIMPLE_LAYOUT (!(face->face_flags & FT_FACE_FLAG_COMPLEX_LAYOUT))
#else
#define IS_SIMPLE_LAYOUT true
#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

static jfloat g_identity_matrix[LLVG_MATRIX_SIZE];

static FT_Face face;

// Freetype layout variables
static unsigned short *current_text;
static unsigned int current_length;
static int current_offset;
static FT_UInt previous_glyph_index; // previous glyph index for kerning

#if defined VG_FEATURE_FONT_COMPLEX_LAYOUT
// Harfbuzz layout variables
static hb_glyph_info_t *glyph_info;
static hb_glyph_position_t  *glyph_pos;
static unsigned int glyph_count;
static int current_glyph;
static hb_buffer_t *buf;
#endif 
// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void MICROVG_HELPER_initialize(void) {
	LLVG_MATRIX_IMPL_identity(g_identity_matrix);
}

// See the header file for the function documentation
int MICROVG_HELPER_get_utf(unsigned short *textCharRam, int length, int *offset) {

	unsigned short highPart = GET_NEXT_CHARACTER(textCharRam, length, *offset);
	int ret = 0;

	if (((highPart >= MIN_HIGH_SURROGATE) && (highPart <= MAX_HIGH_SURROGATE))
			&& (*offset < (length - 1))) {

		unsigned short lowPart = GET_NEXT_CHARACTER(textCharRam, length, *(offset) + 1);

		if ((lowPart >= MIN_LOW_SURROGATE) && (lowPart <= MAX_LOW_SURROGATE)) {
			*offset += 2;

			ret = 0;
			ret += ((int)highPart - (int)MIN_HIGH_SURROGATE);
			ret <<= (int)10;
			ret += ((int)lowPart - (int)MIN_LOW_SURROGATE);
			ret += (int)MIN_SUPPLEMENTARY_CODE_POINT;
		}
	}
	else {
		*offset += 1;

		// standard character or missing low part
		ret = 0x0000FFFF & (int)highPart;
	}

	return ret;
}


// See the header file for the function documentation
void MICROVG_HELPER_layout_configure(int faceHandle, unsigned short *text, int length){
	face = (FT_Face) faceHandle;
	// For Misra rule 2.7
	(void)text;
	(void)length;

	if(IS_SIMPLE_LAYOUT){
		// Freetype variables initialisation
		current_text = text;
		current_length = length;
		current_offset = 0;
		previous_glyph_index = 0;
	}
	else {
#if defined VG_FEATURE_FONT_COMPLEX_LAYOUT
		static hb_font_t *hb_font;
		static jint current_faceHandle = 0;
		// load font in Harfbuzz only when faceHandle changes
		if(faceHandle != current_faceHandle){
			if(0 != current_faceHandle){
				hb_font_destroy(hb_font);
			}
			// FT_Set_Pixel_Sizes() must be called before hb_ft_font_create() see issue M0092MEJAUI-2643
			FT_Set_Pixel_Sizes (face, 0, face->units_per_EM); /* set character size */
			hb_font = hb_ft_font_create(face, NULL);
			current_faceHandle = faceHandle;
		}

		buf = hb_buffer_create();
		hb_buffer_add_utf16(buf, (const uint16_t *)text, length, 0, -1);

		hb_buffer_guess_segment_properties(buf);

		hb_shape(hb_font, buf, NULL, 0);

		glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
		glyph_pos  = hb_buffer_get_glyph_positions(buf, &glyph_count);

		current_glyph = 0;
#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT
	}
}

// See the header file for the function documentation
bool MICROVG_HELPER_layout_load_glyph(int *glyph_idx, int *x_advance, int *y_advance, int *x_offset, int *y_offset){
	// Initiate return value with default values
	*glyph_idx = 0;
	*x_advance = 0;
	*y_advance = 0;
	*x_offset = 0;
	*y_offset = 0;

	bool ret = false;


	if(IS_SIMPLE_LAYOUT){
		// Freetype layout
		FT_ULong  next_char = MICROVG_HELPER_get_utf(current_text, current_length, &current_offset);
		if(0 != next_char){
			FT_UInt glyph_index = FT_Get_Char_Index(face, next_char);

			int error = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
			if(FT_ERR(Ok) != error) {
				MEJ_LOG_ERROR_MICROVG("Error while loading glyphid %d: 0x%x, refer to fterrdef.h\n",glyph_index, error);
			}

			*x_advance = face->glyph->advance.x;

			// Compute Kerning
			if (FT_HAS_KERNING(face) && previous_glyph_index && glyph_index){
				FT_Vector  delta;
				FT_Get_Kerning(face, previous_glyph_index, glyph_index, FT_KERNING_UNSCALED, &delta);

				*x_offset = delta.x;
				*x_advance += delta.x;
			}

			previous_glyph_index = glyph_index;

			*glyph_idx = glyph_index;

			ret = true;
		}
	}
	else {
#if defined VG_FEATURE_FONT_COMPLEX_LAYOUT
		// Harfbuzz layout
		if(((unsigned int) 0) != glyph_count){

			*glyph_idx = glyph_info[current_glyph].codepoint;
			*x_advance = glyph_pos[current_glyph].x_advance/64;
			*y_advance = glyph_pos[current_glyph].y_advance/64;
			*x_offset  = glyph_pos[current_glyph].x_offset/64;
			*y_offset  = glyph_pos[current_glyph].y_offset/64;

			glyph_count --;
			current_glyph ++;

			// Load glyph
			int error = FT_Load_Glyph(face, *glyph_idx, FT_LOAD_NO_SCALE);
			if(FT_ERR(Ok) != error) {
				MEJ_LOG_ERROR_MICROVG("Error while loading glyphid %d: 0x%x, refer to fterrdef.h\n", *glyph_idx, error);
			}

			ret = true;
		} else {
			hb_buffer_destroy(buf);
			ret = false;
		}
#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT
	}

	return ret;
}

// See the header file for the function documentation
jfloat* MICROVG_HELPER_check_matrix(jfloat* matrix) {
	return (NULL == matrix) ? g_identity_matrix : matrix;
}

// See the header file for the function documentation
uint32_t MICROVG_HELPER_apply_alpha(uint32_t color, uint32_t alpha) {
	uint32_t color_alpha = (((color >> 24) & (uint32_t)0xff) * alpha) / (uint32_t)255;
	return (color &  (uint32_t)0xffffff) | (color_alpha << 24);
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
