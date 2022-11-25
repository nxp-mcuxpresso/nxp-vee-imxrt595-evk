/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */


/**
 * @file
 * @brief This MicroUI Events decoder describes the events to the standard output stream.
 *
 * @see LLUI_INPUT_LOG_impl.c file comment
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.1.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

// calls Microui events decoder functions
#include "microui_event_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MICROUIEVENTDECODER_ENABLED

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Default traces.
 */
#define DESCRIBE_EVENT_GENERATOR(event) (LLUI_DEBUG_TRACE(" (event generator %u)", EVENT_GENERATOR_ID(event)))
#define DESCRIBE_EOL() (LLUI_DEBUG_TRACE("\n"))

/*
 * @brief Decodes MicroUI event.
 */
#define EVENT_DATA(event) ((uint16_t)(event))
#define EVENT_GENERATOR_ID(event) ((uint8_t)((event) >> 16))

/*
 * @brief Decodes MicroUI Command event.
 */
#define COMMAND_GET(event) EVENT_DATA(event)

/*
 * @brief Decodes MicroUI Buttons event.
 */
#define BUTTON_ACTION_PRESSED 0
#define BUTTON_ACTION_RELEASED 1
#define BUTTON_ACTION_LONG 2
#define BUTTON_ACTION_REPEATED 3
#define BUTTON_ACTION_CLICK 4
#define BUTTON_ACTION_DOUBLECLICK 5
#define BUTTON_ACTION(event) ((uint8_t)((event) >> 8))
#define BUTTON_ID(event) ((uint8_t)(event))

/*
 * @brief Decodes MicroUI Pointer event.
 */
#define POINTER_ACTION_MOVE 6
#define POINTER_ACTION_DRAG 7
#define POINTER_X(data) (((data) >> 16) & 0xfff)
#define POINTER_Y(data) ((data) & 0xfff)
#define POINTER_TYPE(data) (((data) >> 31) & 0x1)

/*
 * @brief Decodes MicroUI user event.
 */
#define USEREVENT_SIZE(event) EVENT_DATA(event)

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

static void decode_event_command(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	(void)index;
	(void)fct_data_decoder;

	LLUI_DEBUG_TRACE("Command ");

	uint8_t command = (uint8_t)COMMAND_GET(event);

	switch(command) {
	case 0:
		LLUI_DEBUG_TRACE("ESC");
		break;
	case 1:
		LLUI_DEBUG_TRACE("BACK");
		break;
	case 2:
		LLUI_DEBUG_TRACE("UP");
		break;
	case 3:
		LLUI_DEBUG_TRACE("DOWN");
		break;
	case 4:
		LLUI_DEBUG_TRACE("LEFT");
		break;
	case 5:
		LLUI_DEBUG_TRACE("RIGHT");
		break;
	case 6:
		LLUI_DEBUG_TRACE("SELECT");
		break;
	case 7:
		LLUI_DEBUG_TRACE("CANCEL");
		break;
	case 8:
		LLUI_DEBUG_TRACE("HELP");
		break;
	case 9:
		LLUI_DEBUG_TRACE("MENU");
		break;
	case 10:
		LLUI_DEBUG_TRACE("EXIT");
		break;
	case 11:
		LLUI_DEBUG_TRACE("START");
		break;
	case 12:
		LLUI_DEBUG_TRACE("STOP");
		break;
	case 13:
		LLUI_DEBUG_TRACE("PAUSE");
		break;
	case 14:
		LLUI_DEBUG_TRACE("RESUME");
		break;
	case 15:
		LLUI_DEBUG_TRACE("COPY");
		break;
	case 16:
		LLUI_DEBUG_TRACE("CUT");
		break;
	case 17:
		LLUI_DEBUG_TRACE("PASTE");
		break;
	case 18:
		LLUI_DEBUG_TRACE("CLOCKWISE");
		break;
	case 19:
		LLUI_DEBUG_TRACE("ANTICLOCKWISE");
		break;
	case 20:
		LLUI_DEBUG_TRACE("PREVIOUS");
		break;
	case 21:
		LLUI_DEBUG_TRACE("NEXT");
		break;
	default:
		LLUI_DEBUG_TRACE("0x%02x", command);
		break;
	}

	DESCRIBE_EVENT_GENERATOR(event);
	DESCRIBE_EOL();
}

