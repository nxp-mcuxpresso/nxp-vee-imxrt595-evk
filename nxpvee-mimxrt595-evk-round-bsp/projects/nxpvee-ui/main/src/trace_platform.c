/*
 * C
 *
 * Copyright 2020-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "LLTRACE_impl.h"
#include "trace_platform.h"
#include "SEGGER_SYSVIEW.h"

// -----------------------------------------------------------------------------
// Defines & Macros
// -----------------------------------------------------------------------------

/*
 *  @brief base priority for SYSTEMVIEW
 */
#define HW_TASK_BASE_PRIORITY 18

// -----------------------------------------------------------------------------
// External Variables
// -----------------------------------------------------------------------------

/*
 * @brief Current Systemview running task Id.
 */
extern U32 SEGGER_SYSVIEW_currentTaskId;


// -----------------------------------------------------------------------------
// Private Variables
// -----------------------------------------------------------------------------

/*
 * @brief Platform trace group identifier
 */
static int trace_platform_group;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * @brief Initialize the platform trace module
 */
void trace_platform_initialize(void) {
	trace_platform_group = LLTRACE_IMPL_declare_event_group(
								TRACE_PLATFORM_GROUP_NAME,
								TRACE_PLATFORM_NB_EVENTS);
}

/*
 * @brief Gets the platform trace group identified
 *
 * @return:
 * - If trace_platform_initialize not called: undefined
 * - Otherwise: the platform group identifier
 */
int trace_platform_get_group(void) {
	return trace_platform_group;
}


/*
 * @brief Sends list of fake SYSTEMVIEW hardware tasks (not called when SystemView is not used)
 */
void trace_HW_tasks_send_task_list(void)
{
	// Moke a task for Systemview
	SEGGER_SYSVIEW_TASKINFO sysview_thread_info;

	char * hw_task_name_list[] = HW_TASK_NAME_LIST;
	int hw_task_id_list[] = HW_TASK_ID_LIST;

	memset(&sysview_thread_info, 0, sizeof(sysview_thread_info));
	for(int i=0; i< sizeof(hw_task_name_list)/sizeof(char*); i++)
	{
		sysview_thread_info.TaskID = hw_task_id_list[i];
		sysview_thread_info.sName = hw_task_name_list[i];
		sysview_thread_info.Prio = HW_TASK_BASE_PRIORITY + i;
		sysview_thread_info.StackBase = (U32)0;
		sysview_thread_info.StackSize = 0;
		sysview_thread_info.TaskType = SEGGER_SYSVIEW_TASK_TYPE_HW;
		SEGGER_SYSVIEW_SendTaskInfo(&sysview_thread_info);
		SEGGER_SYSVIEW_OnTaskCreate(hw_task_id_list[i]);
	}
}

/*
 * @brief Sends "start" event of hardware task to SYSTEMVIEW
 */
void trace_HW_task_start(uint32_t HW_task_id)
{
#if (ENABLE_SVIEW == 1)
	SEGGER_SYSVIEW_OnTaskStartReady(HW_task_id);
#endif
}

/*
 * @brief Sends "stop" event of hardware task to SYSTEMVIEW
 */
void trace_HW_task_stop(uint32_t HW_task_id)
{
#if (ENABLE_SVIEW == 1)
	U32 currentTaskId = SEGGER_SYSVIEW_currentTaskId;
	SEGGER_SYSVIEW_OnTaskStartExec(HW_task_id);
	SEGGER_SYSVIEW_OnTaskStopReady(HW_task_id, 0);
	if(currentTaskId) {
		SEGGER_SYSVIEW_OnTaskStartExec(currentTaskId);
	}
	else {
		SEGGER_SYSVIEW_OnIdle();
	}
#endif
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

