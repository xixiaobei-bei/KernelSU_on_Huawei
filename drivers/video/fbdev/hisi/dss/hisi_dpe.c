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

#include "hisi_fb.h"
#include "hisi_dpe_utils.h"
#include "product/attrs/hisi_dpe_attrs.h"



#include "hisi_spr_dsc.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"

DEFINE_SEMAPHORE(hisi_fb_dss_regulator_sem);
static int dss_regulator_refcount;
static int dss_regulator_ref_table[HISI_FB_MAX_FBI_LIST] = {0};


/* DSS_LDI0_OFFSET */
static int dpe_init(struct dpu_fb_data_type *dpufd, bool fastboot_enable)
{
	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		init_post_scf(dpufd);
		init_dpp(dpufd);
		init_acm(dpufd);
		init_dpp_csc(dpufd);
#if defined(CONFIG_EFFECT_HIACE)
		init_hiace(dpufd);
#endif
		init_dither(dpufd);
		init_igm_gmp_xcc_gm(dpufd);



		init_ifbc(dpufd);
		init_dbuf(dpufd);
		init_ldi(dpufd, fastboot_enable);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		dpufb_activate_vsync(dpufd_list[PRIMARY_PANEL_IDX]);
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501)
		if (dpufd->dss_pxl1_clk)
			clk_disable(dpufd->dss_pxl1_clk);

		if (!is_dp_panel(dpufd))
			set_reg(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_DSI1_CLK_SEL, 0x1, 1, 0);

		if (dpufd->dss_pxl1_clk)
			(void)clk_enable(dpufd->dss_pxl1_clk);

		if (!is_dp_panel(dpufd))
			set_reg(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_DSI1_RST_SEL, 0x1, 1, 0);
#endif
		/* dual lcd: dsi_mux_sel=1, dual mipi: dsi_mux_sel=0 */
		if (!is_dp_panel(dpufd))
			set_reg(dpufd->dss_base + DSS_MCTRL_SYS_OFFSET + MCTL_DSI_MUX_SEL, 0x1, 1, 0);
		dpufb_deactivate_vsync(dpufd_list[PRIMARY_PANEL_IDX]);

#if defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
	defined(CONFIG_HISI_FB_V360)
		init_dpp(dpufd);
#endif
		init_dbuf(dpufd);
		init_ldi(dpufd, fastboot_enable);
	} else if ((dpufd->index == AUXILIARY_PANEL_IDX) || (dpufd->index == MEDIACOMMON_PANEL_IDX)) {
		;
	} else {
		DPU_FB_ERR("fb%d, not support this device!\n", dpufd->index);
	}

	return 0;
}

static int dpe_deinit(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		deinit_ldi(dpufd);
		deinit_dbuf(dpufd);
		deinit_effect(dpufd);
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		deinit_ldi(dpufd);
#if defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V510) || \
		defined(CONFIG_HISI_FB_V600) || defined(CONFIG_HISI_FB_V360)
		deinit_dbuf(dpufd);
#endif
	} else if ((dpufd->index == AUXILIARY_PANEL_IDX) || (dpufd->index == MEDIACOMMON_PANEL_IDX)) {
		;
	} else {
		DPU_FB_ERR("fb%d, not support this device!\n", dpufd->index);
	}

	return 0;
}

static void dpe_check_itf_status(struct dpu_fb_data_type *dpufd)
{
	uint32_t temp = 0;
	int delay_count = 0;
	bool is_timeout = true;
	int itf_idx = 0;
	char __iomem *mctl_sys_base = NULL;

	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX)) {
		itf_idx = dpufd->index;
		mctl_sys_base =  dpufd->dss_base + DSS_MCTRL_SYS_OFFSET;

		while (1) {
			temp = inp32(mctl_sys_base + MCTL_MOD17_STATUS + itf_idx * 0x4);
			if (((temp & 0x10) == 0x10) || delay_count > 100) {  /* wait times */
				is_timeout = (delay_count > 100) ? true : false;
				break;
			}
			mdelay(1);
			++delay_count;
		}

		if (is_timeout)
			DPU_FB_DEBUG("mctl_itf%d not in idle status,ints=0x%x !\n", dpufd->index, temp);
	}
}

static uint32_t get_dsi_irq(struct dpu_fb_data_type *dpufd)
{
	uint32_t dsi_irq;

	dsi_irq = dpufd->dsi0_irq;
	if (is_dsi1_pipe_switch_connector(dpufd))
		dsi_irq = dpufd->dsi1_irq;
	return dsi_irq;
}