static void decode_event_button(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	(void)index;
	(void)fct_data_decoder;

	LLUI_DEBUG_TRACE("Button %u ", BUTTON_ID(event));
	uint8_t action = BUTTON_ACTION(event);

	switch(action) {
	case BUTTON_ACTION_PRESSED:
		LLUI_DEBUG_TRACE("pressed");
		break;
	case BUTTON_ACTION_RELEASED:
		LLUI_DEBUG_TRACE("released");
		break;
	case BUTTON_ACTION_LONG:
		LLUI_DEBUG_TRACE("long");
		break;
	case BUTTON_ACTION_REPEATED:
		LLUI_DEBUG_TRACE("repeated");
		break;
	case BUTTON_ACTION_CLICK:
		LLUI_DEBUG_TRACE("click");
		break;
	case BUTTON_ACTION_DOUBLECLICK:
		LLUI_DEBUG_TRACE("double-click");
		break;
	default:
		LLUI_DEBUG_TRACE("unknown action: %u", action);
		break;
	}

	DESCRIBE_EVENT_GENERATOR(event);
	DESCRIBE_EOL();
}

#ifdef MICROUIEVENTDECODER_EVENTGEN_TOUCH
/*
 * @brief Input pointer event holds a data that contains x and y.
 */
static void decode_event_pointer_data(uint32_t event, uint32_t data, uint32_t index) {
	(void)event;

	LLUI_DEBUG_TRACE("[%02u: 0x%08x]    at %u,%u (", index, data, POINTER_X(data), POINTER_Y(data));
	if (0 == POINTER_TYPE(data)) {
		LLUI_DEBUG_TRACE("absolute)");
	}
	else {
		LLUI_DEBUG_TRACE("relative)");
	}

	DESCRIBE_EOL();
}
#endif // MICROUIEVENTDECODER_EVENTGEN_TOUCH

static void decode_event_pointer(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	(void)index;
	(void)fct_data_decoder;

	LLUI_DEBUG_TRACE("Pointer ");
	uint8_t action = BUTTON_ACTION(event);

	switch(action) {
	case BUTTON_ACTION_PRESSED:
		LLUI_DEBUG_TRACE("pressed");
		break;
	case BUTTON_ACTION_RELEASED:
		LLUI_DEBUG_TRACE("released");
		break;
	case BUTTON_ACTION_LONG:
		LLUI_DEBUG_TRACE("long");
		break;
	case BUTTON_ACTION_REPEATED:
		LLUI_DEBUG_TRACE("repeated");
		break;
	case BUTTON_ACTION_CLICK:
		LLUI_DEBUG_TRACE("click");
		break;
	case BUTTON_ACTION_DOUBLECLICK:
		LLUI_DEBUG_TRACE("double-click");
		break;
	case POINTER_ACTION_MOVE:
		LLUI_DEBUG_TRACE("moved");
		break;
	case POINTER_ACTION_DRAG:
		LLUI_DEBUG_TRACE("dragged");
		break;
	default:
		LLUI_DEBUG_TRACE("unknown action: %u", action);
		break;
	}

	DESCRIBE_EVENT_GENERATOR(event);
	DESCRIBE_EOL();
}

static void decode_event_state(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	(void)index;
	(void)fct_data_decoder;

	LLUI_DEBUG_TRACE("TODO %s 0x%08x\n", __FUNCTION__, event);
}

/*
 * @brief Decodes the input events: the events sent by the BSP. To decode the events,
 * the decoder must know the event generators identifier defined during the MicroEJ
 * platform build. These identifiers are available in microui_constants.h.
 *
 * @see microui_event_decoder_conf.h
 */
static void decode_event_input(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	LLUI_DEBUG_TRACE("Input event: ");

	uint8_t generator_id = EVENT_GENERATOR_ID(event);

	switch(generator_id) {

#ifdef MICROUIEVENTDECODER_EVENTGEN_COMMAND
	case MICROUI_EVENTGEN_COMMANDS:
		decode_event_command(event, index, fct_data_decoder);
		break;
#endif // MICROUIEVENTDECODER_EVENTGEN_COMMAND

#ifdef MICROUIEVENTDECODER_EVENTGEN_BUTTONS
	case MICROUIEVENTDECODER_EVENTGEN_BUTTONS:
		decode_event_button(event, index, fct_data_decoder);
		break;
#endif // MICROUIEVENTDECODER_EVENTGEN_BUTTONS

#ifdef MICROUIEVENTDECODER_EVENTGEN_TOUCH
	case MICROUIEVENTDECODER_EVENTGEN_TOUCH:
		*fct_data_decoder = decode_event_pointer_data;
		decode_event_pointer(event, index, fct_data_decoder);
		break;
#endif // MICROUIEVENTDECODER_EVENTGEN_TOUCH

	default:
		LLUI_DEBUG_TRACE("unknown ");
		DESCRIBE_EVENT_GENERATOR(event);
		DESCRIBE_EOL();
		break;
	}
}

