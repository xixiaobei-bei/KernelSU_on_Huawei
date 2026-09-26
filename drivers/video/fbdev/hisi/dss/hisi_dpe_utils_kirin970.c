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
/*lint -e838 -e679 -e712*/
DEFINE_SEMAPHORE(hisi_fb_dss_inner_clk_sem);

static int dss_inner_clk_refcount = 0;
static unsigned  int g_comform_value = 0;
static unsigned  int g_acm_State = 0;
static unsigned  int g_gmp_State = 0;
static unsigned int g_led_rg_csc_value[9];
static unsigned int g_is_led_rg_csc_set;
unsigned int g_led_rg_para1 = 7;
unsigned int g_led_rg_para2 = 30983;

#define OFFSET_FRACTIONAL_BITS	(11)
#define ROUND1(x,y)	((x) / (y) + ((x) % (y)  ? 1 : 0))
#define gmp_cnt_cofe (729)
#define xcc_cnt_cofe (12)

static uint32_t sbl_al_calib_lut[33] = {/*lint -save -e* */
	0x0000, 0x0800, 0x1000, 0x1800, 0x2000, 0x2800, 0x3000, 0x3800, 0x4000, 0x4800,
	0x5000, 0x5800, 0x6000, 0x6800, 0x7000, 0x7800, 0x8000, 0x87FF, 0x8FFF, 0x97FF,
	0x9FFF, 0xA7FF, 0xAFFF, 0xB7FF, 0xBFFF, 0xC7FF, 0xCFFF, 0xD7FF, 0xDFFF, 0xE7FF,
	0xEFFF, 0xF7FF, 0xFFFF
};/*lint -restore */

static uint32_t s_calc_al_change_lut[32] = {/*lint -save -e* */
	0x0000, 0x001E, 0x0046, 0x0078, 0x00C8, 0x012C, 0x0258, 0x03E8, 0x0640, 0x09C4,
	0x0FA0, 0x1B58, 0x32C8, 0x5DC0, 0x9C40, 0xFFFF, 0x0000, 0x000F, 0x0028, 0x003C,
	0x0064, 0x00DC, 0x01F4, 0x03E8, 0x04B0, 0x0578, 0x0AF0, 0x157c, 0x2AF8, 0x4E20,
	0x94C0,0xFFFF
};/*lint -restore */

static int get_lcd_frame_rate(struct hisi_panel_info *pinfo)
{
	return pinfo->pxl_clk_rate/(pinfo->xres + pinfo->pxl_clk_rate_div *
		(pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch + pinfo->ldi.h_pulse_width))/(pinfo->yres +
		pinfo->ldi.v_back_porch + pinfo->ldi.v_front_porch + pinfo->ldi.v_pulse_width);
}

struct dss_vote_cmd * get_dss_vote_cmd(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int frame_rate = 0;
	uint64_t default_dss_pri_clk_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return pdss_vote_cmd;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = &(hisifd->dss_vote_cmd);

	frame_rate = get_lcd_frame_rate(pinfo);

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		default_dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_L1;
	} else {
		default_dss_pri_clk_rate = DEFAULT_DSS_CORE_CLK_RATE_ES;
	}

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
					pdss_vote_cmd->dss_pri_clk_rate = default_dss_pri_clk_rate;
					pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
					pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
					pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
					hisifd->core_clk_upt_support = 1;
				}
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1080P)) {
				pdss_vote_cmd->dss_pri_clk_rate = default_dss_pri_clk_rate;
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
				pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
				pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
				hisifd->core_clk_upt_support = 1;
			} else {
				pdss_vote_cmd->dss_pri_clk_rate = default_dss_pri_clk_rate;
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
				pdss_vote_cmd->dss_pclk_dss_rate = DEFAULT_PCLK_DSS_RATE;
				pdss_vote_cmd->dss_pclk_pctrl_rate = DEFAULT_PCLK_PCTRL_RATE;
				hisifd->core_clk_upt_support = 1;
			}
		}

		if (hisifd->index == EXTERNAL_PANEL_IDX) {
			if ((pinfo->xres * pinfo->yres) >= (RES_4K_PHONE)) {
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L3;
			} else if ((pinfo->xres * pinfo->yres) >= (RES_1440P)) {
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L2;
			} else {
				pdss_vote_cmd->dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;
			}
		}
	}

	return pdss_vote_cmd;
}

static int dss_core_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk) {
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

	return 0;
}

static int dss_core_clk_disable(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	if (hisifd->dss_pri_clk) {
		clk_disable(hisifd->dss_pri_clk);
		clk_unprepare(hisifd->dss_pri_clk);
	}

	return 0;
}

static int get_mdc_clk_rate(dss_vote_cmd_t *vote_cmd, uint64_t *clk_rate)
{
	switch (vote_cmd->dss_voltage_level) {
		case PERI_VOLTAGE_LEVEL0: // 0.65v
			*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L1;
			break;
		case PERI_VOLTAGE_LEVEL1: // 0.75v
			*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L2;
			break;
		case PERI_VOLTAGE_LEVEL2: // 0.8v
			*clk_rate = DEFAULT_MDC_CORE_CLK_RATE_L3;
			break;

		default:
			HISI_FB_ERR("no support set dss_voltage_level(%d)!\n",
				vote_cmd->dss_voltage_level);
			return -1;
	}
	return 0;
}

static int set_mdc_core_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
	uint64_t clk_rate = 0;

	if (vote_cmd.dss_voltage_level == hisifd->dss_vote_cmd.dss_voltage_level) {
		return 0;
	}

	if (get_mdc_clk_rate(&vote_cmd, &clk_rate)) {
		return -1;
	}

	ret = clk_set_rate(hisifd->dss_clk_media_common_clk, clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("set dss_clk_media_common_clk(%llu) failed, error=%d!\n", clk_rate, ret);
		return -1;
	}
	hisifd->dss_vote_cmd.dss_voltage_level = vote_cmd.dss_voltage_level;

	HISI_FB_INFO("set dss_clk_media_common_clk = %llu.\n", clk_rate);

	return ret;
}

static int set_primary_core_clk(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t dss_vote_cmd)
{
	int ret = 0;
	struct hisi_fb_data_type *targetfd = NULL;
	bool set_rate_succ = true;

	targetfd = hisifd_list[AUXILIARY_PANEL_IDX];
	if (targetfd && (dss_vote_cmd.dss_pri_clk_rate >= targetfd->dss_vote_cmd.dss_pri_clk_rate)) {
		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
			ret = dss_core_clk_enable(hisifd);
			if (ret < 0) {
				HISI_FB_ERR("dss_core_clk_enable failed, error=%d!\n", ret);
				return -1;
			}
		}
		ret = clk_set_rate(hisifd->dss_pri_clk, dss_vote_cmd.dss_pri_clk_rate);
		HISI_FB_DEBUG("fb%d set dss_pri_clk_rate = %llu.\n",
			hisifd->index, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
		if (ret < 0) {
			set_rate_succ = false;
			HISI_FB_ERR("set dss_pri_clk_rate(%llu) failed, error=%d!\n",
				dss_vote_cmd.dss_pri_clk_rate, ret);
		}

		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_CLK_OFF) {
			ret = dss_core_clk_disable(hisifd);
			if (ret < 0) {
				HISI_FB_ERR("dss_core_clk_disable, error=%d!\n", ret);
				return -1;
			}
		}

		if (set_rate_succ == true) {
			hisifd->dss_vote_cmd.dss_pri_clk_rate = dss_vote_cmd.dss_pri_clk_rate;
		} else {
			return -1;
		}
	}

	HISI_FB_DEBUG("set dss_pri_clk_rate = %llu.\n", dss_vote_cmd.dss_pri_clk_rate);
	return ret;
}

int set_dss_vote_cmd(struct hisi_fb_data_type *hisifd, dss_vote_cmd_t vote_cmd)
{
	int ret = 0;
	struct hisi_fb_data_type *targetfd = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -1;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		ret = set_mdc_core_clk(hisifd, vote_cmd);
		return ret;
	}

	if ((vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L1)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L2)
		&& (vote_cmd.dss_pri_clk_rate != DEFAULT_DSS_CORE_CLK_RATE_L3)) {
		HISI_FB_ERR("no support set dss_pri_clk_rate(%llu)!\n", vote_cmd.dss_pri_clk_rate);
		return -1;
	}

	if (vote_cmd.dss_pri_clk_rate == hisifd->dss_vote_cmd.dss_pri_clk_rate) {
		return ret;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ret = set_primary_core_clk(hisifd, vote_cmd);
		return ret;
	}

	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		targetfd = hisifd_list[PRIMARY_PANEL_IDX];
		if (targetfd && (vote_cmd.dss_pri_clk_rate >= targetfd->dss_vote_cmd.dss_pri_clk_rate)) {
			hisifd->need_tuning_clk = true;
			HISI_FB_DEBUG("fb%d save dss_pri_clk_rate = %llu.\n",
				hisifd->index, vote_cmd.dss_pri_clk_rate);
		}

		hisifd->dss_vote_cmd.dss_pri_clk_rate = vote_cmd.dss_pri_clk_rate;
	}

	HISI_FB_DEBUG("set dss_pri_clk_rate = %llu.\n", vote_cmd.dss_pri_clk_rate);
	return ret;
}

/*lint -e712 -e838*/
int dpe_set_clk_rate(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd;
	struct hisi_panel_info *pinfo;
	struct dss_vote_cmd *pdss_vote_cmd;
	uint64_t dss_pri_clk_rate;
	uint64_t dss_mmbuf_rate;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(hisifd);
	if (NULL == pdss_vote_cmd) {
		HISI_FB_ERR("NULL Pointer!\n");
		return -EINVAL;
	}

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

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ret = clk_set_rate(hisifd->dss_pxl0_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);
			if (g_fpga_flag == 0) {
				return -EINVAL;
			}
		}

		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n",
				pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && !hisifd->panel_info.fake_external) {
		if (hisifd->dp_pxl_ppll7_init != NULL) {
			ret = hisifd->dp_pxl_ppll7_init(hisifd, pinfo->pxl_clk_rate);
		} else {
			ret = clk_set_rate(hisifd->dss_pxl1_clk, pinfo->pxl_clk_rate);
		}

		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);

			if (g_fpga_flag == 0) {
				return -EINVAL;
			}
		}
		HISI_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl1_clk));
	} else {
		;
	}

	dss_mmbuf_rate = pdss_vote_cmd->dss_mmbuf_rate;
	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		if (hisifd_list[EXTERNAL_PANEL_IDX]) {
			if (hisifd_list[EXTERNAL_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate > dss_mmbuf_rate) {
				dss_mmbuf_rate = hisifd_list[EXTERNAL_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate;
			}
		}
		ret = clk_set_rate(hisifd->dss_mmbuf_clk, dss_mmbuf_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, dss_mmbuf_rate, ret);
			return -EINVAL;
		}
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX) || (hisifd->index == EXTERNAL_PANEL_IDX)) {
		if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
			HISI_FB_INFO("dss_mmbuf_clk:[%llu]->[%llu].\n",
				dss_mmbuf_rate, (uint64_t)clk_get_rate(hisifd->dss_mmbuf_clk));
		}
		HISI_FB_INFO("dss_pri_clk:[%llu]->[%llu].\n",
			dss_pri_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pri_clk));
	}

	return ret;
}

#define PERI_VOLTAGE_LEVEL0_065V		(0) // 0.65v
#define PERI_VOLTAGE_LEVEL1_075V		(2) // 0.75v
#define PERI_VOLTAGE_LEVEL2_080V		(3) // 0.80v
int dpe_get_voltage_value(dss_vote_cmd_t *vote_cmd)
{
	if (!vote_cmd) {
		HISI_FB_ERR("vote_cmd is null\n");
		return -1;
	}

	switch (vote_cmd->dss_voltage_level) {
		case PERI_VOLTAGE_LEVEL0:
			return PERI_VOLTAGE_LEVEL0_065V; // 0.65v
		case PERI_VOLTAGE_LEVEL1:
			return PERI_VOLTAGE_LEVEL1_075V; // 0.75v
		case PERI_VOLTAGE_LEVEL2:
			return PERI_VOLTAGE_LEVEL2_080V; // 0.80v
		default:
			HISI_FB_ERR("not support dss_voltage_level is %d\n", vote_cmd->dss_voltage_level);
			return -1;
	}
}

