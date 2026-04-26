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

#define PERI_VOLTAGE_LEVEL0_060V 0 // 0.60v
#define PERI_VOLTAGE_LEVEL1_065V 1 // 0.65v
#define PERI_VOLTAGE_LEVEL2_070V 2 // 0.70v
#define PERI_VOLTAGE_LEVEL3_080V 3 // 0.80v

#define LCD_4K_XRES 2160
#define LCD_4K_YRES 3840

bool is_vote_needed_for_low_temp(bool is_lowtemp, int volt_to_set)
{
	if (is_lowtemp && (volt_to_set == PERI_VOLTAGE_LEVEL3_080V)) {
		DPU_FB_INFO("is_lowtemp, vlotage cannot exceed 0.7v\n");
		return false;
	}
	return true;
}

int dpe_get_voltage_value(uint32_t dss_voltage_level)
{
	switch (dss_voltage_level) {
	case PERI_VOLTAGE_LEVEL0:
		return PERI_VOLTAGE_LEVEL0_060V;
	case PERI_VOLTAGE_LEVEL1:
		return PERI_VOLTAGE_LEVEL1_065V;
	case PERI_VOLTAGE_LEVEL2:
		return PERI_VOLTAGE_LEVEL2_070V;
	case PERI_VOLTAGE_LEVEL3:
		return PERI_VOLTAGE_LEVEL3_080V;
	default:
		DPU_FB_ERR("not support dss_voltage_level is %d\n",
			dss_voltage_level);
		return -1;
	}
}

int dpe_get_voltage_level(int votage_value)
{
	switch (votage_value) {
	case PERI_VOLTAGE_LEVEL0_060V:
		return PERI_VOLTAGE_LEVEL0;
	case PERI_VOLTAGE_LEVEL1_065V:
		return PERI_VOLTAGE_LEVEL1;
	case PERI_VOLTAGE_LEVEL2_070V:
		return PERI_VOLTAGE_LEVEL2;
	case PERI_VOLTAGE_LEVEL3_080V:
		return PERI_VOLTAGE_LEVEL3;
	default:
		DPU_FB_ERR("not support votage_value is %d\n", votage_value);
		return PERI_VOLTAGE_LEVEL0;
	}
}

static int get_mdc_clk_rate(dss_vote_cmd_t vote_cmd, uint64_t *clk_rate)
{
	switch (vote_cmd.dss_voltage_level) {
	case PERI_VOLTAGE_LEVEL0:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L1;
		break;
	case PERI_VOLTAGE_LEVEL1:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL2:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL3:
		*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L4;
		break;
	default:
		DPU_FB_ERR("no support set dss_voltage_level %d!\n",
			vote_cmd.dss_voltage_level);
		return -1;
	}
	DPU_FB_DEBUG("get mdc clk rate: %llu\n", *clk_rate);
	return 0;
}

int set_mdc_core_clk(struct dpu_fb_data_type *dpufd, dss_vote_cmd_t vote_cmd)
{
	int ret;
	uint64_t clk_rate = 0;

	dpu_check_and_return(!dpufd, -1, ERR, "dpufd is NULL\n");
	if (vote_cmd.dss_voltage_level == dpufd->dss_vote_cmd.dss_voltage_level)
		return 0;

	if (get_mdc_clk_rate(vote_cmd, &clk_rate)) {
		DPU_FB_ERR("get mdc clk rate failed!\n");
		return -1;
	}

	DPU_FB_INFO_INTERIM("Trying to set media_common_clk from %llu to %llu\n",
		clk_get_rate(dpufd->dss_clk_media_common_clk), clk_rate);

	ret = clk_set_rate(dpufd->dss_clk_media_common_clk, clk_rate);
	dpu_check_and_return((ret < 0), -1, ERR,
		"set dss_clk_media_common_clk(%llu) failed, error=%d!\n", clk_rate, ret);

	dpufd->dss_vote_cmd.dss_voltage_level = vote_cmd.dss_voltage_level;

	DPU_FB_INFO("set dss_clk_media_common_clk = %llu\n", clk_rate);

	return ret;
}

static uint32_t get_clk_vote_level(dss_vote_cmd_t vote_cmd)
{
	uint32_t level = PERI_VOLTAGE_LEVEL0;

	switch (vote_cmd.dss_pri_clk_rate) {
	case DEFAULT_DSS_CORE_CLK_RATE_L4:
		level = PERI_VOLTAGE_LEVEL3;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L3:
		level = PERI_VOLTAGE_LEVEL2;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L2:
		level = PERI_VOLTAGE_LEVEL1;
		break;
	case DEFAULT_DSS_CORE_CLK_RATE_L1:
		level = PERI_VOLTAGE_LEVEL0;
		break;
	default:
		DPU_FB_ERR("wrong pri clk rate %llu\n", vote_cmd.dss_pri_clk_rate);
		break;
	}

	return level;
}

