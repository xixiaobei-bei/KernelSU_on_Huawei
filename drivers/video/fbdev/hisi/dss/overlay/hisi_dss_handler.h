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

#ifndef HISI_DSS_HANDLER_H
#define HISI_DSS_HANDLER_H

#include "../hisi_fb.h"

void hisi_dss_unflow_handler(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, bool unmask);

int hisi_ov_compose_handler(struct dpu_fb_data_type *dpufd,
	dss_overlay_block_t *pov_h_block,
	dss_layer_t *layer,
	struct hisi_ov_compose_rect *ov_compose_rect,
	struct hisi_ov_compose_flag *ov_compose_flag);

int hisi_wb_compose_handler(struct dpu_fb_data_type *dpufd,
	dss_wb_layer_t *wb_layer,
	dss_rect_t *wb_ov_block_rect,
	bool last_block,
	struct hisi_ov_compose_flag ov_compose_flag);

int hisi_wb_ch_module_set_regs(struct dpu_fb_data_type *dpufd, uint32_t wb_type,
	dss_wb_layer_t *wb_layer, bool enable_cmdlist);


void hisi_vactive0_start_isr_handler(struct dpu_fb_data_type *dpufd);
void hisi_vactive0_end_isr_handler(struct dpu_fb_data_type *dpufd);
int hisi_overlay_ioctl_handler(struct dpu_fb_data_type *dpufd,
	uint32_t cmd, void __user *argp);

#endif /* HISI_DSS_HANDLER_H */
