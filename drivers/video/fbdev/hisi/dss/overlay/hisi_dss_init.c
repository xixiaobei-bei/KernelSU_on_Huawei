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
#include "../hisi_dss_dpm.h"
#ifdef CONFIG_HISI_FB_MEDIACOMMON
#include "../mediacommon/hisi_mdc_overlay.h"
#endif


/* 128 bytes */
#define SMMU_RW_ERR_ADDR_SIZE 128

#define WORKQUEUE_NAME_SIZE 128

#define DSS_MODULE_POST_SCF 0
#define DSS_MODULE_CLD 1
#define DSS_MODULE_MCTRL_SYS 2
#define DSS_MODULE_SMMU 3

#define VIDEO_MODE 0
#define CMD_MODE 1

#define hisi_dss_destroy_wq(wq) \
	do { \
		if (wq != NULL) { \
			destroy_workqueue(wq); \
			wq = NULL; \
		} \
	} while (0)

/* init when power on */
uint32_t g_underflow_count;  /*lint !e552*/

static int g_dss_sr_refcount;

static uint32_t g_vactive_timeout_count;
static uint32_t g_dsm_vactive0_timeout_count;

struct cmdlist_idxs_sq {
	uint32_t cur;
	uint32_t prev;
	uint32_t prev_prev;
};

struct dsm_data {
	struct dpu_fb_data_type *dpufd;
	dss_overlay_t *pov_req;
	int ret;
	struct cmdlist_idxs_sq cmd_idx;
	uint32_t time_diff;
	uint32_t *read_value;
};

struct single_ch_module_init {
	enum dss_chn_module module_id;
	void (*reg_ch_module_init)(char __iomem *dss_base, uint32_t module_base,
		dss_module_reg_t *dss_module, uint32_t ch);
};

struct single_module_init {
	uint32_t module_offset;
	void (*reg_module_init)(char __iomem *dss_base, uint32_t module_base, dss_module_reg_t *dss_module);
};

enum vactive0_panel_type {
	PANEL_CMD,
	PANEL_CMD_RESET,
	PANEL_VIDEO,
};

void dpufb_dss_overlay_info_init(dss_overlay_t *ov_req)
{
	if (ov_req == NULL)
		return;

	memset(ov_req, 0, sizeof(dss_overlay_t));
	ov_req->release_fence = -1;
	ov_req->retire_fence = -1;
}


int hisi_dss_module_init(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		memcpy(&(dpufd->dss_module), &(dpufd->dss_mdc_module_default), sizeof(dss_module_reg_t));
	else
		memcpy(&(dpufd->dss_module), &(dpufd->dss_module_default), sizeof(dss_module_reg_t));

	return 0;
}

static void module_aif0_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->aif_ch_base[ch] = dss_base + module_base;
	hisi_dss_aif_init(dss_module->aif_ch_base[ch], &(dss_module->aif[ch]));
}

static void module_aif1_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->aif1_ch_base[ch] = dss_base + module_base;
	hisi_dss_aif_init(dss_module->aif1_ch_base[ch], &(dss_module->aif1[ch]));
}

static void module_mif_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mif_ch_base[ch] = dss_base + module_base;
	hisi_dss_mif_init(dss_module->mif_ch_base[ch], &(dss_module->mif[ch]), ch);
}

static void module_mctl_chn_mutex_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mctl_ch_base[ch].chn_mutex_base = dss_base + module_base;
}

static void module_mctl_chn_flush_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mctl_ch_base[ch].chn_flush_en_base = dss_base + module_base;
}

static void module_mctl_chn_ov_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mctl_ch_base[ch].chn_ov_en_base = dss_base + module_base;
}

static void module_mctl_chn_starty_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mctl_ch_base[ch].chn_starty_base = dss_base + module_base;
	hisi_dss_mctl_ch_starty_init(dss_module->mctl_ch_base[ch].chn_starty_base,
		&(dss_module->mctl_ch[ch]));
}

static void module_mctl_chn_mod_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->mctl_ch_base[ch].chn_mod_dbg_base = dss_base + module_base;
	hisi_dss_mctl_ch_mod_dbg_init(dss_module->mctl_ch_base[ch].chn_mod_dbg_base,
		&(dss_module->mctl_ch[ch]));
}

static void module_dma_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->dma_base[ch] = dss_base + module_base;
	if (ch < DSS_WCHN_W0 || ch == DSS_RCHN_V2)
		hisi_dss_rdma_init(dss_module->dma_base[ch], &(dss_module->rdma[ch]));
	else
		hisi_dss_wdma_init(dss_module->dma_base[ch], &(dss_module->wdma[ch]));

	if ((ch == DSS_RCHN_V0) || (ch == DSS_RCHN_V1) || (ch == DSS_RCHN_V2)) {
		hisi_dss_rdma_u_init(dss_module->dma_base[ch], &(dss_module->rdma[ch]));
		hisi_dss_rdma_v_init(dss_module->dma_base[ch], &(dss_module->rdma[ch]));
	}
}

static void module_dfc_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->dfc_base[ch] = dss_base + module_base;
	hisi_dss_dfc_init(dss_module->dfc_base[ch], &(dss_module->dfc[ch]));
}

static void module_scl_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->scl_base[ch] = dss_base + module_base;
	hisi_dss_scl_init(dss_module->scl_base[ch], &(dss_module->scl[ch]));
}

static void module_pcsc_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->pcsc_base[ch] = dss_base + module_base;
	hisi_dss_csc_init(dss_module->pcsc_base[ch], &(dss_module->pcsc[ch]));
}

static void module_arsr2p_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->arsr2p_base[ch] = dss_base + module_base;
	hisi_dss_arsr2p_init(dss_module->arsr2p_base[ch], &(dss_module->arsr2p[ch]));
}

static void module_post_clip_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->post_clip_base[ch] = dss_base + module_base;
	hisi_dss_post_clip_init(dss_module->post_clip_base[ch], &(dss_module->post_clip[ch]));
}

static void module_csc_init(char __iomem *dss_base, uint32_t module_base,
	dss_module_reg_t *dss_module, uint32_t ch)
{
	dss_module->csc_base[ch] = dss_base + module_base;
	hisi_dss_csc_init(dss_module->csc_base[ch], &(dss_module->csc[ch]));
}

static void module_post_scf_init(char __iomem *dss_base, uint32_t module_base, dss_module_reg_t *dss_module)
{
	dss_module->post_scf_base = dss_base + module_base;
	hisi_dss_post_scf_init(dss_base, dss_module->post_scf_base, &(dss_module->post_scf));
}


static void module_mctl_sys_init(char __iomem *dss_base, uint32_t module_base, dss_module_reg_t *dss_module)
{
	dss_module->mctl_sys_base = dss_base + module_base;
	hisi_dss_mctl_sys_init(dss_module->mctl_sys_base, &(dss_module->mctl_sys));
}

static void module_smmu_init(char __iomem *dss_base, uint32_t module_base, dss_module_reg_t *dss_module)
{
	dss_module->smmu_base = dss_base + module_base;
	hisi_dss_smmu_init(dss_module->smmu_base, &(dss_module->smmu));
}

static struct single_ch_module_init g_dss_ch_module_init[] = {
	{ MODULE_AIF0_CHN, module_aif0_init },
	{ MODULE_AIF1_CHN, module_aif1_init },
	{ MODULE_MIF_CHN, module_mif_init },
	{ MODULE_MCTL_CHN_MUTEX, module_mctl_chn_mutex_init },
	{ MODULE_MCTL_CHN_FLUSH_EN, module_mctl_chn_flush_init },
	{ MODULE_MCTL_CHN_OV_OEN, module_mctl_chn_ov_init },
	{ MODULE_MCTL_CHN_STARTY, module_mctl_chn_starty_init },
	{ MODULE_MCTL_CHN_MOD_DBG, module_mctl_chn_mod_init },
	{ MODULE_DMA, module_dma_init },
	{ MODULE_DFC, module_dfc_init },
	{ MODULE_SCL, module_scl_init },
	{ MODULE_PCSC, module_pcsc_init },
	{ MODULE_ARSR2P, module_arsr2p_init },
	{ MODULE_POST_CLIP, module_post_clip_init },
	{ MODULE_CSC, module_csc_init },
};

