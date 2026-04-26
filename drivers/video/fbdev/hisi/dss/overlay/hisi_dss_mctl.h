/* Copyright (c) 2013-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef HISI_DSS_MCTL_H
#define HISI_DSS_MCTL_H

#include "../hisi_fb.h"

void hisi_dss_mctl_init(const char __iomem *mctl_base, dss_mctl_t *s_mctl);
void hisi_dss_mctl_ch_starty_init(const char __iomem *mctl_ch_starty_base, dss_mctl_ch_t *s_mctl_ch);
void hisi_dss_mctl_ch_mod_dbg_init(const char __iomem *mctl_ch_dbg_base, dss_mctl_ch_t *s_mctl_ch);
void hisi_dss_mctl_sys_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mctl_sys_base, dss_mctl_sys_t *s_mctl_sys, int ovl_idx);
void hisi_dss_mctl_ov_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mctl_base, dss_mctl_t *s_mctl, int ovl_idx, bool enable_cmdlist);
void hisi_dss_mctl_ch_set_reg(struct dpu_fb_data_type *dpufd,
	dss_mctl_ch_base_t *mctl_ch_base, dss_mctl_ch_t *s_mctl_ch, int32_t mctl_idx, int chn_idx);
void hisi_dss_mctl_sys_ch_set_reg(struct dpu_fb_data_type *dpufd,
	dss_mctl_ch_base_t *mctl_ch_base, dss_mctl_ch_t *s_mctl_ch, int chn_idx, bool normal);
void hisi_dss_mctl_mutex_lock(struct dpu_fb_data_type *dpufd, int ovl_idx);
void hisi_dss_mctl_mutex_unlock(struct dpu_fb_data_type *dpufd, int ovl_idx);
void hisi_dss_mctl_on(struct dpu_fb_data_type *dpufd,
	int mctl_idx, bool enable_cmdlist, bool fastboot_enable);
int hisi_dss_mctl_ch_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_wb_layer_t *wb_layer, dss_rect_t *wb_ov_block_rect, bool has_base);

int hisi_dss_mctl_ov_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	int ovl_idx, bool has_base, bool is_first_ov_block);

#endif /* HISI_DSS_MCTL_H */
