/*
 * C
 *
 * Copyright 2013-2021 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include "fsl_inputmux.h"
#include "pin_mux.h"
#include "fsl_gpio.h"
#include "fsl_pint.h"
#include "microej.h"
#include "interrupts.h"
#include "LLMJVM.h"
#include "buttons_manager.h"
#include "buttons_helper.h"
#include "buttons_helper_configuration.h"
#include "board.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/* @brief SW button (common) */
#define SW_GPIO_PORT 0U

/* @brief SW1 button */
#define SW1_GPIO_PIN 25U

/* @brief SW2 button */
#define SW2_GPIO_PIN 10U

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

/* @brief Button descriptor */
typedef struct {
	int id;							// button id
	int reversed;					// GPIO state reversed
	int port;						// GPIO port
	int pin;						// GPIO pin
	IRQn_Type irq_num;				// IRQ number
	gpio_interrupt_index_t irq_idx;	// IRQ index
} buttons_desc_t;

// -----------------------------------------------------------------------------
// Global Variables
// -----------------------------------------------------------------------------

/* @brief Button descriptors */
const static buttons_desc_t button_descs[BUTTONS_HELPER_NUMBER_OF_BUTTONS] = {
	// descriptor of button 1
	{
		BUTTON_SW1,			// id
		MICROEJ_TRUE,		// reversed
		SW_GPIO_PORT,		// GPIO port
		SW1_GPIO_PIN,		// GPIO pin
		GPIO_INTA_IRQn,		// GPIO IRQ number
		kGPIO_InterruptA,	// GPIO IRQ idx
	},

	// descriptor of button 2
	{
		BUTTON_SW2,			// id
		MICROEJ_TRUE,		// reversed
		SW_GPIO_PORT,		// GPIO port
		SW2_GPIO_PIN,		// GPIO pin
		GPIO_INTA_IRQn,		// GPIO IRQ number
		kGPIO_InterruptA,	// GPIO IRQ idx
	},
};

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

/*
 * @brief Initializes the BUTTONS_MANAGER module
 */
static void __buttons_manager_init();

/*
 * @brief Manages the states of the buttons and sends events accordingly
 *
 * @param[in] button: Identifier of the button
 * @param[in] state: State of the button
 */
static void __buttons_manager_send_events(button_id_t button, int state);

/*!
 * @brief GPIO IRQ handler
 *
 * This function overrides the default GPIO IRQ handler when a button triggers
 * the interrupt
 *
 * @param[in] button: Identifier of the button
 * @param[in] state: State of the button (0 or 1)
 */
static void __buttons_manager_interrupt(button_id_t button, int state);

// -----------------------------------------------------------------------------
// Public functions
// -----------------------------------------------------------------------------

/*!
 * @brief Buttons IRQ handler
 */
void BUTTONS_MANAGER_interrupt(void)
{
	buttons_desc_t d;

	// Find the source (GPIO) of the interrupt
	for(int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++) {
		d = button_descs[i];

		if ((1 << d.pin) == GPIO_PortGetInterruptStatus(GPIO, d.port, kGPIO_InterruptA)) {
#if defined VM_DUMP_ON_BUTTON
			LLMJVM_dump();
#endif

			// Toggle polarity to capture falling and rising edges
			int v = GPIO->INTPOL[d.port];
			int m = 1 << d.pin; // build mask from button id
			GPIO->INTPOL[d.port] = (v & ~m) | (~v & m);

			__buttons_manager_interrupt((button_id_t) i, ~v & m);

			// clear the interrupt status
			GPIO_PinClearInterruptFlag(GPIO, d.port, d.pin, 0);
		}
	}

	/* Add for ARM errata 838869, affects Cortex-M4, Cortex-M4F Store immediate overlapping
	   exception return operation might vector to incorrect interrupt */
#if defined __CORTEX_M && (__CORTEX_M == 4U)
	__DSB();
#endif
}

// See the header file for the function documentation
void BUTTONS_MANAGER_enable_interrupts(void)
{
	for(int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++)
	{
		EnableIRQ(button_descs[i].irq_num);
	}
}

// See the header file for the function documentation
void BUTTONS_MANAGER_disable_interrupts(void)
{
	for(int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++)
	{
		DisableIRQ(button_descs[i].irq_num);
	}
}

// See the header file for the function documentation
void BUTTONS_MANAGER_initialize(void)
{
	BUTTONS_HELPER_initialize();
	__buttons_manager_init();
}

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

// See the header file for the function documentation
void __buttons_manager_interrupt(button_id_t button, int state)
{
	// Set the interrupt flag
	int leave = interrupt_enter();

	// Send the events
	__buttons_manager_send_events(button, state);

	// Reset the interrupt flag
	interrupt_leave(leave);
}

// See the section 'Internal function definitions' for the function documentation
static void __buttons_manager_init()
{
	// Define the init structure for the input switch pin
	gpio_pin_config_t sw_config = {kGPIO_DigitalInput, 0};
	gpio_interrupt_config_t config = {kGPIO_PinIntEnableLevel, kGPIO_PinIntEnableLowOrFall};
	buttons_desc_t d;

	// enable GPIO interrupts
	BUTTONS_MANAGER_enable_interrupts();

	// Initialize GPIO ports
	for (int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++) {
		GPIO_PortInit_noreset(GPIO, button_descs[i].port);
	}

	// Initialize GPIO pins
	for (int i = 0; i < BUTTONS_HELPER_NUMBER_OF_BUTTONS; i++) {
		d = button_descs[i];

		GPIO_PinInit(GPIO, d.port, d.pin, &sw_config);

		// Enable GPIO pin interrupt
		GPIO_SetPinInterruptConfig(GPIO, d.port, d.pin, &config);
		GPIO_PinEnableInterrupt(GPIO, d.port, d.pin, d.irq_idx);
	}
}

// See the section 'Internal function definitions' for the function documentation
static void __buttons_manager_send_events(button_id_t button, int state)
{
	int button_pressed;
	buttons_desc_t d = button_descs[button];
	
	if (state)
	{
		// GPIO == 1
		if (d.reversed == MICROEJ_TRUE)
		{
			// GPIO == 1 means "released"
			button_pressed = MICROEJ_FALSE;
		}
		else
		{
			// GPIO == 1 means "pressed"
			button_pressed = MICROEJ_TRUE;
		}
	}
	else
	{
		// GPIO == 0
		if (d.reversed == MICROEJ_TRUE)
		{
			// GPIO == 0 means "pressed"
			button_pressed = MICROEJ_TRUE;
		}
		else
		{
			// GPIO == 0 means "released"
			button_pressed = MICROEJ_FALSE;
		}
	}

	// Send events
	if (button_pressed == MICROEJ_TRUE)
	{
		BUTTONS_HELPER_pressed(d.id);
	}
	else
	{
		BUTTONS_HELPER_released(d.id);
	}
}

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

