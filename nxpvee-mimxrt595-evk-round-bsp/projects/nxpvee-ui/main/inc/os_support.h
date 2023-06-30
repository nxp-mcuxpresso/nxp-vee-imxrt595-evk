/*
 * C
 *
 * Copyright 2012-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef _OS_SUPPORT_H
#define _OS_SUPPORT_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "microej.h"

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

/**
 * Prevent the OS from scheduling the current thread
 * Calling OS_SUPPORT_disable_context_switching while the OS scheduling is already disable has an undefined behavior.
 * This method may be called from an interrupt.
 */
void OS_SUPPORT_disable_context_switching(void);

/**
 * Reenable the OS scheduling that was disabled by OS_SUPPORT_disable_context_switching.
 * This method may be called from an interrupt.
 */
void OS_SUPPORT_enable_context_switching(void);

#endif //_OS_SUPPORT_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

