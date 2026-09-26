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

void hisi_dss_post_clip_init(char __iomem *post_clip_base,
	dss_post_clip_t *s_post_clip)
{
	if (post_clip_base == NULL) {
		DPU_FB_ERR("post_clip_base is NULL\n");
		return;
	}
	if (s_post_clip == NULL) {
		DPU_FB_ERR("s_post_clip is NULL\n");
		return;
	}

	memset(s_post_clip, 0, sizeof(dss_post_clip_t));
}

int hisi_dss_post_clip_config(struct dpu_fb_data_type *dpufd, dss_layer_t *layer)
{
	dss_post_clip_t *post_clip = NULL;
	int chn_idx;
	dss_rect_t post_clip_rect;

	dpu_check_and_return(!dpufd, -EINVAL, ERR, "dpufd is nullptr!\n");
	dpu_check_and_return(!layer, -EINVAL, ERR, "layer is nullptr!\n");
	chn_idx = layer->chn_idx;
	post_clip_rect = layer->dst_rect;

	if (dpufd->index == MEDIACOMMON_PANEL_IDX)
		return 0;

	if (((chn_idx >= DSS_RCHN_V0) && (chn_idx <= DSS_RCHN_G1)) || (chn_idx == DSS_RCHN_V2))
	{
		post_clip = &(dpufd->dss_module.post_clip[chn_idx]);
		dpufd->dss_module.post_clip_used[chn_idx] = 1;

		post_clip->disp_size = set_bits32(post_clip->disp_size, DSS_HEIGHT(post_clip_rect.h), 13, 0);
		post_clip->disp_size = set_bits32(post_clip->disp_size, DSS_WIDTH(post_clip_rect.w), 13, 16);
		if ((chn_idx == DSS_RCHN_V0) && layer->block_info.arsr2p_left_clip) {
			post_clip->clip_ctl_hrz = set_bits32(post_clip->clip_ctl_hrz,
				layer->block_info.arsr2p_left_clip, 6, 16);
			post_clip->clip_ctl_hrz = set_bits32(post_clip->clip_ctl_hrz, 0x0, 6, 0);
		} else {
			post_clip->clip_ctl_hrz = set_bits32(post_clip->clip_ctl_hrz, 0x0, 32, 0);
		}

		post_clip->clip_ctl_vrz = set_bits32(post_clip->clip_ctl_vrz, 0x0, 32, 0);
		post_clip->ctl_clip_en = set_bits32(post_clip->ctl_clip_en, 0x1, 32, 0);
	}

	return 0;
}
