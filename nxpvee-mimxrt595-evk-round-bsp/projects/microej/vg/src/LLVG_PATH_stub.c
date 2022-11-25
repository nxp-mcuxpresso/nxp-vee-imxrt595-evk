/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroVG library low level API: implementation of path.
 *
 * This implementation uses a 32-bit "integer" value to store a path command and
 * a 32-bit "float" value to store each command parameter.
 *
 * The encoding can be overrided, see "[optional]: weak functions" in "microvg_path.h"
 *
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#ifndef VG_FEATURE_PATH

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLVG_PATH_impl.h>
#include <LLVG_PATH_PAINTER_impl.h>

// -----------------------------------------------------------------------------
// LLVG_PATH_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_PATH_IMPL_initializePath(jbyte* jpath, jint length) {
	(void)jpath;
	(void)length;
	return LLVG_SUCCESS;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand1(jbyte* jpath, jint length, jint cmd, jfloat x, jfloat y) {
	(void)jpath;
	(void)length;
	(void)cmd;
	(void)x;
	(void)y;
	return LLVG_SUCCESS;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand2(jbyte* jpath, jint length, jint cmd, jfloat x1, jfloat y1, jfloat x2,
		jfloat y2) {
	(void)jpath;
	(void)length;
	(void)cmd;
	(void)x1;
	(void)y1;
	(void)x2;
	(void)y2;
	return LLVG_SUCCESS;
}

// See the header file for the function documentation
jint LLVG_PATH_IMPL_appendPathCommand3(jbyte* jpath, jint length, jint cmd, jfloat x1, jfloat y1, jfloat x2,
		jfloat y2, jfloat x3, jfloat y3) {
	(void)jpath;
	(void)length;
	(void)cmd;
	(void)x1;
	(void)y1;
	(void)x2;
	(void)y2;
	(void)x3;
	(void)y3;
	return LLVG_SUCCESS;
}

// See the header file for the function documentation
void LLVG_PATH_IMPL_reopenPath(jbyte* jpath) {
	// nothing to do
	(void)jpath;
}

// -----------------------------------------------------------------------------
// LLVG_PAINTER_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_PATH_PAINTER_IMPL_drawPath(MICROUI_GraphicsContext* gc, jbyte* pathData, jint x, jint y, jfloat* matrix, jint fillRule, jint blend, jint color){
	(void)gc;
	(void)pathData;
	(void)x;
	(void)y;
	(void)matrix;
	(void)fillRule;
	(void)blend;
	(void)color;
	return LLVG_SUCCESS;
}

// See the header file for the function documentation
jint LLVG_PATH_PAINTER_IMPL_drawGradient(MICROUI_GraphicsContext* gc, jbyte* pathData, jint x, jint y, jfloat* matrix, jint fillRule, jint blend, jint* gradientData, jfloat* gradientMatrix, jint alpha){
	(void)gc;
	(void)pathData;
	(void)x;
	(void)y;
	(void)matrix;
	(void)fillRule;
	(void)blend;
	(void)gradientData;
	(void)gradientMatrix;
	(void)alpha;
	return LLVG_SUCCESS;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_PATH
