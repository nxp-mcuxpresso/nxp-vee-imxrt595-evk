/*
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdio.h>

#include "fsl_debug_console.h"

#include "LLBSP_impl.h"
#include "microej.h"

#include "mej_log.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define LLBSP_IS_PTR_IN_INTERNAL_RAM(p)\
	((p >= &__base_RAM) && (p < &__top_RAM))

#define LLBSP_IS_PTR_IN_EXTERNAL_RAM(p) \
	((p >= &__base_EXTRAM) && (p < &__top_EXTRAM))

#define LLBSP_IS_PTR_IN_RAM(p)					\
	(	0										\
		|| LLBSP_IS_PTR_IN_INTERNAL_RAM(ptr)	\
		|| LLBSP_IS_PTR_IN_EXTERNAL_RAM(ptr)	\
		)

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

/*
 * These symbols are exported by the linker
 */

/*
 * @brief start/end of the internal RAM
 */
extern uint32_t __base_RAM; // defined in linker script
extern uint32_t __top_RAM; // defined in linker script


/*
 * @brief start/end of the external RAM
 */
extern uint32_t __base_EXTRAM; // defined in linker script
extern uint32_t __top_EXTRAM; // defined in linker script

// -----------------------------------------------------------------------------
// Java Low Level APIs
// -----------------------------------------------------------------------------

// See the header file for the function documentation
uint8_t LLBSP_IMPL_isInReadOnlyMemory(void* ptr) {
	if (LLBSP_IS_PTR_IN_RAM(ptr)) {
		return MICROEJ_FALSE;
	} else { 
		return MICROEJ_TRUE;
	}
}

// See the header file for the function documentation
void LLBSP_IMPL_putchar(int32_t c) {
	PUTCHAR(c);
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