int dpe_irq_enable(struct dpu_fb_data_type *dpufd)
{

	if (dpufd == NULL) {
		DPU_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if ((dpufd->index != MEDIACOMMON_PANEL_IDX) && dpufd->dpe_irq) {
		enable_irq(dpufd->dpe_irq);
		DPU_FB_DEBUG("enable irq %d\n", dpufd->dpe_irq);
	}


	return 0;
}

int dpe_irq_disable(struct dpu_fb_data_type *dpufd)
{

	if (dpufd == NULL) {
		DPU_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (dpufd->dpe_irq) {
		disable_irq(dpufd->dpe_irq);
		DPU_FB_DEBUG("disable irq %d\n", dpufd->dpe_irq);
	}


	return 0;
}

int dpe_irq_disable_nosync(struct dpu_fb_data_type *dpufd)
{

	if (dpufd == NULL) {
		DPU_FB_ERR("NULL ptr.\n");
		return -EINVAL;
	}

	if (dpufd->dpe_irq)
		disable_irq_nosync(dpufd->dpe_irq);

	return 0;
}

int dpe_regulator_enable(struct dpu_fb_data_type *dpufd)
{
	int ret;

	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");
	dpu_check_and_return((dpufd->index >= HISI_FB_MAX_FBI_LIST), -EINVAL, ERR,
		"dpufd->index exceeds array limit\n");

	down(&hisi_fb_dss_regulator_sem);
	if (dpufd->index == PRIMARY_PANEL_IDX)
		DPU_FB_INFO("get hisi_fb_dss_regulator_sem.\n");

	ret = regulator_enable(dpufd->dpe_regulator->consumer);
	if (ret) {
		DPU_FB_ERR("fb%d dpe regulator_enable failed, error=%d!\n", dpufd->index, ret);
		goto regulator_sem;
	}

	dss_regulator_refcount++;
	dss_regulator_ref_table[dpufd->index]++;
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		DPU_FB_INFO(
			"fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			dpufd->index, dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V320) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V501)
#endif
	} else if (dss_regulator_ref_table[PRIMARY_PANEL_IDX] == 0) {
		DPU_FB_INFO(
			"fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			dpufd->index, dss_regulator_refcount,
			dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX],
			dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	}

regulator_sem:
	up(&hisi_fb_dss_regulator_sem);

	return ret;
}

int dpe_regulator_disable(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;

	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return((dpufd->index >= HISI_FB_MAX_FBI_LIST), -EINVAL, ERR,
		"dpufd->index exceeds array limit\n");
	down(&hisi_fb_dss_regulator_sem);

	dss_regulator_refcount--;
	dss_regulator_ref_table[dpufd->index]--;
	if (dss_regulator_refcount < 0) {
		DPU_FB_ERR("fb%d, dss_regulator_refcount=%d is error\n", dpufd->index, dss_regulator_refcount);
		goto regulator_sem;
	}

	dpe_set_pixel_clk_rate_on_pll0(dpufd);
	if (dss_regulator_refcount == 0)
		dpe_set_common_clk_rate_on_pll0(dpufd);

	ret = regulator_disable(dpufd->dpe_regulator->consumer);
	if (ret != 0) {
		DPU_FB_ERR("fb%d dpe regulator_disable failed, error=%d!\n", dpufd->index, ret);
		goto regulator_sem;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		DPU_FB_INFO(
			"fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			dpufd->index, dss_regulator_refcount, dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX], dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	} else if (dss_regulator_ref_table[PRIMARY_PANEL_IDX] == 0) {
		DPU_FB_INFO(
			"fb%d, dss_regulator_refcount=%d!, dss_regulator_ref_table: fb0: %d, fb1: %d, fb2: %d, fb3: %d\n",
			dpufd->index, dss_regulator_refcount, dss_regulator_ref_table[PRIMARY_PANEL_IDX],
			dss_regulator_ref_table[EXTERNAL_PANEL_IDX], dss_regulator_ref_table[AUXILIARY_PANEL_IDX],
			dss_regulator_ref_table[MEDIACOMMON_PANEL_IDX]);
	}

regulator_sem:
	up(&hisi_fb_dss_regulator_sem);

	return ret;
}

int mediacrg_regulator_enable(struct dpu_fb_data_type *dpufd)
{
	int ret;
	int curr_volt;

	dpu_check_and_return(!dpufd || !dpufd->mediacrg_regulator, -EINVAL, ERR, "dpufd is NULL.\n");

	down(&hisi_fb_dss_regulator_sem);

	ret = regulator_enable(dpufd->mediacrg_regulator->consumer);
	if (ret)
		DPU_FB_ERR("fb%d mediacrg regulator_enable failed, error=%d!\n", dpufd->index, ret);

	up(&hisi_fb_dss_regulator_sem);

	if (g_dss_perf_debug > 0) {
		DPU_FB_INFO("dss performance debug, level = %d", g_dss_perf_debug);
		(void)dss_set_peri_volt(g_dss_perf_debug - 1, &curr_volt);
		mdelay(1);
	}

	return ret;
}

int mediacrg_regulator_disable(struct dpu_fb_data_type *dpufd)
{
	int ret;

	dpu_check_and_return(!dpufd || !dpufd->mediacrg_regulator, -EINVAL, ERR, "dpufd is NULL.\n");

	down(&hisi_fb_dss_regulator_sem);

	ret = regulator_disable(dpufd->mediacrg_regulator->consumer);
	if (ret != 0)
		DPU_FB_ERR("fb%d mediacrg regulator_disable failed, error=%d!\n", dpufd->index, ret);

	up(&hisi_fb_dss_regulator_sem);

	return ret;
}


int dpe_common_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	/* mmbuf_clk */
	clk_tmp = dpufd->dss_mmbuf_clk;
	if (clk_tmp) {
		ret = clk_prepare_enable(clk_tmp);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_mmbuf_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
	}

	/* aclk */
	clk_tmp = dpufd->dss_axi_clk;
	if (clk_tmp) {
		ret = clk_prepare_enable(clk_tmp);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_axi_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
	}

	/* pclk */
	clk_tmp = dpufd->dss_pclk_dss_clk;
	if (clk_tmp) {
		ret = clk_prepare_enable(clk_tmp);
		dpu_check_and_return((ret != 0), -EINVAL, ERR,
			"fb%d dss_pclk_dss_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
	}

	return 0;
}

int dpe_common_clk_enable_mmbuf_clk(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}

	/* mmbuf_clk */
	clk_tmp = dpufd->dss_mmbuf_clk;
	if (clk_tmp) {
		ret = clk_prepare_enable(clk_tmp);
		if (ret) {
			DPU_FB_ERR("fb%d dss_mmbuf_clk clk_prepare_enable failed, error=%d!\n",
				dpufd->index, ret);
			return -EINVAL;
		}
	}
	return 0;
}

static int dpe_pxl_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = dpufd->dss_pxl0_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				DPU_FB_ERR("fb%d dss_pxl0_clk clk_prepare failed, error=%d!\n", dpufd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				DPU_FB_ERR("fb%d dss_pxl0_clk clk_enable failed, error=%d!\n", dpufd->index, ret);
				return -EINVAL;
			}
		}
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		clk_tmp = dpufd->dss_pxl1_clk;
		if (clk_tmp) {
			ret = clk_prepare(clk_tmp);
			if (ret) {
				DPU_FB_ERR("fb%d dss_pxl1_clk clk_prepare failed, error=%d!\n", dpufd->index, ret);
				return -EINVAL;
			}

			ret = clk_enable(clk_tmp);
			if (ret) {
				DPU_FB_ERR("fb%d dss_pxl1_clk clk_enable failed, error=%d!\n", dpufd->index, ret);
				return -EINVAL;
			}
		}
	}

	return ret;
}

