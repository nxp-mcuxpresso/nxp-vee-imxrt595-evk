/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.mock;

import java.util.concurrent.Executors;

import javafx.application.Application;

/**
 * Launch the Mockup Application that mocks HeartRate and Power services.
 */
public class MockupLauncher {

	public static boolean isStarted = false;

	public static void startup() {
		if (!isStarted) {
			Executors.newSingleThreadExecutor().execute(new Runnable() {
				@Override
				public void run() {
					Application.launch(MockupApplication.class);
				}
			});
			isStarted = true;
		}
	}
}
