/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include <stdio.h>

#include "sni.h"

#include "framerate_impl.h"

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

#if FRAMERATE_ENABLED == 1
/*
 * @brief Scheduling periode of the framerate worker task (in ms).
 */
static uint32_t __framerate_schedule_period_ms = 0;	// means "not initialised"

/*
 * @brief Framerate counter
 */
static uint32_t __framerate_counter;

/*
 * @brief Last time the framerate worker task was scheduled (in ms).
 */
static uint32_t __framerate_last_ms;
#endif

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
int32_t framerate_init(int32_t schedule_periode)
{
#if FRAMERATE_ENABLED == 1
	if (__framerate_schedule_period_ms == 0)	// means "not initialized"
	{
		// fix globals
		__framerate_schedule_period_ms = schedule_periode;
		__framerate_counter = 0;
		__framerate_last_ms = 0;

		int32_t ret = framerate_impl_start_task();
		if (ret != FRAMERATE_OK)
		{
			// an error has occurred
			return ret;
		}
	}
	// else: no error because already initialized

	return FRAMERATE_OK;

#else
	return FRAMERATE_ERROR;
#endif
}

// See the header file for the function documentation
void framerate_increment(void)
{
#if FRAMERATE_ENABLED == 1
	__framerate_counter++;
#endif
}

// See the header file for the function documentation
uint32_t framerate_get(void)
{
#if FRAMERATE_ENABLED == 1
	return __framerate_last_ms;
#else
	return 0;
#endif
}

// See the header file for the function documentation
void framerate_task_work(void)
{
#if FRAMERATE_ENABLED == 1
	while(1)
	{
		// sleep during n milliseconds
		framerate_impl_sleep(__framerate_schedule_period_ms);

		// update global counter
		__framerate_last_ms = (int32_t)(__framerate_counter
				* ((float) 1000 / __framerate_schedule_period_ms));

		// reset framerate counter
		__framerate_counter = 0;
	}
#endif
}

// -----------------------------------------------------------------------------
// Java functions
// -----------------------------------------------------------------------------

/*
 * @brief Java interface to initialize the framerate module
 */
jint javaFramerateInit(jint schedule_periode)
{
	return (jint) framerate_init((int32_t) schedule_periode);
}

/*
 * @brief Java interface to get the framerate
 */
uint32_t javaFramerateGet(void)
{
	return framerate_get();
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

