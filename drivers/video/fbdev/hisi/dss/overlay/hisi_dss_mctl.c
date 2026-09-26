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

#include "hisi_overlay_utils.h"
#include "../hisi_display_effect.h"
#include "../hisi_dpe_utils.h"
#include "../hisi_ovl_online_wb.h"
#include "../hisi_mmbuf_manager.h"
#include "../hisi_spr_dsc.h"
#include "../hisi_frame_rate_ctrl.h"


void hisi_dss_mctl_init(const char __iomem *mctl_base, dss_mctl_t *s_mctl)
{
	if (mctl_base == NULL) {
		DPU_FB_ERR("mctl_base is NULL\n");
		return;
	}
	if (s_mctl == NULL) {
		DPU_FB_ERR("s_mctl is NULL\n");
		return;
	}

	memset(s_mctl, 0, sizeof(dss_mctl_t));
}

void hisi_dss_mctl_ch_starty_init(const char __iomem *mctl_ch_starty_base, dss_mctl_ch_t *s_mctl_ch)
{
	if (mctl_ch_starty_base == NULL) {
		DPU_FB_ERR("mctl_ch_starty_base is NULL\n");
		return;
	}
	if (s_mctl_ch == NULL) {
		DPU_FB_ERR("s_mctl_ch is NULL\n");
		return;
	}

	memset(s_mctl_ch, 0, sizeof(dss_mctl_ch_t));

	s_mctl_ch->chn_starty = inp32(mctl_ch_starty_base);
}

void hisi_dss_mctl_ch_mod_dbg_init(const char __iomem *mctl_ch_dbg_base, dss_mctl_ch_t *s_mctl_ch)
{
	if (mctl_ch_dbg_base == NULL) {
		DPU_FB_ERR("mctl_ch_dbg_base is NULL\n");
		return;
	}
	if (s_mctl_ch == NULL) {
		DPU_FB_ERR("s_mctl_ch is NULL\n");
		return;
	}

	s_mctl_ch->chn_mod_dbg = inp32(mctl_ch_dbg_base);
}

void hisi_dss_mctl_sys_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mctl_sys_base, dss_mctl_sys_t *s_mctl_sys, int ovl_idx)
{
	int k;

	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL!\n");
		return;
	}

	if (mctl_sys_base == NULL) {
		DPU_FB_DEBUG("mctl_sys_base is NULL!\n");
		return;
	}

	if (s_mctl_sys == NULL) {
		DPU_FB_DEBUG("s_mctl_sys is NULL!\n");
		return;
	}

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_DEBUG("ovl_idx is out of the range!\n");
		return;
	}

	if (s_mctl_sys->chn_ov_sel_used[ovl_idx]) {
		dpufd->set_reg(dpufd, mctl_sys_base + MCTL_RCH_OV0_SEL + (ovl_idx * 0x4),
			s_mctl_sys->chn_ov_sel[ovl_idx], 32, 0);

		dpufd->set_reg(dpufd, mctl_sys_base + MCTL_RCH_OV0_SEL1 + (ovl_idx * 0x4),
			s_mctl_sys->chn_ov_sel1[ovl_idx], 32, 0);
	}

	for (k = 0; k < DSS_WCH_MAX; k++) {
		if (s_mctl_sys->wch_ov_sel_used[k]) {
			dpufd->set_reg(dpufd, mctl_sys_base + MCTL_WCH_OV2_SEL + (k * 0x4),
				s_mctl_sys->wchn_ov_sel[k], 32, 0);
		}
	}

	if (s_mctl_sys->ov_flush_en_used[ovl_idx]) {
		dpufd->set_reg(dpufd, mctl_sys_base + MCTL_OV0_FLUSH_EN + (ovl_idx * 0x4),
			s_mctl_sys->ov_flush_en[ovl_idx], 32, 0);
	}
}

void hisi_dss_mctl_ov_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *mctl_base, dss_mctl_t *s_mctl, int ovl_idx, bool enable_cmdlist)
{
	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL!\n");
		return;
	}

	if (mctl_base == NULL) {
		DPU_FB_DEBUG("mctl_base is NULL!\n");
		return;
	}

	if (s_mctl == NULL) {
		DPU_FB_DEBUG("s_mctl is NULL!\n");
		return;
	}

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_DEBUG("ovl_idx  is is out of the range !\n");
		return;
	}

	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1)) {
		dpufd->set_reg(dpufd, mctl_base + MCTL_CTL_MUTEX_DBUF, s_mctl->ctl_mutex_dbuf, 32, 0);
		hisi_dss_mctl_ov_set_ctl_dbg_reg(dpufd, mctl_base, enable_cmdlist);
	}

	dpufd->set_reg(dpufd, mctl_base + MCTL_CTL_MUTEX_OV, s_mctl->ctl_mutex_ov, 32, 0);
}

