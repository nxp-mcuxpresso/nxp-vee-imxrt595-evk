/**
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
package com.nxp.simpleGFX;

/**
 *
 */
public class SimpleGFXNatives {

	/* package */ native static int multiply_by_two(int a);

	/* package */ native static int return_x(int display_size);

	/* package */ native static int return_y(int display_size);

	/* package */ native static int get_orange_x(int t, int reclen, int recheight, int display_size);

	/* package */ native static int get_orange_y(int t, int reclen, int recheight, int display_size);

	/* package */ native static int get_blue_x(int t, int reclen, int recheight, int display_size);

	/* package */ native static int get_blue_y(int t, int reclen, int recheight, int display_size);

	/* package */ native static int get_green_x(int t, int reclen, int recheight, int display_size);

	/* package */ native static int get_green_y(int t, int reclen, int recheight, int display_size);

}
