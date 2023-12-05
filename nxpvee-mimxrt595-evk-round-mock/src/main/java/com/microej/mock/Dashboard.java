/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.mock;

import com.microej.service.HeartRateService;
import com.microej.service.PowerService;

import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.collections.ObservableList;
import javafx.geometry.Insets;
import javafx.geometry.Orientation;
import javafx.geometry.Pos;
import javafx.scene.Node;
import javafx.scene.control.Label;
import javafx.scene.control.RadioButton;
import javafx.scene.control.Slider;
import javafx.scene.control.TextField;
import javafx.scene.control.TextFormatter;
import javafx.scene.control.TitledPane;
import javafx.scene.control.Toggle;
import javafx.scene.control.ToggleGroup;
import javafx.scene.image.Image;
import javafx.scene.image.ImageView;
import javafx.scene.layout.HBox;
import javafx.scene.layout.Pane;
import javafx.scene.layout.Region;
import javafx.scene.layout.VBox;
import javafx.util.converter.IntegerStringConverter;

/**
 * The mockup dashboard contains the widgets to control wearable natives.
 */
public class Dashboard {

	private static final String MICROEJ_LOGO_PATH = "/images/logo_microej_1500px.png";

	private static final String WATCHFACE_PATH = "/images/watchface.png";

	/**
	 * Creates a new instance of the dashboard.
	 *
	 * @param root
	 *            the dashboard widgets container
	 */
	public Dashboard(Pane root) {
		root.setPadding(new Insets(10, 10, 10, 10));

		final ObservableList<Node> children = root.getChildren();
		children.add(createBanner());
		children.add(createHeartRateWidget());
		children.add(createPowerLevel());
	}

	private Node createBanner() {
		final HBox banner = new HBox();
		banner.setPadding(new Insets(0, 0, 10, 0));
		banner.setAlignment(Pos.CENTER);

		final int spacerWidth = 45;
		final Region leftSpacer = new Region();
		leftSpacer.setMinWidth(spacerWidth);
		final Region rightSpacer = new Region();
		rightSpacer.setMinWidth(spacerWidth);

		final Image logoImage = new Image(Dashboard.class.getResourceAsStream(MICROEJ_LOGO_PATH), 150, 150, true, true);
		final ImageView logo = new ImageView(logoImage);

		banner.getChildren().add(leftSpacer);
		banner.getChildren().add(logo);
		banner.getChildren().add(rightSpacer);

		final Image watchfaceImage = new Image(Dashboard.class.getResourceAsStream(WATCHFACE_PATH), 100, 100, true,
				true);
		final ImageView watchface = new ImageView(watchfaceImage);

		banner.getChildren().add(watchface);

		return banner;
	}

