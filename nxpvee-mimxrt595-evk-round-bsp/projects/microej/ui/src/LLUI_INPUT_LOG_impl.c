/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file
 * @brief This MicroUI FIFO (queue) logger replaces the default logger embedded in the
 * MicroUI Input Engine. For each queue event, it stores the event's data size. This
 * allows to be able to decode the event when LLUI_INPUT_dump() is called.
 *
 * This logger does not interpret the event: it just recognizes the event's first
 * element and event's data. When an event is detected, the logger calls
 * microui_event_decoder.h functions.
 *
 * @see LLUI_INPUT_impl.h file comment
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.1.0
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <assert.h>
#include <string.h>

// implements some LLUI_INPUT_impl functions
#include "LLUI_INPUT_impl.h"

// deport event description to another file
#include "microui_event_decoder.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef MICROUIEVENTDECODER_ENABLED

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Logger's array size. The MicroUI FIFO cannot be higher than this size.
 */
#define QUEUE_LOG_MAX_SIZE 100

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/*
 * @brief Pointer on event's data decoder.
 */
static MICROUI_EVENT_DECODER_decode_event_data fct_decode_data;

/*
 * @brief Logger array where store the MicroUI event metadata.
 */
static uint8_t queue_log[QUEUE_LOG_MAX_SIZE];

/*
 * @brief true when a new event is logged, false when the event's data is logged.
 */
static bool queue_is_first_element;

/*
 * @brief Current event to decode.
 */
static uint32_t dump_current_event;

// -----------------------------------------------------------------------------
// LLUI_INPUT_impl.h functions
// -----------------------------------------------------------------------------

void LLUI_INPUT_IMPL_log_queue_init(uint32_t length) {
	assert(length <= (uint32_t)QUEUE_LOG_MAX_SIZE);
	(void)memset((void*)queue_log, 0, length);
	queue_is_first_element = true;
}

void LLUI_INPUT_IMPL_log_queue_full(uint32_t data) {
	// queue is full, nothing to log
	(void)data;
}

void LLUI_INPUT_IMPL_log_queue_add(uint32_t data, uint32_t index, uint32_t remaining_elements, uint32_t queue_length) {
	(void)data;
	(void)queue_length;

	if (queue_is_first_element) {
		// start new event: set the event size in array
		queue_log[index] = (uint8_t)(remaining_elements + (uint32_t)1);
	}
	else {
		// continue previous event: drop data
		queue_log[index] = 0;
	}

	// prepare next log
	queue_is_first_element = remaining_elements == (uint32_t)0;
}

void LLUI_INPUT_IMPL_log_queue_replace(uint32_t old, uint32_t data, uint32_t index, uint32_t queue_length) {
	// previous event has been replaced, nothing to log
	(void)old;
	(void)data;
	(void)index;
	(void)queue_length;
}

void LLUI_INPUT_IMPL_log_queue_read(uint32_t data, uint32_t index) {
	// event has been read, nothing to log
	(void)data;
	(void)index;
}

void LLUI_INPUT_IMPL_log_dump(bool log_type, uint32_t log, uint32_t index) {
	if (log_type) {
		// log is an event or a data event

		if ((uint32_t)0 != queue_log[index]) {
			// it is a new event to decode
			dump_current_event = log;

			// reset data decoder
			fct_decode_data = NULL;

			// decode event (decoder can give a data decoder)
			MICROUI_EVENT_DECODER_decode_event(log, index, &fct_decode_data);
		}
		else if (NULL != fct_decode_data) {
			// decode data of previous event
			fct_decode_data(dump_current_event, log, index);
		}
		else {
			// cannot decode this element: drop it
			MICROUI_EVENT_DECODER_drop_data(log, index);
		}
	}
	else {
		// log is a status: see function API comment.

		switch(log) {

		case 0:
			// start of dump (events already consumed)
			MICROUI_EVENT_DECODER_describe_dump_start();
			MICROUI_EVENT_DECODER_describe_dump_past();

			// drop first element if it is not an event
			fct_decode_data = NULL;
			break;

		case 1:
			// continue dump (events not consumed yet)
			MICROUI_EVENT_DECODER_describe_dump_future();
			break;

		case 2:
			// dump java objects
			MICROUI_EVENT_DECODER_describe_dump_events_objects();
			break;

		case 3:
		default:
			// end of dump
			MICROUI_EVENT_DECODER_describe_dump_end();
			break;
		}
	}
}

#endif // MICROUIEVENTDECODER_ENABLED

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif

