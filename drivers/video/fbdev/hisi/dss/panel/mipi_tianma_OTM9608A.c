/* Copyright (c) 2008-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/

#include "hisi_fb.h"


#define DTS_COMP_MIPI_TIANMA_OTM9608A	"hisilicon,mipi_tianma_OTM9608A"

static int g_lcd_fpga_flag = 0;


/********************************tianma_OTM9608A*****start***********************
**Power ON Sequence(sleep mode to Normal mode)
*/

// Enable cmd
static char tianma_OTM9608A_power_on_param1[] = {
	0xff,
	0x96, 0x08, 0x01,
};

static char tianma_OTM9608A_power_on_param2[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param3[] = {
	0xff,
	0x96, 0x08,
};

static char tianma_OTM9608A_power_on_param4[] = {
	0x00,
	0x00,
};

static char tianma_OTM9608A_power_on_param5[] = {
	0xa0,
	0x00,
};

static char tianma_OTM9608A_power_on_param6[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param7[] = {
	0xb3,
	0x00, 0x00, 0x20, 0x00,
	0x00,
};

static char tianma_OTM9608A_power_on_param8[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param9[] = {
	0xb3,
	0x09,
};

static char tianma_OTM9608A_power_on_param10[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param11[] = {
	0xc0,
	0x00, 0x48, 0x00, 0x0f,
	0x11, 0x00, 0x48, 0x0f,
	0x11,
};

static char tianma_OTM9608A_power_on_param12[] = {
	0x00,
	0x92,
};

static char tianma_OTM9608A_power_on_param13[] = {
	0xc0,
	0x00, 0x10, 0x00, 0x13,
};

static char tianma_OTM9608A_power_on_param14[] = {
	0x00,
	0xa2,
};

static char tianma_OTM9608A_power_on_param15[] = {
	0xc0,
	0x0c, 0x05, 0x02,
};

static char tianma_OTM9608A_power_on_param16[] = {
	0x00,
	0xb3,
};

static char tianma_OTM9608A_power_on_param17[] = {
	0xc0,
	0x00, 0x50,
};

static char tianma_OTM9608A_power_on_param18[] = {
	0x00,
	0x81,
};

//refresh rate 0x55->60Hz
static char tianma_OTM9608A_power_on_param19[] = {
	0xc1,
	0x55,
};

static char tianma_OTM9608A_power_on_param20[] = {
	0x00,
	0x80,
};

//0xc480 = 0x30 source output GND during porch and non-display area
static char tianma_OTM9608A_power_on_param21[] = {
	0xc4,
	0x30, 0x84,
};

static char tianma_OTM9608A_power_on_param22[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param23[] = {
	0xc4,
	0x33, 0x09, 0x90, 0x2b,
	0x33, 0x09, 0x90, 0x54,
};

static char tianma_OTM9608A_power_on_param24[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param25[] = {
	0xc5,
	0x08, 0x00, 0xa0, 0x11,
};

static char tianma_OTM9608A_power_on_param26[] = {
	0x00,
	0x90,
};

static char tianma_OTM9608A_power_on_param27[] = {
	0xc5,
	0x96, 0x19, 0x01, 0x79,
	0x33, 0x33, 0x34,
};

static char tianma_OTM9608A_power_on_param28[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param29[] = {
	0xc5,
	0x96, 0x16, 0x00, 0x79,
	0x33, 0x33, 0x34,
};

static char tianma_OTM9608A_power_on_param30[] = {
	0x00,
	0x00,
};
static char tianma_OTM9608A_power_on_param31[] = {
	0xd8,
	0x5f, 0x5f,
};

//gamma 2.5 parameter
static char tianma_OTM9608A_power_on_param32[] = {
	0x00,
	0x00,
};

static char tianma_OTM9608A_power_on_param33[] = {
	0xe1,
	0x01, 0x0e, 0x15, 0x0E,
	0x07, 0x13, 0x0C, 0x0B,
	0x03, 0x06, 0x09, 0x07,
	0x0C, 0x0D, 0x08, 0x01,
};

//GOA ECLK Setting and GOA Other Options1 and GOA Signal Toggle Option Setting
static char tianma_OTM9608A_power_on_param34[] = {
	0x00,
	0x00,
};

static char tianma_OTM9608A_power_on_param35[] = {
	0xe2,
	0x01, 0x0e, 0x15, 0x0E,
	0x07, 0x13, 0x0C, 0x0B,
	0x03, 0x06, 0x09, 0x07,
	0x0C, 0x0D, 0x08, 0x01,
};

static char tianma_OTM9608A_power_on_param36[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param37[] = {
	0xc5,
	0x04, 0xa8,
};

static char tianma_OTM9608A_power_on_param38[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param39[] = {
	0xc6,
	0x64,
};

static char tianma_OTM9608A_power_on_param40[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param41[] = {
	0xc6,
	0x03, 0x10, 0x00, 0x1f,
	0x12,
};

static char tianma_OTM9608A_power_on_param42[] = {
	0x00,
	0xb7,
};

static char tianma_OTM9608A_power_on_param43[] = {
	0xb0,
	0x10,
};

static char tianma_OTM9608A_power_on_param44[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param45[] = {
	0xb0,
	0x55,
};

static char tianma_OTM9608A_power_on_param46[] = {
	0x00,
	0xb1,
};

static char tianma_OTM9608A_power_on_param47[] = {
	0xb0,
	0x03,
};

//increase mipi clk width , or display overturn probability
static char tianma_OTM9608A_power_on_param48[] = {
	0x00,
	0xb2,
};

static char tianma_OTM9608A_power_on_param49[] = {
	0xb0,
	0x06,
};

static char tianma_OTM9608A_power_on_param50[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param51[] = {
	0xcb,
	0x05, 0x00, 0x00, 0x00, 0x00,
	0x05, 0x00, 0x00, 0x00, 0x00,
};

static char tianma_OTM9608A_power_on_param52[] = {
	0x00,
	0x90,
};

static char tianma_OTM9608A_power_on_param53[] = {
	0xcb,
	0x55,0x55,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param54[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param55[] = {
	0xcb,
	0x00,0x00,0x00,0x00,0x00,
	0x55,0x55,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param56[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param57[] = {
	0xcb,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param58[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param59[] = {
	0xcb,
	0x55,0x55,0x00,0x00,0x00,
	0x04,0x00,0x04,0x00,0x04,
	0x00,0x04,0x04,0x04,0x00,
};

static char tianma_OTM9608A_power_on_param60[] = {
	0x00,
	0xd0,
};

static char tianma_OTM9608A_power_on_param61[] = {
	0xcb,
	0x04,0x00,0x00,0x00,0x00,
	0x55,0x55,0x00,0x00,0x00,
	0x04,0x00,0x04,0x00,0x04,
};

static char tianma_OTM9608A_power_on_param62[] = {
	0x00,
	0xe0,
};

static char tianma_OTM9608A_power_on_param63[] = {
	0xcb,
	0x00,0x04,0x04,0x04,0x00,
	0x04,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param64[] = {
	0x00,
	0xf0,
};

static char tianma_OTM9608A_power_on_param65[] = {
	0xcb,
	0x0f,0x00,0xcc,0x00,0x00,
	0x0f,0x00,0xcc,0xc3,0x00,
};

static char tianma_OTM9608A_power_on_param66[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param67[] = {
	0xcc,
	0x25,0x26,0x00,0x00,0x00,
	0x0c,0x00,0x0a,0x00,0x10,
};

static char tianma_OTM9608A_power_on_param68[] = {
	0x00,
	0x90,
};

static char tianma_OTM9608A_power_on_param69[] = {
	0xcc,
	0x00,0x0e,0x02,0x04,0x00,
	0x06,0x00,0x00,0x00,0x00,
	0x25,0x26,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param70[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param71[] = {
	0xcc,
	0x0b,0x00,0x09,0x00,0x0f,
	0x00,0x0d,0x01,0x03,0x00,
	0x05,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param72[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param73[] = {
	0xcc,
	0x26,0x25,0x00,0x00,0x00,
	0x0d,0x00,0x0f,0x00,0x09,
};

static char tianma_OTM9608A_power_on_param74[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param75[] = {
	0xcc,
	0x00,0x0b,0x03,0x01,0x00,
	0x05,0x00,0x00,0x00,0x00,
	0x26,0x25,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param76[] = {
	0x00,
	0xd0,
};

static char tianma_OTM9608A_power_on_param77[] = {
	0xcc,
	0x0e,0x00,0x10,0x00,0x0a,
	0x00,0x0c,0x04,0x02,0x00,
	0x06,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param78[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param79[] = {
	0xce,
	0x8a,0x03,0x28,0x89,
	0x03,0x28,0x88,0x03,
	0x28,0x87,0x03,0x28,
};

static char tianma_OTM9608A_power_on_param80[] = {
	0x00,
	0x90,
};

static char tianma_OTM9608A_power_on_param81[] = {
	0xce,
	0x38,0x0f,0x28,0x38,0x0e,
	0x28,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param82[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param83[] = {
	0xce,
	0x38,0x06,0x03,0xc1,0x00,
	0x28,0x00,0x38,0x05,0x03,
	0xc2,0x00,0x28,0x00,
};

static char tianma_OTM9608A_power_on_param84[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param85[] = {
	0xce,
	0x38,0x04,0x03,0xc3,0x00,
	0x28,0x00,0x38,0x03,0x03,
	0xc4,0x00,0x28,0x00,
};

static char tianma_OTM9608A_power_on_param86[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param87[] = {
	0xce,
	0x38,0x02,0x03,0xc5,0x00,
	0x28,0x00,0x38,0x01,0x03,
	0xc6,0x00,0x28,0x00,
};

static char tianma_OTM9608A_power_on_param88[] = {
	0x00,
	0xd0,
};

static char tianma_OTM9608A_power_on_param89[] = {
	0xce,
	0x38,0x00,0x03,0xc7,0x00,
	0x28,0x00,0x30,0x00,0x03,
	0xc8,0x00,0x28,0x00,
};

static char tianma_OTM9608A_power_on_param90[] = {
	0x00,
	0x80,
};

static char tianma_OTM9608A_power_on_param91[] = {
	0xcf,
	0xf0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xf0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param92[] = {
	0x00,
	0x90,
};

static char tianma_OTM9608A_power_on_param93[] = {
	0xcf,
	0xf0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xf0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param94[] = {
	0x00,
	0xa0,
};

static char tianma_OTM9608A_power_on_param95[] = {
	0xcf,
	0xf0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xf0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param96[] = {
	0x00,
	0xb0,
};

static char tianma_OTM9608A_power_on_param97[] = {
	0xcf,
	0xf0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xf0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param98[] = {
	0x00,
	0xc0,
};

static char tianma_OTM9608A_power_on_param99[] = {
	0xcf,
	0x01,0x01,0x20,0x20,0x00,
	0x00,0x02,0x01,0x00,0x00,
};

static char tianma_OTM9608A_power_on_param100[] = {
	0x35,
	0x00,
};

static char tianma_OTM9608A_power_on_param101[] = {
	0x53,
	0x24,
};

static char tianma_OTM9608A_power_on_param102[] = {
	0x55,
	0x00,
};

// exit sleep mode
static char tianma_OTM9608A_exit_sleep[] = {
	0x11,
};

// set display on
static char tianma_OTM9608A_display_on[] = {
	0x29,
};

/*
**Power OFF Sequence(Normal to power off)
*/
static char tianma_OTM9608A_display_off[] = {
	0x28,
};

static char tianma_OTM9608A_enter_sleep[] = {
	0x10,
};

static struct dsi_cmd_desc tianma_OTM9608A_display_on_cmds[] = {
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param1), tianma_OTM9608A_power_on_param1},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param2), tianma_OTM9608A_power_on_param2},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param3), tianma_OTM9608A_power_on_param3},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param4), tianma_OTM9608A_power_on_param4},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param5), tianma_OTM9608A_power_on_param5},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param6), tianma_OTM9608A_power_on_param6},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param7), tianma_OTM9608A_power_on_param7},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param8), tianma_OTM9608A_power_on_param8},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param9), tianma_OTM9608A_power_on_param9},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param10), tianma_OTM9608A_power_on_param10},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param11), tianma_OTM9608A_power_on_param11},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param12), tianma_OTM9608A_power_on_param12},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param13), tianma_OTM9608A_power_on_param13},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param14), tianma_OTM9608A_power_on_param14},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param15), tianma_OTM9608A_power_on_param15},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param16), tianma_OTM9608A_power_on_param16},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param17), tianma_OTM9608A_power_on_param17},

	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param18), tianma_OTM9608A_power_on_param18},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param19), tianma_OTM9608A_power_on_param19},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param20), tianma_OTM9608A_power_on_param20},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param21), tianma_OTM9608A_power_on_param21},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param22), tianma_OTM9608A_power_on_param22},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param23), tianma_OTM9608A_power_on_param23},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param24), tianma_OTM9608A_power_on_param24},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param25), tianma_OTM9608A_power_on_param25},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param26), tianma_OTM9608A_power_on_param26},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param27), tianma_OTM9608A_power_on_param27},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param28), tianma_OTM9608A_power_on_param28},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param29), tianma_OTM9608A_power_on_param29},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param30), tianma_OTM9608A_power_on_param30},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param31), tianma_OTM9608A_power_on_param31},

	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param32), tianma_OTM9608A_power_on_param32},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param33), tianma_OTM9608A_power_on_param33},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param34), tianma_OTM9608A_power_on_param34},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param35), tianma_OTM9608A_power_on_param35},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param36), tianma_OTM9608A_power_on_param36},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param37), tianma_OTM9608A_power_on_param37},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param38), tianma_OTM9608A_power_on_param38},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param39), tianma_OTM9608A_power_on_param39},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param40), tianma_OTM9608A_power_on_param40},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param41), tianma_OTM9608A_power_on_param41},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param42), tianma_OTM9608A_power_on_param42},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param43), tianma_OTM9608A_power_on_param43},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param44), tianma_OTM9608A_power_on_param44},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param45), tianma_OTM9608A_power_on_param45},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param46), tianma_OTM9608A_power_on_param46},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param47), tianma_OTM9608A_power_on_param47},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param48), tianma_OTM9608A_power_on_param48},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param49), tianma_OTM9608A_power_on_param49},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param50), tianma_OTM9608A_power_on_param50},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param51), tianma_OTM9608A_power_on_param51},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param52), tianma_OTM9608A_power_on_param52},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param53), tianma_OTM9608A_power_on_param53},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param54), tianma_OTM9608A_power_on_param54},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param55), tianma_OTM9608A_power_on_param55},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param56), tianma_OTM9608A_power_on_param56},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param57), tianma_OTM9608A_power_on_param57},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param58), tianma_OTM9608A_power_on_param58},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param59), tianma_OTM9608A_power_on_param59},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param60), tianma_OTM9608A_power_on_param60},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param61), tianma_OTM9608A_power_on_param61},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param62), tianma_OTM9608A_power_on_param62},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param63), tianma_OTM9608A_power_on_param63},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param64), tianma_OTM9608A_power_on_param64},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param65), tianma_OTM9608A_power_on_param65},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param66), tianma_OTM9608A_power_on_param66},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param67), tianma_OTM9608A_power_on_param67},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param68), tianma_OTM9608A_power_on_param68},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param69), tianma_OTM9608A_power_on_param69},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param70), tianma_OTM9608A_power_on_param70},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param71), tianma_OTM9608A_power_on_param71},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param72), tianma_OTM9608A_power_on_param72},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param73), tianma_OTM9608A_power_on_param73},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param74), tianma_OTM9608A_power_on_param74},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param75), tianma_OTM9608A_power_on_param75},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param76), tianma_OTM9608A_power_on_param76},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param77), tianma_OTM9608A_power_on_param77},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param78), tianma_OTM9608A_power_on_param78},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param79), tianma_OTM9608A_power_on_param79},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param80), tianma_OTM9608A_power_on_param80},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param81), tianma_OTM9608A_power_on_param81},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param82), tianma_OTM9608A_power_on_param82},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param83), tianma_OTM9608A_power_on_param83},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param84), tianma_OTM9608A_power_on_param84},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param85), tianma_OTM9608A_power_on_param85},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param86), tianma_OTM9608A_power_on_param86},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param87), tianma_OTM9608A_power_on_param87},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param88), tianma_OTM9608A_power_on_param88},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param89), tianma_OTM9608A_power_on_param89},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param90), tianma_OTM9608A_power_on_param90},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param91), tianma_OTM9608A_power_on_param91},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param92), tianma_OTM9608A_power_on_param92},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param93), tianma_OTM9608A_power_on_param93},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param94), tianma_OTM9608A_power_on_param94},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param95), tianma_OTM9608A_power_on_param95},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param96), tianma_OTM9608A_power_on_param96},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param97), tianma_OTM9608A_power_on_param97},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param98), tianma_OTM9608A_power_on_param98},
	{DTYPE_DCS_LWRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param99), tianma_OTM9608A_power_on_param99},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param100), tianma_OTM9608A_power_on_param100},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param101), tianma_OTM9608A_power_on_param101},
	{DTYPE_DCS_WRITE1, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_power_on_param102), tianma_OTM9608A_power_on_param102},

	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_exit_sleep), tianma_OTM9608A_exit_sleep},
	{DTYPE_DCS_WRITE, 0, 20, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_display_on), tianma_OTM9608A_display_on},
};

