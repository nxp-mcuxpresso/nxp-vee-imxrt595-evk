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

#include <LLUI_DISPLAY_impl.h>

#include <FreeRTOS.h>
#include <semphr.h>

#include "display_dma.h"
#include "display_utils.h"
#include "display_vglite.h"
#include "display_impl.h"
#include "framerate.h"

#include "fsl_dc_fb_dsi_cmd.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#if (CUSTOM_VGLITE_MEMORY_CONFIG != 1)
#error "Application must be compiled with CUSTOM_VGLITE_MEMORY_CONFIG=1"
#else
#define VGLITE_COMMAND_BUFFER_SZ (128 * 1024)
#define VGLITE_HEAP_SZ 0x100000
#endif

#define DISPLAY_STACK_SIZE       (8 * 1024)
#define DISPLAY_TASK_PRIORITY    (12)                       /** Should be > tskIDLE_PRIORITY & < configTIMER_TASK_PRIORITY */
#define DISPLAY_TASK_STACK_SIZE  (DISPLAY_STACK_SIZE / 4)

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/* Allocate the heap and set the command buffer(s) size */
AT_NONCACHEABLE_SECTION_ALIGN(uint8_t vglite_heap[VGLITE_HEAP_SZ], 64);

void *vglite_heap_base        = &vglite_heap;
uint32_t vglite_heap_size     = VGLITE_HEAP_SZ;
uint32_t vglite_cmd_buff_size = VGLITE_COMMAND_BUFFER_SZ;

/*
 * @brief: Semaphore to synchronize the display flush with MicroUI
 */
static SemaphoreHandle_t sync_flush;
static uint8_t* dirty_area_addr;	// Address of the source framebuffer
static int32_t dirty_area_ymin;	// Top-most coordinate of the area to synchronize
static int32_t dirty_area_ymax;	// Bottom-most coordinate of the area to synchronize

// -----------------------------------------------------------------------------
// Private functions
// -----------------------------------------------------------------------------

/*
 * @brief: Flush current framebuffer to the display
 */
static void __display_task_swap_buffers(vg_lite_window_t* pWindow) {

	if ((dirty_area_ymin > 0) || (dirty_area_ymax < (FRAME_BUFFER_HEIGHT - 1))) {
		// No need to send all content: modify framebuffer information to reduce
		// the number of data to send

		// Retrieve framebuffer information
		const dc_fb_t *dc = pWindow->display->g_fbdev.dc;
		dc_fb_dsi_cmd_handle_t *dcHandle = dc->prvData;
		dc_fb_dsi_cmd_layer_t *pLayer    = &(dcHandle->layers[pWindow->display->g_fbdev.layer]);
		dc_fb_info_t *fbInfo             = &(pLayer->fbInfo);

		// Update startY and height
		fbInfo->startY = dirty_area_ymin;
		fbInfo->height = dirty_area_ymax - dirty_area_ymin + 1;

		// Update memory pointer to point on first dirty line
		vg_lite_buffer_t * current_buffer = VGLITE_GetRenderTarget(pWindow);
		uint8_t * original_memory = (uint8_t *) current_buffer->memory;
		current_buffer->memory = &((uint8_t *)current_buffer->memory)[dirty_area_ymin * fbInfo->strideBytes];

		VGLITE_SwapBuffers(pWindow);

		// Restore original context
		current_buffer->memory = original_memory;
		fbInfo->startY = 0;
		fbInfo->height = FRAME_BUFFER_HEIGHT;
	}
	else {
		// just have to swap both buffers
		VGLITE_SwapBuffers(pWindow);
	}
}

/*
 * @brief: Task to manage display flushes and synchronize with hardware rendering
 * operations
 */
static void __display_task(void * pvParameters) {
	(void)pvParameters;

	do {
		xSemaphoreTake(sync_flush, portMAX_DELAY);

		vg_lite_window_t* window = DISPLAY_VGLITE_get_window();

		// Two actions:
		// 1- wait for the end of previous swap (if not already done): wait the
		// end of sending of current frame buffer to display
		// 2- start sending of current_buffer to display (without waiting the
		// end)
		__display_task_swap_buffers(window);

		// Increment framerate
		framerate_increment();

#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 1)
		vg_lite_buffer_t *current_buffer = VGLITE_GetRenderTarget(window);

		// Configure frame buffer powering; at that point current_buffer is back buffer
		// cppcheck-suppress [misra-c2012-11.3] cast to (framebuffer_t *) is valid
		DISPLAY_IMPL_update_frame_buffer_status(current_buffer->memory, (framebuffer_t *)dirty_area_addr);

		DISPLAY_DMA_start(
				(void *) dirty_area_addr,
				current_buffer->memory,
				dirty_area_ymin,
				dirty_area_ymax);
#else
		LLUI_DISPLAY_flushDone(false);
#endif

	} while (1);
}


