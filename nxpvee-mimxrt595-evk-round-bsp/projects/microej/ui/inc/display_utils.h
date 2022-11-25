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

#if !defined DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <LLUI_PAINTER_impl.h>

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Unknown resource format
 */
#define DISPLAY_UNKNOWN_FORMAT		-1

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Returns the bit per pixel from a MicroUI image format
 *
 * @param[in] image_format: The MicroUI format of the image
 *
 * @return The bit per pixel of the image, or -1 if the image format is unknown
 */
int32_t DISPLAY_UTILS_get_bpp(MICROUI_ImageFormat image_format);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined DISPLAY_UTILS_H
