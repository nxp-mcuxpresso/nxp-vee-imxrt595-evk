/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef __MONITOR_H__
#define __MONITOR_H__

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "cpuload.h"
#include "framerate.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

#define MONITOR_ENABLED 0

#define MONITOR_SCHEDULE_TIME       1000    // ms

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

#if MONITOR_ENABLED == 1

/*
 * @brief Initializes the monitor function
 */
int32_t monitor_init(void);

#endif // MONITOR_ENABLED == 1

#endif	// __MONITOR_H__

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

