/*
 * hal_venc.c
 *
 * This is for venc register config.
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
#include <linux/hisi-iommu.h>
#include <linux/iommu.h>
#include "venc_regulator.h"
#include "drv_common.h"
#include "drv_mem.h"

/*lint -e438 -e838 -e826*/
#define LCU_SIZE_H265 64
#define LCU_SIZE_H264 16
#define BASE_BLOCK_SIZE 8
#define dist_protocol(protocol, value1, value2) (((protocol) == VEDU_H265) ? (value1) : (value2))

extern venc_smmu_err_add_t g_smmu_err_mem;
extern struct iommu_domain *g_hisi_mmu_domain;

static void venc_hal_get_reg_stream_len(struct stream_info *stream_info, uint32_t *reg_base)
{
	int i;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base; /*lint !e826 */
	volatile U_FUNC_VLCST_DSRPTR00 *base0 = &(all_reg->FUNC_VLCST_DSRPTR00);
	volatile U_FUNC_VLCST_DSRPTR01 *base1 = &(all_reg->FUNC_VLCST_DSRPTR01);

	for (i = 0; i < MAX_SLICE_NUM; i++) {
		stream_info->slice_len[i] = base0[i * 2].bits.slc_len0 - base1[i * 2].bits.invalidnum0;
		stream_info->aligned_slice_len[i] = base0[i * 2].bits.slc_len0;
		stream_info->slice_num++;
		if (base1[i * 2].bits.islastslc0)
			break;
	}
}

void venc_hal_get_slice_reg(struct stream_info *stream_info, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	stream_info->is_buf_full =
		all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;
	venc_hal_get_reg_stream_len(stream_info, reg_base);
}

void venc_hal_clr_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	vedu_reg->VEDU_VCPI_INTCLR.u32 = 0xFFFFFFFF;
}

void venc_hal_disable_all_int(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	vedu_reg->VEDU_VCPI_INTMASK.u32 = 0;
}

void venc_hal_set_smmu_addr(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	vedu_reg->SMMU_ERR_RDADDR                = g_smmu_err_mem.read_addr & 0xFFFFFFFF; // config alloc phyaddr,in order system don't dump
	vedu_reg->SMMU_ADDR_MSB.bits.msb_errrd   = (g_smmu_err_mem.read_addr >> 32) & 0x7F;
	vedu_reg->SMMU_ERR_WRADDR[0]             = g_smmu_err_mem.write_addr & 0xFFFFFFFF;
	vedu_reg->SMMU_ADDR_MSB.bits.msb_errwr   = (g_smmu_err_mem.write_addr >> 32) & 0x7F;
}

void venc_hal_start_encode(S_HEVC_AVC_REGS_TYPE *vedu_reg)
{
	if (vedu_reg) {
		vedu_reg->VEDU_VCPI_START.bits.vcpi_vstart = 0;
		vedu_reg->VEDU_VCPI_START.bits.vcpi_vstart = 1;
	}
}

void venc_hal_get_reg_venc(struct stream_info *stream_info, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base; /*lint !e826*/
	volatile uint32_t *base = &all_reg->FUNC_VLCST_DSRPTR00.u32;
	uint32_t width = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgwidth_pix + 1;
	uint32_t height = all_reg->VEDU_VCPI_PICSIZE_PIX.bits.vcpi_imgheight_pix + 1;
	uint32_t lcu_size;
	uint32_t i;
	uint32_t lcu_block_num;
	uint32_t lcu8_block_num;

	stream_info->is_buf_full = all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_buffull;
	stream_info->is_eop = all_reg->FUNC_VCPI_RAWINT.bits.vcpi_rint_ve_eop;
	stream_info->block_cnt[BLOCK_4X4] = all_reg->FUNC_SEL_OPT_4X4_CNT.bits.opt_4x4_cnt;
	stream_info->block_cnt[BLOCK_8X8] = all_reg->FUNC_SEL_INTRA_NORMAL_OPT_8X8_CNT.bits.intra_normal_opt_8x8_cnt;
	stream_info->block_cnt[BLOCK_16X16] = all_reg->FUNC_SEL_INTRA_OPT_16X16_CNT.bits.intra_opt_16x16_cnt;
	stream_info->block_cnt[BLOCK_32X32] = all_reg->FUNC_SEL_INTRA_OPT_32X32_CNT.bits.intra_opt_32x32_cnt;
	if (all_reg->VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265) {
		stream_info->pic_stream_size = all_reg->FUNC_CABAC_PIC_STRMSIZE;
		lcu_size = LCU_SIZE_H265;
	} else {
		stream_info->pic_stream_size = all_reg->VLC_PIC_TTBITS;
		stream_info->block_cnt[BLOCK_32X32] = 0;
		lcu_size = LCU_SIZE_H264;
		/* h264 block count register value is real value divided by 2 */
		for (i = 0; i < BLOCK_BUTT; i++)
			stream_info->block_cnt[i] *= 2;
	}

	lcu_block_num = (((width + lcu_size - 1) / lcu_size) * ((height + lcu_size - 1) / lcu_size));
	lcu8_block_num = (((width + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE) *
		((height + BASE_BLOCK_SIZE - 1) / BASE_BLOCK_SIZE));
	stream_info->ave_madi = all_reg->FUNC_PME_MADI_SUM.bits.pme_madi_sum / lcu_block_num;
	stream_info->frame_madp = all_reg->FUNC_PME_MADP_SUM.bits.pme_madp_sum;
	stream_info->average_luma_qp = all_reg->FUNC_SEL_TOTAL_LUMA_QP.bits.total_luma_qp / lcu8_block_num;
	stream_info->average_lcu_mse = all_reg->FUNC_SAO_MSE_SUM / lcu_block_num;

	for (i = 0; i < MAX_SLICE_NUM; i++) {
		stream_info->slice_len[i] = ((U_FUNC_VLCST_DSRPTR00 *)(&base[i * 2]))->bits.slc_len0 -
			((U_FUNC_VLCST_DSRPTR01 *)(&base[i * 2 + 1]))->bits.invalidnum0;
		stream_info->slice_num++;
		if (((U_FUNC_VLCST_DSRPTR01 *)(&base[i * 2 + 1]))->bits.islastslc0)
			break;
	}
}

void vedu_hal_cfg_reg_intraset(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32 = channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32;

	all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32 = channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32;

	all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32 = channel_cfg->all_reg.VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32;

	all_reg->VEDU_PME_NEW_COST.bits.pme_new_cost_en = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_new_cost_en;
}

void vedu_hal_cfg_reg_lambda_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	// lambd reg cfg set
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG00.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG01.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG02.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG03.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG04.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG05.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG06.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG07.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG08.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG09.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG10.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG11.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG12.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG13.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG14.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG15.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG16.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG17.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG18.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG19.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG20.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG21.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG22.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG23.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG24.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG25.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG26.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG27.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG28.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG29.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG30.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG31.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG32.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG33.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG34.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG35.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG36.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG37.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG38.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG39.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG40.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG41.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG42.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG43.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG44.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG45.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG46.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG47.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG48.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG49.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG50.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG51.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG52.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG53.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG54.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG55.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG56.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG57.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG58.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG59.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG60.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG61.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG62.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG63.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG64.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG65.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG66.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG67.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG68.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG69.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG70.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG71.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG72.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG73.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG74.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG75.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG76.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG77.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG78.u32;
	all_reg->VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32 = channel_cfg->all_reg.VEDU_QPG_QP_LAMBDA_CTRL_REG79.u32;
}

void vedu_hal_cfg_reg_qpg_map_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	all_reg->VEDU_VCTRL_ROI_CFG0.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG0.u32;
	all_reg->VEDU_VCTRL_ROI_CFG1.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG1.u32;
	all_reg->VEDU_VCTRL_ROI_CFG2.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_CFG2.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_0.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_0.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_1.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_1.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_2.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_2.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_3.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_3.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_4.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_4.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_5.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_5.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_6.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_6.u32;
	all_reg->VEDU_VCTRL_ROI_SIZE_7.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_SIZE_7.u32;
	all_reg->VEDU_VCTRL_ROI_START_0.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_0.u32;
	all_reg->VEDU_VCTRL_ROI_START_1.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_1.u32;
	all_reg->VEDU_VCTRL_ROI_START_2.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_2.u32;
	all_reg->VEDU_VCTRL_ROI_START_3.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_3.u32;
	all_reg->VEDU_VCTRL_ROI_START_4.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_4.u32;
	all_reg->VEDU_VCTRL_ROI_START_5.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_5.u32;
	all_reg->VEDU_VCTRL_ROI_START_6.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_6.u32;
	all_reg->VEDU_VCTRL_ROI_START_7.u32 = channel_cfg->all_reg.VEDU_VCTRL_ROI_START_7.u32;
	all_reg->VEDU_VCPI_RC_ENABLE.u32 = channel_cfg->all_reg.VEDU_VCPI_RC_ENABLE.u32;
	all_reg->VEDU_QPG_SMART_REG.u32 = channel_cfg->all_reg.VEDU_QPG_SMART_REG.u32;
}

void vedu_hal_cfg_reg_addr_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	HI_DBG_VENC("set Vedu Hal cfg reg addr");
	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	// Col_frm_flag PMC_POC And So On;   I P B All need to Cfg
	all_reg->VEDU_VCPI_STRFMT.u32 = channel_cfg->all_reg.VEDU_VCPI_STRFMT.u32;
	all_reg->VEDU_VCPI_REF_FLAG.u32 = channel_cfg->all_reg.VEDU_VCPI_REF_FLAG.u32;
	all_reg->VEDU_PMV_POC_0 = channel_cfg->all_reg.VEDU_PMV_POC_0;
	all_reg->VEDU_PMV_POC_1 = channel_cfg->all_reg.VEDU_PMV_POC_1;
	all_reg->VEDU_PMV_POC_2 = channel_cfg->all_reg.VEDU_PMV_POC_2;
	all_reg->VEDU_PMV_POC_3 = channel_cfg->all_reg.VEDU_PMV_POC_3;
	all_reg->VEDU_PMV_POC_4 = channel_cfg->all_reg.VEDU_PMV_POC_4;
	all_reg->VEDU_PMV_POC_5 = channel_cfg->all_reg.VEDU_PMV_POC_5;

	all_reg->VEDU_VCPI_TUNLCELL_ADDR = channel_cfg->all_reg.VEDU_VCPI_TUNLCELL_ADDR;
	all_reg->VEDU_VCPI_SRC_YADDR = channel_cfg->all_reg.VEDU_VCPI_SRC_YADDR;
	all_reg->VEDU_VCPI_SRC_CADDR = channel_cfg->all_reg.VEDU_VCPI_SRC_CADDR;
	all_reg->VEDU_VCPI_SRC_VADDR = channel_cfg->all_reg.VEDU_VCPI_SRC_VADDR;

	all_reg->VEDU_VCPI_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_STRIDE.u32;
	HI_DBG_VENC("reg VEDU_VCPI_SRC_YADDR:%pK", (void *)(uintptr_t)(channel_cfg->all_reg.VEDU_VCPI_SRC_YADDR));
	HI_DBG_VENC("reg VEDU_VCPI_SRC_CADDR:%pK", (void *)(uintptr_t)(channel_cfg->all_reg.VEDU_VCPI_SRC_CADDR));
	HI_DBG_VENC("reg VEDU_VCPI_SRC_VADDR:%pK", (void *)(uintptr_t)(channel_cfg->all_reg.VEDU_VCPI_SRC_VADDR));
	HI_DBG_VENC("reg vcpi_curld_c_stride:%x", channel_cfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_c_stride);
	HI_DBG_VENC("reg vcpi_curld_y_stride:%x", channel_cfg->all_reg.VEDU_VCPI_STRIDE.bits.vcpi_curld_y_stride);
	if (channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == 10) {
		all_reg->VEDU_VCPI_YH_ADDR = channel_cfg->all_reg.VEDU_VCPI_YH_ADDR;
		all_reg->VEDU_VCPI_CH_ADDR = channel_cfg->all_reg.VEDU_VCPI_CH_ADDR;
		all_reg->VEDU_CURLD_SRCH_STRIDE.u32 = channel_cfg->all_reg.VEDU_CURLD_SRCH_STRIDE.u32;
		all_reg->VEDU_CURLD_HFBCD.u32 = channel_cfg->all_reg.VEDU_CURLD_HFBCD.u32;
		HI_DBG_VENC("reg VEDU_VCPI_CH_ADDR=%pK", (void *)(uintptr_t)(channel_cfg->all_reg.VEDU_VCPI_YH_ADDR));
		HI_DBG_VENC("reg VEDU_VCPI_CH_ADDR=%pK", (void *)(uintptr_t)(channel_cfg->all_reg.VEDU_VCPI_CH_ADDR));
		HI_DBG_VENC("reg vcpi_curld_srcch_stride=%x", channel_cfg->all_reg.VEDU_CURLD_SRCH_STRIDE.bits.vcpi_curld_srcch_stride);
		HI_DBG_VENC("reg vcpi_curld_srcyh_stride=%x", channel_cfg->all_reg.VEDU_CURLD_SRCH_STRIDE.bits.vcpi_curld_srcyh_stride);
	}

	// -------------------REC,REF0,REF1,PME0,PME1,NBI Relate ADDR Stide-----------------------
	// REC-------------------REC-----------------REC--------------------------------------------
	all_reg->VEDU_VCPI_REC_YADDR = channel_cfg->all_reg.VEDU_VCPI_REC_YADDR;
	all_reg->VEDU_VCPI_REC_CADDR = channel_cfg->all_reg.VEDU_VCPI_REC_CADDR;
	all_reg->VEDU_VCPI_REC_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REC_STRIDE.u32;

	all_reg->VEDU_VCPI_REC_YH_ADDR = channel_cfg->all_reg.VEDU_VCPI_REC_YH_ADDR;
	all_reg->VEDU_VCPI_REC_CH_ADDR = channel_cfg->all_reg.VEDU_VCPI_REC_CH_ADDR;
	all_reg->VEDU_VCPI_REC_HEAD_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REC_HEAD_STRIDE.u32;

	// REF-------------------REF-----------------REF--------------------------------------------
	// REF0
	all_reg->VEDU_VCPI_REFY_L0_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFY_L0_ADDR;
	all_reg->VEDU_VCPI_REFC_L0_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFC_L0_ADDR;
	all_reg->VEDU_VCPI_REF_L0_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REF_L0_STRIDE.u32;
	all_reg->VEDU_VCPI_REFYH_L0_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFYH_L0_ADDR;
	all_reg->VEDU_VCPI_REFCH_L0_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFCH_L0_ADDR;

	all_reg->VEDU_VCPI_REFH_L0_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REFH_L0_STRIDE.u32;

	all_reg->VEDU_VCPI_PMELD_L0_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMELD_L0_ADDR;
	// REF1
	all_reg->VEDU_VCPI_REFY_L1_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFY_L1_ADDR;
	all_reg->VEDU_VCPI_REFC_L1_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFC_L1_ADDR;
	all_reg->VEDU_VCPI_REF_L1_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REF_L1_STRIDE.u32;

	all_reg->VEDU_VCPI_REFYH_L1_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFYH_L1_ADDR;
	all_reg->VEDU_VCPI_REFCH_L1_ADDR = channel_cfg->all_reg.VEDU_VCPI_REFCH_L1_ADDR;

	all_reg->VEDU_VCPI_REFH_L1_STRIDE.u32 = channel_cfg->all_reg.VEDU_VCPI_REFH_L1_STRIDE.u32;
	all_reg->VEDU_VCPI_PMELD_L1_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMELD_L1_ADDR;

	// PME NBI
	all_reg->VEDU_VCPI_PMEST_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMEST_ADDR;

	all_reg->VEDU_VCPI_PMEST_STRIDE = channel_cfg->all_reg.VEDU_VCPI_PMEST_STRIDE;

	all_reg->VEDU_VCPI_PMELD_STRIDE = channel_cfg->all_reg.VEDU_VCPI_PMELD_STRIDE;

	// NBI
	all_reg->VEDU_VCPI_NBI_MVST_ADDR = channel_cfg->all_reg.VEDU_VCPI_NBI_MVST_ADDR;
	all_reg->VEDU_VCPI_NBI_MVLD_ADDR = channel_cfg->all_reg.VEDU_VCPI_NBI_MVLD_ADDR;

	// PMEINFO
	all_reg->VEDU_VCPI_PMEINFO_ST_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMEINFO_ST_ADDR;
	all_reg->VEDU_VCPI_PMEINFO_LD0_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD0_ADDR;
	all_reg->VEDU_VCPI_PMEINFO_LD1_ADDR = channel_cfg->all_reg.VEDU_VCPI_PMEINFO_LD1_ADDR;
	// QPGLD
	all_reg->VEDU_VCPI_QPGLD_INF_ADDR = channel_cfg->all_reg.VEDU_VCPI_QPGLD_INF_ADDR;
}

