/*
 * hal_venc_common.c
 *
 * venc register config
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
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
#include "drv_venc_efl.h"

void venc_hal_clr_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	vedu_reg->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
}

void venc_hal_disable_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	vedu_reg->VEDU_VCPI_INTMASK.u32 = 0;
}

void venc_hal_start_encode(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	if (vedu_reg) {
		vedu_reg->VEDU_VCPI_START.bits.vcpi_vstart = 0;
		vedu_reg->VEDU_VCPI_START.bits.vcpi_vstart = 1;
	}
}

void venc_hal_get_reg_qp_venc(struct stream_info *stream_info, uint32_t *reg_base)
{
	uint32_t i;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	volatile U_FUNC_SEL_LUMA_QP0_CNT* qp_cnt = &all_reg->FUNC_SEL_LUMA_QP0_CNT;

	for (i = 0; i < QP_HISTOGRAM_NUM; i++)
		stream_info->luma_qp_cnt[i] = qp_cnt[i].bits.luma_qp0_cnt;
}

void venc_hal_get_reg_venc(struct stream_info *stream_info, uint32_t *reg_base)
{
	uint32_t lcu_size;
	uint32_t lcu_block_num;
	uint32_t lcu8_block_num;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	uint32_t width = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgwidth_pix + 1;
	uint32_t height = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgheight_pix + 1;

	stream_info->is_buf_full =
		all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;
	stream_info->is_eop = all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;
	if (all_reg->VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265) {
		stream_info->pic_stream_size = all_reg->FUNC_CABAC_PIC_STRMSIZE;
		stream_info->block_cnt[BLOCK_4X4] =
			all_reg->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
		stream_info->block_cnt[BLOCK_8X8] =
			all_reg->FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt;
		stream_info->block_cnt[BLOCK_16X16] =
			all_reg->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt * 4;
		stream_info->block_cnt[BLOCK_32X32] =
			all_reg->FUNC_SEL_INTRA_OPT_32X32_CNT.bits.intra_opt_32x32_cnt * 16;
		lcu_size = LCU_SIZE_H265;
	} else {
		stream_info->pic_stream_size = all_reg->FUNC_VLC_PIC_STRMSIZE;
		stream_info->block_cnt[BLOCK_4X4] =
			all_reg->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
		stream_info->block_cnt[BLOCK_8X8] =
			all_reg->FUNC_SEL_INTRA_OPT_8X8_CNT.bits.intra_opt_8x8_cnt;
		stream_info->block_cnt[BLOCK_16X16] =
			all_reg->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt;
		stream_info->block_cnt[BLOCK_32X32] = 0;
		lcu_size = LCU_SIZE_H264;
	}

	lcu_block_num = (((width + lcu_size - 1) / lcu_size) *
		((height + lcu_size - 1) / lcu_size));
	lcu8_block_num = (((width + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE) *
		((height + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE));

	stream_info->ave_madi =
		all_reg->FUNC_PME_MADI_SUM.bits.pme_madi_sum / lcu_block_num;
	stream_info->frame_madp = all_reg->FUNC_PME_MADP_SUM.bits.pme_madp_sum;
	stream_info->average_luma_qp =
		all_reg->FUNC_SEL_TOTAL_LUMA_QP.bits.total_luma_qp / lcu8_block_num;
	stream_info->average_lcu_mse = all_reg->FUNC_SAO_MSE_SUM / lcu_block_num;

	venc_hal_get_reg_stream_len(stream_info, reg_base);
	venc_hal_get_reg_qp_venc(stream_info, reg_base);
}

void venc_hal_get_slice_reg(struct stream_info *stream_info, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	stream_info->is_buf_full =
		all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;
	venc_hal_get_reg_stream_len(stream_info, reg_base);
}
