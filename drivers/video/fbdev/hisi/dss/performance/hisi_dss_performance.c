/* Copyright (c) 2019-2019, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#include "hisi_fb.h"

#if defined(CONFIG_HISI_PERIDVFS)
#include "peri_volt_poll.h"
#endif

#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
int dss_get_peri_volt(int *curr_volt)
{
	struct peri_volt_poll *pvp = NULL;

	dpu_check_and_return(!curr_volt, -EINVAL, ERR, "nullptr!\n");
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	dpu_check_and_return((pvp == NULL), -EINVAL, ERR, "pvp get failed!\n");
	*curr_volt = peri_get_volt(pvp);

	return 0;
}

int dss_set_peri_volt(int volt_to_set, int *curr_volt)
{
	int ret = 0;

	struct peri_volt_poll *pvp = NULL;
	bool is_lowtemp = false;

	dpu_check_and_return(!curr_volt, -EINVAL, ERR, "nullptr!\n");
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	dpu_check_and_return((pvp == NULL), -EINVAL, ERR, "pvp get failed!\n");

	if (peri_get_temperature(pvp))
		is_lowtemp = true;

	ret = peri_set_volt(pvp, volt_to_set);
	dpu_check_and_return((ret != 0), -EINVAL, ERR,
		"set votage_value=%d failed, ret=%d\n", volt_to_set, ret);

	DPU_FB_DEBUG("set votage_value=%d\n", volt_to_set);

	*curr_volt = peri_get_volt(pvp);

	return ret;
}
#else
int dss_get_peri_volt(int *curr_volt)
{
	(void)curr_volt;

	return 0;
}

int dss_set_peri_volt(int volt_to_set, int *curr_volt)
{
	(void)volt_to_set;
	(void)curr_volt;

	return 0;
}
#endif

int dpufb_get_other_fb_votelevel(struct dpu_fb_data_type *dpufd,
	uint32_t *max_vote_level)
{
	struct dpu_fb_data_type *targetfd1 = NULL;
	struct dpu_fb_data_type *targetfd2 = NULL;
	uint32_t target_dss_voltage_level;

	dpu_check_and_return((!dpufd || !max_vote_level), -EINVAL, ERR, "null ptr\n");

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		targetfd1 = dpufd_list[EXTERNAL_PANEL_IDX];
		targetfd2 = dpufd_list[AUXILIARY_PANEL_IDX];
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		targetfd1 = dpufd_list[PRIMARY_PANEL_IDX];
		targetfd2 = dpufd_list[AUXILIARY_PANEL_IDX];
	} else {
		targetfd1 = dpufd_list[PRIMARY_PANEL_IDX];
		targetfd2 = dpufd_list[EXTERNAL_PANEL_IDX];
	}

	if (targetfd1 && targetfd2)
		target_dss_voltage_level = max(targetfd1->dss_vote_cmd.dss_voltage_level,
			targetfd2->dss_vote_cmd.dss_voltage_level);
	else if (targetfd1)
		target_dss_voltage_level = targetfd1->dss_vote_cmd.dss_voltage_level;
	else if (targetfd2)
		target_dss_voltage_level = targetfd2->dss_vote_cmd.dss_voltage_level;
	else
		target_dss_voltage_level = PERI_VOLTAGE_LEVEL0;

	*max_vote_level = target_dss_voltage_level;

	return 0;
}

int dpufb_set_dss_vote_voltage(struct dpu_fb_data_type *dpufd,
	uint32_t dss_voltage_level, int *curr_volt)
{
	int ret;
	int volt_to_set;
	uint32_t expected_voltage_level;
	struct dpu_fb_data_type *fb0 = dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_fb_data_type *fb1 = dpufd_list[EXTERNAL_PANEL_IDX];
	struct dpu_fb_data_type *fb2 = dpufd_list[AUXILIARY_PANEL_IDX];
	struct dpu_fb_data_type *fb3 = dpufd_list[MEDIACOMMON_PANEL_IDX];

	dpu_check_and_return((dpufd == NULL) || (curr_volt == NULL), -EINVAL,
		ERR, "null ptr\n");

	ret = dpufb_get_other_fb_votelevel(dpufd, &expected_voltage_level);
	dpu_check_and_return((ret != 0), -EINVAL, ERR,
		"set max votage_value=%d failed\n", ret);

	expected_voltage_level = max(expected_voltage_level, dss_voltage_level);

	volt_to_set = dpe_get_voltage_value(dpufd, expected_voltage_level);
	dpu_check_and_return((volt_to_set < 0), -EINVAL, ERR, "get votage_value failed\n");

	dpu_check_and_return(dss_set_peri_volt(volt_to_set, curr_volt), -EINVAL, ERR,
		"dss_set_peri_volt %d failed\n", volt_to_set);

	DPU_FB_DEBUG("fb%d->level %d, [%d, %d, %d, %d], set %d, current %d\n",
		dpufd->index, dss_voltage_level,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
		(fb3 != NULL) ? fb3->dss_vote_cmd.dss_voltage_level : 0,
		volt_to_set, *curr_volt);

	dpufd->dss_vote_cmd.dss_voltage_level = dss_voltage_level;

	return ret;
}

uint64_t dss_get_current_pri_clk_rate(struct dpu_fb_data_type *dpufd)
{
	uint64_t dss_pri_clk_rate;

	dpu_check_and_return((!dpufd), 0, ERR, "dpufd is NULL Pointer!\n");

	dss_pri_clk_rate = clk_get_rate(dpufd->dss_pri_clk);
	DPU_FB_INFO("fb%d get dss_pri_clk = %llu\n", dpufd->index, dss_pri_clk_rate);

	return dss_pri_clk_rate;
}

static bool is_single_dsc_type(struct dpu_panel_info *panel_info)
{
	if (((panel_info->ifbc_type == IFBC_MODE_VESA3X_SINGLE) ||
		(panel_info->ifbc_type == IFBC_MODE_VESA3_75X_SINGLE)))
		return true;
	return false;
}

uint64_t calc_core_rate(struct dpu_fb_data_type *dpufd, uint32_t porch_ratio, uint32_t fps)
{
	struct dpu_panel_info *panel_info = NULL;
	uint32_t core_rate;
	int num_pixel;

	dpu_check_and_return((dpufd == NULL), 0, ERR, "null ptr\n");

	panel_info = &(dpufd->panel_info);
	dpu_check_and_return((panel_info == NULL), 0, ERR, "panel_info null ptr\n");
	/* for a single dsc type, the core clock is divided by 1.
	 * otherwise, the core clock is divided by 2
	 */
	num_pixel = is_single_dsc_type(panel_info) ? 1 : 2;

	/* The calculation formula refers to Kirin990Perf::RequestBoostFpsCoreRate in hwc,
	 * because the kernel does not support float calculation, the transformed calculation formula is as follows:
	 */
	core_rate = (panel_info->xres) * (panel_info->yres) / 100 * fps * (100 + porch_ratio);
	core_rate = core_rate / num_pixel;
	core_rate = core_rate / 10 * 11; // core clk margin 1.1

	DPU_FB_DEBUG("fpsCoreRate=%u, refreshWidth=%d, refreshHeight=%d, mPorchRatio=%d, mFps=%d, numPixel=%d\n",
		core_rate, panel_info->xres, panel_info->yres, porch_ratio, fps, num_pixel);

	return core_rate;
}
