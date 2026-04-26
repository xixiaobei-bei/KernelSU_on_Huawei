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

struct dss_dump_data_type *dpufb_alloc_dumpdss(void)
{
	struct dss_dump_data_type *dumpdss = NULL;

	dumpdss = kmalloc(sizeof(*dumpdss), GFP_KERNEL);
	if (dumpdss == NULL) {
		DPU_FB_ERR("alloc dumpDss failed!\n");
		return NULL;
	}
	memset(dumpdss, 0, sizeof(*dumpdss));

	dumpdss->dss_buf_len = 0;
	dumpdss->dss_buf = kmalloc(DUMP_BUF_SIZE, GFP_KERNEL);
	if (dumpdss->dss_buf == NULL) {
		DPU_FB_ERR("alloc dss_buf failed!\n");
		kfree(dumpdss);
		dumpdss = NULL;
		return NULL;
	}
	memset(dumpdss->dss_buf, 0, DUMP_BUF_SIZE);

	DPU_FB_INFO("alloc dumpDss succeed!\n");

	return dumpdss;
}

void dpufb_free_dumpdss(struct dpu_fb_data_type *dpufd)
{
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL\n");
		return;
	}

	if (dpufd->dumpDss != NULL) {
		if (dpufd->dumpDss->dss_buf != NULL) {
			kfree(dpufd->dumpDss->dss_buf);
			dpufd->dumpDss->dss_buf = NULL;
		}

		kfree(dpufd->dumpDss);
		dpufd->dumpDss = NULL;
	}
}

