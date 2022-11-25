/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @brief Implementation of "display_impl.h"
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "display_impl.h"
#include "display_vglite.h"

#include "fsl_debug_console.h"
#include "power_manager.h"
#include "trace_platform.h"
#include "event_generator.h"
#include "mej_log.h"

#include <stdarg.h>

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Special function that manages overlay and rendering mode.
 *
 * @param[in] params: contain which button is pressed when triggering the special function
 */
static void __vglite_special_function(void *params);

// -----------------------------------------------------------------------------
// display_impl.h functions
// -----------------------------------------------------------------------------

void DISPLAY_IMPL_initialized(void) {

    // registers callback to event generator to enable / disable hardware rendering
    EVENT_GENERATOR_register_special_cb(__vglite_special_function, 0);
}

void DISPLAY_IMPL_error(bool critical, const char* format, ...) {
	va_list arg;
	va_start (arg, format);
	MEJ_LOG_PRINTF(format, arg);
	va_end (arg);
	while (critical){}
}

void DISPLAY_IMPL_notify_gpu_start(void) {
   power_manager_enable_low_power(POWER_GPU, LOW_POWER_FORBIDDEN);
}

void DISPLAY_IMPL_notify_gpu_stop(void) {
   power_manager_enable_low_power(POWER_GPU, LOW_POWER_AUTHORIZED);
}

void DISPLAY_IMPL_notify_dma_start(void) {
    power_manager_enable_low_power(POWER_DMA, LOW_POWER_FORBIDDEN);

    TRACE_HW_TASK_START(HW_TASK_DMA_ID);
}

void DISPLAY_IMPL_notify_dma_stop(void) {
    power_manager_enable_low_power(POWER_DMA, LOW_POWER_AUTHORIZED);

    // Request power_manager to power down this buffer
    power_manager_framebuffer_request_power_off();

    TRACE_HW_TASK_STOP(HW_TASK_DMA_ID);
}

void DISPLAY_IMPL_update_frame_buffer_status(framebuffer_t* frame_buffer_on, framebuffer_t* frame_buffer_off) {
    power_manager_framebuffer_power_on(frame_buffer_on);
    power_manager_framebuffer_set_next_off(frame_buffer_off);
}

// -----------------------------------------------------------------------------
// NHardwareRendering functions
// -----------------------------------------------------------------------------

/*
 * @brief Checks if hardware rendering is enabled
 *
 * @return true if hardware rendering is enabled, false otherwise
 * Updates current Graphics Context with vglite's rendering area
 */
jboolean Java_com_microej_display_utils_NHardwareRendering_isEnabled(void) {
    return DISPLAY_VGLITE_is_hardware_rendering_enabled();
}

/*
 * @brief Checks if hardware rendering is disabled
 *
 * @return true if hardware rendering is disabled, false otherwise
 */
jboolean Java_com_microej_display_utils_NHardwareRendering_isDisabled(void) {
    return !DISPLAY_VGLITE_is_hardware_rendering_enabled();
}

/*
 * @brief Disables hardware rendering
 */
void Java_com_microej_display_utils_NHardwareRendering_disable(void) {
	DISPLAY_VGLITE_disable_hardware_rendering();
}

/*
 * @brief Enables hardware rendering
 */
void Java_com_microej_display_utils_NHardwareRendering_enable(void) {
	DISPLAY_VGLITE_enable_hardware_rendering();
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __vglite_special_function(void *params) {
    DISPLAY_VGLITE_toggle_hardware_rendering();
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

