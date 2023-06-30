/*
 * C
 *
 * Copyright 2019-2022 MicroEJ Corp. All rights reserved.
* Use of this source code is governed by a BSD-style license that can be found with this software.
 */

/**
 * @file fault_handlers.c
 *
 * @brief This file contains the fault exception handlers.
 *
 * @details The CMSIS names are used.
 *
 * There are two modes:
 * - verbose
 * - lite
 *
 * In verbose mode, each fault exception handler will:
 * - print the stack frame,
 * - print its name,
 * - print the associated fault status register,
 * - try to make an analysis of the fault cause,
 * - make an infinite loop.
 *
 * In lite mode, each fault exception handler will:
 * - print its name,
 * - make an infinite loop.
 *
 * Define the @ref VERBOSE_MODE macro to 1 to activate verbose mode.
 */

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include "MIMXRT595S_cm33.h"
#include "LLMJVM.h"

#include "fsl_debug_console.h"

#include "fault_handlers.h" // will check prototypes and import public types

#include "mej_log.h"

// -----------------------------------------------------------------------------
// Macros and Defines
// -----------------------------------------------------------------------------

/**
 * @brief Define this macro to if you are using Keil's ARMCC toolchain. If you are using IAR or GCC toolchains, do not define it.
 */
#define COMPILE_WITH_ARMCC

/**
 * @brief Define this macro to enable verbose handlers. Verbose handlers will consume  more memory than lite handlers.
 */
#define VERBOSE_MODE 1

/**
 * @brief Make an infinite while loop.
 */
#define INFINITE_LOOP()		{while(1);}

// CMSIS doesn't provide macro to get bits from BFSR (which is the 2nd byte in CFSR), so we create our own macros
#define BFSR_IBUSERR_Pos			(0)
#define BFSR_IBUSERR_Msk			(1 << BFSR_IBUSERR_Pos)

#define BFSR_PRECISERR_Pos			(1)
#define BFSR_PRECISERR_Msk			(1 << BFSR_PRECISERR_Pos)

#define BFSR_IMPRECISERR_Pos		(2)
#define BFSR_IMPRECISERR_Msk		(1 << BFSR_IMPRECISERR_Pos)

#define BFSR_BFARVALID_Pos			(15)
#define BFSR_BFARVALID_Msk			(1 << BFSR_BFARVALID_Pos)

// CMSIS doesn't provide macro to get bits from UFSR (which is the upper half-word in CFSR), so we create our own macros
#define UFSR_UNDEFINSTR_Pos			(0)
#define UFSR_UNDEFINSTR_Msk			(1 << UFSR_UNDEFINSTR_Pos)

#define UFSR_INVSTATE_Pos			(1)
#define UFSR_INVSTATE_Msk			(1 << UFSR_INVSTATE_Pos)

// -----------------------------------------------------------------------------
// Internal function definitions
// -----------------------------------------------------------------------------

#if VERBOSE_MODE == 1

/**
 * @brief C function to print the stacked registers (== the stack frame) along with EXEC_RETURN.
 *
 * @warning this function cannot be static because it won't be seen by assembly code during link edition.
 * You should not called this function directly.
 */
static void printer(context_state_frame_t *frame, uint32_t msp, uint32_t psp);

#endif // VERBOSE_MODE == 1

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

#if VERBOSE_MODE == 1

// See the header file for the function documentation
void handle_hard_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	uint32_t hfsr;
	printer(frame, msp, psp);

	hfsr = SCB->HFSR;
	MEJ_LOG_MODULE_ERROR(MAIN, "Hard Fault Status Register =\t%X\n", hfsr);

	if(hfsr & SCB_HFSR_FORCED_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "This hard fault is: FORCED\n");
	}

	LLMJVM_dump();
	LLMJVM_checkIntegrity();

	volatile int i = 42;

	while (i == 42);
}

// See the header file for the function documentation
void handle_memory_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	printer(frame, msp, psp);

	INFINITE_LOOP();
}

