/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

#include "hisi_dpe_utils.h"
#include "peri_volt_poll.h"


DEFINE_SEMAPHORE(hisi_fb_dss_inner_clk_sem);

static int dss_inner_clk_refcount = 0;
static unsigned  int g_comform_value = 0;
static unsigned  int g_acm_State = 0;
static unsigned  int g_gmp_State = 0;
static unsigned int g_led_rg_csc_value[9];
static unsigned int g_is_led_rg_csc_set;
unsigned int g_led_rg_para1 = 7;
unsigned int g_led_rg_para2 = 30983;
extern spinlock_t g_gmp_effect_lock;

#define OFFSET_FRACTIONAL_BITS	(11)
#define ROUND1(x,y)	((x) / (y) + ((x) % (y)  ? 1 : 0))
#define gmp_cnt_cofe (4913) // 17*17*17
#define xcc_cnt_cofe (12)

#define PERI_VOLTAGE_LEVEL0_060V		(0) // 0.60v
#define PERI_VOLTAGE_LEVEL1_065V		(1) // 0.65v
#define PERI_VOLTAGE_LEVEL2_070V		(2) // 0.70v
#define PERI_VOLTAGE_LEVEL3_080V		(3) // 0.80v

#define FPGA_DEFAULT_DSS_CORE_CLK_RATE_L1	(40000000UL)
#define FRAME_RATE_1440P	(110)



/*lint -e647*/
static int get_lcd_frame_rate(struct hisi_panel_info *pinfo)
{
	return pinfo->pxl_clk_rate/(pinfo->xres + pinfo->pxl_clk_rate_div *
		(pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width))/(pinfo->yres +
		pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width);
}
/*lint +e647*/
struct dss_vote_cmd * get_dss_vote_cmd(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return pdss_vote_cmd;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);

	frame_rate = get_lcd_frame_rate(pinfo);

	/* FIXME: TBD  */
	if (g_fpga_flag == 1) {
		if (pdss_vote_cmd->dss_pclk_dss_rate == 0) {
			pdss_vote_cmd->dss_pri_clk_rate = 40 * 1000000UL;
			pdss_vote_cmd->dss_pclk_dss_rate = 20 * 1000000UL;
			pdss_vote_cmd->dss_pclk_pctrl_rate = 20 * 1000000UL;
		}
	} else {
		if (pdss_vote_cmd->dss_pclk_dss_rate == 0) {
			if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
				pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
				pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
				hisifd->core_clk_upt_support = 0;
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
				if (frame_rate >= 110) {
					pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
					pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
					pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
					pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
					hisifd->core_clk_upt_support = 0;
				} else {
					pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE;
					pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
					pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
					pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
					hisifd->core_clk_upt_support = 1;
				}
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
				pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
				pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
				hisifd->core_clk_upt_support = 1;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
				pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
				pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
				hisifd->core_clk_upt_support = 1;
			}
		}
	}

	return pdss_vote_cmd;
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
		HISI_FB_ERR("no support dss_voltage_level(%d)! \n", vote_cmd.dss_voltage_level);
		return -1;
	}

	HISI_FB_DEBUG("get mdc clk rate: %llu \n", *clk_rate);
	return 0;
}

static int set_mdc_core_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret;
	uint64_t clk_rate = 0;

	if (vote_cmd.dss_voltage_level == hisifd->dss_vote_cmd.dss_voltage_level) {
		HISI_FB_INFO("vote dss_voltage_level is equal to current dss_voltage_level \n");
		return 0;
	}

	if (get_mdc_clk_rate(vote_cmd, &clk_rate)) {
		HISI_FB_ERR("get mdc clk rate failed! \n");
		return -1;
	}

	ret = clk_set_rate(hisifd->dss_clk_media_common_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("set dss_clk_media_common_clk(%llu) failed, error= %d \n", clk_rate, ret);
		return -1;
	}
	hisifd->dss_vote_cmd.dss_voltage_level = vote_cmd.dss_voltage_level;

	HISI_FB_DEBUG("set dss_clk_media_common_clk = %llu, dss_voltage_level = %d \n", clk_rate, vote_cmd.dss_voltage_level);

	return ret;
}

static int dss_core_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk != NULL) {
		ret = clk_prepare(hisifd->dss_pri_clk);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(hisifd->dss_pri_clk);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	if (hisifd->dss_axi_clk != NULL) {
		(void)clk_prepare_enable(hisifd->dss_axi_clk);
	}

	if (hisifd->dss_mmbuf_clk != NULL) {
		(void)clk_prepare_enable(hisifd->dss_mmbuf_clk);
	}

	return 0;
}

static int dss_core_clk_disable(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk != NULL) {
		clk_disable(hisifd->dss_pri_clk);
		clk_unprepare(hisifd->dss_pri_clk);
	}

	if (hisifd->dss_axi_clk != NULL) {
		clk_disable_unprepare(hisifd->dss_axi_clk);
	}

	if (hisifd->dss_mmbuf_clk != NULL) {
		clk_disable_unprepare(hisifd->dss_mmbuf_clk);
	}

	return 0;
}


static int set_primary_core_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t dss_vote_cmd)
{
	int ret = 0;
	struct hisi_fb_data_type *targetfd = NULL;
	bool set_rate_succ = true;

	targetfd = hisifd_list[AUXILIARY_PANEL_IDX];
	if (targetfd == NULL) {
		HISI_FB_ERR("fb2 is null\n");
		return -1;
	}

	if ((dss_vote_cmd.dss_pri_clk_rate >= targetfd->dss_vote_cmd.dss_pri_clk_rate)) {
		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
			ret = dss_core_clk_enable(hisifd);
			if (ret < 0) {
				HISI_FB_ERR("dss_core_clk_enable failed, error=%d!\n", ret);
				return -1;
			}
		}
		ret = clk_set_rate(hisifd->dss_pri_clk, dss_vote_cmd.dss_pri_clk_rate);
		if (ret < 0) {
			set_rate_succ = false;
			HISI_FB_ERR("set dss_pri_clk_rate(%llu) failed, error=%d!\n", dss_vote_cmd.dss_pri_clk_rate, ret);
		}

		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
			ret = dss_core_clk_disable(hisifd);
			if (ret < 0) {
				HISI_FB_ERR("dss_core_clk_disable, error=%d!\n", ret);
				return -1;
			}
		}

		HISI_FB_DEBUG("fb%d get dss_pri_clk = %llu.\n", hisifd->index, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
		HISI_FB_DEBUG("fb%d get dss_mmbuf_rate = %llu.\n", hisifd->index, (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));
		if (set_rate_succ) {
			hisifd->dss_vote_cmd.dss_pri_clk_rate = dss_vote_cmd.dss_pri_clk_rate;
			hisifd->dss_vote_cmd.dss_axi_clk_rate = dss_vote_cmd.dss_axi_clk_rate;
		} else {
			return -1;
		}
	}

	HISI_FB_DEBUG("set fb0 coreClk=%llu, fb2->coreClk=%llu.\n", dss_vote_cmd.dss_pri_clk_rate, targetfd->dss_vote_cmd.dss_pri_clk_rate);
	return ret;
}