int dpe_inner_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct clk *clk_tmp = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}

	/* edc0_clk */
	clk_tmp = dpufd->dss_pri_clk;
	if (clk_tmp) {
		ret = clk_prepare_enable(clk_tmp);
		if (ret) {
			DPU_FB_ERR("fb%d dss_pri_clk clk_prepare_enable failed, error=%d!\n", dpufd->index, ret);
			return -EINVAL;
		}
	}

	/* ldi0_clk(pxl0_clk)    ldi1_clk(pxl1_clk) */
	ret = dpe_pxl_clk_enable(dpufd);
	if (ret) {
		DPU_FB_ERR("pxl clk enable failed, error=%d!\n", ret);
		return -EINVAL;
	}

	return 0;
}

int dpe_common_clk_disable(struct dpu_fb_data_type *dpufd)
{
	struct clk *clk_tmp = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}


	clk_tmp = dpufd->dss_pclk_dss_clk;
	if (clk_tmp)
		clk_disable_unprepare(clk_tmp);

	clk_tmp = dpufd->dss_axi_clk;
	if (clk_tmp)
		clk_disable_unprepare(clk_tmp);

	clk_tmp = dpufd->dss_mmbuf_clk;
	if (clk_tmp)
		clk_disable_unprepare(clk_tmp);

	return 0;
}

int dpe_common_clk_disable_mmbuf_clk(struct dpu_fb_data_type *dpufd)
{
	struct clk *clk_tmp = NULL;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}

	clk_tmp = dpufd->dss_mmbuf_clk;
	if (clk_tmp)
		clk_disable_unprepare(clk_tmp);

	return 0;
}

void dpe_pxl_clk_disable(struct dpu_fb_data_type *dpufd)
{
	struct clk *clk_tmp = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		clk_tmp = dpufd->dss_pxl0_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		clk_tmp = dpufd->dss_pxl1_clk;
		if (clk_tmp) {
			clk_disable(clk_tmp);
			clk_unprepare(clk_tmp);
		}
	}
}

