/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "fsl_device_registers.h"

#include "FreeRTOS.h"
#include "task.h"
#include "interrupts.h"

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void OS_SUPPORT_disable_context_switching() {
	if (interrupt_is_in() == MICROEJ_TRUE) {
		taskENTER_CRITICAL_FROM_ISR();
		__get_BASEPRI(); portDISABLE_INTERRUPTS();
	} else {
		taskENTER_CRITICAL();
	}
}

// See the header file for the function documentation
void OS_SUPPORT_enable_context_switching() {
	if (interrupt_is_in() == MICROEJ_TRUE) {
		taskEXIT_CRITICAL_FROM_ISR(0);
		__set_BASEPRI( 0 );
	} else {
		taskEXIT_CRITICAL();
	}
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

