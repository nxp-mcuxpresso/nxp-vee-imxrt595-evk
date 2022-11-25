/* 
 * C
 *
 * Copyright 2015-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#if !defined TOUCH_HELPER_H
#define TOUCH_HELPER_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdint.h>

/* API -----------------------------------------------------------------------*/

/*
 * @brief Notifies to an event handler a touch has been pressed.
 *
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 */
void TOUCH_HELPER_pressed(int32_t x, int32_t y);

/*
 * @brief Notifies to an event handler a touch has moved.
 *
 * @param x the pointer X coordinate
 * @param y the pointer Y coordinate
 */
void TOUCH_HELPER_moved(int32_t x, int32_t y);

/*
 * @brief Notifies to an event handler a touch has been released.
 */
void TOUCH_HELPER_released(void);

#endif // !defined TOUCH_HELPER_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

