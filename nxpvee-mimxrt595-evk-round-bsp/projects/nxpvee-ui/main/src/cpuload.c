/*
 * C
 *
 * Copyright 2014-2022 MicroEJ Corp. All rights reserved.
* Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "fsl_common.h"
#include "fsl_debug_console.h"

#include "cpuload_impl.h"

#include "mej_log.h"
#include "mej_math.h"

#include "time_hardware_timer.h"

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

#ifdef CPULOAD_ENABLED
static volatile uint32_t cpuload_schedule_time;
static volatile uint32_t cpuload_last_load;
static volatile uint32_t cpuload_reference_counter;
static volatile uint32_t cpuload_idle_counter;
static volatile uint32_t cpuload_sleep_counter;
static volatile uint32_t cpuload_ask_counter;
static volatile int64_t sleep_start;
#endif

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void cpuload_idle(void) {

#ifdef CPULOAD_ENABLED
  uint32_t regPrimask;
  regPrimask = DisableGlobalIRQ();
  cpuload_idle_counter++;
  EnableGlobalIRQ(regPrimask);
#endif
}

void cpuload_enter_sleep(void) {
#ifdef CPULOAD_ENABLED
sleep_start = time_hardware_timer_getCurrentTime();
#endif
}

void cpuload_exit_sleep(void) {
#ifdef CPULOAD_ENABLED
	cpuload_sleep_counter += time_hardware_timer_getCurrentTime() - sleep_start;
#endif
}

// See the header file for the function documentation
int32_t cpuload_init(void) {
#if CPULOAD_ENABLED == 1
	// get reference time (must be done before creating task)
	// Note: To get the reference time, this implementation request a sleep
	// for a fraction of the schedule time.
	// This reference time is then normalized according to the fraction.
	cpuload_impl_start_idle_task();
	cpuload_impl_sync_os_tick();
	cpuload_idle_counter = 0;
	cpuload_impl_sleep(CPULOAD_SCHEDULE_TIME_MS/CPULOAD_INIT_SCHEDULE_FRACTION);

	if (cpuload_idle_counter == 0) {
		// it is an error: this counter must have been updated
		// during the previous sleep.
		MEJ_LOG_MODULE_ERROR(MAIN, "CPU load startup: invalid idle counter value: %d\n", cpuload_idle_counter);
		return CPULOAD_INVALID_COUNTER;
	}

	// fix globals
	cpuload_schedule_time = CPULOAD_SCHEDULE_TIME_MS;
	cpuload_reference_counter = (cpuload_idle_counter*CPULOAD_INIT_SCHEDULE_FRACTION)+5;
	cpuload_idle_counter = 0;
	cpuload_last_load = 0;
	cpuload_ask_counter = 0;
	cpuload_sleep_counter = 0;

	// create task
	if (cpuload_impl_start_task() != 0) {
		// an error has occurred
		return CPULOAD_START_TASK_ERROR;
	}

	return CPULOAD_OK;

#else
	return CPULOAD_NOT_ENABLED;
#endif
}

// See the header file for the function documentation
uint32_t cpuload_get(void) {
#if CPULOAD_ENABLED == 1
	uint32_t ret = cpuload_last_load;
	// clear average
	cpuload_ask_counter = 0;
	return ret;
#else
	return 0;
#endif
}

// See the header file for the function documentation
void cpuload_task(void) {
#if CPULOAD_ENABLED == 1
	while(1) {
		// sleep during n milliseconds
		cpuload_impl_sleep(cpuload_schedule_time);

		// average computing
		uint32_t average_compute = cpuload_last_load * cpuload_ask_counter;
		uint32_t last_average;

		if (cpuload_sleep_counter == 0) {
			// Low power disabled.
			last_average = 100 - MEJ_PERCENT(cpuload_idle_counter, cpuload_reference_counter);
		} else {
			// Low power is enabled. Default algo would report 100% cpuload.
			// Compute cpu load based on time spent in sleep
			last_average = 100 - MEJ_PERCENT(cpuload_sleep_counter, cpuload_schedule_time);
		}
		cpuload_ask_counter++;
		cpuload_last_load = (average_compute + last_average) / cpuload_ask_counter;

		// reset cpuload counter
		cpuload_idle_counter = 0;
		cpuload_sleep_counter = 0;
	}
#endif
}

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

/*
 * @brief Java API to initialize the CPU Load module
 *
 * @returns: see cpuload_init documentation for return codes
 */
uint32_t javaCPULoadInit(void) {
	return cpuload_init();
}

/*
 * @brief Java API to get the current CPU Load
 *
 * @returns: see cpuload_get documentation for return codes
 */
uint32_t javaCPULoadGet(void) {
	return cpuload_get();
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