static void dpufb_get_other_fb_clk_vote(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t *other_fb_clk_rate)
{
	struct dpu_fb_data_type *targetfd1 = NULL;
	struct dpu_fb_data_type *targetfd2 = NULL;

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

	if (targetfd1 && targetfd2) {
		other_fb_clk_rate->dss_pri_clk_rate = max(targetfd1->dss_vote_cmd.dss_pri_clk_rate,
			targetfd2->dss_vote_cmd.dss_pri_clk_rate);
		other_fb_clk_rate->dss_mmbuf_rate = max(targetfd1->dss_vote_cmd.dss_mmbuf_rate,
			 targetfd2->dss_vote_cmd.dss_mmbuf_rate);
	} else if (targetfd1) {
		other_fb_clk_rate->dss_pri_clk_rate = targetfd1->dss_vote_cmd.dss_pri_clk_rate;
		other_fb_clk_rate->dss_mmbuf_rate = targetfd1->dss_vote_cmd.dss_mmbuf_rate;
	} else if (targetfd2) {
		other_fb_clk_rate->dss_pri_clk_rate = targetfd2->dss_vote_cmd.dss_pri_clk_rate;
		other_fb_clk_rate->dss_mmbuf_rate = targetfd2->dss_vote_cmd.dss_mmbuf_rate;
	} else {
		return;
	}
}

static int get_dss_clock_value(uint32_t voltage_level, dss_vote_cmd_t *dss_vote_cmd)
{
	if (dss_vote_cmd == NULL) {
		DPU_FB_ERR("dss_vote_cmd is NULL point!\n");
		return -EINVAL;
	}

	switch (voltage_level) {
	case PERI_VOLTAGE_LEVEL3:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		break;
	case PERI_VOLTAGE_LEVEL2:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL1:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL0:
		dss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		dss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		break;
	default:
		DPU_FB_ERR("wrong voltage value %d\n", voltage_level);
		return -EINVAL;
	}

	return 0;
}

#ifdef SUPPORT_DCA_WITHOUT_VOLT
bool is_clk_voltage_matching(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t target_vote)
{
	(void)dpufd;
	(void)target_vote;

	return true;
}
#else
/*lint -emacro(529,*) */
bool is_clk_voltage_matching(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t target_vote)
{
	dpu_check_and_return(!dpufd, false, ERR, "dpufd is NULL\n");

	int current_peri_voltage = 0;
	struct dss_vote_cmd supportable_cmd;
	struct dpu_fb_data_type *fb0 = dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_fb_data_type *fb1 = dpufd_list[EXTERNAL_PANEL_IDX];
	struct dpu_fb_data_type *fb2 = dpufd_list[AUXILIARY_PANEL_IDX];

	memset(&supportable_cmd, 0, sizeof(supportable_cmd));

	(void)dss_get_peri_volt(&current_peri_voltage);
	if (get_dss_clock_value(dpe_get_voltage_level(current_peri_voltage),
		&supportable_cmd)) {
		return false;
	}
	if ((target_vote.dss_pri_clk_rate > supportable_cmd.dss_pri_clk_rate) ||
		(target_vote.dss_mmbuf_rate > supportable_cmd.dss_mmbuf_rate)) {
		DPU_FB_ERR("fb%d, current voltage %d does not match clk rate %llu, %llu\n",
			dpufd->index, current_peri_voltage,
			target_vote.dss_pri_clk_rate,
			target_vote.dss_mmbuf_rate);

		if ((fb0 != NULL) && fb0->panel_power_on)
			DPU_FB_INFO("CLKDIV2 0x%x, PERI_CTRL4 & CTRL5 = 0x%x & 0x%x\n",
				inp32(fb0->media_crg_base + MEDIA_CLKDIV1 + 0x4),
				inp32(fb0->pmctrl_base + PMCTRL_PERI_CTRL4),
				inp32(fb0->pmctrl_base + PMCTRL_PERI_CTRL5));

		DPU_FB_INFO("volt: %d, %d, %d, edcClk: %llu, %llu, %llu\n",
			(fb0 != NULL) ? fb0->dss_vote_cmd.dss_voltage_level : 0,
			(fb1 != NULL) ? fb1->dss_vote_cmd.dss_voltage_level : 0,
			(fb2 != NULL) ? fb2->dss_vote_cmd.dss_voltage_level : 0,
			(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
			(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
			(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0);

		return false;
	}

	return true;
}
/*lint +emacro(529,*) */
#endif

static int dss_core_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret;

	if (dpufd->dss_pri_clk != NULL) {
		ret = clk_prepare_enable(dpufd->dss_pri_clk);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_pri_clk clk_enable failed, error=%d!\n", dpufd->index, ret);
	}

	if (dpufd->dss_axi_clk != NULL) {
		ret = clk_prepare_enable(dpufd->dss_axi_clk);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_axi_clk enable failed, error=%d!\n", dpufd->index, ret);
	}

	if (dpufd->dss_mmbuf_clk != NULL) {
		ret = clk_prepare_enable(dpufd->dss_mmbuf_clk);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_mmbuf_clk enable failed, error=%d!\n", dpufd->index, ret);
	}

	return 0;
}

static void dss_core_clk_disable(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->dss_pri_clk != NULL)
		clk_disable_unprepare(dpufd->dss_pri_clk);

	if (dpufd->dss_axi_clk != NULL)
		clk_disable_unprepare(dpufd->dss_axi_clk);

	if (dpufd->dss_mmbuf_clk != NULL)
		clk_disable_unprepare(dpufd->dss_mmbuf_clk);
}

int dpufb_set_edc_mmbuf_clk(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t target_vote)
{
	int ret = 0;

	dpu_check_and_return(!dpufd, -1, ERR, "dpufd is NULL\n");
	if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
		ret = dss_core_clk_enable(dpufd);
		dpu_check_and_return((ret < 0), -1, ERR,
			"dss_core_clk_enable failed, error=%d\n", ret);
	}

	ret = clk_set_rate(dpufd->dss_pri_clk, target_vote.dss_pri_clk_rate);
	dpu_check_and_return((ret < 0), -1, ERR, "set dss_pri_clk_rate %llu failed, error=%d\n",
		target_vote.dss_pri_clk_rate, ret);

	DPU_FB_DEBUG("Trying to set dss_mmbuf_clk from %llu to %llu\n",
		clk_get_rate(dpufd->dss_mmbuf_clk), target_vote.dss_mmbuf_rate);
	ret = clk_set_rate(dpufd->dss_mmbuf_clk, target_vote.dss_mmbuf_rate);
	dpu_check_and_return((ret < 0), -1, ERR,
		"set dss_mmbuf_rate %llu failed, error=%d\n",
		target_vote.dss_mmbuf_rate, ret);

	if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF)
		dss_core_clk_disable(dpufd);

	return ret;
}

