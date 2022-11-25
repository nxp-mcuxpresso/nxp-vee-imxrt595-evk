/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroEJ MicroVG library low level API: implementation of LinearGradient.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#if !defined MICROVG_GRADIENT_H
#define MICROVG_GRADIENT_H

#if defined __cplusplus
extern "C" {
#endif

#include "microvg_configuration.h"

#if defined (VG_FEATURE_GRADIENT) && defined (VG_FEATURE_GRADIENT_FULL) && defined (VG_FEATURE_GRADIENT_FIRST_COLOR)

#if ((VG_FEATURE_GRADIENT != VG_FEATURE_GRADIENT_FULL) && (VG_FEATURE_GRADIENT != VG_FEATURE_GRADIENT_FIRST_COLOR))
#error "Specify a value for VG_FEATURE_GRADIENT"
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <sni.h>

// -----------------------------------------------------------------------------
// Structs
// -----------------------------------------------------------------------------

/*
 * @brief Map a jint array that represents a linear gradient
 */
typedef struct MICROVG_GRADIENT_HEADER
{

#if (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL)
	jint count;  // number of colors and positions
	jfloat x;
	jfloat y;
	jfloat angle;
	jfloat length;
	jint colors_offset;
	jint positions_offset;
#else
	jint color; // only one color
#endif
} MICROVG_GRADIENT_HEADER_t;

#if (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL)

// -----------------------------------------------------------------------------
// Specific gradient formatting functions [mandatory]
// -----------------------------------------------------------------------------

/*
 * @brief Gets the size of the gradient color scale where the gradient colors
 * positions are stored.
 *
 * @return the gradient scale size
 */
float MICROVG_GRADIENT_get_gradient_scale_size(void);

// -----------------------------------------------------------------------------
// Specific gradient formatting functions [optional]
// -----------------------------------------------------------------------------

/*
 * @brief Gets the gradient's array header size.
 *
 * The default implementation returns sizeof(MICROVG_GRADIENT_HEADER_t) / sizeof(uint32_t).
 *
 * @return the size in 32-bit format ("1" means 4 bytes)
 */
uint32_t MICROVG_GRADIENT_get_gradient_header_size(void);

#endif // #if (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL)

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_GRADIENT

#ifdef __cplusplus
}
#endif

#endif // !defined MICROVG_GRADIENT_H
