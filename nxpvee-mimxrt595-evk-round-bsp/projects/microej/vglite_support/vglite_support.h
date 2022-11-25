/*
 * Copyright 2019 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

/*
 * @file
 * @brief MicroEJ MicroUI library low level API: implementation over VG-Lite
 * @author MicroEJ Developer Team
 * @version 3.0.0
 *
 * 1- make BOARD_InitVGliteClock function as public
 * 2- add BOARD_StopVGliteClock, BOARD_StartVGliteClock
 */

#ifndef _VGLITE_SUPPORT_H
#define _VGLITE_SUPPORT_H

//#include "fsl_fbdev.h"
//#include "fsl_fbdev_ssd1963.h"
#include "fsl_common.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * API
 ******************************************************************************/
#if defined(__cplusplus)
extern "C" {
#endif /* __cplusplus */

status_t BOARD_PrepareVGLiteController(void);

// added by MicroEJ
status_t BOARD_InitVGliteClock(void);

// added by MicroEJ
status_t BOARD_StopVGliteClock(void);

// added by MicroEJ
status_t BOARD_StartVGliteClock(void);

#if defined(__cplusplus)
}
#endif /* __cplusplus */

#endif /* _VGLITE_SUPPORT_H */
