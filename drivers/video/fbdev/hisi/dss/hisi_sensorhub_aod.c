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

DEFINE_SEMAPHORE(hw_lock_semaphore);
DEFINE_SEMAPHORE(hisi_sensorhub_aod_blank_sem);

static int sh_aod_blank_refcount;

#if defined(CONFIG_HISI_FB_AOD)
extern int get_aod_support(void);
extern int hisi_aod_set_blank_mode(int blank_mode);
extern bool hisi_aod_get_aod_lock_status(void);
#endif

extern struct fb_info *g_info_fb0;

bool hisi_sensorhub_aod_hw_lock(struct dpu_fb_data_type *dpufd)
{
	bool hw_lock_succ = false;
	bool is_timeout = false;
	uint32_t lock_status;
	uint32_t delay_count = 0;
	uint32_t timeout_count = 5000;  /* wait 5000 times */

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return hw_lock_succ;
	}

#if defined(CONFIG_HISI_FB_AOD)
	if (get_aod_support() != 1) {
		if (g_dump_sensorhub_aod_hwlock)
			DPU_FB_INFO("sensorhub aod no support!\n");

		return false;
	}
#endif
	down(&hw_lock_semaphore);

	while (1) {
		set_reg(dpufd->pctrl_base + PCTRL_RESOURCE3_LOCK, 0x30000000, 32, 0);
		lock_status = inp32(dpufd->pctrl_base + PCTRL_RESOURCE3_LOCK_STATUS);

		if (((lock_status & 0x70000000) == 0x30000000) || (delay_count > timeout_count)) {
			is_timeout = (delay_count > timeout_count) ? true : false;
			break;
		}

		mdelay(1);
		++delay_count;
	}

	if (!is_timeout)
		hw_lock_succ = true;
	else
		DPU_FB_ERR("fb%d AP hw_lock fail, lock_status = 0x%x, delay_count = %d!\n",
			dpufd->index, lock_status, delay_count);

	up(&hw_lock_semaphore);

	if (g_dump_sensorhub_aod_hwlock)
		DPU_FB_INFO("fb%d +, hw_lock=%d (0: fail; 1: succ), lock_status=0x%x!\n",
			dpufd->index, hw_lock_succ, lock_status);

	return hw_lock_succ;
}

bool hisi_sensorhub_aod_hw_unlock(struct dpu_fb_data_type *dpufd)
{
	bool hw_unlock_succ = false;
	uint32_t unlock_status;

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return hw_unlock_succ;
	}

#if defined(CONFIG_HISI_FB_AOD)
	if (get_aod_support() != 1) {
		if (g_dump_sensorhub_aod_hwlock)
			DPU_FB_INFO("sensorhub aod no support!\n");

		return true;
	}
#endif

	down(&hw_lock_semaphore);
	set_reg(dpufd->pctrl_base + PCTRL_RESOURCE3_UNLOCK, 0x30000000, 32, 0);
	unlock_status = inp32(dpufd->pctrl_base + PCTRL_RESOURCE3_LOCK_STATUS);
	hw_unlock_succ = true;
	up(&hw_lock_semaphore);

	if (g_dump_sensorhub_aod_hwlock)
		DPU_FB_INFO("fb%d -, hw_unlock=%d (0: fail; 1: succ), unlock_status=0x%x!\n",
			dpufd->index, hw_unlock_succ, unlock_status);

	return hw_unlock_succ;
}

static int hisi_check_sh_aod_blank_refcount(struct dpu_fb_data_type *dpufd, uint32_t msg_no)
{
	sh_aod_blank_refcount++;
	DPU_FB_INFO("fb%d +, sh_aod_blank_refcount=%d!\n", dpufd->index, sh_aod_blank_refcount);
	DPU_FB_INFO("Power State Reg is 0x%x\n", inp32(dpufd->sctrl_base + SCBAKDATA0));

	/* high 16bit indicate msg no */
	set_reg(dpufd->sctrl_base + SCBAKDATA0, msg_no & 0xFFFF, 16, 16);

	if (sh_aod_blank_refcount != 1) {
		DPU_FB_ERR("fb%d, sh_aod_blank_refcount=%d is error\n", dpufd->index, sh_aod_blank_refcount);
		return -1;
	}

	return 0;
}

