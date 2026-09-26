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
#include "hisi_mmbuf_manager.h"

static void aif_bw_sort(dss_aif_bw_t a[], int n)
{
	int i;
	int j;
	dss_aif_bw_t tmp;

	for (i = 0; i < n; ++i) {
		for (j = i; j < n - 1; ++j) {
			if (a[j].bw > a[j + 1].bw) {
				tmp = a[j];
				a[j] = a[j + 1];
				a[j + 1] = tmp;
			}
		}
	}
}

static void hisi_dss_aif_cfg_for_wblayer(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block, dss_aif_bw_t aif_bw_tmp[], uint32_t aif_bw_size)
{
	uint32_t i;
	uint32_t k;
	dss_layer_t *layer = NULL;
	dss_wb_layer_t *wb_layer = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	int chn_idx;

	if ((pov_req->wb_layer_nums <= 0) || (pov_req->wb_layer_nums > MAX_DSS_DST_NUM)) {
		DPU_FB_ERR("wb_layer_nums=%d out of range\n", pov_req->wb_layer_nums);
		return;
	}

	for (k = 0; k < pov_req->wb_layer_nums; k++) {
		wb_layer = &(pov_req->wb_layer_infos[k]);
		chn_idx = wb_layer->chn_idx;

		aif_bw = &(dpufd->dss_module.aif_bw[chn_idx]);
		aif_bw->bw = (uint64_t)wb_layer->dst.buf_size *
			(wb_layer->src_rect.w * wb_layer->src_rect.h) /
			(wb_layer->dst.width * wb_layer->dst.height);
		aif_bw->chn_idx = chn_idx;
		if (pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)
			aif_bw->axi_sel = DSS_OFFLINE_AXI0;
		else
			aif_bw->axi_sel = DSS_OFFLINE_AXI1;

		/* wch only support SRT_AXI0 for dssv600 */
		aif_bw->is_used = 1;
	}

	if ((pov_req->wb_compose_type == DSS_WB_COMPOSE_COPYBIT) ||
		(pov_req->wb_compose_type == DSS_WB_COMPOSE_MEDIACOMMON)) {
		for (i = 0; i < pov_h_block->layer_nums; i++) {
			layer = &pov_h_block->layer_infos[i];
			chn_idx = layer->chn_idx;

			if (i >= aif_bw_size) {
				DPU_FB_ERR("aif_bw_tmp overflow\n");
				return;
			}

			aif_bw_tmp[i].chn_idx = chn_idx;
			aif_bw_tmp[i].axi_sel = DSS_OFFLINE_AXI0;
			aif_bw_tmp[i].is_used = 1;
			dpufd->dss_module.aif_bw[chn_idx] = aif_bw_tmp[i];
		}
	}
}

static void hisi_dss_aif_ch_ov_cfg(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_aif_bw_t aif_bw_tmp[], uint32_t bw_sum)
{
	int i;
	uint32_t tmp = 0;
	int axi0_cnt = 0;
	int axi1_cnt = 0;
	int chn_idx;

	/* i is not chn_idx, is array idx */
	for (i = 0; i < DSS_CHN_MAX_DEFINE; i++) {
		if (aif_bw_tmp[i].is_used != 1)
			continue;

		tmp += aif_bw_tmp[i].bw;

		if ((pov_req->ovl_idx == DSS_OVL0) || (pov_req->ovl_idx == DSS_OVL1)) {
			if (tmp <= (bw_sum / 2)) {  /* dichotomy */
				aif_bw_tmp[i].axi_sel = DSS_ONLINE_AXI0;
				if (axi0_cnt >= AXI0_MAX_DSS_CHN_THRESHOLD) {
					aif_bw_tmp[i - AXI0_MAX_DSS_CHN_THRESHOLD].axi_sel = DSS_ONLINE_AXI1; /*lint !e676*/
					axi1_cnt++;
					axi0_cnt--;
				}
				axi0_cnt++;
			} else {
				aif_bw_tmp[i].axi_sel = DSS_ONLINE_AXI1;
				axi1_cnt++;
			}
		} else {
			if (tmp <= (bw_sum / 2)) {  /* dichotomy */
				aif_bw_tmp[i].axi_sel = DSS_OFFLINE_AXI1;
				if (axi1_cnt >= AXI1_MAX_DSS_CHN_THRESHOLD) {
					aif_bw_tmp[i - AXI0_MAX_DSS_CHN_THRESHOLD].axi_sel = DSS_OFFLINE_AXI0; /*lint !e676*/
					axi0_cnt++;
					axi1_cnt--;
				}
				axi1_cnt++;
			} else {
				aif_bw_tmp[i].axi_sel = DSS_OFFLINE_AXI0;
				axi0_cnt++;
			}
		}

		chn_idx = aif_bw_tmp[i].chn_idx;
		dpufd->dss_module.aif_bw[chn_idx] = aif_bw_tmp[i];

		if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
			DPU_FB_INFO("fb%d, aif0, chn_idx=%d, axi_sel=%d, bw=%llu.\n",
				dpufd->index, chn_idx, aif_bw_tmp[i].axi_sel, aif_bw_tmp[i].bw);
	}

}

