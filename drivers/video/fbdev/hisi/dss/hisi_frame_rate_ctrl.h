/*
 * hisi_frame_rate_ctrl.h
 *
 * The driver of dynamic frame rate
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */
#ifndef HISI_FRME_RATE_CTRL_H
#define HISI_FRME_RATE_CTRL_H

#include "hisi_fb.h"
#include <stdbool.h>

/* constraint between dfr and other features
 * BIT(0) - not support mipi clk update in hight frame rate
 * BIT(1-31) - reserved
 */
#define DFR_SUPPORT_MIPI_CLK_UPT_ONLY_60HZ BIT(0)

void mipi_dsi_frm_rate_ctrl_init(struct dpu_fb_data_type *dpufd);
struct mipi_panel_info *get_mipi_ctrl(struct dpu_fb_data_type *dpufd);
int get_dfr_constraint(struct dpu_fb_data_type *dpufd);
void dfr_power_on_notification(struct dpu_fb_data_type *dpufd);
void dfr_status_convert_on_isr_vstart(struct dpu_fb_data_type *dpufd);
bool need_update_frame_rate(struct dpu_fb_data_type *dpufd);
bool need_config_frame_rate_timing(struct dpu_fb_data_type *dpufd);
int mipi_dsi_frm_rate_ctrl(struct dpu_fb_data_type *dpufd, int frm_rate);
int mipi_dsi_frm_rate_para_set_reg(struct dpu_fb_data_type *dpufd);
uint32_t get_porch_ratio(struct dpu_fb_data_type *dpufd);
#endif  /* HISI_FRME_RATE_CTRL_H */