static struct dsi_cmd_desc tianma_OTM9608A_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_display_off), tianma_OTM9608A_display_off},

	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_OTM9608A_enter_sleep), tianma_OTM9608A_enter_sleep},
};
/********************************tianma_OTM9608A*****end*************************/


/********************************tianma_RAMLess*****start**************************
** Power ON Sequence(sleep mode to Normal mode)
*/
static char tianma_RAMLess_power_on_param1[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param2[] = {
	0xFF,
	0x96,0x05,0x01,
};

static char tianma_RAMLess_power_on_param3[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param4[] = {
	0xFF,0x96,0x05
};

static char tianma_RAMLess_power_on_param5[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param6[] = {
	0xA0,
	0x00,
};

static char tianma_RAMLess_power_on_param7[] = {
	0x00,
	0xB4,
};

static char tianma_RAMLess_power_on_param8[] = {
	0xC0,
	0x50,
};

static char tianma_RAMLess_power_on_param9[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param10[] = {
	0xC1,
	0x36,0x66,
};

static char tianma_RAMLess_power_on_param11[] = {
	0x00,
	0x89,
};

static char tianma_RAMLess_power_on_param12[] = {
	0xC0,
	0x01,
};

static char tianma_RAMLess_power_on_param13[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param14[] = {
	0xC4,
	0x9C,
};

static char tianma_RAMLess_power_on_param15[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param16[] = {
	0xC1,
	0x00,
};

static char tianma_RAMLess_power_on_param17[] = {
	0x00,
	0xC0,
};

static char tianma_RAMLess_power_on_param18[] = {
	0xC5,
	0x00,
};

static char tianma_RAMLess_power_on_param19[] = {
	0x00,
	0xC5,
};

static char tianma_RAMLess_power_on_param20[] = {
	0xB0,
	0x03,
};

static char tianma_RAMLess_power_on_param21[] = {
	0x00,
	0x90,
};

static char tianma_RAMLess_power_on_param22[] = {
	0xc5,
	0x96,0x19,0x01,0x79,0x33,
	0x33,0x34,
};

static char tianma_RAMLess_power_on_param23[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param24[] = {
	0xC5,
	0x96,0x16,0x00,0x79,0x33,
	0x33,0x34,
};

static char tianma_RAMLess_power_on_param25[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param26[] = {
	0xD8,
	0x5F,0x5F,
};

static char tianma_RAMLess_power_on_param27[] = {
	0x00,
	0x00,
};
static char tianma_RAMLess_power_on_param28[] = {
	0xD9,
	0x3b,
};

static char tianma_RAMLess_power_on_param29[] = {
	0x00,
	0xB0,
};

static char tianma_RAMLess_power_on_param30[] = {
	0xC5,
	0x04,0x28,
};

static char tianma_RAMLess_power_on_param31[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param32[] = {
	0xE1,
	0x01,0x0D,0x14,0x0E,0x07,
	0x13,0x0B,0x0A,0x02,0x06,
	0x08,0x08,0x0e,0x12,0x0f,
	0x0b,
};

//GOA ECLK Setting and GOA Other Options1 and GOA Signal Toggle Option Setting
static char tianma_RAMLess_power_on_param33[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param34[] = {
	0xE2,
	0x01,0x0D,0x14,0x0E,0x07,
	0x13,0x0B,0x0A,0x02,0x06,
	0x08,0x08,0x0e,0x12,0x0f,
	0x0b,
};

static char tianma_RAMLess_power_on_param35[] = {
	0x00,
	0x80,
};
static char tianma_RAMLess_power_on_param36[] = {
	0xCB,
	0x05,0x00,0x00,0x00,0x00,
	0x05,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param37[] = {
	0x00,
	0x90,
};

static char tianma_RAMLess_power_on_param38[] = {
	0xCB,
	0x55,0x55,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param39[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param40[] = {
	0xCB,
	0x00,0x00,0x00,0x00,0x00,
	0x55,0x55,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param41[] = {
	0x00,
	0xB0,
};

static char tianma_RAMLess_power_on_param42[] = {
	0xCB,
	0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param43[] = {
	0x00,
	0xC0,
};

static char tianma_RAMLess_power_on_param44[] = {
	0xCB,
	0x55,0x55,0x00,0x00,0x00,
	0x04,0x00,0x04,0x00,0x04,
	0x00,0x04,0x04,0x04,0x00,
};

static char tianma_RAMLess_power_on_param45[] = {
	0x00,
	0xD0,
};

static char tianma_RAMLess_power_on_param46[] = {
	0xCB,
	0x04,0x00,0x00,0x00,0x00,
	0x55,0x55,0x00,0x00,0x00,
	0x04,0x00,0x04,0x00,0x04,
};

static char tianma_RAMLess_power_on_param47[] = {
	0x00,
	0xE0,
};

static char tianma_RAMLess_power_on_param48[] = {
	0xCB,
	0x00,0x04,0x04,0x04,0x00,
	0x04,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param49[] = {
	0x00,
	0xF0,
};

static char tianma_RAMLess_power_on_param50[] = {
	0xCB,
	0x0F,0x00,0xCC,0x00,0x00,
	0x0F,0x00,0xCC,0xC3,0x00,
};

static char tianma_RAMLess_power_on_param51[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param52[] = {
	0xCC,
	0x25,0x26,0x00,0x00,0x00,
	0x0C,0x00,0x0A,0x00,0x10,
};

static char tianma_RAMLess_power_on_param53[] = {
	0x00,
	0x90,
};

static char tianma_RAMLess_power_on_param54[] = {
	0xCC,
	0x00,0x0E,0x02,0x04,0x00,
	0x06,0x00,0x00,0x00,0x00,
	0x25,0x26,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param55[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param56[] = {
	0xCC,
	0x0B,0x00,0x09,0x00,0x0F,
	0x00,0x0D,0x01,0x03,0x00,
	0x05,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param57[] = {
	0x00,
	0xB0,
};

static char tianma_RAMLess_power_on_param58[] = {
	0xCC,
	0x26,0x25,0x00,0x00,0x00,
	0x0D,0x00,0x0F,0x00,0x09,
};

static char tianma_RAMLess_power_on_param59[] = {
	0x00,
	0xC0,
};

static char tianma_RAMLess_power_on_param60[] = {
	0xCC,
	0x00,0x0B,0x03,0x01,0x00,
	0x05,0x00,0x00,0x00,0x00,
	0x26,0x25,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param61[] = {
	0x00,
	0xD0,
};

static char tianma_RAMLess_power_on_param62[] = {
	0xCC,
	0x0E,0x00,0x10,0x00,0x0A,
	0x00,0x0C,0x04,0x02,0x00,
	0x06,0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param63[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param64[] = {
	0xCE,
	0x8B,0x03,0x27,0x8A,0x03,
	0x28,0x89,0x03,0x27,0x88,
	0x03,0x28,
};

static char tianma_RAMLess_power_on_param65[] = {
	0x00,
	0x90,
};

static char tianma_RAMLess_power_on_param66[] = {
	0xCE,
	0x39,0x0F,0x27,0x39,0x0E,
	0x28,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param67[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param68[] = {
	0xCE,
	0x38,0x07,0x03,0xC0,0x00,
	0x28,0x00,0x38,0x06,0x03,
	0xC1,0x00,0x28,0x00,
};

static char tianma_RAMLess_power_on_param69[] = {
	0x00,
	0xB0,
};

static char tianma_RAMLess_power_on_param70[] = {
	0xCE,
	0x38,0x05,0x03,0xC2,0x00,
	0x28,0x00,0x38,0x04,0x03,
	0xC3,0x00,0x28,0x00,
};

static char tianma_RAMLess_power_on_param71[] = {
	0x00,
	0xC0,
};

static char tianma_RAMLess_power_on_param72[] = {
	0xCE,
	0x38,0x03,0x03,0xC4,0x00,
	0x28,0x00,0x38,0x02,0x03,
	0xC5,0x00,0x28,0x00,
};

static char tianma_RAMLess_power_on_param73[] = {
	0x00,
	0xD0,
};

static char tianma_RAMLess_power_on_param74[] = {
	0xCE,
	0x38,0x01,0x03,0xC6,0x00,
	0x28,0x00,0x38,0x00,0x03,
	0xC7,0x00,0x28,0x00,
};

static char tianma_RAMLess_power_on_param75[] = {
	0x00,
	0x80,
};

static char tianma_RAMLess_power_on_param76[] = {
	0xCF,
	0xF0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param77[] = {
	0x00,
	0x90,
};

static char tianma_RAMLess_power_on_param78[] = {
	0xCF,
	0xF0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param79[] = {
	0x00,
	0xA0,
};

static char tianma_RAMLess_power_on_param80[] = {
	0xCF,
	0xF0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param81[] = {
	0x00,
	0xB0,
};

static char tianma_RAMLess_power_on_param82[] = {
	0xCF,
	0xF0,0x00,0x00,0x10,0x00,
	0x00,0x00,0xF0,0x00,0x00,
	0x10,0x00,0x00,0x00,
};

static char tianma_RAMLess_power_on_param83[] = {
	0x00,
	0xC0,
};

static char tianma_RAMLess_power_on_param84[] = {
	0xCF,
	0x01,0x01,0x20,0x20,0x00,
	0x00,0x02,0x01,0x00,0x00,
};

static char tianma_RAMLess_power_on_param85[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_power_on_param86[] = {
	0xFF,
	0xFF,0xFF,0xFF,
};

static char tianma_RAMLess_exit_sleep1[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_exit_sleep2[] = {
	0x11,
	0x00,
};

static char tianma_RAMLess_display_on1[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_display_on2[] = {
	0x29,
	0x00,
};

/*
**Power OFF Sequence(Normal to power off)
*/
static char tianma_RAMLess_display_off1[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_display_off2[] = {
	0x28,
	0x00,
};

static char tianma_RAMLess_enter_sleep1[] = {
	0x00,
	0x00,
};

static char tianma_RAMLess_enter_sleep2[] = {
	0x10,
	0x00,
};

static struct dsi_cmd_desc tianma_RAMLess_display_on_cmds[] = {
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param1), tianma_RAMLess_power_on_param1},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param2), tianma_RAMLess_power_on_param2},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param3), tianma_RAMLess_power_on_param3},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param4), tianma_RAMLess_power_on_param4},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param5), tianma_RAMLess_power_on_param5},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param6), tianma_RAMLess_power_on_param6},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param7), tianma_RAMLess_power_on_param7},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param8), tianma_RAMLess_power_on_param8},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param9), tianma_RAMLess_power_on_param9},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param10), tianma_RAMLess_power_on_param10},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param11), tianma_RAMLess_power_on_param11},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param12), tianma_RAMLess_power_on_param12},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param13), tianma_RAMLess_power_on_param13},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param14), tianma_RAMLess_power_on_param14},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param15), tianma_RAMLess_power_on_param15},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param16), tianma_RAMLess_power_on_param16},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param17), tianma_RAMLess_power_on_param17},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param18), tianma_RAMLess_power_on_param18},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param19), tianma_RAMLess_power_on_param19},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param20), tianma_RAMLess_power_on_param20},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param21), tianma_RAMLess_power_on_param21},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param22), tianma_RAMLess_power_on_param22},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param23), tianma_RAMLess_power_on_param23},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param24), tianma_RAMLess_power_on_param24},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param25), tianma_RAMLess_power_on_param25},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param26), tianma_RAMLess_power_on_param26},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param27), tianma_RAMLess_power_on_param27},
	{DTYPE_GEN_WRITE2, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param28), tianma_RAMLess_power_on_param28},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param29), tianma_RAMLess_power_on_param29},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param30), tianma_RAMLess_power_on_param30},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param31), tianma_RAMLess_power_on_param31},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param32), tianma_RAMLess_power_on_param32},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param33), tianma_RAMLess_power_on_param33},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param34), tianma_RAMLess_power_on_param34},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param35), tianma_RAMLess_power_on_param35},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param36), tianma_RAMLess_power_on_param36},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param37), tianma_RAMLess_power_on_param37},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param38), tianma_RAMLess_power_on_param38},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param39), tianma_RAMLess_power_on_param39},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param40), tianma_RAMLess_power_on_param40},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param41), tianma_RAMLess_power_on_param41},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param42), tianma_RAMLess_power_on_param42},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param43), tianma_RAMLess_power_on_param43},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param44), tianma_RAMLess_power_on_param44},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param45), tianma_RAMLess_power_on_param45},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param46), tianma_RAMLess_power_on_param46},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param47), tianma_RAMLess_power_on_param47},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param48), tianma_RAMLess_power_on_param48},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param49), tianma_RAMLess_power_on_param49},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param50), tianma_RAMLess_power_on_param50},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param51), tianma_RAMLess_power_on_param51},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param52), tianma_RAMLess_power_on_param52},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param53), tianma_RAMLess_power_on_param53},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param54), tianma_RAMLess_power_on_param54},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param55), tianma_RAMLess_power_on_param55},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param56), tianma_RAMLess_power_on_param56},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param57), tianma_RAMLess_power_on_param57},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param58), tianma_RAMLess_power_on_param58},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param59), tianma_RAMLess_power_on_param59},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param60), tianma_RAMLess_power_on_param60},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param61), tianma_RAMLess_power_on_param61},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param62), tianma_RAMLess_power_on_param62},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param63), tianma_RAMLess_power_on_param63},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param64), tianma_RAMLess_power_on_param64},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param65), tianma_RAMLess_power_on_param65},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param66), tianma_RAMLess_power_on_param66},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param67), tianma_RAMLess_power_on_param67},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param68), tianma_RAMLess_power_on_param68},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param69), tianma_RAMLess_power_on_param69},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param70), tianma_RAMLess_power_on_param70},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param71), tianma_RAMLess_power_on_param71},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param72), tianma_RAMLess_power_on_param72},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param73), tianma_RAMLess_power_on_param73},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param74), tianma_RAMLess_power_on_param74},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param75), tianma_RAMLess_power_on_param75},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param76), tianma_RAMLess_power_on_param76},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param77), tianma_RAMLess_power_on_param77},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param78), tianma_RAMLess_power_on_param78},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param79), tianma_RAMLess_power_on_param79},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param80), tianma_RAMLess_power_on_param80},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param81), tianma_RAMLess_power_on_param81},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param82), tianma_RAMLess_power_on_param82},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param83), tianma_RAMLess_power_on_param83},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param84), tianma_RAMLess_power_on_param84},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param85), tianma_RAMLess_power_on_param85},
	{DTYPE_GEN_LWRITE, 0, 1, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_power_on_param86), tianma_RAMLess_power_on_param86},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_exit_sleep1), tianma_RAMLess_exit_sleep1},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_exit_sleep2), tianma_RAMLess_exit_sleep2},
	{DTYPE_GEN_WRITE2, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_display_on1), tianma_RAMLess_display_on1},
	{DTYPE_GEN_WRITE2, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_display_on2), tianma_RAMLess_display_on2},
};

