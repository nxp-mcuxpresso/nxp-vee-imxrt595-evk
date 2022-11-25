/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ Freetype wrapper on freetype c files
* @author MicroEJ Developer Team
* @version 2.0.0
*/

#include "microvg_configuration.h"

#if defined (VG_FEATURE_FONT) && defined (VG_FEATURE_FONT_FREETYPE_BITMAP) && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)
#include "../ftsmooth.c"
#include "../ftgrays.c"
#include "../ftdebug.c"
#endif // defined (VG_FEATURE_FONT) && defined (VG_FEATURE_FONT_FREETYPE_BITMAP) && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_BITMAP)

