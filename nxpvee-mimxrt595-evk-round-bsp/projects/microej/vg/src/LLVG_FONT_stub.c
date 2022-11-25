/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: implementation over FreeType
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#ifndef VG_FEATURE_FONT

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLVG_FONT_impl.h>
#include <LLVG_FONT_PAINTER_impl.h>

// -----------------------------------------------------------------------------
// LLVG_FONT_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_FONT_IMPL_load_font(jchar* font_name, jboolean complex_layout) {
	(void)font_name;
	(void)complex_layout;
	return 0;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_string_width(jchar* text, jint faceHandle, jfloat size, jfloat letterSpacing) {
	(void)text;
	(void)faceHandle;
	(void)size;
	(void)letterSpacing;
	return 0;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_string_height(jchar* text, jint faceHandle, jfloat size) {
	(void)text;
	(void)faceHandle;
	(void)size;
	return 0;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_get_baseline_position(jint faceHandle, jfloat size) {
	(void)faceHandle;
	(void)size;
	return 0;
}

// See the header file for the function documentation
jfloat LLVG_FONT_IMPL_get_height(jint faceHandle, jfloat size) {
	(void)faceHandle;
	(void)size;
	return 0;
}

// See the header file for the function documentation
void LLVG_FONT_IMPL_dispose(jint faceHandle) {
	(void)faceHandle;
}
// -----------------------------------------------------------------------------
// LLVG_FONT_PAINTER_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing){
	(void)gc;
	(void)text;
	(void)faceHandle;
	(void)size;
	(void)x;
	(void)y;
	(void)matrix;
	(void)alpha;
	(void)blend;
	(void)letterSpacing;
        
    return(0);
}

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jfloat x, jfloat y,  jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jint *gradientData, jfloat *gradientMatrix){
	(void)gc;
	(void)text;
	(void)faceHandle;
	(void)size;
	(void)x;
	(void)y;
	(void)matrix;
	(void)alpha;
	(void)blend;
	(void)letterSpacing;
	(void)gradientData;
	(void)gradientMatrix;

    return(0);
}

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction){
	(void)gc;
	(void)text;
	(void)faceHandle;
	(void)size;
	(void)x;
	(void)y;
	(void)matrix;
	(void)alpha;
	(void)blend;
	(void)letterSpacing;
	(void)radius;
	(void)direction;

    return(0);
}

// See the header file for the function documentation
jint LLVG_FONT_PAINTER_IMPL_draw_string_on_circle_gradient(MICROUI_GraphicsContext* gc, jchar* text, jint faceHandle, jfloat size,  jint x, jint y, jfloat* matrix, jint alpha, jint blend, jfloat letterSpacing, jfloat radius, jint direction, jint *gradientData, jfloat *gradientMatrix){
	(void)gc;
	(void)text;
	(void)faceHandle;
	(void)size;
	(void)x;
	(void)y;
	(void)matrix;
	(void)alpha;
	(void)blend;
	(void)letterSpacing;
	(void)radius;
	(void)direction;
	(void)gradientData;
	(void)gradientMatrix;

    return(0);
}

// See the header file for the function documentation
void LLVG_FONT_IMPL_set_complex_layout(bool enabled){
	(void)enabled;
}

// See the header file for the function documentation
bool LLVG_FONT_IMPL_has_complex_layouter(void){
	return false;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_FONT