static void decode_event_user_data(uint32_t event, uint32_t data, uint32_t index) {
	(void)event;
	LLUI_DEBUG_TRACE("    [%02u] 0x%08x\n", index, data);
}

static void decode_event_user(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	LLUI_DEBUG_TRACE("User input event");

	uint8_t size = (uint8_t)USEREVENT_SIZE(event);
	if (size > (uint32_t)1) {
		LLUI_DEBUG_TRACE("s (size = %u)", size);
	}
	DESCRIBE_EVENT_GENERATOR(event);
	LLUI_DEBUG_TRACE(": ");

	*fct_data_decoder = decode_event_user_data;
}

// -----------------------------------------------------------------------------
// API
// -----------------------------------------------------------------------------


void MICROUI_EVENT_DECODER_describe_dump_start(void) {
	LLUI_DEBUG_TRACE("============================== MicroUI FIFO Dump ===============================\n");
}

void MICROUI_EVENT_DECODER_describe_dump_past(void) {
	LLUI_DEBUG_TRACE("---------------------------------- Old Events ----------------------------------\n");
}

void MICROUI_EVENT_DECODER_describe_dump_future(void) {
	LLUI_DEBUG_TRACE("---------------------------------- New Events ----------------------------------\n");
}

void MICROUI_EVENT_DECODER_describe_dump_events_objects(void) {
	LLUI_DEBUG_TRACE("--------------------------- New Events' Java objects ---------------------------\n");
}

void MICROUI_EVENT_DECODER_describe_dump_end(void) {
	LLUI_DEBUG_TRACE("================================================================================\n");
}

void MICROUI_EVENT_DECODER_drop_data(uint32_t data, uint32_t index) {
	LLUI_DEBUG_TRACE("[%02u: 0x%08x] garbage\n", index, data);
}

void MICROUI_EVENT_DECODER_decode_event(uint32_t event, uint32_t index, MICROUI_EVENT_DECODER_decode_event_data* fct_data_decoder) {
	LLUI_DEBUG_TRACE("[%02u: 0x%08x] ", index, event);

	uint8_t event_type = (uint8_t)(event >> 24);

	switch(event_type) {
	case 0x00:
		decode_event_command(event, index, fct_data_decoder);
		break;
	case 0x01:
		decode_event_button(event, index, fct_data_decoder);
		break;
	case 0x02:
		decode_event_pointer(event, index, fct_data_decoder);
		break;
	case 0x03:
		decode_event_state(event, index, fct_data_decoder);
		break;
	case 0x04:
		// not used
		break;
	case 0x05:
		LLUI_DEBUG_TRACE("Call serially (Runnable index = %u)\n", EVENT_DATA(event));
		break;
	case 0x06:
		LLUI_DEBUG_TRACE("MicroUI STOP\n");
		break;
	case 0x07:
		decode_event_input(event, index, fct_data_decoder);
		break;
	case 0x08:
		LLUI_DEBUG_TRACE("Display SHOW Displayable (Displayable index = %u)\n", EVENT_DATA(event));
		break;
	case 0x09:
		LLUI_DEBUG_TRACE("Display HIDE Displayable (Displayable index = %u)\n", EVENT_DATA(event));
		break;
	case 0x0a:
		// not used
		break;
	case 0x0b:
		LLUI_DEBUG_TRACE("Display FLUSH\n");
		break;
	case 0x0c:
		LLUI_DEBUG_TRACE("Display FORCE FLUSH\n");
		break;
	case 0x0d:
		LLUI_DEBUG_TRACE("Display REPAINT Displayable (Displayable index = %u)\n", EVENT_DATA(event));
		break;
	case 0x0e:
		LLUI_DEBUG_TRACE("Display REPAINT Current Displayable\n");
		break;
	case 0x0f:
		LLUI_DEBUG_TRACE("Display KF SWITCH Display\n");
		break;
	default:
		decode_event_user(event, index, fct_data_decoder);
		break;
	}
}

#endif // MICROUIEVENTDECODER_ENABLED

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