int set_dss_vote_cmd(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
	struct hisi_fb_data_type *targetfd = NULL;
	uint64_t dssVoteCmd = vote_cmd.dss_pri_clk_rate; //recording the vote clk rate

	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		ret = set_mdc_core_clk(hisifd, vote_cmd);
		return ret;
	}

	if ((vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L2)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L3)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L4)) {
		HISI_FB_ERR("fb%d no support set dss_pri_clk_rate(%llu)! \n", hisifd->index, vote_cmd.dss_pri_clk_rate);
		return -1;
	}

	if (vote_cmd.dss_pri_clk_rate == hisifd->dss_vote_cmd.dss_pri_clk_rate) {
		HISI_FB_DEBUG("vote dss_pri_clk_rate is equal to current dss_pri_clk_rate! the rate is %llu\n", vote_cmd.dss_pri_clk_rate);
		return ret;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		// if dss pri clk rate set from 400M to one of the value 232M/320M/384M, we need first adjust to a temp clk rate in order to avoid high freq for an instance
		if (((vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L1)
			|| (vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L2)
			|| (vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L3))
			&& (hisifd->dss_vote_cmd.dss_pri_clk_rate == DEFAULT_DSS_CORE_CLK_RATE_L4)) {
			vote_cmd.dss_pri_clk_rate = TEMP_DSS_CORE_CLK_RATE;
			ret = set_primary_core_clk(hisifd, vote_cmd);
			if (ret < 0) {
				HISI_FB_ERR("set dss_pri_clk_rate(200) failed, error = %d! \n", ret);
			} else {
				HISI_FB_DEBUG("set dss_pri_clk_rate(200) succ! \n");
			}
		}

		vote_cmd.dss_pri_clk_rate = dssVoteCmd;
		ret = set_primary_core_clk(hisifd, vote_cmd);
		if (ret < 0) {
			HISI_FB_ERR("set dss_pri_clk_rate(%llu) failed, error = %d! \n", vote_cmd.dss_pri_clk_rate, ret);
		} else {
			HISI_FB_DEBUG("set dss_pri_clk_rate(%llu) succ! \n", vote_cmd.dss_pri_clk_rate);
		}

		return ret;
	}

	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		targetfd = hisifd_list[PRIMARY_PANEL_IDX];
		if (targetfd && (vote_cmd.dss_pri_clk_rate >= targetfd->dss_vote_cmd.dss_pri_clk_rate)) {
			hisifd->need_tuning_clk = true;
			HISI_FB_DEBUG("need tuning fb%d pri_clk_rate to %llu.\n",
				hisifd->index, vote_cmd.dss_pri_clk_rate);
		}

		hisifd->dss_vote_cmd.dss_pri_clk_rate = vote_cmd.dss_pri_clk_rate;
	}

	HISI_FB_DEBUG("set dss_pri_clk_rate = %llu.\n", vote_cmd.dss_pri_clk_rate);

	return ret;
}

int dpe_get_voltage_value(dss_vote_cmd_t *vote_cmd)
{
	if (vote_cmd == NULL) {
		HISI_FB_ERR("vote_cmd is null \n");
		return -1;
	}

	switch (vote_cmd->dss_voltage_level) {
	case PERI_VOLTAGE_LEVEL0:
		return PERI_VOLTAGE_LEVEL0_060V;
	case PERI_VOLTAGE_LEVEL1:
		return PERI_VOLTAGE_LEVEL1_065V;
	case PERI_VOLTAGE_LEVEL2:
		return PERI_VOLTAGE_LEVEL2_070V;
	case PERI_VOLTAGE_LEVEL3:
		return PERI_VOLTAGE_LEVEL3_080V;
	default:
		HISI_FB_ERR("not support dss_voltage_level is %d \n", vote_cmd->dss_voltage_level);
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
		HISI_FB_ERR("not support votage_value is %d \n", votage_value);
		return PERI_VOLTAGE_LEVEL0;
	}
}

/*lint -e712 -e838*/
int dpe_set_clk_rate(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct hisi_panel_info *pinfo;
	struct dss_vote_cmd *pdss_vote_cmd;
	uint64_t dss_pri_clk_rate;
	int ret = 0;

	if (pdev == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	hisifd = platform_get_drvdata(pdev);
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(hisifd);
	if (pdss_vote_cmd == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	//mmbuf_clk
	ret = clk_set_rate(hisifd->dss_mmbuf_clk, pdss_vote_cmd->dss_mmbuf_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
			hisifd->index, pdss_vote_cmd->dss_mmbuf_rate, ret);
		return -EINVAL;
	}


	//edc0_clk (pri_clk) (core_clk)
	dss_pri_clk_rate = pdss_vote_cmd->dss_pri_clk_rate;

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		if (hisifd_list[PRIMARY_PANEL_IDX]) {
			if (hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate > dss_pri_clk_rate) {
				dss_pri_clk_rate = hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate;
			}
		}
	}

	ret = clk_set_rate(hisifd->dss_pri_clk, dss_pri_clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n",
			hisifd->index, dss_pri_clk_rate, ret);
		return -EINVAL;
	}

	//ldi0 clk (pxl0_clk)
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ret = clk_set_rate(hisifd->dss_pxl0_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);
			if (g_fpga_flag == 0) {
				return -EINVAL;
			}
		}
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && !hisifd->panel_info.fake_external) {
		HISI_FB_ERR("not support.\n");
	} else {
		;
	}

	//print clk info
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("dss_mmbuf_clk:[%llu]->[%llu].\n",
			pdss_vote_cmd->dss_mmbuf_rate, (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));
		HISI_FB_INFO("dss_pri_clk:[%llu]->[%llu].\n",
			pdss_vote_cmd->dss_pri_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));
	}

	return ret;
}
/*lint +e712 +e838*/

int dpe_set_pixel_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	return 0;
}

static void hisifb_set_default_pri_clk_rate(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null\n");
		return;
	}

	pinfo = &(hisifd->panel_info); //lint !e838
	pdss_vote_cmd = &(hisifd->dss_vote_cmd); //lint !e838
	frame_rate = get_lcd_frame_rate(pinfo);

	if (g_fpga_flag == 1) {
		pdss_vote_cmd->dss_pri_clk_rate = FPGA_DEFAULT_DSS_CORE_CLK_RATE_L1;
	} else {
		if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
			if (frame_rate >= FRAME_RATE_1440P) {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L3;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
			}
		} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		} else {
			pdss_vote_cmd->dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
		}
	}
	return;
}

