/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_FB_RGB_STATS_H
#define HISI_FB_RGB_STATS_H

#include "video/fbdev/hisi/dss/hisi_fb.h"

#ifdef CONFIG_HUAWEI_DUBAI_RGB_STATS
void dpufb_rgb_notify_panel_state(bool state);
void dpufb_rgb_reg_enable(const struct dpu_fb_data_type *dpufd);
void dpufb_rgb_read_register(const struct dpu_fb_data_type *dpufd);
void dpufb_rgb_stats_register(const struct dpu_fb_data_type *dpufd);
#endif

#endif

