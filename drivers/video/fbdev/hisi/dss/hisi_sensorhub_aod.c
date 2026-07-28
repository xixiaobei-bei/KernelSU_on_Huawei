 /* Copyright (c) 2013-2014, Hisilicon Tech. Co., Ltd. All rights reserved.
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

DEFINE_SEMAPHORE(hw_lock_semaphore);
DEFINE_SEMAPHORE(hisi_sensorhub_aod_blank_sem);

static int sh_aod_blank_refcount = 0;
extern int get_aod_support(void);

bool hisi_sensorhub_aod_hw_lock(struct hisi_fb_data_type *hisifd)
{
	bool hw_lock_succ = false;
	bool is_timeout = false;
	uint32_t lock_status = 0;
	uint32_t delay_count = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return hw_lock_succ;
	}

	if (get_aod_support() != 1) {
		if (g_dump_sensorhub_aod_hwlock) {
			HISI_FB_INFO("sensorhub aod no support!\n");
		}
		return false;
	}

	down(&hw_lock_semaphore);

	while (1) {
		set_reg(hisifd->pctrl_base + PCTRL_RESOURCE3_LOCK, 0x30000000, 32, 0);
		lock_status = inp32(hisifd->pctrl_base + PCTRL_RESOURCE3_LOCK_STATUS);

		if (((lock_status & 0x70000000) == 0x30000000) || (delay_count > 1000)) {
			is_timeout = (delay_count > 1000) ? true : false;
			delay_count = 0;
			break;
		} else {
			mdelay(1);
			++delay_count;
		}
	}

	if (!is_timeout) {
		hw_lock_succ = true;
	} else {
		HISI_FB_ERR("AP hw_lock fail, lock_status = 0x%x!\n", lock_status);
	}

	up(&hw_lock_semaphore);

	if (g_dump_sensorhub_aod_hwlock) {
		HISI_FB_INFO("fb%d +, hw_lock=%d (0: fail; 1: succ), lock_status=0x%x!\n", hisifd->index, hw_lock_succ, lock_status);
	}
	return hw_lock_succ;
}

bool hisi_sensorhub_aod_hw_unlock(struct hisi_fb_data_type *hisifd)
{
	bool hw_unlock_succ = false;
	uint32_t unlock_status = 0;

	if (hisifd == NULL) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return hw_unlock_succ;
	}

	if (get_aod_support() != 1) {
		if (g_dump_sensorhub_aod_hwlock) {
			HISI_FB_INFO("sensorhub aod no support!\n");
		}
		return true;
	}

	down(&hw_lock_semaphore);
	set_reg(hisifd->pctrl_base + PCTRL_RESOURCE3_UNLOCK, 0x30000000, 32, 0);
	unlock_status = inp32(hisifd->pctrl_base + PCTRL_RESOURCE3_LOCK_STATUS);
	hw_unlock_succ = true;
	up(&hw_lock_semaphore);

	if (g_dump_sensorhub_aod_hwlock) {
		HISI_FB_INFO("fb%d -, hw_unlock=%d (0: fail; 1: succ), unlock_status=0x%x!\n", hisifd->index, hw_unlock_succ, unlock_status);
	}

	return hw_unlock_succ;
}

int hisi_sensorhub_aod_unblank(void)
{
	int ret = 0;
	uint64_t pxl_clk_rate;
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	down(&hisi_sensorhub_aod_blank_sem);
	if (!hisi_sensorhub_aod_hw_lock(hisifd)) {
		goto up_blank_sem;
	}

	sh_aod_blank_refcount++;
	HISI_FB_INFO("fb%d +, sh_aod_blank_refcount=%d!\n", hisifd->index, sh_aod_blank_refcount);
	HISI_FB_INFO("Power State Reg is 0x%x\n", inp32(hisifd->sctrl_base + SCBAKDATA0));

	if (sh_aod_blank_refcount != 1) {
		HISI_FB_ERR("fb%d, sh_aod_blank_refcount=%d is error\n", hisifd->index, sh_aod_blank_refcount);
		goto hw_unlock;
	}

	mediacrg_regulator_enable(hisifd);

	ret = dpe_common_clk_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d dpe_common_clk_enable, error=%d!\n",
			hisifd->index, ret);
		goto hw_unlock;
	}

	ret = dpe_inner_clk_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d dpe_inner_clk_enable, error=%d!\n",
			hisifd->index, ret);
		goto hw_unlock;
	}

	ret = clk_set_rate(hisifd->dss_pri_clk, DEFAULT_DSS_CORE_CLK_RATE_L1);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pri_clk clk_set_rate(300M) failed, error=%d!\n",
			hisifd->index, ret);
		goto hw_unlock;
	}

	HISI_FB_INFO("dss_pri_clk:[%llu].\n", (uint64_t)clk_get_rate(hisifd->dss_pri_clk));

	pxl_clk_rate = hisifd->panel_info.pxl_clk_rate;
	ret = clk_set_rate(hisifd->dss_pxl0_clk, pxl_clk_rate);
	if (ret < 0) {
		HISI_FB_ERR("fb%d dss_pxl0_clk clk_set_rate(%llu) failed, error=%d!\n",
			hisifd->index, pxl_clk_rate, ret);
		goto hw_unlock;
	}

	HISI_FB_INFO("dss_pxl0_clk:[%llu]->[%llu].\n", pxl_clk_rate, (uint64_t)clk_get_rate(hisifd->dss_pxl0_clk));

	ret = dpe_regulator_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d dpe_regulator_enable, error=%d!\n",
			hisifd->index, ret);
		goto hw_unlock;
	}

	outp32(hisifd->peri_crg_base + PERRSTDIS3, 0x10000000);
	ret = mipi_dsi_clk_enable(hisifd);
	if (ret) {
		HISI_FB_ERR("fb%d mipi_dsi_clk_enable, error=%d!\n",
			hisifd->index, ret);
		goto hw_unlock;
	} else {
		set_reg(hisifd->sctrl_base + SCBAKDATA0, 0x1, 2, 6);
		HISI_FB_INFO("Power State Reg is 0x%x\n", inp32(hisifd->sctrl_base + SCBAKDATA0));
		goto up_blank_sem;
	}

hw_unlock:
	set_reg(hisifd->sctrl_base + SCBAKDATA0, 0x3, 2, 6);
	if (!hisi_sensorhub_aod_hw_unlock(hisifd)) {
		HISI_FB_ERR("aod_hw_unlock fail!\n");
	}

up_blank_sem:
	up(&hisi_sensorhub_aod_blank_sem);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);
	return ret;
}

int hisi_sensorhub_aod_blank(void)
{
	int ret = 0;
	struct hisi_fb_data_type *hisifd = hisifd_list[PRIMARY_PANEL_IDX];

	if (NULL == hisifd) {
		HISI_FB_ERR("hisifd is NULL point!\n");
		return -EINVAL;
	}

	HISI_FB_INFO("fb%d, +.\n", hisifd->index);

	down(&hisi_sensorhub_aod_blank_sem);
	sh_aod_blank_refcount--;
	HISI_FB_INFO("fb%d +, sh_aod_blank_refcount=%d!\n", hisifd->index, sh_aod_blank_refcount);
	HISI_FB_INFO("Power State Reg is 0x%x\n", inp32(hisifd->sctrl_base + SCBAKDATA0));
	if (sh_aod_blank_refcount != 0) {
		set_reg(hisifd->sctrl_base + SCBAKDATA0, 0x3, 2, 6);
		HISI_FB_ERR("fb%d, sh_aod_blank_refcount=%d is error\n", hisifd->index, sh_aod_blank_refcount);
		goto up_blank_sem;
	}

	/* reset DSI */
	outp32(hisifd->peri_crg_base + PERRSTEN3, 0x10000000);
	mipi_dsi_clk_disable(hisifd);

	dpe_regulator_disable(hisifd);
	dpe_inner_clk_disable(hisifd);
	dpe_common_clk_disable(hisifd);
	mediacrg_regulator_disable(hisifd);

	if (!hisi_sensorhub_aod_hw_unlock(hisifd)) {
		HISI_FB_ERR("aod_hw_unlock fail!\n");
	}

	HISI_FB_INFO("Power State Reg is 0x%x\n", inp32(hisifd->sctrl_base + SCBAKDATA0));
	set_reg(hisifd->sctrl_base + SCBAKDATA0, 0x2, 2, 6);
up_blank_sem:
	up(&hisi_sensorhub_aod_blank_sem);

	HISI_FB_INFO("fb%d, -.\n", hisifd->index);
	return ret;
}
