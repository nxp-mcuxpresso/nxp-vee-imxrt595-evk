/*
 * C
 *
 * Copyright 2013-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef _INTERRUPTS
#define _INTERRUPTS

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "microej.h"

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

/**
 * This function must be called when entering in an interrupt.
 *
 * @return: MICROEJ_TRUE if already in an interrupt, MICROEJ_FALSE otherwise.
 */
uint8_t interrupt_enter(void);


/**
 * This function must be called when leaving an interrupt.
 *
 * @param[in] leave: if equals MICROEJ_TRUE, set flag is_in_ISR to MCROEJ_FALSE.
 */
void interrupt_leave(uint8_t leave);

/**
 * This function returns MICROEJ_TRUE or MICROEJ_FALSE to indicate
 * if an ISR is currently executed.
 *
 * @return: MICROEJ_TRUE if the function is executing from an interrupt.
 */
uint8_t interrupt_is_in(void); 

#endif

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