static struct single_module_init g_dss_module_init[] = {
	{ DSS_POST_SCF_OFFSET, module_post_scf_init},
	{ DSS_MCTRL_SYS_OFFSET, module_mctl_sys_init},
	{ DSS_SMMU_OFFSET, module_smmu_init},
};

int hisi_dss_module_default(struct dpu_fb_data_type *dpufd)
{
	dss_module_reg_t *dss_module = NULL;
	uint32_t module_base;
	char __iomem *dss_base = NULL;
	uint32_t i;
	uint32_t j;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");

	dss_base = dpufd->dss_base;
	dpu_check_and_return((dss_base == NULL), -EINVAL, ERR, "dss_base is NULL\n");

	dss_module = &(dpufd->dss_module_default);
	memset(dss_module, 0, sizeof(dss_module_reg_t));

	for (i = 0; i < DSS_MCTL_IDX_MAX; i++) {
		module_base = g_dss_module_ovl_base[i][MODULE_MCTL_BASE];
		if (module_base != 0) {
			dss_module->mctl_base[i] = dss_base + module_base;
			hisi_dss_mctl_init(dss_module->mctl_base[i], &(dss_module->mctl[i]));
		}
	}

	for (i = 0; i < DSS_OVL_IDX_MAX; i++) {
		module_base = g_dss_module_ovl_base[i][MODULE_OVL_BASE];
		if (module_base != 0) {
			dss_module->ov_base[i] = dss_base + module_base;
			hisi_dss_ovl_init(dss_module->ov_base[i], &(dss_module->ov[i]), i);
		}
	}

	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
		defined(CONFIG_HISI_FB_V510) || defined(CONFIG_HISI_FB_V600)
		if (i == DSS_WCHN_W2)
			continue;
#endif
		for (j = 0; j < ARRAY_SIZE(g_dss_ch_module_init); j++) {
			module_base = g_dss_module_base[i][g_dss_ch_module_init[j].module_id];
			if (module_base != 0)
				g_dss_ch_module_init[j].reg_ch_module_init(dss_base, module_base, dss_module, i);
		}
	}

	for (j = 0; j < ARRAY_SIZE(g_dss_module_init); j++) {
		module_base = g_dss_module_init[j].module_offset;
		if (module_base != 0)
			g_dss_module_init[j].reg_module_init(dss_base, module_base, dss_module);
	}

	return 0;
}

static void dpufb_dss_on(struct dpu_fb_data_type *dpufd, int enable_cmdlist)
{
	int prev_refcount;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	hisi_mmbuf_sem_pend();

	prev_refcount = g_dss_sr_refcount++;
	if (!prev_refcount) {
		/* dss qos on */
		hisi_dss_qos_on(dpufd);
		/* mif on */
		hisi_dss_mif_on(dpufd);
		/* smmu on */
		hisi_dss_smmu_on(dpufd);
		/* scl coef load */
		hisi_dss_scl_coef_on(dpufd, false, SCL_COEF_YUV_IDX);  /*lint !e747*/

		if (enable_cmdlist)
			hisi_dss_cmdlist_qos_on(dpufd);
	}
	hisi_mmbuf_sem_post();


	DPU_FB_DEBUG("fb%d, -, g_dss_sr_refcount=%d\n", dpufd->index, g_dss_sr_refcount);
}


void dpufb_dss_off(struct dpu_fb_data_type *dpufd, bool is_lp)
{
	int new_refcount;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return;

	DPU_FB_DEBUG("fb%d, +.\n", dpufd->index);

	hisi_mmbuf_sem_pend();
	new_refcount = --g_dss_sr_refcount;
	if (new_refcount < 0)
		DPU_FB_ERR("dss new_refcount err\n");

	if (is_lp) {
		if (!new_refcount) {
			dpufd->ldi_data_gate_en = 0;

			memset(dpufd->ov_block_infos_prev_prev, 0,
				HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
			memset(dpufd->ov_block_infos_prev, 0,
				HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
			memset(dpufd->ov_block_infos, 0,
				HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));

			dpufb_dss_overlay_info_init(&dpufd->ov_req);
			dpufb_dss_overlay_info_init(&dpufd->ov_req_prev);
			dpufb_dss_overlay_info_init(&dpufd->ov_req_prev_prev);
		}
	}

	if (is_lp) {
		hisi_mmbuf_sem_post();
		return;
	}

	if (!new_refcount)
		hisi_dss_mmbuf_free_all(dpufd);

	hisi_mmbuf_sem_post();

	DPU_FB_DEBUG("fb%d, -, g_dss_sr_refcount=%d\n", dpufd->index, g_dss_sr_refcount);
}

static int hisi_overlay_fastboot(struct dpu_fb_data_type *dpufd)
{
	dss_overlay_t *pov_req_prev = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_h_block = NULL;
	dss_layer_t *layer = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		pov_req_prev = &(dpufd->ov_req_prev);
		memset(pov_req_prev, 0, sizeof(dss_overlay_t));
		pov_req_prev->ov_block_infos_ptr = (uint64_t)(uintptr_t)(dpufd->ov_block_infos_prev);
		pov_req_prev->ov_block_nums = 1;
		pov_req_prev->ovl_idx = DSS_OVL0;
		pov_req_prev->release_fence = -1;
		pov_req_prev->retire_fence = -1;

		pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)pov_req_prev->ov_block_infos_ptr;
		pov_h_block = &(pov_h_block_infos[0]);
		pov_h_block->layer_nums = 1;

		layer = &(pov_h_block->layer_infos[0]);
		layer->img.mmu_enable = 0;
		layer->layer_idx = 0x0;
		layer->chn_idx = DSS_RCHN_D0;
		layer->need_cap = 0;

		memcpy(&(dpufd->dss_module_default.rdma[DSS_RCHN_D0]), &(dpufd->dss_module_default.rdma[DSS_RCHN_D3]),
			sizeof(dss_rdma_t));
		memcpy(&(dpufd->dss_module_default.dfc[DSS_RCHN_D0]), &(dpufd->dss_module_default.dfc[DSS_RCHN_D3]),
			sizeof(dss_dfc_t));
		memcpy(&(dpufd->dss_module_default.ov[DSS_OVL0].ovl_layer[0]),
			&(dpufd->dss_module_default.ov[DSS_OVL0].ovl_layer[1]), sizeof(dss_ovl_layer_t));

		memset(&(dpufd->dss_module_default.mctl_ch[DSS_RCHN_D0]), 0, sizeof(dss_mctl_ch_t));
		memset(&(dpufd->dss_module_default.mctl[DSS_OVL0]), 0, sizeof(dss_mctl_t));

		dpufd->dss_module_default.mctl_sys.chn_ov_sel[DSS_OVL0] = 0xFFFFFFFF;  /* default valve */
		dpufd->dss_module_default.mctl_sys.ov_flush_en[DSS_OVL0] = 0x0;

		if (is_mipi_cmd_panel(dpufd)) {
			if (dpufd->vactive0_start_flag == 0) {
				dpufd->vactive0_start_flag = 1;
				dpufd->vactive0_end_flag = 1;
			}
		}
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}