int dpe_inner_clk_disable(struct dpu_fb_data_type *dpufd)
{
	struct clk *clk_tmp = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}

	dpe_pxl_clk_disable(dpufd);

	clk_tmp = dpufd->dss_pri_clk;
	if (clk_tmp)
		clk_disable_unprepare(clk_tmp);

	return 0;
}

void dpufb_pipe_clk_set_underflow_flag(struct dpu_fb_data_type *dpufd, bool underflow)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is null.\n");
		return;
	}

	DPU_FB_DEBUG("set underflow_int %d.\n", underflow);

}

static int dpe_set_fastboot(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}
	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	dss_inner_clk_common_enable(dpufd, true);
	if (dpufd->index == PRIMARY_PANEL_IDX)
		dss_inner_clk_pdp_enable(dpufd, true);

	dpe_init(dpufd, true);

	ret = panel_next_set_fastboot(pdev);

	/* set inital display region */
	dpufb_panel_display_time_init(dpufd);
	panel_next_tcon_mode(pdev, dpufd, &dpufd->panel_info);

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) {
		dpe_interrupt_mask(dpufd);
		dpe_interrupt_clear(dpufd);
		dpe_irq_enable(dpufd);
		dpe_interrupt_unmask(dpufd);
	}

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_common_clk_get(struct platform_device *pdev, struct dpu_fb_data_type *dpufd)
{
	int ret = 0;

	dpufd->dss_mmbuf_clk = devm_clk_get(&pdev->dev, dpufd->dss_mmbuf_clk_name);
	dpu_ptr_check_and_return(dpufd->dss_mmbuf_clk, ret, "dss_mmbuf_clk error, ret = %d", ret);

	dpufd->dss_axi_clk = devm_clk_get(&pdev->dev, dpufd->dss_axi_clk_name);
	dpu_ptr_check_and_return(dpufd->dss_axi_clk, ret, "dss_axi_clk error, ret = %d", ret);

	dpufd->dss_pclk_dss_clk = devm_clk_get(&pdev->dev, dpufd->dss_pclk_dss_name);
	dpu_ptr_check_and_return(dpufd->dss_pclk_dss_clk, ret, "dss_pclk_dss_clk error, ret = %d", ret);

	dpufd->dss_pri_clk = devm_clk_get(&pdev->dev, dpufd->dss_pri_clk_name);
	dpu_ptr_check_and_return(dpufd->dss_pri_clk, ret, "dss_pri_clk error, ret = %d", ret);

	return ret;
}

static int dpe_clk_get(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	ret = dpe_common_clk_get(pdev, dpufd);
	dpu_check_and_return((ret != 0), ret, ERR, "dpe_common_clk_get error.\n");

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		dpufd->dss_pxl0_clk = devm_clk_get(&pdev->dev, dpufd->dss_pxl0_clk_name);

		dpu_ptr_check_and_return(dpufd->dss_pxl0_clk, ret, "dss_pxl0_clk error, ret = %d", ret);
	} else if ((dpufd->index == EXTERNAL_PANEL_IDX) && !dpufd->panel_info.fake_external) {
#if   defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600) || \
		defined(CONFIG_HISI_FB_V360)
		if (is_dp_panel(dpufd))
			dpufd->dss_pxl1_clk = devm_clk_get(&pdev->dev, dpufd->dss_pxl1_clk_name);
		else
			return ret;
#else
		dpufd->dss_pxl1_clk = devm_clk_get(&pdev->dev, dpufd->dss_pxl1_clk_name);
#endif
		dpu_ptr_check_and_return(dpufd->dss_pxl1_clk, ret,
			"dss_pxl1_clk error, ret = %d, dpufd->dss_pxl1_clk_name=%s, dpufd->panel_info.fake_external=%d",
			ret, dpufd->dss_pxl1_clk_name, dpufd->panel_info.fake_external);
	} else {
		;
	}

	return ret;
}