int dpe_set_common_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint64_t clk_rate;
	struct peri_volt_poll *pvp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL Pointer!\n");
		return -EINVAL;
	}

	if (g_fpga_flag == 1) {
		return 0;
	}

	clk_rate = DEFAULT_DSS_MMBUF_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(hisifd->dss_mmbuf_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
		return -EINVAL;
	}
	HISI_FB_INFO("dss_mmbuf_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));

	clk_rate = DEFAULT_DSS_CORE_CLK_RATE_POWER_OFF;
	ret = clk_set_rate(hisifd->dss_pri_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pri_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
		return -EINVAL;
	}
	HISI_FB_INFO("dss_pri_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_rate = DEFAULT_DSS_PXL0_CLK_RATE_POWER_OFF;
		ret = clk_set_rate(hisifd->dss_pxl0_clk, clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
			return -EINVAL;
		}
		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));

	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && !hisifd->panel_info.fake_external) {
		HISI_FB_ERR("not support.\n");
	} else {
		;
	}

	hisifb_set_default_pri_clk_rate(hisifd_list[PRIMARY_PANEL_IDX]);
	hisifd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL); //lint !e838
	if (pvp == NULL) {
		HISI_FB_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, PERI_VOLTAGE_LEVEL0_060V); // 0.6v
	if (ret) {
		HISI_FB_ERR("set voltage_value=0 failed!\n");
		return -EINVAL;
	}
	hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;
	hisifd_list[AUXILIARY_PANEL_IDX]->dss_vote_cmd.dss_voltage_level = PERI_VOLTAGE_LEVEL0;

	HISI_FB_INFO("set dss_voltage_level=0!\n");

	return ret;
}
/*lint +e712 +e838*/
/*lint -e838 */
void dss_inner_clk_common_enable(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *dss_base = NULL;
	int prev_refcount = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	down(&hisi_fb_dss_inner_clk_sem);

	prev_refcount = dss_inner_clk_refcount++;
	if (!prev_refcount && !fastboot_enable) {
		//core/axi/mmbuf
		outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);  //cmd mem

		if (g_fpga_flag == 1)
			outp32(dss_base + DSS_RCH_VG1_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000000);//rch_v1 ,arsr2p mem
		else
			outp32(dss_base + DSS_RCH_VG1_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);//rch_v1 ,arsr2p mem

		outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);//rch_v1 ,scf mem
		outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);//rch_v1 ,scf mem
		outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);//rch_v1 ,vpp mem
		outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_v1 ,dma_buf mem
		outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);//rch_v1 ,afbcd mem

		outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);//rch_g1 ,scf mem
		outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);//rch_g1 ,scf mem
		outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_g1 ,dma_buf mem
		outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);//rch_g1 ,afbcd mem

		outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_d0 ,dma_buf mem
		outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);//rch_d0 ,afbcd mem
		outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_d1 ,dma_buf mem
		outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);//rch_d1 ,afbcd mem
		outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_d2 ,dma_buf mem
		outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//rch_d3 ,dma_buf mem

		outp32(dss_base + DSS_WCH0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);//wch0 DMA/AFBCE mem
		outp32(dss_base + DSS_WCH0_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);//wch0 DMA/AFBCE mem
		outp32(dss_base + DSS_WCH0_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);//wch0 rot mem
	}

	HISI_FB_DEBUG("fb%d, dss_inner_clk_refcount=%d\n",
		hisifd->index, dss_inner_clk_refcount);

	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_common_disable(struct hisi_fb_data_type *hisifd)
{
	int new_refcount = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	down(&hisi_fb_dss_inner_clk_sem);
	new_refcount = --dss_inner_clk_refcount;
	if (new_refcount < 0) {
		HISI_FB_ERR("dss new_refcount err");
	}

	if (!new_refcount) {
		;
	}

	HISI_FB_DEBUG("fb%d, dss_inner_clk_refcount=%d\n",
		hisifd->index, dss_inner_clk_refcount);
	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_pdp_enable(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *dss_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	if (fastboot_enable)
		return ;

	outp32(dss_base + DSS_LDI0_OFFSET + LDI_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_POST_SCF_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_POST_SCF_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DPP_DITHER_OFFSET + DITHER_MEM_CTRL, 0x00000008);
}

void dss_inner_clk_pdp_disable(struct hisi_fb_data_type *hisifd)
{
}

void dss_inner_clk_sdp_enable(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	outp32(dss_base + DSS_LDI1_OFFSET + LDI_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);
}

void dss_inner_clk_sdp_disable(struct hisi_fb_data_type *hisifd)
{
}

void init_dpp(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dpp_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dpp_base = hisifd->dss_base + DSS_DPP_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	outp32(dpp_base + DPP_IMG_SIZE_BEF_SR, (DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));
	outp32(dpp_base + DPP_IMG_SIZE_AFT_SR, (DSS_HEIGHT(pinfo->yres) << 16) | DSS_WIDTH(pinfo->xres));

}
/*lint +e838 */

void init_sbl(struct hisi_fb_data_type *hisifd)
{
	return;
}

void init_ifbc(struct hisi_fb_data_type *hisifd)
{
	return;
}


/*lint -e438 -e550 -e838 -e573 -e647*/
void init_post_scf(struct hisi_fb_data_type *hisifd)
{
	char __iomem *scf_lut_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	scf_lut_base = hisifd->dss_base + DSS_POST_SCF_LUT_OFFSET;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF)) {
		return;
	}

	hisi_dss_post_scl_load_filter_coef(hisifd, false, scf_lut_base, SCL_COEF_RGB_IDX);

	return;
}