void vedu_hal_cfg_reg_slc_head_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	HI_DBG_VENC("set Vedu Hal SlcHead");
	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	// ----------------------------Stream head info------------------------------------------
	all_reg->VEDU_VLCST_STRMBUFLEN0 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN0;
	all_reg->VEDU_VLCST_STRMBUFLEN1 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN1;
	all_reg->VEDU_VLCST_STRMBUFLEN2 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN2;
	all_reg->VEDU_VLCST_STRMBUFLEN3 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN3;
	all_reg->VEDU_VLCST_STRMBUFLEN4 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN4;
	all_reg->VEDU_VLCST_STRMBUFLEN5 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN5;
	all_reg->VEDU_VLCST_STRMBUFLEN6 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN6;
	all_reg->VEDU_VLCST_STRMBUFLEN7 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN7;
	all_reg->VEDU_VLCST_STRMBUFLEN8 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN8;
	all_reg->VEDU_VLCST_STRMBUFLEN9 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN9;
	all_reg->VEDU_VLCST_STRMBUFLEN10 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN10;
	all_reg->VEDU_VLCST_STRMBUFLEN11 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN11;
	all_reg->VEDU_VLCST_STRMBUFLEN12 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN12;
	all_reg->VEDU_VLCST_STRMBUFLEN13 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN13;
	all_reg->VEDU_VLCST_STRMBUFLEN14 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN14;
	all_reg->VEDU_VLCST_STRMBUFLEN15 = channel_cfg->all_reg.VEDU_VLCST_STRMBUFLEN15;
	all_reg->VEDU_VLCST_STRMADDR0 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR0;
	all_reg->VEDU_VLCST_STRMADDR1 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR1;
	all_reg->VEDU_VLCST_STRMADDR2 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR2;
	all_reg->VEDU_VLCST_STRMADDR3 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR3;
	all_reg->VEDU_VLCST_STRMADDR4 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR4;
	all_reg->VEDU_VLCST_STRMADDR5 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR5;
	all_reg->VEDU_VLCST_STRMADDR6 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR6;
	all_reg->VEDU_VLCST_STRMADDR7 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR7;
	all_reg->VEDU_VLCST_STRMADDR8 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR8;
	all_reg->VEDU_VLCST_STRMADDR9 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR9;
	all_reg->VEDU_VLCST_STRMADDR10 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR10;
	all_reg->VEDU_VLCST_STRMADDR11 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR11;
	all_reg->VEDU_VLCST_STRMADDR12 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR12;
	all_reg->VEDU_VLCST_STRMADDR13 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR13;
	all_reg->VEDU_VLCST_STRMADDR14 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR14;
	all_reg->VEDU_VLCST_STRMADDR15 = channel_cfg->all_reg.VEDU_VLCST_STRMADDR15;

	all_reg->VEDU_CABAC_GLB_CFG.u32 = channel_cfg->all_reg.VEDU_CABAC_GLB_CFG.u32;
	all_reg->VEDU_CABAC_SLCHDR_SIZE.u32 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_SIZE.u32;

	all_reg->VEDU_CABAC_SLCHDR_PART1.bits.cabac_slchdr_part1 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART1.bits.cabac_slchdr_part1;

	all_reg->VEDU_VCPI_VLC_CONFIG.u32 = channel_cfg->all_reg.VEDU_VCPI_VLC_CONFIG.u32;

	if (channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol == VEDU_H265) {
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG1 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG1;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG2 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG2;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG3 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG3;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG4 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG4;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG5 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG5;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG6 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG6;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG7 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG7;
		all_reg->VEDU_CABAC_SLCHDR_PART2_SEG8 = channel_cfg->all_reg.VEDU_CABAC_SLCHDR_PART2_SEG8;
	} else {
		all_reg->VEDU_VLC_SLCHDRSTRM0 = channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM0;
		all_reg->VEDU_VLC_SLCHDRSTRM1 = channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM1;
		all_reg->VEDU_VLC_SLCHDRSTRM2 = channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM2;
		all_reg->VEDU_VLC_SLCHDRSTRM3 = channel_cfg->all_reg.VEDU_VLC_SLCHDRSTRM3;
		all_reg->VEDU_VLC_REORDERSTRM0 = channel_cfg->all_reg.VEDU_VLC_REORDERSTRM0;
		all_reg->VEDU_VLC_REORDERSTRM1 = channel_cfg->all_reg.VEDU_VLC_REORDERSTRM1;
		all_reg->VEDU_VLC_MARKINGSTRM0 = channel_cfg->all_reg.VEDU_VLC_MARKINGSTRM0;
		all_reg->VEDU_VLC_MARKINGSTRM1 = channel_cfg->all_reg.VEDU_VLC_MARKINGSTRM1;

		all_reg->VEDU_VLC_SLCHDRPARA.u32 = channel_cfg->all_reg.VEDU_VLC_SLCHDRPARA.u32;
		all_reg->VEDU_VLC_SVC.u32 = channel_cfg->all_reg.VEDU_VLC_SVC.u32;
	}
	// ----------------------------Cabac/Vlc Stream Head info end-------------------------
	all_reg->VEDU_VLCST_PARAMETER.u32 = channel_cfg->all_reg.VEDU_VLCST_PARAMETER.u32;
	if (channel_cfg->all_reg.VEDU_VLCST_PARAMETER.bits.vlcst_para_set_en == 1) {
		all_reg->VEDU_VLCST_PARA_DATA0 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA0;
		all_reg->VEDU_VLCST_PARA_DATA1 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA1;
		all_reg->VEDU_VLCST_PARA_DATA2 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA2;
		all_reg->VEDU_VLCST_PARA_DATA3 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA3;
		all_reg->VEDU_VLCST_PARA_DATA4 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA4;
		all_reg->VEDU_VLCST_PARA_DATA5 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA5;
		all_reg->VEDU_VLCST_PARA_DATA6 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA6;
		all_reg->VEDU_VLCST_PARA_DATA7 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA7;
		all_reg->VEDU_VLCST_PARA_DATA8 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA8;
		HI_DBG_VENC("reg VLCST_PARA_DATA0:0x%x", all_reg->VEDU_VLCST_PARA_DATA0);
		HI_DBG_VENC("reg VLCST_PARA_DATA1:0x%x", all_reg->VEDU_VLCST_PARA_DATA1);
		HI_DBG_VENC("reg VLCST_PARA_DATA2:0x%x", all_reg->VEDU_VLCST_PARA_DATA2);
		HI_DBG_VENC("reg VLCST_PARA_DATA3:0x%x", all_reg->VEDU_VLCST_PARA_DATA3);
		HI_DBG_VENC("reg VLCST_PARA_DATA4:0x%x", all_reg->VEDU_VLCST_PARA_DATA4);
		HI_DBG_VENC("reg VLCST_PARA_DATA5:0x%x", all_reg->VEDU_VLCST_PARA_DATA5);
		HI_DBG_VENC("reg VLCST_PARA_DATA6:0x%x", all_reg->VEDU_VLCST_PARA_DATA6);
		HI_DBG_VENC("reg VLCST_PARA_DATA7:0x%x", all_reg->VEDU_VLCST_PARA_DATA7);
		HI_DBG_VENC("reg VLCST_PARA_DATA8:0x%x", all_reg->VEDU_VLCST_PARA_DATA8);
		all_reg->VEDU_VLCST_PARA_DATA9 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA9;
		all_reg->VEDU_VLCST_PARA_DATA10 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA10;
		all_reg->VEDU_VLCST_PARA_DATA11 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA11;
		all_reg->VEDU_VLCST_PARA_DATA12 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA12;
		all_reg->VEDU_VLCST_PARA_DATA13 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA13;
		all_reg->VEDU_VLCST_PARA_DATA14 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA14;
		all_reg->VEDU_VLCST_PARA_DATA15 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA15;
		all_reg->VEDU_VLCST_PARA_DATA16 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA16;
		all_reg->VEDU_VLCST_PARA_DATA17 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA17;
		all_reg->VEDU_VLCST_PARA_DATA18 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA18;
		all_reg->VEDU_VLCST_PARA_DATA19 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA19;
		all_reg->VEDU_VLCST_PARA_DATA20 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA20;
		all_reg->VEDU_VLCST_PARA_DATA21 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA21;
		all_reg->VEDU_VLCST_PARA_DATA22 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA22;
		all_reg->VEDU_VLCST_PARA_DATA23 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA23;
		all_reg->VEDU_VLCST_PARA_DATA24 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA24;
		all_reg->VEDU_VLCST_PARA_DATA25 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA25;
		all_reg->VEDU_VLCST_PARA_DATA26 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA26;
		all_reg->VEDU_VLCST_PARA_DATA27 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA27;
		all_reg->VEDU_VLCST_PARA_DATA28 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA28;
		all_reg->VEDU_VLCST_PARA_DATA29 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA29;
		all_reg->VEDU_VLCST_PARA_DATA30 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA30;
		all_reg->VEDU_VLCST_PARA_DATA31 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA31;
		all_reg->VEDU_VLCST_PARA_DATA32 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA32;
		all_reg->VEDU_VLCST_PARA_DATA33 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA33;
		all_reg->VEDU_VLCST_PARA_DATA34 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA34;
		all_reg->VEDU_VLCST_PARA_DATA35 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA35;
		all_reg->VEDU_VLCST_PARA_DATA36 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA36;
		all_reg->VEDU_VLCST_PARA_DATA37 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA37;
		all_reg->VEDU_VLCST_PARA_DATA38 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA38;
		all_reg->VEDU_VLCST_PARA_DATA39 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA39;
		all_reg->VEDU_VLCST_PARA_DATA40 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA40;
		all_reg->VEDU_VLCST_PARA_DATA41 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA41;
		all_reg->VEDU_VLCST_PARA_DATA42 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA42;
		all_reg->VEDU_VLCST_PARA_DATA43 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA43;
		all_reg->VEDU_VLCST_PARA_DATA44 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA44;
		all_reg->VEDU_VLCST_PARA_DATA45 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA45;
		all_reg->VEDU_VLCST_PARA_DATA46 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA46;
		all_reg->VEDU_VLCST_PARA_DATA47 = channel_cfg->all_reg.VEDU_VLCST_PARA_DATA47;
	}
}

