/*
 * C
 *
 * Copyright 2019-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#ifndef LLTRACE_SYSVIEW_CONFIG_H
#define LLTRACE_SYSVIEW_CONFIG_H

#ifdef __cplusplus
	extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/* Defines -------------------------------------------------------------------*/

/*
 * Define the maximum size in characters of MicroEJ threads (extra characters will not be displayed).
 */
#define MICROEJ_TRACE_MAX_THREAD_NAME_LENGTH (32)

/*
 * Define the maximum number of MicroEJ threads infos structures.
 * If there are more threads, only the address will be displayed in SystemView.
 */
#define MICROEJ_TRACE_MAX_NB_THREADS (16)

/*
 * Specify if MicroEJ trace is used over RTOS traces or not.
 */
#define MICROEJ_TRACE_OVER_RTOS         	  1

/*
 * Enable/Disable MicroEJ Object Allocations events.
 */
#define MICROEJ_TRACE_ENABLE_ALLOCATIONS      0

/*
 * Enable/Disable MicroEJ Exception events.
 */
#define MICROEJ_TRACE_ENABLE_EXCEPTIONS       1

/*
 * Define the offset for MicroEJ trace events. This offset must not overlap with RTOS specific events if any.
 */
#define MICROEJ_TRACE_API_ID_OFFSET     (500u)

/*
 * Define the maximum number of SYSVIEW modules for MicroEJ events.
 */
#define MICROEJ_TRACE_MAX_NB_MODULES (8)

/*
 * Define the maximum length of a module description.
 */
#define MICROEJ_TRACE_MAX_MODULE_DESCRIPTION_LENGTH (16)

/*
 * Define the prefix to add to the module description
 */
#define MICROEJ_TRACE_MODULE_DESCRIPTION_PREFIX "M="

/*
 * Enable/Disable MicroEJ trace wait until SystemView host application is connected.
 */
#define MICROEJ_TRACE_WAIT_FOR_SYSVIEW_CONNECTION	0
          
#ifdef __cplusplus
	}
#endif
#endif //LLMJVM_MONITOR_SYSVIEW_CONFIG_H