static int dpufb_edc_mmbuf_clk_vote(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t dss_vote_cmd)
{
	dss_vote_cmd_t other_fb_clk_rate;
	dss_vote_cmd_t target_vote;

	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));
	memset(&target_vote, 0, sizeof(target_vote));

	dpufb_get_other_fb_clk_vote(dpufd, &other_fb_clk_rate);
	if (dss_vote_cmd.dss_pri_clk_rate >= other_fb_clk_rate.dss_pri_clk_rate) {
		target_vote.dss_pri_clk_rate = dss_vote_cmd.dss_pri_clk_rate;
		target_vote.dss_mmbuf_rate = dss_vote_cmd.dss_mmbuf_rate;
	} else {
		target_vote.dss_pri_clk_rate = other_fb_clk_rate.dss_pri_clk_rate;
		target_vote.dss_mmbuf_rate = other_fb_clk_rate.dss_mmbuf_rate;
	}

	if (!is_clk_voltage_matching(dpufd, target_vote))
		return -1;

	if (dpufb_set_edc_mmbuf_clk(dpufd, target_vote))
		return -1;

	DPU_FB_DEBUG("fb%d, pri clk vote %llu, set %llu, mmbuf clk vote %llu, set %llu, "
		"get coreClk = %llu mmbufClk = %llu\n",
		dpufd->index, dss_vote_cmd.dss_pri_clk_rate, target_vote.dss_pri_clk_rate,
		dss_vote_cmd.dss_mmbuf_rate, target_vote.dss_mmbuf_rate,
		(uint64_t)clk_get_rate(dpufd->dss_pri_clk),
		(uint64_t)clk_get_rate(dpufd->dss_mmbuf_clk));

	return 0;
}

#ifdef NEED_SWITCH_TEMP_CORE_CLK
void set_dss_temp_clk(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t vote_cmd)
{
	int ret;

	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return;
	}

	/* if dss pri clk rate set from one of the value 325M/406M/541M to 214M
	 * we need first adjust to a temp clk rate in order to avoid high freq for an instance
	 */
	if ((vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L1)
		&& ((dpufd->dss_vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L2) ||
			(dpufd->dss_vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L3) ||
			(dpufd->dss_vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L4))) {
		vote_cmd.dss_pri_clk_rate = TEMP_DSS_CORE_CLK_RATE;
		ret = dpufb_edc_mmbuf_clk_vote(dpufd, vote_cmd);
		if (ret < 0) {
			DPU_FB_ERR("set dss_pri_clk_rate to temp %llu failed, error = %d\n", vote_cmd.dss_pri_clk_rate, ret);
		} else {
			DPU_FB_DEBUG("set dss_pri_clk_rate to temp %llu succ\n", vote_cmd.dss_pri_clk_rate);
		}
	}
}
#endif

static int check_pri_clk_rate(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t vote_cmd)
{
	if ((vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L2)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L3)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L4)) {
		DPU_FB_ERR("fb%d no support dss_pri_clk_rate %llu\n",
			dpufd->index, vote_cmd.dss_pri_clk_rate);
		return -EINVAL;
	}
	return 0;
}

static int check_mmbuf_clk_rate(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t vote_cmd)
{
	if ((vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L1)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L2)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L3)
		&& (vote_cmd.dss_mmbuf_rate != DEFAULT_DSS_MMBUF_CLK_RATE_L4)) {
		DPU_FB_ERR("fb%d no support dss_mmbuf_rate %llu\n",
			dpufd->index, vote_cmd.dss_pri_clk_rate);
		return -EINVAL;
	}
	return 0;
}

