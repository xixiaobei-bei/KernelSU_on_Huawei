/* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 */

#include "hisi_fb.h"
#include "hisi_dpe_utils.h"
#include "hisi_overlay_utils.h"
#include "product/attrs/hisi_dpe_attrs.h"

#include "hisi_spr_dsc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

DEFINE_SEMAPHORE(hisi_fb_dss_regulator_sem);
static int dss_regulator_refcount = 0;
static int dss_regulator_ref_table[HISI_FB_MAX_FBI_LIST] = {0};
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
DEFINE_SEMAPHORE(hisi_fb_dss_bus_idle_ctrl_sem);
static int dss_bus_idle_ctrl_refcount = 0;
#endif


/*******************************************************************************
**
*/
//DSS_LDI0_OFFSET
static int dpe_init(struct hisi_fb_data_type *hisifd, bool fastboot_enable)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		init_post_scf(hisifd);
		init_dbuf(hisifd);
		init_dpp(hisifd);
		init_acm(hisifd);
		init_acm_ce(hisifd);
		init_dpp_csc(hisifd);
		init_hiace(hisifd);
		init_dither(hisifd);
		init_igm_gmp_xcc_gm(hisifd);
		init_ifbc(hisifd);
		init_ldi(hisifd, fastboot_enable);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		hisifb_activate_vsync(hisifd_list[PRIMARY_PANEL_IDX]);
		// dual lcd: dsi_mux_sel=1, dual mipi: dsi_mux_sel=0
		if (!is_dp_panel(hisifd)) {
			set_reg(hisifd->dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_DSI_MUX_SEL, 0x1, 1, 0);
		}
		hisifb_deactivate_vsync(hisifd_list[PRIMARY_PANEL_IDX]);

		init_dbuf(hisifd);
		init_ldi(hisifd, fastboot_enable);
	} else if ((hisifd->index == AUXILIARY_PANEL_IDX) || (hisifd->index == MEDIACOMMON_PANEL_IDX)) {
		;
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

	return 0;
}
/*lint -e559*/
static int dpe_deinit(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		deinit_ldi(hisifd);
	#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
		defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
		deinit_dbuf(hisifd);
		deinit_effect(hisifd);
	#endif
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		deinit_ldi(hisifd);
	#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
		defined(CONFIG_HISI_FB_V600)
		deinit_dbuf(hisifd);
	#endif
	} else if ((hisifd->index == AUXILIARY_PANEL_IDX) || (hisifd->index == MEDIACOMMON_PANEL_IDX)) {
		;
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
	}

	return 0;
}

static void dpe_check_itf_status(struct hisi_fb_data_type *hisifd)
{
	uint32_t tmp = 0;
	int delay_count = 0;
	bool is_timeout = true;
	int itf_idx = 0;
	char __iomem *mctl_sys_base = NULL;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return;
	}

	if ((hisifd->index == PRIMARY_PANEL_IDX) ||
		(hisifd->index == EXTERNAL_PANEL_IDX)) {
		itf_idx = hisifd->index;
		mctl_sys_base =  hisifd->dss_base + DSS_MCTRL_SYS_OFFSET;

		while (1) {
			tmp = inp32(mctl_sys_base + MCTL_MOD17_STATUS + itf_idx * 0x4);
			if (((tmp & 0x10) == 0x10) || delay_count > 100) {
				is_timeout = (delay_count > 100) ? true : false;
				break;
			} else {
				mdelay(1);
				++delay_count;
			}
		}

		if (is_timeout) {
			HISI_FB_DEBUG("mctl_itf%d not in idle status,ints=0x%x !\n", hisifd->index, tmp);
		}
	}
}

int dpe_irq_enable(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->dpe_irq) {
		enable_irq(hisifd->dpe_irq);
		HISI_FB_DEBUG("enable irq %d \n", hisifd->dpe_irq);
	}

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
	if ((PRIMARY_PANEL_IDX == hisifd->index) && hisifd->dsi0_irq) {
		enable_irq(hisifd->dsi0_irq);
		HISI_FB_DEBUG("enable irq %d \n", hisifd->dsi0_irq);
		if (is_dual_mipi_panel(hisifd) && hisifd->dsi1_irq) {
			enable_irq(hisifd->dsi1_irq);
			HISI_FB_DEBUG("enable irq %d \n", hisifd->dsi1_irq);
		}
	}
	if ((EXTERNAL_PANEL_IDX == hisifd->index) && hisifd->dsi1_irq &&
		!is_dp_panel(hisifd)) {
		enable_irq(hisifd->dsi1_irq);
		HISI_FB_DEBUG("enable irq %d \n", hisifd->dsi1_irq);
	}
#endif

	return 0;
}
/*lint +e559*/
int dpe_irq_disable(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->dpe_irq) {
		disable_irq(hisifd->dpe_irq);
		HISI_FB_DEBUG("disable irq %d \n", hisifd->dpe_irq);
	}

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
	if ((PRIMARY_PANEL_IDX == hisifd->index) && hisifd->dsi0_irq) {
		disable_irq(hisifd->dsi0_irq);
		HISI_FB_DEBUG("disable irq %d \n", hisifd->dsi0_irq);
		if (is_dual_mipi_panel(hisifd) && hisifd->dsi1_irq) {
			disable_irq(hisifd->dsi1_irq);
			HISI_FB_DEBUG("enable irq %d \n", hisifd->dsi1_irq);
		}
	}
	if ((EXTERNAL_PANEL_IDX == hisifd->index) && hisifd->dsi1_irq &&
		!is_dp_panel(hisifd)) {
		disable_irq(hisifd->dsi1_irq);
		HISI_FB_DEBUG("disable irq %d \n", hisifd->dsi1_irq);
	}
#endif

	/*disable_irq_nosync(hisifd->dpe_irq);*/

	return 0;
}