void init_dbuf(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dbuf_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	int sram_valid_num = 0;
	int sram_max_mem_depth = 0;
	int sram_min_support_depth = 0;

	uint32_t thd_rqos_in = 0;
	uint32_t thd_rqos_out = 0;
	uint32_t thd_wqos_in = 0;
	uint32_t thd_wqos_out = 0;
	uint32_t thd_cg_in = 0;
	uint32_t thd_cg_out = 0;
	uint32_t thd_wr_wait = 0;
	uint32_t thd_cg_hold = 0;
	uint32_t thd_flux_req_befdfs_in = 0;
	uint32_t thd_flux_req_befdfs_out = 0;
	uint32_t thd_flux_req_aftdfs_in = 0;
	uint32_t thd_flux_req_aftdfs_out = 0;
	uint32_t thd_dfs_ok = 0;
	uint32_t dfs_ok_mask = 0;
	uint32_t thd_flux_req_sw_en = 0;

	int dfs_time = 0;
	int dfs_time_min = 0;
	int depth = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dbuf_base = hisifd->dss_base + DSS_DBUF0_OFFSET;
		if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_DBUF)) {
			return;
		}

		if (pinfo->xres * pinfo->yres >= RES_4K_PHONE) {
			dfs_time_min = DFS_TIME_MIN_4K;
		} else {
			dfs_time_min = DFS_TIME_MIN;
		}

		dfs_time = DFS_TIME;
		depth = DBUF0_DEPTH;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		dbuf_base = hisifd->dss_base + DSS_DBUF1_OFFSET;

		dfs_time = DFS_TIME;
		dfs_time_min = DFS_TIME_MIN;
		depth = DBUF1_DEPTH;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	/*
	** int K = 0;
	** int Tp = 1000000  / pinfo->pxl_clk_rate;
	** K = (pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch + pinfo->xres +
	**	pinfo->ldi.h_front_porch) / pinfo->xres;
	** thd_cg_out = dfs_time / (Tp * K * 6);
	*/
	if (pinfo->pxl_clk_rate_div <= 0)
		pinfo->pxl_clk_rate_div = 1;
	/*lint -e712 */
	thd_cg_out = (dfs_time * pinfo->pxl_clk_rate * pinfo->xres) /
		(((pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch) * pinfo->pxl_clk_rate_div
		+ pinfo->xres) * 6 * 1000000UL);
	/*lint +e712 */
	sram_valid_num = thd_cg_out / depth;
	thd_cg_in = (sram_valid_num + 1) * depth - 1;

	sram_max_mem_depth = (sram_valid_num + 1) * depth;

	thd_rqos_in = thd_cg_out * 85 / 100;
	thd_rqos_out = thd_cg_out;
	thd_flux_req_befdfs_in = GET_FLUX_REQ_IN(sram_max_mem_depth);
	thd_flux_req_befdfs_out = GET_FLUX_REQ_OUT(sram_max_mem_depth);

	sram_min_support_depth = dfs_time_min * pinfo->xres * pinfo->pxl_clk_rate_div / (1000000 / 60 / (pinfo->yres +
		pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width) * (DBUF_WIDTH_BIT / 3 / BITS_PER_BYTE));

	//thd_flux_req_aftdfs_in   =[(sram_valid_num+1)*depth - 50*HSIZE/((1000000/60/(VSIZE+VFP+VBP+VSW))*6)]/3
	thd_flux_req_aftdfs_in = (sram_max_mem_depth - sram_min_support_depth) / 3;
	//thd_flux_req_aftdfs_out  =  2*[(sram_valid_num+1)* depth - 50*HSIZE/((1000000/60/(VSIZE+VFP+VBP+VSW))*6)]/3
	thd_flux_req_aftdfs_out = 2 * (sram_max_mem_depth - sram_min_support_depth) / 3;

	thd_dfs_ok = thd_flux_req_befdfs_in;

	HISI_FB_DEBUG("sram_valid_num=%d,\n"
		"thd_rqos_in=0x%x\n"
		"thd_rqos_out=0x%x\n"
		"thd_cg_in=0x%x\n"
		"thd_cg_out=0x%x\n"
		"thd_flux_req_befdfs_in=0x%x\n"
		"thd_flux_req_befdfs_out=0x%x\n"
		"thd_flux_req_aftdfs_in=0x%x\n"
		"thd_flux_req_aftdfs_out=0x%x\n"
		"thd_dfs_ok=0x%x\n",
		sram_valid_num,
		thd_rqos_in,
		thd_rqos_out,
		thd_cg_in,
		thd_cg_out,
		thd_flux_req_befdfs_in,
		thd_flux_req_befdfs_out,
		thd_flux_req_aftdfs_in,
		thd_flux_req_aftdfs_out,
		thd_dfs_ok);

	if (g_fpga_flag == 1) {
		thd_flux_req_befdfs_out = 0xacf;
		thd_flux_req_befdfs_in = 0x734;
		thd_flux_req_aftdfs_out = 0x4dc;
		thd_flux_req_aftdfs_in = 0x26e;
		thd_dfs_ok = 0x960;
		dfs_ok_mask = 0;
		thd_rqos_out = 0x9c0;
		thd_rqos_in = 0x898;
		thd_cg_out = 0x9c0;
		thd_cg_in = 0x1780;
	} else {
		/* not use(can save power)
		sram_valid_num = 1
		thd_cg_out = 0x9d8
		thd_cg_in = 0x15df
		thd_rqos_in = 0x8c0
		thd_rqos_out = 0x9d8
		thd_rqos_idle = 0x11de
		thd_flux_req_befdfs_in = 0x7a8
		thd_flux_req_befdfs_out = 0x8c0
		thd_flux_req_aftdfs_in = 0x7a8
		thd_flux_req_aftdfs_out = 0x8c0
		thd_dfs_ok = 0x7a8 */

		// default use
		sram_valid_num = 0;
		thd_cg_out = 0x9d8;
		thd_cg_in = 0xaef;
		thd_rqos_in = 0x8c0;
		thd_rqos_out = 0x9d8;
		thd_flux_req_befdfs_in = 0x7a8;
		thd_flux_req_befdfs_out = 0x8c0;
		thd_flux_req_aftdfs_in = 0x7a8;
		thd_flux_req_aftdfs_out = 0x8c0;
		thd_dfs_ok = 0x7a8;
	}

	outp32(dbuf_base + DBUF_FRM_SIZE, pinfo->xres * pinfo->yres);
	outp32(dbuf_base + DBUF_FRM_HSIZE, DSS_WIDTH(pinfo->xres));
	outp32(dbuf_base + DBUF_SRAM_VALID_NUM, sram_valid_num); // 0x0
	outp32(dbuf_base + DBUF_THD_RQOS, (thd_rqos_out<< 16) | thd_rqos_in); // 0x9d8 0x8c0
	outp32(dbuf_base + DBUF_THD_WQOS, (thd_wqos_out << 16) | thd_wqos_in); // 0,0
	outp32(dbuf_base + DBUF_THD_CG, (thd_cg_out << 16) | thd_cg_in); // 0x9d8 0xaef
	outp32(dbuf_base + DBUF_THD_OTHER, (thd_cg_hold << 16) | thd_wr_wait); // 0 0
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_BEF, (thd_flux_req_befdfs_out << 16) | thd_flux_req_befdfs_in); // 0x8c0 0x7a8
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_AFT, (thd_flux_req_aftdfs_out << 16) | thd_flux_req_aftdfs_in); // 0x8c0 0x7a8
	outp32(dbuf_base + DBUF_THD_DFS_OK, thd_dfs_ok); // 0x7a8
	outp32(dbuf_base + DBUF_FLUX_REQ_CTRL, (dfs_ok_mask << 1) | thd_flux_req_sw_en); // 0 0

	/* outp32(dbuf_base + DBUF_THD_RQOS_IDLE, 0x111de) */
	outp32(dbuf_base + DBUF_THD_RQOS_IDLE, 0); // 0x0

	outp32(dbuf_base + DBUF_DFS_LP_CTRL, 0x1);

	HISI_FB_DEBUG("-.!\n");
}
/*lint +e438 +e550 +e838 +e573 +e647*/
/*lint -e838 -e568 -e685*/
static void init_ldi_pxl_div(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *ldi_base = NULL;
	uint32_t ifbc_type = 0;
	uint32_t mipi_idx = 0;
	uint32_t pxl0_div2_gt_en = 0;
	uint32_t pxl0_div4_gt_en = 0;
	uint32_t pxl0_divxcfg = 0;
	uint32_t pxl0_dsi_gt_en = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == EXTERNAL_PANEL_IDX)
            return;

	ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

	ifbc_type = pinfo->ifbc_type;
	if (ifbc_type  >= IFBC_TYPE_MAX) {
		HISI_FB_ERR("ifbc_type is invalid");
		return;
	}

	mipi_idx = is_dual_mipi_panel(hisifd) ? 1 : 0;

	pxl0_div2_gt_en = g_mipi_ifbc_division[mipi_idx][ifbc_type].pxl0_div2_gt_en;
	pxl0_div4_gt_en = g_mipi_ifbc_division[mipi_idx][ifbc_type].pxl0_div4_gt_en;
	pxl0_divxcfg = g_mipi_ifbc_division[mipi_idx][ifbc_type].pxl0_divxcfg;
	pxl0_dsi_gt_en = g_mipi_ifbc_division[mipi_idx][ifbc_type].pxl0_dsi_gt_en;

	set_reg(ldi_base + LDI_PXL0_DIV2_GT_EN, pxl0_div2_gt_en, 1, 0);
	set_reg(ldi_base + LDI_PXL0_DIV4_GT_EN, pxl0_div4_gt_en, 1, 0);
	set_reg(ldi_base + LDI_PXL0_GT_EN, 0x1, 1, 0);
	set_reg(ldi_base + LDI_PXL0_DSI_GT_EN, pxl0_dsi_gt_en, 2, 0);
	set_reg(ldi_base + LDI_PXL0_DIVXCFG, pxl0_divxcfg, 3, 0);
}
/*lint +e838 +e568 +e685*/
/*lint -e838 */
void init_ldi(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *ldi_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_rect_t rect = {0,0,0,0};
	uint32_t te_source = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
		if (g_fpga_flag == 1) {
			set_reg(hisifd->dss_base + GLB_TP_SEL, 0x2, 2, 0);
		}
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	rect.x = 0;
	rect.y = 0;
	rect.w = pinfo->xres;
	rect.h = pinfo->yres;
	mipi_ifbc_get_rect(hisifd, &rect);

	init_ldi_pxl_div(hisifd);

	if (is_dual_mipi_panel(hisifd)) {
		set_reg(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_DSI1_RST_SEL, 0x0, 1, 0);
		set_reg(hisifd->dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_DSI_MUX_SEL, 0x0, 1, 0);
		if (is_mipi_video_panel(hisifd)) {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, (pinfo->ldi.h_back_porch + DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16);
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL1, 0);
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL2, DSS_WIDTH(rect.w));
		} else {
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL0, pinfo->ldi.h_back_porch << 16);
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
			outp32(ldi_base + LDI_DPI1_HRZ_CTRL2, DSS_WIDTH(rect.w));
		}

		outp32(ldi_base + LDI_OVERLAP_SIZE,
			pinfo->ldi.dpi0_overlap_size | (pinfo->ldi.dpi1_overlap_size << 16));

		/* dual_mode_en */
		set_reg(ldi_base + LDI_CTRL, 1, 1, 5);

		/* split mode */
		set_reg(ldi_base + LDI_CTRL, 0, 1, 16);

		//dual lcd: 0x1, dual mipi: 0x0
		set_reg(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_DSI1_CLK_SEL, 0x0, 1, 0);
	}
	if (is_mipi_video_panel(hisifd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
				pinfo->ldi.h_front_porch | ((pinfo->ldi.h_back_porch + DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
				pinfo->ldi.h_front_porch | (pinfo->ldi.h_back_porch << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(rect.w));
	}
	outp32(ldi_base + LDI_VRT_CTRL0,
		pinfo->ldi.v_front_porch | (pinfo->ldi.v_back_porch << 16));
	outp32(ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(pinfo->ldi.v_pulse_width));
	outp32(ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(rect.h));

	outp32(ldi_base + LDI_PLR_CTRL,
		pinfo->ldi.vsync_plr | (pinfo->ldi.hsync_plr << 1) |
		(pinfo->ldi.pixelclk_plr << 2) | (pinfo->ldi.data_en_plr << 3));

	//sensorhub int msk
	//outp32(ldi_base + LDI_SH_MASK_INT, 0x0);

	// bpp
	set_reg(ldi_base + LDI_CTRL, pinfo->bpp, 2, 3);
	// bgr
	set_reg(ldi_base + LDI_CTRL, pinfo->bgr_fmt, 1, 13);

	// for ddr pmqos
	outp32(ldi_base + LDI_VINACT_MSK_LEN,
		pinfo->ldi.v_front_porch);

	//cmd event sel
	outp32(ldi_base + LDI_CMD_EVENT_SEL, 0x1);

	//outp32(ldi_base + LDI_FRM_VALID_DBG, 0x1);

	// for 1Hz LCD and mipi command LCD
	if (is_mipi_cmd_panel(hisifd)) {
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x1, 2, 0);

		//DSI_TE_CTRL
		// te_source = 0, select te_pin
		// te_source = 1, select te_triger
		te_source = 0;

		// dsi_te_hard_en
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 0);
		// dsi_te0_pin_p , dsi_te1_pin_p
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 2, 1);
		// dsi_te_hard_sel
		set_reg(ldi_base + LDI_DSI_TE_CTRL, te_source, 1, 3);
		// select TE0 PIN
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x01, 2, 6);
		// dsi_te_mask_en
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 8);
		// dsi_te_mask_dis
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 9);
		// dsi_te_mask_und
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 4, 13);
		// dsi_te_pin_en
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x1, 1, 17);

		//TBD:(dsi_te_hs_num+vactive)*htotal/clk_pxl0_div+0.00004<1/60+vs_te_time+(vactive*hotal) /clk_ddic_rd
		set_reg(ldi_base + LDI_DSI_TE_HS_NUM, 0x0, 32, 0);
		set_reg(ldi_base + LDI_DSI_TE_HS_WD, 0x24024, 32, 0);

		// dsi_te0_vs_wd = lcd_te_width / T_pxl_clk, experience lcd_te_width = 2us
		if (pinfo->pxl_clk_rate_div == 0) {
			HISI_FB_ERR("pxl_clk_rate_div is NULL, not support !\n");
			pinfo->pxl_clk_rate_div = 1;
		}
		/*lint -e712 */
		set_reg(ldi_base + LDI_DSI_TE_VS_WD,
			(0x3FC << 12) | (2 * pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div / 1000000), 32, 0);
		/*lint +e712 */
		//set_reg(ldi_base + LDI_DSI_TE_VS_WD, 0x3FC0FF, 32, 0);
		//set_reg(ldi_base + LDI_DSI_TE_VS_WD, 0x3FC01F, 32, 0);
	} else {
		// dsi_te_hard_en
		set_reg(ldi_base + LDI_DSI_TE_CTRL, 0x0, 1, 0);
		set_reg(ldi_base + LDI_DSI_CMD_MOD_CTRL, 0x2, 2, 0);
	}
	//ldi_data_gate(hisifd, true);

	// normal
	set_reg(ldi_base + LDI_WORK_MODE, 0x1, 1, 0);


	if (is_mipi_cmd_panel(hisifd)) {
		set_reg(ldi_base + LDI_FRM_MSK,
			(hisifd->frame_update_flag == 1) ? 0x0 : 0x1, 1, 0);
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX && (is_mipi_panel(hisifd))) {
		set_reg(ldi_base + LDI_DP_DSI_SEL, 0x1, 1, 0);
	}

	// ldi disable
	if (!fastboot_enable)
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);

	HISI_FB_DEBUG("-.!\n");
}
/*lint +e838 */
void deinit_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	set_reg(ldi_base + LDI_CTRL, 0, 1, 0);
}