int dpe_get_voltage_level(int votage_value)
{
	switch (votage_value) {
		case PERI_VOLTAGE_LEVEL0_065V: // 0.65v
			return PERI_VOLTAGE_LEVEL0;
		case PERI_VOLTAGE_LEVEL1_075V: // 0.75v
			return PERI_VOLTAGE_LEVEL1;
		case PERI_VOLTAGE_LEVEL2_080V: // 0.80v
			return PERI_VOLTAGE_LEVEL2;
		default:
			HISI_FB_ERR("not support votage_value is %d\n", votage_value);
			return PERI_VOLTAGE_LEVEL0;
	}
}

int dpe_set_pixel_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	uint64_t clk_rate;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL Pointer!\n");
		return -EINVAL;
	}

	if (g_fpga_flag == 1) {
		return 0;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_rate = DEFAULT_DSS_PXL0_CLK_RATE_POWER_OFF;
		ret = clk_set_rate(hisifd->dss_pxl0_clk, clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
			return -EINVAL;
		}
		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && !hisifd->panel_info.fake_external) {
		clk_rate = DEFAULT_DSS_PXL1_CLK_RATE_POWER_OFF;
		ret = clk_set_rate(hisifd->dss_pxl1_clk, clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n", hisifd->index, clk_rate, ret);
			return -EINVAL;
		}
		HISI_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n", clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl1_clk));
	} else {
		;
	}

	return ret;
}

int dpe_set_common_clk_rate_on_pll0(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint64_t clk_rate;
	struct peri_volt_poll *pvp = NULL;

	if (NULL == hisifd) {
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

	pvp = peri_volt_poll_get(DEV_DSS_VOLTAGE_ID, NULL);
	if (!pvp) {
		HISI_FB_ERR("get pvp failed!\n");
		return -EINVAL;
	}

	ret = peri_set_volt(pvp, 0); // 0.65v
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

static void dss_lp_set_reg(char __iomem *dss_base)
{
	if (NULL == dss_base) {
		HISI_FB_ERR("dss_base is null.\n");
		return;
	}
	/*core axi mmbuf*/
	outp32(dss_base + GLB_MODULE_CLK_SEL, 0x00000001);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_MODULE_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_MODULE_CLK_SEL, 0x00000000);
	/*axi*/
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF0_AIF + AIF_CLK_SEL1, 0x00000000);
	outp32(dss_base + DSS_SMMU_OFFSET + SMMU_LP_CTRL, 0x00000001);
	/*mmbuf*/
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL0, 0x00000000);
	outp32(dss_base + DSS_VBIF1_AIF + AIF_CLK_SEL1, 0x00000000);

	/*core*/
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_WCH0_FBCE_CREG_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_WCH1_FBCE_CREG_CTRL_GATE,  0x00000000);
	outp32(dss_base + DSS_MIF_OFFSET + MIF_CLK_CTL,  0x00000001);
	outp32(dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL1_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL2_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL3_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL4_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_CTL5_OFFSET + MCTL_CTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MCTL_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_MOD_CLK_SEL, 0x00000000);

	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + FBCD_CREG_FBCD_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH0_FBCE_CREG_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + CH_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_WCH1_FBCE_CREG_CTRL_GATE,  0x0000000c);
	outp32(dss_base + DSS_OVL0_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL2_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL1_OFFSET + OV8_CLK_SEL, 0x00000000);
	outp32(dss_base + DSS_OVL3_OFFSET + OV2_CLK_SEL, 0x00000000);
	outp32(dss_base + GLB_DSS_MEM_CTRL, 0x02605550);
}

static void dss_normal_set_reg(char __iomem *dss_base)
{
	if (NULL == dss_base) {
		HISI_FB_ERR("dss_base is null.\n");
		return;
	}
	//core/axi/mmbuf
	outp32(dss_base + DSS_CMDLIST_OFFSET + CMD_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_VG0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	if (g_fpga_flag == 1) {
		outp32(dss_base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000000);
	} else {
		outp32(dss_base + DSS_RCH_VG0_ARSR_OFFSET + ARSR2P_LB_MEM_CTRL, 0x00000008);
	}

	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + VPP_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);

	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_VG1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
		outp32(dss_base + DSS_RCH_VG0_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
		outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL, 0x88888888);
		outp32(dss_base + DSS_RCH_VG1_DMA_OFFSET + HFBCD_MEM_CTRL_1, 0x00000888);
	} else {
		outp32(dss_base + DSS_RCH_VG2_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
		outp32(dss_base + DSS_RCH_VG2_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x00000008);
	}
	outp32(dss_base + DSS_RCH_VG2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_G0_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);

	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_COEF_MEM_CTRL, 0x00000088);
	outp32(dss_base + DSS_RCH_G1_SCL_OFFSET + SCF_LB_MEM_CTRL, 0x0000008);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_G1_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);

	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D0_DMA_OFFSET + AFBCD_MEM_CTRL, 0x00008888);
	outp32(dss_base + DSS_RCH_D1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_RCH_D3_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);

	outp32(dss_base + DSS_WCH0_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + AFBCE_MEM_CTRL, 0x00000888);
	outp32(dss_base + DSS_WCH0_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL, 0x88888888);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + AFBCE_MEM_CTRL_1, 0x00000088);
	outp32(dss_base + DSS_WCH1_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		outp32(dss_base + DSS_WCH1_DMA_OFFSET + WCH_SCF_COEF_MEM_CTRL, 0x00000088);
		outp32(dss_base + DSS_WCH1_DMA_OFFSET+ WCH_SCF_LB_MEM_CTRL, 0x00000088);
		outp32(dss_base + GLB_DSS_MEM_CTRL, 0x02605550);
	} else {
		outp32(dss_base + DSS_WCH2_DMA_OFFSET + DMA_BUF_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_WCH2_DMA_OFFSET + ROT_MEM_CTRL, 0x00000008);
	}
}

void dss_inner_clk_common_enable(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *dss_base = NULL;
	int prev_refcount;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point\n.");
		return;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		return;
	}

	dss_base = hisifd->dss_base;

	down(&hisi_fb_dss_inner_clk_sem);

	prev_refcount = dss_inner_clk_refcount++;
	if (!prev_refcount && !fastboot_enable) {
		if (g_fpga_flag == 1) {
			dss_normal_set_reg(dss_base);
		} else {
			dss_lp_set_reg(dss_base);
		}
	}

	HISI_FB_DEBUG("fb%d, dss_inner_clk_refcount=%d\n",
		hisifd->index, dss_inner_clk_refcount);

	up(&hisi_fb_dss_inner_clk_sem);
}

void dss_inner_clk_common_disable(struct hisi_fb_data_type *hisifd)
{
	int new_refcount;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point\n.");
		return;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
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
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
	}

	if (fastboot_enable) {
		return ;
	}

	dss_base = hisifd->dss_base;
	if (g_fpga_flag == 1) {
		outp32(dss_base + DSS_IFBC_OFFSET + IFBC_MEM_CTRL, 0x00000088);
		outp32(dss_base + DSS_DSC_OFFSET + DSC_MEM_CTRL, 0x00000888);
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_DPP_DITHER_OFFSET + DITHER_MEM_CTRL_ES, 0x00000008);
	} else {
		/*pix0 clk*/
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_MODULE_CLK_SEL, 0xffffff06);
		outp32(dss_base + DSS_HI_ACE_OFFSET + DPE_RAMCLK_FUNC, 0x00000000);
		outp32(dss_base + DSS_DPP_OFFSET + DPP_CLK_SEL, 0x00000400);
		outp32(dss_base + DSS_IFBC_OFFSET + IFBC_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_DSC_OFFSET + DSC_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_LDI0_OFFSET + LDI_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_DBUF0_OFFSET + DBUF_CLK_SEL, 0x00000000);
	}
}

void dss_inner_clk_pdp_disable(struct hisi_fb_data_type *hisifd)
{
}

void dss_inner_clk_sdp_enable(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = NULL;
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
	}

	dss_base = hisifd->dss_base;
	if (g_fpga_flag == 1) {
		outp32(dss_base + DSS_LDI1_OFFSET + LDI_MEM_CTRL, 0x00000008);
		outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_MEM_CTRL, 0x00000008);
	} else {
		/*pix1 clk*/
		outp32(dss_base + DSS_LDI1_OFFSET + LDI_MODULE_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_LDI1_OFFSET + LDI_CLK_SEL, 0x00000000);
		outp32(dss_base + DSS_DBUF1_OFFSET + DBUF_CLK_SEL, 0x00000000);
	}
}

void dss_inner_clk_sdp_disable(struct hisi_fb_data_type *hisifd)
{
	int ret;
	uint64_t dss_mmbuf_rate;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
	}

	hisifd->dss_vote_cmd.dss_mmbuf_rate = DEFAULT_DSS_MMBUF_CLK_RATE_L1;

	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		if (hisifd_list[PRIMARY_PANEL_IDX]) {
			dss_mmbuf_rate = hisifd_list[PRIMARY_PANEL_IDX]->dss_vote_cmd.dss_mmbuf_rate;
			ret = clk_set_rate(hisifd->dss_mmbuf_clk, dss_mmbuf_rate);
			if (ret < 0) {
				HISI_FB_ERR("fb%d dss_mmbuf clk_set_rate(%llu) failed, error=%d!\n",
					hisifd->index, dss_mmbuf_rate, ret);
				return ;
			}
		}
	}
}

void init_dpp(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dpp_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!");
		return ;
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

void init_sbl(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *dpp_base = NULL;
	char __iomem *sbl_base = NULL;
	uint32_t tmp;

	if (hisifd == NULL) {
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (pinfo == NULL) {
		return;
	}

	if (pinfo->sbl_support != 1) {
		return;
	}

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_SBL)) {
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dpp_base = hisifd->dss_base + DSS_DPP_OFFSET;
		sbl_base = hisifd->dss_base + DSS_DPP_SBL_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}
	//mode 1
	//enable cabc, backlight mode 1
	outp32(sbl_base + SBL_REG_CALC_CONTROL, 0x100086);

	//enable ca
	outp32(sbl_base + SBL_REG_VC_VC_CONTROL_0, 0x03);

	//enable Iridix
	outp32(sbl_base + SBL_REG_VC_IRDX_CONTROL, 0x7);

	//res
	outp32(sbl_base + SBL_REG_FRMT_FRAME_DIMEN,((pinfo->yres & 0xffff)<< 16)|(pinfo->xres & 0xffff));

	//ambient light scale
	outp32(sbl_base + SBL_REG_CALC_AL, 0xffff);

	//back light scale
	outp32(sbl_base + SBL_REG_CALC_CALIBRATION_E_BACKLIGHT_SCALE, 0xffff0000);

	//input bl scale
	outp32(sbl_base + SBL_REG_CALC_AB_BL_KNEE2_BL_PANEL_MAX, 0x00ff0000);

	outp32(sbl_base + SBL_REG_CALC_SBC2_TF_STEP_SBC2_TF_ASYM, 0x01ffff);

	outp32(sbl_base + SBL_REG_MANUAL, 0x6000200);

	outp32(sbl_base + SBL_REG_CALC_BL_ATTEN_ALPHA_SBC1_TF_DEPTH, 0x10000);

	outp32(sbl_base + SBL_REG_CALC_SBC1_TF_STEP_SBC1_TF_ASYM, 0x10ffff);

	outp32(sbl_base + SBL_REG_CALC_SBC1_TF_ASYM_LOG_SBC2_TF_DEPTH, 0x00200000);

	//calibration
	outp32(sbl_base + SBL_REG_CALC_CALIBRATION_A_B, 0x005f0008);

	outp32(sbl_base + SBL_REG_CALC_CALIBRATION_C_D, 0x00000003);

	outp32(sbl_base + SBL_REG_CALC_CALIBRATION_E_BACKLIGHT_SCALE, 0xffff0007);

	//strength limit
	outp32(sbl_base + SBL_REG_CALC_GAIN_AA_TF_ASYM_STRENGTH_LIMIT, 0x2c00010);

	//AL temporal filter
	outp32(sbl_base + SBL_REG_CALC_AL_RTF_FILTER_A_7_TO_0, 0x00605025);

	outp32(sbl_base + SBL_REG_CALC_AL_RTF_FILTER_C_AB_AL_KNEE1, 0x012c0222);

	outp32(sbl_base + SBL_REG_VC_DTHR_CONTROL, 0x00);

	outp32(sbl_base + SBL_REG_CALC_GAIN_MIDDLE_CALC_BRIGHTPR, 0x800310);

	outp32(sbl_base + SBL_REG_VC_IRDX_CONTRAST, 0x90);

	outp32(sbl_base + SBL_REG_VC_IRDX_DARKENH, 0x3000);

	outp32(sbl_base + SBL_REG_VC_LOGO_TOP_LEFT, 0x0);

	for(tmp = 0; tmp < 33; tmp++){
		outp32(sbl_base + SBL_CALC_AL_CALIB_LUT_ADDR_I, tmp);
		outp32(sbl_base + SBL_CALC_AL_CALIB_LUT_DATA_W, sbl_al_calib_lut[tmp]);
	}
	for(tmp = 0; tmp < 32; tmp++){
		outp32(sbl_base + SBL_CALC_AL_CHANGE_LUT_ADDR_I, tmp);
		outp32(sbl_base + SBL_CALC_AL_CHANGE_LUT_DATA_W, s_calc_al_change_lut[tmp]);
	}

	outp32(dpp_base + DPP_SBL, 0x0);
}

