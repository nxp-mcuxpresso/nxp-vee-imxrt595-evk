/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

package com.nxp.simpleGFX;

/**
 *
 */
public class SimpleGFXNatives {

	private static final int AMP_ORANGE = 30;
	private static final int AMP_BLUE = 75;
	private static final int AMP_GREEN = 30;

	public static int get_orange_x(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		double sine_orange = AMP_ORANGE * Math.cos(t * Math.PI / 45.0);
		return centre - reclen - reclen / 2 - AMP_ORANGE - (int) sine_orange;
	}

	public static int get_orange_y(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		return centre - recheight / 2;
	}

	public static int get_blue_x(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		return centre - reclen / 2;
	}

	public static int get_blue_y(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		double sine_blue = AMP_BLUE * Math.sin(t * Math.PI / 45.0);
		return centre - recheight / 2 + (int) sine_blue;
	}

	public static int get_green_x(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		double sine_green = AMP_GREEN * Math.cos(t * Math.PI / 45.0);
		return centre + reclen - reclen / 2 + AMP_GREEN + (int) sine_green;
	}

	public static int get_green_y(int t, int reclen, int recheight, int display_size) {
		int centre = display_size / 2;
		return centre - recheight / 2;
	}
}
