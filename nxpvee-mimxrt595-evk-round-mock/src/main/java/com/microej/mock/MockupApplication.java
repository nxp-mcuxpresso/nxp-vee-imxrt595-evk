/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.mock;

import javafx.application.Application;
import javafx.scene.Scene;
import javafx.scene.layout.Pane;
import javafx.scene.layout.VBox;
import javafx.stage.Stage;

/**
 * The main application.
 */
public class MockupApplication extends Application {

	private final Pane root;

	private final Dashboard dashboard;

	/**
	 * Creates a new instance of the application.
	 * <p>
	 * Called by JavaFX.
	 */
	public MockupApplication() {
		this.root = new VBox();
		this.dashboard = new Dashboard(this.root);
	}

	@Override
	public void start(Stage primaryStage) throws Exception {
		primaryStage.setTitle("Smartwatch VD Control Center");

		primaryStage.setScene(new Scene(this.root));
		primaryStage.sizeToScene();
		primaryStage.setResizable(false);
		primaryStage.show();
	}

}
