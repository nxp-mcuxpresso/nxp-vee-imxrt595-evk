/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "mej_math.h"

#include "fsl_powerquad.h"

// -----------------------------------------------------------------------------
// mej_math.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
float32_t mej_tan_f32(float32_t value) {
	float32_t cos;
	float32_t sin;

	PQ_CosF32(&value, &cos);
	PQ_SinF32(&value, &sin);

	PQ_DivF32(&sin, &cos, &value);

	return value;
}

// See the header file for the function documentation
float32_t mej_sqrt_f32(float32_t value) {
	PQ_SqrtF32(&value, &value);

	return value;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
