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

#ifndef HISI_DSS_CLD_H
#define HISI_DSS_CLD_H

#include "hisi_overlay_utils.h"

void hisi_dss_cld_init(const char __iomem *cld_base, struct dss_cld *s_cld);
void hisi_dss_cld_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *cld_base, struct dss_cld *s_cld, int channel);
int hisi_dss_cld_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer);

#endif /* HISI_DSS_CLD_H */