int set_dss_vote_cmd(struct dpu_fb_data_type *dpufd,
	dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
	struct dpu_fb_data_type *fb0 = dpufd_list[PRIMARY_PANEL_IDX];
	struct dpu_fb_data_type *fb1 = dpufd_list[EXTERNAL_PANEL_IDX];
	struct dpu_fb_data_type *fb2 = dpufd_list[AUXILIARY_PANEL_IDX];

	dpu_check_and_return((dpufd == NULL), -1, ERR, "dpufd is NULL!\n");

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		/* clk_media_common's voltage following the frequency */
		return set_mdc_core_clk(dpufd, vote_cmd);

	if (check_pri_clk_rate(dpufd, vote_cmd) != 0)
		return -EINVAL;

	if (check_mmbuf_clk_rate(dpufd, vote_cmd) != 0)
		return -EINVAL;

	if ((dpufd->index != AUXILIARY_PANEL_IDX) &&
		(vote_cmd.dss_pri_clk_rate == dpufd->dss_vote_cmd.dss_pri_clk_rate) &&
		(vote_cmd.dss_mmbuf_rate == dpufd->dss_vote_cmd.dss_mmbuf_rate)) {
		return ret;
	}

	if (!check_primary_panel_power_status(dpufd))
		return -EINVAL;

	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		dpufd->clk_vote_level = get_clk_vote_level(vote_cmd);
		DPU_FB_DEBUG("fb%d, get clk_vote_level = %d\n",
			dpufd->index, dpufd->clk_vote_level);
		return ret;
	}

#ifdef NEED_SWITCH_TEMP_CORE_CLK
	set_dss_temp_clk(dpufd, vote_cmd);
#endif

	ret = dpufb_edc_mmbuf_clk_vote(dpufd, vote_cmd);
	if (ret == 0) {
		dpufd->dss_vote_cmd.dss_pri_clk_rate = vote_cmd.dss_pri_clk_rate;
		dpufd->dss_vote_cmd.dss_axi_clk_rate = vote_cmd.dss_axi_clk_rate;
		dpufd->dss_vote_cmd.dss_mmbuf_rate = vote_cmd.dss_mmbuf_rate;
	}

	DPU_FB_DEBUG("edc: %llu, %llu, %llu, mmbuf: %llu, %llu, %llu\n",
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_pri_clk_rate : 0,
		(fb0 != NULL) ? fb0->dss_vote_cmd.dss_mmbuf_rate : 0,
		(fb1 != NULL) ? fb1->dss_vote_cmd.dss_mmbuf_rate : 0,
		(fb2 != NULL) ? fb2->dss_vote_cmd.dss_mmbuf_rate : 0);

	return ret;
}


static void hisi_recovery_external_clk_rate(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index == EXTERNAL_PANEL_IDX) {
		dpufd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		dpufd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		dpufd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
		(void)dpufb_edc_mmbuf_clk_vote(dpufd, dpufd->dss_vote_cmd);
	}
}

int dpe_set_pixel_clk_rate_on_pll0(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	uint64_t clk_rate;
#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
	struct peri_volt_poll *pvp = NULL;
#endif

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL Pointer!\n");
		return -EINVAL;
	}

	if (is_dp_panel(dpufd)) {
		hisi_recovery_external_clk_rate(dpufd);

		clk_rate = DEFAULT_DSS_PXL1_CLK_RATE_POWER_OFF;
		ret = clk_set_rate(dpufd->dss_pxl1_clk, clk_rate);
		if (ret < 0)
			DPU_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				dpufd->index, clk_rate, ret);

		DPU_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl1_clk));

#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
		pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
		dpu_check_and_return((pvp == NULL), -EINVAL, ERR, "get pvp failed!\n");

		ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "set voltage_value=0 failed!\n");
#endif
	}

	return ret;
}

static void set_dss_perf_level_debug(struct dpu_fb_data_type *dpufd)
{
	struct dss_vote_cmd *pdss_vote_cmd = &(dpufd->dss_vote_cmd);

	if (g_dss_perf_debug <= 0)
		return;

	dpufd->core_clk_upt_support = 0;

	switch (g_dss_perf_debug - 1) {
	case PERI_VOLTAGE_LEVEL0:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		break;
	case PERI_VOLTAGE_LEVEL1:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		break;
	case PERI_VOLTAGE_LEVEL2:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		break;
	case PERI_VOLTAGE_LEVEL3:
		pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		break;
	default:
		break;
	}

	DPU_FB_INFO("set edc %llu, mmbuf %llu",
		pdss_vote_cmd->dss_pri_clk_rate, pdss_vote_cmd->dss_mmbuf_rate);
}

static uint32_t get_external_performance_level(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	uint64_t pxl_clk_rate;
	uint32_t perf_level = EXTERNAL_PERFORMANCE_LEVEL0;

	pinfo = &(dpufd->panel_info);
	pxl_clk_rate = (is_dp_panel(dpufd) && dpufd->dp_get_pxl_clk_rate) ?
		dpufd->dp_get_pxl_clk_rate(&(dpufd->dp)) : pinfo->pxl_clk_rate * 2;

	if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L3)
		perf_level = EXTERNAL_PERFORMANCE_LEVEL3;
	else if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L2)
		perf_level = EXTERNAL_PERFORMANCE_LEVEL2;
	else if (pxl_clk_rate > DEFAULT_DSS_PXL1_CLK_RATE_L1)
		perf_level = EXTERNAL_PERFORMANCE_LEVEL1;
	else
		perf_level = EXTERNAL_PERFORMANCE_LEVEL0;

	/* dp 4k set max performance level */
	if (is_dp_panel(dpufd) &&
		(pinfo->xres * pinfo->yres) >= (LCD_4K_XRES * LCD_4K_YRES))
		perf_level = EXTERNAL_PERFORMANCE_LEVEL3;

	return perf_level;
}

