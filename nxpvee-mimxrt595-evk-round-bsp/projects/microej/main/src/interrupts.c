/*
 * C
 *
 * Copyright 2013-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "interrupts.h"

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

/*
 * @brief Flag indicating if the current execution is from an interrupt
 * sevice routine
 */
static volatile uint8_t is_in_ISR = MICROEJ_FALSE;

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
uint8_t interrupt_is_in(void) {
	return is_in_ISR;
}

// See the header file for the function documentation
uint8_t interrupt_enter(void) {
	if (is_in_ISR == MICROEJ_TRUE) {
		return MICROEJ_FALSE;
	} else {
		is_in_ISR = MICROEJ_TRUE;
		return MICROEJ_TRUE;
	}	
}

// See the header file for the function documentation
void interrupt_leave(uint8_t leave) {
	if (leave == MICROEJ_TRUE) {
		is_in_ISR = MICROEJ_FALSE;
	}	
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

