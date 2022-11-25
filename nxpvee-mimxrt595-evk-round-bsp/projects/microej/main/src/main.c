/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

// Driver includes
#include "fsl_debug_console.h"
#include "fsl_power.h"
#include "fsl_powerquad.h"

// OS includes
#include "FreeRTOS.h"
#include "task.h"

// Platform includes
#include "board.h"
#include "pin_mux.h"

#include "monitor.h"
#include "trace_platform.h"
#include "touch_manager.h"
#include "buttons_manager.h"
#include "display_support.h"
#include "time_hardware_timer.h"

#include "microjvm_main.h"
#include "mej_log.h"

#include "power_manager.h"

#if (ENABLE_SVIEW == 1)
#include "SEGGER_SYSVIEW.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define MICROJVM_STACK_SIZE         (20 * 1024)
#define JAVA_TASK_PRIORITY          (11)                       /** Should be > tskIDLE_PRIORITY & < configTIMER_TASK_PRIORITY */
#define JAVA_TASK_STACK_SIZE        (MICROJVM_STACK_SIZE / 4)

/*******************************************************************************
 * Code
 ******************************************************************************/

void GPIO_INTA_IRQHandler(void)
{
	BOARD_DisplayTEPinHandler();
	TOUCH_MANAGER_interrupt();
	BUTTONS_MANAGER_interrupt();
}

/*!
 * @brief Java task function that runs in an OS task
 */
void xJavaTaskFunction(void * pvParameters) {
#if MONITOR_ENABLED == 1
    monitor_init();
#endif

	microjvm_main();
	vTaskDelete(xTaskGetCurrentTaskHandle());
}

/*!
 * @brief Main function
 */

int main(void) {

       /* Init timer first as we need to wait 1sec for stability */
	time_hardware_timer_initialize();

	/* Init board hardware. */

	/* Init pins */
	BOARD_InitPins();
	BOARD_InitPsRamPins();

	BOARD_InitMipiPanelPins();

#ifndef MEJ_DISABLED_DISPLAY_TEARING
	GPIO_PortInit_noreset(GPIO, BOARD_MIPI_TE_PORT);
#endif

	GPIO_PortInit_noreset(GPIO, BOARD_MIPI_POWER_PORT);
	GPIO_PortInit_noreset(GPIO, BOARD_MIPI_BL_PORT);
	GPIO_PortInit_noreset(GPIO, BOARD_MIPI_RST_PORT);
	GPIO_PortInit_noreset(GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PORT);

	/* Configure main clock. */
	BOARD_BootClockRUN();

	/* Init debug console */
	BOARD_InitDebugConsole();

	/* Initialize power management. */
	power_manager_init();

	/* Use 48 MHz clock for the FLEXCOMM4 */
	CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM4);
	BOARD_MIPIPanelTouch_I2C_Init();

	MEJ_LOG_MODULE_INFO(MAIN, "Initialize PowerQuad\n");
	/* Power up PQ RAM. */
	POWER_DisablePD(kPDRUNCFG_PPD_PQ_SRAM);
	/* Apply power setting. */
	POWER_ApplyPD();

	PQ_Init(POWERQUAD);

	MEJ_LOG_MODULE_INFO(MAIN, "Board init finished.\r\n");

	int silicon_rev_major = (SYSCTL0->SILICONREV_ID & SYSCTL0_SILICONREV_ID_MAJOR_MASK) >> SYSCTL0_SILICONREV_ID_MAJOR_SHIFT;
	int silicon_rev_minor = (SYSCTL0->SILICONREV_ID & SYSCTL0_SILICONREV_ID_MINOR_MASK) >> SYSCTL0_SILICONREV_ID_MINOR_SHIFT;

	MEJ_LOG_MODULE_INFO(MAIN, "Silicon rev id: %X\n", SYSCTL0->SILICONREV_ID);
	MEJ_LOG_MODULE_INFO(MAIN, "Silicon revision: %X.%X\n", silicon_rev_major, silicon_rev_minor);

	/* Enable Trace */
#if (ENABLE_SVIEW == 1)
	SEGGER_SYSVIEW_Conf();
	trace_platform_initialize();
#endif

	/* Create the MicroJvm task. */
	TaskHandle_t pvCreatedTask;
	BaseType_t ret = xTaskCreate(xJavaTaskFunction, "MEJ32 Core Engine", JAVA_TASK_STACK_SIZE, NULL, JAVA_TASK_PRIORITY, &pvCreatedTask);

#if (ENABLE_SVIEW == 1)
	SEGGER_SYSVIEW_setMicroJVMTask((U32)pvCreatedTask);
#endif

	vTaskStartScheduler();
}

/* Functions used in Core Validation test */
#include "sni.h"
jfloat Java_com_microej_core_tests_MicroejCoreValidation_testFloat (jfloat a, jfloat b) {return a * b;}
jdouble Java_com_microej_core_tests_MicroejCoreValidation_testDouble (jdouble a, jdouble b) {return a * b;}