static struct dsi_cmd_desc tianma_RAMLess_display_off_cmds[] = {
	{DTYPE_DCS_WRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_display_off1), tianma_RAMLess_display_off1},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_display_off2), tianma_RAMLess_display_off2},
	{DTYPE_DCS_WRITE, 0, 0, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_enter_sleep1), tianma_RAMLess_enter_sleep1},
	{DTYPE_DCS_WRITE, 0, 120, WAIT_TYPE_MS,
		sizeof(tianma_RAMLess_enter_sleep2), tianma_RAMLess_enter_sleep2},
};
/********************************tianma_RAMLess*****end*************************/

/*******************************************************************************
** LCD VCC
*/
#define VCC_LCDIO_NAME		"lcdio-vcc"
#define VCC_LCDANALOG_NAME	"lcdanalog-vcc"

static struct regulator *vcc_lcdio;
static struct regulator *vcc_lcdanalog;

static struct vcc_desc tianma_lcd_vcc_init_cmds[] = {
	/* vcc get */
	{DTYPE_VCC_GET, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_GET, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},

	/* vcc set voltage */
	{DTYPE_VCC_SET_VOLTAGE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 2800000, 2800000, WAIT_TYPE_MS, 0},
};

static struct vcc_desc tianma_lcd_vcc_finit_cmds[] = {
	/* vcc put */
	{DTYPE_VCC_PUT, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 0},
	{DTYPE_VCC_PUT, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 0},
};

