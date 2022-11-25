/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroVG library low level API over VG-Lite.
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <string.h>

#include <LLVG_impl.h>
#include <LLVG_GRADIENT_impl.h>
#include <LLVG_MATRIX_impl.h>

#include "microvg_gradient.h"
#include "microvg_helper.h"
#include "microvg_font_freetype.h"
#include "microvg_vglite_helper.h"
#include "vg_lite.h"
#include "color.h"
#include "display_vglite.h"
#include "bsp_util.h"

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

#if defined (VG_FEATURE_GRADIENT) && defined (VG_FEATURE_GRADIENT_FULL) && (VG_FEATURE_GRADIENT != VG_FEATURE_GRADIENT_FULL)
#error "This implementation is only compatible with VG_FEATURE_GRADIENT_FULL"
#endif

#define ALPHA_SHIFT 24
#define VGLITE_GRADIENT_SIZE 256

// -----------------------------------------------------------------------------
// microvg_vglite_helper.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
vg_lite_error_t MICROVG_VGLITE_HELPER_to_vg_lite_gradient(vg_lite_linear_gradient_t* gradient, jint* gradientData, jfloat* matrix, jfloat* globalMatrix, jint globalAlpha) {
	MICROVG_GRADIENT_HEADER_t* header = (MICROVG_GRADIENT_HEADER_t*)gradientData;
	jint count = header->count;
	vg_lite_error_t ret ;
	if (count <= VLC_MAX_GRAD) {

		(void)memset(gradient, 0, sizeof(vg_lite_linear_gradient_t));

		// calculate gradient's matrix (scale must be done after the rotate, otherwise the rotation is eccentric (or put
		// the same ratio for scale x and y)).
		jfloat* mapped_gradient_matrix = MAP_VGLITE_GRADIENT_MATRIX(gradient);
		LLVG_MATRIX_IMPL_copy(mapped_gradient_matrix, globalMatrix);
		LLVG_MATRIX_IMPL_concatenate(mapped_gradient_matrix, matrix);
		LLVG_MATRIX_IMPL_translate(mapped_gradient_matrix, header->x, header->y);
		LLVG_MATRIX_IMPL_rotate(mapped_gradient_matrix, header->angle);
		LLVG_MATRIX_IMPL_scale(mapped_gradient_matrix, header->length / VGLITE_GRADIENT_SIZE, 1);

		// update vg lite colors
		uint32_t* colors_addr = &(((uint32_t*)header)[header->colors_offset]);
		uint32_t* positions_addr = &(((uint32_t*)header)[header->positions_offset]);
		// cppcheck-suppress [misra-c2012-18.8] the size is a define
		uint32_t colors_temp[VLC_MAX_GRAD];
		if (0xff != globalAlpha) {
			for (int i = 0; i < count; i++) {
				colors_temp[i] = MICROVG_HELPER_apply_alpha(*colors_addr, globalAlpha);
				++colors_addr;
			}
		}
		else {
			for (int i = 0; i < count; i++)	{
				colors_temp[i] = *colors_addr;
				++colors_addr;
			}
		}

		// this call allocates in VGLite buffer
		ret = vg_lite_init_grad(gradient);
		if (VG_LITE_SUCCESS == ret) {

			// GPU displays ABGR instead of ARGB and vice versa
			gradient->image.format = VG_LITE_RGBA8888;

			ret = vg_lite_set_grad(gradient, count, (uint32_t *) colors_temp, positions_addr);
		}
	}
	else {
		// too many positions
		ret = VG_LITE_INVALID_ARGUMENT;
	}
	return ret;
}

// See the header file for the function documentation
vg_lite_fill_t MICROVG_VGLITE_HELPER_get_fill_rule(jint fill_type) {
	vg_lite_fill_t ret;
	switch (fill_type) {
	case LLVG_FILLTYPE_EVEN_ODD:
		ret = VG_LITE_FILL_EVEN_ODD;
		break;
	case LLVG_FILLTYPE_WINDING:
	default:
		ret = VG_LITE_FILL_NON_ZERO;
		break;
	}
	return ret;
}


// See the header file for the function documentation
vg_lite_blend_t MICROVG_VGLITE_HELPER_get_blend(jint blend) {
	vg_lite_blend_t ret;
	switch (blend) {
	default:
	case LLVG_BLEND_SRC:
		ret = VG_LITE_BLEND_NONE;
		break;
	case LLVG_BLEND_SRC_OVER:
		ret = VG_LITE_BLEND_SRC_OVER;
		break;
	case LLVG_BLEND_DST_OVER:
		ret = VG_LITE_BLEND_DST_OVER;
		break;
	case LLVG_BLEND_SRC_IN:
		ret = VG_LITE_BLEND_SRC_IN;
		break;
	case LLVG_BLEND_DST_IN:
		ret = VG_LITE_BLEND_DST_IN;
		break;
	case LLVG_BLEND_SCREEN:
		ret = VG_LITE_BLEND_SCREEN;
		break;
	case LLVG_BLEND_MULTIPLY:
		ret = VG_LITE_BLEND_MULTIPLY;
		break;
	}
	return ret;
}

// See the header file for the function documentation
bool MICROVG_VGLITE_HELPER_enable_vg_lite_scissor(MICROUI_GraphicsContext* gc)
{
    int32_t width =  gc->clip_x2 - gc->clip_x1 + 1;
    int32_t height =  gc->clip_y2 - gc->clip_y1 + 1;

    bool ret;
    if ((width > 0) && (height > 0))
    {
        vg_lite_enable_scissor();
        vg_lite_set_scissor(gc->clip_x1, gc->clip_y1, width, height);
        ret = true;
    }
    else {
	    // drawing is useless
    	LLUI_DISPLAY_setDrawingStatus(DRAWING_DONE);
    	ret = false;
    }
    return ret;
}

// -----------------------------------------------------------------------------
// microvg_gradient.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
float MICROVG_GRADIENT_get_gradient_scale_size(void) {
	return (float)VGLITE_GRADIENT_SIZE;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