static int dpe_set_clk_and_regulator(struct platform_device *pdev,  struct dpu_fb_data_type *dpufd)
{
	int ret;

	mediacrg_regulator_enable(dpufd);
	if (dpufd->index == AUXILIARY_PANEL_IDX) {
		ret = dpe_common_clk_enable(dpufd);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_common_clk_enable, error=%d!\n",
			dpufd->index, ret);

		ret = dpe_inner_clk_enable(dpufd);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_inner_clk_enable, error=%d!\n",
			dpufd->index, ret);

		ret = dpe_set_clk_rate(pdev);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_set_clk_rate, error=%d!\n",
			dpufd->index, ret);
	} else {
		ret = dpe_set_clk_rate(pdev); /* edc pxl */
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_set_clk_rate, error=%d!\n",
			dpufd->index, ret);

		ret = dpe_common_clk_enable(dpufd);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_common_clk_enable, error=%d!\n",
			dpufd->index, ret);

		ret = dpe_inner_clk_enable(dpufd);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_inner_clk_enable, error=%d!\n",
			dpufd->index, ret);
	}

	ret = dpe_regulator_enable(dpufd);
	dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_regulator_enable, error=%d!\n", dpufd->index, ret);

	dss_inner_clk_common_enable(dpufd, false);
	if (dpufd->index == PRIMARY_PANEL_IDX)
		dss_inner_clk_pdp_enable(dpufd, false);
	else if (dpufd->index == EXTERNAL_PANEL_IDX)
		dss_inner_clk_sdp_enable(dpufd);
	else
		;

	return ret;
}

static int dpe_on(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ret = dpe_set_clk_and_regulator(pdev, dpufd);
	dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d dpe_set_clk_and_regulator, error=%d!\n",
		dpufd->index, ret);

	dpe_init(dpufd, false);

	if (is_ldi_panel(dpufd)) {
		dpufd->panel_info.lcd_init_step = LCD_INIT_POWER_ON;
		ret = panel_next_on(pdev);
		dpu_check_and_return((ret != 0), -EINVAL, ERR, "fb%d failed ret %d\n", dpufd->index, ret);
	}

	ret = panel_next_on(pdev);

	/* set inital display region */
	panel_next_tcon_mode(pdev, dpufd, &dpufd->panel_info);

	if (dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) {
		dpe_interrupt_mask(dpufd);
		dpe_interrupt_clear(dpufd);
		dpe_irq_enable(dpufd);
		dpe_interrupt_unmask(dpufd);
	}

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static void dpe_regulator_and_clk_disable(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index == PRIMARY_PANEL_IDX)
		dss_inner_clk_pdp_disable(dpufd);
	else if (dpufd->index == EXTERNAL_PANEL_IDX)
		dss_inner_clk_sdp_disable(dpufd);
	else
		;
	dss_inner_clk_common_disable(dpufd);


	dpe_regulator_disable(dpufd);
	dpe_inner_clk_disable(dpufd);
	dpe_common_clk_disable(dpufd);
	mediacrg_regulator_disable(dpufd);
}

