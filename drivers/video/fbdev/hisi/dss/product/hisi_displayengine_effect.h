/* Copyright (c) 2020-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_DISPLAYENGINE_EFFECT_H
#define HISI_DISPLAYENGINE_EFFECT_H

#include "hisi_fb.h"

struct effect_bl_buf {
	uint32_t blc_enable;
	int delta;
	struct dss_display_effect_xcc xcc_param;
	uint32_t dc_enable;
	uint32_t dimming_enable;
	int ddic_alpha;
	int panel_id;
};

int display_engine_param_get(struct fb_info *info, void __user *argp);
void dpufb_display_effect_separate_alpha_from_xcc(struct effect_bl_buf *resolved_buf);

#endif /* HISI_DISPLAYENGINE_EFFECT_H */
