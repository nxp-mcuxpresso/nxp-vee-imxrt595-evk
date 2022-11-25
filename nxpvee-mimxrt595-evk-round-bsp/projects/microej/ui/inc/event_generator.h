/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * This header files groups several APIs to notify to a or several events handlers an
 * event has occured on an input driver (buttons, touch etc.).
 *
 * The idea is to disconnect the management of the inputs drivers (how to initialize an
 * input driver, how to get/read an input event, how to clear it, etc.) and how to
 * send these inputs events to the MicroUI application using the MicroUI EventGenerators.
 *
 * The MicroUI framework is using an internal buffer to store the inputs events. The event
 * will not be added if the internal events buffer is full. In this case the input driver
 * has to adapt itself in order to not send a future invalid event. For instance, if a
 * PRESSED event is not sent, the input driver has not to send a REPEAT or RELEASE event.
 * So it may have a distinction between the real input state and the "software" input
 * state.
 */
#ifndef _EVENT_GENERATOR
#define _EVENT_GENERATOR

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>

#define EVENT_GENERATOR_SPECIAL_FUNCTION

/* TYPES ---------------------------------------------------------------------*/

typedef void (* EVENT_GENERATOR_special_cb_t) (void * params);

/* API -----------------------------------------------------------------------*/

/*
 * @brief Notifies to the event generator a button has been pressed.
 *
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_pressed(int32_t buttonId);

/*
 * @brief Notifies to the event generator a button has been repeated.
 *
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_repeated(int32_t buttonId);

/*
 * @brief Notifies to the event generator a button has been released.
 *
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_button_released(int32_t buttonId);

/*
 * @brief Notifies to the event generator a pointer button has been pressed.
 *
 * @param buttonId the button ID, between 0 and 255
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @param type {@link LLUI_INPUT_ABSOLUTE} when (x,y) coordinates are absolute, {@link LLUI_INPUT_RELATIVE} when
 * there are relative to the previous pointer position
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_pointer_pressed(int32_t buttonID, int32_t x, int32_t y, int32_t type);

/*
 * @brief Notifies to the event generator a pointer has moved.
 *
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @param absolute {@link LLUI_INPUT_ABSOLUTE} when (x,y) coordinates are absolute, {@link LLUI_INPUT_RELATIVE} when
 * there are relative to the previous pointer position
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_pointer_moved(int32_t x, int32_t y, int32_t type);

/*
 * @brief Notifies to the event generator a pointer button has been released.
 *
 * @param buttonId the button ID, between 0 and 255
 * @return {@link LLUI_INPUT_OK} if the event has been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_pointer_released(int32_t buttonID);

/*
 * @brief Notifies to the event generator a touch has been pressed.
 *
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_pressed(int32_t x, int32_t y);

/*
 * @brief Notifies to the event generator a touch has moved.
 *
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_moved(int32_t x, int32_t y);

/*
 * @brief Notifies the event generator a touch has been released.
 *
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_touch_released(void);

/*
 * @brief Notifies the event generator that a state has changed.
 *
 * @param stateID the state machine identifier, between 0 and 255
 * @param stateValue the new state if the state machine, between 0 and 255
 * @return {@link LLUI_INPUT_OK} if all events have been added, {@link LLUI_INPUT_NOK} otherwise
 */
int32_t EVENT_GENERATOR_state_changed(int32_t stateID, int32_t stateValue);

/*
 * @brief Registers a callback to execute platform specific functions
 *
 * @param[in] callback: Pointer to the callback function
 * @param[in] event: event on which to register
 */
int EVENT_GENERATOR_register_special_cb(
		EVENT_GENERATOR_special_cb_t callback,
		unsigned int event);

#endif

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

