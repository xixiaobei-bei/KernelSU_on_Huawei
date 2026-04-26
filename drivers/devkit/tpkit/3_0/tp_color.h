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

#define WHITE 0xE1
#define WHITE_OLD 0x02
#define BLACK 0xD2
#define BLACK_OLD 0x01
#define PINK 0xC3
#define RED 0xB4
#define YELLOW 0xA5
#define BLUE 0x96
#define GOLD 0x87
#define GOLD_OLD 0x04
#define SILVER 0x3C
#define GRAY 0x78
#define CAFE 0x69
#define CAFE2 0x5A
#define BLACK2 0x4B
#define GREEN 0x2D
#define PINKGOLD 0x1E

#define TP_COLOR_BUF_SIZE 20
#define TP_COLOR_SIZE 15
#define TP_COLOR_MASK 0x0f
#define TP_COLOR_OFFSET_4BIT 4
#define NV_DATA_SIZE 104
#define DEC_BASE_DATA 10

#define NV_WRITE_TIMEOUT 200
#define NV_READ_TIMEOUT 200

#endif
