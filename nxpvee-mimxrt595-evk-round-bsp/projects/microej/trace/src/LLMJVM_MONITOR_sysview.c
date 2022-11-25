/*
 * C
 *
 * Copyright 2017-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
* @file
* @brief MicroEJ Trace library low level API
* @author MicroEJ Developer Team
* @version 2.1.2
* @date 29 September 2021
*/

/* Includes ------------------------------------------------------------------*/

#include <LLMJVM_MONITOR_impl.h>
#include <MJVM_MONITOR.h>
#include <trace.h>
#include <SEGGER_SYSVIEW.h>
#include <stdio.h>
#include <string.h>
#include "LLMJVM_MONITOR_sysview.h"
#include "LLTRACE_sysview_configuration.h"


/* Defines -------------------------------------------------------------------*/

// UID for the GC thread (0 is not used by any thread)
#define THREAD_GC_UID (0xFF)

#define THREAD_GC_NAME "GC"

// Max Thread priority + 1
#define THREAD_GC_PRIORITY (11)


#define apiID_ALLOCATE				            (0u)
#define apiID_EXCEPTION                         (1u)



/* Private functions ---------------------------------------------------------*/

/**
 * Send thread info to SYSVIEW.
 */
static void LLMJVM_MONITOR_SYSTEMVIEW_send_thread_info(int32_t thread_id) {
	MJVM_MONITOR_thread_info_t thread_info;
	SEGGER_SYSVIEW_TASKINFO sysview_thread_info;
	char thread_name[MICROEJ_TRACE_MAX_THREAD_NAME_LENGTH];
	int32_t result;

	result = MJVM_MONITOR_get_thread_info(thread_id, &thread_info, thread_name, sizeof(thread_name));
	if(result == MJVM_MONITOR_OK){
		memset(&sysview_thread_info, 0, sizeof(sysview_thread_info));
		sysview_thread_info.TaskID = (U32)thread_id;
		sysview_thread_info.sName = thread_name;
		sysview_thread_info.Prio = thread_info.priority;
		sysview_thread_info.StackBase = (U32)0;
		if(thread_info.max_stack_usage == -1){
			thread_info.max_stack_usage = 0;
		}
		sysview_thread_info.StackSize = thread_info.max_stack_usage;
		sysview_thread_info.TaskType = SEGGER_SYSVIEW_TASK_TYPE_MEJ;
		SEGGER_SYSVIEW_SendTaskInfo(&sysview_thread_info);
	}
}

static void LLMJVM_MONITOR_SYSTEMVIEW_send_gc_thread_info() {
	// Declare the GC as a task
	SEGGER_SYSVIEW_TASKINFO taskInfo;
	memset(&taskInfo, 0, sizeof(taskInfo));
	taskInfo.TaskID = (U32)THREAD_GC_UID;
	taskInfo.sName = THREAD_GC_NAME;
	taskInfo.Prio = THREAD_GC_PRIORITY;
	taskInfo.StackBase = (U32)0;
	taskInfo.StackSize = 0;
	taskInfo.TaskType = SEGGER_SYSVIEW_TASK_TYPE_MEJ;
	SEGGER_SYSVIEW_SendTaskInfo(&taskInfo);
}

void LLMJVM_MONITOR_SYSTEMVIEW_send_task_list(void) {
	int32_t java_threads_ids[MICROEJ_TRACE_MAX_NB_THREADS];
	int32_t nb_java_threads = MJVM_MONITOR_get_threads_ids(java_threads_ids, MICROEJ_TRACE_MAX_NB_THREADS);

	// Declare all the threads
	while(--nb_java_threads >= 0){
		LLMJVM_MONITOR_SYSTEMVIEW_send_thread_info(java_threads_ids[nb_java_threads]);
	}

	LLMJVM_MONITOR_SYSTEMVIEW_send_gc_thread_info();
}

/* Globals -------------------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/

void LLMJVM_MONITOR_IMPL_initialize(bool auto_start) {
	if(auto_start == true){
		TRACE_start();
	}
	SEGGER_SYSVIEW_OnTaskCreate(THREAD_GC_UID);
	LLMJVM_MONITOR_SYSTEMVIEW_send_gc_thread_info();
}

void LLMJVM_MONITOR_IMPL_on_shutdown(void) {
	// Nothing to do
}

void LLMJVM_MONITOR_IMPL_on_thread_create(int32_t thread_id) {
	SEGGER_SYSVIEW_OnTaskCreate(thread_id);
	LLMJVM_MONITOR_SYSTEMVIEW_send_thread_info(thread_id);
}

void LLMJVM_MONITOR_IMPL_on_thread_modified(int32_t thread_id) {
	LLMJVM_MONITOR_SYSTEMVIEW_send_thread_info(thread_id);
}

void LLMJVM_MONITOR_IMPL_on_thread_state_changed(int32_t thread_id, MJVM_MONITOR_state_t new_state) {
	switch(new_state){
		case MJVM_MONITOR_STATE_READY:{
			SEGGER_SYSVIEW_OnTaskStartReady(thread_id);
			break;
		}
		case MJVM_MONITOR_STATE_RUNNING:{
                        SEGGER_SYSVIEW_setCurrentMicroEJTask(thread_id);
			SEGGER_SYSVIEW_OnTaskStartExec(thread_id);
			break;
		}
		case MJVM_MONITOR_STATE_WAITING:
		case MJVM_MONITOR_STATE_MONITOR_QUEUED:{
			SEGGER_SYSVIEW_OnTaskStopReady(thread_id, new_state);
			break;
		}
		case MJVM_MONITOR_STATE_TERMINATED:{
			LLMJVM_MONITOR_SYSTEMVIEW_send_thread_info(thread_id);
			SEGGER_SYSVIEW_OnTaskStopExec();
			break;
		}
	}
}

void LLMJVM_MONITOR_IMPL_on_idle(void) {
  // MicroEJ IDLE event is only sent when used without any RTOS.
  // Otherwise it is the only responsibility of the RTOS to send this event.
  if(!MICROEJ_TRACE_OVER_RTOS){
    SEGGER_SYSVIEW_OnIdle();
  }  
}

void LLMJVM_MONITOR_IMPL_on_gc_start(int32_t current_thread_id) {
	SEGGER_SYSVIEW_OnTaskStartExec(THREAD_GC_UID);
}

void LLMJVM_MONITOR_IMPL_on_gc_stop(int32_t current_thread_id) {
	SEGGER_SYSVIEW_OnTaskStartExec(current_thread_id);
}

void LLMJVM_MONITOR_IMPL_on_allocate(void* type, int32_t size, void* method, void* instruction_address, int32_t total_memory, int32_t free_memory, bool immortal) {
	if(MICROEJ_TRACE_ENABLE_ALLOCATIONS){
		SEGGER_SYSVIEW_RecordU32x3(MICROEJ_TRACE_API_ID_OFFSET + apiID_ALLOCATE, size, free_memory, total_memory-free_memory);
	}
}

void LLMJVM_MONITOR_IMPL_on_exception(void* exception_type, void* throw_method, void* throw_instruction_address, void* catch_method, void* catch_instruction_address, const char* message) {
	if(MICROEJ_TRACE_ENABLE_EXCEPTIONS){
		SEGGER_SYSVIEW_RecordU32x5(MICROEJ_TRACE_API_ID_OFFSET + apiID_EXCEPTION, (U32)exception_type, (U32)throw_method, (U32)throw_instruction_address, (U32)catch_method, (U32)catch_instruction_address);
	}
}
