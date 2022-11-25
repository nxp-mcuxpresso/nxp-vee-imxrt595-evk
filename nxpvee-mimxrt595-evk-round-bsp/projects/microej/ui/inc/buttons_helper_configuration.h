/*
 * C
 *
 * Copyright 2019-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined BUTTONS_HELPER_CONFIGURATION_H
#define BUTTONS_HELPER_CONFIGURATION_H

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// Number of buttons the helper has to manage
#define BUTTONS_HELPER_NUMBER_OF_BUTTONS	2

#define BUTTONS_HELPER_VM_DUMP_ON_BUTTON	1

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

typedef enum
{
	BUTTON_SW1 = 0,
	BUTTON_SW2,
} button_id_t;

#endif // !defined BUTTONS_HELPER_CONFIGURATION_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

