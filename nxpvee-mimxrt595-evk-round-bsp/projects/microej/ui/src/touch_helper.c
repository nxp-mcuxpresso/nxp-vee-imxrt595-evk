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
#include "touch_helper_configuration.h"
#include "event_generator.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// Number of pixels to generate a move after a press
#ifndef FIRST_MOVE_PIXEL_LIMIT
#error "Please set the define FIRST_MOVE_PIXEL_LIMIT in touch_helper_configuration.h"
#endif

// Number of pixels to generate a move after a move
#ifndef MOVE_PIXEL_LIMIT
#error "Please set the define MOVE_PIXEL_LIMIT in touch_helper_configuration.h"
#endif

#define DIFF(a,b)					((a) < (b) ? (b-a) : (a-b))
#define KEEP_COORD(p,n,limit)		(DIFF(p,n) <= limit ? MICROEJ_FALSE : MICROEJ_TRUE)
#define KEEP_PIXEL(px,x,py,y,limit)	(KEEP_COORD(px,x,limit) || KEEP_COORD(py,y,limit))
#define KEEP_FIRST_MOVE(px,x,py,y)	(KEEP_PIXEL(px,x,py,y, FIRST_MOVE_PIXEL_LIMIT))
#define KEEP_MOVE(px,x,py,y)		(KEEP_PIXEL(px,x,py,y, MOVE_PIXEL_LIMIT))

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

static uint8_t touch_pressed = MICROEJ_FALSE;
static uint8_t touch_moved = MICROEJ_FALSE;	// == MICROEJ_TRUE after the first "move" event
static uint16_t previous_touch_x, previous_touch_y;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

void TOUCH_HELPER_pressed(int32_t x, int32_t y)
{
	// here, pen is down for sure

	if (touch_pressed == MICROEJ_TRUE)
	{
		// pen was down => move event

		// keep pixel according first "move" event or not
		int keep_pixel;
		if(touch_moved == MICROEJ_TRUE)
		{
			keep_pixel = KEEP_MOVE(previous_touch_x, x, previous_touch_y, y);
		}
		else
		{
			keep_pixel = KEEP_FIRST_MOVE(previous_touch_x, x, previous_touch_y, y);
		}

		if (keep_pixel == MICROEJ_TRUE)
		{
			// store the new pixel
			previous_touch_x = x;
			previous_touch_y = y;
			touch_moved = MICROEJ_TRUE;

			// send a MicroUI touch event (don't care if event is lost)
			EVENT_GENERATOR_touch_moved(x, y);
		}
		// else: same position; no need to send an event
	}
	else
	{
		// pen was up => press event
		if (EVENT_GENERATOR_touch_pressed(x, y) == LLUI_INPUT_OK)
		{
			// the event has been managed: we can store the new touch state
			// touch is pressed now
			previous_touch_x = x;
			previous_touch_y = y;
			touch_pressed = MICROEJ_TRUE;
			touch_moved = MICROEJ_FALSE;
		}
		// else: event has been lost: stay in "release" state
	}
}

void TOUCH_HELPER_moved(int32_t x, int32_t y)
{
	// manage this move like a press event in order to check "software" touch state
	TOUCH_HELPER_pressed(x, y);
}

void TOUCH_HELPER_released(void)
{
	// here, pen is up for sure

	if (touch_pressed == MICROEJ_TRUE)
	{
		// pen was down => release event
		if (EVENT_GENERATOR_touch_released() == LLUI_INPUT_OK)
		{
			// the event has been managed: we can store the new touch state
			// touch is released now
			touch_pressed = MICROEJ_FALSE;
		}
		// else: event has been lost: stay in "press | move" state
	}
	// else: pen was already up
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

