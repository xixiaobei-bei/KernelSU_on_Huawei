/* Copyright (c) 2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef __DPTX_DP_DSC_CFG_H__
#define __DPTX_DP_DSC_CFG_H__
#include "../../../hisi_dp.h"
#include "../../../hisi_fb.h"

#define DPTX_CLEAR_DSC_REGISTER_TO_ZERO		0X0
#define DPTX_DSC_PPS_NUM_MIN				0
#define DPTX_DSC_PPS_NUM_MAX				32
#define DPTX_DSC_PPS_NUM_STEP				0X4

void dptx_program_pps_sdps(struct dp_ctrl *dptx);
void dptx_dsc_enable(struct dp_ctrl *dptx, int stream);

int dptx_dsc_get_clock_div(struct dp_ctrl *dptx);
int dptx_slice_height_limit(struct dp_ctrl *dptx, uint32_t pic_height);
int dptx_line_buffer_depth_limit(uint8_t line_buf_depth);
void dptx_dsc_notice_rx(struct dp_ctrl *dptx);
void dptx_dsc_cfg(struct dp_ctrl *dptx);
void dptx_dsc_sdp_manul_send(struct dp_ctrl *dptx, bool enable);
void dptx_dsc_dss_config(struct dpu_fb_data_type *dpufd);
#endif