static int hisi_sensorhub_aod_set_pxl_clk(struct dpu_fb_data_type *dpufd)
{
	uint64_t pxl_clk_rate;
	int ret;

	pxl_clk_rate = dpufd->panel_info.pxl_clk_rate;
	ret = clk_set_rate(dpufd->dss_pxl0_clk, pxl_clk_rate);
	if (ret < 0) {
		DPU_FB_ERR("fb%d dss_pxl0_clk clk_set_rate[%llu] failed, error=%d!\n",
			dpufd->index, pxl_clk_rate, ret);
		return ret;
	}

	DPU_FB_INFO("dss_pxl0_clk:[%llu]->[%llu]\n", pxl_clk_rate, (uint64_t)clk_get_rate(dpufd->dss_pxl0_clk));

	return 0;
}

static int dpe_clk_enable(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint64_t default_dss_core_clk = DEFAULT_DSS_CORE_CLK_RATE_L1;

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

	if (dpufd->panel_info.update_core_clk_support)
		default_dss_core_clk = DEFAULT_DSS_CORE_CLK_RATE_L2;
	if (dss_get_current_pri_clk_rate(dpufd) < default_dss_core_clk) {
		ret = clk_set_rate(dpufd->dss_pri_clk, default_dss_core_clk);
		if (ret < 0) {
			DPU_FB_ERR("fb%d dss_pri_clk clk_set_rate failed, error=%d!\n", dpufd->index, ret);
			return -EINVAL;
		}
		dpufd->dss_vote_cmd.dss_pri_clk_rate = default_dss_core_clk;
		DPU_FB_INFO("dss_pri_clk:[%llu]\n", (uint64_t)clk_get_rate(dpufd->dss_pri_clk));
	}


	return 0;
}

static int hisi_aod_config_fold_panel(struct dpu_fb_data_type *dpufd)
{
	struct dpu_panel_info *pinfo = NULL;
	uint32_t panel_id;

	pinfo = &(dpufd->panel_info);
	panel_id = hisi_aod_get_panel_id();
	DPU_FB_INFO("[fold] panel id :%d\n", panel_id);

	if (panel_id >= DISPLAY_PANEL_ID_MAX) {
		DPU_FB_ERR("[fold] not supported panel id :%d\n", panel_id);
		return -1;
	}
	dpufd_get_panel_info(dpufd, pinfo, panel_id);

	return 0;
}

int hisi_sensorhub_aod_unblank(uint32_t msg_no)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL point!\n");

	DPU_FB_INFO("fb%d,msg_no is %d +\n", dpufd->index, msg_no);

	down(&hisi_sensorhub_aod_blank_sem);
	if (!hisi_sensorhub_aod_hw_lock(dpufd))
		goto up_blank_sem;

	if (hisi_check_sh_aod_blank_refcount(dpufd, msg_no))
		goto hw_unlock;

	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) {
		if (hisi_aod_config_fold_panel(dpufd))
			goto hw_unlock;
	}

	mediacrg_regulator_enable(dpufd);

	ret = dpe_clk_enable(dpufd);
	if (ret < 0)
		goto hw_unlock;

	ret = hisi_sensorhub_aod_set_pxl_clk(dpufd);
	if (ret < 0)
		goto hw_unlock;

	ret = dpe_regulator_enable(dpufd);
	if (ret) {
		DPU_FB_ERR("fb%d dpe_regulator_enable, error=%d!\n", dpufd->index, ret);
		goto hw_unlock;
	}
	/* dsi apb clk must be enabled after dsi reset, so now disable dsi apb clk */
	inheritor_dss_wr_reg(dpufd->dss_base + GLB_MODULE_CLK_SEL, 0xFE7FFFFF);
	inheritor_dss_wr_reg(dpufd->dss_base + GLB_MODULE_CLK_EN, 0xFE7FFFFF);

	if (need_config_dsi1(dpufd))
		outp32(dpufd->peri_crg_base + PERRSTDIS3, 0x30000000);
	else
		outp32(dpufd->peri_crg_base + PERRSTDIS3, 0x10000000);

	ret = mipi_dsi_clk_enable(dpufd);
	if (ret) {
		DPU_FB_ERR("fb%d mipi_dsi_clk_enable, error=%d!\n", dpufd->index, ret);
		goto hw_unlock;
	} else {
		set_reg(dpufd->sctrl_base + SCBAKDATA0, 0x1, 2, 6);
		DPU_FB_INFO("Power State Reg is 0x%x\n", inp32(dpufd->sctrl_base + SCBAKDATA0));
		goto up_blank_sem;
	}