static void hisi_overlay_on_param_init(struct dpu_fb_data_type *dpufd)
{
	dpufd->vactive0_start_flag = 0;
	dpufd->vactive0_end_flag = 0;
	dpufd->underflow_flag = 0;
	dpufd->crc_flag = 0;
	dpufd->dirty_region_updt.x = 0;
	dpufd->dirty_region_updt.y = 0;
	dpufd->dirty_region_updt.w = dpufd->panel_info.xres;
	dpufd->dirty_region_updt.h = dpufd->panel_info.yres;
	dpufd->resolution_rect.x = 0;
	dpufd->resolution_rect.y = 0;
	dpufd->resolution_rect.w = dpufd->panel_info.xres;
	dpufd->resolution_rect.h = dpufd->panel_info.yres;

	dpufb_dss_overlay_info_init(&dpufd->ov_req);
	dpufd->ov_req.frame_no = 0;
	memset(&dpufd->acm_ce_info, 0, sizeof(dpufd->acm_ce_info));
	memset(dpufd->prefix_ce_info, 0, sizeof(dpufd->prefix_ce_info));
	g_offline_cmdlist_idxs = 0;
}

static void hisi_overlay_reg_modules_init(struct dpu_fb_data_type *dpufd)
{
	static uint32_t index;

	if (g_dss_module_resource_initialized == 0) {
		hisi_dss_module_default(dpufd);
		g_dss_module_resource_initialized = 1;
		dpufd->dss_module_resource_initialized = true;
		index = dpufd->index;
	} else {
		if (!dpufd->dss_module_resource_initialized) {
			if ((dpufd->index != index) && (dpufd_list[index]))
				memcpy(&(dpufd->dss_module_default), &(dpufd_list[index]->dss_module_default),
					sizeof(dpufd->dss_module_default));
			dpufd->dss_module_resource_initialized = true;
		}
	}
}

static bool hisi_overlay_need_udpate_frame(struct dpu_fb_data_type *dpufd)
{
	struct dpu_fb_panel_data *pdata = NULL;

#if defined(CONFIG_HISI_FB_AOD) || defined(CONFIG_DPU_FB_AP_AOD)
	if ((dpufd->aod_function || dpufd->base_frame_mode) && is_mipi_cmd_panel(dpufd))
#else
	if (dpufd->base_frame_mode && is_mipi_cmd_panel(dpufd))
#endif
	{
		DPU_FB_INFO("AOD is enable, disable base frame\n");
		dpufd->vactive0_start_flag = 1;
		dpufd->vactive0_end_flag = 1;

		pdata = dev_get_platdata(&dpufd->pdev->dev);
		if ((pdata != NULL) && pdata->set_display_region)
			pdata->set_display_region(dpufd->pdev, &dpufd->dirty_region_updt);

		return false;
	}

	return true;
}

static int hisi_dss_ovl_base_config_default(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_rect_t *wb_ov_block_rect, int ovl_idx)
{
	int ret;
	dss_ovl_t *ovl = NULL;
	struct img_size img_info = {0};
	int block_size = 0x7FFF;  /* default max reg valve */

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	if ((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)) {
		DPU_FB_ERR("ovl_idx is invalid\n");
		return -EINVAL;
	}

	if (pov_req && pov_req->wb_layer_infos[0].chn_idx == DSS_WCHN_W2)  /* chicago copybit no ovl */
		return 0;

	ovl = &(dpufd->dss_module.ov[ovl_idx]);
	dpufd->dss_module.ov_used[ovl_idx] = 1;

	ret = get_img_size_info(dpufd, pov_req, wb_ov_block_rect, &img_info);
	dpu_check_and_return((ret != 0), -ret, ERR, "get_img_size_info error!\n");

	if (wb_ov_block_rect == NULL &&
		is_arsr_post_need_padding(pov_req, &(dpufd->panel_info), ARSR1P_INC_FACTOR)) {
		img_info.img_width += ARSR_POST_COLUMN_PADDING_NUM;
		DPU_FB_DEBUG("img_width = %d, padding %d column\n", img_info.img_width, ARSR_POST_COLUMN_PADDING_NUM);
	}

	set_ovl_struct(dpufd, ovl, img_info, block_size);

	return ret;
}


static int hisi_overlay_on_update_frame(struct dpu_fb_data_type *dpufd, int enable_cmdlist)
{
	int ret;
	int ovl_idx;
	int mctl_idx;
	uint32_t cmdlist_idxs = 0;
	struct ov_module_set_regs_flag ov_module_flag = { enable_cmdlist, true, 1, 0 };

	ovl_idx = (dpufd->index == PRIMARY_PANEL_IDX) ? DSS_OVL0 : DSS_OVL1;
	mctl_idx = (dpufd->index == PRIMARY_PANEL_IDX) ? DSS_MCTL0 : DSS_MCTL1;

	ret = hisi_dss_module_init(dpufd);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to module_init! ret=%d\n", dpufd->index, ret);

	if (!hisi_overlay_need_udpate_frame(dpufd))
		return -1;

	if (enable_cmdlist) {
		dpufd->set_reg = hisi_cmdlist_set_reg;
		hisi_cmdlist_data_get_online(dpufd);
		cmdlist_idxs = (0x1 << (uint32_t)(ovl_idx + DSS_CMDLIST_OV0));
		hisi_cmdlist_add_nop_node(dpufd, cmdlist_idxs, 0, 0);
	} else {
		dpufd->set_reg = dpufb_set_reg;
		hisi_dss_mctl_mutex_lock(dpufd, ovl_idx);
	}

	dpufd->ov_req_prev.ovl_idx = ovl_idx;
	ret = hisi_dss_ovl_base_config_default(dpufd, NULL, NULL, ovl_idx);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to ovl_base_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_mctl_ov_config(dpufd, NULL, ovl_idx, false, true);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to mctl_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_ov_module_set_regs(dpufd, NULL, ovl_idx, ov_module_flag);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to module_config! ret=%d\n", dpufd->index, ret);

	if (enable_cmdlist) {
		hisi_cmdlist_flush_cache(dpufd, cmdlist_idxs);
		hisi_cmdlist_config_start(dpufd, mctl_idx, cmdlist_idxs, 0);
	} else {
		hisi_dss_mctl_mutex_unlock(dpufd, ovl_idx);
	}

	single_frame_update(dpufd);
	enable_ldi(dpufd);
	dpufb_frame_updated(dpufd);
	dpufd->frame_count++;

	if (g_debug_ovl_cmdlist)
		hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs);

	return 0;
}

int hisi_overlay_on(struct dpu_fb_data_type *dpufd, bool fastboot_enable)
{
	int mctl_idx = 0;
	int enable_cmdlist;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);

	hisi_overlay_on_param_init(dpufd);
	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return 0;

	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX))
		dpufb_activate_vsync(dpufd);

	hisi_overlay_reg_modules_init(dpufd);
	enable_cmdlist = g_enable_ovl_cmdlist_online;
	/* dss on */
	dpufb_dss_on(dpufd, enable_cmdlist);

	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX)) {
		mctl_idx = (dpufd->index == PRIMARY_PANEL_IDX) ? DSS_MCTL0 : DSS_MCTL1;
		if ((dpufd->index == EXTERNAL_PANEL_IDX) && dpufd->panel_info.fake_external)
			enable_cmdlist = 0;

		ldi_data_gate(dpufd, true);
		hisi_dss_mctl_on(dpufd, mctl_idx, enable_cmdlist, fastboot_enable);
		if (fastboot_enable)
			hisi_overlay_fastboot(dpufd);
		else
			hisi_overlay_on_update_frame(dpufd, enable_cmdlist);

		dpufb_deactivate_vsync(dpufd);
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		enable_cmdlist = g_enable_ovl_cmdlist_offline;
		hisi_dss_mctl_on(dpufd, DSS_MCTL2, enable_cmdlist, 0);
		hisi_dss_mctl_on(dpufd, DSS_MCTL3, enable_cmdlist, 0);
		hisi_dss_mctl_on(dpufd, DSS_MCTL5, enable_cmdlist, 0);
	} else {
		DPU_FB_ERR("fb%d, not supported!\n", dpufd->index);
	}

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	if (dpufd->index == PRIMARY_PANEL_IDX)  /*lint -e548*/
		hisi_dss_dpm_init(dpufd);

	return 0;
}