static void get_mmbuf_clk_rate(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	uint32_t perf_level;

	pinfo = &(dpufd->panel_info);
	pdss_vote_cmd = &(dpufd->dss_vote_cmd);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (pdss_vote_cmd->dss_mmbuf_rate < DEFAULT_DSS_MMBUF_CLK_RATE_L1)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		perf_level = get_external_performance_level(dpufd);
		if (perf_level == EXTERNAL_PERFORMANCE_LEVEL3)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		else if (perf_level == EXTERNAL_PERFORMANCE_LEVEL2)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		else if (perf_level == EXTERNAL_PERFORMANCE_LEVEL1)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		else
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	} else {
		if (pdss_vote_cmd->dss_mmbuf_rate == 0)
			pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}
	set_dss_perf_level_debug(dpufd);
}

static int get_lcd_frame_rate(struct dpu_panel_info *pinfo)
{
	uint32_t h_porch = pinfo->ldi.h_back_porch +
		pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width;

	uint32_t v_porch = pinfo->ldi.v_back_porch +
		pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width;

	return pinfo->pxl_clk_rate /
		(pinfo->xres + pinfo->pxl_clk_rate_div * h_porch) /
		(pinfo->yres + v_porch);
}

static uint32_t get_current_porch_ratio(struct dpu_fb_data_type *dpufd)
{
	uint32_t porch_ratio;
	uint32_t real_area;
	uint32_t total_area;
	struct mipi_panel_info *mipi = &(dpufd->panel_info.mipi);

	real_area = dpufd->panel_info.yres;
	total_area = dpufd->panel_info.yres + mipi->vsa + mipi->vbp + mipi->vfp;
	porch_ratio = (total_area - real_area) * 100 / total_area;
	return porch_ratio;
}

static uint64_t adjust_default_pri_clk_rate(struct dpu_fb_data_type *dpufd, uint64_t dss_pri_clk_rate)
{
	uint32_t porch_ratio;
	uint64_t single_layer_clk;

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return dss_pri_clk_rate;

	porch_ratio = get_current_porch_ratio(dpufd);
	single_layer_clk = calc_core_rate(dpufd, porch_ratio, dpufd->panel_info.fps);

	if (single_layer_clk > DEFAULT_DSS_CORE_CLK_RATE_L3)
		single_layer_clk = DEFAULT_DSS_CORE_CLK_RATE_L4;
	else if (single_layer_clk > DEFAULT_DSS_CORE_CLK_RATE_L2)
		single_layer_clk = DEFAULT_DSS_CORE_CLK_RATE_L3;
	else if (single_layer_clk > DEFAULT_DSS_CORE_CLK_RATE_L1)
		single_layer_clk = DEFAULT_DSS_CORE_CLK_RATE_L2;
	else
		single_layer_clk = DEFAULT_DSS_CORE_CLK_RATE_L1;
	DPU_FB_INFO("adjust boot core rate to %llu, fps: %d, porch_ratio: %u, dss_pri_clk_rate: %llu\n",
		single_layer_clk, dpufd->panel_info.fps, porch_ratio, dss_pri_clk_rate);
	return max(single_layer_clk, dss_pri_clk_rate);
}

struct dss_vote_cmd *get_dss_vote_cmd(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null.\n");
		return NULL;
	}

	pinfo = &(dpufd->panel_info);
	pdss_vote_cmd = &(dpufd->dss_vote_cmd);
	frame_rate = get_lcd_frame_rate(pinfo);

	if (g_fpga_flag == 1) {
		if (pdss_vote_cmd->dss_pri_clk_rate == 0)
			pdss_vote_cmd->dss_pri_clk_rate = 40 * 1000000UL;

		return pdss_vote_cmd;
	}

	if (pdss_vote_cmd->dss_pri_clk_rate == 0) {
		if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
			dpufd->core_clk_upt_support = 0;
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
			if (frame_rate >= 110) { // 110Hz is inherited param
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
				dpufd->core_clk_upt_support = 0;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
				dpufd->core_clk_upt_support = 1;
			}
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			dpufd->core_clk_upt_support = 1;
		} else {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			dpufd->core_clk_upt_support = 1;
		}
		pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
	}
	pdss_vote_cmd->dss_pri_clk_rate = adjust_default_pri_clk_rate(dpufd, pdss_vote_cmd->dss_pri_clk_rate);

	if (dpufd->index == EXTERNAL_PANEL_IDX)
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;

	set_dss_perf_level_debug(dpufd);

	return pdss_vote_cmd;
}

