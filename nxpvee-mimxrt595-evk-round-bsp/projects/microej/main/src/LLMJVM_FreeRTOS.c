/*
 * C
 *
 * Copyright 2013-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * This implementation uses a dedicated hardware timer for time implementation,
 * instead of internal FreeRTOS timer which is a 32bits timer.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "timers.h"
#include "task.h"
#include "semphr.h"

#include "LLMJVM_impl.h"
#include "microej.h"
#include "time_hardware_timer.h"
#include "interrupts.h"

#include "power_manager.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * Useful macros and definitions
 */

// ID for the FreeRTOS Timer
#define WAKE_UP_TIMER_ID	42

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

/*
 * Shared variables
 */

/*
 * @brief Application time offset
 */
static int64_t LLMJVM_FREERTOS_application_time_offset = 0;

/*
 * @brief Absolute time in ticks at which timer will be launched
 */
static int64_t LLMJVM_FREERTOS_next_wake_up_time = INT64_MAX;

/*
 * @brief Timer for scheduling next alarm
 */
static TimerHandle_t LLMJVM_FREERTOS_wake_up_timer;

/*
 * @brief Binary semaphore to wakeup microJVM
 */
static SemaphoreHandle_t LLMJVM_FREERTOS_Semaphore;

// -----------------------------------------------------------------------------
// Internal functions definition
// -----------------------------------------------------------------------------

/*
 * @brief Since LLMJVM_schedule() prototype does not match the timer callback prototype,
 * we create a wrapper around it and check the ID of the timer.
 *
 * @param[in] timer: handler on the timer calling the callback.
 */
static void __wake_up_timer_callback(TimerHandle_t timer);

/*
 * @brief Convert a time in milliseconds in ticks.
 * Result is round up to the next tick (ie. converting back the resulting ticks
 * to milliseconds results in a value greater than or equals to given time).
 *
 * @param[in] time_ms: Time to convert (in milliseconds)
 */
static int64_t __convert_ms_to_tick(int64_t time_ms);

// -----------------------------------------------------------------------------
// Java Low Level APIs
// -----------------------------------------------------------------------------

/*
 * Implementation of functions from LLMJVM_impl.h
 * and other helping functions.
 */

// See the header file for the function documentation
int32_t LLMJVM_IMPL_initialize() {
	// Create a timer to scheduler alarm for the VM
	LLMJVM_FREERTOS_wake_up_timer = xTimerCreate(
			NULL,						// timer name
			100,						// timer period in (FreeRTOS) ticks
			pdFALSE,					// auto reload
			(void*) WAKE_UP_TIMER_ID,	// timer Id
			__wake_up_timer_callback	// callback function
			);

	if(LLMJVM_FREERTOS_wake_up_timer == 0) {
		return LLMJVM_ERROR;
	}

	vSemaphoreCreateBinary(LLMJVM_FREERTOS_Semaphore);

	if(LLMJVM_FREERTOS_Semaphore == NULL) {
		return LLMJVM_ERROR;
	}

	// Created semaphore is in a state such that the first call to 'take' the semaphore would pass.
	// Just execute a take to init it at 0.
	// Note: From FreeRTOS version 7.6.0, the function xSemaphoreCreateBinary can be used to avoid
	// the call to xSemaphoreTake.
	xSemaphoreTake(LLMJVM_FREERTOS_Semaphore, portMAX_DELAY);

	time_hardware_timer_wait_ready();

	// initialization is complete: power optimizations can be enabled.
	power_manager_enable_low_power(POWER_GLOBAL, LOW_POWER_AUTHORIZED);

	return LLMJVM_OK;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_vmTaskStarted() {
	return LLMJVM_OK;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_scheduleRequest(int64_t absoluteTime) {
	int64_t relativeTime;
	int64_t relativeTick; // relative from current time to 'absoluteTime'

	portBASE_TYPE xTimerChangePeriodResult;
	portBASE_TYPE xTimerStartResult;

	//First check if absolute time is lower than current schedule time
	if(absoluteTime < LLMJVM_FREERTOS_next_wake_up_time) {
		// Save new alarm absolute time
		LLMJVM_FREERTOS_next_wake_up_time = absoluteTime;

		// Stop current timer (no delay)
		xTimerStop(LLMJVM_FREERTOS_wake_up_timer, 0);

		// Determine relative time/tick
		int64_t currentTime = LLMJVM_IMPL_getCurrentTime(MICROEJ_TRUE);
		relativeTime = absoluteTime - currentTime;
		relativeTick = __convert_ms_to_tick(relativeTime);  // = number of ticks before absoluteTime

		if(relativeTick > 0) {
			// Schedule the new alarm
			xTimerChangePeriodResult = xTimerChangePeriod(LLMJVM_FREERTOS_wake_up_timer, relativeTick, 0);
			xTimerStartResult = xTimerStart(LLMJVM_FREERTOS_wake_up_timer, 0);

			if( (xTimerChangePeriodResult == pdPASS) && (xTimerStartResult == pdPASS) )
				return LLMJVM_OK;
			else
				return LLMJVM_ERROR;
		} else {
			// 'absoluteTime' has been reached already, notify the VM now
			return LLMJVM_schedule();
		}
	}
	// else : nothing to do. An sooner alarm is scheduled.

	return LLMJVM_OK;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_idleVM() {
	portBASE_TYPE res = xSemaphoreTake(LLMJVM_FREERTOS_Semaphore, portMAX_DELAY);

	return res == pdTRUE ? LLMJVM_OK : LLMJVM_ERROR;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_wakeupVM() {
	portBASE_TYPE res;
	if(interrupt_is_in() == MICROEJ_TRUE) {
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		res = xSemaphoreGiveFromISR(LLMJVM_FREERTOS_Semaphore, &xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken != pdFALSE ) {
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	} else {
		res = xSemaphoreGive(LLMJVM_FREERTOS_Semaphore);
	}

	LLMJVM_FREERTOS_next_wake_up_time = INT64_MAX;

	return res == pdTRUE ? LLMJVM_OK : LLMJVM_ERROR;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_ackWakeup() {
	return LLMJVM_OK;
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_getCurrentTaskID() {
	return (int32_t) xTaskGetCurrentTaskHandle();
}

// See the header file for the function documentation
void LLMJVM_IMPL_setApplicationTime(int64_t t) {
    int64_t currentTime = LLMJVM_IMPL_getCurrentTime(MICROEJ_TRUE);
	LLMJVM_FREERTOS_application_time_offset = t - currentTime;
}

// See the header file for the function documentation
int64_t LLMJVM_IMPL_getCurrentTime(uint8_t system) {
    int64_t systemTime = time_hardware_timer_getCurrentTime();
    return system ? systemTime : systemTime + LLMJVM_FREERTOS_application_time_offset;
}

// See the header file for the function documentation
int64_t LLMJVM_IMPL_getTimeNanos() {
	return time_hardware_timer_getTimeNanos();
}

// See the header file for the function documentation
int32_t LLMJVM_IMPL_shutdown(void) {
	// nothing to do
	return LLMJVM_OK;
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __wake_up_timer_callback(TimerHandle_t timer) {
	uint32_t id = (uint32_t) pvTimerGetTimerID(timer);

	if(id == WAKE_UP_TIMER_ID) {
		LLMJVM_schedule();
	}
}

// See the section 'Internal function definitions' for the function documentation
static int64_t __convert_ms_to_tick(int64_t time_ms) {
	int64_t timeus = time_ms * 1000;
	int64_t os_tick = 1000000 / configTICK_RATE_HZ; // tick in microseconds
	return (timeus + (os_tick-1)) / os_tick;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