static int dpe_irq_disable_nosync(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->dpe_irq) {
		disable_irq_nosync(hisifd->dpe_irq);
	}

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
	if ((PRIMARY_PANEL_IDX == hisifd->index) && hisifd->dsi0_irq) {
		disable_irq_nosync(hisifd->dsi0_irq);
		if (is_dual_mipi_panel(hisifd) && hisifd->dsi1_irq)
			disable_irq_nosync(hisifd->dsi1_irq);
	}

	if ((EXTERNAL_PANEL_IDX == hisifd->index) && hisifd->dsi1_irq &&
		!is_dp_panel(hisifd)) {
		disable_irq_nosync(hisifd->dsi1_irq);
	}
#endif
	return 0;
}

int dpe_regulator_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	down(&hisi_fb_dss_regulator_sem);
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("get hisi_fb_dss_regulator_sem.\n");
	}

	ret = regulator_enable(hisifd->dpe_regulator->consumer);
	if (ret) {
		HISI_FB_ERR("fb%d dpe regulator_enable failed, error=%d!\n", hisifd->index, ret);
		goto regulator_sem;
	}

	dss_regulator_refcount++;
	dss_regulator_ref_table[hisifd->index]++;
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			hisifd->index,
			dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	#if !defined(CONFIG_HISI_FB_V510) && !defined(CONFIG_HISI_FB_V350) && \
		!defined(CONFIG_HISI_FB_V600)
	#endif
	} else if (dss_regulator_ref_table[PRIMARY_PANEL_IDX] == 0) {
		HISI_FB_INFO("fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			hisifd->index,
			dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	}

regulator_sem:
	up(&hisi_fb_dss_regulator_sem);

	return ret;
}

int dpe_regulator_disable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (hisifd->index >= HISI_FB_MAX_FBI_LIST)
	{
		HISI_FB_ERR("hisifd->index exceeds array limit\n");
		return -EINVAL;
	}

	down(&hisi_fb_dss_regulator_sem);

	dss_regulator_refcount--;
	dss_regulator_ref_table[hisifd->index]--;
	if (dss_regulator_refcount < 0) {
		HISI_FB_ERR("fb%d, dss_regulator_refcount=%d is error\n", hisifd->index, dss_regulator_refcount);
		goto regulator_sem;
	}

	dpe_set_pixel_clk_rate_on_pll0(hisifd);
	if (dss_regulator_refcount == 0) {
		dpe_set_common_clk_rate_on_pll0(hisifd);
	}

	ret = regulator_disable(hisifd->dpe_regulator->consumer);
	if (ret != 0) {
		HISI_FB_ERR("fb%d dpe regulator_disable failed, error=%d!\n", hisifd->index, ret);
		goto regulator_sem;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		HISI_FB_INFO("fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			hisifd->index,
			dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	} else if (dss_regulator_ref_table[PRIMARY_PANEL_IDX] == 0) {
		HISI_FB_INFO("fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			hisifd->index,
			dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	}

regulator_sem:
	up(&hisi_fb_dss_regulator_sem);

	return ret;
}

void mdc_regulator_enable(struct hisi_fb_data_type *hisifd)
{
	uint32_t mdc_core_clk = 0xffc02180;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return;
	}


	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		outp32(hisifd->media_crg_base + MEDIA_CLKDIV9, 0x08000800);
		outp32(hisifd->media_crg_base + MEDIA_PEREN0, 0x20000000);
		outp32(hisifd->media_crg_base + MEDIA_PEREN1, 0x01801000);
		outp32(hisifd->media_crg_base + MEDIA_PEREN1, 0x00006000);

		outp32(hisifd->media_crg_base + MEDIA_PERDIS0, 0x20000000);
		outp32(hisifd->media_crg_base + MEDIA_PERDIS1, 0x01806000);
		outp32(hisifd->media_crg_base + MEDIA_PERRSTDIS0, 0x001c4000);

		outp32(hisifd->media_crg_base + MEDIA_PEREN0, 0x20000000);
		outp32(hisifd->media_crg_base + MEDIA_PEREN1, 0x01806000);
		outp32(hisifd->media_crg_base + MEDIA_CLKDIV8, mdc_core_clk);
	}
}

void mdc_regulator_disable(struct hisi_fb_data_type *hisifd)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		outp32(hisifd->media_crg_base + MEDIA_PERRSTEN0, 0x001c4000);
		outp32(hisifd->media_crg_base + MEDIA_PERDIS0, 0x20000000);
		outp32(hisifd->media_crg_base + MEDIA_PERDIS1, 0x01806000);
		outp32(hisifd->media_crg_base + MEDIA_PERDIS1, 0x00001000);
		outp32(hisifd->media_crg_base + MEDIA_CLKDIV9, 0x08000000);
	}
}

int mediacrg_regulator_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	int curr_volt;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	down(&hisi_fb_dss_regulator_sem);

	ret = regulator_enable(hisifd->mediacrg_regulator->consumer);
	if (ret) {
		HISI_FB_ERR("fb%d mediacrg regulator_enable failed, error=%d!\n", hisifd->index, ret);
	}

	up(&hisi_fb_dss_regulator_sem);

	if (g_dss_perf_debug > 0) {
		HISI_FB_INFO("dss performance debug, level = %d", g_dss_perf_debug);
		(void)dss_set_peri_volt(g_dss_perf_debug - 1, &curr_volt);
		mdelay(1);
	}

	return ret;
}

int mediacrg_regulator_disable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (NULL == hisifd) {
		HISI_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	down(&hisi_fb_dss_regulator_sem);

	ret = regulator_disable(hisifd->mediacrg_regulator->consumer);
	if (ret != 0) {
		HISI_FB_ERR("fb%d mediacrg regulator_disable failed, error=%d!\n", hisifd->index, ret);
	}

	up(&hisi_fb_dss_regulator_sem);

	return ret;
}

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
static int hisifb_dss_bus_idle_req_config(struct hisi_fb_data_type *hisifd, bool exit_idle)
{
	int count = 0;
	u32 target_bit = 0;
	uint32_t idle_status;

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return -1;
	}

	if (exit_idle)
		outp32(hisifd->pmctrl_base + NOC_POWER_IDLEREQ, 0x20000000);
	else
		outp32(hisifd->pmctrl_base + NOC_POWER_IDLEREQ, 0x20002000);
	while (1) {
		if ((++count) > 100) {
			HISI_FB_ERR("fb%d, noc_dss_power_idlereq exit_idle[%d] fail, idle_status[0x%x], count[%d].\n",
				hisifd->index, exit_idle, idle_status, count);
			return -1;
		}

		idle_status = inp32(hisifd->pmctrl_base + NOC_POWER_IDLE);
		target_bit = BIT(13);
		if (exit_idle) {
			if (!(idle_status & target_bit)) {
				break;
			}
		} else {
			if (idle_status & target_bit) {
				break;
			}
		}
		udelay(2);
	}

	HISI_FB_DEBUG("fb%d, noc_dss_power_idlereq: exit_idle[%d], idle_status[0x%x], count[%d].\n",
			hisifd->index, exit_idle, idle_status, count);
	return 0;
}