static void hisi_overlay_reset_param(struct dpu_fb_data_type *dpufd)
{
	dpufd->ldi_data_gate_en = 0;
	dpufd->masklayer_maxbacklight_flag = false;

	memset(dpufd->ov_block_infos_prev_prev, 0, HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
	memset(dpufd->ov_block_infos_prev, 0, HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
	memset(dpufd->ov_block_infos, 0, HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));

	dpufb_dss_overlay_info_init(&dpufd->ov_req);
	dpufb_dss_overlay_info_init(&dpufd->ov_req_prev);
	dpufb_dss_overlay_info_init(&dpufd->ov_req_prev_prev);

	memset(dpufd->effect_updated_flag, 0, sizeof(dpufd->effect_updated_flag));
}

static int hisi_overlay_off_cmdlist_config(struct dpu_fb_data_type *dpufd, int enable_cmdlist)
{
	int ret;
	int ovl_idx = (dpufd->index == PRIMARY_PANEL_IDX) ? DSS_OVL0 : DSS_OVL1;
	uint32_t cmdlist_pre_idxs = 0;
	uint32_t cmdlist_idxs = 0;
	dss_overlay_t *pov_req_prev = &(dpufd->ov_req_prev);
	struct ov_module_set_regs_flag ov_module_flag = { enable_cmdlist, true, 1, 0 };

	if (enable_cmdlist) {
		dpufd->set_reg = hisi_cmdlist_set_reg;

		hisi_cmdlist_data_get_online(dpufd);

		ret = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmdlist_pre_idxs, NULL);
		dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to get_cmdlist_idxs! ret=%d\n",
			dpufd->index, ret);

		cmdlist_idxs = (1 << (uint32_t)(DSS_CMDLIST_OV0 + ovl_idx));
		cmdlist_pre_idxs &= (~(cmdlist_idxs));

		hisi_cmdlist_add_nop_node(dpufd, cmdlist_pre_idxs, 0, 0);
		hisi_cmdlist_add_nop_node(dpufd, cmdlist_idxs, 0, 0);
	} else {
		dpufd->set_reg = dpufb_set_reg;

		hisi_dss_mctl_mutex_lock(dpufd, ovl_idx);
		cmdlist_pre_idxs = ~0;
	}

	hisi_dss_prev_module_set_regs(dpufd, pov_req_prev, cmdlist_pre_idxs, enable_cmdlist, NULL);

	ret = hisi_dss_ovl_base_config_default(dpufd, NULL, NULL, ovl_idx);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to ovl_base_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_mctl_ov_config(dpufd, NULL, ovl_idx, false, true);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to mctl_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_post_scf_config(dpufd, NULL);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to post_scf_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_dirty_region_dbuf_config(dpufd, NULL);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to dirty_region_config! ret=%d\n", dpufd->index, ret);

	ret = hisi_dss_ov_module_set_regs(dpufd, NULL, ovl_idx, ov_module_flag);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to module_config! ret=%d\n", dpufd->index, ret);

	if (enable_cmdlist) {
		hisi_cmdlist_config_stop(dpufd, cmdlist_pre_idxs);

		cmdlist_idxs |= cmdlist_pre_idxs;
		hisi_cmdlist_flush_cache(dpufd, cmdlist_idxs);

		if (g_debug_ovl_cmdlist)
			hisi_cmdlist_dump_all_node(dpufd, NULL, cmdlist_idxs);

		hisi_cmdlist_config_start(dpufd, ovl_idx, cmdlist_idxs, 0);
	} else {
		hisi_dss_mctl_mutex_unlock(dpufd, ovl_idx);
	}

	return 0;
}

static int hisi_overlay_off_update_frame(struct dpu_fb_data_type *dpufd)
{
	int ret;
	int enable_cmdlist;

#if defined(CONFIG_HISI_FB_AOD) || defined(CONFIG_DPU_FB_AP_AOD)
	if ((dpufd->aod_function == 1) && (dpufd_list[EXTERNAL_PANEL_IDX] &&
		!dpufd_list[EXTERNAL_PANEL_IDX]->panel_power_on)) {
		DPU_FB_INFO("fb%d, aod mode,no base frame when overlay off\n", dpufd->index);
		mdelay(50);  /* delay 50ms */
		return -1;
	}
#endif

	enable_cmdlist = g_enable_ovl_cmdlist_online;
	if ((dpufd->index == EXTERNAL_PANEL_IDX) && dpufd->panel_info.fake_external)
		enable_cmdlist = 0;

	ret = hisi_dss_module_init(dpufd);
	dpu_check_and_return((ret != 0), -1, ERR, "fb%d, fail to module_init! ret=%d\n", dpufd->index, ret);

	dpufd->resolution_rect.x = 0;
	dpufd->resolution_rect.y = 0;
	dpufd->resolution_rect.w = dpufd->panel_info.xres;
	dpufd->resolution_rect.h = dpufd->panel_info.yres;

	if (hisi_overlay_off_cmdlist_config(dpufd, enable_cmdlist) < 0)
		return -1;

	if (dpufd->panel_info.dirty_region_updt_support)
		hisi_dss_dirty_region_updt_config(dpufd, NULL);

	/* cpu config drm layer */
	hisi_drm_layer_online_config(dpufd, &(dpufd->ov_req_prev), NULL);

	ldi_data_gate(dpufd, true);

	single_frame_update(dpufd);
	dpufb_frame_updated(dpufd);

	if (!hisi_dss_check_reg_reload_status(dpufd))
		mdelay(20);  /* delay 20ms */

	ldi_data_gate(dpufd, false);

	if (is_mipi_cmd_panel(dpufd))
		dpufd->ldi_data_gate_en = 1;

	dpufd->frame_count++;

	return 0;
}

int hisi_overlay_off(struct dpu_fb_data_type *dpufd)
{
	int ret;

	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");

	if (dpufd->index == PRIMARY_PANEL_IDX)  /*lint -e548*/
		hisi_dss_dpm_deinit(dpufd);

	DPU_FB_DEBUG("fb%d, +\n", dpufd->index);
	if ((dpufd->index == PRIMARY_PANEL_IDX) || (dpufd->index == EXTERNAL_PANEL_IDX)) {
		dpufb_activate_vsync(dpufd);

		ret = hisi_vactive0_start_config(dpufd, &(dpufd->ov_req_prev));
		if (ret != 0) {
			DPU_FB_ERR("fb%d, hisi_vactive0_start_config failed! ret = %d\n", dpufd->index, ret);
			goto err_out;
		}

		hisi_overlay_off_update_frame(dpufd);
err_out:
		dpufb_deactivate_vsync(dpufd);
	} else if ((dpufd->index == AUXILIARY_PANEL_IDX) || (dpufd->index == MEDIACOMMON_PANEL_IDX)) {
		;  /* do nothing */
	} else {
		DPU_FB_ERR("fb%d, not support!\n", dpufd->index);
		return -EINVAL;
	}

	/* dss off */
	if (dpufd->index == AUXILIARY_PANEL_IDX)
		dpufb_dss_off(dpufd, true);
	else
		dpufb_dss_off(dpufd, false);

	hisi_overlay_reset_param(dpufd);

	DPU_FB_DEBUG("fb%d, -\n", dpufd->index);

	return 0;
}


int hisi_overlay_on_lp(struct dpu_fb_data_type *dpufd)
{
	int mctl_idx;
	int enable_cmdlist;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	enable_cmdlist = g_enable_ovl_cmdlist_online;
	if ((dpufd->index == EXTERNAL_PANEL_IDX) && dpufd->panel_info.fake_external)
		enable_cmdlist = 0;

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		mctl_idx = DSS_MCTL0;
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		mctl_idx = DSS_MCTL1;
	} else {
		DPU_FB_ERR("fb%d, not supported!\n", dpufd->index);
		return -1;
	}

	/* dss on */
	dpufb_dss_on(dpufd, enable_cmdlist);

	hisi_dss_mctl_on(dpufd, mctl_idx, enable_cmdlist, 0);


	return 0;
}

