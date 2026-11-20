/*
 * Huawei Touchscreen Driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HUAWEI_TP_COLOR_H_
#define __HUAWEI_TP_COLOR_H_

#define WHITE    0xE1
#define BLACK    0xD2
#define BLACK2   0x4B
#define PINK     0xC3
#define RED      0xB4
#define YELLOW   0xA5
#define BLUE     0x96
#define GOLD     0x87
#define PINKGOLD 0x1E
#define SILVER   0x3C
#define GRAY     0x78
#define CAFE     0x69
#define CAFE2    0x5A
#define GREEN    0x2D
#define TP_COLOR_BUF_SIZE 20

enum tp_colors_supported {
	TP_COLOR_WHITE = 0,
	TP_COLOR_BLACK,
	TP_COLOR_BLACK2,
	TP_COLOR_PINK,
	TP_COLOR_RED,
	TP_COLOR_YELLOW,
	TP_COLOR_BLUE,
	TP_COLOR_GOLD,
	TP_COLOR_PINKGOLD,
	TP_COLOR_SILVER,
	TP_COLOR_GRAY,
	TP_COLOR_CAFE,
	TP_COLOR_CAFE2,
	TP_COLOR_GREEN,
	TP_COLOR_MAX,
};
#endif
