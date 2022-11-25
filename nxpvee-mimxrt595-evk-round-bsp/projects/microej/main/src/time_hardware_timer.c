/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include "time_hardware_timer.h"
#include "microej.h"
#include "fsl_rtc.h"

#include "fsl_debug_console.h"
#include "fsl_power.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Internal functions definition
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------


void RTC_IRQHandler(void)
{
    RTC_ClearStatusFlags(RTC, kRTC_WakeupFlag);
    SDK_ISR_EXIT_BARRIER;
}

// See the header file for the function documentation
void time_hardware_timer_initialize(void) {
    /* Enable 32KHz Oscillator clock -- needed for SUBSEC counter*/
    CLOCK_EnableOsc32K(true);

    /* Initialize RTC timer used */
    RTC_Deinit(RTC);
    RTC_Init(RTC);

    /* Reset RTC */
    RTC_Reset(RTC);

    /* Enable RTC + RTC->SUBSEC */
    RTC_EnableTimer(RTC, true);
    RTC->CTRL |= RTC_CTRL_RTC_SUBSEC_ENA_MASK;

    /* Clear RTC COUNT*/
    RTC->COUNT = 0;


    /* enable RTC interrupt as deep sleep wakeup source */
    RTC_EnableWakeUpTimerInterruptFromDPD(RTC, true);
    RTC_EnableInterrupts(RTC, RTC_CTRL_WAKE1KHZ_MASK);
    EnableDeepSleepIRQ(RTC_IRQn);
}

void time_hardware_timer_wait_ready(void) {
	// Wait for 1sec to have SUBSEC stable
	while(0 == RTC->COUNT){};
}

// See the header file for the function documentation
int64_t time_hardware_timer_getTimeUs(void) {
    int32_t rtc_count;
    int32_t rtc_subsec;
    int64_t rtc_count_us;
    int32_t rtc_subsec_us;
    int64_t rtc_time_us;

    // Datasheet says to read RTC->COUNT twice until same value
    // Second read or RTC->COUNT is done after the read of RTC->SUBSEC
    // to ensure that no overload occurs before read of RTC->SUBSEC
    do {
        rtc_count = RTC->COUNT;
        // Read SUBSEC twice to avoid reading during a counter update.
        do {
        rtc_subsec = RTC->SUBSEC;
        } while (rtc_subsec != RTC->SUBSEC);
    } while (rtc_count != RTC->COUNT);

    rtc_count_us = COUNT_TO_USEC(rtc_count, 1);
    rtc_subsec_us = COUNT_TO_USEC(rtc_subsec, CLK_RTC_32K_CLK);
    rtc_time_us = rtc_count_us + rtc_subsec_us;

    return (rtc_time_us);
}

// See the header file for the function documentation
int64_t time_hardware_timer_getCurrentTime(void) {
    return time_hardware_timer_getTimeUs() / 1000;
}

// See the header file for the function documentation
int64_t time_hardware_timer_getTimeNanos(void) {
    return time_hardware_timer_getTimeUs() * 1000;
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

