/*
 * Copyright 2020, 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

#ifndef _POWER_MANAGER_H_
#define _POWER_MANAGER_H_

#include "display_framebuffer.h"

void power_manager_init(void);
void GPIO_WakeUp_Enable(void);
void GPIO_WakeUp_Disable(void);
static void GPIO_WakeUp_Power_Enable_Update(void);

typedef enum {
	POWER_GLOBAL = 0,
	POWER_DMA,
	POWER_GPU,
	POWER_SDMA,
	POWER_DEBUGGER,
	POWER_DOMAIN_NUMBER,
} power_domain_t;

typedef enum {
	LOW_POWER_FORBIDDEN = 0,
	LOW_POWER_AUTHORIZED,
} low_power_enabled_t;
void power_manager_enable_low_power(power_domain_t power_d, low_power_enabled_t enabled);

typedef enum {
	PM_ACTIVE = 0,
	PM_SLEEP  = 1,
	PM_DEEP_SLEEP = 2,
} power_mode_t;
power_mode_t power_manager_get_mode(uint32_t config[4]);

/* RAM partitions power on/off */

typedef enum {
	POWER_OFF = 0,
	POWER_ON,
} ram_power_level_t;

typedef enum {
	OTHERS_POWER_OFF = 0,
	OTHERS_DONT_CARE,
} others_power_level_t;

void power_manager_framebuffer_power_on(framebuffer_t * frameBuffer);
void power_manager_framebuffer_set_next_off(framebuffer_t * frameBuffer);
void power_manager_framebuffer_request_power_off(void);

#define POWER_PROFILE_MAX_PERFS     0xFF
#define POWER_PROFILE_POWER_SAVING  0x00

#define power_manager_set_profile Java_com_nxp_rt595_util_PowerManagementHelperNative_setPerfProfile

#endif
