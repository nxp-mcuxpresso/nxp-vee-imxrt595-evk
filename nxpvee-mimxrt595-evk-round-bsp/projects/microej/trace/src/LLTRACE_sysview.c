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

#include <LLTRACE_impl.h>
#include <SEGGER_SYSVIEW.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include "LLTRACE_sysview_configuration.h"


/* Defines -------------------------------------------------------------------*/

SEGGER_SYSVIEW_MODULE LLTRACE_modules[MICROEJ_TRACE_MAX_NB_MODULES];
char* LLTRACE_module_names[MICROEJ_TRACE_MAX_NB_MODULES][MICROEJ_TRACE_MAX_MODULE_DESCRIPTION_LENGTH];

/**
 * Offset of the next free group in LLTRACE_modules
 */
int32_t LLTRACE_next_free_module_offset = 0;
bool LLTRACE_started = false;

/* Private functions ---------------------------------------------------------*/

/* Public functions ----------------------------------------------------------*/


void LLTRACE_IMPL_start() {
	if(MICROEJ_TRACE_WAIT_FOR_SYSVIEW_CONNECTION){
		if(SEGGER_SYSVIEW_isConnected() == 0){
			printf("Wait for SystemView Application Connection...\n");
			while(SEGGER_SYSVIEW_isConnected() == 0){
				// wait
			}
			printf("SystemView Application Connected.\n");
		}
	}
	LLTRACE_started = true;
}

void LLTRACE_IMPL_stop() {
if(!MICROEJ_TRACE_OVER_RTOS){
	SEGGER_SYSVIEW_Stop();
}
	LLTRACE_started = false;
}

bool LLTRACE_IMPL_is_started() {
	return LLTRACE_started;
}

int32_t LLTRACE_IMPL_declare_event_group(const char* group_name, int32_t nb_events) {
	int32_t module_offset = LLTRACE_next_free_module_offset;
	if(module_offset < MICROEJ_TRACE_MAX_NB_MODULES){
		SEGGER_SYSVIEW_MODULE* module = &LLTRACE_modules[module_offset];
		char* module_description = (char*)&LLTRACE_module_names[module_offset];

		// Increment pointer for the next group declaration
		++LLTRACE_next_free_module_offset;

		// Initialize the module description: "M="+group_name
		strcpy(module_description, MICROEJ_TRACE_MODULE_DESCRIPTION_PREFIX);
		strncpy(&module_description[sizeof(MICROEJ_TRACE_MODULE_DESCRIPTION_PREFIX)-1], group_name, MICROEJ_TRACE_MAX_MODULE_DESCRIPTION_LENGTH-sizeof(MICROEJ_TRACE_MODULE_DESCRIPTION_PREFIX-1));
		module_description[MICROEJ_TRACE_MAX_MODULE_DESCRIPTION_LENGTH-1] = '\0';
		module->sModule = module_description;

		module->NumEvents = nb_events;
		module->EventOffset = 0; //set by SystemView
		module->pfSendModuleDesc = NULL;
		module->pNext = NULL; // set by SystemView

		SEGGER_SYSVIEW_RegisterModule(module);
		return (int32_t)module;
	}
	else {
		// No free module description
		return -1;
	}
}

void LLTRACE_IMPL_record_event_void(int32_t group_id, int32_t event_id) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordVoid(module->EventOffset + event_id);
}

void LLTRACE_IMPL_record_event_u32(int32_t group_id, int32_t event_id, uint32_t value0) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32(module->EventOffset + event_id, value0);
}

void LLTRACE_IMPL_record_event_u32x2(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x2(module->EventOffset + event_id, value1, value2);
}

void LLTRACE_IMPL_record_event_u32x3(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x3(module->EventOffset + event_id, value1, value2, value3);
}

void LLTRACE_IMPL_record_event_u32x4(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x4(module->EventOffset + event_id, value1, value2, value3, value4);
}

void LLTRACE_IMPL_record_event_u32x5(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x5(module->EventOffset + event_id, value1, value2, value3, value4, value5);
}

void LLTRACE_IMPL_record_event_u32x6(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5, uint32_t value6) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x6(module->EventOffset + event_id, value1, value2, value3, value4, value5, value6);
}

void LLTRACE_IMPL_record_event_u32x7(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5, uint32_t value6, uint32_t value7) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x7(module->EventOffset + event_id, value1, value2, value3, value4, value5, value6, value7);
}

void LLTRACE_IMPL_record_event_u32x8(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5, uint32_t value6, uint32_t value7, uint32_t value8) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x8(module->EventOffset + event_id, value1, value2, value3, value4, value5, value6, value7, value8);
}

void LLTRACE_IMPL_record_event_u32x9(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5, uint32_t value6, uint32_t value7, uint32_t value8, uint32_t value9) {
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x9(module->EventOffset + event_id, value1, value2, value3, value4, value5, value6, value7, value8, value9);
}

void LLTRACE_IMPL_record_event_u32x10(int32_t group_id, int32_t event_id, uint32_t value1, uint32_t value2, uint32_t value3, uint32_t value4, uint32_t value5, uint32_t value6, uint32_t value7, uint32_t value8, uint32_t value9, uint32_t value10){
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordU32x10(module->EventOffset + event_id, value1, value2, value3, value4, value5, value6, value7, value8, value9, value10);
}

void LLTRACE_IMPL_record_event_end(int32_t group_id, int32_t event_id){
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordEndCall(module->EventOffset + event_id);
}

void LLTRACE_IMPL_record_event_end_u32(int32_t group_id, int32_t event_id, uint32_t value1){
	SEGGER_SYSVIEW_MODULE* module = (SEGGER_SYSVIEW_MODULE*) group_id;
	SEGGER_SYSVIEW_RecordEndCallU32(module->EventOffset + event_id, value1);
}