void vedu_hal_cfg_reg_smmu_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	uint64_t  smmu_page_base_addr = venc_get_smmu_ttb();

	// SMRX_0
	all_reg->MMU_PRE_GLB_SCR.bits.glb_scr = 1;
	all_reg->SMMU_MSTR_GLB_BYPASS.bits.glb_bypass = 0;
	all_reg->SMMU_SCR.bits.glb_bypass = 0;

	// QOS
	all_reg->SMMU_SCR.bits.rqos_en = 1;
	all_reg->SMMU_SCR.bits.wqos_en = 1;
	all_reg->SMMU_SCR.bits.rqos = 0;
	all_reg->SMMU_SCR.bits.wqos = 0;

	all_reg->SMMU_CB_SCTRL.bits.cb_bypass = 0;
	all_reg->SMMU_CB_TTBCR.bits.cb_ttbcr_des = 1;
	all_reg->SMMU_CB_TTBR0 = smmu_page_base_addr;/*lint !e712 */
	all_reg->SMMU_CB_TTBR1 = smmu_page_base_addr;/*lint !e712 */

	all_reg->SMMU_FAMA_CTRL1_NS[0].bits.fama_ptw_msb_ns = (smmu_page_base_addr >> 32) & 0x7F;

	venc_hal_set_smmu_addr(all_reg);

	{
		// nosec
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[15].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[16].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[17].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[18].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 1;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[19].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 3;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[20].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[21].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[22].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 72;
			D32.bits.len = 8;
			all_reg->SMMU_MSTR_SMRX_0[23].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 36;
			D32.bits.len = 4;
			all_reg->SMMU_MSTR_SMRX_0[24].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[25].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[26].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 48;
			D32.bits.len = 8;
			all_reg->SMMU_MSTR_SMRX_0[27].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 8;
			D32.bits.len = 2;
			all_reg->SMMU_MSTR_SMRX_0[28].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 8;
			D32.bits.len = 2;
			all_reg->SMMU_MSTR_SMRX_0[29].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[39].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 1;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[40].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 1;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[41].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[42].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[43].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 18;
			D32.bits.len = 8;
			all_reg->SMMU_MSTR_SMRX_0[44].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 9;
			D32.bits.len = 4;
			all_reg->SMMU_MSTR_SMRX_0[45].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[46].u32 = D32.u32;
		}
		{
			U_SMMU_MSTR_SMRX_0  D32;
			D32.bits.bypass = 0;
			D32.bits.upwin = 0;
			D32.bits.len = 1;
			all_reg->SMMU_MSTR_SMRX_0[47].u32 = D32.u32;
		}
	}
}

void vedu_hal_cfg_reg_premmu_set(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;

	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	HI_DBG_VENC("MMU RRE ADDR enter");
	HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_1[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK", (void *)(&(all_reg->MMU_PRE_NBI_MVST_ADDR_STR)),
				(&(all_reg->SMMU_MSTR_SMRX_1[0])), (void *)(uintptr_t)(&(all_reg->MMU_PRE_STRMADDR0_STR)));
	HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_1[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK", (void *)(&(all_reg->MMU_PRE_NBI_MVST_ADDR_STR)),
				(&(all_reg->SMMU_MSTR_SMRX_1[0])), (void *)(uintptr_t)(&(all_reg->MMU_PRE_STRMADDR0_STR)));
	HI_DBG_VENC("MMU_PRE_NBI_MVST_ADDR_STR:%pK, SMMU_MSTR_SMRX_2[0]:%pK, MMU_PRE_STRMADDR0_STR:%pK", (void *)(&(all_reg->MMU_PRE_NBI_MVST_ADDR_STR)),
				(&(all_reg->SMMU_MSTR_SMRX_2[0])), (void *)(uintptr_t)(&(all_reg->MMU_PRE_STRMADDR0_STR)));
	// PRE_ADDR
	all_reg->MMU_PRE_NBI_MVST_ADDR_STR = channel_cfg->all_reg.MMU_PRE_NBI_MVST_ADDR_STR;
	all_reg->MMU_PRE_NBI_MVST_ADDR_END = channel_cfg->all_reg.MMU_PRE_NBI_MVST_ADDR_END;

	all_reg->MMU_PRE_NBI_MVLD_ADDR_STR = channel_cfg->all_reg.MMU_PRE_NBI_MVLD_ADDR_STR;
	all_reg->MMU_PRE_NBI_MVLD_ADDR_END = channel_cfg->all_reg.MMU_PRE_NBI_MVLD_ADDR_END;

	all_reg->MMU_PRE_PMEST_ADDR_STR = channel_cfg->all_reg.MMU_PRE_PMEST_ADDR_STR;
	all_reg->MMU_PRE_PMEST_ADDR_END = channel_cfg->all_reg.MMU_PRE_PMEST_ADDR_END;

	all_reg->MMU_PRE_PMELD_ADDR_STR = channel_cfg->all_reg.MMU_PRE_PMELD_ADDR_STR;
	all_reg->MMU_PRE_PMELD_ADDR_END = channel_cfg->all_reg.MMU_PRE_PMELD_ADDR_END;

	all_reg->MMU_PRE_PMEINFOST_ADDR_STR = channel_cfg->all_reg.MMU_PRE_PMEINFOST_ADDR_STR;
	all_reg->MMU_PRE_PMEINFOST_ADDR_END = channel_cfg->all_reg.MMU_PRE_PMEINFOST_ADDR_END;

	all_reg->MMU_PRE_PMEINFOLD0_ADDR_STR = channel_cfg->all_reg.MMU_PRE_PMEINFOLD0_ADDR_STR;
	all_reg->MMU_PRE_PMEINFOLD0_ADDR_END = channel_cfg->all_reg.MMU_PRE_PMEINFOLD0_ADDR_END;

	all_reg->MMU_PRE_PMEINFOLD1_ADDR_STR = channel_cfg->all_reg.MMU_PRE_PMEINFOLD1_ADDR_STR;
	all_reg->MMU_PRE_PMEINFOLD1_ADDR_END = channel_cfg->all_reg.MMU_PRE_PMEINFOLD1_ADDR_END;

	all_reg->MMU_PRE_QPGLD_ADDR_STR = channel_cfg->all_reg.MMU_PRE_QPGLD_ADDR_STR;
	all_reg->MMU_PRE_QPGLD_ADDR_END = channel_cfg->all_reg.MMU_PRE_QPGLD_ADDR_END;

	// REC
	all_reg->MMU_PRE_REC_YH_ADDR_STR = channel_cfg->all_reg.MMU_PRE_REC_YH_ADDR_STR;
	all_reg->MMU_PRE_REC_YH_ADDR_END = channel_cfg->all_reg.MMU_PRE_REC_YH_ADDR_END;

	all_reg->MMU_PRE_REC_CH_ADDR_STR = channel_cfg->all_reg.MMU_PRE_REC_CH_ADDR_STR;
	all_reg->MMU_PRE_REC_CH_ADDR_END = channel_cfg->all_reg.MMU_PRE_REC_CH_ADDR_END;

	all_reg->MMU_PRE_REC_YADDR_STR = channel_cfg->all_reg.MMU_PRE_REC_YADDR_STR;
	all_reg->MMU_PRE_REC_YADDR_END = channel_cfg->all_reg.MMU_PRE_REC_YADDR_END;

	all_reg->MMU_PRE_REC_CADDR_STR = channel_cfg->all_reg.MMU_PRE_REC_CADDR_STR;
	all_reg->MMU_PRE_REC_CADDR_END = channel_cfg->all_reg.MMU_PRE_REC_CADDR_END;

	// REF
	all_reg->MMU_PRE_REF_YH_ADDR_STR = channel_cfg->all_reg.MMU_PRE_REF_YH_ADDR_STR;
	all_reg->MMU_PRE_REF_YH_ADDR_END = channel_cfg->all_reg.MMU_PRE_REF_YH_ADDR_END;

	all_reg->MMU_PRE_REF_CH_ADDR_STR = channel_cfg->all_reg.MMU_PRE_REF_CH_ADDR_STR;
	all_reg->MMU_PRE_REF_CH_ADDR_END = channel_cfg->all_reg.MMU_PRE_REF_CH_ADDR_END;

	all_reg->MMU_PRE_REF_YADDR_STR = channel_cfg->all_reg.MMU_PRE_REF_YADDR_STR;
	all_reg->MMU_PRE_REF_YADDR_END = channel_cfg->all_reg.MMU_PRE_REF_YADDR_END;

	all_reg->MMU_PRE_REF_CADDR_STR = channel_cfg->all_reg.MMU_PRE_REF_CADDR_STR;
	all_reg->MMU_PRE_REF_CADDR_END = channel_cfg->all_reg.MMU_PRE_REF_CADDR_END;

	// SRC
	if (channel_cfg->all_reg.VEDU_VCPI_STRFMT.bits.vcpi_str_fmt == 10) {
		all_reg->MMU_PRE_SRC_YHADDR_STR = channel_cfg->all_reg.MMU_PRE_SRC_YHADDR_STR;
		all_reg->MMU_PRE_SRC_YHADDR_END = channel_cfg->all_reg.MMU_PRE_SRC_YHADDR_END;

		all_reg->MMU_PRE_SRC_CHADDR_STR = channel_cfg->all_reg.MMU_PRE_SRC_CHADDR_STR;
		all_reg->MMU_PRE_SRC_CHADDR_END = channel_cfg->all_reg.MMU_PRE_SRC_CHADDR_END;
	}
	all_reg->MMU_PRE_SRC_YADDR_STR = channel_cfg->all_reg.MMU_PRE_SRC_YADDR_STR;
	all_reg->MMU_PRE_SRC_YADDR_END = channel_cfg->all_reg.MMU_PRE_SRC_YADDR_END;

	all_reg->MMU_PRE_SRC_CADDR_STR = channel_cfg->all_reg.MMU_PRE_SRC_CADDR_STR;
	all_reg->MMU_PRE_SRC_CADDR_END = channel_cfg->all_reg.MMU_PRE_SRC_CADDR_END;

	all_reg->MMU_PRE_SRC_VADDR_STR = channel_cfg->all_reg.MMU_PRE_SRC_VADDR_STR;
	all_reg->MMU_PRE_SRC_VADDR_END = channel_cfg->all_reg.MMU_PRE_SRC_VADDR_END;

	// LOWDLY
	all_reg->MMU_PRE_LOWDLY_ADDR_STR = channel_cfg->all_reg.MMU_PRE_LOWDLY_ADDR_STR;
	all_reg->MMU_PRE_LOWDLY_ADDR_END = channel_cfg->all_reg.MMU_PRE_LOWDLY_ADDR_END;

	// STREAM
	all_reg->MMU_PRE_STRMADDR0_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR0_STR;
	all_reg->MMU_PRE_STRMADDR0_END = channel_cfg->all_reg.MMU_PRE_STRMADDR0_END;

	all_reg->MMU_PRE_STRMADDR1_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR1_STR;
	all_reg->MMU_PRE_STRMADDR1_END = channel_cfg->all_reg.MMU_PRE_STRMADDR1_END;

	all_reg->MMU_PRE_STRMADDR2_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR2_STR;
	all_reg->MMU_PRE_STRMADDR2_END = channel_cfg->all_reg.MMU_PRE_STRMADDR2_END;

	all_reg->MMU_PRE_STRMADDR3_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR3_STR;
	all_reg->MMU_PRE_STRMADDR3_END = channel_cfg->all_reg.MMU_PRE_STRMADDR3_END;

	all_reg->MMU_PRE_STRMADDR4_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR4_STR;
	all_reg->MMU_PRE_STRMADDR4_END = channel_cfg->all_reg.MMU_PRE_STRMADDR4_END;

	all_reg->MMU_PRE_STRMADDR5_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR5_STR;
	all_reg->MMU_PRE_STRMADDR5_END = channel_cfg->all_reg.MMU_PRE_STRMADDR5_END;

	all_reg->MMU_PRE_STRMADDR6_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR6_STR;
	all_reg->MMU_PRE_STRMADDR6_END = channel_cfg->all_reg.MMU_PRE_STRMADDR6_END;

	all_reg->MMU_PRE_STRMADDR7_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR7_STR;
	all_reg->MMU_PRE_STRMADDR7_END = channel_cfg->all_reg.MMU_PRE_STRMADDR7_END;

	all_reg->MMU_PRE_STRMADDR8_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR8_STR;
	all_reg->MMU_PRE_STRMADDR8_END = channel_cfg->all_reg.MMU_PRE_STRMADDR8_END;

	all_reg->MMU_PRE_STRMADDR9_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR9_STR;
	all_reg->MMU_PRE_STRMADDR9_END = channel_cfg->all_reg.MMU_PRE_STRMADDR9_END;

	all_reg->MMU_PRE_STRMADDR10_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR10_STR;
	all_reg->MMU_PRE_STRMADDR10_END = channel_cfg->all_reg.MMU_PRE_STRMADDR10_END;

	all_reg->MMU_PRE_STRMADDR11_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR11_STR;
	all_reg->MMU_PRE_STRMADDR11_END = channel_cfg->all_reg.MMU_PRE_STRMADDR11_END;

	all_reg->MMU_PRE_STRMADDR12_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR12_STR;
	all_reg->MMU_PRE_STRMADDR12_END = channel_cfg->all_reg.MMU_PRE_STRMADDR12_END;

	all_reg->MMU_PRE_STRMADDR13_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR13_STR;
	all_reg->MMU_PRE_STRMADDR13_END = channel_cfg->all_reg.MMU_PRE_STRMADDR13_END;

	all_reg->MMU_PRE_STRMADDR14_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR14_STR;
	all_reg->MMU_PRE_STRMADDR14_END = channel_cfg->all_reg.MMU_PRE_STRMADDR14_END;

	all_reg->MMU_PRE_STRMADDR15_STR = channel_cfg->all_reg.MMU_PRE_STRMADDR15_STR;
	all_reg->MMU_PRE_STRMADDR15_END = channel_cfg->all_reg.MMU_PRE_STRMADDR15_END;

	// smmu_mstr
	// mstr1 mstr2
	{
		all_reg->SMMU_MSTR_SMRX_1[15].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[15].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[15].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[15].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[16].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[16].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[16].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[16].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[17].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[17].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[17].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[17].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[18].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[18].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[18].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[18].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[19].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[19].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[19].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[19].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[20].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[20].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[20].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[20].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[21].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[21].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[21].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[21].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[22].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[22].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[22].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[22].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[23].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[23].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[23].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[23].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[24].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[24].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[24].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[24].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[25].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[25].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[25].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[25].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[26].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[26].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[26].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[26].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[27].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[27].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[27].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[27].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[28].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[28].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[28].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[28].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[29].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[29].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[29].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[29].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[39].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[39].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[39].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[39].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[40].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[40].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[40].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[40].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[41].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[41].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[41].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[41].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[42].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[42].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[42].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[42].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[43].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[43].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[43].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[43].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[44].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[44].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[44].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[44].bits.va_end;

		all_reg->SMMU_MSTR_SMRX_1[45].bits.va_str = channel_cfg->all_reg.SMMU_MSTR_SMRX_1[45].bits.va_str;
		all_reg->SMMU_MSTR_SMRX_2[45].bits.va_end = channel_cfg->all_reg.SMMU_MSTR_SMRX_2[45].bits.va_end;
	}
}