static struct vcc_desc tianma_lcd_vcc_enable_cmds[] = {
	/* vcc enable */
	{DTYPE_VCC_ENABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_ENABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
};

static struct vcc_desc tianma_lcd_vcc_disable_cmds[] = {
	/* vcc disable */
	{DTYPE_VCC_DISABLE, VCC_LCDANALOG_NAME, &vcc_lcdanalog, 0, 0, WAIT_TYPE_MS, 3},
	{DTYPE_VCC_DISABLE, VCC_LCDIO_NAME, &vcc_lcdio, 0, 0, WAIT_TYPE_MS, 3},
};


/*******************************************************************************
** LCD GPIO
*/
#define GPIO_LCD_POWER_2V85_EN0_NAME	"LCD_2V85_EN0"
#define GPIO_LCD_POWER_2V85_EN1_NAME	"LCD_2V85_EN1"
#define GPIO_LCD_POWER_1V8_EN0_NAME	"LCD_1V8_EN0"
#define GPIO_LCD_POWER_1V8_EN1_NAME	"LCD_1V8_EN1"
#define GPIO_LCD_RST_NAME	"LCD_RST"
#define GPIO_LCD_ID1_NAME	"LCD_ID1"

static uint32_t gpio_lcd_power_2v85_en0;
static uint32_t gpio_lcd_power_1v8_en0;
static uint32_t gpio_lcd_power_2v85_en1;
static uint32_t gpio_lcd_power_1v8_en1;
static uint32_t gpio_lcd_rst;
static uint32_t gpio_lcd_id1;

static struct gpio_desc tianma_lcd_gpio_request_cmds[] = {
	/* power 2.85v */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_2V85_EN0_NAME, &gpio_lcd_power_2v85_en0, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_2V85_EN1_NAME, &gpio_lcd_power_2v85_en1, 0},
	/* power 1.8v */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN0_NAME, &gpio_lcd_power_1v8_en0, 0},
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN1_NAME, &gpio_lcd_power_1v8_en1, 0},
	/* reset */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 0},
	/* lcd_id1 */
	{DTYPE_GPIO_REQUEST, WAIT_TYPE_MS, 0,
		GPIO_LCD_ID1_NAME, &gpio_lcd_id1, 0},
};

