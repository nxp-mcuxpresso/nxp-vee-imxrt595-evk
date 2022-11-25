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

#if !defined DISPLAY_FRAMEBUFFER_H
#define DISPLAY_FRAMEBUFFER_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

#include "display_configuration.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Available number of bytes per pixels (RGB565)
 */
#define FRAME_BUFFER_BYTE_PER_PIXEL (2)

/*
 * @brief Frame buffer alignment. 
 * - 4 bits are required by SMARTDMA
 * - 64 bits are required by DMA
 * Note: Full frame buffer memory size is a value aligned on 64 too;
 * so next allocation behind frame buffer memory is aligned on 64 too.
 */
#define FRAME_BUFFER_ALIGN (64)

/*
 * @brief Macro to align data
 */
#define ALIGN(value, align) (((value) + (align) - 1) & ~((align) - 1))

/*
 * @brief Frame buffer line size in bytes (stride), according to FRAME_BUFFER_LINE_ALIGN
 */
#define FRAME_BUFFER_STRIDE_BYTE ALIGN(FRAME_BUFFER_WIDTH * FRAME_BUFFER_BYTE_PER_PIXEL, FRAME_BUFFER_LINE_ALIGN_BYTE)

/*
 * @brief Frame buffer line size in pixels (stride), according to FRAME_BUFFER_LINE_ALIGN
 */
#define FRAME_BUFFER_STRIDE_PIXELS (FRAME_BUFFER_STRIDE_BYTE / FRAME_BUFFER_BYTE_PER_PIXEL)

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/*
 * @brief Frame buffer structure definition
 */
typedef struct s_framebuffer {
	uint16_t p[FRAME_BUFFER_HEIGHT][FRAME_BUFFER_STRIDE_PIXELS];
} framebuffer_t;

// -----------------------------------------------------------------------------
// Variables
// -----------------------------------------------------------------------------

/*
 * @brief Frame buffer address array
 */
extern const framebuffer_t * s_frameBufferAddress[FRAME_BUFFER_COUNT];

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif	// !defined  DISPLAY_FRAMEBUFFER_H
