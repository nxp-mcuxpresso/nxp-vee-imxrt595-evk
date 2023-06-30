/*
 * C
 *
 * Copyright 2020-2022 MicroEJ Corp. All rights reserved.
* Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "FreeRTOS.h"

#include "task.h"

#include "fsl_debug_console.h"

#include "mej_log.h"

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

void vApplicationStackOverflowHook( TaskHandle_t pxTask, signed char *pcTaskName ) {
    MEJ_LOG_MODULE_ERROR(MAIN, "Stack overflow in task: %s\n", pcTaskName);

    do { } while(1);
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