void veduhal_cfgreg_rcset(const struct encode_info *channel_cfg, uint32_t *reg_base)
{
	S_HEVC_AVC_REGS_TYPE *all_reg = NULL;
	all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;

	all_reg->VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32 = channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG0.u32;
	all_reg->VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32 = channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG1.u32;
	all_reg->VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32 = channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG2.u32;
	all_reg->VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32 = channel_cfg->all_reg.VEDU_QPG_CU_QP_DELTA_THRESH_REG3.u32;

	{
		U_VEDU_QPG_MADI_SWITCH_THR  D32;
		D32.bits.qpg_qp_madi_switch_thr = channel_cfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr;
		all_reg->VEDU_QPG_MADI_SWITCH_THR.u32 = D32.u32;
	}
	HI_DBG_VENC("----qpg_qp_madi_switch_thr:%d----", channel_cfg->all_reg.VEDU_QPG_MADI_SWITCH_THR.bits.qpg_qp_madi_switch_thr);
}

void vedu_hal_cfg_reg_simple(const struct encode_info *channel_cfg, int32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	uint32_t *reg_base = venc->ctx[core_id].reg_base;
	bool *first_cfg_flag = &venc->ctx[core_id].first_cfg_flag;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	{
		U_VEDU_QPG_MAX_MIN_QP  D32;
		D32.bits.qpg_min_qp = channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;
		D32.bits.qpg_max_qp = channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
		D32.bits.qpg_cu_qp_delta_enable_flag = 1;
		all_reg->VEDU_QPG_MAX_MIN_QP.u32 = D32.u32;
	}
	{
		U_VEDU_VCPI_MULTISLC  D32;
		D32.bits.vcpi_slice_size = channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slice_size;
		D32.bits.vcpi_slcspilt_mod = channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slcspilt_mod;
		D32.bits.vcpi_multislc_en = channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_multislc_en;
		all_reg->VEDU_VCPI_MULTISLC.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_MODE.bits.vcpi_idr_pic = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_idr_pic;

	all_reg->VEDU_VCPI_MODE.bits.vcpi_frame_type = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
	all_reg->VEDU_VCPI_MODE.bits.vcpi_ref_num = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;

	all_reg->VEDU_VCPI_QPCFG.bits.vcpi_frm_qp = channel_cfg->all_reg.VEDU_VCPI_QPCFG.bits.vcpi_frm_qp;
	all_reg->VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits = channel_cfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.bits.qpg_ave_lcu_bits;
	all_reg->VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits = channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;
	all_reg->VEDU_VCPI_MODE.bits.vcpi_sao_chroma = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
	all_reg->VEDU_VCPI_MODE.bits.vcpi_sao_luma = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;

	{
		U_VEDU_VCPI_MEM_CTRL_T16  D32;
		D32.bits.mem_ctrl_s = 0x5550;
		D32.bits.mem_ctrl_d1w2r = 0x0260;
		all_reg->VEDU_VCPI_MEM_CTRL_T16.u32 = D32.u32;
	}
	all_reg->VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail = channel_cfg->all_reg.VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail;

	veduhal_cfgreg_rcset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_intraset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_lambda_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_qpg_map_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_addr_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_slc_head_set(channel_cfg, reg_base);

	if (*first_cfg_flag == true) {
		vedu_hal_cfg_reg_smmu_set(channel_cfg, reg_base);
		*first_cfg_flag = false;
	}

	vedu_hal_cfg_reg_premmu_set(channel_cfg, reg_base);
}

void vedu_hal_cfg_reg(const struct encode_info *channel_cfg, int32_t core_id)
{
	venc_entry_t *venc = platform_get_drvdata(venc_get_device());

	uint32_t *reg_base = venc->ctx[core_id].reg_base;
	bool *first_cfg_flag = &venc->ctx[core_id].first_cfg_flag;
	S_HEVC_AVC_REGS_TYPE *all_reg = (S_HEVC_AVC_REGS_TYPE *)reg_base;
	uint32_t vcpi_protocol = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;

	// nosec reg
	{
		U_VEDU_VCPI_INTMASK  D32;
		D32.bits.vcpi_enable_ve_eop = 1;
		D32.bits.vcpi_enable_vedu_slice_end = 1;
		D32.bits.vcpi_enable_ve_buffull = 1;
		D32.bits.vcpi_enable_ve_pbitsover = 0;
		D32.bits.vcpi_enable_vedu_brkpt = 0;
		D32.bits.vcpi_enable_vedu_step = 0;
		D32.bits.vcpi_enable_vedu_timeout = 1;
		D32.bits.vcpi_enable_cfg_err = 0;
		all_reg->VEDU_VCPI_INTMASK.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_INTCLR D32;
		D32.bits.vcpi_clr_ve_eop = 0;
		D32.bits.vcpi_clr_vedu_slice_end = 0;
		D32.bits.vcpi_clr_ve_buffull = 0;
		D32.bits.vcpi_clr_ve_pbitsover = 0;
		D32.bits.vcpi_clr_vedu_brkpt = 0;
		D32.bits.vcpi_clr_vedu_step = 0;
		D32.bits.vcpi_clr_vedu_timeout = 0;
		D32.bits.vcpi_clr_cfg_err = 0;
		all_reg->VEDU_VCPI_INTCLR.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_CNTCLR.u32 = 0;

	all_reg->VEDU_VCPI_FRAMENO = 0;

	{
		U_VEDU_VCPI_BP_POS  D32;
		D32.bits.vcpi_bp_lcu_x = 0;
		D32.bits.vcpi_bp_lcu_y = 0;
		D32.bits.vcpi_bkp_en = 0;
		D32.bits.vcpi_dbgmod = 0;
		all_reg->VEDU_VCPI_BP_POS.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_TIMEOUT = 0;

	{
		U_VEDU_VCPI_MODE  D32;
		D32.bits.vcpi_vedsel = 0;
		D32.bits.vcpi_lcu_time_sel = 1;
		D32.bits.vcpi_protocol = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_protocol;
		D32.bits.vcpi_cfg_mode = 0;
		D32.bits.vcpi_slice_int_en = 1;
		D32.bits.vcpi_sao_luma = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_luma;
		D32.bits.vcpi_sao_chroma = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_sao_chroma;
		D32.bits.vcpi_rec_cmp_en = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_rec_cmp_en;
		D32.bits.vcpi_img_improve_en = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_img_improve_en;
		D32.bits.vcpi_frame_type = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_frame_type;
		D32.bits.vcpi_entropy_mode = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_entropy_mode;
		D32.bits.vcpi_long_term_refpic = 0;
		D32.bits.vcpi_ref_num = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_num;
		D32.bits.vcpi_2line_paral_enc = 0;
		D32.bits.vcpi_idr_pic = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_idr_pic;
		D32.bits.vcpi_pskip_en = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_pskip_en;
		D32.bits.vcpi_trans_mode = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_trans_mode;
		D32.bits.vcpi_blk8_inter = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_blk8_inter;
		D32.bits.vcpi_sobel_weight_en = 0;
		D32.bits.vcpi_high_speed_en = 0;
		D32.bits.vcpi_tiles_en = 0;
		D32.bits.vcpi_10bit_mode = 0;
		D32.bits.vcpi_lcu_size = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_lcu_size;
		D32.bits.vcpi_time_en = 0;
		D32.bits.vcpi_ref_cmp_en = channel_cfg->all_reg.VEDU_VCPI_MODE.bits.vcpi_ref_cmp_en;
		D32.bits.vcpi_refc_nload = 0;
		all_reg->VEDU_VCPI_MODE.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_TILE_SIZE.u32 = 0;

	all_reg->VEDU_VCPI_PICSIZE_PIX.u32 = channel_cfg->all_reg.VEDU_VCPI_PICSIZE_PIX.u32;

	{
		U_VEDU_VCPI_MULTISLC  D32;
		D32.bits.vcpi_slice_size = channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_slice_size;
		D32.bits.vcpi_slcspilt_mod = 1;
		D32.bits.vcpi_multislc_en = channel_cfg->all_reg.VEDU_VCPI_MULTISLC.bits.vcpi_multislc_en;
		all_reg->VEDU_VCPI_MULTISLC.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_QPCFG.u32 = channel_cfg->all_reg.VEDU_VCPI_QPCFG.u32;
	all_reg->VEDU_VCPI_DBLKCFG.u32 = channel_cfg->all_reg.VEDU_VCPI_DBLKCFG.u32;

	{
		U_VEDU_VCPI_LOW_POWER  D32;
		D32.bits.vcpi_intra_lowpow_en = 0;
		D32.bits.vcpi_fme_lowpow_en = channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_fme_lowpow_en;
		D32.bits.vcpi_ime_lowpow_en = channel_cfg->all_reg.VEDU_VCPI_LOW_POWER.bits.vcpi_ime_lowpow_en;
		D32.bits.vcpi_ddr_cross_idx = 0;
		D32.bits.vcpi_tqitq_gtck_en = 1;
		D32.bits.vcpi_mrg_gtck_en = 1;
		D32.bits.vcpi_fme_gtck_en = 1;
		D32.bits.vcpi_clkgate_en = 2;
		D32.bits.vcpi_mem_clkgate_en = 1;
		D32.bits.vcpi_hfbc_clkgate_en = 1;
		D32.bits.vcpi_ddr_cross_en = 0;
		D32.bits.vcpi_10bit_addr_mode = 0;
		all_reg->VEDU_VCPI_LOW_POWER.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_OUTSTD  D32;
		D32.bits.vcpi_r_outstanding = 0x1F;
		D32.bits.vcpi_w_outstanding = 0x7;
		all_reg->VEDU_VCPI_OUTSTD.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail = channel_cfg->all_reg.VEDU_VCPI_TMV_LOAD.bits.vcpi_tmv_wr_rd_avail;
	all_reg->VEDU_VCPI_CROSS_TILE_SLC.u32 = channel_cfg->all_reg.VEDU_VCPI_CROSS_TILE_SLC.u32;

	{
		U_VEDU_VCPI_MEM_CTRL  D32;
		D32.bits.vcpi_ema = 3;
		D32.bits.vcpi_emaw = 1;
		D32.bits.vcpi_emaa = 3;
		D32.bits.vcpi_emab = 3;
		all_reg->VEDU_VCPI_MEM_CTRL.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_INTRA_INTER_CU_EN  D32;
		D32.bits.vcpi_intra_cu_en = channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_intra_cu_en;
		D32.bits.vcpi_ipcm_en = 1;
		D32.bits.vcpi_intra_h264_cutdiag = 1;
		D32.bits.vcpi_fme_cu_en = channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_fme_cu_en;
		D32.bits.vcpi_mrg_cu_en = channel_cfg->all_reg.VEDU_VCPI_INTRA_INTER_CU_EN.bits.vcpi_mrg_cu_en;
		all_reg->VEDU_VCPI_INTRA_INTER_CU_EN.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PRE_JUDGE_EXT_EN  D32;
		D32.bits.vcpi_iblk_pre_en = channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_iblk_pre_en;
		D32.bits.vcpi_pblk_pre_en = channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_pblk_pre_en;
		D32.bits.vcpi_force_inter = channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_force_inter;
		D32.bits.vcpi_pintra_inter_flag_disable = channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_pintra_inter_flag_disable;
		D32.bits.vcpi_ext_edge_en = channel_cfg->all_reg.VEDU_VCPI_PRE_JUDGE_EXT_EN.bits.vcpi_ext_edge_en;
		all_reg->VEDU_VCPI_PRE_JUDGE_EXT_EN.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PRE_JUDGE_COST_THR  D32;
		D32.bits.vcpi_iblk_pre_cost_thr = dist_protocol(vcpi_protocol, 0x1F4, 0x100);
		D32.bits.vcpi_pblk_pre_cost_thr = 0x64;
		all_reg->VEDU_VCPI_PRE_JUDGE_COST_THR.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_IBLK_PRE_MV_THR  D32;
		D32.bits.vcpi_iblk_pre_mv_dif_thr0 = 0xF;
		D32.bits.vcpi_iblk_pre_mv_dif_thr1 = 0xF;
		D32.bits.vcpi_iblk_pre_mvx_thr = dist_protocol(vcpi_protocol, 5, 0xC0);
		D32.bits.vcpi_iblk_pre_mvy_thr = dist_protocol(vcpi_protocol, 5, 0xA0);
		all_reg->VEDU_VCPI_IBLK_PRE_MV_THR.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PME_PARAM  D32;
		D32.bits.vcpi_move_sad_en = 0;
		D32.bits.vcpi_pblk_pre_mvx_thr = dist_protocol(vcpi_protocol, 2, 0x10);
		D32.bits.vcpi_pblk_pre_mvy_thr = dist_protocol(vcpi_protocol, 2, 0x10);
		all_reg->VEDU_VCPI_PME_PARAM.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PIC_STRONG_EN  D32;
		D32.bits.vcpi_skin_en = channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_skin_en;
		D32.bits.vcpi_strong_edge_en = channel_cfg->all_reg.VEDU_VCPI_PIC_STRONG_EN.bits.vcpi_strong_edge_en;
		D32.bits.vcpi_still_en = 0;
		D32.bits.vcpi_skin_close_angle = 0;
		D32.bits.vcpi_rounding_sobel_en = 0;
		all_reg->VEDU_VCPI_PIC_STRONG_EN.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PINTRA_THRESH0  D32;
		D32.bits.vcpi_pintra_pu16_amp_th = dist_protocol(vcpi_protocol, 0x51, 0x80);
		D32.bits.vcpi_pintra_pu32_amp_th = dist_protocol(vcpi_protocol, 0x5E, 0x80);
		D32.bits.vcpi_pintra_pu64_amp_th = 0x80;
		all_reg->VEDU_VCPI_PINTRA_THRESH0.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PINTRA_THRESH1  D32;
		D32.bits.vcpi_pintra_pu16_std_th = dist_protocol(vcpi_protocol, 0x66, 0x80);
		D32.bits.vcpi_pintra_pu32_std_th = dist_protocol(vcpi_protocol, 0x4C, 0x80);
		all_reg->VEDU_VCPI_PINTRA_THRESH1.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_PINTRA_THRESH2  D32;
		D32.bits.vcpi_pintra_pu16_angel_cost_th = 0x80;
		D32.bits.vcpi_pintra_pu32_angel_cost_th = 0x80;
		all_reg->VEDU_VCPI_PINTRA_THRESH2.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_CLKDIV_ENABLE  D32;
		D32.bits.vcpi_clkdiv_en = 0;
		D32.bits.vcpi_down_freq_en = 0;
		all_reg->VEDU_VCPI_CLKDIV_ENABLE.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_SW_L0_SIZE.u32 = channel_cfg->all_reg.VEDU_VCPI_SW_L0_SIZE.u32;
	all_reg->VEDU_VCPI_SW_L1_SIZE.u32 = channel_cfg->all_reg.VEDU_VCPI_SW_L1_SIZE.u32;

	{
		U_VEDU_VCPI_CROP_START  D32;
		D32.bits.vcpi_crop_xstart = 0;
		D32.bits.vcpi_crop_ystart = 0;
		all_reg->VEDU_VCPI_CROP_START.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_ORI_PICSIZE.u32 = channel_cfg->all_reg.VEDU_VCPI_ORI_PICSIZE.u32;

	{
		U_VEDU_VCPI_INTRA32_LOW_POWER  D32;
		D32.bits.vcpi_intra32_low_power_thr = 0x400;
		D32.bits.vcpi_intra32_low_power_en = channel_cfg->all_reg.VEDU_VCPI_INTRA32_LOW_POWER.bits.vcpi_intra32_low_power_en;
		all_reg->VEDU_VCPI_INTRA32_LOW_POWER.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_INTRA16_LOW_POWER  D32;
		D32.bits.vcpi_intra16_low_power_thr = 0x200;
		D32.bits.vcpi_intra16_low_power_en = channel_cfg->all_reg.VEDU_VCPI_INTRA16_LOW_POWER.bits.vcpi_intra16_low_power_en;
		all_reg->VEDU_VCPI_INTRA16_LOW_POWER.u32 = D32.u32;
	}

	{
		U_VEDU_VCPI_INTRA_REDUCE_RDO_NUM  D32;
		D32.bits.vcpi_intra_reduce_rdo_num_thr = 0x100;
		D32.bits.vcpi_intra_reduce_rdo_num_en = 0;
		all_reg->VEDU_VCPI_INTRA_REDUCE_RDO_NUM.u32 = D32.u32;
	}

	all_reg->VEDU_VCPI_NOFORCEZERO.u32 = 0;
	all_reg->VEDU_VCTRL_LCU_TARGET_BIT.bits.vctrl_lcu_target_bit = 0x64;
	all_reg->VEDU_VCTRL_NARROW_THRESHOLD.bits.vctrl_narrow_tile_width = 3;

	{
		U_VEDU_VCTRL_LCU_BASELINE  D32;
		D32.bits.vctrl_lcu_performance_baseline = dist_protocol(vcpi_protocol, 0x186A, 0x1806);
		all_reg->VEDU_VCTRL_LCU_BASELINE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_norm32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x100, 0);
		D32.bits.vctrl_norm32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingoffset32x32 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_NORM_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_norm16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x40, 0);
		D32.bits.vctrl_norm16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x3, 0);
		D32.bits.vctrl_roundingoffset16x16 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_NORM_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_NORM_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingdegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundinglowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundinglowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_NORM_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_NORM_ENG_DENOISE  D32;
		D32.bits.vctrl_norm_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_norm_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_norm_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_norm_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_norm_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_norm_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_NORM_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_skin32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x20, 0);
		D32.bits.vctrl_skin32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingskinoffset32x32 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_SKIN_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_skin16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_skin16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingskinoffset16x16 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_SKIN_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SKIN_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingskinmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingskindegreethresh = dist_protocol(vcpi_protocol, 0x5A, 0);
		D32.bits.vctrl_roundingskinforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingskinac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingskinac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingskinlowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingskinlowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_SKIN_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SKIN_ENG_DENOISE  D32;
		D32.bits.vctrl_skin_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_skin_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_skin_engsum_32 = 0;
		D32.bits.vctrl_skin_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_skin_engsum_16 = 0;
		D32.bits.vctrl_skin_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_SKIN_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_hedge32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x20, 0);
		D32.bits.vctrl_hedge32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingedgeoffset32x32 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_HEDGE_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_hedge16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_hedge16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingedgeoffset16x16 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_HEDGE_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGE_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingedgemechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingedgedegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingedgeforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingedgeac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingedgeac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingedgelowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingedgelowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_HEDGE_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGE_ENG_DENOISE  D32;
		D32.bits.vctrl_stredge_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_stredge_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_stredge_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_stredge_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_stredge_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_stredge_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_HEDGE_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_edgemov32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x20, 0);
		D32.bits.vctrl_edgemov32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingedgemovoffset32x32 = 0;
		all_reg->VEDU_VCTRL_HEDGEMOV_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_edgemov16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_edgemov16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingedgemovoffset16x16 = 0;
		all_reg->VEDU_VCTRL_HEDGEMOV_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingedgemovmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingedgemovdegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingedgemovforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingedgemovac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingedgemovac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingedgemovlowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingedgemovlowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_HEDGEMOV_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_HEDGEMOV_ENG_DENOISE  D32;
		D32.bits.vctrl_edgemov_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_edgemov_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_edgemov_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_edgemov_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_edgemov_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_edgemov_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_HEDGEMOV_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_static32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_static32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingstilloffset32x32 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_STATIC_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_static16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_static16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingstilloffset16x16 = dist_protocol(vcpi_protocol, 0x7, 0);
		all_reg->VEDU_VCTRL_STATIC_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_STATIC_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingstillmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingstilldegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingstillforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingstillac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingstillac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingstilllowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingstilllowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_STATIC_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_STATIC_ENG_DENOISE  D32;
		D32.bits.vctrl_still_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_still_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_still_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_still_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_still_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_still_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_STATIC_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_sobelstr32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_sobelstr32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingsobelstroffset32x32 = 0;
		all_reg->VEDU_VCTRL_SOBELSTR_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_sobelstr16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelstr16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingsobelstroffset16x16 = 0;
		all_reg->VEDU_VCTRL_SOBELSTR_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELSTR_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingsobelstrmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingsobelstrdegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingsobelstrforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingsobelstrac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingsobelstrac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingsobelstrlowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingsobelstrlowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_SOBELSTR_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELSTR_ENG_DENOISE  D32;
		D32.bits.vctrl_sobelstr_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_sobelstr_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_sobelstr_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelstr_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelstr_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_sobelstr_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_SOBELSTR_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE  D32;
		D32.bits.vctrl_sobelweak32_coeff_protect_num = dist_protocol(vcpi_protocol, 0x10, 0);
		D32.bits.vctrl_sobelweak32_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingsobelweakoffset32x32 = 0;
		all_reg->VEDU_VCTRL_SOBELWEAK_TR32X32_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE  D32;
		D32.bits.vctrl_sobelweak16_coeff_protect_num = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelweak16_tr1_denois_max_num = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingsobelweakoffset16x16 = 0;
		all_reg->VEDU_VCTRL_SOBELWEAK_TR16X16_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE  D32;
		D32.bits.vctrl_roundingsobelweakmechanism = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_roundingsobelweakdegreethresh = dist_protocol(vcpi_protocol, 0x21, 0);
		D32.bits.vctrl_roundingsobelweakforcezeroresidthresh = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingsobelweakac32sum = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_roundingsobelweakac16sum = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_roundingsobelweaklowfreqacblk32 = dist_protocol(vcpi_protocol, 0x4, 0);
		D32.bits.vctrl_roundingsobelweaklowfreqacblk16 = dist_protocol(vcpi_protocol, 0x4, 0);
		all_reg->VEDU_VCTRL_SOBELWEAK_COEFF_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_SOBELWEAK_ENG_DENOISE  D32;
		D32.bits.vctrl_sobelwk_isolate_ac_enable = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_sobelwk_force_zero_cnt = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_sobelwk_engsum_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelwk_engcnt_32 = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.vctrl_sobelwk_engsum_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		D32.bits.vctrl_sobelwk_engcnt_16 = dist_protocol(vcpi_protocol, 0x6, 0);
		all_reg->VEDU_VCTRL_SOBELWEAK_ENG_DENOISE.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_0  D32;
		D32.bits.vctrl_norm_intra_cu4_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_norm_intra_cu8_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_norm_intra_cu16_rdcost_offset = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_norm_intra_cu32_rdcost_offset = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_strmov_intra_cu4_rdcost_offset = 0;
		D32.bits.vctrl_strmov_intra_cu8_rdcost_offset = 0;
		D32.bits.vctrl_strmov_intra_cu16_rdcost_offset = 0;
		D32.bits.vctrl_strmov_intra_cu32_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_0.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_1  D32;
		D32.bits.vctrl_skin_intra_cu4_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_skin_intra_cu8_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_skin_intra_cu16_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_skin_intra_cu32_rdcost_offset = dist_protocol(vcpi_protocol, 0xf, 0);
		D32.bits.vctrl_sobel_str_intra_cu4_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_intra_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_intra_cu16_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_intra_cu32_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_1.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_INTRA_RDO_FACTOR_2  D32;
		D32.bits.vctrl_hedge_intra_cu4_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_hedge_intra_cu8_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_hedge_intra_cu16_rdcost_offset = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.vctrl_hedge_intra_cu32_rdcost_offset = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.vctrl_sobel_tex_intra_cu4_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_intra_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_intra_cu16_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_intra_cu32_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_INTRA_RDO_FACTOR_2.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_0  D32;
		D32.bits.vctrl_norm_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_norm_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_norm_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_norm_mrg_cu64_rdcost_offset = 0;
		D32.bits.vctrl_strmov_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_strmov_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_strmov_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_strmov_mrg_cu64_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_MRG_RDO_FACTOR_0.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_1  D32;
		D32.bits.vctrl_skin_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_skin_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_skin_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_skin_mrg_cu64_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_mrg_cu64_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_MRG_RDO_FACTOR_1.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_MRG_RDO_FACTOR_2  D32;
		D32.bits.vctrl_hedge_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_hedge_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_hedge_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_hedge_mrg_cu64_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_mrg_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_mrg_cu16_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_mrg_cu32_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_mrg_cu64_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_MRG_RDO_FACTOR_2.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_0  D32;
		D32.bits.vctrl_norm_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_norm_fme_cu16_rdcost_offset = 0;
		D32.bits.vctrl_norm_fme_cu32_rdcost_offset = 0;
		D32.bits.vctrl_norm_fme_cu64_rdcost_offset = 0;
		D32.bits.vctrl_strmov_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_strmov_fme_cu16_rdcost_offset = 0;
		D32.bits.vctrl_strmov_fme_cu32_rdcost_offset = 0;
		D32.bits.vctrl_strmov_fme_cu64_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_FME_RDO_FACTOR_0.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_1  D32;
		D32.bits.vctrl_skin_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_skin_fme_cu16_rdcost_offset = 0;
		D32.bits.vctrl_skin_fme_cu32_rdcost_offset = 0;
		D32.bits.vctrl_skin_fme_cu64_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_str_fme_cu16_rdcost_offset = dist_protocol(vcpi_protocol, 0xA, 0);
		D32.bits.vctrl_sobel_str_fme_cu32_rdcost_offset = dist_protocol(vcpi_protocol, 0xA, 0);
		D32.bits.vctrl_sobel_str_fme_cu64_rdcost_offset = dist_protocol(vcpi_protocol, 0xA, 0);
		all_reg->VEDU_VCTRL_FME_RDO_FACTOR_1.u32 = D32.u32;
	}

	{
		U_VEDU_VCTRL_FME_RDO_FACTOR_2  D32;
		D32.bits.vctrl_hedge_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_hedge_fme_cu16_rdcost_offset = 0;
		D32.bits.vctrl_hedge_fme_cu32_rdcost_offset = 0;
		D32.bits.vctrl_hedge_fme_cu64_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_fme_cu8_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_fme_cu16_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_fme_cu32_rdcost_offset = 0;
		D32.bits.vctrl_sobel_tex_fme_cu64_rdcost_offset = 0;
		all_reg->VEDU_VCTRL_FME_RDO_FACTOR_2.u32 = D32.u32;
	}

	{
		U_VEDU_CURLD_GCFG  D32;
		D32.bits.curld_osd0_global_en = 0;
		D32.bits.curld_osd1_global_en = 0;
		D32.bits.curld_osd2_global_en = 0;
		D32.bits.curld_osd3_global_en = 0;
		D32.bits.curld_osd4_global_en = 0;
		D32.bits.curld_osd5_global_en = 0;
		D32.bits.curld_osd6_global_en = 0;
		D32.bits.curld_osd7_global_en = 0;
		D32.bits.curld_col2gray_en = 0;
		D32.bits.curld_clip_en = 0;
		D32.bits.curld_read_interval = 1;
		D32.bits.curld_lowdly_en = 0;
		D32.bits.curld_osd_rgbfmt = 0;
		all_reg->VEDU_CURLD_GCFG.u32 = D32.u32;
	}

	all_reg->VEDU_CURLD_OSD01_ALPHA.u32 = 0;
	all_reg->VEDU_CURLD_OSD23_ALPHA.u32 = 0;
	all_reg->VEDU_CURLD_OSD45_ALPHA.u32 = 0;
	all_reg->VEDU_CURLD_OSD67_ALPHA.u32 = 0;
	all_reg->VEDU_CURLD_OSD_GALPHA0.u32 = 0;
	all_reg->VEDU_CURLD_OSD_GALPHA1.u32 = 0;
	all_reg->VEDU_CURLD_OSD0_ADDR = 0;
	all_reg->VEDU_CURLD_OSD1_ADDR = 0;
	all_reg->VEDU_CURLD_OSD2_ADDR = 0;
	all_reg->VEDU_CURLD_OSD3_ADDR = 0;
	all_reg->VEDU_CURLD_OSD4_ADDR = 0;
	all_reg->VEDU_CURLD_OSD5_ADDR = 0;
	all_reg->VEDU_CURLD_OSD6_ADDR = 0;
	all_reg->VEDU_CURLD_OSD7_ADDR = 0;
	all_reg->VEDU_CURLD_OSD01_STRIDE.u32 = 0;
	all_reg->VEDU_CURLD_OSD23_STRIDE.u32 = 0;
	all_reg->VEDU_CURLD_OSD45_STRIDE.u32 = 0;
	all_reg->VEDU_CURLD_OSD67_STRIDE.u32 = 0;

	{
		U_VEDU_CURLD_CLIP_THR  D32;
		D32.bits.curld_clip_luma_min = 16;
		D32.bits.curld_clip_luma_max = 0xEB;
		D32.bits.curld_clip_chrm_min = 16;
		D32.bits.curld_clip_chrm_max = dist_protocol(vcpi_protocol, 0xF0, 240);
		all_reg->VEDU_CURLD_CLIP_THR.u32 = D32.u32;
	}

	{
		U_VEDU_CURLD_HOR_FILTER  D32;
		D32.bits.curld_filter_h0 = 0x1;
		D32.bits.curld_filter_h1 = 0x1;
		D32.bits.curld_filter_h2 = 0x1;
		D32.bits.curld_filter_h3 = 0x1;
		D32.bits.curld_filter_hrnd = channel_cfg->all_reg.VEDU_CURLD_HOR_FILTER.bits.curld_filter_hrnd;
		D32.bits.curld_filter_hshift = 0x2;
		all_reg->VEDU_CURLD_HOR_FILTER.u32 = D32.u32;
	}

	{
		U_VEDU_CURLD_VER_FILTER  D32;
		D32.bits.curld_filter_v0 = 0x1;
		D32.bits.curld_filter_v1 = 0x1;
		D32.bits.curld_filter_v2 = 0x1;
		D32.bits.curld_filter_v3 = 0x1;
		D32.bits.curld_filter_vrnd = channel_cfg->all_reg.VEDU_CURLD_VER_FILTER.bits.curld_filter_vrnd;
		D32.bits.curld_filter_vshift = 0x2;
		all_reg->VEDU_CURLD_VER_FILTER.u32 = D32.u32;
	}

	all_reg->VEDU_CURLD_ARGB_YUV_0COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_0COEFF.u32;
	all_reg->VEDU_CURLD_ARGB_YUV_1COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_1COEFF.u32;
	all_reg->VEDU_CURLD_ARGB_YUV_2COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_2COEFF.u32;
	all_reg->VEDU_CURLD_ARGB_YUV_3COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_3COEFF.u32;
	all_reg->VEDU_CURLD_ARGB_YUV_4COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_4COEFF.u32;
	all_reg->VEDU_CURLD_ARGB_YUV_5COEFF.u32 = channel_cfg->all_reg.VEDU_CURLD_ARGB_YUV_5COEFF.u32;

	{
		U_VEDU_CURLD_ARGB_YUV_6COEFF  D32;
		D32.bits.vcpi_rgb_rndcr = 128;
		D32.bits.vcpi_rgb_rndcb = 128;
		all_reg->VEDU_CURLD_ARGB_YUV_6COEFF.u32 = D32.u32;
	}

	{
		U_VEDU_CURLD_ARGB_CLIP  D32;
		D32.bits.vcpi_rgb_clpmin = 16;
		D32.bits.vcpi_rgb_clpmax = 235;
		D32.bits.vcpi_rgb_clip_en = 0;
		all_reg->VEDU_CURLD_ARGB_CLIP.u32 = D32.u32;
	}

	all_reg->VEDU_CURLD_NARROW_EN.u32 = channel_cfg->all_reg.VEDU_CURLD_NARROW_EN.u32;

	{
		U_VEDU_PME_SW_ADAPT_EN  D32;
		D32.bits.pme_l0_psw_adapt_en = dist_protocol(vcpi_protocol, 1, 0);
		D32.bits.pme_l1_psw_adapt_en = dist_protocol(vcpi_protocol, 1, 0);
		all_reg->VEDU_PME_SW_ADAPT_EN.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SW_THR0  D32;
		D32.bits.pme_l0_psw_thr0 = 0x14;
		D32.bits.pme_l1_psw_thr0 = 0x14;
		all_reg->VEDU_PME_SW_THR0.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SW_THR1  D32;
		D32.bits.pme_l0_psw_thr1 = 0x32;
		D32.bits.pme_l1_psw_thr1 = 0x32;
		all_reg->VEDU_PME_SW_THR1.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SW_THR2  D32;
		D32.bits.pme_l0_psw_thr2 = 0x96;
		D32.bits.pme_l1_psw_thr2 = 0x96;
		all_reg->VEDU_PME_SW_THR2.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SKIP_PRE  D32;
		D32.bits.pme_skipblk_pre_cost_thr = dist_protocol(vcpi_protocol, 0x1E, 0);
		D32.bits.pme_skipblk_pre_en = 0;
		all_reg->VEDU_PME_SKIP_PRE.u32 = D32.u32;
	}

	{
		U_VEDU_PME_TR_WEIGHTX  D32;
		D32.bits.pme_tr_weightx_0 = dist_protocol(vcpi_protocol, 0x4, 0x10);
		D32.bits.pme_tr_weightx_1 = dist_protocol(vcpi_protocol, 0x8, 0x20);
		D32.bits.pme_tr_weightx_2 = dist_protocol(vcpi_protocol, 0xC, 0x30);
		all_reg->VEDU_PME_TR_WEIGHTX.u32 = D32.u32;
	}

	{
		U_VEDU_PME_TR_WEIGHTY  D32;
		D32.bits.pme_tr_weighty_0 = dist_protocol(vcpi_protocol, 0x4, 0x10);
		D32.bits.pme_tr_weighty_1 = dist_protocol(vcpi_protocol, 0x8, 0x20);
		D32.bits.pme_tr_weighty_2 = dist_protocol(vcpi_protocol, 0xC, 0x30);
		all_reg->VEDU_PME_TR_WEIGHTY.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SR_WEIGHT  D32;
		D32.bits.pme_sr_weight_0 = 6;
		D32.bits.pme_sr_weight_1 = 4;
		D32.bits.pme_sr_weight_2 = 2;
		D32.bits.pme_pskip_strongedge_madi_thr = 0x14;
		D32.bits.pme_pskip_strongedge_madi_times = 0x03;
		all_reg->VEDU_PME_SR_WEIGHT.u32 = D32.u32;
	}

	{
		U_VEDU_PME_INTRABLK_DET  D32;
		D32.bits.pme_intrablk_det_cost_thr0 = dist_protocol(vcpi_protocol, 0x174, 0x64);
		D32.bits.pme_pskip_mvy_consistency_thr = 0;
		D32.bits.pme_pskip_mvx_consistency_thr = 0;
		all_reg->VEDU_PME_INTRABLK_DET.u32 = D32.u32;
	}

	{
		U_VEDU_PME_INTRABLK_DET_THR  D32;
		D32.bits.pme_intrablk_det_mv_dif_thr1 = dist_protocol(vcpi_protocol, 0x2, 0x0F);
		D32.bits.pme_intrablk_det_mv_dif_thr0 = dist_protocol(vcpi_protocol, 0x2, 0x0F);
		D32.bits.pme_intrablk_det_mvy_thr = dist_protocol(vcpi_protocol, 4, 0x10);
		D32.bits.pme_intrablk_det_mvx_thr = dist_protocol(vcpi_protocol, 4, 0x10);
		all_reg->VEDU_PME_INTRABLK_DET_THR.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SKIN_THR  D32;
		D32.bits.pme_skin_u_max_thr = 0x7F;
		D32.bits.pme_skin_u_min_thr = 0x64;
		D32.bits.pme_skin_v_max_thr = 0xA0;
		D32.bits.pme_skin_v_min_thr = 0x87;
		all_reg->VEDU_PME_SKIN_THR.u32 = D32.u32;
	}

	{
		U_VEDU_PME_INTRA_LOWPOW  D32;
		D32.bits.pme_intra16_madi_thr = dist_protocol(vcpi_protocol, 0xA, 0);
		D32.bits.pme_intra32_madi_thr = dist_protocol(vcpi_protocol, 0x14, 0);
		D32.bits.pme_intra_lowpow_en = channel_cfg->all_reg.VEDU_PME_INTRA_LOWPOW.bits.pme_intra_lowpow_en;
		D32.bits.pme_inter_first = 0;
		all_reg->VEDU_PME_INTRA_LOWPOW.u32 = D32.u32;
	}

	{
		U_VEDU_PME_IBLK_COST_THR  D32;
		D32.bits.pme_iblk_pre_cost_thr_h264 = dist_protocol(vcpi_protocol, 0x02FF, 0x400);
		D32.bits.pme_intrablk_det_cost_thr1 = dist_protocol(vcpi_protocol, 0x0200, 0xC8);
		all_reg->VEDU_PME_IBLK_COST_THR.u32 = D32.u32;
	}

	{
		U_VEDU_PME_STRONG_EDGE  D32;
		D32.bits.pme_skin_num = dist_protocol(vcpi_protocol, 0x80, 0x30);
		D32.bits.pme_strong_edge_thr = dist_protocol(vcpi_protocol, 0x1E, 0x28);
		D32.bits.pme_strong_edge_cnt = dist_protocol(vcpi_protocol, 0x3, 0x06);
		D32.bits.pme_still_scene_thr = dist_protocol(vcpi_protocol, 1, 0);
		all_reg->VEDU_PME_STRONG_EDGE.u32 = D32.u32;
	}

	{
		U_VEDU_PME_LARGE_MOVE_THR  D32;
		D32.bits.pme_move_scene_thr = dist_protocol(vcpi_protocol, 0x3, 0);
		D32.bits.pme_move_sad_thr = dist_protocol(vcpi_protocol, 0x200, 0);
		all_reg->VEDU_PME_LARGE_MOVE_THR.u32 = D32.u32;
	}

	{
		U_VEDU_PME_INTER_STRONG_EDGE  D32;
		D32.bits.pme_interdiff_max_min_madi_abs = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.pme_interdiff_max_min_madi_times = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.pme_interstrongedge_madi_thr = dist_protocol(vcpi_protocol, 0x3C, 0);
		all_reg->VEDU_PME_INTER_STRONG_EDGE.u32 = D32.u32;
	}

	{
		all_reg->VEDU_PME_NEW_COST.bits.pme_cost_lamda0 = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda0;
		all_reg->VEDU_PME_NEW_COST.bits.pme_cost_lamda1 = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda1;
		all_reg->VEDU_PME_NEW_COST.bits.pme_cost_lamda2 = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda2;
		all_reg->VEDU_PME_NEW_COST.bits.pme_cost_lamda_en = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_cost_lamda_en;
		all_reg->VEDU_PME_NEW_COST.bits.pme_mvp3median_en = channel_cfg->all_reg.VEDU_PME_NEW_COST.bits.pme_mvp3median_en;
	}

	all_reg->VEDU_PME_WINDOW_SIZE0_L0.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L0.u32;
	all_reg->VEDU_PME_WINDOW_SIZE1_L0.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE1_L0.u32;
	all_reg->VEDU_PME_WINDOW_SIZE2_L0.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE2_L0.u32;
	all_reg->VEDU_PME_WINDOW_SIZE3_L0.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE3_L0.u32;
	all_reg->VEDU_PME_WINDOW_SIZE0_L1.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE0_L1.u32;
	all_reg->VEDU_PME_WINDOW_SIZE1_L1.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE1_L1.u32;
	all_reg->VEDU_PME_WINDOW_SIZE2_L1.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE2_L1.u32;
	all_reg->VEDU_PME_WINDOW_SIZE3_L1.u32 = channel_cfg->all_reg.VEDU_PME_WINDOW_SIZE3_L1.u32;
	all_reg->VEDU_PME_COST_OFFSET.u32 = 0;

	{
		U_VEDU_PME_SAFE_CFG  D32;
		D32.bits.pme_safe_line = channel_cfg->all_reg.VEDU_PME_SAFE_CFG.bits.pme_safe_line;
		D32.bits.pme_safe_line_val = 0;
		all_reg->VEDU_PME_SAFE_CFG.u32 = D32.u32;
	}

	all_reg->VEDU_PME_IBLK_REFRESH.u32 = 0;
	all_reg->VEDU_PME_IBLK_REFRESH_NUM.u32 = 0;

	{
		U_VEDU_PME_QPG_RC_THR0  D32;
		D32.bits.pme_madi_dif_thr = dist_protocol(vcpi_protocol, 0, 0x5);
		D32.bits.pme_cur_madi_dif_thr = dist_protocol(vcpi_protocol, 0, 0x5);
		all_reg->VEDU_PME_QPG_RC_THR0.u32 = D32.u32;
	}

	{
		U_VEDU_PME_QPG_RC_THR1  D32;
		D32.bits.pme_min_sad_thr_offset = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.pme_min_sad_thr_gain = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.pme_smooth_madi_thr = dist_protocol(vcpi_protocol, 0, 0x2);
		D32.bits.pme_min_sad_thr_offset_cur = dist_protocol(vcpi_protocol, 0x8, 0);
		D32.bits.pme_min_sad_thr_gain_cur = dist_protocol(vcpi_protocol, 0x8, 0);
		all_reg->VEDU_PME_QPG_RC_THR1.u32 = D32.u32;
	}

	{
		U_VEDU_PME_LOW_LUMA_THR  D32;
		D32.bits.pme_low_luma_thr = 0;
		D32.bits.pme_low_luma_madi_thr = dist_protocol(vcpi_protocol, 0x5, 0x4);
		D32.bits.pme_high_luma_thr = dist_protocol(vcpi_protocol, 0xF, 0x3C);
		all_reg->VEDU_PME_LOW_LUMA_THR.u32 = D32.u32;
	}

	{
		U_VEDU_PME_PBLK_PRE1  D32;
		D32.bits.pme_pblk_pre_mv_dif_thr1 = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.pme_pblk_pre_mv_dif_thr0 = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.pme_pblk_pre_mv_dif_cost_thr = dist_protocol(vcpi_protocol, 0xC8, 0);
		all_reg->VEDU_PME_PBLK_PRE1.u32 = D32.u32;
	}

	{
		U_VEDU_PME_CHROMA_FLAT  D32;
		D32.bits.pme_flat_v_thr_high = dist_protocol(vcpi_protocol, 0x85, 0);
		D32.bits.pme_flat_v_thr_low = dist_protocol(vcpi_protocol, 0x7D, 0);
		D32.bits.pme_flat_u_thr_high = dist_protocol(vcpi_protocol, 0x85, 0);
		D32.bits.pme_flat_u_thr_low = dist_protocol(vcpi_protocol, 0x7D, 0);
		all_reg->VEDU_PME_CHROMA_FLAT.u32 = D32.u32;
	}

	{
		U_VEDU_PME_LUMA_FLAT  D32;
		D32.bits.pme_flat_pmemv_thr = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.pme_flat_luma_madi_thr = dist_protocol(vcpi_protocol, 0x2, 0);
		D32.bits.pme_flat_low_luma_thr = dist_protocol(vcpi_protocol, 0x28, 0);
		D32.bits.pme_flat_high_luma_thr = dist_protocol(vcpi_protocol, 0x64, 0);
		all_reg->VEDU_PME_LUMA_FLAT.u32 = D32.u32;
	}

	{
		U_VEDU_PME_MADI_FLAT  D32;
		D32.bits.pme_flat_pmesad_thr = dist_protocol(vcpi_protocol, 0x40, 0);
		D32.bits.pme_flat_icount_thr = dist_protocol(vcpi_protocol, 0xC8, 0);
		D32.bits.pme_flat_region_cnt = dist_protocol(vcpi_protocol, 0xD, 0);
		D32.bits.pme_flat_madi_times = dist_protocol(vcpi_protocol, 0x1, 0);
		all_reg->VEDU_PME_MADI_FLAT.u32 = D32.u32;
	}

	{
		U_VEDU_PME_SKIP_LARGE_RES  D32;
		D32.bits.pme_skip_sad_thr_offset = 0x8;
		D32.bits.pme_skip_sad_thr_gain = 0x8;
		D32.bits.pme_skip_large_res_det = 0x0;
		all_reg->VEDU_PME_SKIP_LARGE_RES.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_MAX_MIN_QP  D32;
		D32.bits.qpg_min_qp = channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_min_qp;
		D32.bits.qpg_max_qp = channel_cfg->all_reg.VEDU_QPG_MAX_MIN_QP.bits.qpg_max_qp;
		D32.bits.qpg_cu_qp_delta_enable_flag = 1;
		all_reg->VEDU_QPG_MAX_MIN_QP.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_ROW_TARGET_BITS  D32;
		D32.bits.qpg_qp_delta = channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_qp_delta;
		D32.bits.qpg_row_target_bits = channel_cfg->all_reg.VEDU_QPG_ROW_TARGET_BITS.bits.qpg_row_target_bits;
		all_reg->VEDU_QPG_ROW_TARGET_BITS.u32 = D32.u32;
	}

	all_reg->VEDU_QPG_AVERAGE_LCU_BITS.u32 = channel_cfg->all_reg.VEDU_QPG_AVERAGE_LCU_BITS.u32;

	{
		U_VEDU_QPG_LOWLUMA  D32;
		D32.bits.qpg_lowluma_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_lowluma_max_qp = dist_protocol(vcpi_protocol, 0xF, 0x33);
		D32.bits.qpg_lowluma_qp_delta = dist_protocol(vcpi_protocol, 0xA, 0x3);
		all_reg->VEDU_QPG_LOWLUMA.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_HEDGE  D32;
		D32.bits.qpg_hedge_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_hedge_max_qp = 0x33;
		D32.bits.qpg_hedge_qp_delta = dist_protocol(vcpi_protocol, 0x0, 0x4);
		all_reg->VEDU_QPG_HEDGE.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_HEDGE_MOVE  D32;
		D32.bits.qpg_hedge_move_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_hedge_move_max_qp = 0x33;
		D32.bits.qpg_hedge_move_qp_delta = dist_protocol(vcpi_protocol, 0x0, 0x5);
		all_reg->VEDU_QPG_HEDGE_MOVE.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_LARGE_MOVE  D32;
		D32.bits.qpg_large_move_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_large_move_max_qp = 0x33;
		D32.bits.qpg_large_move_qp_delta = dist_protocol(vcpi_protocol, 0x0, 0x0);
		all_reg->VEDU_QPG_LARGE_MOVE.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_SKIN  D32;
		D32.bits.qpg_skin_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_skin_max_qp = channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_max_qp;
		D32.bits.qpg_skin_qp_delta = channel_cfg->all_reg.VEDU_QPG_SKIN.bits.qpg_skin_qp_delta;
		all_reg->VEDU_QPG_SKIN.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_INTRA_DET  D32;
		D32.bits.qpg_intra_det_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_intra_det_max_qp = 0x33;
		D32.bits.qpg_intra_det_qp_delta = dist_protocol(vcpi_protocol, 0x4, 0x3);
		all_reg->VEDU_QPG_INTRA_DET.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_H264_SMOOTH  D32;
		D32.bits.qpg_h264_smooth_min_qp = dist_protocol(vcpi_protocol, 0x0, 0xA);
		D32.bits.qpg_h264_smooth_max_qp = dist_protocol(vcpi_protocol, 0, 0x2D);
		D32.bits.qpg_h264_smooth_qp_delta = dist_protocol(vcpi_protocol, 0, 0x5);
		D32.bits.qpg_h264_smooth_qp_delta1 = dist_protocol(vcpi_protocol, 0, 0x3);
		all_reg->VEDU_QPG_H264_SMOOTH.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_DELTA_LEVEL  D32;
		D32.bits.qpg_qp_delta_level_0 = 1;
		D32.bits.qpg_qp_delta_level_1 = 1;
		D32.bits.qpg_qp_delta_level_2 = 1;
		D32.bits.qpg_qp_delta_level_3 = 1;
		D32.bits.qpg_qp_delta_level_4 = 1;
		D32.bits.qpg_qp_delta_level_5 = 1;
		D32.bits.qpg_qp_delta_level_6 = 1;
		D32.bits.qpg_qp_delta_level_7 = 1;
		D32.bits.qpg_qp_delta_level_8 = 1;
		D32.bits.qpg_qp_delta_level_9 = 1;
		D32.bits.qpg_qp_delta_level_10 = 1;
		D32.bits.qpg_qp_delta_level_11 = 1;
		D32.bits.qpg_qp_delta_level_12 = 1;
		D32.bits.qpg_qp_delta_level_13 = 1;
		D32.bits.qpg_qp_delta_level_14 = 1;
		D32.bits.qpg_qp_delta_level_15 = 1;
		all_reg->VEDU_QPG_DELTA_LEVEL.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_CU32_DELTA  D32;
		D32.bits.qpg_cu32_delta_low = 0xC;
		D32.bits.qpg_cu32_delta_high = 0xC;
		all_reg->VEDU_QPG_CU32_DELTA.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_LAMBDA_MODE  D32;
		D32.bits.qpg_lambda_qp_offset = dist_protocol(vcpi_protocol, 0x0, 0x1A);
		D32.bits.qpg_rdo_lambda_choose_mode = 0;
		D32.bits.qpg_lambda_inter_stredge_en = dist_protocol(vcpi_protocol, 1, 0);
		all_reg->VEDU_QPG_LAMBDA_MODE.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_QP_RESTRAIN  D32;
		D32.bits.qpg_qp_restrain_madi_thr = dist_protocol(vcpi_protocol, 0xA, 0x8);
		D32.bits.qpg_qp_restrain_en = dist_protocol(vcpi_protocol, 0x1, 0);
		D32.bits.qpg_qp_restrain_mode = 0;
		D32.bits.qpg_qp_restrain_delta_blk16 = dist_protocol(vcpi_protocol, 0x4, 1);
		D32.bits.qpg_qp_restrain_delta_blk32 = dist_protocol(vcpi_protocol, 0x5, 3);
		all_reg->VEDU_QPG_QP_RESTRAIN.u32 = D32.u32;
	}

	all_reg->VEDU_QPG_CU_MIN_SAD_THRESH_0 = 0x0C080401;
	all_reg->VEDU_QPG_CU_MIN_SAD_THRESH_1 = 0xB4825A3C;

	{
		U_VEDU_QPG_CU_MIN_SAD_REG  D32;
		D32.bits.qpg_min_sad_level = dist_protocol(vcpi_protocol, 0x5555, 0x5595);
		D32.bits.qpg_low_min_sad_mode = 0;
		D32.bits.qpg_high_min_sad_mode = 0;
		D32.bits.qpg_min_sad_madi_en = dist_protocol(vcpi_protocol, 0x1, 0x0);
		D32.bits.qpg_min_sad_qp_restrain_en = 0;
		all_reg->VEDU_QPG_CU_MIN_SAD_REG.u32 = D32.u32;
	}

	{
		U_VEDU_QPG_FLAT_REGION  D32;
		D32.bits.qpg_flat_region_qp_delta = dist_protocol(vcpi_protocol, 5, 0);
		D32.bits.qpg_flat_region_max_qp = 0x33;
		D32.bits.qpg_flat_region_min_qp = dist_protocol(vcpi_protocol, 0xF, 0);
		D32.bits.vcpi_cu32_use_cu16_mean_en = 0;
		all_reg->VEDU_QPG_FLAT_REGION.u32 = D32.u32;
	}

	{
		U_VEDU_IME_INTER_MODE  D32;
		D32.bits.ime_layer3to2_en = 0;
		D32.bits.ime_inter8x8_en = dist_protocol(vcpi_protocol, 0, 1);
		D32.bits.ime_flat_region_force_low3layer = 0;
		D32.bits.ime_high3pre_en = 0;
		D32.bits.ime_intra4_lowpow_en = 0;
		all_reg->VEDU_IME_INTER_MODE.u32 = D32.u32;
	}

	all_reg->VEDU_IME_RDOCFG.u32 = 0;

	{
		U_VEDU_IME_FME_LPOW_THR  D32;
		D32.bits.ime_lowpow_fme_thr0 = 0xA;
		D32.bits.ime_lowpow_fme_thr1 = dist_protocol(vcpi_protocol, 0x10, 0x14);
		all_reg->VEDU_IME_FME_LPOW_THR.u32 = D32.u32;
	}

	{
		U_VEDU_IME_LAYER3TO2_THR  D32;
		D32.bits.ime_layer3to2_thr0 = 0x70;
		D32.bits.ime_layer3to2_thr1 = 0x10E;
		all_reg->VEDU_IME_LAYER3TO2_THR.u32 = D32.u32;
	}

	{
		U_VEDU_IME_LAYER3TO2_THR1  D32;
		D32.bits.ime_layer3to2_cost_diff_thr = 0x100;
		all_reg->VEDU_IME_LAYER3TO2_THR1.u32 = D32.u32;
	}

	{
		U_VEDU_IME_LAYER3TO1_THR  D32;
		D32.bits.ime_layer3to1_en = 0;
		D32.bits.ime_layer3to1_pu64_madi_thr = 5;
		all_reg->VEDU_IME_LAYER3TO1_THR.u32 = D32.u32;
	}

	{
		U_VEDU_IME_LAYER3TO1_THR1  D32;
		D32.bits.ime_layer3to1_pu32_cost_thr = 0xBB8;
		D32.bits.ime_layer3to1_pu64_cost_thr = 0x1000;
		all_reg->VEDU_IME_LAYER3TO1_THR1.u32 = D32.u32;
	}

	all_reg->VEDU_FME_BIAS_COST0.u32 = 0;
	all_reg->VEDU_FME_BIAS_COST1.u32 = 0;

	{
		U_VEDU_FME_PU64_LWP  D32;
		D32.bits.fme_pu64_lwp_flag = channel_cfg->all_reg.VEDU_FME_PU64_LWP.bits.fme_pu64_lwp_flag;
		all_reg->VEDU_FME_PU64_LWP.u32 = D32.u32;
	}

	{
		U_VEDU_MRG_FORCE_ZERO_EN  D32;
		D32.bits.mrg_force_zero_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_zero_en;
		D32.bits.mrg_force_y_zero_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_y_zero_en;
		D32.bits.mrg_force_u_zero_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_u_zero_en;
		D32.bits.mrg_force_v_zero_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.mrg_force_v_zero_en;
		D32.bits.fme_lpw_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.fme_lpw_en;
		D32.bits.dct4_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.dct4_en;
		D32.bits.force_adapt_en = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.force_adapt_en;
		D32.bits.rqt_bias_weight = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.rqt_bias_weight;
		D32.bits.fme_lpw_th = channel_cfg->all_reg.VEDU_MRG_FORCE_ZERO_EN.bits.fme_lpw_th;
		all_reg->VEDU_MRG_FORCE_ZERO_EN.u32 = D32.u32;
	}

	all_reg->VEDU_MRG_FORCE_SKIP_EN.u32 = 0;
	all_reg->VEDU_MRG_BIAS_COST0.u32 = 0;
	all_reg->VEDU_MRG_BIAS_COST1.u32 = 0;
	all_reg->VEDU_MRG_ABS_OFFSET0.u32 = 0;
	all_reg->VEDU_MRG_ABS_OFFSET1.u32 = 0;
	all_reg->VEDU_MRG_ADJ_WEIGHT.u32 = 0;
	all_reg->VEDU_INTRA_CFG.u32 = 0;

	{
		U_VEDU_INTRA_SMOOTH  D32;
		D32.bits.intra_smooth = dist_protocol(vcpi_protocol, 1, 0);
		all_reg->VEDU_INTRA_SMOOTH.u32 = D32.u32;
	}

	{
		U_VEDU_INTRA_BIT_WEIGHT  D32;
		D32.bits.intra_bit_weight = dist_protocol(vcpi_protocol, 0xD, 0);
		all_reg->VEDU_INTRA_BIT_WEIGHT.u32 = D32.u32;
	}

	all_reg->VEDU_INTRA_RDO_COST_OFFSET_0.u32 = 0;
	all_reg->VEDU_INTRA_RDO_COST_OFFSET_1.u32 = 0;
	all_reg->VEDU_INTRA_NO_DC_COST_OFFSET_0.u32 = 0;
	all_reg->VEDU_INTRA_NO_DC_COST_OFFSET_1.u32 = 0;
	all_reg->VEDU_INTRA_CHNL4_ANG_0EN.u32 = dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
	all_reg->VEDU_INTRA_CHNL4_ANG_1EN.u32 = (7 & 0x7);
	all_reg->VEDU_INTRA_CHNL8_ANG_0EN.u32 = dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
	all_reg->VEDU_INTRA_CHNL8_ANG_1EN.u32 = (7 & 0x7);
	all_reg->VEDU_INTRA_CHNL16_ANG_0EN.u32 = dist_protocol(vcpi_protocol, (0xffffffff | 0x2), (0xffffffff | 0x4));
	all_reg->VEDU_INTRA_CHNL16_ANG_1EN.u32 = (7 & 0x7);
	all_reg->VEDU_INTRA_CHNL32_ANG_0EN.u32 = (0xffffffff | 0x2);
	all_reg->VEDU_INTRA_CHNL32_ANG_1EN.u32 = (7 & 0x7);
	all_reg->VEDU_INTRA_RDO_COST_OFFSET_3.u32 = 0;

	{
		U_VEDU_PMV_TMV_EN  D32;
		D32.bits.pmv_tmv_en = dist_protocol(vcpi_protocol, 1, 0);
		all_reg->VEDU_PMV_TMV_EN.u32 = D32.u32;
	}

	{
		U_VEDU_TQITQ_DEADZONE  D32;
		D32.bits.tqitq_deadzone_intra_slice = 0xAB;
		D32.bits.tqitq_deadzone_inter_slice = 0x55;
		all_reg->VEDU_TQITQ_DEADZONE.u32 = D32.u32;
	}

	{
		U_VEDU_SEL_OFFSET_STRENGTH  D32;
		D32.bits.sel_offset_strength = dist_protocol(vcpi_protocol, 0, 2);
		all_reg->VEDU_SEL_OFFSET_STRENGTH.u32 = D32.u32;
	}

	{
		U_VEDU_SEL_CU32_DC_AC_TH_OFFSET  D32;
		D32.bits.sel_cu32_dc_ac_th_offset = dist_protocol(vcpi_protocol, 0, 1);
		all_reg->VEDU_SEL_CU32_DC_AC_TH_OFFSET.u32 = D32.u32;
	}

	{
		U_VEDU_SEL_CU32_QP_TH  D32;
		D32.bits.sel_cu32_qp0_th = dist_protocol(vcpi_protocol, 0x0, 0x26);
		D32.bits.sel_cu32_qp1_th = dist_protocol(vcpi_protocol, 0x0, 0x21);
		all_reg->VEDU_SEL_CU32_QP_TH.u32 = D32.u32;
	}

	{
		U_VEDU_SEL_RES_DC_AC_TH  D32;
		D32.bits.sel_res16_luma_dc_th = dist_protocol(vcpi_protocol, 0, 3);
		D32.bits.sel_res16_chroma_dc_th = dist_protocol(vcpi_protocol, 0, 2);
		D32.bits.sel_res16_luma_ac_th = dist_protocol(vcpi_protocol, 0, 3);
		D32.bits.sel_res16_chroma_ac_th = dist_protocol(vcpi_protocol, 0, 2);
		D32.bits.sel_res32_luma_dc_th = dist_protocol(vcpi_protocol, 0, 4);
		D32.bits.sel_res32_chroma_dc_th = dist_protocol(vcpi_protocol, 0, 3);
		D32.bits.sel_res32_luma_ac_th = dist_protocol(vcpi_protocol, 0, 4);
		D32.bits.sel_res32_chroma_ac_th = dist_protocol(vcpi_protocol, 0, 3);
		all_reg->VEDU_SEL_RES_DC_AC_TH.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_WAIT_TIM_OUT  D32;
		D32.bits.vcpi_wtmax = 0xFF;
		D32.bits.vcpi_rtmax = 0xFF;
		all_reg->VEDU_EMAR_WAIT_TIM_OUT.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_RCH_RPT_TH0  D32;
		D32.bits.vcpi_ch00_rrmax = 0xF;
		D32.bits.vcpi_ch01_rrmax = 0xF;
		D32.bits.vcpi_ch02_rrmax = 0xF;
		D32.bits.vcpi_ch03_rrmax = 0xF;
		D32.bits.vcpi_ch04_rrmax = 0xF;
		D32.bits.vcpi_ch05_rrmax = 0xF;
		all_reg->VEDU_EMAR_RCH_RPT_TH0.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_RCH_RPT_TH1  D32;
		D32.bits.vcpi_ch06_rrmax = 0xF;
		D32.bits.vcpi_ch07_rrmax = 0xF;
		D32.bits.vcpi_ch08_rrmax = 0xF;
		D32.bits.vcpi_ch09_rrmax = 0xF;
		D32.bits.vcpi_ch10_rrmax = 0xF;
		D32.bits.vcpi_ch11_rrmax = 0xF;
		all_reg->VEDU_EMAR_RCH_RPT_TH1.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_RCH_RPT_TH2  D32;
		D32.bits.vcpi_ch12_rrmax = 0xF;
		all_reg->VEDU_EMAR_RCH_RPT_TH2.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_WCH_RPT_TH0  D32;
		D32.bits.vcpi_ch00_wrmax = 0xF;
		D32.bits.vcpi_ch01_wrmax = 0xF;
		D32.bits.vcpi_ch02_wrmax = 0xF;
		D32.bits.vcpi_ch03_wrmax = 0xF;
		D32.bits.vcpi_ch04_wrmax = 0xF;
		D32.bits.vcpi_ch05_wrmax = 0xF;
		all_reg->VEDU_EMAR_WCH_RPT_TH0.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_WCH_RPT_TH1  D32;
		D32.bits.vcpi_ch06_wrmax = 0xF;
		D32.bits.vcpi_ch07_wrmax = 0xF;
		D32.bits.vcpi_ch08_wrmax = 0xF;
		D32.bits.vcpi_ch09_wrmax = 0xF;
		D32.bits.vcpi_ch10_wrmax = 0xF;
		D32.bits.vcpi_ch11_wrmax = 0xF;
		all_reg->VEDU_EMAR_WCH_RPT_TH1.u32 = D32.u32;
	}

	{
		U_VEDU_EMAR_WCH_RPT_TH2  D32;
		D32.bits.vcpi_ch12_wrmax = 0xF;
		D32.bits.vcpi_ch13_wrmax = 0xF;
		D32.bits.vcpi_ch14_wrmax = 0xF;
		all_reg->VEDU_EMAR_WCH_RPT_TH2.u32 = D32.u32;
	}

	all_reg->VEDU_EMAR_SCRAMBLE_TYPE.u32 = 0;
	all_reg->VEDU_PACK_SYNTAX_CONFIG = 0;

	{
		U_VEDU_PACK_CU_PARAMETER  D32;
		D32.bits.pack_vcpi2cu_tq_bypass_enabled_flag = 0;
		D32.bits.pack_vcpi2cu_qp_min_cu_size = 2;
		all_reg->VEDU_PACK_CU_PARAMETER.u32 = D32.u32;
	}

	{
		U_VEDU_PACK_PCM_PARAMETER  D32;
		D32.bits.pack_vcpi2pu_log2_min_ipcm_cbsizey = 3;
		D32.bits.pack_vcpi2pu_log2_max_ipcm_cbsizey = 3;
		all_reg->VEDU_PACK_PCM_PARAMETER.u32 = D32.u32;
	}

	{
		U_VEDU_PACK_TF_SKIP_FLAG  D32;
		D32.bits.pack_vcpi2res_tf_skip_enabled_flag = 0;
		all_reg->VEDU_PACK_TF_SKIP_FLAG.u32 = D32.u32;
	}

	{
		U_VEDU_VLCST_PTBITS_EN  D32;
		D32.bits.vlcst_ptbits_en = 0;
		all_reg->VEDU_VLCST_PTBITS_EN.u32 = D32.u32;
	}

	all_reg->VEDU_VLCST_PTBITS = 0;

	veduhal_cfgreg_rcset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_intraset(channel_cfg, reg_base);

	vedu_hal_cfg_reg_lambda_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_qpg_map_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_addr_set(channel_cfg, reg_base);

	vedu_hal_cfg_reg_slc_head_set(channel_cfg, reg_base);

	// ------------------------------MMU MAIN REG CFG---------------------------------------------
	// MMU ABOUT SETTING
	// MST INT_NS

	if (*first_cfg_flag == true) {
		vedu_hal_cfg_reg_smmu_set(channel_cfg, reg_base);
		*first_cfg_flag = false;
	}

	vedu_hal_cfg_reg_premmu_set(channel_cfg, reg_base);
}