void hisi_dss_mctl_ch_set_reg(struct dpu_fb_data_type *dpufd,
	dss_mctl_ch_base_t *mctl_ch_base, dss_mctl_ch_t *s_mctl_ch, int32_t mctl_idx, int chn_idx)
{
	char __iomem *chn_mutex_base = NULL;
	int i = 0;

	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is nullptr!\n");
	dpu_check_and_no_retval(!mctl_ch_base, ERR, "mctl_ch_base is nullptr!\n");
	dpu_check_and_no_retval(!s_mctl_ch, ERR, "s_mctl_ch is nullptr!\n");
	dpu_check_and_no_retval((mctl_idx < DSS_MCTL0) || (mctl_idx >= DSS_MCTL_IDX_MAX),
		ERR, "mctl_idx=%d!\n", mctl_idx);

	if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		chn_mutex_base = mctl_ch_base->chn_mutex_base + DSS_MCTRL_CTL0_OFFSET;
		/* precompose use mctl1 */
		if (dpufd->ov_req.wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON) {
			dpufd->set_reg(dpufd, chn_mutex_base, 0, 32, 0);
			chn_mutex_base =
				mctl_ch_base->chn_mutex_base + DSS_MCTRL_CTL1_OFFSET;
		}
		dpufd->set_reg(dpufd, chn_mutex_base, s_mctl_ch->chn_mutex, 32, 0);
	} else {
		for (i = 0; i < DSS_MCTL_IDX_MAX; i++) {
			if (g_dss_module_ovl_base[i][MODULE_MCTL_BASE] == 0)
				continue;
			chn_mutex_base = mctl_ch_base->chn_mutex_base +
				g_dss_module_ovl_base[i][MODULE_MCTL_BASE];

			if (i != mctl_idx)
				dpufd->set_reg(dpufd, chn_mutex_base, 0, 32, 0);
		}
		chn_mutex_base = mctl_ch_base->chn_mutex_base +
			g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
		dpu_check_and_no_retval(!chn_mutex_base, ERR, "chn_mutex_base is nullptr!\n");

		dpufd->set_reg(dpufd, chn_mutex_base, s_mctl_ch->chn_mutex, 32, 0);
	}
}

void hisi_dss_mctl_sys_ch_set_reg(struct dpu_fb_data_type *dpufd,
	dss_mctl_ch_base_t *mctl_ch_base, dss_mctl_ch_t *s_mctl_ch, int chn_idx, bool normal)
{
	if (dpufd == NULL) {
		DPU_FB_DEBUG("dpufd is NULL!\n");
		return;
	}

	if (mctl_ch_base == NULL) {
		DPU_FB_DEBUG("mctl_ch_base is NULL!\n");
		return;
	}

	if (s_mctl_ch == NULL) {
		DPU_FB_DEBUG("s_mctl_ch is NULL!\n");
		return;
	}

	if (mctl_ch_base->chn_ov_en_base)
		dpufd->set_reg(dpufd, mctl_ch_base->chn_ov_en_base, s_mctl_ch->chn_ov_oen, 32, 0);

	dpufd->set_reg(dpufd, mctl_ch_base->chn_flush_en_base, s_mctl_ch->chn_flush_en, 32, 0);
}

void hisi_dss_mctl_mutex_lock(struct dpu_fb_data_type *dpufd,
	int ovl_idx)
{
	char __iomem *mctl_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovl_idx is invalid\n");
		return;
	}

	mctl_base = dpufd->dss_module.mctl_base[ovl_idx];

	dpufd->set_reg(dpufd, mctl_base + MCTL_CTL_MUTEX, 0x1, 1, 0);
}

void hisi_dss_mctl_mutex_unlock(struct dpu_fb_data_type *dpufd,
	int ovl_idx)
{
	char __iomem *mctl_base = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovl_idx is invalid\n");
		return;
	}

	mctl_base = dpufd->dss_module.mctl_base[ovl_idx];

	dpufd->set_reg(dpufd, mctl_base + MCTL_CTL_MUTEX, 0x0, 1, 0);
}

