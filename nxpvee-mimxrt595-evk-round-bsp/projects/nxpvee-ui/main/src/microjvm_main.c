/*
 * C
 *
 * Copyright 2014-2020 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

#include "fsl_debug_console.h"

#include "microjvm_main.h"
#include "LLMJVM.h"
#include "sni.h"


/*
 * Generic VM main function
 */
void microjvm_main(void) {
	void* vm;
	int32_t err;
	int32_t exitcode;

	// create VM
	vm = SNI_createVM();

	if (vm == NULL) {
		PRINTF("VM initialization error.\n");
	} else {
		PRINTF("VM START\n");
		err = SNI_startVM(vm, 0, NULL);

		if (err < 0) {
			// Error occurred
			if (err == LLMJVM_E_EVAL_LIMIT) {
				PRINTF("Evaluation limits reached.\n");
			} else {
				PRINTF("VM execution error (err = %d).\n", err);
			}
		} else {
			// VM execution ends normally
			exitcode = SNI_getExitCode(vm);
			PRINTF("VM END (exit code = %d)\n", exitcode);
		}

		// delete VM
		SNI_destroyVM(vm);
	}
}