int dpufb_set_mmbuf_clk_rate(struct dpu_fb_data_type *dpufd)
{
	int ret;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd other_fb_clk_rate;
	struct dss_vote_cmd vote_cmd_tmp;

	memset(&vote_cmd_tmp, 0, sizeof(vote_cmd_tmp));
	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	get_mmbuf_clk_rate(dpufd);

	pinfo = &(dpufd->panel_info);

	if ((dpufd->index == AUXILIARY_PANEL_IDX) &&
		dpufd_list[PRIMARY_PANEL_IDX]->panel_power_on)
		return 0;

	down(&g_dpufb_dss_clk_vote_sem);
	pdss_vote_cmd = &(dpufd->dss_vote_cmd);
	vote_cmd_tmp.dss_mmbuf_rate = pdss_vote_cmd->dss_mmbuf_rate;

	dpufb_get_other_fb_clk_vote(dpufd, &other_fb_clk_rate);

	vote_cmd_tmp.dss_mmbuf_rate = max(vote_cmd_tmp.dss_mmbuf_rate,
		other_fb_clk_rate.dss_mmbuf_rate);

	if (!is_clk_voltage_matching(dpufd, vote_cmd_tmp)) {
		DPU_FB_ERR("clk voltage does not match\n");
		vote_cmd_tmp.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}

	ret = clk_set_rate(dpufd->dss_mmbuf_clk, vote_cmd_tmp.dss_mmbuf_rate);
	if (ret < 0) {
		up(&g_dpufb_dss_clk_vote_sem);
		DPU_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
			dpufd->index, vote_cmd_tmp.dss_mmbuf_rate, ret);
		return -EINVAL;
	}

	if ((dpufd->index == PRIMARY_PANEL_IDX) ||
		(dpufd->index == EXTERNAL_PANEL_IDX))
		DPU_FB_INFO("fb%d mmbuf clk rate[%llu], set[%llu],get[%llu].\n",
			dpufd->index, pdss_vote_cmd->dss_mmbuf_rate,
			vote_cmd_tmp.dss_mmbuf_rate,
			(uint64_t)clk_get_rate(dpufd->dss_mmbuf_clk));

	up(&g_dpufb_dss_clk_vote_sem);
	return 0;
}

static int dpe_set_pxl_clk_rate(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo;
	int ret = 0;

	pinfo = &(dpufd->panel_info);
	if (is_dp_panel(dpufd)) {
		if (dpufd->dp_pxl_ppll7_init != NULL)
			ret = dpufd->dp_pxl_ppll7_init(dpufd, pinfo->pxl_clk_rate);
		else
			ret = clk_set_rate(dpufd->dss_pxl1_clk, pinfo->pxl_clk_rate);

		if (ret < 0) {
			DPU_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				dpufd->index, pinfo->pxl_clk_rate, ret);

			if (g_fpga_flag == 0)
				return -EINVAL;
		}
		DPU_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl1_clk));
	}

	return ret;
}

int dpe_set_clk_rate(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	dss_vote_cmd_t tmp;
	struct dss_vote_cmd other_fb_clk_rate;
	int ret;

	if (pdev == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	down(&g_dpufb_dss_clk_vote_sem);
	pdss_vote_cmd = get_dss_vote_cmd(dpufd);
	if (pdss_vote_cmd == NULL) {
		DPU_FB_ERR("NULL Pointer!\n");
		ret = -EINVAL;
		goto ERR_OUT;
	}
	memset(&tmp, 0, sizeof(tmp));
	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));

	tmp.dss_pri_clk_rate = pdss_vote_cmd->dss_pri_clk_rate;

	/* dss_pri_clk_rate */
	dpufb_get_other_fb_clk_vote(dpufd, &other_fb_clk_rate);

	tmp.dss_pri_clk_rate = max(tmp.dss_pri_clk_rate, other_fb_clk_rate.dss_pri_clk_rate);
	if (!is_clk_voltage_matching(dpufd, tmp)) {
		DPU_FB_ERR("clk voltage does not match\n");
		tmp.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
	}


	ret = clk_set_rate(dpufd->dss_pri_clk, tmp.dss_pri_clk_rate);
	if (ret < 0) {
		DPU_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n",
			dpufd->index, tmp.dss_pri_clk_rate, ret);
		ret = -EINVAL;
		goto ERR_OUT;
	}

	ret = dpe_set_pxl_clk_rate(dpufd);
	if (ret < 0) {
		DPU_FB_ERR("fb%d set pxl clk rate failed, error=%d!\n", dpufd->index, ret);
		ret = -EINVAL;
		goto ERR_OUT;
	}

	if ((dpufd->index == PRIMARY_PANEL_IDX) ||
		(dpufd->index == EXTERNAL_PANEL_IDX))
		DPU_FB_INFO("fb%d, dss_pri_clk set %llu get %llu\n", dpufd->index,
			tmp.dss_pri_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pri_clk));

ERR_OUT:

	up(&g_dpufb_dss_clk_vote_sem);
	return ret;
}

static void dpufb_set_default_pri_clk_rate(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null.\n");
		return;
	}

	pinfo = &(dpufd->panel_info);
	pdss_vote_cmd = &(dpufd->dss_vote_cmd);
	frame_rate = get_lcd_frame_rate(pinfo);

	if (g_fpga_flag == 1) {
		pdss_vote_cmd->dss_pri_clk_rate = 40 * 1000000UL;
	} else {
		if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
			if (frame_rate >= 110) // 110Hz is inherited param
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
			else
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		} else {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		}
		pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}
}

int dpe_set_common_clk_rate_on_pll0(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint64_t clk_rate;
#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
	struct peri_volt_poll *pvp = NULL;
#endif

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL Pointer!\n");

	if (g_fpga_flag == 1)
		return 0;

	down(&g_dpufb_dss_clk_vote_sem);
	clk_rate = DEFAULT_DSS_MMBUF_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(dpufd->dss_mmbuf_clk, clk_rate);
	if (ret < 0) {
		DPU_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
				dpufd->index, clk_rate, ret);
		ret = -EINVAL;
		goto ERR_OUT;
	}

	DPU_FB_INFO("dss_mmbuf_clk:[%llu]->[%llu]\n",
		clk_rate, (uint64_t)clk_get_rate(dpufd->dss_mmbuf_clk));

	clk_rate = DEFAULT_DSS_CORE_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(dpufd->dss_pri_clk, clk_rate);
	if (ret < 0) {
		DPU_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n",
			dpufd->index, clk_rate, ret);
		ret = -EINVAL;
		goto ERR_OUT;
	}

	DPU_FB_INFO("dss_pri_clk:[%llu]->[%llu]\n",
		clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pri_clk));
	dpufb_set_default_pri_clk_rate(dpufd_list[PRIMARY_PANEL_IDX]);
	dpufd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
	dpufd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	dpufd_list[AUXILIARY_PANEL_IDX]->clk_vote_level = PERI_VOLTAGE_LEVEL0;

