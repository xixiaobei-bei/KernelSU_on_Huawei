/*
 * lcd_kit_sysfs_hs.h
 *
 * lcdkit sys node function for lcd driver head file
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

#include "hisi_fb.h"

/* oem info */
#define OEM_INFO_SIZE_MAX  500
#define OEM_INFO_BLOCK_NUM 1
/* 2d barcode */
#define BARCODE_LENGTH    46
#define BARCODE_BLOCK_NUM 3
#define BARCODE_BLOCK_LEN 16
/* project id */
#define PROJECT_ID_LENGTH 10
#define FPS_ELEM_NUM 2

enum oem_type {
	PROJECT_ID_TYPE,
	BARCODE_2D_TYPE,
	BRIGHTNESS_TYPE,
	COLOROFWHITE_TYPE,
	BRIGHTNESSANDCOLOR_TYPE,
	REWORK_TYPE,
	BRIGHTNESS_COLOROFWHITE_TYPE,
	RGBW_WHITE_TYPE,
};

/* struct */
struct oem_info_cmd {
	unsigned char type;
	int (*func)(char *oem_data, struct hisi_fb_data_type *hisifd);
};
#endif
