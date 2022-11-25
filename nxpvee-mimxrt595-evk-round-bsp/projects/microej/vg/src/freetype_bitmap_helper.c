/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief FREETYPE helper implementation.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#if defined VG_FEATURE_FONT && defined VG_FEATURE_FONT_FREETYPE_BITMAP && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)


// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_DISPLAY_impl.h>
#include <LLVG_FONT_impl.h>

#include "microvg_helper.h"
#include "freetype_bitmap_helper.h"

// -----------------------------------------------------------------------------
// Macros
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/**
 * @brief Stores the actual image width in use, calculate before printing string, that is by default the screen's width.
 */
static uint32_t image_width;

/**
 * @brief Stores the actual image height in use, calculate before printing string, that is by default the screen's height.
 */
static uint32_t image_height;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------


int ft_helper_print_jstring_clipped(MICROUI_GraphicsContext* gc, Freetype_context_type *freetype_context, jchar* string, jint s_size, jint x, jint y, jint color, jint alpha, jfloat size, jint blend, jfloat letterSpacing){
	FT_UInt previous = 0; // previous glyph index for kerning
	FT_Bool use_kerning;
	int res = FREETYPE_OK;
	
	// Misra unused variable, to be checked
	(void)size;
	(void)blend;

	image_width = LLUI_DISPLAY_getStrideInPixels(&gc->image);
	image_height = gc->image.height;
	use_kerning = FT_HAS_KERNING(freetype_context->face);
	freetype_context->slot = freetype_context->face->glyph;

	uint32_t local_x = x;
	int c=0;
	for(c=0;c<s_size;c++){
		freetype_context->glyph_index = FT_Get_Char_Index( freetype_context->face, string[c] );
		if (!freetype_context->glyph_index){
			MEJ_LOG_INFO_MICROVG("Warning bad glyph_index = %d \n",freetype_context->glyph_index);
		}

		freetype_context->error = FT_Load_Glyph (freetype_context->face, freetype_context->glyph_index,  FT_LOAD_RENDER);
		if(FT_ERR(Ok) != freetype_context->error){
			MEJ_LOG_INFO_MICROVG("error while loading glyph, errno=%d \n",freetype_context->error);
			res = FREETYPE_INTERNAL_ERROR;
			break;
		}

		// metrics structure retrieved in 64th pixel unit
		uint32_t bearing_x = freetype_context->face->glyph->metrics.horiBearingX >> METRICS_DIVISOR;
		uint32_t bearing_y = freetype_context->face->glyph->metrics.horiBearingY >> METRICS_DIVISOR;
		uint32_t advance_aux = freetype_context->face->glyph->advance.x >> METRICS_DIVISOR;

		if (use_kerning && previous && freetype_context->glyph_index){
			FT_Vector  delta;
			FT_Get_Kerning(freetype_context->face, previous, freetype_context->glyph_index, FT_KERNING_DEFAULT, &delta);
			local_x += (delta.x >> METRICS_DIVISOR);
		}

		ft_helper_write_to_framebuffer_clipped(gc, freetype_context, (local_x + bearing_x), (y - bearing_y), color, alpha);
		
		local_x += advance_aux + (uint32_t) letterSpacing;

		if (local_x > (image_width - (uint32_t) FT_HELPER_X_MIN - (uint32_t) freetype_context->slot->bitmap.width)){
			MEJ_LOG_INFO_MICROVG("\n FT_HELPER_OUT_RIGHT_SCREEN_LIMIT\n");
		}
		previous = freetype_context->glyph_index;
	}
	return res;
}

static void ft_helper_write_to_framebuffer_clipped(MICROUI_GraphicsContext* gc, Freetype_context_type *freetype_context, jint x, jint y, jint color, jint alpha){
	jint n_rows = freetype_context->slot->bitmap.rows;
	jint n_cols = freetype_context->slot->bitmap.width;
	jint original_foreground_color = gc->foreground_color;

	// Misra unused variable, to be checked
	(void)alpha;

    if( (0 > gc->clip_x1) ||  (image_width < gc->clip_x1) || (0 > gc->clip_x2) ||  (image_width < gc->clip_x2) ||
		(0 >	gc->clip_y1) ||  (image_height < gc->clip_y1) ||	(0 >	gc->clip_y2) ||  (image_height < gc->clip_y2)){
    	MEJ_LOG_INFO_MICROVG("The graphics context clip is out of the screen! \n");
    	MEJ_LOG_INFO_MICROVG("clip_x1 = %d ; clip_x2 = %d ; clip_y1 = %d ; clip_y2 = %d ; \n",gc->clip_x1, gc->clip_x2,gc->clip_y1, gc->clip_y2);
    } else {

		// Check if we have an intersection between the glyph area and the Graphics Context's area
		jint intersec_xmin = max(gc->clip_x1, x);
		jint intersec_ymin = max(gc->clip_y1, y);
		jint intersec_xmax = min(gc->clip_x2, x+n_cols-1);
		jint intersec_ymax = min(gc->clip_y2, y+n_rows-1);
		jint intersec_width = intersec_xmax - intersec_xmin +1;
		jint intersec_height = intersec_ymax - intersec_ymin +1;

		if((0 >= intersec_width) || (0 >= intersec_height)){ //No intersection we leave the function
			MEJ_LOG_INFO_MICROVG("\n Glyph out of the Graphics Context's area!");
		} else {
			jint y_start_bitmap = intersec_ymin-y;
			jint x_start_bitmap = intersec_xmin-x;
			for (jint y_bitmap = y_start_bitmap; y_bitmap < (y_start_bitmap + intersec_height); ++y_bitmap) {
				for (jint x_bitmap = x_start_bitmap; x_bitmap < (x_start_bitmap + intersec_width); ++x_bitmap) {
					if(0x00 != freetype_context->slot->bitmap.buffer[(y_bitmap * n_cols) + x_bitmap]){
						FT_Bitmap *bitmap = &freetype_context->slot->bitmap;
						uint32_t pix_gray_color = bitmap->buffer[(y_bitmap * bitmap->width) + x_bitmap];
						uint32_t background_argb = LLUI_DISPLAY_readPixel(&gc->image, intersec_xmin + x_bitmap, intersec_ymin + y_bitmap);
						uint32_t blended_color = LLUI_DISPLAY_blend(color, background_argb, pix_gray_color);
						gc->foreground_color = blended_color;
						UI_DRAWING_writePixel(gc, intersec_xmin + x_bitmap, intersec_ymin + y_bitmap);
					}else{
						// Black pixel is for background, we let the pixel buffer as it is.
					}
				}
			}
			// Set back configured color
			gc->foreground_color = original_foreground_color;
		}
    }
}

void ft_helper_free(Freetype_context_type *freetype_context){
	FT_Done_Face    ( freetype_context->face );
    FT_Done_FreeType( freetype_context->library );
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // #if (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)
