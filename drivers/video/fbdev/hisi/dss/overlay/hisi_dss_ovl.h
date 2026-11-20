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

#ifndef HISI_DSS_OVL_H
#define HISI_DSS_OVL_H

#include "../hisi_fb.h"

struct img_size {
	int img_width;
	int img_height;
};

int hisi_dss_ovl_base_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block, dss_rect_t *wb_ov_block_rect, int ov_h_block_idx);

int get_img_size_info(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect, struct img_size *img_info);

void set_ovl_struct(struct dpu_fb_data_type *dpufd, dss_ovl_t *ovl, struct img_size img_info, int block_size);

int hisi_dss_ovl_layer_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_layer_t *layer, dss_rect_t *wb_ov_block_rect, bool has_base);

#endif  /* HISI_DSS_OVL_H */
