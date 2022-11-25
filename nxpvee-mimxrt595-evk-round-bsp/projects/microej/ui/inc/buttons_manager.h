/*
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined BUTTONS_MANAGER_H
#define BUTTONS_MANAGER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>
#include "buttons_helper_configuration.h"

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*!
 * @brief Initializes the buttons_manager module
 */
void BUTTONS_MANAGER_initialize(void);

/*
 * @brief Enable the GPIO hardware interrupts
 */
void BUTTONS_MANAGER_enable_interrupts(void);

/*
 * @brief Disable the GPIO hardware interrupts
 */
void BUTTONS_MANAGER_disable_interrupts(void);

/*
 * @brief Buttons interrupt
 */
void BUTTONS_MANAGER_interrupt(void);

#endif // !defined BUTTONS_MANAGER_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

