/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * Copyright 2023 NXP
 * SPDX-License-Identifier: BSD-3-Clause
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

#include <stdint.h>

#include <LLUI_DISPLAY.h>

#include "display_vglite.h"
#include "display_impl.h"
#include "display_configuration.h"
#include "color.h"

#include "vg_lite_hal.h"

// -----------------------------------------------------------------------------
// Configuration Sanity Check
// -----------------------------------------------------------------------------

/**
 * Sanity check between the expected version of the configuration and the actual version of
 * the configuration.
 * If an error is raised here, it means that a new version of the CCO has been installed and
 * the configuration async_select_configuration.h must be updated based on the one provided
 * by the new CCO version.
 */

#if !defined DISPLAY_CONFIGURATION_VERSION
#error "Undefined DISPLAY_CONFIGURATION_VERSION, it must be defined in display_configuration.h"
#endif

#if defined DISPLAY_CONFIGURATION_VERSION && DISPLAY_CONFIGURATION_VERSION != 1
#error "Version of the configuration file display_configuration.h is not compatible with this implementation."
#endif

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief To check if the image format is known
 */
#define VG_LITE_UNKNOWN_FORMAT		((vg_lite_buffer_format_t) -1)

/*
 * @brief Width of the Tesselation window
 *
 * @Warning: this impacts the VGLite allocation size
 */
#define DISPLAY_VGLITE_TESSELATION_WIDTH	256

/*
 * @brief Height of the Tesselation window
 *
 * @Warning: this impacts the VGLite allocation size
 */
#define DISPLAY_VGLITE_TESSELATION_HEIGHT	256

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/*
 * @brief VGLite destination buffer (there is only one destination at any time)
 */
static vg_lite_buffer_t destination_buffer;

/*
 * @brief VGLite display context
 */
static vg_lite_display_t display;

/*
 * @brief VGLite window context
 */
static vg_lite_window_t window;

#ifdef VGLITE_OPTION_TOGGLE_GPU
/*
 * @brief flag to enable/disable hardware rendering
 */
static bool hardware_rendering;
#endif

/*
 * @brief flag to wakeup the Graphics Engine when GPU drawing is done
 */
static volatile bool vg_lite_operation_wakeup_graphics_engine;

/*
 * @brief vglite operation semaphore
 */
static SemaphoreHandle_t vg_lite_operation_semaphore;

// -----------------------------------------------------------------------------
// Static Constants
// -----------------------------------------------------------------------------

/*
 * @brief LUT to convert MicroUI image format to VGLite image format
 */
const vg_lite_buffer_format_t __microui_to_vg_lite_format[] = {
		VG_LITE_RGB565,	        // MICROUI_IMAGE_FORMAT_LCD = 0,
		VG_LITE_UNKNOWN_FORMAT,	// UNKNOWN = 1,
		VG_LITE_RGBA8888,		// MICROUI_IMAGE_FORMAT_ARGB8888 = 2,
		VG_LITE_UNKNOWN_FORMAT,	// MICROUI_IMAGE_FORMAT_RGB888 = 3, unsupported
		VG_LITE_RGB565,			// MICROUI_IMAGE_FORMAT_RGB565 = 4,
		VG_LITE_RGBA5551,	// MICROUI_IMAGE_FORMAT_ARGB1555 = 5
		VG_LITE_RGBA4444,		// MICROUI_IMAGE_FORMAT_ARGB4444 = 6,
		VG_LITE_A4,				// MICROUI_IMAGE_FORMAT_A4 = 7,
		VG_LITE_A8,				// MICROUI_IMAGE_FORMAT_A8 = 8,
		// outside of the table ... MICROUI_IMAGE_FORMAT_CUSTOM = 255,
};

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief This callback must be called when the GCNanoLite-V gpu irq is
 * triggered
 */
static void __gpu_irq_callback(void);

/*
 * @brief Configures a vg_lite buffer for a RGB565 image
 *
 * @param[in] b: buffer to configure
 */
static void __buffer_default_configuration(vg_lite_buffer_t* b);

/*
 * @brief Configures and return current vg_lite buffer for the given image (address and size)
 *
 * @param[in] image: image from which buffer should be configured
 * @param[in] b: buffer to configure
 */
static void __buffer_set_address_and_size(MICROUI_Image* image, vg_lite_buffer_t* b);

/*
 * @brief Convert MICROUI_ImageFormat to vg_lite_buffer_format_t
 *
 * @param[in] microui_format: MicroUI image format
 */
static vg_lite_buffer_format_t __convert_format(MICROUI_ImageFormat microui_format);