static void set_mctl_mode_reg(struct dpu_fb_data_type *dpufd, int mctl_idx, char __iomem *mctl_base,
	char __iomem *mctl_sys_base, int tmp)
{
	int i;

	for (i = 0; i < tmp; i++) {
		if (i < MCTL_MOD_DBG_CH_NUM)
			set_reg(mctl_sys_base + MCTL_MOD0_DBG + i * 0x4, 0xA8000, 32, 0);
		else
			set_reg(mctl_sys_base + MCTL_MOD0_DBG + i * 0x4, 0xA0000, 32, 0);
	}

	for (i = 0; i < MCTL_MOD_DBG_ITF_NUM; i++)
		set_reg(mctl_sys_base + MCTL_MOD17_DBG + i * 0x4, 0xA0F00, 32, 0);

	set_reg(mctl_sys_base + MCTL_MOD19_DBG, 0xA8000, 32, 0);
}

void hisi_dss_mctl_on(struct dpu_fb_data_type *dpufd, int mctl_idx, bool enable_cmdlist, bool fastboot_enable)
{
	char __iomem *mctl_base = NULL;
	char __iomem *mctl_sys_base = NULL;
	int tmp = 0;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}
	if ((mctl_idx < DSS_MCTL0) || (mctl_idx >= DSS_MCTL_IDX_MAX)) {
		DPU_FB_ERR("mctl_idx is invalid\n");
		return;
	}

	if (g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE] == 0)
		return;

	mctl_base = dpufd->dss_base +
		g_dss_module_ovl_base[mctl_idx][MODULE_MCTL_BASE];
	mctl_sys_base = dpufd->dss_base + DSS_MCTRL_SYS_OFFSET;

	set_reg(mctl_base + MCTL_CTL_EN, 0x1, 32, 0);

	if ((mctl_idx == DSS_MCTL0) || (mctl_idx == DSS_MCTL1))
		set_reg(mctl_base + MCTL_CTL_MUTEX_ITF, mctl_idx + 1, 32, 0);

	if (enable_cmdlist) {
		tmp = MCTL_MOD_DBG_CH_NUM + MCTL_MOD_DBG_OV_NUM +
				MCTL_MOD_DBG_DBUF_NUM + MCTL_MOD_DBG_SCF_NUM;
		set_mctl_mode_reg(dpufd, mctl_idx, mctl_base, mctl_sys_base, tmp);
		if (!fastboot_enable)
			set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
	} else {
		set_reg(mctl_base + MCTL_CTL_DBG, 0xB13A00, 32, 0);
		if (is_mipi_cmd_panel(dpufd)) {
			set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
		} else {
			if (mctl_idx == DSS_MCTL0)
				set_reg(mctl_base + MCTL_CTL_TOP, 0x2, 32, 0);
			else if (mctl_idx == DSS_MCTL1)
				set_reg(mctl_base + MCTL_CTL_TOP, 0x3, 32, 0);
			else
				set_reg(mctl_base + MCTL_CTL_TOP, 0x1, 32, 0);
		}
	}
}

static int mctl_ch_config_wb_layer(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_wb_layer_t *wb_layer, int ovl_idx)
{
	int chn_idx = wb_layer->chn_idx;
	dss_mctl_sys_t *mctl_sys = NULL;
	dss_mctl_ch_t *mctl_ch = NULL;

	dpu_check_and_return((chn_idx < 0) || (chn_idx >= DSS_CHN_MAX_DEFINE), -EINVAL,
		ERR, "wb_layer->chn_idx exceeds array limit\n");

	mctl_sys = &(dpufd->dss_module.mctl_sys);
	dpufd->dss_module.mctl_sys_used = 1;
	mctl_ch = &(dpufd->dss_module.mctl_ch[chn_idx]);
	dpufd->dss_module.mctl_ch_used[chn_idx] = 1;

	if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		if (pov_req->wb_compose_type == DSS_WB_COMPOSE_COPYBIT) {
			mctl_ch->chn_ov_oen = set_bits32(mctl_ch->chn_ov_oen, 3, 32, 0);
		} else {
			mctl_ch->chn_ov_oen = set_bits32(mctl_ch->chn_ov_oen, 1, 32, 0);
			mctl_sys->wchn_ov_sel[0] = set_bits32(mctl_sys->wchn_ov_sel[0],
				(chn_idx - DSS_WCHN_W0 + 1), 32, 0);
			mctl_sys->wch_ov_sel_used[0] = 1;
		}
	} else {
		if (chn_idx != DSS_WCHN_W2) {  /* chicago copybit */
			mctl_ch->chn_ov_oen = set_bits32(mctl_ch->chn_ov_oen, (ovl_idx - 1), 32, 0);

			if (pov_req->wb_layer_nums == MAX_DSS_DST_NUM) {
				mctl_sys->wchn_ov_sel[0] = set_bits32(mctl_sys->wchn_ov_sel[0], 3, 32, 0);
				mctl_sys->wch_ov_sel_used[0] = 1;
				mctl_sys->wchn_ov_sel[1] = set_bits32(mctl_sys->wchn_ov_sel[1], 3, 32, 0);
				mctl_sys->wch_ov_sel_used[1] = 1;
			} else {
				mctl_sys->wchn_ov_sel[ovl_idx - DSS_OVL2] =
					set_bits32(mctl_sys->wchn_ov_sel[ovl_idx - DSS_OVL2],
						(chn_idx - DSS_WCHN_W0 + 1), 32, 0);
				mctl_sys->wch_ov_sel_used[ovl_idx - DSS_OVL2] = 1;
			}
		}
	}

	mctl_ch->chn_mutex = set_bits32(mctl_ch->chn_mutex, 0x1, 1, 0);
	mctl_ch->chn_flush_en = set_bits32(mctl_ch->chn_flush_en, 0x1, 1, 0);

	return 0;
}

