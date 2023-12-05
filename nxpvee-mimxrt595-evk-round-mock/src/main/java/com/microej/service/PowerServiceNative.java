/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.service;

import com.microej.mock.MockupLauncher;

/**
 * The {@link PowerService} natives implementation.
 */
public class PowerServiceNative {

	/**
	 * Returns the current power level to the application.
	 *
	 * @return the power level in percent
	 */
	public static int getPowerLevel() {
		MockupLauncher.startup();
		return PowerService.INSTANCE.getPowerLevel();
	}

	/**
	 * Returns the charge mode to the application.
	 *
	 * @return true is the battery is charging, false otherwise
	 */
	public static boolean isCharging() {
		MockupLauncher.startup();
		return PowerService.INSTANCE.isCharging();
	}
}
