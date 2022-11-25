/*
 * C
 *
 * Copyright 2013-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
 
// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "microej.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

// Mutex used to define critical sections
static SemaphoreHandle_t LLSP_mutex = NULL;

// -----------------------------------------------------------------------------
// Jave Low Level APIs
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void LLSP_IMPL_initialize(void) {
	// Prevent preventive context switches only if necessary
	portENTER_CRITICAL();
	if( LLSP_mutex == NULL ) {
		LLSP_mutex = xSemaphoreCreateMutex();
	}
	portEXIT_CRITICAL();
}

// See the header file for the function documentation
void LLSP_IMPL_syncWriteBlockEnter(int32_t databaseID, int32_t blockID) {
	xSemaphoreTake(LLSP_mutex, portMAX_DELAY);
}

// See the header file for the function documentation
void LLSP_IMPL_syncWriteBlockExit(int32_t databaseID, int32_t blockID) {
	xSemaphoreGive(LLSP_mutex);
}

// See the header file for the function documentation
void LLSP_IMPL_syncReadBlockEnter(int32_t databaseID, int32_t blockID) {
	// Read synchronization not specific
	LLSP_IMPL_syncWriteBlockEnter(databaseID, blockID);
}

// See the header file for the function documentation
void LLSP_IMPL_syncReadBlockExit(int32_t databaseID, int32_t blockID) {
	// Read synchronization not specific
	LLSP_IMPL_syncWriteBlockExit(databaseID, blockID);
}

// See the header file for the function documentation
int32_t LLSP_IMPL_wait(void) {
	// Suspend ourselves until we are woken up
	vTaskSuspend( xTaskGetCurrentTaskHandle() );
	//return LLSP_OK;
        return 0;
}

// See the header file for the function documentation
void LLSP_IMPL_wakeup(int32_t taskID) {
	// TaskID is a handle.
	// See LLMJVM_IMPL_getCurrentTaskID() function in LLMJVM_FreeRTOS.c
	vTaskResume( (TaskHandle_t)taskID );
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

