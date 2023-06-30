/*
 * C
 *
 * Copyright 2020-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 *
 * Copyright 2023 NXP
 * SPDX-License-Identifier: BSD-3-Clause
 */

#if !defined __TRACE_PLATFORM_H__
#define __TRACE_PLATFORM_H__

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "board.h"

#include "LLTRACE_impl.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief name of the trace group
 */
#define TRACE_PLATFORM_GROUP_NAME		"RT595"

/*
 * @brief Number of events for the platform trace group
 */
#define TRACE_PLATFORM_NB_EVENTS		1000

/*
 * #brief Event subgroup offsets
 */
#define TRACE_PLATFORM_GROUP_VGLITE				100
#define TRACE_PLATFORM_GROUP_LLVGLITE			200
#define TRACE_PLATFORM_GROUP_VGLITE_HAL			300
#define TRACE_PLATFORM_GROUP_DISPLAY_TASK		400
#define TRACE_PLATFORM_GROUP_VECTOR_FONT		500
#define TRACE_PLATFORM_GROUP_MVG				600

/*
 *  @brief HW fake tasks ids
 *
 */
#define HW_TASK_SDMA_ID 	0xAADD
#define HW_TASK_DMA_ID  	0xAAFF
#define HW_TASK_DS_ID   	0xBBDD
#define HW_TASK_SLEEP_ID  	0xBBFF

/*
 *  @brief HW fake tasks names list
 *  Last listed will have highest priority
 */
#define HW_TASK_NAME_LIST {"SDMA"         , "DMA"         , "DEEPSLEEP"  , "SLEEP"}
#define HW_TASK_ID_LIST   {HW_TASK_SDMA_ID, HW_TASK_DMA_ID, HW_TASK_DS_ID, HW_TASK_SLEEP_ID}

#define TRACE_HW_TASK_START(id)  trace_HW_task_start(id)
#define TRACE_HW_TASK_STOP(id)   trace_HW_task_stop(id)

/*
 * #brief Translate subgroup and event_id to an event identifier
 */
#define __ID(subgroup, event_id) \
	trace_platform_get_group(), event_id + TRACE_PLATFORM_GROUP_ ## subgroup

/*
 * #brief Trace macros
 *
 * TRACE_PLATFORM_START_XXX logs a start event
 * TRACE_PLATFORM_END_XXX logs an end event
 *
 * TRACE_PLATFORM_XXX_VOID does not send any parameter
 * TRACE_PLATFORM_XXX_U32(X{Y}) sends Y parameters
 *
 * @param subgroup: The subgroup of events
 * @param event_id: The event identifier
 * @param v{Y}: 32 bits parameter
 */
#define TRACE_PLATFORM_START_VOID(subgroup, event_id) \
	LLTRACE_IMPL_record_event_void(__ID(subgroup, event_id))

#define TRACE_PLATFORM_START_U32(subgroup, event_id, v1) \
	LLTRACE_IMPL_record_event_u32(__ID(subgroup, event_id), v1)

#define TRACE_PLATFORM_START_U32X2(subgroup, event_id, v1, v2) \
	LLTRACE_IMPL_record_event_u32x2(__ID(subgroup, event_id), v1, v2)

#define TRACE_PLATFORM_START_U32X3(subgroup, event_id, v1, v2, v3) \
	LLTRACE_IMPL_record_event_u32x3(__ID(subgroup, event_id), v1, v2, v3 )

#define TRACE_PLATFORM_START_U32X4(subgroup, event_id, v1, v2, v3, v4) \
	LLTRACE_IMPL_record_event_u32x4(__ID(subgroup, event_id), v1, v2, v3, v4)

#define TRACE_PLATFORM_START_U32X5(subgroup, event_id, v1, v2, v3, v4, v5) \
	LLTRACE_IMPL_record_event_u32x5( \
									__ID(subgroup, event_id), \
									v1, v2, v3, v4, v5 \
									)

#define TRACE_PLATFORM_START_U32X6(subgroup, event_id, \
		v1, v2, v3, v4, v5, v6) \
	LLTRACE_IMPL_record_event_u32x6( \
									__ID(subgroup, event_id), \
									v1, v2, v3, v4, v5, v6 \
									)

#define TRACE_PLATFORM_START_U32X7(subgroup, event_id, \
		v1, v2, v3, v4, v5, v6, v7) \
	LLTRACE_IMPL_record_event_u32x7( \
									__ID(subgroup, event_id), \
									v1, v2, v3, v4, v5, v6, v7 \
									)

#define TRACE_PLATFORM_START_U32X8(subgroup, event_id, \
		v1, v2, v3, v4, v5, v6, v7, v8) \
	LLTRACE_IMPL_record_event_u32x8( \
									__ID(subgroup, event_id), \
									v1, v2, v3, v4, v5, v6, v7, v8 \
									)

#define TRACE_PLATFORM_START_U32X9(subgroup, event_id, \
		v1, v2, v3, v4, v5, v6, v7, v8, v9) \
	LLTRACE_IMPL_record_event_u32x9( \
									__ID(subgroup, event_id), \
									event_id, \
									v1, v2, v3, v4, v5, v6, v7, v8, v9 \
									)

#define TRACE_PLATFORM_START_U32X10(subgroup, event_id, \
		v1, v2, v3, v4, v5, v6, v7, v8, v9, v10) \
	LLTRACE_IMPL_record_event_u32x10( \
									__ID(subgroup, event_id), \
									event_id, \
									v1, v2, v3, v4, v5, v6, v7, v8, v9, v10 \
									)

#define TRACE_PLATFORM_END_VOID(subgroup, event_id) \
	LLTRACE_IMPL_record_event_end(__ID(subgroup, event_id))

#define TRACE_PLATFORM_END_U32(subgroup, event_id, v) \
	LLTRACE_IMPL_record_event_end_u32(__ID(subgroup, event_id), v)

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * @brief Initializes the platform trace module
 */
void trace_platform_initialize(void);

/*
 * @brief Gets the platform group identifier
 *
 * @return the platform group identifier
 */
int trace_platform_get_group(void);

/*
 * @brief Gets the maximum number of events
 *
 * @return the platform group identifier
 */
int trace_platform_get_nb_events(void);


/*
 * @brief Sends list of fake hardware tasks
 */
void trace_HW_tasks_send_task_list(void);

/*
 * @brief Sends "start" event of hardware task
 */
void trace_HW_task_start(uint32_t HW_task_id);

/*
 * @brief Sends "stop" event of hardware task
 */
void trace_HW_task_stop(uint32_t HW_task_id);

#endif // !defined __TRACE_PLATFORM_H__

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------
