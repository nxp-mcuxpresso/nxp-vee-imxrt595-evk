/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * Implementation for FreeRTOS
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "framerate_conf.h"
#ifdef FRAMERATE_ENABLED

#include "FreeRTOS.h"
#include "task.h"
#include "framerate_impl.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define FRAMERATE_STACK_SIZE ( 4*1024 )
#define FRAMERATE_TASK_PRIORITY      ( 11 )
#define FRAMERATE_TASK_STACK_SIZE     FRAMERATE_STACK_SIZE/4

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

static void _framerate_task(void * pvParameters);

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

int32_t framerate_impl_start_task(void)
{
	BaseType_t xReturn = xTaskCreate( _framerate_task, "FRAMERATE", FRAMERATE_TASK_STACK_SIZE, NULL, FRAMERATE_TASK_PRIORITY, NULL );
	return xReturn == pdPASS ? FRAMERATE_OK : FRAMERATE_ERROR;
}

void framerate_impl_sleep(uint32_t ms)
{
	TickType_t ticks = ms / portTICK_PERIOD_MS;
	vTaskDelay(ticks ? ticks : 1);          /* Minimum delay = 1 tick */
	return;
}

#endif

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

static void _framerate_task(void * pvParameters)
{
	// launch framerate job
	framerate_task_work();
	// job end, cleanup resources
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

