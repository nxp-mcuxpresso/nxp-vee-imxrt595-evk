/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.service;

import com.microej.mock.MockupLauncher;

/**
 * The {@link HeartRateService} natives implementation.
 */
public class HeartRateServiceNative {

	/**
	 * Returns the current heart rate to the application.
	 *
	 * @return the current heart rate
	 */
	public static int getHeartRate() {
		MockupLauncher.startup();
		return HeartRateService.INSTANCE.getHeartRate();
	}
}