// See the header file for the function documentation
void handle_bus_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	uint32_t cfsr;
	uint8_t bfsr;
	uint32_t bfar;
	bool still_valid;
	printer(frame, msp, psp);

	cfsr = SCB->CFSR;
	MEJ_LOG_MODULE_ERROR(MAIN, "Configurable Fault Status Register =\t%.8X\n", cfsr);

	bfsr = (cfsr & SCB_CFSR_BUSFAULTSR_Msk) >> SCB_CFSR_BUSFAULTSR_Pos;
	MEJ_LOG_MODULE_ERROR(MAIN, "Bus Fault Status Register =\t%.2X\n", bfsr);

	if(bfsr & BFSR_IBUSERR_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "Instruction access error\n");
		// This case has not been experimented yet
	}

	if(bfsr & BFSR_PRECISERR_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "Precise data access error\n");

		// Faulting instruction
		// --> see the PC of the printed stack frame

		// Address of faulting data access
		bfar = SCB->BFAR;
		MEJ_LOG_MODULE_ERROR(MAIN, "Address of faulting data access (BFAR) = %.8X\n", bfar);

		still_valid = (cfsr & BFSR_BFARVALID_Msk);
		if(!still_valid) {
			MEJ_LOG_MODULE_WARNING(MAIN, "BFAR is no longer valid!\n");
		}
	}

	if(bfsr & BFSR_IMPRECISERR_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "Imprecise data access error\n");
	}

	INFINITE_LOOP();
}

// See the header file for the function documentation
void handle_usage_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	uint32_t cfsr;
	uint16_t ufsr;
	// TODO Activate usage for for division by 0 and unaligned access? (see page 384)

	printer(frame, msp, psp);

	cfsr = SCB->CFSR;
	MEJ_LOG_MODULE_ERROR(MAIN, "Configurable Fault Status Register =\t%X\n", cfsr);

	ufsr = (cfsr & SCB_CFSR_USGFAULTSR_Msk) >> SCB_CFSR_USGFAULTSR_Pos;
	MEJ_LOG_MODULE_ERROR(MAIN, "Usage Fault Status Register =\t%X\n", ufsr);

	if(ufsr & UFSR_UNDEFINSTR_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "Attempt to execute an undefined instruction\n");
	}

	if(ufsr & UFSR_INVSTATE_Msk) {
		MEJ_LOG_MODULE_ERROR(MAIN, "Attempt to switch to invalid mode (like ARM state)\n");
	}

	INFINITE_LOOP();
}

#else // VERBOSE_MODE == 1

// See the header file for the function documentation
void handle_hard_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	MEJ_LOG_MODULE_ERROR(MAIN, "Hard Fault\n");
	INFINITE_LOOP();
}


// See the header file for the function documentation
void handle_memory_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	MEJ_LOG_MODULE_ERROR(MAIN, "Memory Fault\n");
	INFINITE_LOOP();
}


// See the header file for the function documentation
void handle_bus_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	MEJ_LOG_MODULE_ERROR(MAIN, "Bus Fault\n");
	INFINITE_LOOP();
}


// See the header file for the function documentation
void handle_usage_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	MEJ_LOG_MODULE_ERROR(MAIN, "Usage Fault\n");
	INFINITE_LOOP();
}

#endif // VERBOSE_MODE == 1

// -----------------------------------------------------------------------------
// Internal functions
// -----------------------------------------------------------------------------

#if VERBOSE_MODE == 1

// See the section 'Internal function definitions' for the function documentation
static void printer(context_state_frame_t *frame, uint32_t msp, uint32_t psp) {
	MEJ_LOG_MODULE_INFO(MAIN, "---------------------------------------------------------------------\n");
	// Show stack pointers
	MEJ_LOG_MODULE_INFO(MAIN, "Current SP = 0x%08X\n", frame);
	MEJ_LOG_MODULE_INFO(MAIN, "MSP = 0x%08X\n", msp);
	MEJ_LOG_MODULE_INFO(MAIN, "PSP = 0x%08X\n", psp);
	MEJ_LOG_MODULE_INFO(MAIN, "");

	// Show stacked registers == stack frame (see section 8.1.3 and figure 12.4)
	MEJ_LOG_MODULE_INFO(MAIN, "Stack frame:\n");
	MEJ_LOG_MODULE_INFO(MAIN, "R0 =\t0x%08X\n", frame->r0);
	MEJ_LOG_MODULE_INFO(MAIN, "R1 =\t0x%08X\n", frame->r1);
	MEJ_LOG_MODULE_INFO(MAIN, "R2 =\t0x%08X\n", frame->r2);
	MEJ_LOG_MODULE_INFO(MAIN, "R3 =\t0x%08X\n", frame->r3);
	MEJ_LOG_MODULE_INFO(MAIN, "R12 =\t0x%08X\n", frame->r12);
	MEJ_LOG_MODULE_INFO(MAIN, "LR =\t0x%08X\n", frame->lr);
	MEJ_LOG_MODULE_INFO(MAIN, "\n");

	MEJ_LOG_MODULE_INFO(MAIN, "---------------------------------------------------------------------\n");
}

#endif // VERBOSE_MODE == 1

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

