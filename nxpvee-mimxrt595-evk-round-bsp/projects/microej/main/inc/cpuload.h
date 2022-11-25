/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#ifndef _CPULOAD_INTERN
#define _CPULOAD_INTERN

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "cpuload_conf.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Scheduling (sampling) period of the CPU Load
 */
#ifndef CPULOAD_SCHEDULE_TIME_MS
#define CPULOAD_SCHEDULE_TIME_MS 250	// ms
#endif

#define CPULOAD_INIT_SCHEDULE_FRACTION 10

#define CPULOAD_OK 					 0
#define CPULOAD_NOT_ENABLED			-1
#define CPULOAD_INVALID_COUNTER 	-2
#define CPULOAD_START_TASK_ERROR 	-3

// -----------------------------------------------------------------------------
// Project Functions
// -----------------------------------------------------------------------------

/*
 * @brief Initialize the framework
 * Must be called in very first OS stack. No more task must run
 * at same time.
 *
 * @return:
 *   - CPULOAD_OK: success.
 *   - CPULOAD_START_TASK_ERROR: error during CPU load worker task initialization.
 *   - CPULOAD_INVALID_COUNTER: idle counter not incremented,
 *         the CPU load idle hook has not been called during the initialization.
 *   - CPULOAD_NOT_ENABLED: the CPU load function is disabled.
 */
int32_t cpuload_init(void);

/*
 * Must be called by the OS idle function
 */
void cpuload_idle(void);

/*
 * to be called before and after sleep to compute idle time
 */
void cpuload_enter_sleep(void);
void cpuload_exit_sleep(void);

/*
 * Return the last CPU load measure
 *
 * @return: last CPU load over CPULOAD_SCHEDULE_TIME_MS period
 */
uint32_t cpuload_get(void);

// -----------------------------------------------------------------------------
// Java APIs
// -----------------------------------------------------------------------------

#ifndef javaCPULoadInit
#define javaCPULoadInit		Java_com_microej_util_Util_initCpuLoad
#endif

#ifndef javaCPULoadGet
//#define javaCPULoadGet		Java_com_is2t_debug_CPULoad_get
#define javaCPULoadGet		Java_com_microej_util_Util_getCpuLoad

#endif

#endif	// _CPULOAD_INTERN

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

