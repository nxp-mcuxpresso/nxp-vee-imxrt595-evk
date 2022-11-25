/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 *
 * @brief Stub implementation of "display_impl.h"
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdarg.h>

#include "display_impl.h"
#include "bsp_util.h"
#include "fsl_debug_console.h"

// -----------------------------------------------------------------------------
// display_impl.h functions
// -----------------------------------------------------------------------------

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_initialized(void) {
	// does nothing by default
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_error(bool critical, const char* format, ...) {
	va_list arg;
	va_start (arg, format);
	PRINTF(format, arg);
	va_end (arg);
	while (critical){}
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_notify_gpu_start(void) {
	// does nothing by default
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_notify_gpu_stop(void) {
	// does nothing by default
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_notify_dma_start(void) {
	// does nothing by default
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_notify_dma_stop(void) {
	// does nothing by default
}

BSP_DECLARE_WEAK_FCNT void DISPLAY_IMPL_update_frame_buffer_status(framebuffer_t* frame_buffer_on, framebuffer_t* frame_buffer_off) {
	// does nothing by default
	(void)frame_buffer_on;
	(void)frame_buffer_off;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