int hisi_overlay_off_lp(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	if ((dpufd->index != PRIMARY_PANEL_IDX) && (dpufd->index != EXTERNAL_PANEL_IDX)) {
		DPU_FB_ERR("fb%d, not supported!\n", dpufd->index);
		return -1;
	}

	dpufb_dss_off(dpufd, true);

	return 0;
}

static void hisi_overlay_init_param_init(struct dpu_fb_data_type *dpufd)
{
	dpufd->dss_module_resource_initialized = false;

	dpufd->vactive0_start_flag = 0;
	dpufd->vactive0_end_flag = 0;
	dpufd->ldi_data_gate_en = 0;
	init_waitqueue_head(&dpufd->vactive0_start_wq);
	dpufd->crc_flag = 0;

	dpufd->frame_update_flag = 0;

	dpufd->online_play_count = 0;
	dpufd->masklayer_maxbacklight_flag = false;

	memset(&dpufd->ov_req, 0, sizeof(dss_overlay_t));
	dpufd->ov_req.release_fence = -1;
	dpufd->ov_req.retire_fence = -1;
	memset(&dpufd->ov_req_prev, 0, sizeof(dss_overlay_t));
	dpufd->ov_req_prev.release_fence = -1;
	dpufd->ov_req_prev.retire_fence = -1;
	memset(&dpufd->dss_module, 0, sizeof(dss_module_reg_t));
	memset(&dpufd->dss_module_default, 0, sizeof(dss_module_reg_t));

	dpufd->dirty_region_updt.x = 0;
	dpufd->dirty_region_updt.y = 0;
	dpufd->dirty_region_updt.w = dpufd->panel_info.xres;
	dpufd->dirty_region_updt.h = dpufd->panel_info.yres;

	dpufd->resolution_rect.x = 0;
	dpufd->resolution_rect.y = 0;
	dpufd->resolution_rect.w = dpufd->panel_info.xres;
	dpufd->resolution_rect.h = dpufd->panel_info.yres;

	dpufd->pan_display_fnc = hisi_overlay_pan_display;
	dpufd->ov_ioctl_handler = hisi_overlay_ioctl_handler;

	dpufd->dss_debug_wq = NULL;
	dpufd->ldi_underflow_wq = NULL;
	dpufd->rch2_ce_end_wq = NULL;
	dpufd->rch4_ce_end_wq = NULL;
	dpufd->dpp_ce_end_wq = NULL;
	dpufd->masklayer_backlight_notify_wq = NULL;
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || defined(CONFIG_HISI_FB_V320)
	dpufd->hiace_end_wq = NULL;
#endif
	dpufd->delayed_cmd_queue_wq = NULL;
}

static int hisi_overlay_init_common(struct dpu_fb_data_type *dpufd)
{
	char wq_name[WORKQUEUE_NAME_SIZE] = {0};

	if ((dpufd->index == PRIMARY_PANEL_IDX) ||
		(dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external)) {
		snprintf(wq_name, WORKQUEUE_NAME_SIZE, "fb%d_dss_debug", dpufd->index);
		dpufd->dss_debug_wq = create_singlethread_workqueue(wq_name);
		dpu_check_and_return((dpufd->dss_debug_wq == NULL), -EINVAL, ERR,
			"fb%d, create dss debug workqueue failed!\n", dpufd->index);
		INIT_WORK(&dpufd->dss_debug_work, hisi_dss_debug_func);

		snprintf(wq_name, WORKQUEUE_NAME_SIZE, "fb%d_ldi_underflow", dpufd->index);
		dpufd->ldi_underflow_wq = create_singlethread_workqueue(wq_name);
		dpu_check_and_return((dpufd->ldi_underflow_wq == NULL), -EINVAL, ERR,
			"fb%d, create ldi underflow workqueue failed!\n", dpufd->index);
		INIT_WORK(&dpufd->ldi_underflow_work, hisi_ldi_underflow_handle_func);


#if defined(CONFIG_EFFECT_HIACE)
		if (dpufd->panel_info.hiace_support) {
			snprintf(wq_name, WORKQUEUE_NAME_SIZE, "fb%d_hiace_end", dpufd->index);
			dpufd->hiace_end_wq = create_singlethread_workqueue(wq_name);
			dpu_check_and_return((dpufd->hiace_end_wq == NULL), -EINVAL, ERR,
				"fb%d, create hiace end workqueue failed!\n", dpufd->index);
			INIT_WORK(&dpufd->hiace_end_work, hisi_dpp_hiace_end_handle_func);
		}
#endif
		if (dpufd->panel_info.delayed_cmd_queue_support) {
			snprintf(wq_name, WORKQUEUE_NAME_SIZE, "fb%d_delayed_cmd_queue", dpufd->index);
			dpufd->delayed_cmd_queue_wq = create_singlethread_workqueue(wq_name);
			dpu_check_and_return((dpufd->delayed_cmd_queue_wq == NULL), -EINVAL, ERR,
				"fb%d, create delayed cmd queue workqueue failed!\n", dpufd->index);
			INIT_WORK(&dpufd->delayed_cmd_queue_work, mipi_dsi_delayed_cmd_queue_handle_func);
			DPU_FB_INFO("Init delayed_cmd_queue_wq!\n");
		}
	}

	return 0;
}

static int hisi_overlay_init_primary_panel(struct dpu_fb_data_type *dpufd)
{
#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_970)
	char wq_name[WORKQUEUE_NAME_SIZE] = {0};
#endif

	dpufd->set_reg = hisi_cmdlist_set_reg;
	dpufd->ov_online_play = hisi_ov_online_play;
	dpufd->ov_offline_play = NULL;
	dpufd->ov_media_common_play = NULL;
	dpufd->ov_wb_isr_handler = NULL;
	dpufd->ov_vactive0_start_isr_handler = hisi_vactive0_start_isr_handler;

	dpufd->crc_isr_handler = NULL;

	hisi_effect_init(dpufd);

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_970)
	/* for gmp lut set */
	dpufd->gmp_lut_wq = NULL;
	if (dpufd->panel_info.gmp_support) {
		snprintf(wq_name, WORKQUEUE_NAME_SIZE, "fb0_gmp_lut");
		dpufd->gmp_lut_wq = create_singlethread_workqueue(wq_name);
		dpu_check_and_return((dpufd->gmp_lut_wq == NULL), -EINVAL, ERR, "create gmp lut workqueue failed!\n");
		INIT_WORK(&dpufd->gmp_lut_work, dpufb_effect_gmp_lut_workqueue_handler);
	}
#endif

	/* Init masklayer backlight notify workqueue for UDfingerprint. */
	hisi_init_masklayer_backlight_notify_wq(dpufd);

	return 0;
}

static void hisi_overlay_init_external_panel(struct dpu_fb_data_type *dpufd)
{
	dpufd->set_reg = dpufb_set_reg;
	dpufd->ov_online_play = hisi_ov_online_play;
	dpufd->ov_offline_play = NULL;
	dpufd->ov_media_common_play = NULL;
	dpufd->ov_wb_isr_handler = NULL;
	dpufd->ov_vactive0_start_isr_handler = hisi_vactive0_start_isr_handler;

	dpufd->crc_isr_handler = NULL;
}

static void hisi_overlay_init_auxiliary_panel(struct dpu_fb_data_type *dpufd)
{
	dpufd->set_reg = hisi_cmdlist_set_reg;
	dpufd->ov_online_play = NULL;
	dpufd->ov_offline_play = hisi_ov_offline_play;
	dpufd->ov_media_common_play = NULL;
	dpufd->ov_wb_isr_handler = NULL;
	dpufd->ov_vactive0_start_isr_handler = NULL;

	dpufd->crc_isr_handler = NULL;

	if (!hisi_mdc_resource_init(dpufd, g_dss_version_tag))
		DPU_FB_INFO("mdc channel manager init success!\n");
}

