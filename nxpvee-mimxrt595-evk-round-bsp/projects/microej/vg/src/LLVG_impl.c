/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: basic implementation
 * of matrix APIs.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include <LLVG_impl.h>

#include "microvg_configuration.h"
#include "microvg_helper.h"
#include "microvg_font_freetype.h"

// See the header file for the function documentation
void LLVG_IMPL_initialize(void) {
	MICROVG_HELPER_initialize();

#if defined VG_FEATURE_FONT && \
	(defined VG_FEATURE_FONT_FREETYPE_VECTOR || defined VG_FEATURE_FONT_FREETYPE_BITMAP) && \
	(VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR || VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

	MICROVG_FONT_FREETYPE_initialize();

#endif
}