hw_unlock:
	set_reg(dpufd->sctrl_base + SCBAKDATA0, 0x3, 2, 6);
	if (!hisi_sensorhub_aod_hw_unlock(dpufd))
		DPU_FB_ERR("aod_hw_unlock fail!\n");

up_blank_sem:
	up(&hisi_sensorhub_aod_blank_sem);

	DPU_FB_INFO("fb%d, -\n", dpufd->index);
	return ret;
}

int hisi_sensorhub_aod_blank(uint32_t msg_no)
{
	int ret = 0;
	struct dpu_fb_data_type *dpufd = dpufd_list[PRIMARY_PANEL_IDX];

	if (!dpufd) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return -EINVAL;
	}

	DPU_FB_INFO("fb%d,msg_no is %d +\n", dpufd->index, msg_no);

	down(&hisi_sensorhub_aod_blank_sem);

	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) {
		if (hisi_aod_config_fold_panel(dpufd))
			goto up_blank_sem;
	}

	sh_aod_blank_refcount--;
	DPU_FB_INFO("fb%d +, sh_aod_blank_refcount=%d!\n", dpufd->index, sh_aod_blank_refcount);
	DPU_FB_INFO("Power State Reg is 0x%x\n", inp32(dpufd->sctrl_base + SCBAKDATA0));

	/* high 16bit indicate msg no */
	set_reg(dpufd->sctrl_base + SCBAKDATA0, msg_no & 0xFFFF, 16, 16);

	if (sh_aod_blank_refcount != 0) {
		set_reg(dpufd->sctrl_base + SCBAKDATA0, 0x3, 2, 6);
		DPU_FB_ERR("fb%d, sh_aod_blank_refcount=%d is error\n", dpufd->index, sh_aod_blank_refcount);
		goto up_blank_sem;
	}

	/* reset DSI */
	if (need_config_dsi1(dpufd))
		outp32(dpufd->peri_crg_base + PERRSTEN3, 0x30000000);
	else
		outp32(dpufd->peri_crg_base + PERRSTEN3, 0x10000000);
	mipi_dsi_clk_disable(dpufd);

	dpe_regulator_disable(dpufd);
	dpe_inner_clk_disable(dpufd);
	dpe_common_clk_disable(dpufd);
	mediacrg_regulator_disable(dpufd);

	if (!hisi_sensorhub_aod_hw_unlock(dpufd))
		DPU_FB_ERR("aod_hw_unlock fail!\n");

	DPU_FB_INFO("Power State Reg is 0x%x\n", inp32(dpufd->sctrl_base + SCBAKDATA0));
	set_reg(dpufd->sctrl_base + SCBAKDATA0, 0x2, 2, 6);
up_blank_sem:
	up(&hisi_sensorhub_aod_blank_sem);

	DPU_FB_INFO("fb%d, -\n", dpufd->index);
	return ret;
}

#if defined(CONFIG_HISI_FB_AOD)
bool hisi_fb_request_aod_hw_lock(struct dpu_fb_data_type *dpufd)
{
	return hisi_sensorhub_aod_hw_lock(dpufd);
}

void hisi_fb_release_aod_hw_lock(struct dpu_fb_data_type *dpufd, bool locked)
{
	if (locked) {
		if (!hisi_sensorhub_aod_hw_unlock(dpufd))
			DPU_FB_ERR("aod_hw_unlock fail!\n");
	}
}

int hisi_fb_aod_blank(struct dpu_fb_data_type *dpufd, int blank_mode)
{
	int ret = 0;

	if (!dpufd)
		return ret;

	if (dpufd->index != PRIMARY_PANEL_IDX)
		return ret;

	if (blank_mode == FB_BLANK_UNBLANK) {
		DPU_FB_INFO("+, aod blank_mode %d\n", blank_mode);
		ret = hisi_aod_set_blank_mode(blank_mode);

	} else if (blank_mode == FB_BLANK_POWERDOWN) {
		if ((!dpufd->panel_power_on) &&
			(dpufd_list[EXTERNAL_PANEL_IDX] != NULL) &&
			(!dpufd_list[EXTERNAL_PANEL_IDX]->panel_power_on)) {
			DPU_FB_INFO("+, aod blank_mode %d\n", blank_mode);
			ret = hisi_aod_set_blank_mode(blank_mode);
		}
	}

	return ret;
}

