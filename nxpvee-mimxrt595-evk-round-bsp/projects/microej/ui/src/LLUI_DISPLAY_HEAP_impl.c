/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief This MicroUI images heap allocator replaces the default allocator embedded in the
 * MicroUI Graphics Engine. It is using a best fit allocator and provides some additional APIs
 * to retrieve the heap information: total space, free space, number of blocks allocated.
 *
 * @see LLUI_DISPLAY_impl.h file comment
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.1.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "microui_heap.h"
#include "BESTFIT_ALLOCATOR.h"

#ifdef __cplusplus
extern "C" {
#endif

// --------------------------------------------------------------------------------
// Macros and Defines
// --------------------------------------------------------------------------------

/*
 * @brief The best fit allocator stores a main header of 68 bytes (0x44) in the heap.
 */
#define BESTFITALLOCATOR_HEADER_SIZE (68)

/*
 * @brief The best fit allocator stores a header before an allocated block and a footer
 * after. The header holds the block size plus the header and footer sizes. This macro
 * retrieves the block full size (header + data + footer).
 */
#define BESTFITALLOCATOR_BLOCK_SIZE(block) ((*(uint32_t*)((block)-sizeof(uint32_t))) & 0x7ffffff)

// --------------------------------------------------------------------------------
// Private fields
// --------------------------------------------------------------------------------

static BESTFIT_ALLOCATOR image_heap;
static uint32_t heap_size;
static uint32_t free_space;
static uint32_t allocated_blocks_number;

// --------------------------------------------------------------------------------
// microui_heap.h functions
// --------------------------------------------------------------------------------

uint32_t MICROUI_HEAP_total_space(void) {
	return heap_size;
}

uint32_t MICROUI_HEAP_free_space(void) {
	return free_space;
}

uint32_t MICROUI_HEAP_number_of_allocated_blocks(void) {
	return allocated_blocks_number;
}

// --------------------------------------------------------------------------------
// LLUI_DISPLAY_impl.h functions
// --------------------------------------------------------------------------------


void LLUI_DISPLAY_IMPL_image_heap_initialize(uint8_t* heap_start, uint8_t* heap_limit) {
	heap_size = heap_limit - heap_start - BESTFITALLOCATOR_HEADER_SIZE;
	free_space = heap_size;
	BESTFIT_ALLOCATOR_new(&image_heap);
	BESTFIT_ALLOCATOR_initialize(&image_heap, (int32_t)heap_start, (int32_t)heap_limit);
}

uint8_t* LLUI_DISPLAY_IMPL_image_heap_allocate(uint32_t size) {
	uint8_t* addr = (uint8_t*)BESTFIT_ALLOCATOR_allocate(&image_heap, (int32_t)size);

	if ((uint8_t*)0 != addr) {
		free_space -= BESTFITALLOCATOR_BLOCK_SIZE(addr);
		allocated_blocks_number++;
	}
	return addr;
}

void LLUI_DISPLAY_IMPL_image_heap_free(uint8_t* block) {
	free_space += BESTFITALLOCATOR_BLOCK_SIZE(block);
	allocated_blocks_number--;
	BESTFIT_ALLOCATOR_free(&image_heap, (void*)block);
}

// --------------------------------------------------------------------------------
// EOF
// --------------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