	private Node createHeartRateWidget() {
		VBox heartBox = new VBox(6);
		heartBox.setAlignment(Pos.TOP_CENTER);

		// Mode Radio buttons
		Label modeLabel = new Label("Mode");
		final ToggleGroup modeToggleGroup = new ToggleGroup();
		final RadioButton modeZone = new RadioButton("By zone");
		modeZone.setToggleGroup(modeToggleGroup);
		final RadioButton modeValue = new RadioButton("By value");
		modeValue.setToggleGroup(modeToggleGroup);
		modeValue.setSelected(true);
		HBox modeBox = new HBox(6);
		modeBox.setAlignment(Pos.CENTER);
		modeBox.getChildren().addAll(modeLabel, modeZone, modeValue);

		// Zone range
		Label lowLabel = new Label("Low:");
		final TextField lowField = new TextField();
		lowField.setPrefColumnCount(5);
		lowField.setTextFormatter(new TextFormatter<>(new IntegerStringConverter()));
		lowField.setText(Integer.toString(HeartRateService.DEFAULT_ZONE_LOW_HR));
		Label highLabel = new Label("High:");
		final TextField highField = new TextField();
		highField.setPrefColumnCount(5);
		highField.setTextFormatter(new TextFormatter<>(new IntegerStringConverter()));
		highField.setText(Integer.toString(HeartRateService.DEFAULT_ZONE_HIGH_HR));
		HBox byZoneBox = new HBox(6);
		byZoneBox.getChildren().addAll(lowLabel, lowField, highLabel, highField);
		byZoneBox.setAlignment(Pos.CENTER);
		TitledPane byZoneGroup = new TitledPane("Zone", byZoneBox);
		byZoneGroup.setCollapsible(false);

		// Value slider
		final Slider valueSlider = new Slider(HeartRateService.MIN_HR_VALUE, HeartRateService.MAX_HR_VALUE,
				HeartRateService.DEFAULT_HR_VALUE);
		valueSlider.setOrientation(Orientation.HORIZONTAL);
		valueSlider.setShowTickMarks(true);
		valueSlider.setShowTickLabels(true);
		TitledPane byValueGroup = new TitledPane("Value", valueSlider);
		byValueGroup.setCollapsible(false);

		VBox inputBox = new VBox(6);
		inputBox.getChildren().addAll(byZoneGroup, byValueGroup);

		heartBox.getChildren().addAll(modeBox, inputBox);

		TitledPane pane = new TitledPane("Heart Rate", heartBox);
		pane.setCollapsible(false);

		// Add behavior

		// Enable heart rate generation when zone mode is selected
		modeToggleGroup.selectedToggleProperty().addListener(new ChangeListener<Toggle>() {
			@Override
			public void changed(ObservableValue<? extends Toggle> observable, Toggle oldValue, Toggle newValue) {
				if (newValue == modeZone) {
					HeartRateService.INSTANCE.startGeneratingHeartRateValues(lowField.textProperty(),
							highField.textProperty());
				} else {
					HeartRateService.INSTANCE.stopGeneratingHeartRateValues();
					HeartRateService.INSTANCE.setHeartRate((int) valueSlider.getValue());

				}
			}
		});

		// Update heart rate value when slider is changed and value mode is selected
		valueSlider.valueProperty().addListener(new ChangeListener<Number>() {
			@Override
			public void changed(ObservableValue<? extends Number> observable, Number oldValue, Number newValue) {
				if (modeToggleGroup.getSelectedToggle() == modeValue) {
					HeartRateService.INSTANCE.setHeartRate(newValue.intValue());
				}
			}
		});

		HeartRateService.INSTANCE.setHeartRate((int) valueSlider.getValue());

		return pane;
	}

	private Node createPowerLevel() {
		VBox powerLeverlBox = new VBox(6);
		powerLeverlBox.setAlignment(Pos.TOP_CENTER);

		// Mode Radio buttons
		Label modeLabel = new Label("Mode");
		final ToggleGroup modeToggleGroup = new ToggleGroup();
		final RadioButton modeCharge = new RadioButton("Charge");
		modeCharge.setToggleGroup(modeToggleGroup);
		final RadioButton modeDischarge = new RadioButton("Discharge");
		modeDischarge.setToggleGroup(modeToggleGroup);
		modeDischarge.setSelected(true);
		HBox modeBox = new HBox(6);
		modeBox.setAlignment(Pos.CENTER);
		modeBox.getChildren().addAll(modeLabel, modeCharge, modeDischarge);

		// Value slider
		final Slider valueSlider = new Slider(PowerService.MIN_PERCENTAGE, PowerService.MAX_PERCENTAGE,
				PowerService.DEFAULT_POWER_LEVEL);
		valueSlider.setOrientation(Orientation.HORIZONTAL);
		valueSlider.setShowTickMarks(true);
		valueSlider.setShowTickLabels(true);

		powerLeverlBox.getChildren().addAll(modeBox, valueSlider);

		TitledPane pane = new TitledPane("Power Level", powerLeverlBox);
		pane.setCollapsible(false);

		// Add behavior

		// Set charge or discharge mode
		modeToggleGroup.selectedToggleProperty().addListener(new ChangeListener<Toggle>() {
			@Override
			public void changed(ObservableValue<? extends Toggle> observable, Toggle oldValue, Toggle newValue) {
				PowerService.INSTANCE.setCharging(newValue == modeCharge);
			}
		});

		// Update power level when moving slider
		valueSlider.valueProperty().addListener(new ChangeListener<Number>() {
			@Override
			public void changed(ObservableValue<? extends Number> observable, Number oldValue, Number newValue) {
				PowerService.INSTANCE.setPowerLevel(newValue.intValue());
			}
		});

		// Update slider when charging / discharging
		PowerService.INSTANCE.getPowerProperty().addListener(new ChangeListener<Number>() {
			@Override
			public void changed(ObservableValue<? extends Number> observable, Number oldValue, Number newValue) {
				valueSlider.setValue(newValue.intValue());
			}
		});

		PowerService.INSTANCE.setPowerLevel((int) valueSlider.getValue());

		return pane;
	}
}
