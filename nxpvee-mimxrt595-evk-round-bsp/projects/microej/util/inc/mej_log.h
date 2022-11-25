/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined MEJ_LOG_H
# define MEJ_LOG_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "fsl_debug_console.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------


// Main log function definition
#define MEJ_LOG_PRINTF PRINTF

#define MEJ_LOG(type, tag, fmt, ...)  do {\
	   MEJ_LOG_PRINTF("[" STRINGIFY(type) " - " STRINGIFY(tag) "] : " fmt, ##__VA_ARGS__ );\
	} while(0);

// module log enable
#define MEJ_LOG_MICROVG

// Log levels
#define MEJ_LOG_INFO_LEVEL
//#define MEJ_ERROR_INFO_LEVEL
//#define MEJ_WARNIING_INFO_LEVEL
//#define MEJ_CRITICAL_INFO_LEVEL



/* Module log function redirection */
#define MEJ_LOG_INFO_MAIN(fmt, ...) MEJ_LOG(INFO,MAIN,fmt, ##__VA_ARGS__ )
#define MEJ_LOG_ERROR_MAIN(fmt, ...) MEJ_LOG(ERROR,MAIN,fmt, ##__VA_ARGS__ )
#define MEJ_LOG_WARNING_MAIN(fmt, ...) MEJ_LOG(WARNING,MAIN,fmt, ##__VA_ARGS__ )
#define MEJ_LOG_CRITICAL_MAIN(fmt, ...) MEJ_LOG(CRITICAL,MAIN,fmt, ##__VA_ARGS__ )

/* Log macros declaration */
#define __STRINGIFY(x) #x
#define STRINGIFY(x) __STRINGIFY(x)

#define MEJ_LOG_MODULE_INFO(tag, fmt, ...) MEJ_LOG_INFO_##tag (fmt, ##__VA_ARGS__ )
#define MEJ_LOG_MODULE_ERROR(tag, fmt, ...) MEJ_LOG_ERROR_##tag (fmt, ##__VA_ARGS__ )
#define MEJ_LOG_MODULE_WARNING(tag, fmt, ...) MEJ_LOG_WARNING_##tag (fmt, ##__VA_ARGS__ )
#define MEJ_LOG_MODULE_CRITICAL(tag, fmt, ...) MEJ_LOG_CRITICAL_##tag (fmt, ##__VA_ARGS__ )

#endif // !defined __MEJ_LOG_H__

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

