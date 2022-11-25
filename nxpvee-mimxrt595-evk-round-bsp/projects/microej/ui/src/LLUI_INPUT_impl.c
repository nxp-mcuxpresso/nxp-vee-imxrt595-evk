/*
 * C
 *
 * Copyright 2019-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "LLUI_INPUT_impl.h"
#include "microej.h"
#include "buttons_manager.h"
#include "touch_manager.h"
#include "os_support.h"
#include "interrupts.h"

// RTOS
#include "FreeRTOS.h"
#include "semphr.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/*
 * @brief vglite operation semaphore
 */
static SemaphoreHandle_t g_sem_input;

// -----------------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

void LLUI_INPUT_IMPL_initialize(void)
{
	g_sem_input = xSemaphoreCreateBinary();
	xSemaphoreGive(g_sem_input); // first take must pass
	BUTTONS_MANAGER_initialize();
	TOUCH_MANAGER_initialize(); 
}

int32_t LLUI_INPUT_IMPL_getInitialStateValue(int32_t stateMachinesID, int32_t stateID)
{
	// no state on this BSP
	return 0;
}

void LLUI_INPUT_IMPL_enterCriticalSection()
{
	if (interrupt_is_in() == MICROEJ_FALSE)
	{
		xSemaphoreTake(g_sem_input, portMAX_DELAY);
		BUTTONS_MANAGER_disable_interrupts();
	}
}

void LLUI_INPUT_IMPL_leaveCriticalSection()
{
	if (interrupt_is_in() == MICROEJ_FALSE)
	{
		BUTTONS_MANAGER_enable_interrupts();
		xSemaphoreGive(g_sem_input);
	}
}

// -----------------------------------------------------------------------------
// Private functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

