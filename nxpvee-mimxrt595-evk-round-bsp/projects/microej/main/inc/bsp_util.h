/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef BSP_UTIL_H
#define BSP_UTIL_H

/* Includes ------------------------------------------------------------------*/

#include <stdint.h>


/* Defines -------------------------------------------------------------------*/

/*
 * MicroEJ buffer declaration. Used "BSP_DECLARE_BUFFER(XXX)" macro to
 * declare a MicroEJ buffer. It generates a "XXX_START" and "XXX_END"
 * constant value with the start address of the buffer in "XXX_START"
 * and the end address of the buffer in "XXX_END".
 */

#if defined( __GNUC__ )
#define BSP_DECLARE_BUFFER(NAME)				\
	void* __ ## NAME ## _Start__(void);		\
	void* __ ## NAME ## _End__(void);		\
	const uint32_t NAME ## _START = (uint32_t)&__ ## NAME ## _Start__;	\
	const uint32_t NAME ## _END = (uint32_t)&__ ## NAME ## _End__;
#endif // __GNUC__

#if defined( __CC_ARM )
#define BSP_DECLARE_BASE(NAME)	Image$$ ## NAME ## $$ZI$$Base
#define BSP_DECLARE_LIMIT(NAME)	Image$$ ## NAME ## $$ZI$$Limit
#define BSP_DECLARE_BUFFER(NAME)				\
	extern uint32_t BSP_DECLARE_BASE(NAME);		\
	extern uint32_t BSP_DECLARE_LIMIT(NAME);	\
	const uint32_t NAME ## _START = (uint32_t)&BSP_DECLARE_BASE(NAME);	\
	const uint32_t NAME ## _END = (uint32_t)&BSP_DECLARE_LIMIT(NAME);
#endif // __CC_ARM

#if defined( __ICCARM__ )
#define BSP_DECLARE_BASE(NAME)	NAME ## $$Base
#define BSP_DECLARE_LIMIT(NAME)	NAME ## $$Limit
#define BSP_DECLARE_BUFFER(NAME)				\
	extern uint32_t BSP_DECLARE_BASE(NAME);		\
	extern uint32_t BSP_DECLARE_LIMIT(NAME);	\
	const uint32_t NAME ## _START = (uint32_t)&BSP_DECLARE_BASE(NAME);	\
	const uint32_t NAME ## _END = (uint32_t)&BSP_DECLARE_LIMIT(NAME);
#endif // __ICCARM__

/*
 * MicroEJ weak function declaration. Used "BSP_DECLARE_WEAK_FCNT" macro to
 * declare a MicroEJ weak function as follow:
 * 	export my_return_type BSP_DECLARE_WEAK_FCNT my_function(my_params);
 */
 
#if defined( __CC_ARM ) || defined(__GNUC__)
#define BSP_DECLARE_WEAK_FCNT	__attribute__((weak))
#endif // __CC_ARM || __GNUC__

#if defined( __ICCARM__ )
#define BSP_DECLARE_WEAK_FCNT	__weak
#endif // __ICCARM__

#endif // BSP_UTIL_H