static void init_dsc(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dsc_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dsc_panel_info *dsc = NULL;

	uint32_t dsc_en = 0;
	uint32_t pic_width = 0;
	uint32_t pic_height = 0;
	uint32_t chunk_size = 0;
	uint32_t groups_per_line = 0;
	uint32_t rbs_min = 0;
	uint32_t hrd_delay = 0;
	uint32_t target_bpp_x16 =0;
	uint32_t num_extra_mux_bits = 0;
	uint32_t slice_bits = 0;
	uint32_t final_offset = 0;
	uint32_t final_scale = 0;
	uint32_t nfl_bpg_offset = 0;
	uint32_t groups_total = 0;
	uint32_t slice_bpg_offset = 0;
	uint32_t scale_increment_interval = 0;
	uint32_t initial_scale_value = 0;
	uint32_t scale_decrement_interval = 0;
	uint32_t adjustment_bits =0;
	uint32_t adj_bits_per_grp = 0;
	uint32_t bits_per_grp = 0;
	uint32_t slices_per_line = 0;
	uint32_t pic_line_grp_num = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);
	dsc = &(pinfo->vesa_dsc);

	dsc_base = hisifd->dss_base + DSS_DSC_OFFSET;

	if ((pinfo->ifbc_type == IFBC_TYPE_VESA2X_SINGLE) ||
		(pinfo->ifbc_type == IFBC_TYPE_VESA3X_SINGLE)) {
		// dual_dsc_en = 0, dsc_if_bypass = 1, reset_ich_per_line = 0
		dsc_en = 0x5;
		pic_width = DSS_WIDTH(pinfo->xres);
		outp32(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_VESA_CLK_SEL, 0);
	} else {
		// dual_dsc_en = 1, dsc_if_bypass = 0, reset_ich_per_line = 1
		dsc_en = 0xb;
		pic_width = DSS_WIDTH(pinfo->xres / 2);
		outp32(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_VESA_CLK_SEL, 1);
	}

	pic_height =  DSS_HEIGHT(pinfo->yres);
	chunk_size = ROUND1((dsc->slice_width + 1) * dsc->bits_per_pixel, 8);

	groups_per_line = (dsc->slice_width + 3) / 3;
	rbs_min = dsc->rc_model_size - dsc->initial_offset + dsc->initial_xmit_delay * dsc->bits_per_pixel +
		groups_per_line * dsc->first_line_bpg_offset;
	hrd_delay = ROUND1(rbs_min, dsc->bits_per_pixel);

	target_bpp_x16 = dsc->bits_per_pixel * 16;
	slice_bits = 8 * chunk_size * (dsc->slice_height + 1);

	num_extra_mux_bits = 3 * (dsc->mux_word_size + (4 * dsc->bits_per_component + 4) - 2);
	while ((num_extra_mux_bits > 0) && ((slice_bits - num_extra_mux_bits) % dsc->mux_word_size))
		num_extra_mux_bits--;

	final_offset = dsc->rc_model_size - ((dsc->initial_xmit_delay * target_bpp_x16 + 8) >> 4) + num_extra_mux_bits; //4336(0x10f0)
	final_scale = 8 * dsc->rc_model_size / (dsc->rc_model_size - final_offset);
	nfl_bpg_offset = ROUND1(dsc->first_line_bpg_offset << OFFSET_FRACTIONAL_BITS, dsc->slice_height); //793(0x319)
	groups_total = groups_per_line * (dsc->slice_height + 1);
	slice_bpg_offset = ROUND1((1 << OFFSET_FRACTIONAL_BITS) *
		(dsc->rc_model_size - dsc->initial_offset + num_extra_mux_bits), groups_total); // 611(0x263)
	scale_increment_interval = (1 << OFFSET_FRACTIONAL_BITS) * final_offset /
		((final_scale - 9) * (nfl_bpg_offset + slice_bpg_offset)); // 903(0x387)

	initial_scale_value = 8 * dsc->rc_model_size / (dsc->rc_model_size - dsc->initial_offset);
	if (groups_per_line < initial_scale_value - 8)	{
		initial_scale_value = groups_per_line + 8;
	}

	if (initial_scale_value > 8) {
		scale_decrement_interval = groups_per_line / (initial_scale_value - 8);
	} else {
		scale_decrement_interval = 4095;
	}

	adjustment_bits = (8 - (dsc->bits_per_pixel * (dsc->slice_width + 1)) % 8) % 8;
	adj_bits_per_grp = dsc->bits_per_pixel * 3 - 3;
	bits_per_grp = dsc->bits_per_pixel * 3;
	slices_per_line = (pic_width > dsc->slice_width) ? 1 : 0;
	pic_line_grp_num = ((dsc->slice_width + 3)/3)*(slices_per_line+1)-1;

	set_reg(dsc_base + DSC_REG_DEFAULT, 0x1, 1, 0);

	// dsc_en
	set_reg(dsc_base + DSC_EN, dsc_en, 4, 0);

	// bits_per_component, convert_rgb, bits_per_pixel
	set_reg(dsc_base + DSC_CTRL, dsc->bits_per_component | (dsc->linebuf_depth << 4) | (dsc->block_pred_enable << 10) |
		(0x1 << 11) | (dsc->bits_per_pixel << 16), 26, 0);

	// pic_width, pic_height
	set_reg(dsc_base + DSC_PIC_SIZE, (pic_width << 16) | pic_height, 32, 0);

	// slice_width, slice_height
	set_reg(dsc_base + DSC_SLICE_SIZE, (dsc->slice_width << 16) | dsc->slice_height, 32, 0);

	// chunk_size
	set_reg(dsc_base + DSC_CHUNK_SIZE, chunk_size, 16, 0);

	// initial_xmit_delay, initial_dec_delay = hrd_delay -initial_xmit_delay
	set_reg(dsc_base + DSC_INITIAL_DELAY, dsc->initial_xmit_delay |
		((hrd_delay - dsc->initial_xmit_delay) << 16), 32, 0);

	// initial_scale_value, scale_increment_interval
	set_reg(dsc_base + DSC_RC_PARAM0, initial_scale_value | (scale_increment_interval << 16), 32, 0);

	// scale_decrement_interval, first_line_bpg_offset
	set_reg(dsc_base + DSC_RC_PARAM1, (dsc->first_line_bpg_offset << 16) | scale_decrement_interval, 21, 0);

	// nfl_bpg_offset, slice_bpg_offset
	set_reg(dsc_base + DSC_RC_PARAM2, nfl_bpg_offset | (slice_bpg_offset << 16), 32, 0);

	//DSC_RC_PARAM3
	set_reg(dsc_base + DSC_RC_PARAM3,
		((final_offset << 16) | dsc->initial_offset), 32, 0);

	//DSC_FLATNESS_QP_TH
	set_reg(dsc_base + DSC_FLATNESS_QP_TH,
		((dsc->flatness_max_qp << 16) | (dsc->flatness_min_qp << 0)), 24, 0);

	//DSC_RC_PARAM4
	set_reg(dsc_base + DSC_RC_PARAM4,
		((dsc->rc_edge_factor << 20) | (dsc->rc_model_size << 0)), 24, 0);
	//DSC_RC_PARAM5
	set_reg(dsc_base + DSC_RC_PARAM5,
		((dsc->rc_tgt_offset_lo << 20) |(dsc->rc_tgt_offset_hi << 16) |
		(dsc->rc_quant_incr_limit1 << 8) |(dsc->rc_quant_incr_limit0 << 0)), 24, 0);

	//DSC_RC_BUF_THRESH
	set_reg(dsc_base + DSC_RC_BUF_THRESH0,
		((dsc->rc_buf_thresh0 << 24) | (dsc->rc_buf_thresh1 << 16) |
		(dsc->rc_buf_thresh2 << 8) | (dsc->rc_buf_thresh3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH1,
		((dsc->rc_buf_thresh4 << 24) | (dsc->rc_buf_thresh5 << 16) |
		(dsc->rc_buf_thresh6 << 8) | (dsc->rc_buf_thresh7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH2,
		((dsc->rc_buf_thresh8 << 24) | (dsc->rc_buf_thresh9 << 16) |
		(dsc->rc_buf_thresh10 << 8) | (dsc->rc_buf_thresh11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_BUF_THRESH3,
		((dsc->rc_buf_thresh12 << 24) | (dsc->rc_buf_thresh13 << 16)), 32, 0);

	//DSC_RC_RANGE_PARAM
	set_reg(dsc_base + DSC_RC_RANGE_PARAM0,
		((dsc->range_min_qp0 << 27) | (dsc->range_max_qp0 << 22) |
		(dsc->range_bpg_offset0 << 16) | (dsc->range_min_qp1 << 11) |
		(dsc->range_max_qp1 << 6) | (dsc->range_bpg_offset1 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM1,
		((dsc->range_min_qp2 << 27) | (dsc->range_max_qp2 << 22) |
		(dsc->range_bpg_offset2 << 16) | (dsc->range_min_qp3 << 11) |
		(dsc->range_max_qp3 << 6) | (dsc->range_bpg_offset3 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM2,
		((dsc->range_min_qp4 << 27) | (dsc->range_max_qp4 << 22) |
		(dsc->range_bpg_offset4 << 16) | (dsc->range_min_qp5 << 11) |
		(dsc->range_max_qp5 << 6) | (dsc->range_bpg_offset5 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM3,
		((dsc->range_min_qp6 << 27) | (dsc->range_max_qp6 << 22) |
		(dsc->range_bpg_offset6 << 16) | (dsc->range_min_qp7 << 11) |
		(dsc->range_max_qp7 << 6) | (dsc->range_bpg_offset7 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM4,
		((dsc->range_min_qp8 << 27) | (dsc->range_max_qp8 << 22) |
		(dsc->range_bpg_offset8 << 16) | (dsc->range_min_qp9 << 11) |
		(dsc->range_max_qp9 << 6) | (dsc->range_bpg_offset9 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM5,
		((dsc->range_min_qp10 << 27) | (dsc->range_max_qp10 << 22) |
		(dsc->range_bpg_offset10 << 16) | (dsc->range_min_qp11 << 11) |
		(dsc->range_max_qp11 << 6) | (dsc->range_bpg_offset11 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM6,
		((dsc->range_min_qp12 << 27) | (dsc->range_max_qp12 << 22) |
		(dsc->range_bpg_offset12 << 16) | (dsc->range_min_qp13 << 11) |
		(dsc->range_max_qp13 << 6) | (dsc->range_bpg_offset13 << 0)), 32, 0);
	set_reg(dsc_base + DSC_RC_RANGE_PARAM7,
		((dsc->range_min_qp14 << 27) | (dsc->range_max_qp14 << 22) |
		(dsc->range_bpg_offset14 << 16)), 32, 0);

	// adjustment_bits
	set_reg(dsc_base + DSC_ADJUSTMENT_BITS, adjustment_bits, 4, 0);

	// bits_per_grp, adj_bits_per_grp
	set_reg(dsc_base + DSC_BITS_PER_GRP, bits_per_grp | (adj_bits_per_grp << 8), 14, 0);

	//slices_per_line, pic_line_grp_num
	set_reg(dsc_base + DSC_MULTI_SLICE_CTL, slices_per_line |
		(pic_line_grp_num << 16), 32, 0);

	//dsc_out_mode
	if ((chunk_size % 3 == 0)) {
		set_reg(dsc_base + DSC_OUT_CTRL, 0x0, 1, 0);
	} else if ((chunk_size % 2 == 0)) {
		set_reg(dsc_base + DSC_OUT_CTRL, 0x1, 1, 0);
	} else {
		HISI_FB_ERR("fb%d, chunk_size should be mode by 3 or 2, but chunk_size = %u\n",
			hisifd->index, chunk_size);
		return;
	}

	set_reg(dsc_base + DSC_CLK_SEL, 0x0, 32, 0);
	set_reg(dsc_base + DSC_CLK_EN, 0x7, 32, 0);
	set_reg(dsc_base + DSC_MEM_CTRL, 0x0, 32, 0);
	set_reg(dsc_base + DSC_ST_DATAIN, 0x0, 28, 0);
	set_reg(dsc_base + DSC_ST_DATAOUT, 0x0, 16, 0);
	set_reg(dsc_base + DSC0_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_SLC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC1_ST_PIC_POS, 0x0, 28, 0);
	set_reg(dsc_base + DSC0_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC1_ST_FIFO, 0x0, 14, 0);
	set_reg(dsc_base + DSC0_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC1_ST_LINEBUF, 0x0, 24, 0);
	set_reg(dsc_base + DSC_ST_ITFC, 0x0, 10, 0);
	set_reg(dsc_base + DSC_RD_SHADOW_SEL, 0x1, 1, 0);
	set_reg(dsc_base + DSC_REG_DEFAULT, 0x0, 1, 0);
}

void init_ifbc(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ifbc_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	uint32_t mipi_idx;
	uint32_t comp_mode;

	uint32_t ifbc_out_mode = 0;
	uint32_t dpk_mode_sel = 0;
	uint32_t dup_mode_sel = 0;
	uint32_t porch_num = 0;
	uint32_t insert_byte_num = 0;
	uint32_t insert_byte = 0;
	uint32_t num_pad = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is null.\n");
		return;
	}
	pinfo = &(hisifd->panel_info);
	if (pinfo->ifbc_type >= IFBC_TYPE_MAX) {
		HISI_FB_ERR("ifbc_type is larger than IFBC_TYPE_MAX.\n");
		return;
	}

	/* VESA_CLK_SEL is set to 0 for initial, 1 is needed only by vesa dual pipe compress */
	set_reg(hisifd->dss_base + DSS_LDI0_OFFSET + LDI_VESA_CLK_SEL, 0, 1, 0);

	if (pinfo->ifbc_type == IFBC_TYPE_NONE)
		return ;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_IFBC))
		return;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ifbc_base = hisifd->dss_base + DSS_IFBC_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return ;
	}

	mipi_idx = is_dual_mipi_panel(hisifd) ? 1 : 0;
	comp_mode = g_mipi_ifbc_division[mipi_idx][pinfo->ifbc_type].comp_mode;

	if (is_ifbc_vesa_panel(hisifd)) {
		init_dsc(hisifd);

		// select comp_mode
		set_reg(ifbc_base + IFBC_CTRL, comp_mode, 3, 0);
		return ;
	}

	// git ifbc_out_mode
	if (pinfo->bpp == LCD_RGB888) {
		ifbc_out_mode = 1;
	} else if (pinfo->bpp == LCD_RGB565) {
		ifbc_out_mode = 0;
	}

	if (((pinfo->ifbc_type == IFBC_TYPE_ORISE2X) && (pinfo->ifbc_cmp_dat_rev0 == 1)) ||
		((pinfo->ifbc_type == IFBC_TYPE_RSP3X) &&
		(pinfo->type != PANEL_MIPI_VIDEO) &&
		(pinfo->xres % 3 != 0)))
		if (pinfo->ifbc_auto_sel != 0) {
			HISI_FB_ERR("fb%d, auto_sel = %u not support!", hisifd->index, pinfo->ifbc_auto_sel);
			return ;
		}

	if (pinfo->ifbc_type == IFBC_TYPE_ORISE2X) {
		if ((pinfo->xres % 2 != 0) &&
			(pinfo->yres % 2 != 0)) {
			HISI_FB_ERR("fb%d, IFBC_ORISE2X not support (xres = %u, yres = %u)!",
				hisifd->index, pinfo->xres, pinfo->yres);
			return;
		}

		dpk_mode_sel = 1;
		dup_mode_sel = 2;
		porch_num = 0;
	} else if (pinfo->ifbc_type == IFBC_TYPE_ORISE3X) {
		dpk_mode_sel = 0;
		dup_mode_sel = 3;
		porch_num = 5;
	} else if (pinfo->ifbc_type == IFBC_TYPE_HIMAX2X) {
		if ((pinfo->xres % 2 != 0) &&
			(pinfo->yres % 2 != 0)) {
			HISI_FB_ERR("fb%d, IFBC_HIMAX2X not support (xres = %u, yres = %u)!",
				hisifd->index, pinfo->xres, pinfo->yres);
			return;
		}

		dpk_mode_sel = 1;
		dup_mode_sel = 2;
		porch_num = 0;
	} else if (pinfo->ifbc_type == IFBC_TYPE_RSP2X) {
		dpk_mode_sel = 1;
		dup_mode_sel = 0;
		porch_num = 0;

		if ((pinfo->type == PANEL_MIPI_CMD) ||
			(pinfo->type == PANEL_DUAL_MIPI_CMD))
			num_pad = (4 - pinfo->xres % 4) % 4;
		else
			num_pad = 0;
	} else if (pinfo->ifbc_type == IFBC_TYPE_RSP3X) {
		if ((pinfo->yres % 2 != 0) || (pinfo->yres < 8)) {
			HISI_FB_ERR("fb%d, IFBC_RSP3X not support (xres = %u, yres = %u)!",
				hisifd->index, pinfo->xres, pinfo->yres);
			return;
		}

		dpk_mode_sel = 2;
		dup_mode_sel = 1;
		porch_num = 0;

		insert_byte = DSS_WIDTH(pinfo->xres) / (mipi_idx + 1) + 1;

		if ((pinfo->type == PANEL_MIPI_VIDEO) ||
			(pinfo->type == PANEL_DUAL_MIPI_VIDEO)) {
			insert_byte_num = (3 - insert_byte % 3) % 3;
			num_pad = (8 - pinfo->xres % 8) % 8;
		} else if ((pinfo->type == PANEL_MIPI_CMD) ||
			(pinfo->type == PANEL_DUAL_MIPI_CMD)) {
			insert_byte_num = 0;
			num_pad = (4 - pinfo->xres % 4) % 4;
		}
	}

	// reg_default
	//set_reg(ifbc_base + IFBC_REG_DEFAULT, 0x1, 1, 0);

	// ifbc_rd_shadow = 1
	//set_reg(ifbc_base + IFBC_RD_SHADOW, 0x1, 1, 0);

	// set ifbc_size
	set_reg(ifbc_base + IFBC_SIZE,
		((DSS_WIDTH(pinfo->xres) << 16) | DSS_HEIGHT(pinfo->yres)), 32, 0);

	// set IFBC_CTRL
	set_reg(ifbc_base + IFBC_CTRL, comp_mode, 3, 0);
	set_reg(ifbc_base + IFBC_CTRL, ifbc_out_mode, 1, 3);
	set_reg(ifbc_base + IFBC_CTRL, pinfo->ifbc_cmp_dat_rev0, 1, 4);
	set_reg(ifbc_base + IFBC_CTRL, pinfo->ifbc_cmp_dat_rev1, 1, 5);
	set_reg(ifbc_base + IFBC_CTRL, pinfo->ifbc_auto_sel, 1, 6);

	if (pinfo->ifbc_auto_sel == 0) {
		set_reg(ifbc_base + IFBC_CTRL, dpk_mode_sel, 3, 7);
		set_reg(ifbc_base + IFBC_CTRL, dup_mode_sel, 3, 10);
		set_reg(ifbc_base + IFBC_CTRL, porch_num, 8, 13);

		// set insert_byte_num, insert_byte
		set_reg(ifbc_base + IFBC_INSERT, insert_byte_num | (insert_byte << 16), 32, 0);

		// set num_pad
		set_reg(ifbc_base + IFBC_PAD, num_pad, 3, 0);
	}

	//orise3x initialization
	if (pinfo->ifbc_type == IFBC_TYPE_ORISE3X) {
		if (pinfo->ifbc_orise_ctr == 1) {
			set_reg(ifbc_base + IFBC_CORE_GT, 0x0, 2, 0);

			if (pinfo->ifbc_orise_ctl == IFBC_ORISE_CTL_8LINE)
				set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0208, 32, 0);
			else if (pinfo->ifbc_orise_ctl == IFBC_ORISE_CTL_16LINE)
				set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0210, 32, 0);
			else if (pinfo->ifbc_orise_ctl == IFBC_ORISE_CTL_32LINE)
				set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0220, 32, 0);
			else
				set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0200, 32, 0);

			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0300, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0419, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0500, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x063f, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0700, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0801, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0900, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0a64, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0b00, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0c5c, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0d00, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0e01, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x0f00, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x10a0, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x1100, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x125f, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x1300, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x14a0, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x1500, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x16ff, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x1700, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x200c, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x2100, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4000, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4100, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4200, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4300, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4800, 32, 0);
			set_reg(ifbc_base + IFBC_ORISE_CTL, 0x4900, 32, 0);
		} else {
			set_reg(ifbc_base + IFBC_CORE_GT, 0x2, 2, 0);
		}
	}

	// set ifbc_clk_sel
	set_reg(ifbc_base + IFBC_CLK_SEL, 0x0, 32, 0);

	// ifbc_rd_shadow = 1
	//set_reg(ifbc_base + IFBC_RD_SHADOW, 0x0, 1, 0);

	//enable ifbc
	set_reg(ifbc_base + IFBC_EN, 0x3, 2, 0);
}

/*lint -e438 -e550*/
void init_post_scf(struct hisi_fb_data_type *hisifd)
{
	char __iomem *scf_lut_base;
	char __iomem *scf_base;
	int ihright;
	int ihright1;
	int ivbottom;

	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	pinfo = &(hisifd->panel_info);

	scf_lut_base = hisifd->dss_base + DSS_POST_SCF_LUT_OFFSET;

	if (!HISI_DSS_SUPPORT_DPP_MODULE_BIT(DPP_MODULE_POST_SCF)) {
		return;
	}

	pinfo->post_scf_support = 1;
	hisi_dss_post_scl_load_filter_coef(hisifd, false, scf_lut_base, SCL_COEF_RGB_IDX);

	scf_base = hisifd->dss_base + DSS_POST_SCF_OFFSET;

	outp32(scf_base + ARSR_POST_SKIN_THRES_Y, 332<<10 | 300);//0x5312c
	outp32(scf_base + ARSR_POST_SKIN_THRES_U, 40<<10 | 20);//0xa014
	outp32(scf_base + ARSR_POST_SKIN_THRES_V, 48<<10 | 24);//0xc018
	outp32(scf_base + ARSR_POST_SKIN_EXPECTED, 580<<20 | 452<<10 | 600);//0x24471258
	outp32(scf_base + ARSR_POST_SKIN_CFG, 12<<16 | 10<<8 | 6);//0xc0a06
	outp32(scf_base + ARSR_POST_SHOOT_CFG1, 8<<16 | 20);//0x80014
	outp32(scf_base + ARSR_POST_SHOOT_CFG2, (-64 & 0x1ff));//0x1c0
	outp32(scf_base + ARSR_POST_SHOOT_CFG3, 512);//0x200
	outp32(scf_base + ARSR_POST_SHARP_CFG1_H, 256<<16 | 192);//0x10000c0
	outp32(scf_base + ARSR_POST_SHARP_CFG1_L, 24<<16 | 8);//0x180008
	outp32(scf_base + ARSR_POST_SHARP_CFG2_H, 256<<16 | 192);//0x10000c0
	outp32(scf_base + ARSR_POST_SHARP_CFG2_L, 24<<16 | 8);//0x180008
	outp32(scf_base + ARSR_POST_SHARP_CFG3, 150<<16 | 150);//0x960096
	outp32(scf_base + ARSR_POST_SHARP_CFG4, 200<<16 | 0);//0xc80000
	outp32(scf_base + ARSR_POST_SHARP_CFG5, 200<<16 | 0);//0xc80000
	outp32(scf_base + ARSR_POST_SHARP_CFG6, 16<<16 | 6);//0x100006
	outp32(scf_base + ARSR_POST_SHARP_CFG6_CUT, 160<<16 | 96);//0xa00060
	outp32(scf_base + ARSR_POST_SHARP_CFG7, 1<<17 | 4);//0x20004
	outp32(scf_base + ARSR_POST_SHARP_CFG7_RATIO, 160<<16 | 16);//0xa00010
	outp32(scf_base + ARSR_POST_SHARP_CFG8, 3<<22 | 800);//0xc00320
	outp32(scf_base + ARSR_POST_SHARP_CFG9, 8<<22 | 12800);//0x2003200
	outp32(scf_base + ARSR_POST_SHARP_CFG10, 800);//320
	outp32(scf_base + ARSR_POST_SHARP_CFG11, 20 << 22 | 12800);//0x5003200
	outp32(scf_base + ARSR_POST_DIFF_CTRL, 20<<8 | 16);//0x1410
	outp32(scf_base + ARSR_POST_SKIN_SLOP_Y, 512);//0x200
	outp32(scf_base + ARSR_POST_SKIN_SLOP_U, 819);//0x333
	outp32(scf_base + ARSR_POST_SKIN_SLOP_V, 682);//0x2aa

	ihright1 = ((int)pinfo->xres - 1) * ARSR1P_INC_FACTOR;
	ihright = ihright1 + 2 * ARSR1P_INC_FACTOR;
	if (ihright >= ((int)pinfo->xres) * ARSR1P_INC_FACTOR) {
		ihright = ((int)pinfo->xres) * ARSR1P_INC_FACTOR - 1;
	}

	ivbottom = ((int)pinfo->yres - 1) * ARSR1P_INC_FACTOR;
	if (ivbottom >= ((int)pinfo->yres) * ARSR1P_INC_FACTOR) {
		ivbottom = ((int)pinfo->yres) * ARSR1P_INC_FACTOR - 1;
	}

	outp32(scf_base + ARSR_POST_IHLEFT, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT, ihright);
	outp32(scf_base + ARSR_POST_IHLEFT1, 0x0);
	outp32(scf_base + ARSR_POST_IHRIGHT1, ihright1);
	outp32(scf_base + ARSR_POST_IVTOP, 0x0);
	outp32(scf_base + ARSR_POST_IVBOTTOM, ivbottom);
	outp32(scf_base + ARSR_POST_IHINC, ARSR1P_INC_FACTOR);
	outp32(scf_base + ARSR_POST_IVINC, ARSR1P_INC_FACTOR);

	outp32(scf_base + ARSR_POST_MODE, 0x1);

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
	uint32_t thd_flux_req_sw_en = 1;

	int dfs_time = 0;
	int dfs_time_min = 0;
	int depth = 0;
	int dfs_ram = 0;

	if (NULL == hisifd) {
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
			dfs_ram = 0x0;
		} else {
			dfs_time_min = DFS_TIME_MIN;
			dfs_ram = 0xF00;
		}

		dfs_time = DFS_TIME;
		depth = DBUF0_DEPTH;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		dbuf_base = hisifd->dss_base + DSS_DBUF1_OFFSET;

		dfs_time = DFS_TIME;
		dfs_time_min = DFS_TIME_MIN;
		depth = DBUF1_DEPTH;
		dfs_ram = 0x1FF;
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

	thd_cg_out = (dfs_time * pinfo->pxl_clk_rate * pinfo->xres) /
		(((pinfo->ldi.h_pulse_width + pinfo->ldi.h_back_porch + pinfo->ldi.h_front_porch) * pinfo->pxl_clk_rate_div
		+ pinfo->xres) * 6 * 1000000UL);
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

	if ((g_dss_version_tag == FB_ACCEL_KIRIN970) && (g_fpga_flag==0)) {
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			if (pinfo->xres * pinfo->yres >= RES_4K_PHONE) {
				sram_valid_num = 2;
				thd_rqos_out = 0x32bf;
				thd_rqos_in = 0x2b22;
				thd_cg_out = 0x32bf;
				thd_cg_in = 0x33bf;
				thd_flux_req_befdfs_out = 0x2e93;
				thd_flux_req_befdfs_in = 0x19e0;
				thd_flux_req_aftdfs_out = 0x124c;
				thd_flux_req_aftdfs_in = 0x926;
			} else if (pinfo->xres * pinfo->yres >= RES_1440P) {
				sram_valid_num = 1;
				thd_rqos_out = 0x217f;
				thd_rqos_in = 0x1c78;
				thd_cg_out = 0x217f;
				thd_cg_in = 0x227f;
				thd_flux_req_befdfs_out = 0x1f0c;
				thd_flux_req_befdfs_in = 0x1140;
				thd_flux_req_aftdfs_out = 0xfcc;
				thd_flux_req_aftdfs_in = 0x7e6;
			} else { // for 1080p
				sram_valid_num = 1;
				thd_rqos_out = 0x217f;
				thd_rqos_in = 0x1c78;
				thd_cg_out = 0x217f;
				thd_cg_in = 0x227f;
				thd_flux_req_befdfs_out = 0x1f0c;
				thd_flux_req_befdfs_in = 0x1140;
				thd_flux_req_aftdfs_out = 0xfcc;
				thd_flux_req_aftdfs_in = 0x7e6;
			}
		} else {
			if (pinfo->xres * pinfo->yres >= RES_4K_PHONE) {
				sram_valid_num = 0;
				thd_rqos_out = 0x103f;
				thd_rqos_in = 0xdcf;
				thd_cg_out = 0x103f;
				thd_cg_in = 0x113f;
				thd_flux_req_befdfs_out = 0xf86;
				thd_flux_req_befdfs_in = 0x8a0;
				thd_flux_req_aftdfs_out = 0x773;
				thd_flux_req_aftdfs_in = 0x3b9;
			} else if (pinfo->xres * pinfo->yres >= RES_1440P) {
				sram_valid_num = 0;
				thd_rqos_out = 0x103f;
				thd_rqos_in = 0xdcf;
				thd_cg_out = 0x103f;
				thd_cg_in = 0x113f;
				thd_flux_req_befdfs_out = 0xf86;
				thd_flux_req_befdfs_in = 0x8a0;
				thd_flux_req_aftdfs_out = 0x773;
				thd_flux_req_aftdfs_in = 0x3b9;
			} else {
				sram_valid_num = 0;
				thd_rqos_out = 0x103f;
				thd_rqos_in = 0xdcf;
				thd_cg_out = 0x103f;
				thd_cg_in = 0x113f;
				thd_flux_req_befdfs_out = 0xf86;
				thd_flux_req_befdfs_in = 0x8a0;
				thd_flux_req_aftdfs_out = 0x773;
				thd_flux_req_aftdfs_in = 0x3b9;
			}
		}
	}

	outp32(dbuf_base + DBUF_FRM_SIZE, pinfo->xres * pinfo->yres);
	outp32(dbuf_base + DBUF_FRM_HSIZE, DSS_WIDTH(pinfo->xres));
	outp32(dbuf_base + DBUF_SRAM_VALID_NUM, sram_valid_num);

	outp32(dbuf_base + DBUF_THD_RQOS, (thd_rqos_out<< 16) | thd_rqos_in);
	outp32(dbuf_base + DBUF_THD_WQOS, (thd_wqos_out << 16) | thd_wqos_in);
	outp32(dbuf_base + DBUF_THD_CG, (thd_cg_out << 16) | thd_cg_in);
	outp32(dbuf_base + DBUF_THD_OTHER, (thd_cg_hold << 16) | thd_wr_wait);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_BEF, (thd_flux_req_befdfs_out << 16) | thd_flux_req_befdfs_in);
	outp32(dbuf_base + DBUF_THD_FLUX_REQ_AFT, (thd_flux_req_aftdfs_out << 16) | thd_flux_req_aftdfs_in);
	outp32(dbuf_base + DBUF_THD_DFS_OK, thd_dfs_ok);
	outp32(dbuf_base + DBUF_FLUX_REQ_CTRL, (dfs_ok_mask << 1) | thd_flux_req_sw_en);

	outp32(dbuf_base + DBUF_DFS_LP_CTRL, 0x1);
	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		outp32(dbuf_base + DBUF_DFS_RAM_MANAGE, dfs_ram);
	}
}
/*lint +e438 +e550*/

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

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == EXTERNAL_PANEL_IDX)
		return;

	ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

	ifbc_type = pinfo->ifbc_type;
	if ((ifbc_type  < IFBC_TYPE_NONE) || (ifbc_type >= IFBC_TYPE_MAX)) {
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

static void itf_ctrl_config(struct hisi_fb_data_type *hisifd, char __iomem *ldi_base)
{
	if ((NULL == hisifd) || (NULL == ldi_base)) {
		return;
	}

	if (is_mipi_cmd_panel(hisifd)) {
		set_reg(ldi_base + LDI_FRM_MSK,
			(hisifd->frame_update_flag == 1) ? 0x0 : 0x1, 1, 0);
	} else {
		set_reg(ldi_base + LDI_FRM_MSK, 0x0, 1, 0);
	}

	if (hisifd->index == EXTERNAL_PANEL_IDX && (is_mipi_panel(hisifd))) {
		set_reg(ldi_base + LDI_DP_DSI_SEL, 0x1, 1, 0);
	}

	return;
}

void init_ldi(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	char __iomem *ldi_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	dss_rect_t rect = {0,0,0,0};
	uint32_t te_source = 0;

	if (NULL == hisifd) {
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
		if (pinfo->pxl_clk_rate_div== 0) {
			HISI_FB_ERR("pxl_clk_rate_div is NULL, not support !\n");
			pinfo->pxl_clk_rate_div = 1;
		}
		set_reg(ldi_base + LDI_DSI_TE_VS_WD,
			(0x3FC << 12) | (2 * pinfo->pxl_clk_rate / pinfo->pxl_clk_rate_div / 1000000), 32, 0);
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

	itf_ctrl_config(hisifd, ldi_base);

	if (!fastboot_enable) {
		set_reg(ldi_base + LDI_CTRL, 0x0, 1, 0);
	}

	if (pinfo->dpi01_exchange_flag == 1){
		set_reg(ldi_base + LDI_DPI_SET, 0x01, 1, 0);
	}
	HISI_FB_DEBUG("-.!\n");
}

void deinit_ldi(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;

	if (NULL == hisifd) {
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

	if (NULL == hisifd) {
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

	if (NULL == hisifd) {
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

//set pixel clock to the exact value which is larger than 288M
int dpe_recover_pxl_clock(struct hisi_fb_data_type *hisifd)
{
	(void)hisifd;
	return 0;
}

void ldi_frame_update(struct hisi_fb_data_type *hisifd, bool update)
{
	char __iomem *ldi_base = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

		if (is_mipi_cmd_panel(hisifd)) {
			set_reg(ldi_base + LDI_FRM_MSK, (update ? 0x0 : 0x1), 1, 0);
			if (update) {
				set_reg(ldi_base + LDI_CTRL, 0x1, 1, 0);
			}
		}
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
	}
}

void single_frame_update(struct hisi_fb_data_type *hisifd)
{
	char __iomem *ldi_base = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return;
	}
	pinfo = &(hisifd->panel_info);

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

void dpe_interrupt_clear(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t clear = 0;

	if (NULL == hisifd) {
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
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INTS, clear);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_DSS_GLB_INTS, clear);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_SDP_INTS, clear);
		outp32(dss_base + DSS_LDI1_OFFSET + LDI_CPU_ITF_INTS, clear);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		clear = ~0;
		outp32(dss_base + GLB_CPU_OFF_INTS, clear);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		clear = ~0;
		outp32(hisifd->media_common_base + GLB_CPU_OFF_INTS, clear);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_unmask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t unmask = 0;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
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

		if (pinfo->hiace_support == 1 && g_dss_version_tag == FB_ACCEL_KIRIN970)
			unmask &= ~(BIT_HIACE_IND);

		outp32(dss_base + DSS_DPP_OFFSET + DPP_INT_MSK, unmask);

	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		unmask = ~0;
		//unmask &= ~(BIT_SDP_ITF1_INTS  | BIT_SDP_DSS_GLB_INTS | BIT_SDP_MMU_IRPT_NS);
		unmask &= ~(BIT_SDP_ITF1_INTS | BIT_SDP_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, unmask);

		unmask = ~0;
		if (is_mipi_cmd_panel(hisifd)) {
			unmask &= ~(BIT_LCD_TE0_PIN | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		} else {
			unmask &= ~(BIT_VSYNC | BIT_VACTIVE0_START | BIT_VACTIVE0_END | BIT_FRM_END);
		}
		outp32(dss_base +  DSS_LDI1_OFFSET + LDI_CPU_ITF_INT_MSK, unmask);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH0_INTS | BIT_OFF_WCH1_INTS | BIT_OFF_WCH0_WCH1_FRM_END_INT | BIT_OFF_MMU_IRPT_NS);
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, unmask);
		unmask = ~0;
		unmask &= ~(BIT_OFF_CAM_WCH2_FRMEND_INTS);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, unmask);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		unmask = ~0;
		unmask &= ~(BIT_OFF_WCH1_INTS);

		outp32(hisifd->media_common_base + GLB_CPU_OFF_INT_MSK, unmask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void dpe_interrupt_mask(struct hisi_fb_data_type *hisifd)
{
	char __iomem *dss_base = 0;
	uint32_t mask = 0;

	if (NULL == hisifd) {
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
		outp32(dss_base + DSS_DBG_OFFSET + DBG_WCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH0_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH1_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH2_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH3_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH4_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH5_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH6_INT_MSK, mask);
		outp32(dss_base + DSS_DBG_OFFSET + DBG_RCH7_INT_MSK, mask);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_SDP_INT_MSK, mask);
		outp32(dss_base + DSS_LDI1_OFFSET + LDI_CPU_ITF_INT_MSK, mask);
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		mask = ~0;
		outp32(dss_base + GLB_CPU_OFF_INT_MSK, mask);
		outp32(dss_base + GLB_CPU_OFF_CAM_INT_MSK, mask);
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		mask = ~0;
		outp32(hisifd->media_common_base + GLB_CPU_OFF_INT_MSK, mask);
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

}

void ldi_data_gate(struct hisi_fb_data_type *hisifd, bool enble)
{
	char __iomem *ldi_base = NULL;

	if (NULL == hisifd) {
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

/* dpp csc config */
#define CSC_ROW	(3)
#define CSC_COL	(5)

/*
** Rec.601 for Computer
** [ p00 p01 p02 cscidc2 cscodc2 ]
** [ p10 p11 p12 cscidc1 cscodc1 ]
** [ p20 p21 p22 cscidc0 cscodc0 ]
*/
static int CSC10B_YUV2RGB709_WIDE[CSC_ROW][CSC_COL] = {
	{0x4000, 0x00000, 0x064ca, 0x000, 0x000},
	{0x4000, 0x1f403, 0x1e20a, 0x600, 0x000},
	{0x4000, 0x076c2, 0x00000, 0x600, 0x000},
};

static int CSC10B_RGB2YUV709_WIDE[CSC_ROW][CSC_COL] = {
	{0x00d9b, 0x02dc6, 0x0049f, 0x000, 0x000},
	{0x1f8ab, 0x1e755, 0x02000, 0x000, 0x200},
	{0x02000, 0x1e2ef, 0x1fd11, 0x000, 0x200},
};

static void init_csc10b(struct hisi_fb_data_type *hisifd, char __iomem * dpp_csc10b_base)
{
	int (*csc_coe)[CSC_COL];

	if (hisifd == NULL || dpp_csc10b_base == NULL) {
		HISI_FB_ERR("hisifd or dpp_csc10b_base is NULL!\n");
		return;
	}

	if (dpp_csc10b_base == (hisifd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET)) {
		csc_coe = CSC10B_RGB2YUV709_WIDE;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x2);
	} else if (dpp_csc10b_base == (hisifd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET)) {
		csc_coe = CSC10B_YUV2RGB709_WIDE;
		outp32(dpp_csc10b_base + CSC10B_MPREC, 0x0);
	} else {
		return;
	}

	outp32(dpp_csc10b_base + CSC10B_IDC0, csc_coe[2][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC1, csc_coe[1][3]);
	outp32(dpp_csc10b_base + CSC10B_IDC2, csc_coe[0][3]);
	outp32(dpp_csc10b_base + CSC10B_ODC0, csc_coe[2][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC1, csc_coe[1][4]);
	outp32(dpp_csc10b_base + CSC10B_ODC2, csc_coe[0][4]);
	outp32(dpp_csc10b_base + CSC10B_P00, csc_coe[0][0]);
	outp32(dpp_csc10b_base + CSC10B_P01, csc_coe[0][1]);
	outp32(dpp_csc10b_base + CSC10B_P02, csc_coe[0][2]);
	outp32(dpp_csc10b_base + CSC10B_P10, csc_coe[1][0]);
	outp32(dpp_csc10b_base + CSC10B_P11, csc_coe[1][1]);
	outp32(dpp_csc10b_base + CSC10B_P12, csc_coe[1][2]);
	outp32(dpp_csc10b_base + CSC10B_P20, csc_coe[2][0]);
	outp32(dpp_csc10b_base + CSC10B_P21, csc_coe[2][1]);
	outp32(dpp_csc10b_base + CSC10B_P22, csc_coe[2][2]);

	outp32(dpp_csc10b_base + CSC10B_MODULE_EN, 0x1);
}

void init_dpp_csc(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("init_dpp_csc hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);

	if (pinfo->acm_support || pinfo->arsr1p_sharpness_support || pinfo->post_scf_support) {
		// init csc10b rgb2yuv
		init_csc10b(hisifd, hisifd->dss_base + DSS_DPP_CSC_RGB2YUV10B_OFFSET);
		// init csc10b yuv2rgb
		init_csc10b(hisifd, hisifd->dss_base + DSS_DPP_CSC_YUV2RGB10B_OFFSET);
	}
}

void acm_set_lut(char __iomem *address, uint32_t table[], uint32_t size)
{
	uint32_t data = 0;
	uint32_t index = 0;
	uint32_t i = 0;

	size /= 4;

	for (i = 0; i < size; i++) {
		index = i << 2;
		data = table[index] + (table[index + 1] << 8) + (table[index + 2] << 16) + (table[index + 3] << 24);
		outp32(address + (i << 2), data);
	}
}

void acm_set_lut_hue(char __iomem *address, uint32_t table[], uint32_t size)
{
	uint32_t data;
	uint32_t index;
	uint32_t i;

	size /= 2;

	for (i = 0; i < size; i++) {
		index = i << 1;
		data = table[index] + (table[index + 1] << 16);//lint !e679
		outp32(address + (i << 2), data);
	}
}//lint !e550 !e715
static void acm_set_lut_lh(char __iomem *address, uint32_t table[], uint32_t size)
{
	uint32_t data;
	uint32_t index;
	uint32_t i;

	size /= 2;

	for (i = 0; i < size; i++) {
		index = i << 1;
		data = (table[index] & 0x1FFF) | ((table[index + 1] & 0x1FFF) << 16);
		outp32(address + (i << 2), data);
	}
}//lint !e550 !e715
static void acm_set_lut_table(char __iomem *acm_lut_base, struct hisi_panel_info *pinfo, uint32_t index)
{
	if (pinfo->acm_lut_hue_table && pinfo->acm_lut_hue_table_len > 0) {
		acm_set_lut_hue(acm_lut_base + ACM_U_H_COEF, pinfo->acm_lut_hue_table, pinfo->acm_lut_hue_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_hue_table is NULL or hue_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_sata_table && pinfo->acm_lut_sata_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATA_COEF, pinfo->acm_lut_sata_table, pinfo->acm_lut_sata_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_sata_table is NULL or sata_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr0_table && pinfo->acm_lut_satr0_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR0_COEF, pinfo->acm_lut_satr0_table, pinfo->acm_lut_satr0_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr0_table is NULL or satr0_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr1_table && pinfo->acm_lut_satr1_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR1_COEF, pinfo->acm_lut_satr1_table, pinfo->acm_lut_satr1_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr1_table is NULL or satr1_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr2_table && pinfo->acm_lut_satr2_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR2_COEF, pinfo->acm_lut_satr2_table, pinfo->acm_lut_satr2_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr2_table is NULL or satr2_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr3_table && pinfo->acm_lut_satr3_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR3_COEF, pinfo->acm_lut_satr3_table, pinfo->acm_lut_satr3_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr3_table is NULL or satr3_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr4_table && pinfo->acm_lut_satr4_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR4_COEF, pinfo->acm_lut_satr4_table, pinfo->acm_lut_satr4_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr4_table is NULL or satr4_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr5_table && pinfo->acm_lut_satr5_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR5_COEF, pinfo->acm_lut_satr5_table, pinfo->acm_lut_satr5_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr5_table is NULL or satr5_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr6_table && pinfo->acm_lut_satr6_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR6_COEF, pinfo->acm_lut_satr6_table, pinfo->acm_lut_satr6_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr6_table is NULL or satr6_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_satr7_table && pinfo->acm_lut_satr7_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_SATR7_COEF, pinfo->acm_lut_satr7_table, pinfo->acm_lut_satr7_table_len);
	} else {
		HISI_FB_ERR("fb%d, acm_lut_satr7_table is NULL or satr7_table_len less than 0!\n", index);
		return;
	}

	return;
}
static void acm_set_lut_LTx_table(char __iomem *acm_lut_base, struct hisi_panel_info *pinfo, uint32_t index)
{
	if (pinfo->acm_lut_satr_face_table && pinfo->acm_lut_satr_face_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_SATR_FACE_COEF, pinfo->acm_lut_satr_face_table, pinfo->acm_lut_satr_face_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_satr_face_table is NULL or acm_lut_satr_face_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lta_table && pinfo->acm_lut_lta_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTA_COEF, pinfo->acm_lut_lta_table, pinfo->acm_lut_lta_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lta_table is NULL or acm_lut_lta_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr0_table && pinfo->acm_lut_ltr0_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR0_COEF, pinfo->acm_lut_ltr0_table, pinfo->acm_lut_ltr0_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr0_table is NULL or acm_lut_ltr0_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr1_table && pinfo->acm_lut_ltr1_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR1_COEF, pinfo->acm_lut_ltr1_table, pinfo->acm_lut_ltr1_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr1_table is NULL or acm_lut_ltr1_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr2_table && pinfo->acm_lut_ltr2_table_len> 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR2_COEF, pinfo->acm_lut_ltr2_table, pinfo->acm_lut_ltr2_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr2_table is NULL or acm_lut_ltr2_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr3_table && pinfo->acm_lut_ltr3_table_len> 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR3_COEF, pinfo->acm_lut_ltr3_table, pinfo->acm_lut_ltr3_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr3_table is NULL or acm_lut_ltr3_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr4_table && pinfo->acm_lut_ltr4_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR4_COEF, pinfo->acm_lut_ltr4_table, pinfo->acm_lut_ltr4_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr4_table is NULL or acm_lut_ltr4_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr5_table && pinfo->acm_lut_ltr5_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR5_COEF, pinfo->acm_lut_ltr5_table, pinfo->acm_lut_ltr5_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr5_table is NULL or acm_lut_ltr5_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr6_table && pinfo->acm_lut_ltr6_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR6_COEF, pinfo->acm_lut_ltr6_table, pinfo->acm_lut_ltr6_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr6_table is NULL or acm_lut_ltr6_table_len less than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ltr7_table && pinfo->acm_lut_ltr7_table_len > 0) {
		acm_set_lut(acm_lut_base + ACM_U_ACM_LTR7_COEF, pinfo->acm_lut_ltr7_table, pinfo->acm_lut_ltr7_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ltr4_table is NULL or acm_lut_ltr4_table_len less than 0!\n", index);
		return;
	}

	return;
}
static void acm_set_lut_LHx_table(char __iomem *acm_lut_base, struct hisi_panel_info *pinfo, uint32_t index)
{
	if (pinfo->acm_lut_lh0_table && pinfo->acm_lut_lh0_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH0_COFF, pinfo->acm_lut_lh0_table, pinfo->acm_lut_lh0_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh0_table is NULL or acm_lut_lh0_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh1_table && pinfo->acm_lut_lh1_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH1_COFF, pinfo->acm_lut_lh1_table, pinfo->acm_lut_lh1_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh1_table is NULL or acm_lut_lh1_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh2_table && pinfo->acm_lut_lh2_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH2_COFF, pinfo->acm_lut_lh2_table, pinfo->acm_lut_lh2_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh2_table is NULL or acm_lut_lh2_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh3_table && pinfo->acm_lut_lh3_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH3_COFF, pinfo->acm_lut_lh3_table, pinfo->acm_lut_lh3_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh3_table is NULL or acm_lut_lh3_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh4_table && pinfo->acm_lut_lh4_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH4_COFF, pinfo->acm_lut_lh4_table, pinfo->acm_lut_lh4_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh4_table is NULL or acm_lut_lh4_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh5_table && pinfo->acm_lut_lh5_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH5_COFF, pinfo->acm_lut_lh5_table, pinfo->acm_lut_lh5_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh5_table is NULL or acm_lut_lh5_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh6_table && pinfo->acm_lut_lh6_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH6_COFF, pinfo->acm_lut_lh6_table, pinfo->acm_lut_lh6_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh6_table is NULL or acm_lut_lh6_table_lenless than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_lh7_table && pinfo->acm_lut_lh7_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_LH7_COFF, pinfo->acm_lut_lh7_table, pinfo->acm_lut_lh7_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_lh7_table is NULL or acm_lut_lh7_table_lenless than 0!\n", index);
		return;
	}

	return;
}
static void acm_set_lut_CHx_table(char __iomem *acm_lut_base, struct hisi_panel_info *pinfo, uint32_t index)
{
	if (pinfo->acm_lut_ch0_table && pinfo->acm_lut_ch0_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH0_COFF, pinfo->acm_lut_ch0_table, pinfo->acm_lut_ch0_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch0_table is NULL or acm_lut_ch0_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch1_table && pinfo->acm_lut_ch1_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH1_COFF, pinfo->acm_lut_ch1_table, pinfo->acm_lut_ch1_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch1_table is NULL or acm_lut_ch1_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch2_table && pinfo->acm_lut_ch2_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH2_COFF, pinfo->acm_lut_ch2_table, pinfo->acm_lut_ch2_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch2_table is NULL or acm_lut_ch2_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch3_table && pinfo->acm_lut_ch3_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH3_COFF, pinfo->acm_lut_ch3_table, pinfo->acm_lut_ch3_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch3_table is NULL or acm_lut_ch3_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch4_table && pinfo->acm_lut_ch4_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH4_COFF, pinfo->acm_lut_ch4_table, pinfo->acm_lut_ch4_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch4_table is NULL or acm_lut_ch4_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch5_table && pinfo->acm_lut_ch5_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH5_COFF, pinfo->acm_lut_ch5_table, pinfo->acm_lut_ch5_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch5_table is NULL or acm_lut_ch5_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch6_table && pinfo->acm_lut_ch6_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH6_COFF, pinfo->acm_lut_ch6_table, pinfo->acm_lut_ch6_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch6_table is NULL or acm_lut_ch6_table_len than 0!\n", index);
		return;
	}

	if (pinfo->acm_lut_ch7_table && pinfo->acm_lut_ch7_table_len > 0) {
		acm_set_lut_lh(acm_lut_base + ACM_U_ACM_CH7_COFF, pinfo->acm_lut_ch7_table, pinfo->acm_lut_ch7_table_len);
	} else {
		HISI_FB_INFO("fb%d, acm_lut_ch7_table is NULL or acm_lut_ch7_table_len than 0!\n", index);
		return;
	}
	return;
}
void init_acm(struct hisi_fb_data_type *hisifd)
{
	char __iomem *acm_base = NULL;
	char __iomem *acm_lut_base = NULL;
	struct hisi_panel_info *pinfo = NULL;
	uint32_t lut_sel = 0;

	if (hisifd == NULL)	{
		HISI_FB_DEBUG("init_acm hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	acm_base = hisifd->dss_base + DSS_DPP_ACM_OFFSET;

	if (pinfo->acm_support != 1) {
		outp32(acm_base + ACM_MEM_CTRL_ES, 0x4);
		HISI_FB_DEBUG("fb%d, not support acm!\n", hisifd->index);
		return;
	}

	acm_lut_base = hisifd->dss_base + DSS_DPP_ACM_LUT_OFFSET;
	set_reg(acm_base + ACM_SATA_OFFSET, 0x20, 6, 0);
	//Rec.709(wide):
	set_reg(acm_base + ACM_CSC_IDC0, 0x600, 11, 0);
	set_reg(acm_base + ACM_CSC_IDC1, 0x600, 11, 0);
	set_reg(acm_base + ACM_CSC_IDC2, 0x0, 11, 0);

	set_reg(acm_base + ACM_CSC_P00, 0x4000, 17, 0);
	set_reg(acm_base + ACM_CSC_P01, 0x0, 17, 0);
	set_reg(acm_base + ACM_CSC_P02, 0x64CA, 17, 0);
	set_reg(acm_base + ACM_CSC_P10, 0x4000, 17, 0);
	set_reg(acm_base + ACM_CSC_P11, 0x1F403, 17, 0);
	set_reg(acm_base + ACM_CSC_P12, 0x1E20A, 17, 0);
	set_reg(acm_base + ACM_CSC_P20, 0x4000, 17, 0);
	set_reg(acm_base + ACM_CSC_P21, 0x76C2, 17, 0);
	set_reg(acm_base + ACM_CSC_P22, 0x0, 17, 0);

	set_reg(acm_base + ACM_HUE_RLH01, (((pinfo->r1_lh & 0x3ff) << 16) | (pinfo->r0_lh & 0x3ff)), 26, 0);
	set_reg(acm_base + ACM_HUE_RLH23, (((pinfo->r3_lh & 0x3ff) << 16) | (pinfo->r2_lh & 0x3ff)), 26, 0);
	set_reg(acm_base + ACM_HUE_RLH45, (((pinfo->r5_lh & 0x3ff) << 16) | (pinfo->r4_lh & 0x3ff)), 26, 0);
	set_reg(acm_base + ACM_HUE_RLH67, (((pinfo->r6_hh & 0x3ff) << 16)| (pinfo->r6_lh & 0x3ff)), 26, 0);//needconfirm
	set_reg(acm_base + ACM_HUE_PARAM01, ((0x200 << 16) | (0x200)), 32, 0);
	set_reg(acm_base + ACM_HUE_PARAM23, ((0x1FC << 16) | 0x200), 32, 0);
	set_reg(acm_base + ACM_HUE_PARAM45, ((0x204 << 16) | (0x200)), 32, 0);
	set_reg(acm_base + ACM_HUE_PARAM67, ((0x200 << 16) | (0x200)), 32, 0);
	set_reg(acm_base + ACM_HUE_SMOOTH0, 0x0040003F, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH1, 0x00C000BF, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH2, 0x0140013F, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH3, 0x01C001BF, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH4, 0x02410240, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH5, 0x02C102C0, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH6, 0x0340033F, 26, 0); //needconfirm
	set_reg(acm_base + ACM_HUE_SMOOTH7, 0x03C003BF, 26, 0); //needconfirm
	set_reg(acm_base + ACM_COLOR_CHOOSE, 1, 1, 0); //needconfirm
	//ACM RGB2YUV
	set_reg(acm_base + ACM_RGB2YUV_IDC0, 0x00000200, 11, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_IDC1, 0x00000200, 11, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_IDC2, 0x00000000, 11, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P00, 0x00000D9B, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P01, 0x00002DC6, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P02, 0x0000049F, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P10, 0x0001F8AB, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P11, 0x0001E755, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P12, 0x00002000, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P20, 0x00002000, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P21, 0x0001E2EF, 17, 0);//needconfirm
	set_reg(acm_base + ACM_RGB2YUV_P22, 0x0001FD11, 17, 0);//needconfirm
	//ACM FACE
	set_reg(acm_base + ACM_FACE_CRTL, 0x01180118, 32, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_STARTXY, 0x004600DC, 29, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_LEN01, 0x00100010, 29, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_LEN23, 0x00100010, 29, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM0, 0x00010000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM1, 0x00010000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM2, 0x00010000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM3, 0x00010000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM4, 0x00001000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM5, 0x00001000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM6, 0x00001000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SMOOTH_PARAM7, 0x00001000, 20, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_AREA_SEL, 0x00000002, 3, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SAT_LH, 0x02AE0000, 26, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SAT_SMOOTH_LH, 0x02D60000, 26, 0);//needconfirm
	set_reg(acm_base + ACM_FACE_SAT_SMO_PARAM_LH, 0x06660001, 16, 0);//needconfirm
	//ACM L CONTRAST
	set_reg(acm_base + ACM_L_CONT_EN, 0x00000000, 1, 0);//needconfirm
	set_reg(acm_base + ACM_LC_PARAM01, 0x020401FC, 16, 0);//needconfirm
	set_reg(acm_base + ACM_LC_PARAM23, 0x02000200, 16, 0);//needconfirm
	set_reg(acm_base + ACM_LC_PARAM45, 0x020801F8, 16, 0);//needconfirm
	set_reg(acm_base + ACM_LC_PARAM67, 0x020401FC, 16, 0);//needconfirm
	//ACM L ADJ
	set_reg(acm_base + ACM_L_ADJ_CTRL, 0, 9, 0);//needconfirm
	//ACM CAPTURE
	set_reg(acm_base + ACM_CAPTURE_CTRL, 0, 32, 0);//needconfirm
	set_reg(acm_base + ACM_CAPTURE_IN, 0, 30, 0);//needconfirm
	set_reg(acm_base + ACM_CAPTURE_OUT, 0, 30, 0);//needconfirm
	//ACM INK
	set_reg(acm_base + ACM_INK_CTRL, 0, 32, 0);//needconfirm
	set_reg(acm_base + ACM_INK_OUT, 0, 30, 0);//needconfirm

	acm_set_lut_table(acm_lut_base, pinfo, hisifd->index);
	acm_set_lut_LTx_table(acm_lut_base, pinfo, hisifd->index);
	acm_set_lut_LHx_table(acm_lut_base, pinfo, hisifd->index);
	acm_set_lut_CHx_table(acm_lut_base, pinfo, hisifd->index);

	lut_sel = inp32(acm_base + ACM_LUT_SEL);
	set_reg(acm_base + ACM_LUT_SEL, (~lut_sel) & 0x7F80, 16, 0);

	//not enable acm
	set_reg(acm_base + ACM_EN, 0x0, 1, 0);

	g_acm_State = 1;
	/*acm reg dimming init*/
	hisi_effect_color_dimming_acm_reg_init(hisifd);
}
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
	char __iomem *gamma_pre_lut_base = NULL;

	if (hisifd == NULL)	{
		HISI_FB_ERR("init_degmma_xcc_gmp hisifd is NULL!\n");
		return;
	}

	pinfo = &(hisifd->panel_info);
	gamma_lut_base = hisifd->dss_base + DSS_DPP_GAMA_LUT_OFFSET;
	if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
		gamma_pre_lut_base = hisifd->dss_base + DSS_DPP_GAMA_PRE_LUT_OFFSET;
	}

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
				if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
					//GAMA PRE LUT
					outp32(gamma_pre_lut_base + (U_GAMA_PRE_R_COEF + i * 4), pinfo->gamma_lut_table_R[index] | pinfo->gamma_lut_table_R[index+1] << 16 );
					outp32(gamma_pre_lut_base + (U_GAMA_PRE_G_COEF + i * 4), pinfo->gamma_lut_table_G[index] | pinfo->gamma_lut_table_G[index+1] << 16 );
					outp32(gamma_pre_lut_base + (U_GAMA_PRE_B_COEF + i * 4), pinfo->gamma_lut_table_B[index] | pinfo->gamma_lut_table_B[index+1] << 16 );
				}
			}
			outp32(gamma_lut_base + U_GAMA_R_LAST_COEF, pinfo->gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_G_LAST_COEF, pinfo->gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
			outp32(gamma_lut_base + U_GAMA_B_LAST_COEF, pinfo->gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);
			if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
				//GAMA PRE LUT
				outp32(gamma_pre_lut_base + U_GAMA_PRE_R_LAST_COEF, pinfo->gamma_lut_table_R[pinfo->gamma_lut_table_len - 1]);
				outp32(gamma_pre_lut_base + U_GAMA_PRE_G_LAST_COEF, pinfo->gamma_lut_table_G[pinfo->gamma_lut_table_len - 1]);
				outp32(gamma_pre_lut_base + U_GAMA_PRE_B_LAST_COEF, pinfo->gamma_lut_table_B[pinfo->gamma_lut_table_len - 1]);
			}
		}
	}
}
void init_igm_gmp_xcc_gm(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *lcp_base = NULL;
	char __iomem *xcc_base = NULL;
	char __iomem *gmp_base = NULL;
	char __iomem *degamma_base = NULL;
	char __iomem *gmp_lut_base = NULL;
	char __iomem *gamma_base = NULL;
	uint32_t i = 0;
	uint32_t gama_lut_sel = 0;
	uint32_t degama_lut_sel = 0;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;
	uint32_t gmp_lut_sel;

	if (hisifd == NULL)	{
		HISI_FB_ERR("init_degmma_xcc_gmp hisifd is NULL!\n");
		return;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		lcp_base = hisifd->dss_base + DSS_DPP_LCP_OFFSET_ES;
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
		gmp_base = hisifd->dss_base + DSS_DPP_GMP_OFFSET;
		degamma_base = hisifd->dss_base + DSS_DPP_DEGAMMA_OFFSET;
		gmp_lut_base = hisifd->dss_base + DSS_DPP_GMP_LUT_OFFSET;
		gamma_base = hisifd->dss_base + DSS_DPP_GAMA_OFFSET;
	} else {
		HISI_FB_ERR("fb%d, not support!\n", hisifd->index);
		return;
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
	//Degamma
	if (pinfo->gamma_support == 1) {
		//disable degamma
		set_reg(degamma_base + DEGAMA_EN, 0x0, 1, 0);

		degamma_set_lut(hisifd);
		if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
			degama_lut_sel = (uint32_t)inp32(degamma_base + DEGAMA_LUT_SEL);
			set_reg(degamma_base + DEGAMA_LUT_SEL, (~(degama_lut_sel & 0x1)) & 0x1, 1, 0);
		}

		//enable degama
		set_reg(degamma_base + DEGAMA_EN, 0x1, 1, 0);
	} else {
		//degama memory shutdown
		outp32(degamma_base + DEGAMA_MEM_CTRL, 0x4);
	}

	//XCC
	if (pinfo->xcc_support == 1) {
		// XCC matrix
		if (pinfo->xcc_table_len ==xcc_cnt_cofe && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);

			//enable xcc
			set_reg(xcc_base + XCC_EN, 0x1, 1, 0);
			//not enable pre xcc
			//set_reg(xcc_base + XCC_EN, 0x0, 1, 1);
		}
	}

	//GMP
	if (pinfo->gmp_support == 1) {
		//disable gmp
		set_reg(gmp_base + GMP_EN, 0x0, 1, 0);

		//gmp lut
		if (pinfo->gmp_lut_table_len == gmp_cnt_cofe
			&& pinfo->gmp_lut_table_low32bit
			&& pinfo->gmp_lut_table_high4bit) {
			for (i = 0; i < gmp_cnt_cofe; i++) {
				outp32(gmp_lut_base + i * 2 * 4, pinfo->gmp_lut_table_low32bit[i]);
				outp32(gmp_lut_base + i * 2 * 4 + 4, pinfo->gmp_lut_table_high4bit[i]);
			}
			gmp_lut_sel = (uint32_t)inp32(gmp_base + GMP_LUT_SEL);
			set_reg(gmp_base + GMP_LUT_SEL, (~(gmp_lut_sel & 0x1)) & 0x1, 1, 0);
			//not enable gmp
			set_reg(gmp_base + GMP_EN, 0x0, 1, 0);
			g_gmp_State = 1;
		}
	} else {
		//gmp memory shutdown
		outp32(gmp_base + GMP_MEM_CTRL, 0x4);
	}

	//GAMMA
	if (pinfo->gamma_support == 1) {
		//disable gamma
		set_reg(gamma_base + GAMA_EN, 0x0, 1, 0);
		//set gama lut
		gamma_set_lut(hisifd);
		if (g_dss_version_tag == FB_ACCEL_KIRIN970) {
			gama_lut_sel = (uint32_t)inp32(gamma_base + GAMA_LUT_SEL);
			set_reg(gamma_base + GAMA_LUT_SEL, (~(gama_lut_sel & 0x1)) & 0x1, 1, 0);
		}

		//enable gamma
		set_reg(gamma_base + GAMA_EN, 0x1, 1, 0);
	} else {
		//gama memory shutdown
		outp32(gamma_base + GAMA_MEM_CTRL, 0x4);
	}
}

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

	set_reg(dither_base + DITHER_CTL1, 0x00000024, 6, 0);
	set_reg(dither_base + DITHER_CTL0, 0x0000001A, 5, 0);
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

	set_reg(dither_base + DITHER_HIFREQ_REG_INI_CFG_EN, 0x00000000, 1, 0);
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

	set_reg(dither_base + DITHER_FRC_CTL, 0x00000000, 4, 0);
	set_reg(dither_base + DITHER_FRC_01_PART1, 0xFFFF0000, 32, 0);
	set_reg(dither_base + DITHER_FRC_01_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_10_PART0, 0x00000000, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART1, 0xFFFFFFFF, 32, 0);
	set_reg(dither_base + DITHER_FRC_11_PART0, 0xFFFF0000, 32, 0);
}
//lint +e838 +e550 +e438

void dpe_store_ct_cscValue(struct hisi_fb_data_type *hisifd, unsigned int csc_value[])
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
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
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
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
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
			hisifd->color_temperature_flag = 2;
		}
	}

	return 0;
}

ssize_t dpe_show_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == hisifd) {
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

int dpe_set_xcc_cscValue(struct hisi_fb_data_type *hisifd)
{
	return 0;
}

int dpe_set_comform_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
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
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
		}
	}

	return 0;
}

