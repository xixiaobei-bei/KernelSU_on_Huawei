/*
 * hal_venc.c
 *
 * This is venc register config.
 *
 * Copyright (c) 2010-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include "hal_venc.h"
#include "drv_common.h"

void venc_hal_get_reg_stream_len(struct stream_info *stream_info, uint32_t *reg_base)
{
	return;
}

void venc_hal_cfg_curld_osd01(struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;/*lint !e826 */
	U_VEDU_CURLD_OSD01_ALPHA D32;

	D32.bits.rgb_clip_min = channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.bits.rgb_clip_min;
	D32.bits.rgb_clip_max = channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.bits.rgb_clip_max;
	D32.bits.curld_hfbcd_clk_gt_en = channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.bits.curld_hfbcd_clk_gt_en;
	D32.bits.curld_hfbcd_bypass_en = channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.bits.curld_hfbcd_raw_en;
	D32.bits.curld_hfbcd_raw_en = channel_cfg->all_reg.VEDU_CURLD_OSD01_ALPHA.bits.curld_hfbcd_raw_en;
	all_reg->VEDU_CURLD_OSD01_ALPHA.u32 = D32.u32;
}

void vedu_hal_set_int(uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base; /*lint !e826 */
	all_reg->VEDU_VCPI_INTMASK.u32 = VENC_MASK;
}

void vedu_hal_cfg_smmu(struct encode_info *channel_cfg, uint32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());
	struct venc_context *ctx = &venc->ctx[core_id];

	if (ctx->first_cfg_flag == true) {
		venc_smmu_init(channel_cfg->is_protected, core_id);
		ctx->first_cfg_flag = false;
	}

	venc_smmu_cfg(channel_cfg, ctx->reg_base);
}

void vedu_hal_cfg(volatile uint32_t *base, uint32_t offset, uint32_t value)
{
	base[offset / 4] = value;
}
