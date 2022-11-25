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
 * @brief This file lists a set of functions called by the display*.c files. First functions
 * are mandatory; optional functions are listed in a second time.
 *
 * These functions are
 * optional and a stub implementation is already available in the CCO (see display_stub.c).
 */

#if !defined DISPLAY_IMPL_H
# define DISPLAY_IMPL_H

#if defined __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "display_framebuffer.h"

// --------------------------------------------------------------------------------
// Functions that must be implemented
// --------------------------------------------------------------------------------

/*
 * Same signature than interrupts.h
 */
uint8_t interrupt_enter(void);

/*
 * Same signature than interrupts.h
 */
void interrupt_leave(uint8_t leave);

/*
 * Same signature than interrupts.h
 */
uint8_t interrupt_is_in(void);

// --------------------------------------------------------------------------------
// Optional functions to implement
// --------------------------------------------------------------------------------

/*
 * @brief Notifies the LLUI_DISPLAY_IMPL is initialized. The implementation can
 * initialize 3rd-party elements.
 */
void DISPLAY_IMPL_initialized(void);

/*
 * @brief Notifies an error has occurred. This error may be critical, in this  case,
 * the implementation must stop the application execution (message + infinite loop).
 *
 * @param[in] critical: tell whether the error is critical or not.
 * @param[in] format: the error message and its arguments
 */
void DISPLAY_IMPL_error(bool critical, const char* format, ...);

/*
 * @brief Notifies the GPU will be used just after this call. The implementation
 * must ensure the GPU can be used (power management, clocks, etc.).
 */
void DISPLAY_IMPL_notify_gpu_start(void);

/*
 * @brief Notifies the GPU is not used anymore by the library until next call to
 * DISPLAY_IMPL_notify_gpu_start().
 */
void DISPLAY_IMPL_notify_gpu_stop(void);

/*
 * @brief Notifies the DMA will be used just after this call. The implementation
 * must ensure the DMA can be used (power management, clocks, etc.).
 */
void DISPLAY_IMPL_notify_dma_start(void);

/*
 * @brief Notifies the DMA is not used anymore by the library until next call to
 * DISPLAY_IMPL_notify_dma_start().
 */
void DISPLAY_IMPL_notify_dma_stop(void);

/*
 * @brief Notifies the first frame buffer will be used just after this call and
 * the second frame buffer will not used anymore until next call to
 * DISPLAY_IMPL_update_frame_buffer_status().
 *
 * @param[in] frame_buffer_on: the frame buffer that will be used
 * @param[in] frame_buffer_off: the frame buffer that not used anymore
 */
void DISPLAY_IMPL_update_frame_buffer_status(framebuffer_t* frame_buffer_on, framebuffer_t* frame_buffer_off);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif // !defined DISPLAY_IMPL_H
