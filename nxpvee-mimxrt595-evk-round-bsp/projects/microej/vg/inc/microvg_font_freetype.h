/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ MicroVG library low level API: implementation over FreeType.
* @author MicroEJ Developer Team
* @version 2.0.0
*/

#if !defined MICROVG_FONT_FREETYPE_H
#define MICROVG_FONT_FREETYPE_H

#if defined __cplusplus
extern "C" {
#endif

#include "microvg_configuration.h"

#if defined VG_FEATURE_FONT && \
	(defined VG_FEATURE_FONT_FREETYPE_VECTOR || defined VG_FEATURE_FONT_FREETYPE_BITMAP) && \
	(VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR || VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

// --------------------------------------------------------------------------------
// API
// --------------------------------------------------------------------------------

/*
 * @brief Initializes the lowlevel font library.
 *
 */
void MICROVG_FONT_FREETYPE_initialize(void);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
#endif // defined VG_FEATURE_FONT && \
	   // (defined VG_FEATURE_FONT_FREETYPE_VECTOR || defined VG_FEATURE_FONT_FREETYPE_BITMAP) && \
	   // (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR || VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_FONT_FREETYPE_H

