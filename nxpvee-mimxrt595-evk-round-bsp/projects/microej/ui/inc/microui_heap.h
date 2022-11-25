/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief See LLUI_DISPLAY_HEAP_impl.c.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.1.0
 */

#if !defined MICROUI_HEAP_H
# define MICROUI_HEAP_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------

/*
 * @brief Returns the MicroUI image heap size in bytes.
 */
uint32_t MICROUI_HEAP_total_space(void);

/*
 * @brief Returns the MicroUI image heap free space size in bytes.
 *
 * Warnings: The total free space cannot contain a block whose size is equal to
 * the total free space:
 * 	- The best fit allocator adds a header and a footer for each allocated
 * 	  block.
 * 	- Consecutive malloc/free produce cause memory fragmentation (all the free
 * 	  blocks are not contiguous in the memory). The function returns the sum of
 * 	  all the free blocks.
 */
uint32_t MICROUI_HEAP_free_space(void);

/*
 * @brief Returns the number of blocks allocated.
 *
 * The MicroUI image heap is mainly used to allocate the pixels buffers of MicroUI
 * ResourceImages (images decoded dynamically at runtime, copy of images located in
 * a non-byte addressable memory, and MicroUI BufferedImages).
 */
uint32_t MICROUI_HEAP_number_of_allocated_blocks(void);

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
#endif // MICROUI_HEAP_H
