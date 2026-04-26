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

void hisi_dss_post_clip_set_reg(struct dpu_fb_data_type *dpufd,
	char __iomem *post_clip_base, dss_post_clip_t *s_post_clip, int chn_idx)
{
	if (!dpufd || !post_clip_base || !s_post_clip) {
		DPU_FB_ERR("dpufd post_clip_base or s_post_clip is null\n");
		return;
	}

	dpufd->set_reg(dpufd, post_clip_base + POST_CLIP_DISP_SIZE, s_post_clip->disp_size, 32, 0);
	dpufd->set_reg(dpufd, post_clip_base + POST_CLIP_CTL_HRZ, s_post_clip->clip_ctl_hrz, 32, 0);
	dpufd->set_reg(dpufd, post_clip_base + POST_CLIP_CTL_VRZ, s_post_clip->clip_ctl_vrz, 32, 0);
	dpufd->set_reg(dpufd, post_clip_base + POST_CLIP_EN, s_post_clip->ctl_clip_en, 32, 0);
}

