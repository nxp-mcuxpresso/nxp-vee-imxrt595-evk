/*
 * C
 *
 * Copyright 2017-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef OSAL_H
#define OSAL_H

/**
 * @file
 * @brief OS Abstraction Layer API
 * @author MicroEJ Developer Team
 * @version 0.2.0
 * @date 25 January 2018
 */

#include <stdint.h>

/** @brief define an infinite time */
#define OSAL_INFINITE_TIME	0xFFFFFFFF

/** @brief return code list */
typedef enum {
	OSAL_OK,
	OSAL_ERROR,
	OSAL_NOMEM,
	OSAL_WRONG_ARGS,
	OSAL_NOT_IMPLEMENTED
} OSAL_status_t;

/** @brief task function entry point */
typedef void*( *OSAL_task_entry_point_t)( void *args);

/** @brief OS task handle */
typedef void* OSAL_task_handle_t;

/** @brief OS queue handle */
typedef void* OSAL_queue_handle_t;

/** @brief OS counter semaphore handle */
typedef void* OSAL_counter_semaphore_handle_t;

/** @brief OS binary semaphore handle */
typedef void* OSAL_binary_semaphore_handle_t;

/** @brief OS mutex handle */
typedef void* OSAL_mutex_handle_t;

/*
 * Each OSAL port has a unique osal_portmacro.h header file.
 *
 * This file must declare the following macro:
 * - OSAL_task_stack_declare:
 * 		@brief Declare a task stack.
 * 		@param[in] _name name of the variable that defines the stack.
 * 		@param[in] _size size of the stack in bytes. _size must be compile time constant value.
 * 		OSAL_task_stack_declare(_name, _size);
 *
 * This file must declare the following type:
 * - OSAL_task_stack_t: OS task stack
 */
#include "osal_portmacro.h"

#ifndef  OSAL_task_stack_declare
	#error "osal_portmacro.h doesn't define OSAL_task_stack_declare() macro."
#endif