static struct gpio_desc tianma_lcd_gpio_free_cmds[] = {
	/* lcd_id1 */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_ID1_NAME, &gpio_lcd_id1, 0},
	/* reset */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 0},
	/* power 1.8v */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN0_NAME, &gpio_lcd_power_1v8_en0, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN1_NAME, &gpio_lcd_power_1v8_en1, 0},
	/* power 2.85v */
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_2V85_EN0_NAME, &gpio_lcd_power_2v85_en0, 0},
	{DTYPE_GPIO_FREE, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_2V85_EN1_NAME, &gpio_lcd_power_2v85_en1, 0},
};

static struct gpio_desc tianma_lcd_gpio_normal_cmds[] = {
	/*power 1.8v  */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN0_NAME, &gpio_lcd_power_1v8_en0, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN1_NAME, &gpio_lcd_power_1v8_en1, 1},
	/* power 2.85v */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 25,
		GPIO_LCD_POWER_2V85_EN0_NAME, &gpio_lcd_power_2v85_en0, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 25,
		GPIO_LCD_POWER_2V85_EN1_NAME, &gpio_lcd_power_2v85_en1, 1},
	/* reset */
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 1},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_US, 50,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 0},
	{DTYPE_GPIO_OUTPUT, WAIT_TYPE_MS, 25,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 1},
	/*lcd_id1 */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 100,
		GPIO_LCD_ID1_NAME, &gpio_lcd_id1, 0},
};