#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (pvp == NULL) {
		DPU_FB_ERR("get pvp failed!\n");
		ret = -EINVAL;
		goto ERR_OUT;
	}

	ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V);
	if (ret != 0) {
		DPU_FB_ERR("set voltage_value=0 failed!\n");
		ret = -EINVAL;
		goto ERR_OUT;
	}

	dpufd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	dpufd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;

	DPU_FB_INFO("set dss_voltage_level=0!\n");
#endif

ERR_OUT:

	up(&g_dpufb_dss_clk_vote_sem);
	return ret;
}

int dpufb_restore_dss_voltage_clk_vote(
	struct dpu_fb_data_type *dpufd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
#ifndef SUPPORT_DCA_WITHOUT_VOLT
	int volt_to_set;
	int timeout = 10;
	int current_peri_voltage = 0;
#endif

	if (dpufd == NULL || dpufd->index != PRIMARY_PANEL_IDX)
		return 0;

	down(&g_dpufb_dss_clk_vote_sem);
	if ((dpufd->dss_vote_cmd.dss_voltage_level != 0) ||
		(dpufd->dss_vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1))
		DPU_FB_ERR("break restore, expect: %d, %llu, now: %d, %llu\n",
			vote_cmd.dss_voltage_level, vote_cmd.dss_pri_clk_rate,
			dpufd->dss_vote_cmd.dss_voltage_level, dpufd->dss_vote_cmd.dss_pri_clk_rate);

#ifndef SUPPORT_DCA_WITHOUT_VOLT
	if (vote_cmd.dss_voltage_level == 0)
		goto vote_out;

	volt_to_set = dpe_get_voltage_value(vote_cmd.dss_voltage_level);
	ret = dpufb_set_dss_vote_voltage(dpufd, vote_cmd.dss_voltage_level, &current_peri_voltage);
	if (ret < 0) {
		DPU_FB_ERR("set volt %d fail\n", vote_cmd.dss_voltage_level);
		goto vote_out;
	}

	while ((current_peri_voltage < volt_to_set) && (timeout > 0)) {
		msleep(1);
		(void)dss_get_peri_volt(&current_peri_voltage);
		timeout--;
	}

	if (timeout > 0) {
		ret = set_dss_vote_cmd(dpufd, vote_cmd);
		if (ret < 0) {
			DPU_FB_ERR("set clk %llu, %llu fail\n", vote_cmd.dss_pri_clk_rate, vote_cmd.dss_mmbuf_rate);
			goto vote_out;
		}
	} else {
		DPU_FB_ERR("timeout, current_peri_voltage %d, less than expect %d\n", current_peri_voltage, volt_to_set);
		goto vote_out;
	}
#else
	ret = set_dss_vote_cmd(dpufd, vote_cmd);
	if (ret < 0) {
		DPU_FB_ERR("set clk %llu, %llu fail\n", vote_cmd.dss_pri_clk_rate, vote_cmd.dss_mmbuf_rate);
		goto vote_out;
	}
#endif
	DPU_FB_INFO("restore vote: %d, %llu, %llu succ\n",dpufd->dss_vote_cmd.dss_voltage_level,
		dpufd->dss_vote_cmd.dss_pri_clk_rate, dpufd->dss_vote_cmd.dss_mmbuf_rate);

vote_out:
	up(&g_dpufb_dss_clk_vote_sem);
	return ret;
}

static void set_external_performance(struct dpu_fb_data_type *dpufd,
	uint64_t pixel_clock, uint32_t *voltage, dss_vote_cmd_t *vote_cmd)
{
	uint32_t perf_level;

	perf_level = get_external_performance_level(dpufd);

	if (perf_level == EXTERNAL_PERFORMANCE_LEVEL3) {
		dpufd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL3;
		*voltage = PERI_VOLTAGE_LEVEL3_080V;
		vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L4;
		vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L4;
		vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL3;
	} else if (perf_level == EXTERNAL_PERFORMANCE_LEVEL2) {
		dpufd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL2;
		*voltage = PERI_VOLTAGE_LEVEL2_070V;
		vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
		vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL2;
	} else if (perf_level == EXTERNAL_PERFORMANCE_LEVEL1) {
		dpufd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL1;
		*voltage = PERI_VOLTAGE_LEVEL1_065V;
		vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L2;
		vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
		vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL1;
	} else {
		dpufd->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
		*voltage = PERI_VOLTAGE_LEVEL0_060V;
		vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
		vote_cmd->dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	}
}

