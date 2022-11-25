/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * The MicroUI framework is using an internal buffer to store the inputs events. The event
 * will not be added if the internal events buffer is full. In this case the input driver
 * has to adapt itself in order to not send a future invalid event. For instance, if a
 * PRESSED event is not sent, the input driver has not to send a REPEAT or RELEASE event.
 * So it may have a distinction between the real input state and the "software" input
 * state.
 *
 * When an event has not been managed because the system events queue is full:
 * - increase the size of the system events queue (see Memory page of the application launcher
 * - limit rate of events of your drivers (for example, calibrate buttons to send an event
 * at most every X ms)
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "LLUI_INPUT.h"
#include "microej.h"
#include "buttons_helper_configuration.h"
#include "event_generator.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// Number of pixels to generate a move after a press
#ifndef BUTTONS_HELPER_NUMBER_OF_BUTTONS
#error "Please set the define NUMBER_OF_BUTTONS in touch_helper_configuration.h"
#endif

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

/*
 * @brief button states
 * */
static int buttons_pressed[BUTTONS_HELPER_NUMBER_OF_BUTTONS];

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void BUTTONS_HELPER_initialize(void)
{
	// Initialize the button states
	for(int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++)
	{
		buttons_pressed[i] = MICROEJ_FALSE;
	}
}

// See the header file for the function documentation
void BUTTONS_HELPER_pressed(int32_t button_id)
{
	// button is pressed
	if (buttons_pressed[button_id] == MICROEJ_TRUE)
	{
		// button was pressed => repeat event  (don't care if event is lost)
		EVENT_GENERATOR_button_repeated(button_id);
	}
	else
	{
		// button was released => press event
		if (EVENT_GENERATOR_button_pressed(button_id) == LLUI_INPUT_OK)
		{
			// the event has been managed: we can store the new button state
			// button is pressed now
			buttons_pressed[button_id] = MICROEJ_TRUE;
		}
		// else: event has been lost: stay in "release" state
	}
}

// See the header file for the function documentation
void BUTTONS_HELPER_repeated(int32_t button_id)
{
	// manage this repeat event like a press event in order to check "software" button state
	BUTTONS_HELPER_pressed(button_id);
}

// See the header file for the function documentation
void BUTTONS_HELPER_released(int32_t button_id)
{
	// button is now released

	if (buttons_pressed[button_id] == MICROEJ_TRUE)
	{
		// button was pressed => release event
		if (EVENT_GENERATOR_button_released(button_id) == LLUI_INPUT_OK)
		{
			// the event has been managed: we can store the new button state
			// button is released now
			buttons_pressed[button_id] = MICROEJ_FALSE;
		}
		// else: event has been lost: stay in "press" state
	}
	// else: already released
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

