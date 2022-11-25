/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 */

#if !defined MEJ_MATH_H
# define MEJ_MATH_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <arm_math.h>

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Returns the minimum of two values
 */
# define MEJ_MIN(a, b)  (((a) < (b)) ? (a) : (b))

/*
 * @brief Returns the minimum of two values
 */
# define MEJ_ABS(a)  (((a) < 0) ? (-a) : (a))

/*
 * @brief Returns the minimum of two values
 */
# define MEJ_MAX(a, b)  (((a) < (b)) ? (b) : (a))

/*
 * @brief Computes the percentage between a and b
 */
# define MEJ_PERCENT(a, b) ((100 * (a)) / (b))

/*
 * @brief Converts degrees to radians
 */
# define MEJ_DEG2RAD(d)		(((d) * PI) / 180)

/*
 * @brief Converts radians to degrees
 */
# define MEJ_RAD2DEG(d)		(((d) * 180) / PI)

/*
 * @brief Computes a bounded value
 */
# define MEJ_BOUNDS(v, min, max)		MEJ_MAX((min), MEJ_MIN((v), (max)))

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Floating-point square root function.
 * @param[in] value: input value
 *
 * @return: square root of input value
 */
float32_t mej_sqrt_f32(float32_t value);

/*
 * @brief Floating-point return the tangent of value, where value is given in radians.
 * @param[in] value: input value
 *
 * @return: the tangent of input value
 */
float32_t mej_tan_f32(float32_t value);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined MEJ_MATH_H