/*lint -emacro(1564,*) */
int dpufb_set_dss_external_vote_pre(struct dpu_fb_data_type *dpufd,
	uint64_t pixel_clock)
{
	int ret;
	uint32_t voltage = 0;
	dss_vote_cmd_t vote_cmd;
#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
	int voltage_timeout = 15;
	struct peri_volt_poll *pvp = NULL;
#endif

	dpu_check_and_return((dpufd == NULL), -1, ERR, "NULL Pointer!\n");

	dpu_check_and_return((pixel_clock > (ULLONG_MAX / 2)), -1, ERR,
		"pixel_clock will overflow!\n");

	if (dpufd->index != EXTERNAL_PANEL_IDX)
		return 0;

	memset(&vote_cmd, 0, sizeof(vote_cmd));

	set_external_performance(dpufd, pixel_clock, &voltage, &vote_cmd);

#if defined(CONFIG_HISI_PERIDVFS) && !defined(SUPPORT_DCA_WITHOUT_VOLT)
	pvp = peri_volt_poll_get(DEV_LDI1_VOLTAGE_ID, NULL);
	dpu_check_and_return((pvp == NULL), -EINVAL, ERR, "[DP] get pvp failed!\n");

	ret = peri_set_volt(pvp, voltage);
	dpu_check_and_return((ret != 0), -EINVAL, ERR, "[DP] set voltage_value failed!\n");

	while (voltage_timeout > 0) {
		mdelay(10);
		if (voltage <= peri_get_volt(pvp))
			break;
		voltage_timeout--;
	}

	dpu_check_and_return((voltage > peri_get_volt(pvp)), -EINVAL, ERR,
		"[DP] set voltage_value time out!! request volt %d, cur volt %d\n",
		voltage, peri_get_volt(pvp));
#endif
	/* pixel clock/2 need more than DSS core clk. In double pixel mode,
	 * pixel clock is half of previous.
	 * So it only need pixel clock more than DSS core clk.
	 */
	if ((voltage > PERI_VOLTAGE_LEVEL0_060V) &&
		((vote_cmd.dss_pri_clk_rate) > DEFAULT_DSS_CORE_CLK_RATE_L1)) {
		ret = set_dss_vote_cmd(dpufd, vote_cmd);
		if (ret < 0) {
			DPU_FB_ERR("[DP] DSS core clk set failed!!\n");
			return ret;
		}
	} else {
		dpufd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		dpufd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}
	return 0;
}
/*lint +emacro(1564,*) */

int dpufb_offline_vote_ctrl(struct dpu_fb_data_type *dpufd,
	bool offline_start)
{
	int ret = 0;
	struct dss_vote_cmd other_fb_clk_rate, copybit_clk_rate;

	dpu_check_and_return(!dpufd, -1, ERR, "dpufd is NULL\n");
	if (dpufd->index != AUXILIARY_PANEL_IDX)
		return -1;

	memset(&other_fb_clk_rate, 0, sizeof(other_fb_clk_rate));
	memset(&copybit_clk_rate, 0, sizeof(copybit_clk_rate));

	down(&g_dpufb_dss_clk_vote_sem);
	dpufb_get_other_fb_clk_vote(dpufd, &other_fb_clk_rate);

	if (offline_start) {
		ret = get_dss_clock_value(dpufd->clk_vote_level, &copybit_clk_rate);
		if (ret < 0) {
			DPU_FB_ERR("get_dss_clock_value fail, level = %d\n", dpufd->clk_vote_level);
			goto offline_vote_out;
		}

		if (copybit_clk_rate.dss_pri_clk_rate > other_fb_clk_rate.dss_pri_clk_rate) {
			if (!is_clk_voltage_matching(dpufd, copybit_clk_rate)) {
				ret = -1;
				DPU_FB_ERR("offline play vote mismatch\n");
				goto offline_vote_out;
			}

			ret = dpufb_set_edc_mmbuf_clk(dpufd, copybit_clk_rate);
			if (ret < 0) {
				DPU_FB_ERR("fb%d offline start set dss_pri_clk_rate %llu fail\n",
					dpufd->index, copybit_clk_rate.dss_pri_clk_rate);
				ret = -1;
				goto offline_vote_out;
			}
		}

		dpufd->dss_vote_cmd.dss_pri_clk_rate = copybit_clk_rate.dss_pri_clk_rate;
		dpufd->dss_vote_cmd.dss_mmbuf_rate = copybit_clk_rate.dss_mmbuf_rate;
		DPU_FB_DEBUG("fb%d offline start set dss_pri_clk_rate %llu\n",
			dpufd->index, dpufd->dss_vote_cmd.dss_pri_clk_rate);
	} else {
		/* offline play end, recover fb2 voltage&CoreClk to max of other fb */
		if (dpufd->dss_vote_cmd.dss_pri_clk_rate > other_fb_clk_rate.dss_pri_clk_rate) {
			ret = dpufb_set_edc_mmbuf_clk(dpufd, other_fb_clk_rate);
			if (ret < 0)
				DPU_FB_ERR("offline play end, set pri clk %llu fail\n",
					other_fb_clk_rate.dss_pri_clk_rate);
			else
				DPU_FB_DEBUG("fb%d offline end set dss_pri_clk_rate %llu\n",
					dpufd->index, other_fb_clk_rate.dss_pri_clk_rate);
		}

		dpufd->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		dpufd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
	}

offline_vote_out:
	up(&g_dpufb_dss_clk_vote_sem);
	return ret;
}
