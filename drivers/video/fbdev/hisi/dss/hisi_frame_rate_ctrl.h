/*
 * hisi_frame_rate_ctrl.h
 *
 * The driver of dynamic frame rate
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#define FPS_60HZ 60
#define FPS_90HZ 90

void hisi_dfr_notice_handle_func(struct work_struct *work);
void mipi_dsi_frm_rate_ctrl_init(struct hisi_fb_data_type *hisifd);
struct mipi_panel_info *get_mipi_ctrl(struct hisi_fb_data_type *hisifd);

#ifdef CONFIG_HISI_DISPLAY_DFR
void dfr_power_on_notification(struct hisi_fb_data_type *hisifd);
void dfr_status_convert_on_isr_vstart(struct hisi_fb_data_type *hisifd);
bool need_update_frame_rate(struct hisi_fb_data_type *hisifd);
bool need_config_frame_rate_timing(struct hisi_fb_data_type *hisifd);
int mipi_dsi_frm_rate_ctrl(struct hisi_fb_data_type *hisifd, int frm_rate);
int mipi_dsi_frm_rate_para_set_reg(struct hisi_fb_data_type *hisifd);
#else
static inline void dfr_power_on_notification(struct hisi_fb_data_type *hisifd) {return;}
static inline void dfr_status_convert_on_isr_vstart(struct hisi_fb_data_type *hisifd) {return;}
static inline bool need_update_frame_rate(struct hisi_fb_data_type *hisifd) {return false;}
static inline bool need_config_frame_rate_timing(struct hisi_fb_data_type *hisifd) {return false;}
static inline int mipi_dsi_frm_rate_ctrl(struct hisi_fb_data_type *hisifd, int frm_rate) {return 0;}
static inline int mipi_dsi_frm_rate_para_set_reg(struct hisi_fb_data_type *hisifd) {return 0;}
#endif
#endif  /* HISI_FRME_RATE_CTRL_H */