static void mctl_ch_config_layer_need_cap(struct dpu_fb_data_type *dpufd, int ovl_idx,
		int layer_idx, int ch_ov_sel_pattern)
{
	/* ov sel up to 7 channels, display register 32bit, single channel 4bit,
	 * except for base channel, 7 channels remain
	 */
	int chn_ov_sel_max_num = 7;
	dss_mctl_sys_t *mctl_sys = NULL;

	mctl_sys = &(dpufd->dss_module.mctl_sys);

	if (layer_idx < chn_ov_sel_max_num) {
		mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx],
			ch_ov_sel_pattern, 4, (layer_idx + 1) * 4);
		mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
	} else {
		mctl_sys->chn_ov_sel1[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel1[ovl_idx],
			ch_ov_sel_pattern, 4, 0);
		mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
	}
}

static int mctl_ch_config_layer(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
		int ovl_idx, dss_rect_t *wb_ov_block_rect, bool has_base)
{
	int chn_idx = layer->chn_idx;
	int layer_idx;
	int ch_ov_sel_pattern = 0xE;  /* default valve */
	/* ov sel up to 7 channels, display register 32bit, single channel 4bit,
	 * except for base channel, 7 channels remain
	 */
	int chn_ov_sel_max_num = 7;
	dss_mctl_sys_t *mctl_sys = NULL;
	dss_mctl_ch_t *mctl_ch = NULL;

	dpu_check_and_return((chn_idx < 0) || (chn_idx >= DSS_CHN_MAX_DEFINE),
		-EINVAL, ERR, "layer->chn_idx exceeds array limit\n");

	layer_idx = layer->layer_idx;

	if (layer->need_cap & CAP_BASE)
		return 0;

	if (has_base) {
		layer_idx -= 1;
		dpu_check_and_return((layer_idx < 0), -EINVAL, ERR,
			"fb%d, layer_idx(%d) is out of range!\n", dpufd->index, layer_idx);
	}
	mctl_sys = &(dpufd->dss_module.mctl_sys);
	dpufd->dss_module.mctl_sys_used = 1;

	if (layer->need_cap & (CAP_DIM | CAP_PURE_COLOR)) {
		mctl_ch_config_layer_need_cap(dpufd, ovl_idx, layer_idx, ch_ov_sel_pattern);
	} else {
		mctl_ch = &(dpufd->dss_module.mctl_ch[chn_idx]);
		dpufd->dss_module.mctl_ch_used[chn_idx] = 1;

		mctl_ch->chn_mutex = set_bits32(mctl_ch->chn_mutex, 0x1, 1, 0);
		mctl_ch->chn_flush_en = set_bits32(mctl_ch->chn_flush_en, 0x1, 1, 0);

		mctl_ch->chn_ov_oen = set_bits32(mctl_ch->chn_ov_oen, 0x100 << (uint32_t)ovl_idx, 32, 0);
		if (wb_ov_block_rect != NULL)
			mctl_ch->chn_starty = set_bits32(mctl_ch->chn_starty,
				((uint32_t)(layer->dst_rect.y - wb_ov_block_rect->y) | (0x8 << 16)), 32, 0);
		else
			mctl_ch->chn_starty = set_bits32(mctl_ch->chn_starty,
				((uint32_t)layer->dst_rect.y | (0x8 << 16)), 32, 0);

		if (chn_idx == DSS_RCHN_V2)
			chn_idx = 0x8;

		if (layer_idx < chn_ov_sel_max_num) {
			mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx],
				chn_idx, 4, (layer_idx + 1) * 4);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		} else {
			mctl_sys->chn_ov_sel1[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel1[ovl_idx],
				chn_idx, 4, 0);
			mctl_sys->chn_ov_sel_used[ovl_idx] = 1;
		}
	}

	return 0;
}