#ifdef CONFIG_HISI_FB_MEDIACOMMON
static void hisi_overlay_init_mediacommon_panel(struct dpu_fb_data_type *dpufd)
{
	dpufd->set_reg = hisi_cmdlist_set_reg;
	dpufd->ov_online_play = NULL;
	dpufd->ov_offline_play = NULL;
	dpufd->ov_media_common_play = hisi_ov_media_common_play;
	dpufd->ov_wb_isr_handler = NULL;
	dpufd->ov_vactive0_start_isr_handler = NULL;
	dpufd->crc_isr_handler = NULL;
}
#endif

int hisi_overlay_init(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return((dpufd->dss_base == NULL), -EINVAL, ERR, "dpufd->dss_base\n");

	hisi_overlay_init_param_init(dpufd);

	if (hisi_overlay_init_common(dpufd) < 0) {
		DPU_FB_ERR("hisi_overlay_init_common failed\n");
		return -EINVAL;
	}

	if (dpufd->index == PRIMARY_PANEL_IDX) {
		if (hisi_overlay_init_primary_panel(dpufd) < 0) {
			DPU_FB_ERR("hisi_overlay_init_primary_panel failed\n");
			return -EINVAL;
		}
	} else if (dpufd->index == EXTERNAL_PANEL_IDX) {
		hisi_overlay_init_external_panel(dpufd);
	} else if (dpufd->index == AUXILIARY_PANEL_IDX) {
		hisi_overlay_init_auxiliary_panel(dpufd);
#ifdef CONFIG_HISI_FB_MEDIACOMMON
	} else if (dpufd->index == MEDIACOMMON_PANEL_IDX) {
		hisi_overlay_init_mediacommon_panel(dpufd);
#endif
	} else {
		DPU_FB_ERR("fb%d not support this device!\n", dpufd->index);
		return -EINVAL;
	}

	if (!(dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external)) {
		if (hisi_cmdlist_init(dpufd)) {
			DPU_FB_ERR("fb%d hisi_cmdlist_init failed!\n", dpufd->index);
			return -EINVAL;
		}
	}

	hisi_dss_mmbuf_init(dpufd);

	return 0;
}

int hisi_overlay_deinit(struct dpu_fb_data_type *dpufd)
{
	dpu_check_and_return((dpufd == NULL), -EINVAL, ERR, "dpufd is NULL\n");

	if (dpufd->index == PRIMARY_PANEL_IDX)
		hisi_effect_deinit(dpufd);

	hisi_dss_destroy_wq(dpufd->rch4_ce_end_wq);
	hisi_dss_destroy_wq(dpufd->rch2_ce_end_wq);
	hisi_dss_destroy_wq(dpufd->dpp_ce_end_wq);

#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501) || \
	defined(CONFIG_HISI_FB_V330) || defined(CONFIG_HISI_FB_V320)
	hisi_dss_destroy_wq(dpufd->hiace_end_wq);
#endif
	hisi_dss_destroy_wq(dpufd->dss_debug_wq);
	hisi_dss_destroy_wq(dpufd->ldi_underflow_wq);
	hisi_dss_destroy_wq(dpufd->delayed_cmd_queue_wq);

	if (!(dpufd->index == EXTERNAL_PANEL_IDX && !dpufd->panel_info.fake_external))
		hisi_cmdlist_deinit(dpufd);

	hisi_dss_mmbuf_deinit(dpufd);

#if !defined(CONFIG_HISI_FB_V320) && !defined(CONFIG_HISI_FB_970)
	hisi_dss_destroy_wq(dpufd->gmp_lut_wq);
#endif


	return 0;
}

static int dpufb_get_lcd_id(struct dpu_fb_data_type *dpufd)
{
	int ret = 0;
	struct dpu_fb_panel_data *pdata = NULL;

	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return -EINVAL;
	}
	pdata = dev_get_platdata(&dpufd->pdev->dev);
	if (pdata == NULL) {
		DPU_FB_ERR("pdata is NULL\n");
		return -EINVAL;
	}

	if (pdata->get_lcd_id != NULL)
		ret = pdata->get_lcd_id(dpufd->pdev);

	return ret;
}

static int hisi_vactive0_wait_event(struct dpu_fb_data_type *dpufd,
	uint32_t condition_type, uint32_t *time_diff)
{
	int ret;
	struct timeval tv0;
	struct timeval tv1;
	int times = 0;
	uint32_t timeout_interval =
		(g_fpga_flag == 0) ? DSS_COMPOSER_TIMEOUT_THRESHOLD_ASIC : DSS_COMPOSER_TIMEOUT_THRESHOLD_FPGA;
	uint32_t prev_vactive0_start = dpufd->vactive0_start_flag;

	dpufb_get_timestamp(&tv0);
	while (times < 50) {  /* wait times */
		if (condition_type)
			ret = wait_event_interruptible_timeout(dpufd->vactive0_start_wq, dpufd->vactive0_start_flag,
				msecs_to_jiffies(timeout_interval));
		else
			ret = wait_event_interruptible_timeout(dpufd->vactive0_start_wq,
				(prev_vactive0_start != dpufd->vactive0_start_flag),
				msecs_to_jiffies(timeout_interval));

		if (ret != -ERESTARTSYS)
			break;

		times++;
		mdelay(10);  /* delay 10ms */
	}

	dpufb_get_timestamp(&tv1);
	*time_diff = dpufb_timestamp_diff(&tv0, &tv1);

	return ret;
}

static void hisi_vactive0_dsm_report(struct dpu_fb_data_type *dpufd, struct dsm_data *data,
	enum vactive0_panel_type report_type)
{
#if defined(CONFIG_HUAWEI_DSM)
	uint32_t index = data->dpufd->index;
	uint32_t vactive0_start_flag = data->dpufd->vactive0_start_flag;
	uint32_t dump_frame_no =
		(report_type == PANEL_VIDEO) ? data->dpufd->ov_req_prev.frame_no :
			data->dpufd->ov_req_prev_prev.frame_no;
	uint32_t frame_no = data->pov_req->frame_no;
	struct cmdlist_idxs_sq cmd_idxs = data->cmd_idx;
	int lcd_id;

	if (g_dsm_vactive0_timeout_count < VACTIVE0_TIMEOUT_EXPIRE_COUNT) {
		g_dsm_vactive0_timeout_count++;
		return;
	}

	g_dsm_vactive0_timeout_count = 0;
	if (!lcd_dclient || dsm_client_ocuppy(lcd_dclient))
		return;