// -----------------------------------------------------------------------------
// display_vglite.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void DISPLAY_VGLITE_init(void) {
	vg_lite_error_t ret = VG_LITE_SUCCESS;

	ret = VGLITE_CreateDisplay(&display);
	if (VG_LITE_SUCCESS != ret) {
		DISPLAY_IMPL_error(true, "VGLITE_CreateDisplay failed: VGLITE_CreateDisplay() returned error %d", ret);
	}

	// Initialize the window.
	ret = VGLITE_CreateWindow(&display, &window);
	if (VG_LITE_SUCCESS != ret) {
		DISPLAY_IMPL_error(true, "VGLITE_CreateWindow failed: VGLITE_CreateWindow() returned error %d", ret);
	}

	// Initialize the VGLite library and it's tesselation buffer.
	ret = vg_lite_init(DISPLAY_VGLITE_TESSELATION_WIDTH, DISPLAY_VGLITE_TESSELATION_HEIGHT);
	if (VG_LITE_SUCCESS != ret) {
		DISPLAY_IMPL_error(true, "vg_lite engine init failed: vg_lite_init() returned error %d", ret);
	}

	// Configures the vg_lite destination buffer for a RGB565 image
	__buffer_default_configuration(&destination_buffer);

	// Enable by default hardware rendering
	DISPLAY_VGLITE_enable_hardware_rendering();

	vg_lite_operation_semaphore = xSemaphoreCreateBinary();

	vg_lite_hal_register_irq_callback(&__gpu_irq_callback);
}

// See the header file for the function documentation
vg_lite_window_t* DISPLAY_VGLITE_get_window(void) {
	return &window;
}

// See the header file for the function documentation
vg_lite_buffer_t *DISPLAY_VGLITE_get_next_graphics_buffer(void) {
	return VGLITE_GetNextBuffer(&window);
}

void DISPLAY_VGLITE_enable_hardware_rendering(void) {
#ifdef VGLITE_OPTION_TOGGLE_GPU
	hardware_rendering = true;
#endif
}

// See the header file for the function documentation
void DISPLAY_VGLITE_disable_hardware_rendering(void) {
#ifdef VGLITE_OPTION_TOGGLE_GPU
	hardware_rendering = false;
#endif
}

// See the header file for the function documentation
void DISPLAY_VGLITE_toggle_hardware_rendering(void) {
#ifdef VGLITE_OPTION_TOGGLE_GPU
	hardware_rendering = !hardware_rendering;
#endif
}

// See the header file for the function documentation
bool DISPLAY_VGLITE_is_hardware_rendering_enabled(void) {
#ifdef VGLITE_OPTION_TOGGLE_GPU
	return hardware_rendering;
#else
	return true;
#endif
}

// See the header file for the function documentation
vg_lite_buffer_t* DISPLAY_VGLITE_configure_destination(MICROUI_GraphicsContext* gc) {
	MICROUI_Image* image = &gc->image;

	if ((uint32_t)LLUI_DISPLAY_getBufferAddress(image) != destination_buffer.address)
	{
		// we target another destination than previous drawing
		// -> have to use another context to force vg_lite reloading the new context
		__buffer_set_address_and_size(image, &destination_buffer);
	}
	// else: we target the same destination than previous drawing: nothing to do

	return &destination_buffer;
}

// See the header file for the function documentation
bool DISPLAY_VGLITE_configure_source(vg_lite_buffer_t *buffer, MICROUI_Image* image) {

	bool ret = false;
#if defined (FRAME_BUFFER_LINE_ALIGN_BYTE) && defined (VGLITE_IMAGE_LINE_ALIGN_BYTE) && (FRAME_BUFFER_LINE_ALIGN_BYTE < VGLITE_IMAGE_LINE_ALIGN_BYTE)
	if (!LLUI_DISPLAY_isLCD(image)){ // else: frame buffer does not respect VGLite alignment
#endif

		vg_lite_buffer_format_t format = __convert_format((MICROUI_ImageFormat)image->format);

		if (VG_LITE_UNKNOWN_FORMAT != format) {

			__buffer_default_configuration(buffer);
			__buffer_set_address_and_size(image, buffer);

			buffer->image_mode = VG_LITE_MULTIPLY_IMAGE_MODE; // image only
			buffer->transparency_mode = VG_LITE_IMAGE_TRANSPARENT; // FIXME
			buffer->format = format;
			buffer->stride = LLUI_DISPLAY_getStrideInBytes(image);
			ret = true;
		}
#if defined (FRAME_BUFFER_LINE_ALIGN_BYTE) && defined (VGLITE_IMAGE_LINE_ALIGN_BYTE) && (FRAME_BUFFER_LINE_ALIGN_BYTE < VGLITE_IMAGE_LINE_ALIGN_BYTE)
	}
#endif

	return ret;
}

