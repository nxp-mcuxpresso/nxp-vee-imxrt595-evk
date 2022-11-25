/*
 * C
 *
 * Copyright 2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ Harfbuzz memory allocation functions.
* @author MicroEJ Developer Team
* @version 1.0.0
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "BESTFIT_ALLOCATOR.h"

#include "microvg_helper.h"
#include "microvg_configuration.h"

#if defined (VG_FEATURE_FONT_COMPLEX_LAYOUT)

#define HB_HEAP_SIZE    VG_FEATURE_FONT_COMPLEX_LAYOUT_HEAP_SIZE


BESTFIT_ALLOCATOR harfbuzzAllocatorInstance;

static uint8_t _hb_heap[HB_HEAP_SIZE];

#define HB_HEAP_START   ( &_hb_heap[0] )
#define HB_HEAP_END     ( &_hb_heap[HB_HEAP_SIZE] )

#ifdef __cplusplus
extern "C" {
#endif


static int first = 1;

#ifdef MICROVG_MONITOR_HEAP
static uint32_t current_heap_size = 0;
#endif

void* hb_malloc_impl (size_t size)
{
	if(first)
	{
		 BESTFIT_ALLOCATOR_new(&harfbuzzAllocatorInstance);
		 BESTFIT_ALLOCATOR_initialize(&harfbuzzAllocatorInstance, (uint32_t) HB_HEAP_START, (uint32_t) HB_HEAP_END);
		 first = 0;
	}

	void * ptr = BESTFIT_ALLOCATOR_allocate(&harfbuzzAllocatorInstance, size);

	#ifdef MICROVG_MONITOR_HEAP
    if(NULL != ptr) {
		current_heap_size += size;
		MEJ_LOG_INFO_MICROVG("HB Heap - alloc -> size= %d\n", current_heap_size);
    }
	#endif
	return ptr;
}

void* hb_calloc_impl (size_t nmemb, size_t size)
{
    int sizeBytes = nmemb * size;
	void *ptr = hb_malloc_impl(sizeBytes);
    memset(ptr, 0, sizeBytes);

	return ptr;
}

void  hb_free_impl (void *block)
{

	if(block){

		#ifdef MICROVG_MONITOR_HEAP
		uint32_t * ptr  = (uint32_t*) block;
		current_heap_size -= *(ptr -1) & 0x7FFFFFFF;
		MEJ_LOG_INFO_MICROVG("HB Heap - free -> size= %d\n", current_heap_size);
		#endif

		BESTFIT_ALLOCATOR_free(&harfbuzzAllocatorInstance, block);
	}
}

void* hb_realloc_impl (void *block, size_t size)
{


	if(!block){
		return hb_malloc_impl(size);
	} else {
	void* _pNewBock = block;
	_pNewBock =hb_malloc_impl(size);

	int previous_size = ((* ((int*)(block)-1)) & 0xffffff) - 8;
	if (_pNewBock != NULL) {
		memcpy(_pNewBock, block, previous_size);
	}

	hb_free_impl(block);

	return _pNewBock;
	}
}

#endif // VG_FEATURE_FONT_COMPLEX_LAYOUT

#ifdef __cplusplus
}
#endif
