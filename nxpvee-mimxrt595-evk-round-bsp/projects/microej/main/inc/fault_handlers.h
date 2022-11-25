/*
 * C
 *
 * Copyright 2019-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#ifndef FAULT_HANDLERS_H
#define FAULT_HANDLERS_H

// -----------------------------------------------------------------------------
// Types
// -----------------------------------------------------------------------------

typedef struct __attribute__((packed)) context_state_frame {
  uint32_t r0;
  uint32_t r1;
  uint32_t r2;
  uint32_t r3;
  uint32_t r12;
  uint32_t lr;
  uint32_t return_address;
  uint32_t xpsr;
} context_state_frame_t;

// -----------------------------------------------------------------------------
// Project functions
// -----------------------------------------------------------------------------

/*
 * @brief Hard Fault exception handler.
 */
void handle_hard_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp);

/*
 * @brief Memory Management Fault exception handler.
 */
void handle_memory_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp);

/*
 * @brief Bus Fault exception handler.
 */
void handle_bus_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp);

/*
 * @brief Usage Fault exception handler.
 */
void handle_usage_fault(context_state_frame_t *frame, uint32_t msp, uint32_t psp);

#endif // FAULT_HANDLERS_H

// -----------------------------------------------------------------------------
// EOF
// -----------------------------------------------------------------------------