static struct gpio_desc tianma_lcd_gpio_lowpower_cmds[] = {
	/* reset */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_US, 50,
		GPIO_LCD_RST_NAME, &gpio_lcd_rst, 0},
	/* power 2.85v */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 25,
		GPIO_LCD_POWER_2V85_EN0_NAME, &gpio_lcd_power_2v85_en0, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 25,
		GPIO_LCD_POWER_2V85_EN1_NAME, &gpio_lcd_power_2v85_en1, 0},
	/*power 1.8v  */
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN0_NAME, &gpio_lcd_power_1v8_en0, 0},
	{DTYPE_GPIO_INPUT, WAIT_TYPE_MS, 0,
		GPIO_LCD_POWER_1V8_EN1_NAME, &gpio_lcd_power_1v8_en1, 0},
};


static struct hisi_fb_panel_data g_panel_data;


/*******************************************************************************
** ACM
*/


/*******************************************************************************
**
*/
static int mipi_tianma_panel_set_fastboot(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	// lcd gpio request
	gpio_cmds_tx(tianma_lcd_gpio_request_cmds, \
		ARRAY_SIZE(tianma_lcd_gpio_request_cmds));

	// backlight on
	hisi_lcd_backlight_on(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_tianma_panel_on(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);
	BUG_ON(pinfo == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (pinfo->lcd_init_step == LCD_INIT_POWER_ON) {
		// lcd vcc enable
		vcc_cmds_tx(NULL, tianma_lcd_vcc_enable_cmds, \
			ARRAY_SIZE(tianma_lcd_vcc_enable_cmds));

		pinfo->lcd_init_step = LCD_INIT_MIPI_LP_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_LP_SEND_SEQUENCE) {
		// lcd gpio request
		gpio_cmds_tx(tianma_lcd_gpio_request_cmds, \
			ARRAY_SIZE(tianma_lcd_gpio_request_cmds));
		// lcd gpio normal
		gpio_cmds_tx(tianma_lcd_gpio_normal_cmds, \
			ARRAY_SIZE(tianma_lcd_gpio_normal_cmds));

		if (gpio_get_value(gpio_lcd_id1) == 1) {
			HISI_FB_INFO("fb%d, tianma_OTM9608A.\n", hisifd->index);
			rgb2mipi_cmds_tx(pinfo->spi_dev, tianma_OTM9608A_display_on_cmds, \
				ARRAY_SIZE(tianma_OTM9608A_display_on_cmds));
		} else {
			HISI_FB_INFO("fb%d, tianma_RAMLess.\n", hisifd->index);
			rgb2mipi_cmds_tx(pinfo->spi_dev, tianma_RAMLess_display_on_cmds, \
				ARRAY_SIZE(tianma_RAMLess_display_on_cmds));
		}

		pinfo->lcd_init_step = LCD_INIT_MIPI_HS_SEND_SEQUENCE;
	} else if (pinfo->lcd_init_step == LCD_INIT_MIPI_HS_SEND_SEQUENCE) {
		;
	} else {
		HISI_FB_ERR("failed to init lcd!\n");
	}

	// backlight on
	hisi_lcd_backlight_on(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_tianma_panel_off(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);
	pinfo = &(hisifd->panel_info);
	BUG_ON(pinfo == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	// backlight off
	hisi_lcd_backlight_off(pdev);

	// lcd display off sequence
	if (gpio_get_value(gpio_lcd_id1) == 1) {
		HISI_FB_INFO("fb%d, tianma_OTM9608A.\n", hisifd->index);
		rgb2mipi_cmds_tx(pinfo->spi_dev, tianma_OTM9608A_display_off_cmds, \
			ARRAY_SIZE(tianma_OTM9608A_display_off_cmds));
	} else {
		HISI_FB_INFO("fb%d, tianma_RAMLess.\n", hisifd->index);
		rgb2mipi_cmds_tx(pinfo->spi_dev, tianma_RAMLess_display_off_cmds, \
			ARRAY_SIZE(tianma_RAMLess_display_off_cmds));
	}

	// lcd gpio lowpower
	gpio_cmds_tx(tianma_lcd_gpio_lowpower_cmds, \
		ARRAY_SIZE(tianma_lcd_gpio_lowpower_cmds));
	// lcd gpio free
	gpio_cmds_tx(tianma_lcd_gpio_free_cmds, \
		ARRAY_SIZE(tianma_lcd_gpio_free_cmds));

	// lcd vcc disable
	vcc_cmds_tx(NULL, tianma_lcd_vcc_disable_cmds, \
		ARRAY_SIZE(tianma_lcd_vcc_disable_cmds));

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_tianma_panel_remove(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	// lcd vcc finit
	vcc_cmds_tx(pdev, tianma_lcd_vcc_finit_cmds, \
		ARRAY_SIZE(tianma_lcd_vcc_finit_cmds));

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;
}

static int mipi_tianma_panel_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (hisifd->panel_info.bl_set_type & BL_SET_BY_PWM) {
		ret = hisi_pwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_BLPWM) {
		ret = hisi_blpwm_set_backlight(hisifd, bl_level);
	} else if (hisifd->panel_info.bl_set_type & BL_SET_BY_SH_BLPWM) {
		ret = hisi_sh_blpwm_set_backlight(hisifd, bl_level);
	} else {
		;
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}


/******************************************************************************/
static ssize_t mipi_tianma_panel_lcd_model_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = snprintf(buf, PAGE_SIZE, "mipi_tianma_OTM9608A\n");

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tianma_panel_lcd_cabc_mode_show(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tianma_panel_lcd_cabc_mode_store(struct platform_device *pdev,
	const char *buf, size_t count)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tianma_panel_lcd_check_reg(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static ssize_t mipi_tianma_panel_lcd_mipi_detect(struct platform_device *pdev,
	char *buf)
{
	struct hisi_fb_data_type *hisifd = NULL;
	ssize_t ret = 0;

	BUG_ON(pdev == NULL);
	hisifd = platform_get_drvdata(pdev);
	BUG_ON(hisifd == NULL);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static struct hisi_panel_info g_panel_info = {0};
static struct hisi_fb_panel_data g_panel_data = {
	.panel_info = &g_panel_info,
	.set_fastboot = mipi_tianma_panel_set_fastboot,
	.on = mipi_tianma_panel_on,
	.off = mipi_tianma_panel_off,
	.remove = mipi_tianma_panel_remove,
	.set_backlight = mipi_tianma_panel_set_backlight,

	.lcd_model_show = mipi_tianma_panel_lcd_model_show,
	.lcd_cabc_mode_show = mipi_tianma_panel_lcd_cabc_mode_show,
	.lcd_cabc_mode_store = mipi_tianma_panel_lcd_cabc_mode_store,
	.lcd_check_reg = mipi_tianma_panel_lcd_check_reg,
	.lcd_mipi_detect = mipi_tianma_panel_lcd_mipi_detect,
	.set_display_region = NULL,
	.set_display_resolution = NULL,
};


/*******************************************************************************
**
*/
static int mipi_tianma_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_panel_info *pinfo = NULL;
	struct device_node *np = NULL;
	uint32_t bl_type = 0;

	np = of_find_compatible_node(NULL, NULL, DTS_COMP_MIPI_TIANMA_OTM9608A);
	if (!np) {
		HISI_FB_ERR("NOT FOUND device node %s!\n", DTS_COMP_MIPI_TIANMA_OTM9608A);
		goto err_return;
	}

	ret = of_property_read_u32(np, LCD_BL_TYPE_NAME, &bl_type);
	if (ret) {
		HISI_FB_ERR("get lcd_bl_type failed!\n");
		bl_type = BL_SET_BY_BLPWM;
	}
	HISI_FB_INFO("bl_type=0x%x.", bl_type);

	if (hisi_fb_device_probe_defer(PANEL_RGB2MIPI, bl_type)) {
		goto err_probe_defer;
	}

	HISI_FB_DEBUG("+.\n");

	HISI_FB_INFO("%s\n", DTS_COMP_MIPI_TIANMA_OTM9608A);

	//GPIO_6_2
	gpio_lcd_power_2v85_en1 = of_get_named_gpio(np, "gpios", 0);
	//GPIO_6_3
	gpio_lcd_power_2v85_en0 = of_get_named_gpio(np, "gpios", 1);
	//GPIO_6_4
	gpio_lcd_power_1v8_en0 = of_get_named_gpio(np, "gpios", 2);
	//GPIO_6_5
	gpio_lcd_power_1v8_en1 = of_get_named_gpio(np, "gpios", 3);
	//GPIO_8_4
	gpio_lcd_rst = of_get_named_gpio(np, "gpios", 4);
	//GPIO_4_1
	gpio_lcd_id1 = of_get_named_gpio(np, "gpios", 6);

	ret = of_property_read_u32(np, "fpga_flag", &g_lcd_fpga_flag);
	if (ret) {
		HISI_FB_WARNING("need to get g_lcd_fpga_flag resource in fpga, not needed in asic!\n");
	}

	pdev->id = 1;
	/* init lcd panel info */
	pinfo = g_panel_data.panel_info;
	memset(pinfo, 0, sizeof(struct hisi_panel_info));
	pinfo->xres = 540;
	pinfo->yres = 960;
	pinfo->width = 62;
	pinfo->height = 110;
	pinfo->orientation = LCD_PORTRAIT;
	pinfo->bpp = LCD_RGB888;
	pinfo->bgr_fmt = LCD_RGB;
	pinfo->bl_set_type = bl_type;

	if (pinfo->bl_set_type == BL_SET_BY_BLPWM)
		pinfo->blpwm_input_ena = 0;

	pinfo->bl_min = 157;
	pinfo->bl_max = 9960;
	pinfo->bl_default = 4000;
	pinfo->blpwm_precision_type = BLPWM_PRECISION_10000_TYPE;

	pinfo->spi_dev = g_spi_dev;

	pinfo->frc_enable = 0;
	pinfo->esd_enable = 0;

	//sbl
	pinfo->sbl_support = 0;

	//ldi
	pinfo->ldi.h_back_porch = 50;
	pinfo->ldi.h_front_porch = 697; //177
	pinfo->ldi.h_pulse_width = 11;
	pinfo->ldi.v_back_porch = 32;
	pinfo->ldi.v_front_porch = 32;
	pinfo->ldi.v_pulse_width = 4;
	pinfo->ldi.hsync_plr = 1;
	pinfo->ldi.vsync_plr = 1;
	pinfo->ldi.pixelclk_plr = 0;
	pinfo->ldi.data_en_plr = 0;

	//mipi
	pinfo->mipi.lane_nums = DSI_2_LANES;
	pinfo->mipi.color_mode = DSI_24BITS_1;
	pinfo->mipi.vc = 0;
	pinfo->mipi.max_tx_esc_clk = 10 * 1000000;
	pinfo->mipi.burst_mode = 0;

	pinfo->mipi.dsi_bit_clk = 241;

	pinfo->pxl_clk_rate = 40 * 1000000UL;

	pinfo->type = PANEL_RGB2MIPI;
	pinfo->mipi.burst_mode = DSI_BURST_SYNC_PULSES_1;
	pinfo->ifbc_type = IFBC_TYPE_NONE;
	pinfo->vsync_ctrl_type = 0;
	pinfo->dirty_region_updt_support = 0;

	if (g_lcd_fpga_flag == 0) {
		// lcd vcc init
		ret = vcc_cmds_tx(pdev, tianma_lcd_vcc_init_cmds, \
			ARRAY_SIZE(tianma_lcd_vcc_init_cmds));
		if (ret != 0) {
			HISI_FB_ERR("LCD vcc init failed!\n");
			goto err_return;
		}

		// lcd vcc enable
		if (is_fastboot_display_enable()) {
			vcc_cmds_tx(pdev, tianma_lcd_vcc_enable_cmds,
				ARRAY_SIZE(tianma_lcd_vcc_enable_cmds));
		}
	}

	// alloc panel device data
	ret = platform_device_add_data(pdev, &g_panel_data,
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		HISI_FB_ERR("platform_device_add_data failed!\n");
		goto err_device_put;
	}

	hisi_fb_add_device(pdev);

	HISI_FB_DEBUG("-.\n");

	return 0;

err_device_put:
	platform_device_put(pdev);
err_return:
	return ret;
err_probe_defer:
	return -EPROBE_DEFER;
}

static const struct of_device_id hisi_panel_match_table[] = {
	{
		.compatible = DTS_COMP_MIPI_TIANMA_OTM9608A,
		.data = NULL,
	},
	{},
};

static struct platform_driver this_driver = {
	.probe = mipi_tianma_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = "mipi_tianma_OTM9608A",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_panel_match_table),
	},
};

static int __init mipi_tianma_panel_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(mipi_tianma_panel_init);
