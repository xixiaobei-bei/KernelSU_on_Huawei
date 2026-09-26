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

#ifndef HISI_DSS_INIT_H
#define HISI_DSS_INIT_H

#include "../hisi_fb.h"

#define DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA 10000
#define DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC 300
#define DSS_UNDERFLOW_COUNT 1
#define ONLINE_PLAY_BYPASS_MAX_COUNT 20

enum ENUM_LDI_VSTATE {
	LDI_VSTATE_IDLE = 0x1,
	LDI_VSTATE_VSW = 0x2,
	LDI_VSTATE_VBP = 0x4,
	LDI_VSTATE_VACTIVE0 = 0x8,
	LDI_VSTATE_VACTIVE_SPACE = 0x10,
	LDI_VSTATE_VACTIVE1 = 0x20,
	LDI_VSTATE_VFP = 0x40,
	LDI_VSTATE_V_WAIT_TE0 = 0x80,
	LDI_VSTATE_V_WAIT_TE1 = 0x100,
	LDI_VSTATE_V_WAIT_TE_EN = 0x200,
	LDI_VSTATE_V_WAIT_GPU = 0x400,
};

int hisi_dss_module_init(struct dpu_fb_data_type *dpufd);
int hisi_dss_module_default(struct dpu_fb_data_type *dpufd);
int hisi_overlay_on(struct dpu_fb_data_type *dpufd, bool fastboot_enable);
int hisi_overlay_off(struct dpu_fb_data_type *dpufd);
int hisi_overlay_on_lp(struct dpu_fb_data_type *dpufd);
int hisi_overlay_off_lp(struct dpu_fb_data_type *dpufd);
int hisi_overlay_init(struct dpu_fb_data_type *dpufd);
int hisi_overlay_deinit(struct dpu_fb_data_type *dpufd);
int hisi_vactive0_start_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req);
void dpufb_dss_off(struct dpu_fb_data_type *dpufd, bool is_lp);
void dpufb_dss_overlay_info_init(dss_overlay_t *ov_req);


#endif /* HISI_DSS_INIT_H */
