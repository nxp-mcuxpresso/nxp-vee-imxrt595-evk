;
; ASM
;
; Copyright 2020 MicroEJ Corp. All rights reserved.
; Use of this source code is governed by a BSD-style license that can be found with this software.
;

	EXTERN  handle_hard_fault
	EXTERN  handle_memory_fault
	EXTERN  handle_bus_fault
	EXTERN  handle_usage_fault

THUMB

;
; @brief Generic assembler routine to handle ARM CM33 faults
;
; @param[in] R0: C handler to call
;
	SECTION .text:CODE:REORDER:NOROOT(1)
handle_generic_fault

		; move stack registers and link register to R0, R1, R2 and R3
		TST		LR, #4
		ITE		EQ
		MRSEQ	R0, MSP
		MRSNE	R0, PSP
		MRS		R1, MSP
		MRS		R2, PSP

		; call C handler
		BX		R3

;
; @brief Hard Fault handler
;
	PUBLIC HardFault_Handler
	SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler

		LDR		R3, =handle_hard_fault
		B		handle_generic_fault

;
; @brief Memory Fault handler
;
	PUBLIC MemFault_Handler
	SECTION .text:CODE:REORDER:NOROOT(1)
MemFault_Handler

		LDR		R3, =handle_memory_fault
		B		handle_generic_fault

;
; @brief Bus Fault handler
;
	PUBLIC BusFault_Handler
	SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler

		LDR		R3, =handle_bus_fault
		B		handle_generic_fault

;
; @brief Usage Fault handler
;
	PUBLIC UsageFault_Handler
	SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler

		LDR		R3, =handle_usage_fault
		B		handle_generic_fault

	END

; -----------------------------------------------------------------------------
; EOF
; -----------------------------------------------------------------------------