	if (report_type == PANEL_CMD_RESET) {
		if (g_fake_lcd_flag || data->dpufd->lcd_self_testing)
			return;

		lcd_id = dpufb_get_lcd_id(dpufd);
			dsm_client_record(lcd_dclient, "fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
				"vactive0_start_flag=%d, pre_pre_frame_no=0x%x, frame_no=0x%x, TIMESTAMP_DIFF is 0x%x, "
				"cmdlist_idxs_prev=0x%x, cmdlist_idxs_prev_prev=0x%x, cmdlist_idxs=0x%x!, "
				"Number of the Errors on DSI : 0x05 = 0x%x, Display Power Mode : 0x0A = 0x%x, "
				"Display Signal Mode : 0x0E = 0x%x, Display Self-Diagnostic Result : 0x0F = 0x%x, lcd_id = 0x%x\n",
				index, data->ret, vactive0_start_flag, dump_frame_no, frame_no,
				data->time_diff, cmd_idxs.prev, cmd_idxs.prev_prev, cmd_idxs.cur,
				data->read_value[0], data->read_value[1], data->read_value[2], data->read_value[3],
				lcd_id);
			dsm_client_notify(lcd_dclient, DSM_LCD_TE_TIME_OUT_ERROR_NO);
	} else if (report_type == PANEL_CMD) {
		if (g_fake_lcd_flag || data->dpufd->lcd_self_testing)
			return;

		lcd_id = dpufb_get_lcd_id(data->dpufd);
		dsm_client_record(lcd_dclient, "fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
			"vactive0_start_flag=%d, pre_pre_frame_no=0x%x, frame_no=0x%x, TIMESTAMP_DIFF is 0x%x, "
			"cmdlist_idxs_prev=0x%x, cmdlist_idxs_prev_prev=0x%x, cmdlist_idxs=0x%x, lcd_id = 0x%x\n",
			index, data->ret, vactive0_start_flag, dump_frame_no, frame_no,
			data->time_diff, cmd_idxs.prev, cmd_idxs.prev_prev, cmd_idxs.cur, lcd_id);
		dsm_client_notify(lcd_dclient, DSM_LCD_TE_TIME_OUT_ERROR_NO);
	} else { /* video */
		dsm_client_record(lcd_dclient, "fb%d, 1wait_for vactive0_start_flag timeout!ret = %d, "
			"vactive0_start_flag = %d, frame_no = %u, TIMESTAMP_DIFF is %u us, cmdlist_idxs = 0x%x!\n",
			index, data->ret, vactive0_start_flag, dump_frame_no, data->time_diff, cmd_idxs.cur);
		dsm_client_notify(lcd_dclient, DSM_LCD_VACTIVE_TIMEOUT_ERROR_NO);
	}
#endif
}

static void hisi_vactive0_dsm_reset(void)
{
#if defined(CONFIG_HUAWEI_DSM)
	g_dsm_vactive0_timeout_count = 0;
#endif
}

static void hisi_vactive0_print_ldi(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req_dump)
{
	uint32_t isr_s1 = 0;
	uint32_t isr_s2 = 0;
	uint32_t isr_s2_mask = 0;
	char __iomem *ldi_base = NULL;

	if (!pov_req_dump)
		return;

	if (pov_req_dump->ovl_idx == DSS_OVL0) {
		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_PDP_INTS);
		isr_s2_mask = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INT_MSK);
		isr_s2 = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS);
		ldi_base = dpufd->dss_base + DSS_LDI0_OFFSET;
		DPU_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, LDI_CTRL(0x%x), LDI_FRM_MSK(0x%x).\n",
			dpufd->index, isr_s1, isr_s2_mask, isr_s2,
			inp32(ldi_base + LDI_CTRL), inp32(ldi_base + LDI_FRM_MSK));
	} else if (pov_req_dump->ovl_idx == DSS_OVL1) {
		ldi_base = dpufd->dss_base + DSS_LDI1_OFFSET;

		isr_s1 = inp32(dpufd->dss_base + GLB_CPU_SDP_INTS);
#if defined(CONFIG_HISI_FB_970) || defined(CONFIG_HISI_FB_V501)
		isr_s2_mask = inp32(ldi_base + LDI_CPU_ITF_INT_MSK);
		isr_s2 = inp32(ldi_base + LDI_CPU_ITF_INTS);
#endif

		DPU_FB_ERR("fb%d, isr_s1=0x%x, isr_s2_mask=0x%x, isr_s2=0x%x, LDI_CTRL(0x%x), LDI_FRM_MSK(0x%x).\n",
			dpufd->index, isr_s1, isr_s2_mask, isr_s2,
			inp32(ldi_base + LDI_CTRL), inp32(ldi_base + LDI_FRM_MSK));
	}
}

/* return value */
static bool hisi_vactive0_check_esd(struct dpu_fb_data_type *dpufd)
{
	int ret = -ETIMEDOUT;

	g_vactive_timeout_count++;
	if ((g_vactive_timeout_count >= 1) && dpufd->panel_info.esd_enable) {
		dpufd->esd_recover_state = ESD_RECOVER_STATE_START;
		if (dpufd->esd_ctrl.esd_check_wq)
			queue_work(dpufd->esd_ctrl.esd_check_wq, &(dpufd->esd_ctrl.esd_check_work));

		ret = 0;
	}

	return ret;
}

static void hisi_vactive0_esd_reset(void)
{
	g_vactive_timeout_count = 0;
}

static void hisi_vactive0_get_cmdlist_indexs(struct dpu_fb_data_type *dpufd,
	struct cmdlist_idxs_sq *cmd_idxs)
{
	int ret1;

	ret1 = hisi_cmdlist_get_cmdlist_idxs(&(dpufd->ov_req_prev), &(cmd_idxs->prev), NULL);
	if (ret1 != 0)
		DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n",
			dpufd->index, ret1);

	ret1 = hisi_cmdlist_get_cmdlist_idxs(&(dpufd->ov_req_prev_prev), &(cmd_idxs->prev_prev), NULL);
	if (ret1 != 0)
		DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev_prev failed! ret = %d\n",
			dpufd->index, ret1);

	cmd_idxs->cur = cmd_idxs->prev | cmd_idxs->prev_prev;
}

static int hisi_vactive0_start_config_video(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int ret;
	uint32_t prev_vactive0_start;
	uint32_t time_diff = 0;
	struct cmdlist_idxs_sq cmd_idxs = {0};
	dss_overlay_t *pov_req_prev = &(dpufd->ov_req_prev);
	struct dsm_data dmd_data = {0};

	ldi_data_gate(dpufd, false);
	prev_vactive0_start = dpufd->vactive0_start_flag;
	ret = hisi_vactive0_wait_event(dpufd, VIDEO_MODE, &time_diff);

	if (g_fastboot_enable_flag && g_enable_ovl_cmdlist_online)
		set_reg(dpufd->dss_base + DSS_MCTRL_CTL0_OFFSET + MCTL_CTL_TOP, 0x1, 32, 0);

	if (ret > 0) { /* success */
		hisi_vactive0_dsm_reset();
		return 0;
	}
	/* error handle */
	ret = hisi_cmdlist_get_cmdlist_idxs(pov_req_prev, &cmd_idxs.cur, NULL);
	if (ret != 0)
		DPU_FB_INFO("fb%d, hisi_cmdlist_get_cmdlist_idxs pov_req_prev failed! ret = %d\n", dpufd->index, ret);

	DPU_FB_ERR("fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
		"vactive0_start_flag=%d, frame_no=%u, TIMESTAMP_DIFF is %u us,"
		"cmdlist_idxs=0x%x!\n",
		dpufd->index, ret, dpufd->vactive0_start_flag, pov_req_prev->frame_no,
		time_diff, cmd_idxs.cur);

	hisi_dump_current_info(dpufd);

	dmd_data.cmd_idx = cmd_idxs;
	dmd_data.dpufd = dpufd;
	dmd_data.pov_req = pov_req;
	dmd_data.ret = ret;
	dmd_data.time_diff = time_diff;
	hisi_vactive0_dsm_report(dpufd, &dmd_data, PANEL_VIDEO);

	hisi_dss_underflow_dump_cmdlist(dpufd, pov_req_prev, NULL);
	if (g_debug_ovl_online_composer_hold)
		mdelay(HISI_DSS_COMPOSER_HOLD_TIME);

	/* for blank display of video mode */
	mipi_dsi_reset(dpufd);

	return 0;
}

static bool hisi_vactive0_need_frame_update(struct dpu_fb_data_type *dpufd)
{
	dss_overlay_t *pov_req_prev = &(dpufd->ov_req_prev);

	if ((dpufd->vactive0_start_flag == 1) &&
		(dpufd->secure_ctrl.secure_event == DSS_SEC_ENABLE) &&
		(pov_req_prev->sec_enable_status == DSS_SEC_ENABLE)) {
		dpufd->vactive0_start_flag = 0;
		single_frame_update(dpufd);
	}
	if (dpufd->vactive0_start_flag == 0)
		return true;

	return false;
}