static int dpe_off(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	if ((dpufd->panel_info.vsync_ctrl_type == VSYNC_CTRL_NONE) ||
		((dpufd->vsync_ctrl.vsync_ctrl_enabled == 1) &&
		(dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF))) {
		dpe_interrupt_mask(dpufd);
		dpe_irq_disable(dpufd);
		DPU_FB_DEBUG("fb%d, need to disable dpe irq! vsync_ctrl_enabled=%d.\n",
			dpufd->index, dpufd->vsync_ctrl.vsync_ctrl_enabled);
	}

	ret = panel_next_off(pdev);

	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX)) {
		down(&dpufd->buf_sync_ctrl.layerbuf_sem);
		/* ov base display ok, disable irq, now layerbuf unlock */
		dpufb_layerbuf_unlock(dpufd, &(dpufd->buf_sync_ctrl.layerbuf_list));
		up(&dpufd->buf_sync_ctrl.layerbuf_sem);
	}

	dpe_deinit(dpufd);

	dpe_check_itf_status(dpufd);

	dpe_regulator_and_clk_disable(dpufd);

	if (dpufd->vsync_ctrl_type != VSYNC_CTRL_NONE) {
		if (!is_dss_idle_enable())
			dpufd->panel_info.vsync_ctrl_type = VSYNC_CTRL_NONE;
		else
			dpufd->panel_info.vsync_ctrl_type = dpufd->vsync_ctrl_type;
	}


	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_lp_ctrl(struct platform_device *pdev, bool lp_enter)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	if (lp_enter) {
		ret = panel_next_lp_ctrl(pdev, lp_enter);

		if (dpufd->index == PRIMARY_PANEL_IDX)
			dss_inner_clk_pdp_disable(dpufd);
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			dss_inner_clk_sdp_disable(dpufd);
		else
			;
		dss_inner_clk_common_disable(dpufd);
	} else {
		dss_inner_clk_common_enable(dpufd, false);
		if (dpufd->index == PRIMARY_PANEL_IDX)
			dss_inner_clk_pdp_enable(dpufd, false);
		else if (dpufd->index == EXTERNAL_PANEL_IDX)
			dss_inner_clk_sdp_enable(dpufd);
		else
			;

		dpe_init(dpufd, false);

		ret = panel_next_lp_ctrl(pdev, lp_enter);
	}

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_remove(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ret = panel_next_remove(pdev);

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_set_backlight(struct platform_device *pdev, uint32_t bl_level)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	DPU_FB_DEBUG("fb%d, bl_level=%d.\n", dpufd->index, bl_level);

	if (pinfo->bl_max < 1) {
		DPU_FB_ERR("bl_max = %d is out of range!!", pinfo->bl_max);
		return -EINVAL;
	}

	if (bl_level > pinfo->bl_max)
		bl_level = pinfo->bl_max;

	if (bl_level < pinfo->bl_min && bl_level)
		bl_level = pinfo->bl_min;

	ret = panel_next_set_backlight(pdev, bl_level);

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_vsync_ctrl(struct platform_device *pdev, int enable)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	if (enable) {
		ret = panel_next_vsync_ctrl(pdev, enable);

		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF) {
			dpe_interrupt_mask(dpufd);
			dpe_interrupt_clear(dpufd);
			dpe_irq_enable(dpufd);
			dpe_interrupt_unmask(dpufd);
		}
	} else {
		ret = panel_next_vsync_ctrl(pdev, enable);
		if (dpufd->panel_info.vsync_ctrl_type & VSYNC_CTRL_ISR_OFF) {
			dpe_interrupt_mask(dpufd);
			dpe_interrupt_clear(dpufd);
			dpe_irq_disable_nosync(dpufd);
		}
	}

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_lcd_fps_scence_handle(struct platform_device *pdev, uint32_t scence)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ret = panel_next_lcd_fps_scence_handle(pdev, scence);

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_lcd_fps_updt_handle(struct platform_device *pdev)
{
	int ret;
	char __iomem *ldi_base = NULL;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	uint32_t hbp_updt;
	uint32_t hfp_updt;
	uint32_t hpw_updt;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	pinfo = &(dpufd->panel_info);

	if (dpufd->index != PRIMARY_PANEL_IDX) {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return 0;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ldi_base = dpufd->dss_base + DSS_LDI0_OFFSET;

	ret = panel_next_lcd_fps_updt_handle(pdev);

	hbp_updt = pinfo->ldi_updt.h_back_porch;
	hfp_updt = pinfo->ldi_updt.h_front_porch;
	hpw_updt = pinfo->ldi_updt.h_pulse_width;


	if (is_mipi_video_panel(dpufd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, hfp_updt | ((hbp_updt + DSS_WIDTH(hpw_updt)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	}

	pinfo->fps = pinfo->fps_updt;

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);
	return ret;
}

static int dpe_esd_handle(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL) {
		DPU_FB_ERR("pdev is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ret = panel_next_esd_handle(pdev);

	DPU_FB_DEBUG("fb%d, -.\n", dpufd->index);

	return ret;
}

static int dpe_set_display_region(struct platform_device *pdev,
	struct dss_rect *dirty)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;

	if (pdev == NULL || dirty == NULL) {
		DPU_FB_ERR("pdev or dirty is NULL");
		return -EINVAL;
	}

	dpufd = platform_get_drvdata(pdev);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL");
		return -EINVAL;
	}

	DPU_FB_DEBUG("index=%d, enter!\n", dpufd->index);

	ret = panel_next_set_display_region(pdev, dirty);

	DPU_FB_DEBUG("index=%d, exit!\n", dpufd->index);

	return ret;
}

static void dpe_set_ldi(struct dpu_fb_data_type *dpufd)
{
	char __iomem *ldi_base = 0;
	struct dpu_panel_info *pinfo = NULL;

	pinfo = &(dpufd->panel_info);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		ldi_base = dpufd->dss_base + DSS_LDI0_OFFSET;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		ldi_base = dpufd->dss_base + DSS_LDI1_OFFSET;
	}

	if (is_mipi_video_panel(dpufd)) {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, pinfo->ldi.h_front_porch | ((pinfo->ldi.h_back_porch +
			DSS_WIDTH(pinfo->ldi.h_pulse_width)) << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, 0);
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	} else {
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL0, pinfo->ldi.h_front_porch | (pinfo->ldi.h_back_porch << 16));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL1, DSS_WIDTH(pinfo->ldi.h_pulse_width));
		outp32(ldi_base + LDI_DPI0_HRZ_CTRL2, DSS_WIDTH(pinfo->xres));
	}
	outp32(ldi_base + LDI_VRT_CTRL0, pinfo->ldi.v_front_porch | (pinfo->ldi.v_back_porch << 16));
	outp32(ldi_base + LDI_VRT_CTRL1, DSS_HEIGHT(pinfo->ldi.v_pulse_width));
	outp32(ldi_base + LDI_VRT_CTRL2, DSS_HEIGHT(pinfo->yres));

	outp32(ldi_base + LDI_PLR_CTRL, pinfo->ldi.vsync_plr | (pinfo->ldi.hsync_plr << 1) |
		(pinfo->ldi.pixelclk_plr << 2) | (pinfo->ldi.data_en_plr << 3));
}

