/*
 * Copyright 2014-2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2019, 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 * Copyright 2020-2022 MicroEJ Corp. This file has been modified by MicroEJ Corp.
 */

/* Compiler includes. */
#if defined(__ICCARM__)
#include <intrinsics.h>
#endif

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"

#if configUSE_TICKLESS_IDLE == 2
#include "fsl_rtc.h"
#else
#include "fsl_device_registers.h"
#endif

#include "fsl_tickless_rtc.h"
#include "power_manager.h"
#include "fsl_debug_console.h"
#include "fsl_power.h"

#include "cpuload.h"
#include "board.h"
#include "fsl_gpio.h"
#include "time_hardware_timer.h"


extern uint32_t SystemCoreClock; /* in Kinetis SDK, this contains the system core clock speed */

/*
 * LPT timer base address and interrupt number
 */
#if configUSE_TICKLESS_IDLE == 2
extern RTC_Type *vPortGetRtcBase(void);
extern IRQn_Type vPortGetRtcIrqn(void);
#endif /* configUSE_TICKLESS_IDLE */


#if configUSE_TICKLESS_IDLE == 2

RTC_Type *vPortGetRtcBase(void)
{
    return RTC;
}

IRQn_Type vPortGetRtcIrqn(void)
{
    return RTC_IRQn;
}

void vPortSuppressTicksAndSleep(TickType_t xExpectedIdleTime)
{
    uint32_t ulReloadValue;
    RTC_Type *pxRtcBase;
    uint32_t power_down_config[4] = {0};

    uint32_t initial_rtc_count;

    uint32_t power_down_mode = power_manager_get_mode(power_down_config);

#ifdef PM_DEBUG
    PRINTF("power_down_mode:0x%x\n", power_down_mode);
#endif

    if (power_down_mode == PM_ACTIVE)
        return;

    pxRtcBase = vPortGetRtcBase();
    if (pxRtcBase == 0)
        return;

    if (xExpectedIdleTime == 0)
        return;
    /* Calculate the reload value required to wait xExpectedIdleTime
    tick periods.  -1 is used because this code will execute part way
    through one of the tick periods. */
    ulReloadValue = ((uint32_t)configRTCWAKE_CLOCK_HZ * ((uint32_t)xExpectedIdleTime - 1UL))  / (uint32_t)configTICK_RATE_HZ;

    /* Make sure the SysTick reload value does not overflow the counter. */
    if (ulReloadValue > portMAX_16_BIT_NUMBER) {
        ulReloadValue = portMAX_16_BIT_NUMBER;
    }


    /* Enter a critical section but don't use the taskENTER_CRITICAL()
    method as that will mask interrupts that should exit sleep mode. */
    __disable_irq();
    __DSB();
    __ISB();

    /* If a context switch is pending or a task is waiting for the scheduler
    to be unsuspended then abandon the low power entry. */
    if (eTaskConfirmSleepModeStatus() == eAbortSleep)
    {
        /* Re-enable interrupts - see comments above __disable_irq()
        call above. */
        __enable_irq();
    }
    else
    {
        /* Stop systick. */
        SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
        /* Register RTC counter when tick counter is stopped */
        initial_rtc_count = time_hardware_timer_getCurrentTime();
        /* Stop application timer.
         * Relying on deep sleep enter/exit to pause this timer
         * would cause some drift difficult to compensate
         * because of the time spent in SDK power API calls.
         * So: stop it and restart it at the same time than Systick.
         */
        cpuload_enter_sleep();

#ifdef PM_DEBUG
        PRINTF("enter_sleep\n");
#endif
        /* Start RTC wake countdown. */
        RTC_SetWakeupCount(pxRtcBase, ulReloadValue);

        if (power_down_mode == PM_SLEEP) {
            POWER_EnterSleep();
        } else if (power_down_mode == PM_DEEP_SLEEP) {
        	/* Enable GPIO WakeUp interrupt */
        	GPIO_WakeUp_Enable();
        	/* Enter DeepSleep */
        	POWER_EnterDeepSleep(power_down_config);
        	/* Disable GPIO WakeUp interrupt */
        	GPIO_WakeUp_Disable();
        }

#ifdef PM_DEBUG
        PRINTF("exit_sleep\n");
#endif
        /* Stop RTC when CPU waked up then set SysTick->LOAD back to its standard
        value.  The critical section is used to ensure the tick interrupt
        can only execute once in the case that the reload register is near
        zero. */
        RTC_EnableWakeupTimer(RTC, false);
        portENTER_CRITICAL();
        {
            /* Update tick counter with time spent in sleep*/
        	uint32_t stepTick = (((time_hardware_timer_getCurrentTime() - initial_rtc_count) * (uint32_t)configTICK_RATE_HZ)/1000);
           if(stepTick > xExpectedIdleTime)
           {
           	stepTick = xExpectedIdleTime;
           }
           vTaskStepTick(stepTick);

            cpuload_exit_sleep();
            SysTick->LOAD = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
            SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
        }
        portEXIT_CRITICAL();
        /* Re-enable interrupts - see comments above __disable_irq()
        call above. */
        __enable_irq();
        __NOP();

    }
}

/*
 * Setup the systick timer to generate the tick interrupts at the required
 * frequency.
 */
void vPortSetupTimerInterrupt(void)
{
    NVIC_EnableIRQ(vPortGetRtcIrqn());

    /* Configure SysTick to interrupt at the requested rate. */
    SysTick->LOAD = (configCPU_CLOCK_HZ / configTICK_RATE_HZ) - 1UL;
    SysTick->VAL  = 0UL;
    SysTick->CTRL = (SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_TICKINT_Msk | SysTick_CTRL_ENABLE_Msk);
}
#endif /* configUSE_TICKLESS_IDLE */