void enable_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	/* ldi enable */
	set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
}

void disable_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	/* ldi disable */
	set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
}

void ldi_frame_update(struct hisi_fb_data_type *hisifd, bool update)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

		if (is_mipi_cmd_panel(hisifd)) {
			set_reg(ldi_base + LDI_FRM_MSK, (update ? 0x0 : 0x1), 1, 0);
			if (update)
				set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		}
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
	}
}

void single_frame_update(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
		if (is_mipi_cmd_panel(hisifd)) {
			set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);
			set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		} else {
			set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		}

	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;

		if (is_mipi_cmd_panel(hisifd)) {
			set_reg(ldi_base + LDI_FRM_MSK_UP, 0x1, 1, 0);
			set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		} else {
			set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
		}
	} else {
		;
	}
}
/*lint -e838 */
void dpe_interrupt_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t clear = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_PDP_INTS, clear);
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS, clear);
		outp32(dss_base + DSS_DPP_OFFSET + DPP_INTS, clear);

		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INTS, clear);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_OFF_INTS, clear);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_unmask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t unmask = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	pinfo = &(hisifd->panel_info);
	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		unmask = ~0;
		//unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS | BIT_DSS_GLB_INTS | BIT_MMU_IRPT_NS);
		unmask &= ~(BIT_DPP_INTS | BIT_ITF0_INTS | BIT_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, unmask);

		unmask = ~0;
		if (is_mipi_cmd_panel(hisifd)) {
			unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		} else {
			unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		}
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);

		unmask = ~0;
		//unmask &= ~(BIT_CE_END_IND | BIT_BACKLIGHT_INTP);
		if ((pinfo->acm_ce_support == 1) && HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_ACE))
			unmask &= ~(BIT_CE_END_IND);

		if (pinfo->hiace_support == 1)
			unmask &= ~(BIT_HIACE_IND);

		outp32(dss_base + DSS_DPP_OFFSET + DPP_INT_MSK, unmask);

	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH0_INTS | BIT_OFF_WCH1_INTS | BIT_OFF_WCH0_WCH1_FRM_END_INT | BIT_OFF_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, unmask);
		unmask = ~0;
		unmask &= ~(BIT_OFF_CAM_WCH2_FRMEND_INTS);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, unmask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_mask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t mask = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	dss_base = hisifd->dss_base;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_PDP_INT_MSK, mask);
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
		outp32(dss_base + DSS_DPP_OFFSET + DPP_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_MCTL_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INT_MSK, mask);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, mask);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, mask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}
