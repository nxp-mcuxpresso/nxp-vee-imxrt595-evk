/*
 * C
 *
 * Copyright 2019-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "display_support.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "board.h"

#include "fsl_gpio.h"
#include "fsl_debug_console.h"

#include "fsl_ft3267.h"

#include "touch_helper.h"

#include "mej_log.h"

// RTOS
#include "FreeRTOS.h"
#include "semphr.h"


// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/*
 * @brief Task delay (in ms) between detecting a multiple pressed events
 */
#define TOUCH_DELAY 20

/*
 * @brief Task stack size
 */
#define TOUCH_STACK_SIZE  (128)

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/**
 * Retrieves the touch state and coordinates
 */
static void __touch_manager_read(void);

/**
 * Touch thread routine
 */
static void __touch_manager_task(void *pvParameters);

/*
 * @brief Pulls the reset pin of the touch controller
 */
void __touch_manager_pull_reset_pin(bool pullUp);

// -----------------------------------------------------------------------------
// Static Constants
// -----------------------------------------------------------------------------

static const ft3267_config_t s_touchConfig = {
		.I2C_ReceiveFunc  = BOARD_MIPIPanelTouch_I2C_Receive,
		.pullResetPinFunc = __touch_manager_pull_reset_pin,
		.pullPowerPinFunc = NULL,
		.timeDelayMsFunc  = VIDEO_DelayMs,
};

// -----------------------------------------------------------------------------
// Static Variables
// -----------------------------------------------------------------------------

static ft3267_handle_t s_touchHandle;
static volatile bool s_touchEvent = false;

/*
 * @brief vglite operation semaphore
 */
static SemaphoreHandle_t touch_interrupt_sem;

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void TOUCH_MANAGER_initialize(void)
{
	status_t status;
	const gpio_pin_config_t resetPinConfig  = {.pinDirection = kGPIO_DigitalOutput, .outputLogic = 0};
	const gpio_pin_config_t intPinConfig    = {.pinDirection = kGPIO_DigitalInput, .outputLogic = 0};
	gpio_interrupt_config_t intPinIntConfig = {kGPIO_PinIntEnableEdge, kGPIO_PinIntEnableLowOrFall};

	GPIO_PinInit(GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PORT, BOARD_MIPI_PANEL_TOUCH_RST_PIN, &resetPinConfig);

	status = FT3267_Init(&s_touchHandle, &s_touchConfig);

	if (kStatus_Success != status)
	{
		PRINTF("Touch IC initialization failed\r\n");
		assert(false);
	}

	touch_interrupt_sem = xSemaphoreCreateBinary();

	GPIO_PinInit(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, BOARD_MIPI_PANEL_TOUCH_INT_PIN, &intPinConfig);
	GPIO_SetPinInterruptConfig(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, BOARD_MIPI_PANEL_TOUCH_INT_PIN, &intPinIntConfig);
	GPIO_PinEnableInterrupt(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, BOARD_MIPI_PANEL_TOUCH_INT_PIN, kGPIO_InterruptA);

	/* Create the touch screen task */
	(void)xTaskCreate(__touch_manager_task, "Touch screen task", TOUCH_STACK_SIZE, NULL, 13, NULL);
}

void TOUCH_MANAGER_interrupt(void)
{
	uint32_t intStat = GPIO_PortGetInterruptStatus(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, kGPIO_InterruptA);
	if (intStat & (1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN))
	{
		GPIO_PortClearInterruptFlags(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, kGPIO_InterruptA,
				(1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN));

		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		xSemaphoreGiveFromISR(touch_interrupt_sem, &xHigherPriorityTaskWoken);
		if(xHigherPriorityTaskWoken != pdFALSE )
		{
			// Force a context switch here.
			portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
		}
	}
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the section 'Internal function definitions' for the function documentation
static void __touch_manager_read(void)
{
	touch_event_t touch_event;
	int touch_x;
	int touch_y;

	if (kStatus_Success == FT3267_GetSingleTouch(&s_touchHandle, &touch_event, &touch_x, &touch_y))
	{
		if (touch_event == kTouch_Down || touch_event == kTouch_Contact)
		{
			TOUCH_HELPER_pressed(FRAME_BUFFER_WIDTH - touch_x, FRAME_BUFFER_HEIGHT - touch_y);
		}
		else if (touch_event== kTouch_Up)
		{
			// Send Release event
			TOUCH_HELPER_released();
		}
	}
}

// See the section 'Internal function definitions' for the function documentation
static void __touch_manager_task(void *pvParameters)
{
	while (1)
	{
		/* Suspend ourselves */
		xSemaphoreTake(touch_interrupt_sem, portMAX_DELAY);

		/* We have been woken up, lets work ! */
		__touch_manager_read();
	}
}

// See the section 'Internal function definitions' for the function documentation
void __touch_manager_pull_reset_pin(bool pullUp)
{
	if (pullUp)
	{
		GPIO_PinWrite(GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PORT, BOARD_MIPI_PANEL_TOUCH_RST_PIN, 1);
	}
	else
	{
		GPIO_PinWrite(GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PORT, BOARD_MIPI_PANEL_TOUCH_RST_PIN, 0);
	}
}


// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

