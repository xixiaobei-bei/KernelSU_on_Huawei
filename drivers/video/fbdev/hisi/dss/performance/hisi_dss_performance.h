/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License version 2 and
* only version 2 as published by the Free Software Foundation.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
*/
#ifndef HISI_DSS_PERFORMANCE_H
#define HISI_DSS_PERFORMANCE_H

#include <stdbool.h>

enum PERI_VOLTAGE_LEVEL {
	PERI_VOLTAGE_LEVEL0 = 0x0,
	PERI_VOLTAGE_LEVEL1 = 0x1,
	PERI_VOLTAGE_LEVEL2 = 0x2,
	PERI_VOLTAGE_LEVEL3 = 0x3,
	PERI_VOLTAGE_LEVEL4 = 0x4,
	PERI_VOLTAGE_LEVEL5 = 0x5,
	PERI_VOLTAGE_MAXLEVEL = 0x7,
};

enum EXTERNAL_PERFORMANCE_LEVEL {
	EXTERNAL_PERFORMANCE_LEVEL0 = 0x0,
	EXTERNAL_PERFORMANCE_LEVEL1 = 0x1,
	EXTERNAL_PERFORMANCE_LEVEL2 = 0x2,
	EXTERNAL_PERFORMANCE_LEVEL3 = 0x3,
};

int dss_get_peri_volt(int *curr_volt);
int dss_set_peri_volt(int volt_to_set, int *curr_volt);
int dpe_get_voltage_value(uint32_t dss_voltage_level); /* get value by level */
int dpe_get_voltage_level(int votage_value); /* get level by value */
int dpufb_get_other_fb_votelevel(struct dpu_fb_data_type *dpufd,
	uint32_t *max_vote_level);
int dpufb_set_dss_vote_voltage(struct dpu_fb_data_type *dpufd,
	uint32_t dss_voltage_level, int *curr_volt);
bool is_clk_voltage_matching(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t target_vote);
int dpufb_set_edc_mmbuf_clk(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t target_vote);
int set_dss_vote_cmd(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t dss_vote_cmd);
int dpe_set_pixel_clk_rate_on_pll0(struct dpu_fb_data_type *dpufd);
struct dss_vote_cmd * get_dss_vote_cmd(struct dpu_fb_data_type *dpufd);
int dpufb_set_mmbuf_clk_rate(struct dpu_fb_data_type *dpufd);
int dpe_set_clk_rate(struct platform_device *pdev);
int dpe_set_common_clk_rate_on_pll0(struct dpu_fb_data_type *dpufd);
int dpufb_restore_dss_voltage_clk_vote(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t vote_cmd);
int dpufb_set_dss_external_vote_pre(struct dpu_fb_data_type *dpufd, uint64_t pixel_clock);
bool is_vote_needed_for_low_temp(bool is_lowtemp, int volt_to_set);
uint64_t dss_get_current_pri_clk_rate(struct dpu_fb_data_type *dpufd);
uint64_t calc_core_rate(struct dpu_fb_data_type *dpufd, uint32_t porch_ratio, uint32_t fps);
#ifdef ADJUST_MMBUF_BY_PRI_CLK
uint64_t dpufb_get_mmbuf_clk_by_pri_clk(uint64_t dss_pri_clk_rate);
int dpufb_set_mmbuf_clk_value(struct dpu_fb_data_type *dpufd, uint64_t dss_mmbuf_clk_rate);
#endif
#endif