static int hisifb_dss_bus_idle_req_handle(struct hisi_fb_data_type *hisifd, bool exit_idle)
{
	int ret = 0;
	static int refcount_dbg[4] = {0};

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return -1;
	}

	down(&hisi_fb_dss_bus_idle_ctrl_sem);
	if (exit_idle) {
		dss_bus_idle_ctrl_refcount++;
		refcount_dbg[(int)(hisifd->index)]++;
		if (!dss_regulator_refcount) {
			HISI_FB_INFO("primary panel on +,dss_regulator_refcount[%d], return\n", dss_regulator_refcount);
			up(&hisi_fb_dss_bus_idle_ctrl_sem);
			return 0;
		}

		if (dss_bus_idle_ctrl_refcount == 1) {
			ret = hisifb_dss_bus_idle_req_config(hisifd, true);
		}

	} else {
		dss_bus_idle_ctrl_refcount--;
		refcount_dbg[(int)(hisifd->index)]--;

		if (dss_bus_idle_ctrl_refcount == 0) {
			ret = hisifb_dss_bus_idle_req_config(hisifd, false);
		}

	}

	HISI_FB_DEBUG("fb%d exit_idle[%d], total refcount[%d], fb0[%d] fb1[%d] fb2[%d] fb3][%d] .\n",
		hisifd->index, exit_idle, dss_bus_idle_ctrl_refcount,
		refcount_dbg[0], refcount_dbg[1], refcount_dbg[2], refcount_dbg[3]);

	up(&hisi_fb_dss_bus_idle_ctrl_sem);
	return ret;
}
#endif

int dpe_common_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	// mmbuf_clk
	clk_tmp = hisifd->dss_mmbuf_clk;
	if (clk_tmp != NULL) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_mmbuf_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_mmbuf_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	// aclk
	clk_tmp = hisifd->dss_axi_clk;
	if (clk_tmp != NULL) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_axi_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_axi_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	// pclk
	clk_tmp = hisifd->dss_pclk_dss_clk;
	if (clk_tmp != NULL) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pclk_dss_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pclk_dss_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
	hisifb_dss_bus_idle_req_handle(hisifd, true);
#endif
	return 0;
}

#if   defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
static int dpe_pxl_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (!is_dp_panel(hisifd)) {
		return ret;
	}

	if (hisifd->dss_pxl1_clk) {
		ret = clk_prepare_enable(hisifd->dss_pxl1_clk);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pxl1_clk enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	return ret;
}
#else
static int dpe_pxl_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = hisifd->dss_pxl0_clk;
		if (clk_tmp != NULL) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pxl0_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pxl0_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		clk_tmp = hisifd->dss_pxl1_clk;
		if (clk_tmp != NULL) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pxl1_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_pxl1_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}
	}

	return ret;
}
#endif

int dpe_inner_clk_enable(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	// edc0_clk
	clk_tmp = hisifd->dss_pri_clk;
	if (clk_tmp != NULL) {
		ret = clk_prepare(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_prepare failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = clk_enable(clk_tmp);
		if (ret) {
			HISI_FB_ERR("fb%d dss_pri_clk clk_enable failed, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

	// ldi0_clk(pxl0_clk)    ldi1_clk(pxl1_clk)
	ret = dpe_pxl_clk_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("pxl clk enable failed, error=%d!\n", ret);
		return -EINVAL;
	}

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		clk_tmp = hisifd->dss_clk_media_common_clk;
		if (clk_tmp != NULL) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_clk_media_common_clk clk_prepare failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				HISI_FB_ERR("fb%d dss_clk_media_common_clk clk_enable failed, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}
		}
	} else {
		;
	}

	return 0;
}

int dpe_common_clk_disable(struct hisi_fb_data_type *hisifd)
{
	struct clk *clk_tmp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
	hisifb_dss_bus_idle_req_handle(hisifd, false);
#endif

	clk_tmp = hisifd->dss_pclk_dss_clk;
	if (clk_tmp != NULL) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_axi_clk;
	if (clk_tmp != NULL) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	clk_tmp = hisifd->dss_mmbuf_clk;
	if (clk_tmp != NULL) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	return 0;
}

#if   defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
void dpe_pxl_clk_disable(struct hisi_fb_data_type *hisifd)
{
	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return;
	}

	if (is_dp_panel(hisifd)) {
		if (hisifd->dss_pxl1_clk) {
			clk_disable_unprepare(hisifd->dss_pxl1_clk);
		}
	}
	return;
}
#else
void dpe_pxl_clk_disable(struct hisi_fb_data_type *hisifd)
{
	struct clk *clk_tmp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = hisifd->dss_pxl0_clk;
		if (clk_tmp != NULL) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		clk_tmp = hisifd->dss_pxl1_clk;
		if (clk_tmp != NULL) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	}
	return;
}
#endif

int dpe_inner_clk_disable(struct hisi_fb_data_type *hisifd)
{
	struct clk *clk_tmp = NULL;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	dpe_pxl_clk_disable(hisifd);

	if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		clk_tmp = hisifd->dss_clk_media_common_clk;
		if (clk_tmp != NULL) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	} else {
		;
	}

	clk_tmp = hisifd->dss_pri_clk;
	if (clk_tmp != NULL) {
		clk_disable(clk_tmp);
		clk_unprepare(clk_tmp);
	}

	return 0;
}

