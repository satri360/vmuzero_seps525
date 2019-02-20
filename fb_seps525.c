/*
 * FB driver for the SEPS525 PM-OLED Controller
 *
 * Copyright (C) 2018 Shige Hayashi
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/delay.h>

#include "fbtft.h"

#define DRVNAME		"fb_seps525"
#define WIDTH		160
#define HEIGHT		128

static int init_display(struct fbtft_par *par)
{
	par->fbtftops.reset(par);

        write_reg(par, 0x04, 0x01); /* REDUCE_CURRENT */
        udelay(1000);
        write_reg(par, 0x04, 0x00); /* REDUCE_CURRENT */
        msleep(1);
        write_reg(par, 0x05, 0x00); /* SOFT_RST */
        write_reg(par, 0x06, 0x00); /* DISP_ON_OFF */
        write_reg(par, 0x02, 0x01); /* OSC_CTL */
        write_reg(par, 0x03, 0x30); /* CLOCK_DIV */
        write_reg(par, 0x28, 0x7f); /* DUTY */
        write_reg(par, 0x29, 0x00); /* DSL */
        write_reg(par, 0x14, 0x31); /* RGB_IF */
        write_reg(par, 0x15, 0x00); /* RGB_POL */
        write_reg(par, 0x13, 0x00); /* DISPLAY_MODE_SET */
        write_reg(par, 0x16, 0x66); /* MEMORY_WRITE_MODE */
        write_reg(par, 0x10, 0x45); /* DRIVING_CURRENT_R */
        write_reg(par, 0x11, 0x34); /* DRIVING_CURRENT_G */
        write_reg(par, 0x12, 0x23); /* DRIVING_CURRENT_B */
        write_reg(par, 0x08, 0x04); /* PRECHARGE_TIME_R */
        write_reg(par, 0x09, 0x05); /* PRECHARGE_TIME_G */
        write_reg(par, 0x0A, 0x05); /* PRECHARGE_TIME_B */
        write_reg(par, 0x0B, 0x9d); /* PRECHARGE_CURRENT_R */
        write_reg(par, 0x0C, 0x8c); /* PRECHARGE_CURRENT_G */
        write_reg(par, 0x0D, 0x57); /* PRECHARGE_CURRENT_B */
        write_reg(par, 0x80, 0x00); /* IREF */

        // Turn on the display
        write_reg(par, 0x06, 0x01); /* DISP_ON_OFF */
        msleep(100);

	return 0;
}

static void set_addr_win(struct fbtft_par *par, int xs, int ys, int xe, int ye)
{
	write_reg(par, 0x17, xs); /* MX1_ADDR */
	write_reg(par, 0x18, xe); /* MX2_ADDR */
	write_reg(par, 0x19, ys); /* MY1_ADDR */
	write_reg(par, 0x1a, ye); /* MY2_ADDR */
	write_reg(par, 0x20, 0x00); /* MEMORY_ACCESS_PTR_X */
	write_reg(par, 0x21, 0x00); /* MEMORY_ACCESS_PTR_Y */
	write_reg(par, 0x22); /* DDRAM_ACCESS_PORT */
}

static int set_var(struct fbtft_par *par)
{
	unsigned mode;

	if (par->fbtftops.init_display != init_display) {
		/* don't risk messing up register 13h */
		fbtft_par_dbg(DEBUG_INIT_DISPLAY, par,
			"%s: skipping since custom init_display() is used\n",
			__func__);
		return 0;
	}

	mode =  (par->bgr << 7); /* Set RGB Swap */

	switch (par->info->var.rotate) {
	case 0:
		write_reg(par, 0x13, mode );
		break;
	case 270:
		write_reg(par, 0x13, mode | 0x30);
		break;
	case 180:
		write_reg(par, 0x13, mode | 0x30);
		break;
	case 90:
		write_reg(par, 0x13, mode );
		break;
	}

	return 0;
}

static int blank(struct fbtft_par *par, bool on)
{
	fbtft_par_dbg(DEBUG_BLANK, par, "%s(blank=%s)\n",
		__func__, on ? "true" : "false");
	if (on)
		write_reg(par, 0x06, 0x01); /* Display On */
	else
		write_reg(par, 0x06, 0x00); /* Display Off */
	return 0;
}

static struct fbtft_display display = {
	.regwidth = 8,
	.width = WIDTH,
	.height = HEIGHT,
	.fbtftops = {
		.init_display = init_display,
		.set_addr_win = set_addr_win,
		.set_var = set_var,
		.blank = blank,
	},
};


FBTFT_REGISTER_DRIVER(DRVNAME, "syncoam,seps525", &display);

MODULE_ALIAS("spi:" DRVNAME);
MODULE_ALIAS("platform:" DRVNAME);
MODULE_ALIAS("spi:seps525");
MODULE_ALIAS("platform:seps525");

MODULE_DESCRIPTION("SEPS525 OLED Driver");
MODULE_AUTHOR("Shige Hayashi");
MODULE_LICENSE("GPL");
