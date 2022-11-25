/*
 * C
 *
 * Copyright 2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined __MEJ_DEBUG_H__
# define __MEJ_DEBUG_H__

#include <stdint.h>

 #include "fsl_debug_console.h"

// --------------------------------------------------------------------------------
// Macros and Defines
// --------------------------------------------------------------------------------

//#define MEJ_DEBUG_ENABLED

#define MEJ_DEBUG_LVL(i)        ( 1 << (i) )

#define MEJ_DEBUG_DISPLAY_IMPL_LVL          MEJ_DEBUG_LVL(1)
#define MEJ_DEBUG_DISPLAY_DMA_LVL           MEJ_DEBUG_LVL(2)
#define MEJ_DEBUG_DISPLAY_TASK_LVL          MEJ_DEBUG_LVL(3)
#define MEJ_DEBUG_MICROUI_LVL               MEJ_DEBUG_LVL(4)
#define MEJ_DEBUG_VGLITE_LVL                MEJ_DEBUG_LVL(5)
#define MEJ_DEBUG_MICROUI_DRAWINGS_LVL      MEJ_DEBUG_LVL(6)

#if defined MEJ_DEBUG_ENABLED
#define MEJ_DEBUG(lvl, fmt, ...)     if (lvl & __mej_debug_lvls) PRINTF("%s: " fmt, __func__, ##__VA_ARGS__)
#else
#define MEJ_DEBUG(...)
#endif

// --------------------------------------------------------------------------------
// Globals
// --------------------------------------------------------------------------------
extern uint32_t __mej_debug_lvls;

void MEJ_DEBUG_enable(int level);
void MEJ_DEBUG_disable(int level);

#endif // !defined __MEJ_DEBUG_H__