void hisifb_pipe_clk_set_underflow_flag(struct hisi_fb_data_type *hisifd, bool underflow)
{
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null .\n");
		return;
	}

	HISI_FB_DEBUG("set underflow_int %d .\n", underflow);

}

/*******************************************************************************
**
*/
static int dpe_set_fastboot(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);/*[false alarm]*/
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	dss_inner_clk_common_enable(hisifd, true);
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dss_inner_clk_pdp_enable(hisifd, true);
	}

	dpe_init(hisifd, true);

	ret = panel_next_set_fastboot(pdev);

	//set inital display region
	hisifb_panel_display_time_init(hisifd);
	panel_next_tcon_mode(pdev, hisifd, &hisifd->panel_info);

	if (hisifd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) {
		dpe_interrupt_mask(hisifd);
		dpe_interrupt_clear(hisifd);
		dpe_irq_enable(hisifd);
		dpe_interrupt_unmask(hisifd);
	}

	//ldi_data_gate(hisifd, true);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_clk_get(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is null.\n");
		return -EINVAL;
	}

	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is null.\n");
		return -EINVAL;
	}

	hisifd->dss_mmbuf_clk = devm_clk_get(&pdev->dev, hisifd->dss_mmbuf_clk_name);
	if (IS_ERR(hisifd->dss_mmbuf_clk)) {
		ret = PTR_ERR(hisifd->dss_mmbuf_clk);
		HISI_FB_ERR("dss_mmbuf_clk error, ret = %d", ret);
		return ret;
	}

	hisifd->dss_axi_clk = devm_clk_get(&pdev->dev, hisifd->dss_axi_clk_name);
	if (IS_ERR(hisifd->dss_axi_clk)) {
		ret = PTR_ERR(hisifd->dss_axi_clk);
		HISI_FB_ERR("dss_axi_clk error, ret = %d", ret);
		return ret;
	}

	hisifd->dss_pclk_dss_clk = devm_clk_get(&pdev->dev, hisifd->dss_pclk_dss_name);
	if (IS_ERR(hisifd->dss_pclk_dss_clk)) {
		ret = PTR_ERR(hisifd->dss_pclk_dss_clk);
		HISI_FB_ERR("dss_pclk_dss_clk error, ret = %d", ret);
		return ret;
	}

	hisifd->dss_pri_clk = devm_clk_get(&pdev->dev, hisifd->dss_pri_clk_name);
	if (IS_ERR(hisifd->dss_pri_clk)) {
		ret = PTR_ERR(hisifd->dss_pri_clk);
		HISI_FB_ERR("dss_pri_clk error, ret = %d", ret);
		return ret;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
	#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
		defined(CONFIG_HISI_FB_V600)
		return ret;
	#endif
		hisifd->dss_pxl0_clk = devm_clk_get(&pdev->dev, hisifd->dss_pxl0_clk_name);  //lint !e527
		if (IS_ERR(hisifd->dss_pxl0_clk)) {
			ret = PTR_ERR(hisifd->dss_pxl0_clk);
			HISI_FB_ERR("dss_pxl0_clk error, ret = %d", ret);
			return ret;
		}
	} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && !hisifd->panel_info.fake_external) {
		hisifd->dss_pxl1_clk = devm_clk_get(&pdev->dev, hisifd->dss_pxl1_clk_name);
		if (IS_ERR(hisifd->dss_pxl1_clk)) {
			ret = PTR_ERR(hisifd->dss_pxl1_clk);
			HISI_FB_ERR("dss_pxl1_clk error, ret = %d, hisifd->dss_pxl1_clk_name=%s, hisifd->panel_info.fake_external=%d",
				ret, hisifd->dss_pxl1_clk_name, hisifd->panel_info.fake_external);
			return ret;
		}
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		hisifd->dss_clk_media_common_clk = clk_get(NULL, "clk_media_common");
		if (IS_ERR(hisifd->dss_clk_media_common_clk)) {
			ret = PTR_ERR(hisifd->dss_clk_media_common_clk);
			HISI_FB_ERR("clk_media_common error, ret = %d.", ret);
			return ret;
		}
	} else {
		;
	}

	return ret;

}

