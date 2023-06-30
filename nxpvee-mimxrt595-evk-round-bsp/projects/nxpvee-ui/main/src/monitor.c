/*
 * C
 *
 * Copyright 2013-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"

#include "monitor.h"

#include "buttons_manager.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#if MONITOR_ENABLED == 1

#define MONITOR_STACK_SIZE          ( 2048 )
#define MONITOR_TASK_PRIORITY       ( 15 )
#define MONITOR_TASK_STACK_SIZE     ( MONITOR_STACK_SIZE/4 )

#endif // MONITOR_ENABLED == 1

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

#if MONITOR_ENABLED == 1

static void _monitor_task(void * pvParameters);

#endif // MONITOR_ENABLED == 1

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

#if MONITOR_ENABLED == 1

int32_t monitor_init(void) {
  
#if CPULOAD_ENABLED == 1
	cpuload_init();
#endif

#if FRAMERATE_ENABLED == 1
	framerate_init(MONITOR_SCHEDULE_TIME);
#endif

	BaseType_t xReturn = xTaskCreate( _monitor_task, "MONITOR", MONITOR_TASK_STACK_SIZE, NULL, MONITOR_TASK_PRIORITY, NULL );
	return xReturn == pdPASS ? 0 : -1;
}

#endif // MONITOR_ENABLED == 1

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

#if MONITOR_ENABLED == 1

// See the header file for the function documentation
static void _monitor_task(void * pvParameters) {
	TickType_t delay = MONITOR_SCHEDULE_TIME / portTICK_PERIOD_MS;
    TickType_t xNextWakeTime;
    xNextWakeTime = xTaskGetTickCount();

	while(1) {
		vTaskDelayUntil(&xNextWakeTime, delay);

#if CPULOAD_ENABLED == 1
		uint32_t cpuload = cpuload_get();
		PRINTF("cpuload: %d ", cpuload);
#endif

#if FRAMERATE_ENABLED == 1
		uint32_t fps = framerate_get();
		PRINTF("fps: %d ", fps);
#endif

		PRINTF("\n");
	}
}

#endif // MONITOR_ENABLED == 1

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