static int dpe_set_pixclk_rate(struct platform_device *pdev)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct dpu_panel_info *pinfo = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;
	int ret = 0;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	pinfo = &(dpufd->panel_info);
	pdss_vote_cmd = get_dss_vote_cmd(dpufd);
	dpu_check_and_return((!pdss_vote_cmd), -EINVAL, ERR, "pdss_vote_cmd is Pointer.\n");

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (IS_ERR(dpufd->dss_pxl0_clk)) {
			ret = PTR_ERR(dpufd->dss_pxl0_clk);
			return ret;
		}

		ret = clk_set_rate(dpufd->dss_pxl0_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			DPU_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
			dpufd->index, pinfo->pxl_clk_rate, ret);
			return -EINVAL;
		}

		DPU_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl0_clk));
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		if (IS_ERR(dpufd->dss_pxl1_clk)) {
			ret = PTR_ERR(dpufd->dss_pxl1_clk);
			return ret;
		}

		ret = clk_set_rate(dpufd->dss_pxl1_clk, pinfo->pxl_clk_rate);
		if (ret < 0) {
			DPU_FB_ERR("fb%d dss_pxl1_clk clk_set_rate(%llu) failed, error=%d!\n",
				dpufd->index, pinfo->pxl_clk_rate, ret);
			return -EINVAL;
		}

		DPU_FB_INFO("dss_pxl1_clk:[%llu]->[%llu].\n",
			pinfo->pxl_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl1_clk));
	} else {
		DPU_FB_ERR("fb%d, not support!", dpufd->index);
		return -EINVAL;
	}

	init_dbuf(dpufd);

	dpe_set_ldi(dpufd);

	return 0;
}

static int dpe_dsi1_isr_func_setup(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;

	if (dpufd->dsi1_irq == 0) {
		DPU_FB_INFO("dsi1_irq == 0");
		return ret;
	}

	DPU_FB_INFO("request dsi1 irq[%d]\n", dpufd->dsi1_irq);
	ret = request_irq(dpufd->dsi1_irq, dss_dsi1_isr, 0, IRQ_DSI1_NAME, (void *)dpufd);
	if (ret != 0) {
		DPU_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n",
			dpufd->index, dpufd->dsi1_irq, ret);
		return ret;
	}
	disable_irq(dpufd->dsi1_irq);

	return ret;
}


static int dpe_isr_fnc_get(struct dpu_fb_data_type *dpufd,
	struct dpe_irq *dpe_irq_info)
{
	if (dpufd->index == PRIMARY_PANEL_IDX) {
		dpe_irq_info->irq_name = IRQ_PDP_NAME;
		dpe_irq_info->isr_fnc = dss_pdp_isr;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		dpe_irq_info->irq_name = IRQ_SDP_NAME;
		dpe_irq_info->isr_fnc = dss_sdp_isr;
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		dpe_irq_info->irq_name = IRQ_ADP_NAME;
		dpe_irq_info->isr_fnc = dss_adp_isr;
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		dpe_irq_info->irq_name = IRQ_MDC_NAME;
		dpe_irq_info->isr_fnc = dss_mdc_isr;
	} else {
		DPU_FB_ERR("fb%d, not support this device!\n", dpufd->index);
		return -EINVAL;
	}

	return 0;
}

static int dpe_isr_fnc_setup(struct dpu_fb_data_type *dpufd)
{
	int ret;
	struct dpe_irq dpe_irq_info = { NULL, NULL, NULL, NULL };

	ret = dpe_isr_fnc_get(dpufd, &dpe_irq_info);
	if (ret != 0)
		return ret;

	if (dpufd->dpe_irq) {
		ret = request_irq(dpufd->dpe_irq, dpe_irq_info.isr_fnc, 0, dpe_irq_info.irq_name, (void *)dpufd);
		if (ret != 0) {
			DPU_FB_ERR("fb%d request_irq failed, irq_no=%d error=%d!\n",
				dpufd->index, dpufd->dpe_irq, ret);
			return ret;
		}
		disable_irq(dpufd->dpe_irq);
	}

	return ret;
}

static int dpe_clk_enable(struct platform_device *pdev, struct dpu_fb_data_type *dpufd)
{
	int ret;

	ret = dpe_set_clk_rate(pdev);
	if (ret) {
		DPU_FB_ERR("fb%d dpe_set_clk_rate, error=%d!\n", dpufd->index, ret);
		return -EINVAL;
	}

	ret = dpe_common_clk_enable(dpufd);
	if (ret) {
		DPU_FB_ERR("fb%d dpe_common_clk_enable, error=%d!\n", dpufd->index, ret);
		return -EINVAL;
	}

	ret = dpe_inner_clk_enable(dpufd);
	if (ret) {
		DPU_FB_ERR("fb%d dpe_inner_clk_enable, error=%d!\n", dpufd->index, ret);
		return -EINVAL;
	}


	return ret;
}

