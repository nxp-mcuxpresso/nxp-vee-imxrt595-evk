/**
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
package com.nxp.simpleGFX;

import ej.microui.MicroUI;
import ej.microui.display.Display;
import ej.microui.display.GraphicsContext;
import ej.microui.display.Painter;

/**
 * Main class
 */
public class SimpleGFX extends Thread {

	final static int NXP_ORANGE = 0xF9B500;
	final static int NXP_BLUE = 0x7BB1DB;
	final static int NXP_GREEN = 0xC9D200;
	final static int DISPLAY_SIZE = 390;

	private static int grey = 0xFFFFFF;

	private static boolean decrease_grey = true;

	/**
	 * Simple main.
	 *
	 * @param args
	 *            command line arguments.
	 */
	public static void main(String[] args) {
		MicroUI.start();
		Display display = Display.getDisplay();
		GraphicsContext g = display.getGraphicsContext();

		if (display.isDoubleBuffered()) {
			System.out.println("Display is double buffered\r\n"); //$NON-NLS-1$
		} else {
			System.out.println("Display *not* double buffered\r\n"); //$NON-NLS-1$
		}

		int t = 0;
		while (true) {
			draw_all(g, t);
			display.flush();
			try {
				Thread.sleep(16);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			t++;
		}
	}

	private static void draw_rect(GraphicsContext g, int x, int y, int dx, int dy, int colour) {
		g.setColor(colour);
		Painter.fillRectangle(g, x, y, dx, dy);
	}

	private static void draw_all(GraphicsContext g, int t) {
		int reclen = 100, recheight = 75;

		g.setColor(grey);
		if (decrease_grey) {
			grey -= 0x010101;
			if (grey == 0x808080) {
				decrease_grey = false;
			}
		} else {
			grey += 0x010101;
			if (grey == 0xffffff) {
				decrease_grey = true;
			}
		}
		Painter.fillCircle(g, 0, 0, 390);
		// int centre = this.DISPLAY_SIZE / 2;
		// double sine_orange = 30.0 * Math.cos(t * Math.PI / 45.0);
		// double sine_blue = 75.0 * Math.sin(t * Math.PI / 45.0);
		// draw_rect(centre - reclen - reclen / 2 - 30 - (int) sine_orange, centre - recheight / 2, reclen, recheight,
		// this.NXP_ORANGE);
		draw_rect(g, SimpleGFXNatives.get_orange_x(t, reclen, recheight, DISPLAY_SIZE),
				SimpleGFXNatives.get_orange_y(t, reclen, recheight, DISPLAY_SIZE), reclen, recheight, NXP_ORANGE);
		// draw_rect(centre - reclen / 2, centre - recheight / 2 + (int) sine_blue, reclen, recheight, this.NXP_BLUE);
		draw_rect(g, SimpleGFXNatives.get_blue_x(t, reclen, recheight, DISPLAY_SIZE),
				SimpleGFXNatives.get_blue_y(t, reclen, recheight, DISPLAY_SIZE), reclen, recheight, NXP_BLUE);
		// draw_rect(centre + reclen - reclen / 2 + 30 + (int) sine_orange, centre - recheight / 2, reclen, recheight,
		// this.NXP_GREEN);
		draw_rect(g, SimpleGFXNatives.get_green_x(t, reclen, recheight, DISPLAY_SIZE),
				SimpleGFXNatives.get_green_y(t, reclen, recheight, DISPLAY_SIZE), reclen, recheight, NXP_GREEN);
	}
}