static int hisi_vactive0_tryagain(struct dpu_fb_data_type *dpufd)
{
	int ret;
	uint32_t read_value[4] = {0};  /* Mipi read back 4 register valve */
	uint32_t ldi_vstate;
	uint32_t time_diff = 0;
	dss_overlay_t *pov_req_prev_prev = &(dpufd->ov_req_prev_prev);

	ldi_data_gate(dpufd, false);
	mipi_panel_check_reg(dpufd, read_value, ARRAY_SIZE(read_value));
	ldi_vstate = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_VSTATE);

	DPU_FB_ERR("fb%d, "
		"Number of the Errors on DSI : 0x05 = 0x%x\n"
		"Display Power Mode : 0x0A = 0x%x\n"
		"Display Signal Mode : 0x0E = 0x%x\n"
		"Display Self-Diagnostic Result : 0x0F = 0x%x\n"
		"LDI vstate : 0x%x, LDI dpi0_hstate : 0x%x\n",
		dpufd->index, read_value[0], read_value[1], read_value[2], read_value[3],
		ldi_vstate, inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_DPI0_HSTATE));

	ret = hisi_vactive0_wait_event(dpufd, CMD_MODE, &time_diff);
	if (ret <= 0) {
		DPU_FB_ERR("fb%d, 2wait_for vactive0_start_flag timeout!ret=%d, "
			"vactive0_start_flag=%d, frame_no=%u.\n",
			dpufd->index, ret, dpufd->vactive0_start_flag, pov_req_prev_prev->frame_no);

		ldi_data_gate(dpufd, false);
		ret = -ETIMEDOUT;
		if (ldi_vstate == LDI_VSTATE_V_WAIT_TE0)
			ret = hisi_vactive0_check_esd(dpufd);

	} else {
		ldi_data_gate(dpufd, true);
		ret = 0;
	}

	return ret;
}

static void hisi_vactive0_reset_config(struct dpu_fb_data_type *dpufd, struct cmdlist_idxs_sq cmd_idxs)
{
	hisi_cmdlist_config_reset(dpufd, &(dpufd->ov_req_prev_prev), cmd_idxs.cur);
	ldi_data_gate(dpufd, false);
	mdelay(10);  /* delay 10ms */

	memset(dpufd->ov_block_infos_prev, 0, HISI_DSS_OV_BLOCK_NUMS * sizeof(dss_overlay_block_t));
	dpufb_dss_overlay_info_init(&(dpufd->ov_req_prev));
}

static int hisi_vactive0_reset(struct dpu_fb_data_type *dpufd, uint32_t read_value[], int num_read_value,
	struct dsm_data *dmd_data, struct cmdlist_idxs_sq cmd_idxs)
{
	bool panel_check = true;
	uint32_t phy_status;
	uint32_t ldi_vstate;

	if (num_read_value < 4)  /* Mipi read back 4 register valve */
		DPU_FB_INFO("number read value overflow\n");

	panel_check = mipi_panel_check_reg(dpufd, read_value, num_read_value);
	phy_status = inp32(dpufd->mipi_dsi0_base + MIPIDSI_PHY_STATUS_OFFSET);
	ldi_vstate = inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_VSTATE);
	DPU_FB_ERR("panel_check = %d, phy_status = 0x%x, LDI0_VSTATE = 0x%x.\n", panel_check, phy_status, ldi_vstate);

	if (g_debug_ldi_underflow_clear && g_ldi_data_gate_en) {
		hisi_vactive0_reset_config(dpufd, cmd_idxs);

		DPU_FB_ERR("fb%d, Number of the Errors on DSI : 0x05 = 0x%x\n"
			"Display Power Mode : 0x0A = 0x%x\n Display Signal Mode : 0x0E = 0x%x\n"
			"Display Self-Diagnostic Result : 0x0F = 0x%x\n LDI vstate : 0x%x, LDI dpi0_hstate : 0x%x\n",
			dpufd->index, read_value[0], read_value[1], read_value[2], read_value[3],
			ldi_vstate, inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_DPI0_HSTATE));

		if ((ldi_vstate == LDI_VSTATE_V_WAIT_TE0) || (!panel_check && (phy_status & BIT(1))))
			hisi_vactive0_check_esd(dpufd);

		hisi_vactive0_dsm_report(dpufd, dmd_data, PANEL_CMD_RESET);
		return 0;
	}

	return 1;
}

static int hisi_vactive0_start_config_cmd(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int ret = 0;
	uint32_t time_diff = 0;
	struct cmdlist_idxs_sq cmd_idxs = {0};
	uint32_t read_value[4] = {0};  /* Mipi read back 4 register valve */
	struct dsm_data dmd_data = {0};

	if (!hisi_vactive0_need_frame_update(dpufd))
		goto END;

	ret = hisi_vactive0_wait_event(dpufd, CMD_MODE, &time_diff);
	if (ret > 0) { /* success */
		ldi_data_gate(dpufd, true);
		hisi_vactive0_dsm_reset();
		ret = 0;
		goto END;
	}

	/* error handle */
	hisi_vactive0_get_cmdlist_indexs(dpufd, &cmd_idxs);

	DPU_FB_ERR("fb%d, 1wait_for vactive0_start_flag timeout!ret=%d, "
		"vactive0_start_flag=%d, pre_pre_frame_no=%u, frame_no=%u, TIMESTAMP_DIFF is %u us, "
		"cmdlist_idxs_prev=0x%x, cmdlist_idxs_prev_prev=0x%x, cmdlist_idxs=0x%x, itf0_ints=0x%x\n",
		dpufd->index, ret, dpufd->vactive0_start_flag, dpufd->ov_req_prev_prev.frame_no, pov_req->frame_no,
		time_diff, cmd_idxs.prev, cmd_idxs.prev_prev, cmd_idxs.cur,
		inp32(dpufd->dss_base + DSS_LDI0_OFFSET + LDI_CPU_ITF_INTS));

	hisi_dump_current_info(dpufd);

	hisi_dss_underflow_dump_cmdlist(dpufd, &(dpufd->ov_req_prev), &(dpufd->ov_req_prev_prev));
	if (g_debug_ovl_online_composer_hold)
		mdelay(HISI_DSS_COMPOSER_HOLD_TIME);

	dmd_data.cmd_idx = cmd_idxs;
	dmd_data.dpufd = dpufd;
	dmd_data.pov_req = pov_req;
	dmd_data.read_value = read_value;
	dmd_data.ret = ret;
	dmd_data.time_diff = time_diff;

	ret = hisi_vactive0_reset(dpufd, read_value, ARRAY_SIZE(read_value), &dmd_data, cmd_idxs);
	if (ret == 0)
		return 0;

	hisi_vactive0_dsm_report(dpufd, &dmd_data, PANEL_CMD);
	ret = hisi_vactive0_tryagain(dpufd);

END:
	ldi_data_gate(dpufd, true);
	dpufd->vactive0_start_flag = 0;
	dpufd->vactive0_end_flag = 0;
	if (ret >= 0)
		hisi_vactive0_esd_reset();

	return ret;
}

int hisi_vactive0_start_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	int ret;
	dss_overlay_t *pov_req_dump = NULL;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL\n");

	if (is_mipi_cmd_panel(dpufd) && (dpufd->frame_update_flag == 0)) {
		pov_req_dump = &(dpufd->ov_req_prev_prev);
		ret = hisi_vactive0_start_config_cmd(dpufd, pov_req);
	} else { /* video mode */
		pov_req_dump = &(dpufd->ov_req_prev);
		ret = hisi_vactive0_start_config_video(dpufd, pov_req);
	}

	if (ret == -ETIMEDOUT)
		hisi_vactive0_print_ldi(dpufd, pov_req_dump);

	return ret;
}