ssize_t dpe_show_comform_ct_cscValue(struct hisi_fb_data_type *hisifd, char *buf)
{
	struct hisi_panel_info *pinfo = NULL;
	if (NULL == hisifd) {
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

int dpe_set_led_rg_ct_cscValue(struct hisi_fb_data_type *hisifd)
{
	struct hisi_panel_info *pinfo = NULL;
	char __iomem *xcc_base = NULL;
	uint32_t color_temp_rectify_R = 32768, color_temp_rectify_G = 32768, color_temp_rectify_B = 32768;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		xcc_base = hisifd->dss_base + DSS_DPP_XCC_OFFSET;
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
	if (1 == g_is_led_rg_csc_set && 1 == pinfo->xcc_support) {
		HISI_FB_DEBUG("real set color temperature: g_is_led_rg_csc_set = %d, R = 0x%x, G = 0x%x, B = 0x%x .\n",
				g_is_led_rg_csc_set, g_led_rg_csc_value[0], g_led_rg_csc_value[4], g_led_rg_csc_value[8]);
		// XCC matrix
		if (pinfo->xcc_table_len > 0 && pinfo->xcc_table) {
			outp32(xcc_base + XCC_COEF_00, pinfo->xcc_table[0]);
			outp32(xcc_base + XCC_COEF_01, pinfo->xcc_table[1]
				* g_led_rg_csc_value[0] / 32768 * color_temp_rectify_R / 32768);
			outp32(xcc_base + XCC_COEF_02, pinfo->xcc_table[2]);
			outp32(xcc_base + XCC_COEF_03, pinfo->xcc_table[3]);
			outp32(xcc_base + XCC_COEF_10, pinfo->xcc_table[4]);
			outp32(xcc_base + XCC_COEF_11, pinfo->xcc_table[5]);
			outp32(xcc_base + XCC_COEF_12, pinfo->xcc_table[6]
				* g_led_rg_csc_value[4] / 32768 * color_temp_rectify_G / 32768);
			outp32(xcc_base + XCC_COEF_13, pinfo->xcc_table[7]);
			outp32(xcc_base + XCC_COEF_20, pinfo->xcc_table[8]);
			outp32(xcc_base + XCC_COEF_21, pinfo->xcc_table[9]);
			outp32(xcc_base + XCC_COEF_22, pinfo->xcc_table[10]);
			outp32(xcc_base + XCC_COEF_23, pinfo->xcc_table[11]
				* g_led_rg_csc_value[8] / 32768 * DISCOUNT_COEFFICIENT(g_comform_value) / CHANGE_MAX
				* color_temp_rectify_B / 32768);
		}
	}

	return 0;
}

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

	if(value == hisifd->panel_info.gamma_type) {
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

ssize_t dpe_show_acm_state(char *buf)
{
	ssize_t ret = 0;

	if (NULL == buf) {
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

	if (NULL == buf) {
		HISI_FB_ERR("NULL Pointer!\n");
		return 0;
	}

	ret = snprintf(buf, PAGE_SIZE, "g_gmp_State = %d\n", g_gmp_State);

	return ret;
}

void dpe_sbl_set_al_bl(struct hisi_fb_data_type *hisifd)
{
	uint32_t temp;
	char __iomem *sbl_base = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd, NUll pointer warning.\n");
		return;
	}

	sbl_base = hisifd->dss_base + DSS_DPP_SBL_OFFSET;

	temp = ((uint32_t)(hisifd->sbl.sbl_backlight_h & 0xff) << 24) | ((uint32_t)(hisifd->sbl.sbl_backlight_l & 0xff) << 16)\
		| ((uint32_t)(hisifd->sbl.sbl_ambient_light_h & 0xff) << 8) | (hisifd->sbl.sbl_ambient_light_l & 0xff);
	set_reg(sbl_base + SBL_REG_AL_BL, temp, 8, 0);
	return;
}
/*lint +e838 +e679 +e712*/
