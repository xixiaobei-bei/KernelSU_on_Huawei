/**
 * @file
 * Copyright (c) 2021-2021, Huawei Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef DPU_FB_H
#define DPU_FB_H
#include <linux/types.h>
#include "dpu_comp.h"

#define FB_BUFFER_MAX_COUNT 3

enum {
	FB_FORMAT_BGRA8888,
	FB_FORMAT_RGB565,
	FB_FORMAT_YUV_422_I,
	FB_FOARMT_MAX
};

struct fb_fix_var_screeninfo {
	uint32_t fix_type;
	uint32_t fix_xpanstep;
	uint32_t fix_ypanstep;
	uint32_t var_vmode;

	uint32_t var_blue_offset;
	uint32_t var_green_offset;
	uint32_t var_red_offset;
	uint32_t var_transp_offset;

	uint32_t var_blue_length;
	uint32_t var_green_length;
	uint32_t var_red_length;
	uint32_t var_transp_length;

	uint32_t var_blue_msb_right;
	uint32_t var_green_msb_right;
	uint32_t var_red_msb_right;
	uint32_t var_transp_msb_right;
	uint32_t bpp;
};

struct device_fb {
	uint32_t scene_id;
	atomic_t ref_cnt;
	bool be_connected;
	bool power_on;
	uint32_t bpp;

	struct semaphore blank_sem;

	struct fb_info *fbi_info;
	const struct dkmd_connector_info *connector_info;
	struct composer *next_composer;
};

void fb_device_register(struct composer *comp);
void fb_device_unregister(struct composer *comp);

#endif