static int dpe_regulator_clk_irq_setup(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	struct dss_vote_cmd *pdss_vote_cmd = NULL;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	pdss_vote_cmd = get_dss_vote_cmd(dpufd);
	if (pdss_vote_cmd == NULL) {
		DPU_FB_ERR("pdss_vote_cmd is NULL");
		return -EINVAL;
	}

	if (dpe_clk_get(pdev)) {
		DPU_FB_ERR("fb%d dpe_clk_get fail\n", dpufd->index);
		return -EINVAL;
	}

	ret = dpe_isr_fnc_setup(dpufd);
	if (ret) {
		DPU_FB_ERR("dpe_isr_fnc_setup fail\n");
		return ret;
	}

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return 0;

	if (is_fastboot_display_enable()) {
		mediacrg_regulator_enable(dpufd);

		ret = dpe_clk_enable(pdev, dpufd);

		dpu_check_and_return((ret != 0), ret, ERR, "dpe_clk_enable error.\n");

		dpe_regulator_enable(dpufd);
	}

	return 0;
}

static void hisi_pdata_cd_fun_init(struct dpu_fb_panel_data *pdata)
{
#if defined(CONFIG_DEVICE_ATTRS)
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
#endif

}

static void hisi_pdata_cb_fun_init(struct platform_device *pdev, struct dpu_fb_panel_data *pdata)
{
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
#if defined(CONFIG_DEVICE_ATTRS)
	pdata->mipi_dsi_bit_clk_upt_show = dpe_mipi_dsi_bit_clk_upt_show;
	pdata->mipi_dsi_bit_clk_upt_store = dpe_mipi_dsi_bit_clk_upt_store;
	pdata->panel_switch = dpe_panel_switch;
	pdata->get_panel_info = dpe_get_panel_info;
#endif
	pdata->next = pdev;
}

static int dpe_probe(struct platform_device *pdev)
{
	int ret;
	struct dpu_fb_data_type *dpufd = NULL;
	struct platform_device *hisi_fb_dev = NULL;
	struct dpu_fb_panel_data *pdata = NULL;

	dpu_check_and_return((!pdev), -EINVAL, ERR, "pdev is NULL.\n");

	dpufd = platform_get_drvdata(pdev);
	dpu_check_and_return((!dpufd), -EINVAL, ERR, "dpufd is NULL.\n");

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	ret = dpe_regulator_clk_irq_setup(pdev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d dpe_irq_clk_setup failed, error=%d!\n", dpufd->index, ret);
		goto err;
	}

	/* alloc device */
	hisi_fb_dev = platform_device_alloc(DEV_NAME_FB, pdev->id);
	if (hisi_fb_dev == NULL) {
		dev_err(&pdev->dev, "fb%d platform_device_alloc failed, error=%d!\n", dpufd->index, ret);
		ret = -ENOMEM;
		goto err;
	}

	/* link to the latest pdev */
	dpufd->pdev = hisi_fb_dev;

	/* alloc panel device data */
	ret = platform_device_add_data(hisi_fb_dev, dev_get_platdata(&pdev->dev), sizeof(struct dpu_fb_panel_data));
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add_data failed, error=%d!\n", dpufd->index, ret);
		goto err_device_put;
	}

	/* data chain */
	pdata = dev_get_platdata(&hisi_fb_dev->dev);
	hisi_pdata_cd_fun_init(pdata);
	hisi_pdata_cb_fun_init(pdev, pdata);

	/* get/set panel info */
	if (pdata->panel_info == NULL) {
		dev_err(&pdev->dev, "pdata->panel_info is null pointer\n");
		goto err_device_put;
	}
	memcpy(&dpufd->panel_info, pdata->panel_info, sizeof(struct dpu_panel_info));

	dpufd->vsync_ctrl_type = dpufd->panel_info.vsync_ctrl_type;

	/* set driver data */
	platform_set_drvdata(hisi_fb_dev, dpufd);
	ret = platform_device_add(hisi_fb_dev);
	if (ret) {
		dev_err(&pdev->dev, "fb%d platform_device_add failed, error=%d!\n", dpufd->index, ret);
		goto err_device_put;
	}

	dpe_init_led_rg_ct_csc_value();

	return 0;

err_device_put:
	platform_device_put(hisi_fb_dev);
	dpufd->pdev = NULL;
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
	int ret;

	ret = platform_driver_register(&this_driver);
	if (ret) {
		DPU_FB_ERR("platform_driver_register failed, error=%d!\n", ret);
		return ret;
	}

	return ret;
}

module_init(dpe_driver_init);
#pragma GCC diagnostic pop
