/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*
 * @brief Initializes the touch manager module
 */
void TOUCH_MANAGER_initialize(void);

/*
 * @brief Touch interrupt
 */
void TOUCH_MANAGER_interrupt(void);

#endif // !defined TOUCH_MANAGER_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

