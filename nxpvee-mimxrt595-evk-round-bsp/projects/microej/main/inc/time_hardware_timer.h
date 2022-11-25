/*
 * C
 *
 * Copyright 2012-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */
#ifndef __TIME_HARDWARE_TIMER_H
#define __TIME_HARDWARE_TIMER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

/**
 * Provide functions to get time using an hardware timer
 */
void time_hardware_timer_initialize(void);
void time_hardware_timer_wait_ready(void);
int64_t time_hardware_timer_getCurrentTime(void);
int64_t time_hardware_timer_getTimeNanos(void);
int64_t time_hardware_timer_getTimeUs(void);

#endif /* __TIME_HARDWARE_TIMER_H */

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

