/*
 * C
 *
 * Copyright 2021-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/*
 * @file
 * @brief This file allows to configure the implementation of microui_event_decoder.c.
 * @author MicroEJ Developer Team
 * @version 2.0.0
 * @date 22 July 2022
 * @since MicroEJ UI Pack 13.1.0
 */

#if !defined MICROUI_EVENT_DECODER_CONF_H
# define MICROUI_EVENT_DECODER_CONF_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdio.h>

// header file created by MicroEJ Platform builder.
#include "microui_constants.h"

#ifdef __cplusplus
extern "C" {
#endif

// -----------------------------------------------------------------------------
// Defines
// -----------------------------------------------------------------------------

/*
 * @brief When defined, the logger is enabled. The call to LLUI_INPUT_dump()
 * has no effect when the logger is disabled.
 *
 * By default the logger is not enabled.
 */
#define MICROUIEVENTDECODER_ENABLED

#ifdef MICROUIEVENTDECODER_ENABLED

/*
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Command" events. The define's value is the MicroUI Event Generator
 * "Command" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Command" is "MICROUI_EVENTGEN_COMMANDS":
 *
 *   #define MICROUIEVENTDECODER_EVENTGEN_COMMAND MICROUI_EVENTGEN_COMMANDS
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Command".
 */
#define MICROUIEVENTDECODER_EVENTGEN_COMMAND MICROUI_EVENTGEN_COMMANDS

/*
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Buttons" events. The define's value is the MicroUI Event Generator
 * "Buttons" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Buttons" is "MICROUI_EVENTGEN_BUTTONS":
 *
 *   #define MICROUIEVENTDECODER_EVENTGEN_BUTTONS MICROUI_EVENTGEN_BUTTONS
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Buttons".
 */
#define MICROUIEVENTDECODER_EVENTGEN_BUTTONS MICROUI_EVENTGEN_BUTTONS

/*
 * @brief When defined, the MicroUI event decoder is able to decode the *input*
 * "Touch" events. The define's value is the MicroUI Event Generator
 * "Touch" fixed in the microui.xml file and used to build the MicroEJ Platform.
 * Most of time the MicroUI Event Generator "Touch" is "MICROUI_EVENTGEN_TOUCH":
 *
 *   #define MICROUIEVENTDECODER_EVENTGEN_TOUCH MICROUI_EVENTGEN_TOUCH
 *
 * When not defined, the MicroUI event decoder does not try to decode the MicroUI
 * events "Touch".
 */
#define MICROUIEVENTDECODER_EVENTGEN_TOUCH MICROUI_EVENTGEN_TOUCH

/*
 * Standard "printf" indirection.
 */
#define LLUI_DEBUG_TRACE (void)printf

#endif // MICROUIEVENTDECODER_ENABLED

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

#ifdef __cplusplus
}
#endif
#endif // MICROUI_EVENT_DECODER_CONF_H