int hisi_dss_mctl_ch_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer,
	dss_wb_layer_t *wb_layer, dss_rect_t *wb_ov_block_rect, bool has_base)
{
	int ret;
	dss_overlay_t *pov_req = NULL;
	int ovl_idx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!layer && !wb_layer, -EINVAL, ERR, "layer or wb_layer is nullptr!\n");
	pov_req = &(dpufd->ov_req);
	ovl_idx = pov_req->ovl_idx;

	if (wb_layer != NULL) {
		ret = mctl_ch_config_wb_layer(dpufd, pov_req, wb_layer, ovl_idx);
		if (ret < 0) {
			DPU_FB_ERR("mctl_ch_config_wb_layer failed!\n");
			return -EINVAL;
		}
	} else if (layer != NULL) {
		ret = mctl_ch_config_layer(dpufd, layer, ovl_idx, wb_ov_block_rect, has_base);
		if (ret < 0) {
			DPU_FB_ERR("mctl_ch_config_layer failed!\n");
			return -EINVAL;
		}
	}

	return 0;
}

static void hisi_dss_mctl_mutex_config(struct dpu_fb_data_type *dpufd, int ovl_idx)
{
	dss_mctl_t *mctl = NULL;

	mctl = &(dpufd->dss_module.mctl[ovl_idx]);
	dpufd->dss_module.mctl_used[ovl_idx] = 1;

	if (ovl_idx == DSS_OVL0) {
		mctl->ctl_mutex_itf = set_bits32(mctl->ctl_mutex_itf, 0x1, 2, 0);
		mctl->ctl_mutex_dbuf = set_bits32(mctl->ctl_mutex_dbuf, 0x1, 2, 0);
	} else if (ovl_idx == DSS_OVL1) {
		mctl->ctl_mutex_itf = set_bits32(mctl->ctl_mutex_itf, 0x2, 2, 0);
		mctl->ctl_mutex_dbuf = set_bits32(mctl->ctl_mutex_dbuf, 0x2, 2, 0);
	} else {
		;
	}

	mctl->ctl_mutex_ov = set_bits32(mctl->ctl_mutex_ov, 1 << (uint32_t)ovl_idx, 4, 0);
}

static void hisi_dss_mctl_sys_config(struct dpu_fb_data_type *dpufd,
		int ovl_idx, bool is_first_ov_block)
{
	dss_mctl_sys_t *mctl_sys = NULL;

	mctl_sys = &(dpufd->dss_module.mctl_sys);
	dpufd->dss_module.mctl_sys_used = 1;

	/* ov base pattern */
#ifdef SUPPORT_FULL_CHN_10BIT_COLOR
	mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx], 0xE, 4, 0);
#else
	mctl_sys->chn_ov_sel[ovl_idx] = set_bits32(mctl_sys->chn_ov_sel[ovl_idx], 0x8, 4, 0);
#endif
	mctl_sys->chn_ov_sel_used[ovl_idx] = 1;

	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1)) {
		if (is_first_ov_block) {
			mctl_sys->ov_flush_en[ovl_idx] = set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0xd, 4, 0);
		} else {
			mctl_sys->ov_flush_en[ovl_idx] = set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0x1, 1, 0);
		}
		mctl_sys->ov_flush_en_used[ovl_idx] = 1;
	} else {
		mctl_sys->ov_flush_en[ovl_idx] = set_bits32(mctl_sys->ov_flush_en[ovl_idx], 0x1, 1, 0);
		mctl_sys->ov_flush_en_used[ovl_idx] = 1;
	}
}

int hisi_dss_mctl_ov_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	int ovl_idx, bool has_base, bool is_first_ov_block)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovl_idx is invalid\n");
		return -EINVAL;
	}

	if ((pov_req != NULL) && pov_req->wb_layer_infos[0].chn_idx == DSS_WCHN_W2)  /* chicago copybit no ovl */
		return 0;

	/* MCTL_MUTEX */
	hisi_dss_mctl_mutex_config(dpufd, ovl_idx);

	/* MCTL_SYS */
	hisi_dss_mctl_sys_config(dpufd, ovl_idx, is_first_ov_block);

	return 0;
}