bool hisi_fb_get_aod_lock_status(void)
{
	return hisi_aod_get_aod_lock_status();
}
#endif


static int dpufb_unblank_refresh(struct dpu_fb_data_type *dpufd, struct fb_info *info)
{
	int ret;

	ret = hisi_fb_blank_sub(FB_BLANK_UNBLANK, info);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, blank_mode %d failed!\n", dpufd->index, FB_BLANK_UNBLANK);
		return -1;
	}

	hisi_fb_frame_refresh(dpufd, "aod");

#if defined(CONFIG_EFFECT_HIACE)
	ret = dpufb_ce_service_blank(FB_BLANK_UNBLANK, info);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, blank_mode %d dpufb_ce_service_blank() failed!\n", dpufd->index, FB_BLANK_UNBLANK);
		return -1;
	}
#endif

	ret = dpufb_display_engine_blank(FB_BLANK_UNBLANK, info);
	if (ret != 0) {
		DPU_FB_ERR("fb%d, blank_mode %d dpufb_display_engine_blank() failed!\n",
			dpufd->index, FB_BLANK_UNBLANK);
		return -1;
	}

	return 0;
}

void hisi_fb_unblank_wq_handle(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;
	struct fb_info *info = g_info_fb0;

	dpu_check_and_no_retval(!info, ERR, "info is NULL\n");

	dpufd = container_of(work, struct dpu_fb_data_type, aod_ud_fast_unblank_work);
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	if (dpufd->panel_info.fake_external && (dpufd->index == EXTERNAL_PANEL_IDX)) {
		dpufd->enable_fast_unblank = FALSE;
		DPU_FB_INFO("it is fake, blank it fail\n");
		return;
	}

	DPU_FB_INFO("fb%d +\n", dpufd->index);

	down(&dpufd_list[AUXILIARY_PANEL_IDX]->blank_sem);
	down(&dpufd->fast_unblank_sem);
	if ((dpufd->panel_power_on) && (dpufd->secure_ctrl.secure_blank_flag)) {
		/* wait for blank */
		DPU_FB_INFO("wait for tui blank!\n");
		while (dpufd->panel_power_on)
			mdelay(1);
	}


	down(&dpufd->blank_aod_sem);
	if (hisi_fb_get_aod_lock_status() == false) {
		DPU_FB_INFO("ap had released aod lock, stop unblank wq handle!\n");
		goto fast_unblank_fail;
	}

	if (dpufd->dp_device_srs) {
		dpufd->dp_device_srs(dpufd, true);
	} else {
		if (dpufb_unblank_refresh(dpufd, info) < 0)
			goto fast_unblank_fail;
	}

	DPU_FB_INFO("fb%d -\n", dpufd->index);

fast_unblank_fail:
	dpufd->enable_fast_unblank = FALSE;
	up(&dpufd->blank_aod_sem);
	up(&dpufd->fast_unblank_sem);
	up(&dpufd_list[AUXILIARY_PANEL_IDX]->blank_sem);
}

void wait_for_aod_stop(struct dpu_fb_data_type *dpufd)
{
	if (dpufd->index != PRIMARY_PANEL_IDX)
		return;
	if ((dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) == 0)
		return;

	DPU_FB_INFO("+\n");
	hisi_aod_wait_stop_nolock();
	DPU_FB_INFO("-\n");
}

void wait_for_fast_unblank_end(struct dpu_fb_data_type *dpufd)
{
	int try_times = 0;

	DPU_FB_INFO("+\n");

	while (dpufd->enable_fast_unblank) {
		mdelay(1);
		if (++try_times > 1000) { /* wait times */
			DPU_FB_ERR("[fold] wait for fast unblank end timeout!\n");
			break;
		}
	}
}

/* aod end include two steps: 1. aod stop 2. fast unblank end */
void wait_for_aod_end(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");

	wait_for_aod_stop(dpufd);
	wait_for_fast_unblank_end(dpufd);
}

