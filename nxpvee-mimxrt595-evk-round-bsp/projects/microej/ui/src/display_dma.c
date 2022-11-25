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

#include <LLUI_DISPLAY.h>

#include "display_dma.h"
#include "display_impl.h"
#include "vglite_window.h"

#include "fsl_dma.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/* @brief Maximum DMA transfers (DMA spec) */
#define DISPLAY_DMA_MAX_TRANSFER        (4096)

/* @brief Width of one element in a DMA transfer (DMA spec) */
#define DISPLAY_DMA_TRANSFER_WIDTH      (4U)

/* @brief Number of lines per DMA transfer */
#define DISPLAY_DMA_NB_LINES            (DISPLAY_DMA_MAX_TRANSFER / FRAME_BUFFER_STRIDE_BYTE)

/* @brief Number of descriptors in the DMA chain */
#define DISPLAY_DMA_NB_DESCS            \
	(1 + (FRAME_BUFFER_HEIGHT / DISPLAY_DMA_NB_LINES))

/* @brief Length of a DMA transfer */
#define DISPLAY_DMA_TRANSFER_LENGTH     \
	(DISPLAY_DMA_NB_LINES * FRAME_BUFFER_STRIDE_BYTE)

/* @brief Number of lines of last DMA transfer */
#define DISPLAY_LAST_DMA_NB_LINES            (FRAME_BUFFER_HEIGHT % DISPLAY_DMA_NB_LINES)

/* @brief Length of a Last DMA transfer */
#define DISPLAY_LAST_DMA_TRANSFER_LENGTH     \
	(DISPLAY_LAST_DMA_NB_LINES * FRAME_BUFFER_STRIDE_BYTE)

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

#if defined (DISPLAY_DMA_ENABLED) && (DISPLAY_DMA_ENABLED != 0)
static dma_handle_t g_DMA_Handle;

#if defined(__ICCARM__)
#pragma data_alignment = 16U
static dma_descriptor_t dma_descriptors[2 * DISPLAY_DMA_NB_DESCS];
#elif defined(__CC_ARM)
static dma_descriptor_t __attribute__((aligned(16U))) dma_descriptors[2 * DISPLAY_DMA_NB_DESCS];
#elif defined(__GNUC__)
static dma_descriptor_t __attribute__((aligned(16U))) dma_descriptors[2 * DISPLAY_DMA_NB_DESCS];
#endif

#endif // DISPLAY_DMA_ENABLED != 0

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief: Callback called when a DMA transfer is done
 *
 * @param[in] handle: Handle on the DMA transfer
 * @param[in] param: User parameters for the callback
 * @param[in] transfer_done: Flag indicating if the current transfer is done
 */
static void __dma_callback(dma_handle_t *handle, void *param, bool transfer_done, uint32_t tcds);

// -----------------------------------------------------------------------------
// display_dma.h
// -----------------------------------------------------------------------------

#if defined (DISPLAY_DMA_ENABLED) && (DISPLAY_DMA_ENABLED != 0)
// See the header file for the function documentation
void DISPLAY_DMA_initialize(const framebuffer_t * framebuffers[]) {

	DMA_Init(DMA1);
	DMA_CreateHandle(&g_DMA_Handle, DMA1, 0);
	DMA_EnableChannel(DMA1, 0);
	DMA_SetCallback(&g_DMA_Handle, __dma_callback, NULL);

	// Initialize the dma descriptors
	for (int i = 0; i < 2; i++) {
		framebuffer_t const * fb_src = framebuffers[i];
		framebuffer_t const * fb_dst = framebuffers[1-i];
		int offset = i*DISPLAY_DMA_NB_DESCS;
		int y;
		int d;

		d = offset;

		do {
			y = DISPLAY_DMA_NB_LINES * (d - offset);

			if (d >= (offset+(DISPLAY_DMA_NB_DESCS-1))) {
				break;
			}

			DMA_SetupDescriptor(
					&(dma_descriptors[d]),
					DMA_CHANNEL_XFER(
						true, false, false, false,
						DISPLAY_DMA_TRANSFER_WIDTH,
						kDMA_AddressInterleave1xWidth,
						kDMA_AddressInterleave1xWidth,
						DISPLAY_DMA_TRANSFER_LENGTH
						),

						// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_src->p[y]),
						// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_dst->p[y]),
					&(dma_descriptors[d+1])
					);

			d++;
		} while (1);

		// Check if we need a last descriptor to send last lines
#if (0 != DISPLAY_LAST_DMA_TRANSFER_LENGTH)
			y = DISPLAY_DMA_NB_LINES * (d - offset);
			// Setup last descriptor
			DMA_SetupDescriptor(
					&(dma_descriptors[d]),
					DMA_CHANNEL_XFER(
						false, false, true, false,
						DISPLAY_DMA_TRANSFER_WIDTH,
						kDMA_AddressInterleave1xWidth,
						kDMA_AddressInterleave1xWidth,
						DISPLAY_LAST_DMA_TRANSFER_LENGTH
						),
						// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_src->p[y]),
					// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_dst->p[y]),
					NULL);
#else
			// Reconfigure previous descriptor to be the last descriptor
			DMA_SetupDescriptor(
					&(dma_descriptors[d-1]),
					DMA_CHANNEL_XFER(
						false, false, true, false,
						DISPLAY_DMA_TRANSFER_WIDTH,
						kDMA_AddressInterleave1xWidth,
						kDMA_AddressInterleave1xWidth,
						DISPLAY_DMA_TRANSFER_LENGTH
						),
						// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_src->p[y]),
					// cppcheck-suppress [misra-c2012-11.8] cast to (void *) is valid
					(void *) &(fb_dst->p[y]),
					NULL);
#endif
	}
}

// See the header file for the function documentation
void DISPLAY_DMA_start(framebuffer_t *src, framebuffer_t *dst, int ymin, int ymax) {

	// DMA always copies the full content and is pre-configured (see DISPLAY_DMA_initialize())
	(void)ymin;
	(void)ymax;
	(void)dst;

	dma_descriptor_t    *p_dma_desc;

	DISPLAY_IMPL_notify_dma_start();

	if (src == s_frameBufferAddress[1]) {
		p_dma_desc = &dma_descriptors[DISPLAY_DMA_NB_DESCS];
	} else {
		p_dma_desc = &dma_descriptors[0];
	}

	DMA_SubmitChannelDescriptor(&g_DMA_Handle, p_dma_desc);
	DMA_StartTransfer(&g_DMA_Handle);
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __dma_callback(dma_handle_t *handle, void *param, bool transfer_done, uint32_t tcds)
{
	(void)handle;
	(void)param;
	(void)tcds;

	if (transfer_done)
	{
		uint8_t it = interrupt_enter();
		LLUI_DISPLAY_flushDone(true);
		DISPLAY_IMPL_notify_dma_stop();
		interrupt_leave(it);
	}
}

#endif // DISPLAY_DMA_ENABLED != 0

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