// -----------------------------------------------------------------------------
// LLUI_DISPLAY_impl.h functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void LLUI_DISPLAY_IMPL_initialize(LLUI_DISPLAY_SInitData* init_data) {

	/***************
	 * Init VGLite *
	 ***************/

	if (kStatus_Success != BOARD_PrepareVGLiteController()) {
		DISPLAY_IMPL_error(true, "Prepare VGlite controlor error");
	}

	DISPLAY_VGLITE_init();

	/************
	 * Init DMA *
	 ************/

#if defined (FRAME_BUFFER_COUNT) && (FRAME_BUFFER_COUNT > 1)
	static void ** tmp;

	tmp = VGLITE_GetBuffers();

	for (int i = 0; i < FRAME_BUFFER_COUNT; i++) {
		// cppcheck-suppress [misra-c2012-11.5] VGLITE_GetBuffers() return an array of framebuffer_t*
		s_frameBufferAddress[i] = tmp[i];
	}
#endif

#if defined (DISPLAY_DMA_ENABLED) && (DISPLAY_DMA_ENABLED != 0)
	DISPLAY_DMA_initialize(s_frameBufferAddress);
#endif // defined DISPLAY_DMA_ENABLED

	/*************
	 * Init task *
	 *************/

	sync_flush = xSemaphoreCreateBinary();
	if (NULL == xTaskCreate(
			__display_task,
			"Display",
			DISPLAY_TASK_STACK_SIZE,
			NULL,
			DISPLAY_TASK_PRIORITY,
			NULL)){
		DISPLAY_IMPL_error(true, "failed to create task __display\n");
	}

	/****************
	 * Init MicroUI *
	 ****************/

	vg_lite_window_t* window = DISPLAY_VGLITE_get_window();
	vg_lite_buffer_t *buffer = VGLITE_GetRenderTarget(window);
	init_data->binary_semaphore_0 = (void*)xSemaphoreCreateBinary();
	init_data->binary_semaphore_1 = (void*)xSemaphoreCreateBinary();
	init_data->lcd_width = window->width;
	init_data->lcd_height = window->height;
	init_data->memory_width = FRAME_BUFFER_STRIDE_PIXELS;
	init_data->back_buffer_address = (uint8_t*)buffer->memory;

	// notify that the display is initialized
	DISPLAY_IMPL_initialized();
}

// See the header file for the function documentation
uint8_t* LLUI_DISPLAY_IMPL_flush(MICROUI_GraphicsContext* gc, uint8_t* addr, uint32_t xmin, uint32_t ymin, uint32_t xmax, uint32_t ymax) {

	// x bounds are not used by the DMA
	(void)gc;
	(void)xmin;
	(void)xmax;

	uint8_t* ret = (uint8_t*) DISPLAY_VGLITE_get_next_graphics_buffer()->memory;

	// store dirty area to restore after the flush
	dirty_area_addr = addr;
	dirty_area_ymin = ymin;
	dirty_area_ymax = ymax;

	// wakeup display task
	xSemaphoreGive(sync_flush);

	return ret;
}

// See the header file for the function documentation
void LLUI_DISPLAY_IMPL_binarySemaphoreTake(void* sem) {
	xSemaphoreTake((SemaphoreHandle_t)sem, portMAX_DELAY);
}

// See the header file for the function documentation
void LLUI_DISPLAY_IMPL_binarySemaphoreGive(void* sem, bool under_isr) {

	if (under_isr) {
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR((SemaphoreHandle_t)sem, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken != pdFALSE ) {
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
	else  {
		xSemaphoreGive((SemaphoreHandle_t)sem);
	}
}

// See the header file for the function documentation
uint32_t LLUI_DISPLAY_IMPL_getNewImageStrideInBytes(jbyte image_format, uint32_t image_width, uint32_t image_height, uint32_t default_stride) {

	(void)image_height;
	(void)default_stride;

	/*
	 * VGLite library requires a stride on 16 pixels: 64 bytes for 32bpp, 32 bytes
	 * for 16bpp and 16 bytes for 8bpp. Exception 8 bytes for <= 4bpp.
	 */
	uint32_t bpp = DISPLAY_UTILS_get_bpp((MICROUI_ImageFormat)image_format);
	return (bpp >= (uint32_t)8) ? ALIGN(image_width, (uint32_t)16) * (bpp / (uint32_t)8) : ALIGN(image_width, (uint32_t)8);
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
