/* Copyright (c) 2019-2020, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#ifndef HISI_MDC_DEV_H
#define HISI_MDC_DEV_H
#include <soc_mdc_interface.h>

#define WCH0_OFFSET 0x59000

typedef struct hisi_mdc_chn_res {
	int32_t chn_idx;
	uint32_t chn_addr_base;

	void (*aif_init)(dss_module_reg_t *mdc_module,
		char __iomem *addr_base, int32_t chn_idx);

	void (*mif_init)(dss_module_reg_t *mdc_module,
		char __iomem *addr_base, int32_t chn_idx);

	void (*mctl_init)(dss_module_reg_t *mdc_module,
		char __iomem *addr_base, int32_t chn_idx);

	void (*chn_init)(dss_module_reg_t *mdc_module,
		char __iomem *addr_base, int32_t chn_idx);
} hisi_mdc_chn_res_t;

int set_mdc_core_clk(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t vote_cmd);
int mdc_clk_regulator_enable(struct dpu_fb_data_type *dpufd);
void mdc_clk_regulator_disable(struct dpu_fb_data_type *dpufd);

void hisi_mdc_interrupt_config(struct dpu_fb_data_type *dpufd);
void hisi_mdc_interrupt_deconfig(struct dpu_fb_data_type *dpufd);
void hisi_mdc_mif_on(struct dpu_fb_data_type *dpufd);
void hisi_mdc_scl_coef_on(struct dpu_fb_data_type *dpufd, int coef_lut_idx);
void hisi_mdc_module_default(struct dpu_fb_data_type *dpufd);
void hisi_mdc_mctl_on(struct dpu_fb_data_type *dpufd);
int hisi_mdc_power_off(struct dpu_fb_data_type *dpufd);

void hisi_mdc_cmdlist_config_reset(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, uint32_t cmdlist_idxs);
int hisi_mediacommon_ioctl_handler(struct dpu_fb_data_type *dpufd,
	uint32_t cmd, const void __user *argp);
#endif
