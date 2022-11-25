/*
 * C
 *
 * Copyright 2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "mej_debug.h"

// -----------------------------------------------------------------------------
// Variables
// -----------------------------------------------------------------------------

uint32_t __mej_debug_lvls = 0;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * Enable a debug level
 * @param level: level to enable
 */
void MEJ_DEBUG_enable(int level) {
    __mej_debug_lvls |= level;
}

void MEJ_DEBUG_disable(int level) {
    __mej_debug_lvls &= ~(level);
}
