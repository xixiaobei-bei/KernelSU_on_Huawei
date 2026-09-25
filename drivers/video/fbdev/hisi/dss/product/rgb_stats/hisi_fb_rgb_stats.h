/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#define DISP_GLB_TIME_PERIOD  (100)
#define DISP_GLB_REG_LENGTH  (16)

struct hisifb_rgb_data {
	uint64_t red[DISP_GLB_REG_LENGTH];
	uint64_t green[DISP_GLB_REG_LENGTH];
	uint64_t blue[DISP_GLB_REG_LENGTH];
} __packed;

#if defined (CONFIG_HUAWEI_DUBAI_RGB_STATS)
void hisifb_rgb_start_wq(void);
void hisifb_rgb_cancel_wq(void);
void hisifb_rgb_reg_enable(struct hisi_fb_data_type *hisifd);
void hisifb_rgb_read_register(struct hisi_fb_data_type *hisifd);
uint32_t hisifb_rgb_get_data(struct hisifb_rgb_data *data);
bool hisifb_rgb_set_enable(bool enable);
#else
static inline void hisifb_rgb_start_wq(void) {return;}
static inline void hisifb_rgb_cancel_wq(void) {return;}
static inline void hisifb_rgb_reg_enable(struct hisi_fb_data_type *hisifd) {return;}
static inline void hisifb_rgb_read_register(struct hisi_fb_data_type *hisifd){return;}
static inline uint32_t hisifb_rgb_get_data(struct hisifb_rgb_data *data){return -1;}
static inline bool hisifb_rgb_set_enable(bool enable){return false;}
#endif
#endif
