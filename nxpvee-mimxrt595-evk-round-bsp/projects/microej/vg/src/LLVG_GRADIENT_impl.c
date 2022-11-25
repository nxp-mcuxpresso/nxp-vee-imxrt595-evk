/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief MicroVG library low level API: implementation of gradient.
 *
 * This implementation uses 32-bit "integer" values to store the colors and the colors
 * positions.
 *
 * @author MicroEJ Developer Team
 * @version 2.0.0
 */

#include "microvg_configuration.h"

#ifdef VG_FEATURE_GRADIENT

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <math.h>
#include <string.h>

#include <LLVG_GRADIENT_impl.h>

#include "microvg_gradient.h"
#include "microvg_helper.h"
#include "bsp_util.h"

#if defined (VG_FEATURE_GRADIENT) && defined (VG_FEATURE_GRADIENT_FULL) && (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL)

// -----------------------------------------------------------------------------
// Specific gradient format functions [optional]: weak functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
BSP_DECLARE_WEAK_FCNT uint32_t MICROVG_GRADIENT_get_gradient_header_size(void) {
	return sizeof(MICROVG_GRADIENT_HEADER_t) / sizeof(uint32_t);
}

// -----------------------------------------------------------------------------
// LLVG_GRADIENT_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
jint LLVG_GRADIENT_IMPL_initializeGradient(jint* jgradient, jint length, jint* colors, jint count,
		jfloat* positions, jfloat xStart, jfloat yStart, jfloat xEnd, jfloat yEnd) {
	MICROVG_GRADIENT_HEADER_t* gradient = (MICROVG_GRADIENT_HEADER_t*)jgradient;
	uint32_t expectedLength = MICROVG_GRADIENT_get_gradient_header_size() + (count * 2 /* colors and positions */);
	jint ret;
	if (length >= expectedLength) {

		// fill header
		float angle = RAD_TO_DEG(atan2f(yEnd - yStart, xEnd - xStart));
		float l = sqrtf(powf(xEnd - xStart, 2) + powf(yEnd - yStart, 2));

		gradient->x = xStart;
		gradient->y = yStart;
		gradient->angle = angle;

		// If gradient length is null, create a horizontal gradient
		// with length==1 using first and last color.
		// The transition point is placed at the center of the scale.
		if(0.f == l){
			gradient->length = 1;
			gradient->count = 2;

			gradient->colors_offset = MICROVG_GRADIENT_get_gradient_header_size();
			gradient->positions_offset = MICROVG_GRADIENT_get_gradient_header_size() + (jint)2;

			uint32_t* colors_addr = &((uint32_t*)gradient)[gradient->colors_offset];
			*colors_addr = *colors;
			colors_addr[1] = colors[count - 1];

			uint32_t* positions_addr = &((uint32_t*)gradient)[gradient->positions_offset];
			*positions_addr = (uint32_t) (MICROVG_GRADIENT_get_gradient_scale_size() / 2);
			positions_addr[1] = (*positions_addr) + 1;
		}
		else{

			gradient->count = (uint32_t)count;
			gradient->length = l;

			gradient->colors_offset = MICROVG_GRADIENT_get_gradient_header_size();
			gradient->positions_offset = MICROVG_GRADIENT_get_gradient_header_size() + count;

			// fill colors
			void* colors_addr = (void*)&((uint32_t*)gradient)[gradient->colors_offset];
			(void)memcpy(colors_addr, colors, count * sizeof(uint32_t));

			// fill positions
			uint32_t max_position = (uint32_t) (MICROVG_GRADIENT_get_gradient_scale_size() - 1);
			uint32_t* positions_addr = &((uint32_t*)gradient)[gradient->positions_offset];
			if (NULL == positions) {
				float step = (float) (max_position / (count - 1));
				float currentStep = 0.f;
				for (uint32_t i = 0; i < count; i++) {
					*positions_addr = (uint32_t) currentStep;
					++positions_addr;
					currentStep += step;
				}
			}
			else {
				// scales an Android gradient stops array with the gradient scale
				for (uint32_t i = 0; i < count; i++) {
					*positions_addr = (uint32_t) (positions[i] * max_position);
					++positions_addr;
				}
			}
		}
		ret = LLVG_SUCCESS;
	}
	else {
		// the given array is too small
		ret = expectedLength;
	}
	return ret;
}

#else // #if (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL) => VG_FEATURE_GRADIENT_FIRST_COLOR

// See the header file for the function documentation
jint LLVG_GRADIENT_IMPL_initializeGradient(jint* gradient, jint length, jint* colors, jint count,
		jfloat* positions, jfloat xStart, jfloat yStart, jfloat xEnd, jfloat yEnd) {

	(void)count;
	(void)positions;
	(void)xStart;
	(void)yStart;
	(void)xEnd;
	(void)yEnd;

	jint ret;

	if (length < 1) {
		// the given array is too small
		ret = 1; // return the expected minimal size
	}
	else {
		// just store the first color
		gradient[0] = colors[0];
		ret = LLVG_SUCCESS;
	}

	return ret;
}

#endif // #if (VG_FEATURE_GRADIENT == VG_FEATURE_GRADIENT_FULL)

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#endif // VG_FEATURE_GRADIENT
