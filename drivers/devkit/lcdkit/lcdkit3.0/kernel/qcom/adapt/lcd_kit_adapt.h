/*
 * lcd_kit_adapt.h
 *
 * lcdkit adapt function head file for lcd driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

/* dsc dsi1.2 vesa3x long write */
#define DTYPE_DSC_LWRITE  0x0A
#define MTK_READ_MAX_LEN 10

#ifndef __LCD_KIT_ADAPT_H_
#define __LCD_KIT_ADAPT_H_
int lcd_kit_adapt_init(void);
#endif