static bool is_aif_bcd_layer(dss_layer_t *layer)
{
	return (layer && ((layer->need_cap & CAP_AFBCD) ||
		(layer->need_cap & CAP_HFBCD) || (layer->need_cap & CAP_HEBCD)));
}

static int8_t get_aif1_axi_sel(int32_t ovl_idx, uint32_t use_mmbuf_cnt)
{
	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1))
		return ((use_mmbuf_cnt % 2) ? AXI_CHN0 : AXI_CHN1);
	else
		return ((use_mmbuf_cnt % 2) ? AXI_CHN1 : AXI_CHN0);
}

static int8_t get_aif_axi_sel(int32_t ovl_idx, uint32_t use_axi_cnt)
{
	if ((ovl_idx == DSS_OVL0) || (ovl_idx == DSS_OVL1))
		return ((use_axi_cnt % 2) ? DSS_ONLINE_AXI0 : DSS_ONLINE_AXI1);
	else
		return ((use_axi_cnt % 2) ? DSS_OFFLINE_AXI1 : DSS_OFFLINE_AXI0);
}

static void hisi_set_ch_aif_sel(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block, uint32_t aif_bw_size)
{
	uint32_t i;
	dss_layer_t *layer = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	dss_aif_bw_t *aif1_bw = NULL;
	int chn_idx = 0;

	/* i is not chn_idx, is array idx */
	for (i = 0; i < pov_h_block->layer_nums; i++) {
		layer = &pov_h_block->layer_infos[i];
		chn_idx = layer->chn_idx;

		if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
			continue;

		if ((layer->img.width == 0) || (layer->img.height == 0))
			continue;

		if (layer->dst_rect.y < pov_h_block->ov_block_rect.y)
			continue;

		/* MMBUF has only two aix chn (AXI0 & AXI1) */
		if (is_aif_bcd_layer(layer)) {
			dpufd->use_mmbuf_cnt++;
			aif1_bw = &(dpufd->dss_module.aif1_bw[chn_idx]);
			aif1_bw->is_used = 1;
			aif1_bw->chn_idx = chn_idx;
			aif1_bw->axi_sel = get_aif1_axi_sel(pov_req->ovl_idx, dpufd->use_mmbuf_cnt);

			if (g_debug_ovl_online_composer)
				DPU_FB_INFO("fb%d, aif1, chn_idx=%d, axi_sel=%d.\n", dpufd->index, chn_idx, aif1_bw->axi_sel);
		}

		if (i >= aif_bw_size) {
			DPU_FB_ERR("aif_bw_tmp overflow\n");
			return;
		}

		dpufd->use_axi_cnt++;
		aif_bw = &(dpufd->dss_module.aif_bw[chn_idx]);
		aif_bw->is_used = 1;
		aif_bw->chn_idx = chn_idx;
		aif_bw->axi_sel = get_aif_axi_sel(pov_req->ovl_idx, dpufd->use_axi_cnt);

		if (g_debug_ovl_online_composer)
			DPU_FB_INFO("fb%d, aif, chn_idx=%d, axi_sel=%d.\n", dpufd->index, chn_idx, aif_bw->axi_sel);
	}
}

