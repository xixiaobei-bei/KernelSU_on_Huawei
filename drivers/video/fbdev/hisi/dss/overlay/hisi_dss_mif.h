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

#ifndef HISI_DSS_MIF_H
#define HISI_DSS_MIF_H

#include "../hisi_fb.h"

void hisi_dss_mif_init(char __iomem *mif_ch_base, dss_mif_t *s_mif, int chn_idx);
void hisi_dss_mif_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mif_ch_base, dss_mif_t *s_mif, int chn_idx);
void hisi_dss_mif_on(struct dpu_fb_data_type *dpufd);
int hisi_dss_mif_config(struct dpu_fb_data_type *dpufd,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer, bool rdma_stretch_enable);


#endif /* HISI_DSS_MIF_H */