/*lint +e838 */
void ldi_data_gate(struct hisi_fb_data_type *hisifd, bool enble)
{
	char __iomem *ldi_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (!is_mipi_cmd_panel(hisifd)) {
		hisifd->ldi_data_gate_en = (enble ? 1 : 0);
		return ;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	if (g_ldi_data_gate_en == 1) {
		hisifd->ldi_data_gate_en = (enble ? 1 : 0);
		set_reg(ldi_base + LDI_CTRL, (enble ? 0x1 : 0x0), 1, 2);
	} else {
		hisifd->ldi_data_gate_en = 0;
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 2);
	}

	HISI_FB_DEBUG("ldi_data_gate_en=%d!\n", hisifd->ldi_data_gate_en);
}

/* dpp csc not surport */
void init_dpp_csc(struct hisi_fb_data_type *hisifd)
{
	return;
}

/*lint -e838 */
void init_acm(struct hisi_fb_data_type *hisifd)
{
	return;
}
/*lint +e838 */
//lint -e838 -e550 -e438
static void degamma_set_lut(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	uint32_t i = 0;
	uint32_t index = 0;
	char __iomem *degamma_lut_base = NULL;//lint !e838

	if (hisifd == NULL)	{
		HISI_FB_ERR("init_degmma_xcc_gmp hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	degamma_lut_base = hisifd->dss_base + DSS_DPP_DEGAMMA_LUT_OFFSET;

	if (!hisifb_use_dynamic_degamma(hisifd, degamma_lut_base)) {
		if (pinfo->igm_lut_table_len > 0
			&& pinfo->igm_lut_table_R
			&& pinfo->igm_lut_table_G
			&& pinfo->igm_lut_table_B) {
			for (i = 0; i < pinfo->igm_lut_table_len / 2; i++) {
				index = i << 1;
				outp32(degamma_lut_base + (U_DEGAMA_R_COEF +  i * 4), pinfo->igm_lut_table_R[index] | pinfo->igm_lut_table_R[index+1] << 16);
				outp32(degamma_lut_base + (U_DEGAMA_G_COEF +  i * 4), pinfo->igm_lut_table_G[index] | pinfo->igm_lut_table_G[index+1] << 16);
				outp32(degamma_lut_base + (U_DEGAMA_B_COEF +  i * 4), pinfo->igm_lut_table_B[index] | pinfo->igm_lut_table_B[index+1] << 16);
			}
			outp32(degamma_lut_base + U_DEGAMA_R_LAST_COEF, pinfo->igm_lut_table_R[pinfo->igm_lut_table_len - 1]);
			outp32(degamma_lut_base + U_DEGAMA_G_LAST_COEF, pinfo->igm_lut_table_G[pinfo->igm_lut_table_len - 1]);
			outp32(degamma_lut_base + U_DEGAMA_B_LAST_COEF, pinfo->igm_lut_table_B[pinfo->igm_lut_table_len - 1]);
		}
	}//lint !e438 !e550
}
static void gamma_set_lut(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	uint32_t i = 0;
	uint32_t index = 0;
	char __iomem *gamma_lut_base = NULL;

	if (hisifd == NULL)	{
		HISI_FB_ERR("init_degmma_xcc_gmp hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;

	if (!hisifb_use_dynamic_gamma(hisifd, gamma_lut_base)) {
		if (pinfo->gamma_lut_table_len > 0
			&& pinfo->gamma_lut_table_R
			&& pinfo->gamma_lut_table_G
			&& pinfo->gamma_lut_table_B) {
			for (i = 0; i < pinfo->gamma_lut_table_len / 2; i++) {
				index = i << 1;
				//GAMA LUT
				outp32(gamma_lut_base + (U_GAMA_R_COEF + i * 4), pinfo->gamma_lut_table_R[index] | pinfo->gamma_lut_table_R[index+1] << 16 );
				outp32(gamma_lut_base + (U_GAMA_G_COEF + i * 4), pinfo->gamma_lut_table_G[index] | pinfo->gamma_lut_table_G[index+1] << 16 );
				outp32(gamma_lut_base + (U_GAMA_B_COEF + i * 4), pinfo->gamma_lut_table_B[index] | pinfo->gamma_lut_table_B[index+1] << 16 );
			}
			outp32(gamma_lut_base + U_GAMA_R_LAST_COEF, pinfo->gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_G_LAST_COEF, pinfo->gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_B_LAST_COEF, pinfo->gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);
		}
	}
}
/*lint -e695*/
inline void xcc_set_coef(char __iomem *base_addr, struct hisi_panel_info *pinfo,
    uint32_t rectify_R, uint32_t rectify_G, uint32_t rectify_B)
{
	if (pinfo == NULL) {
		HISI_FB_ERR("xcc_set_coef pinfo is NULL!\n");
		return;
	}

	outp32(base_addr + LCP_XCC_COEF_00, pinfo->xcc_table[0]);
	outp32(base_addr + LCP_XCC_COEF_01, pinfo->xcc_table[1]
	    * g_led_rg_csc_value[0] / 32768 * rectify_R / 32768);
	outp32(base_addr + LCP_XCC_COEF_02, pinfo->xcc_table[2]);
	outp32(base_addr + LCP_XCC_COEF_03, pinfo->xcc_table[3]);
	outp32(base_addr + LCP_XCC_COEF_10, pinfo->xcc_table[4]);
	outp32(base_addr + LCP_XCC_COEF_11, pinfo->xcc_table[5]);
	outp32(base_addr + LCP_XCC_COEF_12, pinfo->xcc_table[6]
	    * g_led_rg_csc_value[4] / 32768 * rectify_G / 32768);
	outp32(base_addr + LCP_XCC_COEF_13, pinfo->xcc_table[7]);
	outp32(base_addr + LCP_XCC_COEF_20, pinfo->xcc_table[8]);
	outp32(base_addr + LCP_XCC_COEF_21, pinfo->xcc_table[9]);
	outp32(base_addr + LCP_XCC_COEF_22, pinfo->xcc_table[10]);
	outp32(base_addr + LCP_XCC_COEF_23, pinfo->xcc_table[11]
	    * g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
	    * rectify_B / 32768);
}

static void gmp_set_lut(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct lcp_info *lcp_param = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *gmp_lut_base = NULL;
	uint32_t gmp_lut_sel = 0;
	int i;

	if (hisifd == NULL) {
		HISI_FB_ERR("[effect] hisifd is NULL!\n");
		return;
	}
	gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
	gmp_lut_base = hisifd->dss_base + DSS_DPP_GMP_LUT_OFFSET;

	pinfo = &(hisifd->panel_info);

	lcp_param = &(hisifd->effect_info.lcp);
	if (lcp_param == NULL) {
		HISI_FB_ERR("fb%d, lcp_param is NULL!\n", hisifd->index);
		return;
	}

	if (hisifd->effect_gmp_update_flag) {
		spin_lock(&g_gmp_effect_lock);
		if (lcp_param->gmp_table_low32 && lcp_param->gmp_table_high4) {
			for (i = 0; i < gmp_cnt_cofe; i++) {
				set_reg(gmp_lut_base + i * 2 * 4, lcp_param->gmp_table_low32[i], 32, 0);//lint !e679
				set_reg(gmp_lut_base + i * 2 * 4 + 4, lcp_param->gmp_table_high4[i], 4, 0);//lint !e679
				if ((i % 500) == 0)
					HISI_FB_INFO("[effect] lcp_param gmp_table_low32[%d]=%d,gmp_table_high4[%d]=%d\n",i,lcp_param->gmp_table_low32[i],i,lcp_param->gmp_table_high4[i]);
			}
		}
		spin_unlock(&g_gmp_effect_lock);

		gmp_lut_sel = (uint32_t)inp32(gmp_base + GMP_LUT_SEL);
		set_reg(gmp_base + GMP_LUT_SEL, (~(gmp_lut_sel & 0x1)) & 0x1, 1, 0);

		//enable gmp
		HISI_FB_INFO("[effect] gmp enable=%d, gmp_lut_sel=%d\n",lcp_param->gmp_enable,gmp_lut_sel);
		set_reg(gmp_base + GMP_EN, lcp_param->gmp_enable, 1, 0);
	} else {
		if ((pinfo->gmp_lut_table_len == gmp_cnt_cofe)
		&& (pinfo->gmp_lut_table_low32bit)
		&& (pinfo->gmp_lut_table_high4bit)) {
			for (i = 0; i < gmp_cnt_cofe; i++) {
				outp32(gmp_lut_base + i * 2 * 4, pinfo->gmp_lut_table_low32bit[i]);
				outp32(gmp_lut_base + i * 2 * 4 + 4, pinfo->gmp_lut_table_high4bit[i]);
				if ((i%500) == 0) {
					HISI_FB_INFO("[effect] pinfo gmp_table_low32[%d]=%d,gmp_table_high4[%d]=%d\n",i,pinfo->gmp_lut_table_low32bit[i],i,pinfo->gmp_lut_table_high4bit[i]);
				}
			}

			gmp_lut_sel = (uint32_t)inp32(gmp_base + GMP_LUT_SEL);
			set_reg(gmp_base + GMP_LUT_SEL, (~(gmp_lut_sel & 0x1)) & 0x1, 1, 0);
			HISI_FB_INFO("[effect] gmp_lut_sel=%d\n",gmp_lut_sel);
			//not enable gmp
			set_reg(gmp_base + GMP_EN, 0x0, 1, 0);
		}
	}
}

/*lint +e695*/
void init_igm_gmp_xcc_gm(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *degamma_base = NULL;
	char __iomem *gamma_base = NULL;
	uint32_t gama_lut_sel = 0;
	uint32_t degama_lut_sel = 0;
	uint32_t color_temp_rectify_R = 32768;
	uint32_t color_temp_rectify_G = 32768;
	uint32_t color_temp_rectify_B = 32768;

	if (hisifd == NULL)	{
		HISI_FB_ERR("init_degmma_xcc_gmp hisifd is NULL!\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
		gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
		degamma_base = hisifd->dss_base + DSS_DPP_DEGAMMA_OFFSET;
		gamma_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!\n", hisifd->index);
		return;
	}
	// avoid the partial update
	hisifd->display_effect_flag = 40;

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}
	//Degamma
	if (pinfo->gamma_support == 1) {
		//disable degamma
		set_reg(degamma_base + DEGAMA_EN, 0x0, 1, 0);

		degamma_set_lut(hisifd);
			degama_lut_sel = (uint32_t)inp32(degamma_base + DEGAMA_LUT_SEL);
			set_reg(degamma_base + DEGAMA_LUT_SEL, (~(degama_lut_sel & 0x1)) & 0x1, 1, 0);

		//enable degamma
		set_reg(degamma_base + DEGAMA_EN, 0x1, 1, 0);
	} else {
		//degama memory shutdown
		outp32(degamma_base + DEGAMA_MEM_CTRL, 0x4);
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len == xcc_cnt_cofe && pinfo->xcc_table) {
			xcc_set_coef(xcc_base, pinfo, color_temp_rectify_R, color_temp_rectify_G, color_temp_rectify_B);
			//enable xcc
			set_reg(xcc_base + LCP_XCC_BYPASS_EN, 0x0, 1, 0);
		}
	}

	//GMP
	if (pinfo->gmp_support == 1) {
		gmp_set_lut(hisifd);
		g_gmp_State = 1;
	} else {
		//gmp memory shutdown
		outp32(gmp_base + GMP_MEM_CTRL, 0x2);    //only support deep sleep
	}

	//GAMMA
	if (pinfo->gamma_support == 1) {
		//disable gamma
		set_reg(gamma_base + GAMA_EN, 0x0, 1, 0);
		//set gama lut
		gamma_set_lut(hisifd);

		gama_lut_sel = (uint32_t)inp32(gamma_base + GAMA_LUT_SEL);
		set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);

		//enable gamma
		set_reg(gamma_base + GAMA_EN, 0x1, 1, 0);
	} else {
		//gama memory shutdown
		outp32(gamma_base + GAMA_MEM_CTRL, 0x4);
	}
}
/* +e679 */
void init_dither(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *dither_base = NULL;

	if (hisifd == NULL)	{
		HISI_FB_ERR("hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->dither_support != 1) {
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dither_base = hisifd->dss_base + DSS_DPP_DITHER_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	set_reg(dither_base + DITHER_CTL1, 0x00000005, 6, 0);
	set_reg(dither_base + DITHER_CTL0, 0x0000000B, 5, 0);
	set_reg(dither_base + DITHER_TRI_THD12_0, 0x00080080, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_1, 0x00000080, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10, 0x02008020, 30, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_0, 0x00100100, 24, 0);
	set_reg(dither_base + DITHER_TRI_THD12_UNI_1, 0x00000000, 12, 0);
	set_reg(dither_base + DITHER_TRI_THD10_UNI, 0x00010040, 30, 0);
	set_reg(dither_base + DITHER_BAYER_CTL, 0x00000000, 28, 0);
	set_reg(dither_base + DITHER_BAYER_ALPHA_THD, 0x00000000, 30, 0);
	set_reg(dither_base + DITHER_MATRIX_PART1, 0x5D7F91B3, 32, 0);
	set_reg(dither_base + DITHER_MATRIX_PART0, 0x6E4CA280, 32, 0);

	set_reg(dither_base + DITHER_HIFREQ_REG_INI_CFG_EN, 0x00000001, 1, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_0, 0x6495FC13, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_1, 0x27E5DB75, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_2, 0x69036280, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI0_3, 0x7478D47C, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_0, 0x36F5325D, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_1, 0x90757906, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_2, 0xBBA85F01, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI1_3, 0x74B34461, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_0, 0x76435C64, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_1, 0x4989003F, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_2, 0xA2EA34C6, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_REG_INI2_3, 0x4CAD42CB, 31, 0);
	set_reg(dither_base + DITHER_HIFREQ_POWER_CTRL, 0x00000009, 4, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_0, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_1, 0x00000701, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_FILT_2, 0x00000421, 15, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_R1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_G1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B0, 0x6D92B7DB, 32, 0);
	set_reg(dither_base + DITHER_HIFREQ_THD_B1, 0x00002448, 16, 0);
	set_reg(dither_base + DITHER_ERRDIFF_CTL, 0x00000000, 3, 0);
	set_reg(dither_base + DITHER_ERRDIFF_WEIGHT, 0x01232134, 28, 0);

	set_reg(dither_base + DITHER_FRC_CTL, 0x00000001, 4, 0);
	set_reg(dither_base + DITHER_FRC_01_PART1, 0xFFFF0000, 32, 0);
	set_reg(dither_base + DITHER_FRC_01_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART0, 0xFFFF0000, 32, 0);
}
//lint +e838 +e550 +e438
/*lint -e838 */
void dpe_store_ct_cscValue(struct hisi_fb_data_type *hisifd, unsigned int csc_value[])
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return;
	}

	pinfo->xcc_table[1] = csc_value[0];
	pinfo->xcc_table[2] = csc_value[1];
	pinfo->xcc_table[3] = csc_value[2];
	pinfo->xcc_table[5] = csc_value[3];
	pinfo->xcc_table[6] = csc_value[4];
	pinfo->xcc_table[7] = csc_value[5];
	pinfo->xcc_table[9] = csc_value[6];
	pinfo->xcc_table[10] = csc_value[7];
	pinfo->xcc_table[11] = csc_value[8];

	return;
}

void dpe_update_g_comform_discount(unsigned int value)
{
	g_comform_value = value;
	HISI_FB_INFO(" g_comform_value = %d" , g_comform_value);
}

int dpe_set_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *lcp_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			xcc_set_coef(lcp_base, pinfo, color_temp_rectify_R, color_temp_rectify_G, color_temp_rectify_B);
			hisifd->color_temperature_flag = 2;
		}
	}

	return 0;
}

