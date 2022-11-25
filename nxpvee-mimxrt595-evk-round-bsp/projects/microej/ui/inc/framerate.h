/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#if !defined FRAMERATE_H
#define FRAMERATE_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "framerate_conf.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define FRAMERATE_OK 0
#define FRAMERATE_ERROR -1

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * @brief Initialize the framerate module
 *
 * @param[in] schedule_time_ms: Scheduling time in milliseconds
 * @return: FRAMERATE_OK if initialization succeeded, FRAMERATE_ERROR otherwise
 */
int32_t framerate_init(int32_t schedule_time_ms);

/*
 * @brief Increment the framerate counter
 */
void framerate_increment(void);

/*
 * @brief Return the last framerate
 */
uint32_t framerate_get(void);

// -----------------------------------------------------------------------------
// Java APIs
// -----------------------------------------------------------------------------

#ifndef javaFramerateInit
#define javaFramerateInit		Java_com_microej_util_Util_initFrameRate
#endif
#ifndef javaFramerateGet
#define javaFramerateGet		Java_com_microej_util_Util_getFrameRate
#endif

#endif // !defined FRAMERATE_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