static int dpe_on(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	mediacrg_regulator_enable(hisifd);
	if (hisifd->index == AUXILIARY_PANEL_IDX) {
		ret = dpe_common_clk_enable(hisifd);
		if (ret) {
			HISI_FB_ERR("fb%d dpe_common_clk_enable, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = dpe_inner_clk_enable(hisifd);
		if (ret) {
			HISI_FB_ERR("fb%d dpe_inner_clk_enable, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = dpe_set_clk_rate(pdev);
		if (ret) {
			HISI_FB_ERR("fb%d dpe_set_clk_rate, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	} else {
		ret = dpe_set_clk_rate(pdev); // edc pxl
		if (ret) {
			HISI_FB_ERR("fb%d dpe_set_clk_rate, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = dpe_common_clk_enable(hisifd);
		if (ret) {
			HISI_FB_ERR("fb%d dpe_common_clk_enable, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}

		ret = dpe_inner_clk_enable(hisifd);
		if (ret) {
			HISI_FB_ERR("fb%d dpe_inner_clk_enable, error=%d!\n",
				hisifd->index, ret);
			return -EINVAL;
		}
	}

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
	if (hisifb_set_mmbuf_clk_rate(hisifd) < 0) {
		HISI_FB_ERR("fb%d reset_mmbuf_clk_rate failed !\n", hisifd->index);
		return -EINVAL;
	}
#endif

	ret = dpe_regulator_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d dpe_regulator_enable, error=%d!\n",
			hisifd->index, ret);
		return -EINVAL;
	}
	mdc_regulator_enable(hisifd);


	dss_inner_clk_common_enable(hisifd, false);
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dss_inner_clk_pdp_enable(hisifd, false);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		dss_inner_clk_sdp_enable(hisifd);
	} else {
		;
	}

	dpe_init(hisifd, false);


	if (is_ldi_panel(hisifd)) {
		hisifd->panel_info.lcd_init_step = LCD_INIT_POWER_ON;
		ret = panel_next_on(pdev);
		if (ret) {
		    HISI_FB_ERR("fb%d failed ret %d\n", hisifd->index,ret);
		    return -EINVAL;
		}
	}

	ret = panel_next_on(pdev);

	//set inital display region
	panel_next_tcon_mode(pdev, hisifd, &hisifd->panel_info);

	if (hisifd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) {
		dpe_interrupt_mask(hisifd);
		dpe_interrupt_clear(hisifd);
		dpe_irq_enable(hisifd);
		dpe_interrupt_unmask(hisifd);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_off(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (hisifd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) {
		dpe_interrupt_mask(hisifd);
		dpe_irq_disable(hisifd);
	} else {
		if (hisifd->vsync_ctrl.vsync_ctrl_enabled == 1) {
			if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF) {
				dpe_interrupt_mask(hisifd);
				dpe_irq_disable(hisifd);
				HISI_FB_INFO("fb%d, need to disable dpe irq! vsync_ctrl_enabled=%d.\n",
					hisifd->index, hisifd->vsync_ctrl.vsync_ctrl_enabled);
			}
		}
	}

	ret = panel_next_off(pdev);

	dpe_deinit(hisifd);

	dpe_check_itf_status(hisifd);

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		dss_inner_clk_pdp_disable(hisifd);
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		dss_inner_clk_sdp_disable(hisifd);
	} else {
		;
	}
	dss_inner_clk_common_disable(hisifd);
	mdc_regulator_disable(hisifd);
	dpe_regulator_disable(hisifd);
	dpe_inner_clk_disable(hisifd);
	dpe_common_clk_disable(hisifd);
	mediacrg_regulator_disable(hisifd);

	if (hisifd->vsync_ctrl_type != VSYNC_CTRL_NONE) {
		if (!is_dss_idle_enable())
			hisifd->panel_info.vsync_ctrl_type = VSYNC_CTRL_NONE;
		else
			hisifd->panel_info.vsync_ctrl_type = hisifd->vsync_ctrl_type;
	}

#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
	defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
	if (hisifd->video_idle_ctrl.gpu_compose_idle_frame)
		(void)hisifb_video_idle_release_cache(hisifd);
	else
		hisifb_video_idle_buffer_free(hisifd);
#endif

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_lp_ctrl(struct platform_device *pdev, bool lp_enter)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (lp_enter) {
		ret = panel_next_lp_ctrl(pdev, lp_enter);

		if (hisifd->index == PRIMARY_PANEL_IDX) {
			dss_inner_clk_pdp_disable(hisifd);
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			dss_inner_clk_sdp_disable(hisifd);
		} else {
			;
		}
		dss_inner_clk_common_disable(hisifd);
	} else {
		dss_inner_clk_common_enable(hisifd, false);
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			dss_inner_clk_pdp_enable(hisifd, false);
		} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
			dss_inner_clk_sdp_enable(hisifd);
		} else {
			;
		}

		dpe_init(hisifd, false);

		ret = panel_next_lp_ctrl(pdev, lp_enter);
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_remove(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = panel_next_remove(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	HISI_FB_DEBUG("fb%d, bl_level=%d.\n", hisifd->index, bl_level);

	if (pinfo->bl_max < 1) {
		HISI_FB_ERR("bl_max(%d) is out of range!!", pinfo->bl_max);
		return -EINVAL;
	}

	if (bl_level > pinfo->bl_max) {
		bl_level = pinfo->bl_max;
	}

	if (bl_level < pinfo->bl_min && bl_level) {
		bl_level = pinfo->bl_min;
	}

	ret = panel_next_set_backlight(pdev, bl_level);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_vsync_ctrl(struct platform_device *pdev, int enable)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	if (enable) {
		ret = panel_next_vsync_ctrl(pdev, enable);
		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF) {
			dpe_interrupt_mask(hisifd);
			dpe_interrupt_clear(hisifd);
			dpe_irq_enable(hisifd);
			dpe_interrupt_unmask(hisifd);
		}
	} else {
		ret = panel_next_vsync_ctrl(pdev, enable);
		if (hisifd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF) {
			dpe_interrupt_mask(hisifd);
			dpe_interrupt_clear(hisifd);
			dpe_irq_disable_nosync(hisifd);
		}
	}

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = panel_next_lcd_fps_scence_handle(pdev, scence);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_lcd_fps_updt_handle(struct platform_device *pdev)
{
	int ret = 0;
	char __iomem *ldi_base = NULL;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	uint32_t hbp_updt = 0;
	uint32_t hfp_updt = 0;
	uint32_t hpw_updt = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}
	pinfo = &(hisifd->panel_info);

	if (hisifd->index != PRIMARY_PANEL_IDX) {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return 0;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ldi_base = hisifd->dss_base+ DSS_LDI0_OFFSET;

	ret = panel_next_lcd_fps_updt_handle(pdev);

	hbp_updt = pinfo->ldi_updt.h_back_porch;
	hfp_updt = pinfo->ldi_updt.h_front_porch;
	hpw_updt = pinfo->ldi_updt.h_pulse_width;


	if (is_mipi_video_panel(hisifd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, hfp_updt
			| ((hbp_updt + DSS_WIDTH(hpw_updt)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	}

	pinfo->fps = pinfo->fps_updt;

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);
	return ret;
}

static int dpe_esd_handle(struct platform_device *pdev)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = panel_next_esd_handle(pdev);

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return ret;
}

static int dpe_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = NULL;

	if (pdev == NULL || dirty == NULL) {
		HISI_FB_ERR("pdev or dirty is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("index=%d, enter!\n", hisifd->index);

	ret = panel_next_set_display_region(pdev, dirty);

	HISI_FB_DEBUG("index=%d, exit!\n", hisifd->index);

	return ret;
}

static int dpe_set_pixclk_rate(struct platform_device *pdev)
{
	char __iomem *ldi_base = 0;
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(hisifd);
	if (NULL == pdss_vote_cmd) {
		HISI_FB_ERR("pdss_vote_cmd is NULL");
		return -EINVAL;
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI0_OFFSET;

		if (IS_ERR(hisifd->dss_pxl0_clk)) {
			ret = PTR_ERR(hisifd->dss_pxl0_clk);
			return ret;
		}

		ret = clk_set_rate(hisifd->dss_pxl0_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);
			return -EINVAL;
		}

		HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = hisifd->dss_base + DSS_LDI1_OFFSET;

		if (IS_ERR(hisifd->dss_pxl1_clk)) {
			ret = PTR_ERR(hisifd->dss_pxl1_clk);
			return ret;
		}

		ret = clk_set_rate(hisifd->dss_pxl1_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			HISI_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				hisifd->index, pinfo->pxl_clk_rate, ret);
			return -EINVAL;
		}

		HISI_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl1_clk));
	} else {
		HISI_FB_ERR("fb%d, not support!", hisifd->index);
		return -EINVAL;
	}

	init_dbuf(hisifd);

	if (is_mipi_video_panel(hisifd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
				pinfo->ldi.h_front_porch | ((pinfo->ldi.h_back_porch + DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0,
			pinfo->ldi.h_front_porch | (pinfo->ldi.h_back_porch << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	}
	outp32(ldi_base + LDI_VRT_CTRL0,
		pinfo->ldi.v_front_porch | (pinfo->ldi.v_back_porch << 16));
	outp32(ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(pinfo->ldi.v_pulse_width));
	outp32(ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(pinfo->yres));

	outp32(ldi_base + LDI_PLR_CTRL,
		pinfo->ldi.vsync_plr | (pinfo->ldi.hsync_plr << 1) |
		(pinfo->ldi.pixelclk_plr << 2) | (pinfo->ldi.data_en_plr << 3));

	return 0;
}

/*******************************************************************************
**
*/
/*only for hi6250 mmbuf asc test */
#define MMBUF_ASC_INT_STATUS	(0x120)
#define MMBUF_ASC_INT_CLEAR		(0x124)
irqreturn_t dss_mmbuf_asc0_isr(int irq, void *ptr)
{
	struct hisi_fb_data_type *hisifd = NULL;
	uint32_t isr_flag = 0;

	hisifd = (struct hisi_fb_data_type *)ptr;
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return IRQ_NONE;
	}

	if (hisifd->mmbuf_asc0_base) {
		isr_flag = inp32(hisifd->mmbuf_asc0_base + MMBUF_ASC_INT_STATUS);
		HISI_FB_ERR("mmbuf access error!!!< irq: %d flag: %d  >\n", irq, isr_flag);

		outp32(hisifd->dss_base + DSS_RCH_D0_DMA_OFFSET + CH_SECU_EN, 0x1);
		outp32(hisifd->dss_base + DSS_RCH_D1_DMA_OFFSET + CH_SECU_EN, 0x1);
		outp32(hisifd->dss_base + DSS_RCH_D2_DMA_OFFSET + CH_SECU_EN, 0x1);
		outp32(hisifd->dss_base + DSS_RCH_D3_DMA_OFFSET + CH_SECU_EN, 0x1);
		outp32(hisifd->dss_base + DSS_RCH_G1_DMA_OFFSET + CH_SECU_EN, 0x1);
		outp32(hisifd->dss_base + DSS_RCH_VG1_DMA_OFFSET + CH_SECU_EN, 0x1);

		outp32(hisifd->mmbuf_asc0_base + MMBUF_ASC_INT_CLEAR, isr_flag);
	}
	return IRQ_HANDLED;
}

static int dpe_dsi1_isr_func_setup(struct hisi_fb_data_type *hisifd)
{
	int ret = 0;

	if (hisifd->dsi1_irq == 0) {
		HISI_FB_INFO("dsi1_irq == 0");
		return ret;
	}
	HISI_FB_INFO("request dsi1 irq[%d]\n", hisifd->dsi1_irq);
	ret = request_irq(hisifd->dsi1_irq, dss_dsi1_isr, 0, IRQ_DSI1_NAME, (void *)hisifd);
	if (ret != 0) {
		HISI_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n",
			hisifd->index, hisifd->dsi1_irq, ret);
		return ret;
	} else {
		disable_irq(hisifd->dsi1_irq);
	}

	return ret;
}

static int dpe_isr_fnc_setup(struct hisi_fb_data_type *hisifd)
{
	const char *irq_name = NULL;
	irqreturn_t (*isr_fnc)(int irq, void *ptr) = NULL;
	int ret = 0;

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
	const char *dsi_irq_name = NULL;
	irqreturn_t (*dsi_isr_fnc)(int irq, void *ptr) = NULL;
	uint32_t dsi_irq = 0;
#endif

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V350) || \
	defined(CONFIG_HISI_FB_V600)
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		irq_name = IRQ_PDP_NAME;
		isr_fnc = dss_pdp_isr;
		dsi_irq_name = IRQ_DSI0_NAME;
		dsi_isr_fnc = dss_dsi0_isr;
		if (is_dual_mipi_panel(hisifd))
			dpe_dsi1_isr_func_setup(hisifd);

	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		if (is_dp_panel(hisifd)) {
			irq_name = IRQ_SDP_NAME;
			isr_fnc = dss_sdp_isr_dp;
		} else {
			irq_name = IRQ_SDP_NAME;
			isr_fnc = dss_sdp_isr_mipi_panel;
			dsi_irq_name = IRQ_DSI1_NAME;
			dsi_isr_fnc = dss_dsi1_isr;
		}
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		irq_name = IRQ_ADP_NAME;
		isr_fnc = dss_adp_isr;
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		irq_name = IRQ_MDC_NAME;
		isr_fnc = dss_mdc_isr;
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
		return -EINVAL;
	}

	if (irq_name) {
		HISI_FB_INFO("fb%d, request irq[%s]\n", hisifd->index, irq_name);
		ret = request_irq(hisifd->dpe_irq, isr_fnc, 0, irq_name, (void *)hisifd);
		if (ret != 0) {
			HISI_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n", hisifd->index, hisifd->dpe_irq, ret);
			return ret;
		} else {
			disable_irq(hisifd->dpe_irq);
		}
	}

	if (dsi_irq_name) {
		if (hisifd->index == PRIMARY_PANEL_IDX) {
			dsi_irq = hisifd->dsi0_irq;
		} else if ((hisifd->index == EXTERNAL_PANEL_IDX) && is_mipi_panel(hisifd)) {
			dsi_irq = hisifd->dsi1_irq;
		}
		if (dsi_irq == 0) {
			return ret;
		}
		HISI_FB_INFO("request dsi_interrupt irq[%s]\n", dsi_irq_name);
		ret = request_irq(dsi_irq, dsi_isr_fnc, 0, dsi_irq_name, (void *)hisifd); //lint !e644
		if (ret != 0) {
			HISI_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n", hisifd->index, hisifd->dpe_irq, ret);
			return ret;
		} else {
			disable_irq(dsi_irq);
		}
	}
#else
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		irq_name = IRQ_PDP_NAME;
		isr_fnc = dss_pdp_isr;
	} else if (hisifd->index == EXTERNAL_PANEL_IDX) {
		irq_name = IRQ_SDP_NAME;
		isr_fnc = dss_sdp_isr;
	} else if (hisifd->index == AUXILIARY_PANEL_IDX) {
		irq_name = IRQ_ADP_NAME;
		isr_fnc = dss_adp_isr;
	} else if (hisifd->index == MEDIACOMMON_PANEL_IDX) {
		irq_name = IRQ_MDC_NAME;
		isr_fnc = dss_mdc_isr;
	} else {
		HISI_FB_ERR("fb%d, not support this device!\n", hisifd->index);
		return -EINVAL;
	}

	if (hisifd->dpe_irq) {
		ret = request_irq(hisifd->dpe_irq, isr_fnc, 0, irq_name, (void *)hisifd);
		if (ret != 0) {
			HISI_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n", hisifd->index, hisifd->dpe_irq, ret);
			return ret;
		} else {
			disable_irq(hisifd->dpe_irq);
		}
	}
#endif
	return ret;
}

static int dpe_regulator_clk_irq_setup(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct hisi_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	uint64_t pxl_clk_rate = 0;
	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL");
		return -EINVAL;
	}

	pinfo = &(hisifd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(hisifd);
	if (NULL == pdss_vote_cmd) {
		HISI_FB_ERR("pdss_vote_cmd is NULL");
		return -EINVAL;
	}

	(void)pxl_clk_rate;
	if (dpe_clk_get(pdev)) {
		HISI_FB_ERR("fb%d dpe_clk_get fail\n", hisifd->index);
		return -EINVAL;
	}

	ret = dpe_isr_fnc_setup(hisifd);
	if (ret) {
		HISI_FB_ERR("dpe_isr_fnc_setup fail\n");
		return ret;
	}
	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (hisifd->mmbuf_asc0_irq) {
			ret = request_irq(hisifd->mmbuf_asc0_irq, dss_mmbuf_asc0_isr, 0, "irq_mmbuf_asc0", (void *)hisifd);
			if (ret != 0) {
				HISI_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n", hisifd->index, hisifd->mmbuf_asc0_irq, ret);
				return ret;
			} else {
				disable_irq(hisifd->mmbuf_asc0_irq);
			}
		}
	}

	if (hisifd->index == PRIMARY_PANEL_IDX) {
		if (is_fastboot_display_enable()) {

			mediacrg_regulator_enable(hisifd);
			ret = dpe_set_clk_rate(pdev);
			if (ret) {
				HISI_FB_ERR("fb%d dpe_set_clk_rate, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = dpe_common_clk_enable(hisifd);
			if (ret) {
				HISI_FB_ERR("fb%d dpe_common_clk_enable, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			ret = dpe_inner_clk_enable(hisifd);
			if (ret) {
				HISI_FB_ERR("fb%d dpe_inner_clk_enable, error=%d!\n",
					hisifd->index, ret);
				return -EINVAL;
			}

			#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
				defined(CONFIG_HISI_FB_V350) || defined(CONFIG_HISI_FB_V600)
			if (hisifb_set_mmbuf_clk_rate(hisifd) < 0) {
				HISI_FB_ERR("fb%d reset_mmbuf_clk_rate failed !\n", hisifd->index);
				return -EINVAL;
			}
			#endif

			dpe_regulator_enable(hisifd);
		}
	}

	//hisi_irqaffinity_register(hisifd->dpe_irq, 1);

	return 0;
}

static int dpe_probe(struct platform_device *pdev)
{
	struct hisi_fb_data_type *hisifd = NULL;
	struct platform_device *hisi_fb_dev = NULL;
	struct hisi_fb_panel_data *pdata = NULL;

	int ret = 0;

	if (NULL == pdev) {
		HISI_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	hisifd = platform_get_drvdata(pdev);
	if (NULL == hisifd) {
		dev_err(&pdev->dev, "hisifd is NULL");
		return -EINVAL;
	}

	HISI_FB_DEBUG("fb%d, +.\n", hisifd->index);

	ret = dpe_regulator_clk_irq_setup(pdev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d dpe_irq_clk_setup failed, error=%d!\n", hisifd->index, ret);
		goto err;
	}

	/* alloc device */
	hisi_fb_dev = platform_device_alloc(DEV_NAME_FB, pdev->id);
	if (hisi_fb_dev == NULL) {
		dev_err(&pdev->dev, "fb%d platform_device_alloc failed, error=%d!\n", hisifd->index, ret);
		ret = -ENOMEM;
		goto err_device_alloc;
	}

	/* link to the latest pdev */
	hisifd->pdev = hisi_fb_dev;

	/* alloc panel device data */
	ret = platform_device_add_data(hisi_fb_dev, dev_get_platdata(&pdev->dev),
		sizeof(struct hisi_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add_data failed, error=%d!\n", hisifd->index, ret);
		goto err_device_put;
	}

	/* data chain */
	pdata = dev_get_platdata(&hisi_fb_dev->dev);
	pdata->set_fastboot = dpe_set_fastboot;
	pdata->on = dpe_on;
	pdata->off = dpe_off;
	pdata->lp_ctrl = dpe_lp_ctrl;
	pdata->remove = dpe_remove;
	pdata->set_backlight = dpe_set_backlight;
	pdata->vsync_ctrl = dpe_vsync_ctrl;
	pdata->lcd_fps_scence_handle = dpe_lcd_fps_scence_handle;
	pdata->lcd_fps_updt_handle = dpe_lcd_fps_updt_handle;
	pdata->esd_handle = dpe_esd_handle;
	pdata->set_display_region = dpe_set_display_region;
	pdata->set_pixclk_rate = dpe_set_pixclk_rate;
	pdata->lcd_model_show = dpe_lcd_model_show;
	pdata->lcd_check_reg = dpe_lcd_check_reg_show;
	pdata->lcd_mipi_detect = dpe_lcd_mipi_detect_show;
	pdata->lcd_hkadc_debug_show = dpe_lcd_hkadc_debug_show;
	pdata->lcd_hkadc_debug_store = dpe_lcd_hkadc_debug_store;
	pdata->lcd_gram_check_show = dpe_lcd_gram_check_show;
	pdata->lcd_gram_check_store = dpe_lcd_gram_check_store;
	pdata->lcd_dynamic_sram_checksum_show = dpe_lcd_dynamic_sram_checksum_show;
	pdata->lcd_dynamic_sram_checksum_store = dpe_lcd_dynamic_sram_checksum_store;
	pdata->lcd_color_temperature_show = dpe_lcd_color_temperature_show;
	pdata->lcd_color_temperature_store = dpe_lcd_color_temperature_store;
	pdata->lcd_ic_color_enhancement_mode_show = dpe_lcd_ic_color_enhancement_mode_show;
	pdata->lcd_ic_color_enhancement_mode_store = dpe_lcd_ic_color_enhancement_mode_store;
	pdata->led_rg_lcd_color_temperature_show = dpe_led_rg_lcd_color_temperature_show;
	pdata->led_rg_lcd_color_temperature_store = dpe_led_rg_lcd_color_temperature_store;
	pdata->lcd_comform_mode_show = dpe_lcd_comform_mode_show;
	pdata->lcd_comform_mode_store = dpe_lcd_comform_mode_store;
	pdata->lcd_cinema_mode_show = dpe_lcd_cinema_mode_show;
	pdata->lcd_cinema_mode_store = dpe_lcd_cinema_mode_store;
	pdata->lcd_support_mode_show = dpe_lcd_support_mode_show;
	pdata->lcd_support_mode_store = dpe_lcd_support_mode_store;
	pdata->lcd_voltage_enable_store = dpe_lcd_voltage_enable_store;
	pdata->lcd_bist_check = dpe_lcd_bist_check;
	pdata->mipi_dsi_bit_clk_upt_show = dpe_mipi_dsi_bit_clk_upt_show;
	pdata->mipi_dsi_bit_clk_upt_store = dpe_mipi_dsi_bit_clk_upt_store;
	pdata->lcd_sleep_ctrl_show = dpe_lcd_sleep_ctrl_show;
	pdata->lcd_sleep_ctrl_store = dpe_lcd_sleep_ctrl_store;
	pdata->lcd_test_config_show = dpe_lcd_test_config_show;
	pdata->lcd_test_config_store = dpe_lcd_test_config_store;
	pdata->lcd_reg_read_show = dpe_lcd_reg_read_show;
	pdata->lcd_reg_read_store = dpe_lcd_reg_read_store;
	pdata->lcd_support_checkmode_show = dpe_lcd_support_checkmode_show;
	pdata->lcd_lp2hs_mipi_check_show = dpe_lcd_lp2hs_mipi_check_show;
	pdata->lcd_lp2hs_mipi_check_store = dpe_lcd_lp2hs_mipi_check_store;
	pdata->amoled_pcd_errflag_check = dpe_amoled_pcd_errflag_check;
	pdata->sharpness2d_table_store = dpe_sharpness2d_table_store;
	pdata->sharpness2d_table_show = dpe_sharpness2d_table_show;
	pdata->panel_info_show = dpe_panel_info_show;
	pdata->lcd_hbm_ctrl_show = dpe_lcd_hbm_ctrl_show;
	pdata->lcd_hbm_ctrl_store = dpe_lcd_hbm_ctrl_store;
	pdata->lcd_acm_state_show = dpe_acm_state_show;
	pdata->lcd_acm_state_store = dpe_acm_state_store;
	pdata->lcd_acl_ctrl_show = dpe_lcd_acl_ctrl_show;
	pdata->lcd_acl_ctrl_store = dpe_lcd_acl_ctrl_store;
	pdata->get_lcd_id = dpe_get_lcd_id;
	pdata->lcd_gmp_state_show = dpe_gmp_state_show;
	pdata->lcd_gmp_state_store = dpe_gmp_state_store;
	pdata->amoled_alpm_setting_store = dpe_alpm_setting_store;
	pdata->lcd_xcc_store = dpe_lcd_xcc_store;
	pdata->next = pdev;

	/* get/set panel info */
	if (pdata->panel_info == NULL) {
		dev_err(&pdev->dev, "pdata->panel_info is null pointer\n");
		goto err_device_put;
	}
	memcpy(&hisifd->panel_info, pdata->panel_info, sizeof(struct hisi_panel_info));

	hisifd->vsync_ctrl_type = hisifd->panel_info.vsync_ctrl_type;

	/* set driver data */
	platform_set_drvdata(hisi_fb_dev, hisifd);
	ret = platform_device_add(hisi_fb_dev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add failed, error=%d!\n", hisifd->index, ret);
		goto err_device_put;
	}

	dpe_init_led_rg_ct_csc_value();

	HISI_FB_DEBUG("fb%d, -.\n", hisifd->index);

	return 0;

err_device_put:
	platform_device_put(hisi_fb_dev);
err_device_alloc:
err:
	return ret;
}

static struct platform_driver this_driver = {
	.probe = dpe_probe,
	.remove = NULL,
	.suspend = NULL,
	.resume = NULL,
	.shutdown = NULL,
	.driver = {
		.name = DEV_NAME_DSS_DPE,
	},
};

static int __init dpe_driver_init(void)
{
	int ret = 0;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		HISI_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(dpe_driver_init);
#pragma GCC diagnostic pop
