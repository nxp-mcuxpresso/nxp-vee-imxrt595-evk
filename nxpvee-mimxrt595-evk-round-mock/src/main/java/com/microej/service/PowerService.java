/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.service;

import java.util.Timer;
import java.util.TimerTask;

import com.is2t.hil.HIL;
import com.is2t.hil.StopListener;

import javafx.beans.property.IntegerProperty;
import javafx.beans.property.SimpleIntegerProperty;

/**
 * Represents the power service.
 */
public class PowerService {

	/** The {@link PowerService} singleton. */
	public static final PowerService INSTANCE = new PowerService();

	/** The minimum power level. */
	public static final int MIN_PERCENTAGE = 0;

	/** The maximum power level. */
	public static final int MAX_PERCENTAGE = 100;

	/** The default power level. */
	public static final int DEFAULT_POWER_LEVEL = 80;

	private static final long CHARGING_PERIOD = 5000;

	private static final int CHARGING_STEP = 2;

	private static final int DISCHARGING_STEP = -1;

	private final IntegerProperty powerLevel;

	private boolean charging;

	private final Timer timer;

	private PowerService() {
		// This is a singleton, direct instantiation is forbidden.

		this.powerLevel = new SimpleIntegerProperty();

		this.timer = new Timer();
		HIL.getInstance().addStopListener(new StopListener() {
			@Override
			public void stop(boolean isEvalVersion) {
				PowerService.this.timer.cancel();
			}
		});

		this.timer.scheduleAtFixedRate(new TimerTask() {
			@Override
			public void run() {
				updateCharge();
			}
		}, 0, CHARGING_PERIOD);
	}

	private void updateCharge() {
		final int step = this.isCharging() ? CHARGING_STEP : DISCHARGING_STEP;
		setPowerLevel(step + getPowerLevel());
	}

	/**
	 * Gets the power level property.
	 * <p>
	 * This property can be used to listen changes on the power level values.
	 *
	 * @return an {@link IntegerProperty} representing the power level
	 */
	public IntegerProperty getPowerProperty() {
		return this.powerLevel;
	}

	/**
	 * Gets the power level.
	 *
	 * @return the current power level in percent
	 */
	public int getPowerLevel() {
		return this.powerLevel.get();
	}

	/**
	 * Sets the current power level.
	 * <p>
	 * A negative value will set level to 0, whereas a value greater than 100 will set the power level to 100.
	 *
	 * @param powerLevel
	 *            the power level to set in percent.
	 */
	public void setPowerLevel(int powerLevel) {
		if (powerLevel < MIN_PERCENTAGE) {
			this.powerLevel.set(MIN_PERCENTAGE);
		} else if (powerLevel > MAX_PERCENTAGE) {
			this.powerLevel.set(MAX_PERCENTAGE);
		} else {
			this.powerLevel.set(powerLevel);
		}

	}

	/**
	 * Tells whether the battery is charging.
	 *
	 * @return true if the battery is charging, false otherwise
	 */
	public boolean isCharging() {
		return this.charging;
	}

	/**
	 * Sets the charge mode.
	 *
	 * @param charging
	 *            a flag meaning that the battery is in charge mode if true
	 */
	public void setCharging(boolean charging) {
		this.charging = charging;
	}
}