void dump_dss_overlay(struct dpu_fb_data_type *dpufd, dss_overlay_t *pov_req)
{
	uint32_t i = 0;
	uint32_t k = 0;
	dss_layer_t const *layer = NULL;
	dss_wb_layer_t const *wb_layer = NULL;
	dss_overlay_block_t *pov_h_block_infos = NULL;
	dss_overlay_block_t *pov_block_info = NULL;

	struct dss_dump_data_type *dumpdss = NULL;

	dpu_check_and_no_retval((!dpufd), ERR, "dpufd is NULL\n");
	dpu_check_and_no_retval((!pov_req), ERR, "pov_req is NULL\n");
	dpu_check_and_no_retval((pov_req->ovl_idx < DSS_OVL0) || (pov_req->ovl_idx >= DSS_OVL_IDX_MAX),
		ERR, "ovl_idx is NULL\n");

	dumpdss = dpufd->dumpDss;
	dpu_check_and_no_retval((!dumpdss), ERR, "dumpDss is NULL\n");
	dpu_check_and_no_retval((dumpdss->dss_buf == NULL) || (dumpdss->dss_buf_len != 0),
		INFO, "dss_buf is NULL or dumpDss->dss_buf_len=%d", dumpdss->dss_buf_len);

	dumpdss->dss_buf_len += snprintf(dumpdss->dss_buf + dumpdss->dss_buf_len, 4 * SZ_1K,  /* 4K */
		"\n\n----------------------------<dump begin>----------------------------\n"
		"frame_no=%d\n"
		"ovl_idx=%d\n"
		"res_updt_rect[%d, %d, %d, %d]\n"
		"dirty_rect[%d,%d, %d,%d]\n"
		"release_fence=%d\n"
		"crc_enable_status=%d\n"
		"crc_info[%d,%d]\n"
		"ov_block_nums=%d\n"
		"ov_block_infos_ptr=0x%llx\n"
		"wb_enable=%d\n"
		"wb_layer_nums=%d\n"
		"wb_ov_rect[%d,%d, %d,%d]\n",
		pov_req->frame_no,
		pov_req->ovl_idx,
		pov_req->res_updt_rect.x,
		pov_req->res_updt_rect.y,
		pov_req->res_updt_rect.w,
		pov_req->res_updt_rect.h,
		pov_req->dirty_rect.x,
		pov_req->dirty_rect.y,
		pov_req->dirty_rect.w,
		pov_req->dirty_rect.h,
		pov_req->release_fence,
		pov_req->crc_enable_status,
		pov_req->crc_info.crc_ov_result,
		pov_req->crc_info.err_status,
		pov_req->ov_block_nums,
		pov_req->ov_block_infos_ptr,
		pov_req->wb_enable,
		pov_req->wb_layer_nums,
		pov_req->wb_ov_rect.x,
		pov_req->wb_ov_rect.y,
		pov_req->wb_ov_rect.w,
		pov_req->wb_ov_rect.h);

	for (i = 0; i < pov_req->ov_block_nums; i++) {
		pov_h_block_infos = (dss_overlay_block_t *)(uintptr_t)(pov_req->ov_block_infos_ptr);
		pov_block_info = &(pov_h_block_infos[i]);

		dumpdss->dss_buf_len += snprintf(dumpdss->dss_buf + dumpdss->dss_buf_len, 4 * SZ_1K,  /* 4K */
			"\nov_block_rect[%d,%d, %d,%d]\n"
			"layer_nums=%d\n",
			pov_block_info->ov_block_rect.x,
			pov_block_info->ov_block_rect.y,
			pov_block_info->ov_block_rect.w,
			pov_block_info->ov_block_rect.h,
			pov_block_info->layer_nums);

		for (k = 0; k < pov_block_info->layer_nums; k++) {
			layer = &(pov_block_info->layer_infos[k]);

			dumpdss->dss_buf_len += snprintf(dumpdss->dss_buf + dumpdss->dss_buf_len, 4 * SZ_1K,  /* 4K */
				"\nLayerInfo[%d]:\n"
				"format=%d\n"
				"width=%d\n"
				"height=%d\n"
				"bpp=%d\n"
				"buf_size=%d\n"
				"stride=%d\n"
				"stride_plane1=0x%x\n"
				"stride_plane2=0x%x\n"
				"offset_plane1=%d\n"
				"offset_plane2=%d\n"
				"afbc_header_stride=%d\n"
				"afbc_payload_stride=%d\n"
				"afbc_scramble_mode=%d\n"
				"mmbuf_base=0x%x\n"
				"mmbuf_size=%d\n"
				"mmu_enable=%d\n"
				"hfbc_header_stride0=0x%x\n"
				"hfbc_payload_stride0=0x%x\n"
				"hfbc_header_stride1=0x%x\n"
				"hfbc_payload_stride1=0x%x\n"
				"hfbc_scramble_mode=%d\n"
				"hebc_header_stride0=0x%x\n"
				"hebc_payload_stride0=0x%x\n"
				"hebc_header_stride1=0x%x\n"
				"hebc_payload_stride1=0x%x\n"
				"hebc_scramble_mode=%d\n"
				"csc_mode=%d\n"
				"secure_mode=%d\n"
				"shared_fd=%d\n"
				"src_rect[%d,%d, %d,%d]\n"
				"src_rect_mask[%d,%d, %d,%d]\n"
				"dst_rect[%d,%d, %d,%d]\n"
				"transform=%d\n"
				"blending=%d\n"
				"glb_alpha=0x%x\n"
				"color=0x%x\n"
				"layer_idx=%d\n"
				"chn_idx=%d\n"
				"need_cap=0x%x\n"
				"acquire_fence=%d\n",
				k,
				layer->img.format,
				layer->img.width,
				layer->img.height,
				layer->img.bpp,
				layer->img.buf_size,
				layer->img.stride,
				layer->img.stride_plane1,
				layer->img.stride_plane2,
				layer->img.offset_plane1,
				layer->img.offset_plane2,
				layer->img.afbc_header_stride,
				layer->img.afbc_payload_stride,
				layer->img.afbc_scramble_mode,
				layer->img.mmbuf_base,
				layer->img.mmbuf_size,
				layer->img.mmu_enable,
				layer->img.hfbc_header_stride0,
				layer->img.hfbc_payload_stride0,
				layer->img.hfbc_header_stride1,
				layer->img.hfbc_payload_stride1,
				layer->img.hfbc_scramble_mode,
				layer->img.hebc_header_stride0,
				layer->img.hebc_payload_stride0,
				layer->img.hebc_header_stride1,
				layer->img.hebc_payload_stride1,
				layer->img.hebc_scramble_mode,
				layer->img.csc_mode,
				layer->img.secure_mode,
				layer->img.shared_fd,
				layer->src_rect.x,
				layer->src_rect.y,
				layer->src_rect.w,
				layer->src_rect.h,
				layer->src_rect_mask.x,
				layer->src_rect_mask.y,
				layer->src_rect_mask.w,
				layer->src_rect_mask.h,
				layer->dst_rect.x,
				layer->dst_rect.y,
				layer->dst_rect.w,
				layer->dst_rect.h,
				layer->transform,
				layer->blending,
				layer->glb_alpha,
				layer->color,
				layer->layer_idx,
				layer->chn_idx,
				layer->need_cap,
				layer->acquire_fence);
		}
	}

	for (k = 0; k < pov_req->wb_layer_nums; k++) {
		wb_layer = &(pov_req->wb_layer_infos[k]);

		dumpdss->dss_buf_len += snprintf(dumpdss->dss_buf + dumpdss->dss_buf_len, 4 * SZ_1K,  /* 4K */
			"\nWbLayerInfo[%d]:\n"
			"format=%d\n"
			"width=%d\n"
			"height=%d\n"
			"bpp=%d\n"
			"buf_size=%d\n"
			"stride=%d\n"
			"stride_plane1=%d\n"
			"stride_plane2=%d\n"
			"offset_plane1=%d\n"
			"offset_plane2=%d\n"
			"afbc_header_stride=%d\n"
			"afbc_payload_stride=%d\n"
			"afbc_scramble_mode=%d\n"
			"mmbuf_base=0x%x\n"
			"mmbuf_size=%d\n"
			"hfbc_header_stride0=0x%x\n"
			"hfbc_payload_stride0=0x%x\n"
			"hfbc_header_stride1=0x%x\n"
			"hfbc_payload_stride1=0x%x\n"
			"hfbc_scramble_mode=%d\n"
			"hebc_header_stride0=0x%x\n"
			"hebc_payload_stride0=0x%x\n"
			"hebc_header_stride1=0x%x\n"
			"hebc_payload_stride1=0x%x\n"
			"hebc_scramble_mode=%d\n"
			"mmu_enable=%d\n"
			"csc_mode=%d\n"
			"secure_mode=%d\n"
			"shared_fd=%d\n"
			"src_rect[%d,%d, %d,%d]\n"
			"dst_rect[%d,%d, %d,%d]\n"
			"transform=%d\n"
			"chn_idx=%d\n"
			"need_cap=0x%x\n"
			"acquire_fence=%d\n"
			"release_fence=%d\n",
			k,
			wb_layer->dst.format,
			wb_layer->dst.width,
			wb_layer->dst.height,
			wb_layer->dst.bpp,
			wb_layer->dst.buf_size,
			wb_layer->dst.stride,
			wb_layer->dst.stride_plane1,
			wb_layer->dst.stride_plane2,
			wb_layer->dst.offset_plane1,
			wb_layer->dst.offset_plane2,
			wb_layer->dst.afbc_header_stride,
			wb_layer->dst.afbc_payload_stride,
			wb_layer->dst.afbc_scramble_mode,
			wb_layer->dst.mmbuf_base,
			wb_layer->dst.mmbuf_size,
			wb_layer->dst.hfbc_header_stride0,
			wb_layer->dst.hfbc_payload_stride0,
			wb_layer->dst.hfbc_header_stride1,
			wb_layer->dst.hfbc_payload_stride1,
			wb_layer->dst.hfbc_scramble_mode,
			wb_layer->dst.hebc_header_stride0,
			wb_layer->dst.hebc_payload_stride0,
			wb_layer->dst.hebc_header_stride1,
			wb_layer->dst.hebc_payload_stride1,
			wb_layer->dst.hebc_scramble_mode,
			wb_layer->dst.mmu_enable,
			wb_layer->dst.csc_mode,
			wb_layer->dst.secure_mode,
			wb_layer->dst.shared_fd,
			wb_layer->src_rect.x,
			wb_layer->src_rect.y,
			wb_layer->src_rect.w,
			wb_layer->src_rect.h,
			wb_layer->dst_rect.x,
			wb_layer->dst_rect.y,
			wb_layer->dst_rect.w,
			wb_layer->dst_rect.h,
			wb_layer->transform,
			wb_layer->chn_idx,
			wb_layer->need_cap,
			wb_layer->acquire_fence,
			wb_layer->release_fence);
	}

	dumpdss->dss_buf_len += snprintf(dumpdss->dss_buf + dumpdss->dss_buf_len, 4 * SZ_1K,  /* 4K */
		"----------------------------<dump end>----------------------------\n\n");

	for (k = 0; k < dumpdss->dss_buf_len; k += 255)
		DPU_FB_INFO("%.255s", dumpdss->dss_buf + k);

	memset(dumpdss->dss_buf, 0, DUMP_BUF_SIZE);
	dumpdss->dss_buf_len = 0;
}

void hisi_dss_debug_func(struct work_struct *work)
{
	struct dpu_fb_data_type *dpufd = NULL;

	dpu_check_and_no_retval(!work, ERR, "work is NULL\n");
	dpufd = container_of(work, struct dpu_fb_data_type, dss_debug_work);
	if (dpufd == NULL) {
		DPU_FB_ERR("dpufd is NULL point!\n");
		return;
	}

	dump_dss_overlay(dpufd, &dpufd->ov_req);
}
