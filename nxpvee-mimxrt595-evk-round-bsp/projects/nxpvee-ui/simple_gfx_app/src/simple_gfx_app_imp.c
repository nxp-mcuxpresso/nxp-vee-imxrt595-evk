/*
 * Copyright 2022 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <math.h>

#define AMP_ORANGE 30
#define AMP_BLUE   75
#define AMP_GREEN  30

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1orange_1x(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	double sine_orange = AMP_ORANGE * cos(t * M_PI / 45.0);
	return centre - reclen - reclen / 2 - AMP_ORANGE - (int)sine_orange;
}

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1orange_1y(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	return centre - recheight / 2;
}

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1blue_1x(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	return centre - reclen / 2;
}

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1blue_1y(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	double sine_blue = AMP_BLUE * sin(t * M_PI / 45.0);
	return centre - recheight / 2 + (int)sine_blue;
}

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1green_1x(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	double sine_green = AMP_GREEN * cos(t * M_PI / 45.0);
	return centre + reclen - reclen / 2 + AMP_GREEN + (int)sine_green;
}

int Java_com_nxp_simpleGFX_SimpleGFXNatives_get_1green_1y(int t, int reclen, int recheight, int display_size)
{
	int centre = display_size / 2;
	return centre - recheight / 2;
}