ssize_t dpe_show_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11]);
}
/*lint +e838 */
int dpe_set_xcc_cscValue(struct hisi_fb_data_type *hisifd)
{
	return 0;
}
/*lint -e550, -e838*/
int dpe_set_comform_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	uint32_t color_temp_rectify_R = 32768;
	uint32_t color_temp_rectify_G = 32768;
	uint32_t color_temp_rectify_B = 32768;
	char __iomem *lcp_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET;

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R <= 32768 && pinfo->color_temp_rectify_R) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B <= 32768 && pinfo->color_temp_rectify_B) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			xcc_set_coef(lcp_base, pinfo, color_temp_rectify_R, color_temp_rectify_G, color_temp_rectify_B);
		}
	}

	return 0;
}
/*lint +e550, +e838*/
/*lint -e838*/
ssize_t dpe_show_comform_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo->xcc_support == 0 || pinfo->xcc_table == NULL) {
		return 0;
	}

	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,g_comform_value = %d\n",
		pinfo->xcc_table[1], pinfo->xcc_table[2], pinfo->xcc_table[3],
		pinfo->xcc_table[5], pinfo->xcc_table[6], pinfo->xcc_table[7],
		pinfo->xcc_table[9], pinfo->xcc_table[10], pinfo->xcc_table[11],
		g_comform_value);
}
/*lint +e838*/
void dpe_init_led_rg_ct_cscValue(void)
{
	g_led_rg_csc_value[0] = 32768;
	g_led_rg_csc_value[1] = 0;
	g_led_rg_csc_value[2] = 0;
	g_led_rg_csc_value[3] = 0;
	g_led_rg_csc_value[4] = 32768;
	g_led_rg_csc_value[5] = 0;
	g_led_rg_csc_value[6] = 0;
	g_led_rg_csc_value[7] = 0;
	g_led_rg_csc_value[8] = 32768;
	g_is_led_rg_csc_set = 0;

	return;
}

