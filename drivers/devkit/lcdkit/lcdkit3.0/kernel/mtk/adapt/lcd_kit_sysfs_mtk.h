/*
 * lcd_kit_sysfs_mtk.c
 *
 * lcdkit sysfs function head file for lcd driver mtk platform
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __LCD_KIT_SYSFS_HS_H_
#define __LCD_KIT_SYSFS_HS_H_
/* oem info */
#define OEM_INFO_SIZE_MAX 500
/* 2d barcode */
#define BARCODE_LENGTH    46
#define BARCODE_BLOCK_NUM 3
#define BARCODE_BLOCK_LEN 16

enum oem_type {
	PRODUCT_ID_TYPE,
	BARCODE_2D_TYPE,
	BRIGHTNESS_TYPE,
};

#endif
