/**
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
package com.nxp.animatedMascot;

import ej.bon.Util;
import ej.microui.MicroUI;
import ej.microui.display.Colors;
import ej.microui.display.Display;
import ej.microui.display.GraphicsContext;
import ej.microui.display.Painter;
import ej.microvg.Matrix;
import ej.microvg.VectorGraphicsPainter;
import ej.microvg.VectorImage;
import ej.mwt.animation.Animation;
import ej.mwt.animation.Animator;

/**
 *
 */
public class AnimatedMascot {

	private long startTime;

	/**
	 * Simple main.
	 *
	 * @param args
	 *            command line arguments.
	 */
	public static void main(String[] args) {
		AnimatedMascot demo = new AnimatedMascot();
		demo.start();
	}

	private void start() {
		MicroUI.start();
		final Display display = Display.getDisplay();
		final GraphicsContext g = display.getGraphicsContext();

		final Matrix matrix = new Matrix();

		final VectorImage mascot = VectorImage.getImage("/images/mascot.xml"); //$NON-NLS-1$

		// Prepare matrix to scale the image to the display size
		matrix.setScale(display.getWidth() / mascot.getWidth(), display.getHeight() / mascot.getHeight());

		Animator animator = new Animator();
		this.startTime = Util.platformTimeMillis();
		animator.startAnimation(new Animation() {

			@Override
			public boolean tick(long currentTimeMillis) {
				// Clear screen between each frame
				g.setColor(Colors.BLACK);
				Painter.fillRectangle(g, 0, 0, display.getWidth(), display.getHeight());

				// Calculate current elapsed time
				int elapsed = (int) (currentTimeMillis - AnimatedMascot.this.startTime);

				// Draw the image
				VectorGraphicsPainter.drawAnimatedImage(g, mascot, matrix, elapsed);

				// Flush the display
				display.flush();

				// Update startTime at the end of animation to loop back to start
				if (mascot.getDuration() < (elapsed)) {
					AnimatedMascot.this.startTime = Util.platformTimeMillis();
				}
				return true;
			}
		});
	}
}