void dpe_store_led_rg_ct_cscValue(unsigned int csc_value[])
{
	g_led_rg_csc_value [0] = csc_value[0];
	g_led_rg_csc_value [1] = csc_value[1];
	g_led_rg_csc_value [2] = csc_value[2];
	g_led_rg_csc_value [3] = csc_value[3];
	g_led_rg_csc_value [4] = csc_value[4];
	g_led_rg_csc_value [5] = csc_value[5];
	g_led_rg_csc_value [6] = csc_value[6];
	g_led_rg_csc_value [7] = csc_value[7];
	g_led_rg_csc_value [8] = csc_value[8];
	g_is_led_rg_csc_set = 1;

	return;
}
/*lint -e838*/
int dpe_set_led_rg_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *lcp_base = NULL;
	uint32_t color_temp_rectify_R = 32768;
	uint32_t color_temp_rectify_G = 32768;
	uint32_t color_temp_rectify_B = 32768;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -1;
	}

	if (pinfo->color_temp_rectify_R && pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_R <= 32768) {
		color_temp_rectify_R = pinfo->color_temp_rectify_R;
	}

	if (pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_G && pinfo->color_temp_rectify_G <= 32768) {
		color_temp_rectify_G = pinfo->color_temp_rectify_G;
	}

	if (pinfo->color_temp_rectify_B && pinfo->color_temp_rectify_support && pinfo->color_temp_rectify_B <= 32768) {
		color_temp_rectify_B = pinfo->color_temp_rectify_B;
	}

	//XCC
	if (g_is_led_rg_csc_set == 1 && pinfo->xcc_support == 1) {
		HISI_FB_DEBUG("real set color temperature: g_is_led_rg_csc_set = %d, R = 0x%x, G = 0x%x, B = 0x%x .\n",
				g_is_led_rg_csc_set, g_led_rg_csc_value[0], g_led_rg_csc_value[4], g_led_rg_csc_value[8]);
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(lcp_base + LCP_XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(lcp_base + LCP_XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(lcp_base + LCP_XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(lcp_base + LCP_XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(lcp_base + LCP_XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(lcp_base + LCP_XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(lcp_base + LCP_XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(lcp_base + LCP_XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(lcp_base + LCP_XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(lcp_base + LCP_XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(lcp_base + LCP_XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(lcp_base + LCP_XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
		}
	}

	return 0;
}
/*lint +e838*/
ssize_t dpe_show_led_rg_ct_cscValue(char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d\n",
		g_led_rg_para1, g_led_rg_para2,
		g_led_rg_csc_value [0], g_led_rg_csc_value [1], g_led_rg_csc_value [2],
		g_led_rg_csc_value [3], g_led_rg_csc_value [4], g_led_rg_csc_value [5],
		g_led_rg_csc_value [6], g_led_rg_csc_value [7], g_led_rg_csc_value [8]);
}

ssize_t dpe_show_cinema_value(struct hisi_fb_data_type *hisifd, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "gamma type is = %d\n", hisifd->panel_info.gamma_type);
}

int dpe_set_cinema(struct hisi_fb_data_type *hisifd, unsigned int value)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd, NUll pointer warning.\n");
		return -1;
	}

	if (hisifd->panel_info.gamma_type == value) {
		HISI_FB_DEBUG("fb%d, cinema mode is already in %d!\n", hisifd->index, value);
		return 0;
	}

	hisifd->panel_info.gamma_type = value;
	return 0;
}

void dpe_update_g_acm_state(unsigned int value)
{
	return;
}

void dpe_set_acm_state(struct hisi_fb_data_type *hisifd)
{
	return;
}
/*lint -e838*/
ssize_t dpe_show_acm_state(char *buf)
{
	ssize_t ret = 0;

	if (buf == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_acm_State = %d\n", g_acm_State);

	return ret;
}

void dpe_update_g_gmp_state(unsigned int value)
{
	return;
}

void dpe_set_gmp_state(struct hisi_fb_data_type *hisifd)
{
	return;
}

ssize_t dpe_show_gmp_state(char *buf)
{
	ssize_t ret = 0;

	if (buf == NULL) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_gmp_State = %d\n", g_gmp_State);

	return ret;
}
/*lint +e838*/
void dpe_sbl_set_al_bl(struct hisi_fb_data_type *hisifd)
{
	return;
}

