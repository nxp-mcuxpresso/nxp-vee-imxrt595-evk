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

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "display_framebuffer.h"

#include "fsl_dc_fb.h"

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

static framebuffer_t s_frameBuffer0 __attribute__((section(".framebuffer0")));
#define FRAME_BUFFER0_ADDR &s_frameBuffer0

#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 1)
static framebuffer_t s_frameBuffer1  __attribute__((section(".framebuffer1")));
#define FRAME_BUFFER1_ADDR &s_frameBuffer1
#endif

#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 2)
static framebuffer_t s_frameBuffer2  __attribute__((section(".framebuffer2")));
#define FRAME_BUFFER2_ADDR &s_frameBuffer2
#endif

// cppcheck-suppress [misra-c2012-9.3] array is fully initialized
const framebuffer_t * s_frameBufferAddress[FRAME_BUFFER_COUNT] = {
		FRAME_BUFFER0_ADDR,
#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 1)
		FRAME_BUFFER1_ADDR,
#endif
#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 2)
		FRAME_BUFFER2_ADDR
#endif
	};

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
