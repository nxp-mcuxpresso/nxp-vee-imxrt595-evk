/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief Freetype vglite renderer
* @author MicroEJ Developer Team
* @version 3.0.0
*/

#ifndef FTVECTOR_H
#define FTVECTOR_H

#if defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)

#include <stdint.h>
#include "vg_lite.h"

#define FT_PARAM_TAG_VGLITE_DESTINATION     FT_MAKE_TAG( 'd', 'e', 's', 't' )
#define FT_PARAM_TAG_VGLITE_MATRIX          FT_MAKE_TAG( 'm', 'a', 't', 'r' )
#define FT_PARAM_TAG_VGLITE_QUALITY         FT_MAKE_TAG( 'q', 'u', 'a', 'l' )
#define FT_PARAM_TAG_VGLITE_FORMAT          FT_MAKE_TAG( 'f', 'o', 'r', 'm' )
#define FT_PARAM_TAG_VGLITE_BLEND           FT_MAKE_TAG( 'b', 'l', 'e', 'n' )
#define FT_PARAM_TAG_VGLITE_COLOR           FT_MAKE_TAG( 'c', 'o', 'l', 'o' )
#define FT_PARAM_TAG_VGLITE_GRADIENT        FT_MAKE_TAG( 'g', 'r', 'a', 'd' )

#define FT_VGLITE_PATH_POOL_CHUNK       0x100

// Internal debug define to print to console the glyphs outlines path strings
// path strings can then be observed with https://yqnn.github.io/svg-path-editor
//#define FT_VGLITE_LOG_OUTLINES

#define VGLITE_CMD_MOVE_TO         2
typedef struct path_move_to_16 {
    int16_t cmd;
    int16_t x;
    int16_t y;
} path_move_to_16_t;

#define VGLITE_CMD_LINE_TO         4
typedef struct path_line_to_16 {
    int16_t cmd;
    int16_t x;
    int16_t y;
} path_line_to_16_t;

#define VGLITE_CMD_QUAD_TO         6
typedef struct path_quad_to_16 {
    int16_t cmd;
    int16_t cx;
    int16_t cy;
    int16_t x;
    int16_t y;
} path_quad_to_16_t;

#define VGLITE_CMD_CUBIC_TO        8
typedef struct path_cubic_to_16 {
    int16_t cmd;
    int16_t cx1;
    int16_t cy1;
    int16_t cx2;
    int16_t cy2;
    int16_t x;
    int16_t y;
} path_cubic_to_16_t;

#define VGLITE_CMD_END             0
typedef struct path_end_16 {
    int16_t cmd;
} path_end_16_t;

#endif // defined VG_FEATURE_FONT && (VG_FEATURE_FONT == VG_FEATURE_FONT_FREETYPE_VECTOR)

#endif // FTVECTOR_H