static void hisi_dss_aif_ch_cfg(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_overlay_block_t *pov_h_block, dss_aif_bw_t aif_bw_tmp[], uint32_t aif_bw_size)
{
	uint32_t i;
	dss_layer_t *layer = NULL;
	dss_aif_bw_t *aif1_bw = NULL;
	int chn_idx = 0;
	uint32_t bw_sum = 0;
	int rch_cnt = 0;

	if (dpufd->panel_info.product_type & PANEL_SUPPORT_TWO_PANEL_DISPLAY_TYPE) {
		hisi_set_ch_aif_sel(dpufd, pov_req, pov_h_block, DSS_CHN_MAX_DEFINE);
		return;
	}

	/* i is not chn_idx, is array idx */
	for (i = 0; i < pov_h_block->layer_nums; i++) {
		layer = &pov_h_block->layer_infos[i];
		chn_idx = layer->chn_idx;

		if (layer->need_cap & (CAP_BASE | CAP_DIM | CAP_PURE_COLOR))
			continue;

		if ((layer->img.width == 0) || (layer->img.height == 0))
			continue;

		/* MMBUF has only two aix chn (AXI0 & AXI1) */
		if (is_aif_bcd_layer(layer)) {
			if (layer->dst_rect.y < pov_h_block->ov_block_rect.y)
				continue;

			dpufd->use_mmbuf_cnt++;
			aif1_bw = &(dpufd->dss_module.aif1_bw[chn_idx]);
			aif1_bw->is_used = 1;
			aif1_bw->chn_idx = chn_idx;
			aif1_bw->axi_sel = get_aif1_axi_sel(pov_req->ovl_idx, dpufd->use_mmbuf_cnt);

			if (g_debug_ovl_online_composer || g_debug_ovl_offline_composer)
				DPU_FB_INFO("fb%d, aif1, chn_idx=%d, axi_sel=%d.\n",
					dpufd->index, chn_idx, aif1_bw->axi_sel);
		}

		if (i >= aif_bw_size) {
			DPU_FB_ERR("aif_bw_tmp overflow\n");
			return;
		}
		if (layer->is_cld_layer == 1)
			aif_bw_tmp[i].bw = (uint64_t)layer->img.buf_size;
		else
			aif_bw_tmp[i].bw = (uint64_t)layer->img.buf_size *
				(layer->src_rect.w * layer->src_rect.h) / (layer->img.width * layer->img.height);
		aif_bw_tmp[i].chn_idx = chn_idx;
		aif_bw_tmp[i].axi_sel = AXI_CHN0;
		aif_bw_tmp[i].is_used = 1;

		bw_sum += aif_bw_tmp[i].bw;
		rch_cnt++;
	}

	/* sort */
	aif_bw_sort(aif_bw_tmp, rch_cnt);

	hisi_dss_aif_ch_ov_cfg(dpufd, pov_req, aif_bw_tmp, bw_sum);
}

int hisi_dss_aif_handler(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_overlay_block_t *pov_h_block)
{
	dss_aif_bw_t aif_bw_tmp[DSS_CHN_MAX_DEFINE];

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL!\n");
	dpu_check_and_return(!pov_h_block, -EINVAL, ERR, "pov_h_block is NULL!\n");

	memset(aif_bw_tmp, 0, sizeof(aif_bw_tmp));

	if (pov_req->wb_enable)
		hisi_dss_aif_cfg_for_wblayer(dpufd, pov_req, pov_h_block, aif_bw_tmp, DSS_CHN_MAX_DEFINE);

	hisi_dss_aif_ch_cfg(dpufd, pov_req, pov_h_block, aif_bw_tmp, DSS_CHN_MAX_DEFINE);

	return 0;
}

/*******************************************************************************
 * DSS AIF
 */
void hisi_dss_aif_init(const char __iomem *aif_ch_base,
	dss_aif_t *s_aif)
{
	dpu_check_and_no_retval(!aif_ch_base, ERR, "aif_ch_base is NULL\n");
	dpu_check_and_no_retval(!s_aif, ERR, "s_aif is NULL\n");

	memset(s_aif, 0, sizeof(dss_aif_t));

	s_aif->aif_ch_ctl = inp32(aif_ch_base + AIF_CH_CTL);
	s_aif->aif_ch_hs = inp32(aif_ch_base + AIF_CH_HS);
	s_aif->aif_ch_ls = inp32(aif_ch_base + AIF_CH_LS);
}

