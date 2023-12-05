/*
 * Java
 *
 * Copyright 2023 MicroEJ Corp. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be found with this software.
 */

package com.microej.service;

import java.util.Random;
import java.util.Timer;
import java.util.TimerTask;

import com.is2t.hil.HIL;
import com.is2t.hil.StopListener;

import javafx.beans.property.StringProperty;

/**
 * Represents the Heart Rate activity.
 */
public class HeartRateService {

	/** The {@link HeartRateService} singleton. */
	public static final HeartRateService INSTANCE = new HeartRateService();

	/** The minimum heart rate that can be set. */
	public static final int MIN_HR_VALUE = 0;

	/** The maximum heart rate that can be set. */
	public static final int MAX_HR_VALUE = 240;

	/** The default heart rate. */
	public static final int DEFAULT_HR_VALUE = 120;

	/** The default low heart rate zone. */
	public static final int DEFAULT_ZONE_LOW_HR = 100;

	/** The default high heart rate zone. */
	public static final int DEFAULT_ZONE_HIGH_HR = 120;

	private static final long HR_GENERATION_PERIOD = 500;

	private final Timer timer;

	private TimerTask hrGenerationTask;

	private int heartRate;

	private HeartRateService() {
		this.timer = new Timer();
		HIL.getInstance().addStopListener(new StopListener() {
			@Override
			public void stop(boolean isEvalVersion) {
				HeartRateService.this.timer.cancel();
			}
		});
	}

	/**
	 * Gets the current heart rate.
	 *
	 * @return the heart rate
	 */
	public int getHeartRate() {
		return this.heartRate;
	}

	/**
	 * Sets the current heart rate.
	 *
	 * @param heartRate
	 *            the heart rate to set
	 */
	public void setHeartRate(int heartRate) {
		this.heartRate = heartRate;
	}

	/**
	 * Starts random heart rate between low and high limits.
	 *
	 * @param zoneLowLimit
	 *            the minimum random heart rate
	 * @param zoneHighLimit
	 *            the maximum random heart rate
	 */
	public void startGeneratingHeartRateValues(final StringProperty zoneLowLimit, final StringProperty zoneHighLimit) {

		this.hrGenerationTask = new TimerTask() {
			@Override
			public void run() {
				int heartRate = generateHeartRateValue(zoneLowLimit, zoneHighLimit);
				setHeartRate(heartRate);
			}
		};

		this.timer.scheduleAtFixedRate(this.hrGenerationTask, 0, HR_GENERATION_PERIOD);
	}

	/**
	 * Stops the random heart rate.
	 */
	public void stopGeneratingHeartRateValues() {
		TimerTask hrGenerationTask = this.hrGenerationTask;
		if (hrGenerationTask != null) {
			hrGenerationTask.cancel();
			this.hrGenerationTask = null;
		}
	}

	private int generateHeartRateValue(StringProperty zoneLowLimitString, StringProperty zoneHighLimitString) {
		final int zoneLowLimit = getIntValue(zoneLowLimitString, DEFAULT_ZONE_LOW_HR);
		final int zoneHighLimit = getIntValue(zoneHighLimitString, DEFAULT_ZONE_HIGH_HR);

		if (zoneLowLimit > zoneHighLimit) {
			return 0;
		}

		final int amplitude = zoneHighLimit - zoneLowLimit;

		return new Random().nextInt(amplitude) + zoneLowLimit;
	}

	/**
	 * Converts the given {@link StringProperty} into an int. Returns the defaultValue if an error occurs.
	 */
	private int getIntValue(StringProperty property, int defaultValue) {
		try {
			return Integer.decode(property.getValue());
		} catch (Throwable e) {
			return defaultValue;
		}
	}

}
