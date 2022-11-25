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

#if !defined COLOR_H
#define COLOR_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Mask of a channel in ARGB8888 format
 */
#define COLOR_ARGB8888_CHANNEL_SIZE 8

/*
 * @brief Mask of a channel in ARGB8888 format
 */
#define COLOR_ARGB8888_CHANNEL_MASK \
	((1 << COLOR_ARGB8888_CHANNEL_SIZE) - 1)

/*
 * @brief Mask of a channel
 *
 * @param format: The color format
 * @param channel: The channel
 */
#define COLOR_CHANNEL_MASK(format, channel) \
	((1 << COLOR_CHANNEL_SIZE(format, channel)) - 1)

/*
 * @brief Size of a channel
 *
 * @param format: The color format
 * @param channel: The channel
 */
#define COLOR_CHANNEL_SIZE(format, channel) \
	COLOR_ ## format ## _ ## channel ## _SIZE

/*
 * @brief Offset of a channel
 *
 * @param format: The color format
 * @param channel: The channel
 */
#define COLOR_CHANNEL_OFFSET(format, channel) \
	COLOR_ ## format ## _ ## channel ## _OFFSET

/*
 * @brief Alpha channel size for ARGB8888 format
 */
#define COLOR_ARGB8888_ALPHA_SIZE COLOR_ARGB8888_CHANNEL_SIZE

/*
 * @brief Alpha channel offset for ARGB8888 format
 */
#define COLOR_ARGB8888_ALPHA_OFFSET 24

/*
 * @brief Alpha channel mask for ARGB8888 format
 */
#define COLOR_ARGB8888_ALPHA_MASK \
	COLOR_CHANNEL_MASK(ARGB8888, ALPHA)

/*
 * @brief Red channel size for ARGB8888 format
 */
#define COLOR_ARGB8888_RED_SIZE COLOR_ARGB8888_CHANNEL_SIZE

/*
 * @brief Red channel offset for ARGB8888 format
 */
#define COLOR_ARGB8888_RED_OFFSET 16

/*
 * @brief Red channel mask for ARGB8888 format
 */
#define COLOR_ARGB8888_RED_MASK \
	COLOR_CHANNEL_MASK(ARGB8888, RED)

/*
 * @brief Green channel size for ARGB8888 format
 */
#define COLOR_ARGB8888_GREEN_SIZE COLOR_ARGB8888_CHANNEL_SIZE

/*
 * @brief Green channel offset for ARGB8888 format
 */
#define COLOR_ARGB8888_GREEN_OFFSET 8

/*
 * @brief Green channel mask for ARGB8888 format
 */
#define COLOR_ARGB8888_GREEN_MASK \
	COLOR_CHANNEL_MASK(ARGB8888, GREEN)

/*
 * @brief Blue channel size for ARGB8888 format
 */
#define COLOR_ARGB8888_BLUE_SIZE COLOR_ARGB8888_CHANNEL_SIZE

/*
 * @brief Blue channel offset for ARGB8888 format
 */
#define COLOR_ARGB8888_BLUE_OFFSET 0

/*
 * @brief Blue channel mask for ARGB8888 format
 */
#define COLOR_ARGB8888_BLUE_MASK \
	COLOR_CHANNEL_MASK(ARGB8888, BLUE)

/*
 * @brief Gets a channel value from a color
 *
 * param color: The color from which to retrieve a channel
 * param format: The color format (i.e. ARGB8888, etc...)
 */
#define COLOR_GET_CHANNEL(color, format, channel) \
	(uint8_t) ( \
			((color) >> COLOR_CHANNEL_OFFSET(format, channel)) \
			& COLOR_CHANNEL_MASK(format, channel) \
			)

/*
 * @brief Sets a color
 *
 * param alpha: The alpha channel value
 * param red: The red channel value
 * param green: The green channel value
 * param blue: The blue channel value
 * param format: The color format (i.e. ARGB8888, etc...)
 */
#define COLOR_SET_COLOR(alpha, red, green, blue, format) \
	(uint32_t) ( 0\
			| (((alpha) & COLOR_ ## format ## _ALPHA_MASK) \
				<< (COLOR_ ## format ## _ALPHA_OFFSET)) \
			| (((red) & COLOR_ ## format ## _RED_MASK) \
				<< (COLOR_ ## format ## _RED_OFFSET)) \
			| (((green) & COLOR_ ## format ## _GREEN_MASK) \
				<< (COLOR_ ## format ## _GREEN_OFFSET)) \
			| (((blue) & COLOR_ ## format ## _BLUE_MASK) \
				<< (COLOR_ ## format ## _BLUE_OFFSET)) \
				)

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined COLOR_H