// See the header file for the function documentation
void DISPLAY_VGLITE_start_operation(bool wakeup_graphics_engine) {
	DISPLAY_IMPL_notify_gpu_start();
	vg_lite_operation_wakeup_graphics_engine = wakeup_graphics_engine;

	// VG drawing has been added to the GPU commands list: ask to start VG operation
	vg_lite_flush();

	if (!wakeup_graphics_engine) {
		// active waiting until the GPU interrupt is thrown

		(void)xSemaphoreTake(vg_lite_operation_semaphore, portMAX_DELAY);
	}
}

// See the header file for the function documentation
uint32_t DISPLAY_VGLITE_porter_duff_workaround_ARGB8888(uint32_t color) {
	uint8_t alpha = COLOR_GET_CHANNEL(color, ARGB8888, ALPHA);
	uint32_t ret;

	if (alpha == (uint8_t)0) {
		ret = 0;
	}
	else if(alpha == (uint8_t)0xff) {
		ret = color;
	}
	else {
		uint8_t red = (uint8_t)(
				(uint32_t)(alpha * COLOR_GET_CHANNEL(color, ARGB8888, RED))
				/ (uint32_t)0xff);
		uint8_t green = (uint8_t)(
				(uint32_t)(alpha * COLOR_GET_CHANNEL(color, ARGB8888, GREEN))
				/ (uint32_t)0xff);
		uint8_t blue = (uint8_t)(
				(uint32_t)(alpha * COLOR_GET_CHANNEL(color, ARGB8888, BLUE))
				/ (uint32_t)0xff);
		ret = COLOR_SET_COLOR(alpha, red, green, blue, ARGB8888);
	}
	return ret;
}

// -----------------------------------------------------------------------------
// vg_lite.c functions
// -----------------------------------------------------------------------------

/*
 * Updates current Graphics Context with vglite's rendering area
 */
void vg_lite_draw_notify_render_area(uint32_t x, uint32_t y, uint32_t right, uint32_t bottom) {
	LLUI_DISPLAY_setDrawingLimits(x, y, right, bottom);
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __gpu_irq_callback(void) {
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	uint8_t it = interrupt_enter();
	DISPLAY_IMPL_notify_gpu_stop();

	if (vg_lite_operation_wakeup_graphics_engine) {
		// wake up the Graphics Engine
		LLUI_DISPLAY_notifyAsynchronousDrawingEnd(true);
	}
	else {
		// wake up the caller of DISPLAY_VGLITE_start_operation()
		xSemaphoreGiveFromISR(vg_lite_operation_semaphore, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken != pdFALSE ) {
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	interrupt_leave(it);

}

// See the section 'Internal function definitions' for the function documentation
static void __buffer_default_configuration(vg_lite_buffer_t* b) {
	/* Reset planar. */
	b->yuv.uv_planar = 0;
	b->yuv.v_planar = 0;
	b->yuv.alpha_planar = 0;

	b->tiled = VG_LITE_LINEAR;
	b->format = VG_LITE_RGB565;
	b->handle = NULL;
	b->memory = NULL;
	b->address = 0;
	b->yuv.swizzle = VG_LITE_SWIZZLE_UV;
	b->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
	b->transparency_mode = VG_LITE_IMAGE_OPAQUE;
}

// See the section 'Internal function definitions' for the function documentation
static void __buffer_set_address_and_size(MICROUI_Image* image, vg_lite_buffer_t* b) {
	b->width = image->width;
	b->height = image->height;
	b->stride = LLUI_DISPLAY_getStrideInBytes(image);
	b->memory = (void*)LLUI_DISPLAY_getBufferAddress(image);
	b->address = (uint32_t)LLUI_DISPLAY_getBufferAddress(image);
}

// See the section 'Internal function definitions' for the function documentation
vg_lite_buffer_format_t __convert_format(MICROUI_ImageFormat microui_format) {
	vg_lite_buffer_format_t vg_lite_format = VG_LITE_UNKNOWN_FORMAT;
	if (microui_format < (sizeof(__microui_to_vg_lite_format) / sizeof(__microui_to_vg_lite_format[0]))) {
		vg_lite_format = __microui_to_vg_lite_format[microui_format];
	}

	return vg_lite_format;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
