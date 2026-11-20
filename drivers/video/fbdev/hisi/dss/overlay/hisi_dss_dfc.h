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

#ifndef HISI_DSS_DFC_H
#define HISI_DSS_DFC_H

#include "../hisi_fb.h"

void hisi_dss_dfc_init(const char __iomem *dfc_base, dss_dfc_t *s_dfc);
void hisi_dss_dfc_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dfc_base, dss_dfc_t *s_dfc);
int hisi_dss_rdfc_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer,
	dss_rect_t *aligned_rect,
	dss_rect_ltrb_t clip_rect,
	struct hisi_ov_compose_flag *ov_compose_flag);
int hisi_dss_wdfc_config(struct dpu_fb_data_type *dpufd, dss_wb_layer_t *layer,
	dss_rect_t *aligned_rect, dss_rect_t *ov_block_rect);

#endif /* HISI_DSS_DFC_H */