void hisi_dss_aif_ch_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *aif_ch_base, dss_aif_t *s_aif)
{
	dpu_check_and_no_retval(!dpufd, ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval(!aif_ch_base, ERR, "aif_ch_base is NULL\n");
	dpu_check_and_no_retval(!s_aif, ERR, "s_aif is NULL\n");


	dpufd->set_reg(dpufd, aif_ch_base + AIF_CH_CTL, s_aif->aif_ch_ctl, 32, 0);
	dpufd->set_reg(dpufd, aif_ch_base + AIF_CH_HS, s_aif->aif_ch_hs, 32, 0);
	dpufd->set_reg(dpufd, aif_ch_base + AIF_CH_LS, s_aif->aif_ch_ls, 32, 0);
}

static bool is_not_aif_bcd_layer(dss_layer_t *layer)
{
	return (layer && ((layer->need_cap & CAP_AFBCD) != CAP_AFBCD) &&
		((layer->need_cap & CAP_HFBCD) != CAP_HFBCD) &&
		((layer->need_cap & CAP_HEBCD) != CAP_HEBCD));
}

static bool is_not_aif_bce_wb_layer(dss_wb_layer_t *wb_layer)
{
	return (wb_layer && ((wb_layer->need_cap & CAP_AFBCE) != CAP_AFBCE) &&
		((wb_layer->need_cap & CAP_HFBCE) != CAP_HFBCE) &&
		((wb_layer->need_cap & CAP_HEBCE) != CAP_HEBCE));
}

int hisi_dss_aif_ch_config(struct dpu_fb_data_type *dpufd,
	dss_overlay_t *pov_req, dss_layer_t *layer, dss_rect_t *wb_dst_rect,
	dss_wb_layer_t *wb_layer)
{
	dss_aif_t *aif = NULL;
	dss_aif_bw_t *aif_bw = NULL;
	int chn_idx;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL Point!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL Point!\n");
	dpu_check_and_return((!layer && !wb_layer), -EINVAL, ERR, "layer & wb_layer is NULL Point!\n");

	dpu_check_and_return(((pov_req->ovl_idx < DSS_OVL0) || (pov_req->ovl_idx >= DSS_OVL_IDX_MAX)),
		-EINVAL, ERR, "ovl_idx %d is invalid!\n", pov_req->ovl_idx);

	if (wb_layer)
		chn_idx = wb_layer->chn_idx;
	else
		chn_idx = layer->chn_idx;


	aif = &(dpufd->dss_module.aif[chn_idx]);
	dpufd->dss_module.aif_ch_used[chn_idx] = 1;

	aif_bw = &(dpufd->dss_module.aif_bw[chn_idx]);
	dpu_check_and_return((aif_bw->is_used != 1), -EINVAL, ERR,
		"fb%d, aif_bw->is_used %d is invalid!\n", dpufd->index, aif_bw->is_used);

	aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, aif_bw->axi_sel, 2, 0);

	if (g_fpga_flag == 0) {
		if ((pov_req->ovl_idx == DSS_OVL2) || (pov_req->ovl_idx == DSS_OVL3)) {
			if (is_not_aif_bcd_layer(layer) || is_not_aif_bce_wb_layer(wb_layer)) {
				aif->aif_ch_ctl = set_bits32(aif->aif_ch_ctl, 0x1, 3, 8);
				aif->aif_ch_ls = set_bits32(aif->aif_ch_ls, DSS_AIF0_CH_LS, 32, 0);
			}
		}
	}

	return 0;
}

static bool is_aif1_bce_wb_layer(dss_wb_layer_t *wb_layer)
{
	return (wb_layer && ((wb_layer->need_cap & CAP_AFBCE) ||
		(wb_layer->need_cap & CAP_HFBCE) || (wb_layer->need_cap & CAP_HEBCE)));
}

int hisi_dss_aif1_ch_config(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req,
	dss_layer_t *layer, dss_wb_layer_t *wb_layer, int ovl_idx)
{
	dss_aif_t *aif1 = NULL;
	dss_aif_bw_t *aif1_bw = NULL;
	int chn_idx;
	uint32_t need_cap;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is NULL!\n");
	dpu_check_and_return(!pov_req, -EINVAL, ERR, "pov_req is NULL!\n");
	dpu_check_and_return(!layer && !wb_layer, -EINVAL, ERR, "layer & wb_layer is NULL!\n");

	dpu_check_and_return(((ovl_idx < DSS_OVL0) || (ovl_idx >= DSS_OVL_IDX_MAX)), -EINVAL, ERR,
		"ovl_idx %d is invalid!\n\n", ovl_idx);

	if (wb_layer) {
		chn_idx = wb_layer->chn_idx;
		need_cap = wb_layer->need_cap;
	} else {
		chn_idx = layer->chn_idx;
		need_cap = layer->need_cap;
	}

	if (!(need_cap & (CAP_AFBCD | CAP_HFBCD | CAP_HEBCD)))
		return 0;


	aif1_bw = &(dpufd->dss_module.aif1_bw[chn_idx]);
	if (aif1_bw->is_used != 1) {
		DPU_FB_ERR("fb%d, aif1_bw->is_used=%d no equal to 1 is err!\n", dpufd->index, aif1_bw->is_used);
		return 0;
	}
	aif1 = &(dpufd->dss_module.aif1[chn_idx]);
	dpufd->dss_module.aif1_ch_used[chn_idx] = 1;

	aif1->aif_ch_ctl = set_bits32(aif1->aif_ch_ctl, aif1_bw->axi_sel, 2, 0);

	if (g_fpga_flag == 0) {
		if ((ovl_idx == DSS_OVL2) || (ovl_idx == DSS_OVL3)) {
			if (is_aif_bcd_layer(layer) || is_aif1_bce_wb_layer(wb_layer)) {
				aif1->aif_ch_ctl = set_bits32(aif1->aif_ch_ctl, 0x1, 3, 8);
				aif1->aif_ch_ls = set_bits32(aif1->aif_ch_ls, DSS_AIF1_CH_LS, 32, 0);
			}
		}
	}

	return 0;
}