/**
 * @brief Create an OS task and start it.
 *
 * @param[in] entry_point function called at task startup
 * @param[in] name the task name
 * @param[in] stack task stack declared using OSAL_task_stack_declare() macro
 * @param[in] priority task priority
 * @param[in] parameters task entry parameters. NULL if no entry parameters
 * @param[in,out] handle pointer on a task handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_task_create(OSAL_task_entry_point_t entry_point, uint8_t* name, OSAL_task_stack_t stack, int32_t priority, void* parameters, OSAL_task_handle_t* handle);

/**
 * @brief Delete an OS task and start it.
 *
 * @param[in] handle pointer on the task handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_task_delete(OSAL_task_handle_t* handle);

/**
 * @brief Create an OS queue with a predefined queue size.
 *
 * @param[in,out] handle pointer on a queue handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_queue_create(uint8_t* name, uint32_t size, OSAL_queue_handle_t* handle);

/**
 * @brief Delete an OS queue.
 *
 * @param[in] handle pointer on the queue handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_queue_delete(OSAL_queue_handle_t* handle);

/**
 * @brief Post a message in an OS queue.
 *
 * @param[in] handle pointer on the queue handle
 * @param[in] msg message to post in the message queue
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_queue_post(OSAL_queue_handle_t* handle, void* msg);

/**
 * @brief Fetch a message from an OS queue. Blocks until a message arrived or a timeout occurred.
 *
 * @param[in] handle pointer on the queue handle
 * @param[in,out] msg message fetched in the OS queue
 * @param[in] timeout maximum time to wait for message arrival, OSAL_INFINITE_TIME for infinite timeout
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_queue_fetch(OSAL_queue_handle_t* handle, void** msg, uint32_t timeout);

/**
 * @brief Create an OS counter semaphore with a semaphore count initial value.
 *
 * @param[in] name counter semaphore name
 * @param[in] initial_count counter semaphore initial count value
 * @param[in] max_count counter semaphore maximum count value
 * @param[in,out] handle pointer on a counter semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_counter_semaphore_create(uint8_t* name, uint32_t initial_count, uint32_t max_count, OSAL_counter_semaphore_handle_t* handle);

/**
 * @brief Delete an OS counter semaphore.
 *
 * @param[in] handle pointer on the counter semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_counter_semaphore_delete(OSAL_counter_semaphore_handle_t* handle);

/**
 * @brief Take operation on OS counter semaphore. Block the current task until counter semaphore
 * become available or timeout occurred. Decrease the counter semaphore count value by 1 and
 * block the current task if count value equals to 0.
 *
 * @param[in] handle pointer on the counter semaphore handle
 * @param[in] timeout maximum time to wait until the counter semaphore become available, OSAL_INFINITE_TIME for infinite timeout
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_counter_semaphore_take(OSAL_counter_semaphore_handle_t* handle, uint32_t timeout);

/**
 * @brief Give operation on OS counter semaphore. Increase the counter semaphore count value by 1 and unblock the current task if count value.
 * equals to 0.
 *
 * @param[in] handle pointer on the counter semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_counter_semaphore_give(OSAL_counter_semaphore_handle_t* handle);

/**
 * @brief Create an OS binary semaphore with a semaphore count initial value (0 or 1).
 *
 * @param[in] name counter semaphore name
 * @param[in] initial_count counter semaphore initial count value
 * @param[in,out] handle pointer on a binary semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_binary_semaphore_create(uint8_t* name, uint32_t initial_count, OSAL_binary_semaphore_handle_t* handle);

/**
 * @brief Delete an OS binary semaphore.
 *
 * @param[in] handle pointer on the binary semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_binary_semaphore_delete(OSAL_binary_semaphore_handle_t* handle);

/**
 * @brief Take operation on OS binary semaphore. Block the current task until binary semaphore
 * become available or timeout occurred. Decrease the binary semaphore count value by 1 and
 * block the current task if count value equals to 0.
 *
 * @param[in] handle pointer on the binary semaphore handle
 * @param[in] timeout maximum time to wait until the binary semaphore become available, OSAL_INFINITE_TIME for infinite timeout
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_binary_semaphore_take(OSAL_binary_semaphore_handle_t* handle, uint32_t timeout);

/**
 * @brief Give operation on OS binary semaphore. Increase the binary semaphore count value by 1 and unblock the current task if count value.
 * equals to 0.
 *
 * @param[in] handle pointer on the binary semaphore handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_binary_semaphore_give(OSAL_binary_semaphore_handle_t* handle);

/**
 * @brief Create an OS mutex.
 *
 * @param[in] name mutex name
 * @param[in,out] handle pointer on a mutex handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_mutex_create(uint8_t* name, OSAL_mutex_handle_t* handle);

/**
 * @brief Delete an OS mutex.
 *
 * @param[in] handle pointer on the mutex handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_mutex_delete(OSAL_mutex_handle_t* handle);

/**
 * @brief Take operation on OS mutex.
 *
 * @param[in] handle pointer on the mutex handle
 * @param[in] timeout maximum time to wait until the mutex become available, OSAL_INFINITE_TIME for infinite timeout
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_mutex_take(OSAL_mutex_handle_t* handle, uint32_t timeout);

/**
 * @brief Give operation on OS mutex.
 *
 * @param[in] handle pointer on the mutex handle
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_mutex_give(OSAL_mutex_handle_t* handle);

/**
 * @brief Disable the OS scheduler context switching. Prevent the OS from
 * scheduling the current thread calling #OSAL_disable_context_switching while
 * the OS scheduling is already disable has an undefined behavior. This method
 * may be called from an interrupt.
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_disable_context_switching(void);

/**
 * @brief Reenable the OS scheduling that was disabled by #OSAL_disable_context_switching.
 * This method may be called from an interrupt.
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_enable_context_switching(void);

/**
 * @brief Asleep the current task during specified number of milliseconds.
 *
 * @param[in] milliseconds number of milliseconds
 *
 * @return operation status (@see OSAL_status_t)
 */
OSAL_status_t OSAL_sleep(uint32_t milliseconds);

#endif // OSAL_H
