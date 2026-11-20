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

#ifndef HISI_DSS_DIRTY_H
#define HISI_DSS_DIRTY_H

#include "../hisi_fb.h"

void hisi_dss_dirty_region_dbuf_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *dss_base, dirty_region_updt_t *s_dirty_region_updt);

int hisi_dss_dirty_region_dbuf_config(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req);

void hisi_dss_dirty_region_updt_config(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req);

#endif /* HISI_DSS_DIRTY_H */
