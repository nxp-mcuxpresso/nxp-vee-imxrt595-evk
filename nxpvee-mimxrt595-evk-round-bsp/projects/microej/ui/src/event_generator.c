/*
 * C
 *
 * Copyright 2019-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * This file converts the input events in MicroUI Event Generator events.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <string.h>

#include "LLUI_INPUT.h"

#include "time_hardware_timer.h"

#include "buttons_helper_configuration.h"

#include "event_generator.h"

// this h file is created by buildSystemMicroUI step
#include "microui_constants.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * Specify which MicroUI Command to send
 */
#define BUTTON_COMMAND_EVENT(buttonId)	(buttonId == 0 /*SW1*/ ? LLUI_INPUT_COMMAND_DOWN : LLUI_INPUT_COMMAND_UP)

#if defined EVENT_GENERATOR_SPECIAL_FUNCTION

/*
 * Minimum time between two call of the special callback
 */
#define __SPECIAL_CB_MIN_DELAY_NS       (300*1000*1000)  // 100ms = 300*1000*1000 nano seconds

#define EVENT_GENERATOR_NB_CALLBACKS BUTTONS_HELPER_NUMBER_OF_BUTTONS

#endif

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/* Special function ----------------------------------------------------------*/

#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
static EVENT_GENERATOR_special_cb_t special_cbs[EVENT_GENERATOR_NB_CALLBACKS];
static int button_pressed = 0;
static int64_t last_special_cb_time = 0;
#endif

// -----------------------------------------------------------------------------
// Internal functions definition
// -----------------------------------------------------------------------------

#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
/*
 * @brief Calls the special callback.
 *
 * This function checks that the previous call to the special callback is more
 * than __SPECIAL_CB_MIN_DELAY_NS.
 */
static void __call_special_cb(void);
#endif

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
int EVENT_GENERATOR_register_special_cb(
		EVENT_GENERATOR_special_cb_t i_special_cb,
		unsigned int event) {
	if (event >= EVENT_GENERATOR_NB_CALLBACKS) {
		return -1;
	}

	special_cbs[event] = i_special_cb;

	return event;
}
#endif

/* Buttons -------------------------------------------------------------------*/

int32_t EVENT_GENERATOR_button_pressed(int32_t buttonId)
{
# if defined EVENT_GENERATOR_SPECIAL_FUNCTION
	button_pressed |= (1 << buttonId);
# endif // defined EVENT_GENERATOR_SPECIAL_FUNCTION

	// do not send a Command on press event

	return LLUI_INPUT_OK; // the event has been managed
}

int32_t EVENT_GENERATOR_button_repeated(int32_t buttonId)
{
	// do not send a Command on repeat event
	return LLUI_INPUT_OK; // the event has been managed
}

int32_t EVENT_GENERATOR_button_released(int32_t buttonId)
{

# if defined EVENT_GENERATOR_SPECIAL_FUNCTION
	button_pressed &= ~(1 << buttonId);

	if (last_special_cb_time != 0) {
		last_special_cb_time = 0;
		return LLUI_INPUT_OK;
	}
# endif // defined EVENT_GENERATOR_SPECIAL_FUNCTION

	// send a MicroUI Command event
	return LLUI_INPUT_sendCommandEvent(MICROUI_EVENTGEN_COMMANDS, BUTTON_COMMAND_EVENT(buttonId));
}

/* Touch ---------------------------------------------------------------------*/

int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y)
{
#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
	if (button_pressed) {
		__call_special_cb();
		return LLUI_INPUT_OK;
	}
#endif

	return LLUI_INPUT_sendTouchPressedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y)
{
#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
	if (button_pressed) {
		__call_special_cb();
		return LLUI_INPUT_OK;
	}
#endif

	return LLUI_INPUT_sendTouchMovedEvent(MICROUI_EVENTGEN_TOUCH, x, y);
}

int32_t EVENT_GENERATOR_touch_released(void)
{
#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
	if (button_pressed) {
		return LLUI_INPUT_NOK;
	}
#endif

	return LLUI_INPUT_sendTouchReleasedEvent(MICROUI_EVENTGEN_TOUCH);
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

#if defined EVENT_GENERATOR_SPECIAL_FUNCTION
// See the section 'Internal function definitions' for the function documentation
static void __call_special_cb(void)
{
	int64_t special_cb_time;

	if (button_pressed) {
		special_cb_time = time_hardware_timer_getTimeNanos();

		if ((special_cb_time - last_special_cb_time) > __SPECIAL_CB_MIN_DELAY_NS) {
			for (int i = 0, temp = button_pressed; temp > 0; i++, temp >>= 1) {
				if ((temp & 1) && (special_cbs[i] != 0)) {

					special_cbs[i]((void *) i);
				}
			}
		}
		last_special_cb_time = special_cb_time;
	}
}
#endif

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

