/*
 * hi6405_high_res_cfg.h
 *
 * high resistance headset cfg
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __HI6405_HIGH_RES_CFG_H__
#define __HI6405_HIGH_RES_CFG_H__

#include "linux/hisi/hi64xx/hi6405_type.h"
#include "linux/hisi/hi64xx/hi6405_regs.h"

const struct reg_seq_config enable_res_det_table[] = {
	{ { IMP_DET_CTRL0_REG, 0x1 << IMP_DET_EN_OFFSET,
		0x1 << IMP_DET_EN_OFFSET, true }, 0, 0 },
	{ { IMP_DET_CTRL1_REG, 0xff, 0x17, false }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL12_REG, 0xff, 0x0a, false }, 0, 0 },
	{ { MISC_CLK_EN_REG, 0xff, 0x53, false }, 0, 0 },
	{ { DACL_PRE_PGA_CTRL0_REG, 0x1 << DACL_PRE_PGA_BYPASS_OFFSET,
		0, true }, 0, 0 },
	{ { DACL_PRE_PGA_CTRL1_REG, 0xff, 0x5c, false }, 0, 0 },
	{ { DACL_POST_PGA_CTRL0_REG, 0x1 << DACL_POST_PGA_BYPASS_OFFSET,
		0, true }, 0, 0 },
	{ { DACL_POST_PGA_CTRL1_REG, 0xff, 0x88, false }, 0, 0 },
	{ { DACR_PRE_PGA_CTRL0_REG, 0x1 << DACR_PRE_PGA_BYPASS_OFFSET,
		0, true }, 0, 0 },
	{ { DACR_PRE_PGA_CTRL1_REG, 0xff, 0x5c, false }, 0, 0 },
	{ { DACR_POST_PGA_CTRL0_REG, 0x1 << DACR_POST_PGA_BYPASS_OFFSET,
		0, true }, 0, 0 },
	{ { DACR_POST_PGA_CTRL1_REG, 0xff, 0x88, false }, 0, 0 },
	{ { I2S_DSPIF_CLK_EN_REG, 0x1 << CLK_IMP_DET_EN_OFFSET,
		0x1 << CLK_IMP_DET_EN_OFFSET, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_012, 0x1 << MBHD_PD_MBHD_VTN, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_HSD_EN_BIT,
		0x1 << MBHD_HSD_EN_BIT, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_ECO_EN_BIT, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_PULLOUT_POP_EN_BIT, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_073, 0xf, 0x2, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_073, 0x1 << HSMICB2_DSCHG, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_013, 0x1 << HS_MICB_PD_BIT, 0, true }, 0, 0 },
};

const struct reg_seq_config disable_res_det_table[] = {
	{ { CODEC_ANA_RWREG_077, 0x3 << SAR_INPUT_SEL_BIT, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_PULLOUT_POP_EN_BIT,
		0x1 << MBHD_PULLOUT_POP_EN_BIT, true }, 0, 0 },
	{ { MISC_CLK_EN_REG, 0xff, 0, false }, 0, 0 },
	{ { IMP_DET_CTRL0_REG, 0x1 << IMP_DET_EN_OFFSET, 0, true }, 0, 0 },
	{ { SC_CODEC_MUX_CTRL12_REG, 0xff, 0, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_073, 0xf << HSMICB_ADJ,
		0xb << HSMICB_ADJ, true }, 0, 0 },
	{ { DACL_PRE_PGA_CTRL1_REG, 0xff, 0x6e, false }, 0, 0 },
	{ { DACR_PRE_PGA_CTRL1_REG, 0xff, 0x6e, false }, 0, 0 },
	{ { I2S_DSPIF_CLK_EN_REG, 0x1 << CLK_IMP_DET_EN_OFFSET, 0, true }, 0, 0 },
	{ { DACL_PRE_PGA_CTRL0_REG, 0x1 << DACL_PRE_PGA_BYPASS_OFFSET,
		0x1 << DACL_PRE_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_PGA_CTRL0_REG, 0x1 << DACR_PRE_PGA_BYPASS_OFFSET,
		0x1 << DACR_PRE_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACL_POST_PGA_CTRL0_REG, 0x1 << DACL_POST_PGA_BYPASS_OFFSET,
		0x1 << DACL_POST_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACR_POST_PGA_CTRL0_REG, 0x1 << DACR_POST_PGA_BYPASS_OFFSET,
		0x1 << DACR_POST_PGA_BYPASS_OFFSET, true }, 0, 0 },
};

const struct reg_seq_config enable_path_table[] = {
	{ { DACL_PRE_PGA_CTRL0_REG, 1 << DACL_PRE_PGA_BYPASS_OFFSET,
		1 << DACL_PRE_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACL_PRE_PGA_CTRL0_REG, 0x7 << DACL_PRE_PGA_CFG_OFFSET,
		0x4 << DACL_PRE_PGA_CFG_OFFSET, true }, 0, 0 },
	{ { DACR_PRE_PGA_CTRL0_REG, 1 << DACR_PRE_PGA_BYPASS_OFFSET,
		1 << DACR_PRE_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACL_POST_PGA_CTRL0_REG, 1 << DACL_POST_PGA_BYPASS_OFFSET,
		1 << DACL_POST_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { DACL_POST_PGA_CTRL0_REG, 0x7 << DACL_POST_PGA_CFG_OFFSET,
		0x4 << DACL_POST_PGA_CFG_OFFSET, true }, 0, 0 },
	{ { DACR_POST_PGA_CTRL0_REG, 1 << DACR_POST_PGA_BYPASS_OFFSET,
		1 << DACR_POST_PGA_BYPASS_OFFSET, true }, 0, 0 },
	{ { HP_SDM_L_CTRL7_REG, 0xff, 0xF2, false }, 0, 0 },
	{ { HP_SDM_L_CTRL8_REG, 0xff, 0x28, false }, 0, 0 },
	{ { HP_SDM_L_CTRL9_REG, 0x3 << HP_SDM_L_CELLSEL_MODE_OFFSET,
		0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL12_REG, 1 << HP_SDM_L_CALT_VLD_OFFSET,
		1 << HP_SDM_L_CALT_VLD_OFFSET, true }, 0, 0 },
	{ { HP_SDM_R_CTRL7_REG, 0xff, 0xF2, false }, 0, 0 },
	{ { HP_SDM_R_CTRL8_REG, 0xff, 0x28, false }, 0, 0 },
	{ { HP_SDM_R_CTRL9_REG, 0x3 << HP_SDM_R_CELLSEL_MODE_OFFSET,
		0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL12_REG, 1 << HP_SDM_R_CALT_VLD_OFFSET,
		1 << HP_SDM_R_CALT_VLD_OFFSET, true }, 0, 0 },
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << CLK_DACL_UP16_EN_OFFSET,
		0x1 << CLK_DACL_UP16_EN_OFFSET, true }, 0, 0 },
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << CLK_DACR_UP16_EN_OFFSET,
		0x1 << CLK_DACR_UP16_EN_OFFSET, true }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, 0x1 << HP_SDM_R_CLK_EN_OFFSET,
		0x1 << HP_SDM_R_CLK_EN_OFFSET, true }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, 0x1 << HP_SDM_L_CLK_EN_OFFSET,
		0x1 << HP_SDM_L_CLK_EN_OFFSET, true }, 0, 0 },
	{ { ADC_DAC_CLK_EN_REG, 0x1 << CLK_DACL_EN_OFFSET,
		0x1 << CLK_DACL_EN_OFFSET, true }, 0, 0 },
	{ { ADC_DAC_CLK_EN_REG, 0x1 << CLK_DACR_EN_OFFSET,
		0x1 << CLK_DACR_EN_OFFSET, true }, 0, 0 },
};

const struct reg_seq_config disable_path_table[] = {
	{ { ADC_DAC_CLK_EN_REG, 0x1 << CLK_DACL_EN_OFFSET, 0, true }, 0, 0 },
	{ { ADC_DAC_CLK_EN_REG, 0x1 << CLK_DACR_EN_OFFSET, 0, true }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, 0x1 << HP_SDM_R_CLK_EN_OFFSET, 0, true }, 0, 0 },
	{ { DAC_DP_CLK_EN_2_REG, 0x1 << HP_SDM_L_CLK_EN_OFFSET, 0, true }, 0, 0 },
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << CLK_DACL_UP16_EN_OFFSET,
		0, true }, 0, 0 },
	{ { S1_MIXER_EQ_CLK_EN_REG, 0x1 << CLK_DACR_UP16_EN_OFFSET,
		0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL12_REG, 0x1 << HP_SDM_R_CALT_VLD_OFFSET, 0, true }, 0, 0 },
	{ { HP_SDM_R_CTRL8_REG, 0xff, 0, false }, 0, 0 },
	{ { HP_SDM_R_CTRL7_REG, 0xff, 0, false }, 0, 0 },
	{ { HP_SDM_L_CTRL12_REG, 0x1 << HP_SDM_L_CALT_VLD_OFFSET, 0, true }, 0, 0 },
	{ { HP_SDM_L_CTRL8_REG, 0xff, 0, false }, 0, 0 },
	{ { HP_SDM_L_CTRL7_REG, 0xff, 0, false }, 0, 0 },
};

const struct reg_seq_config enable_get_res_table[] = {
	{ { CODEC_ANA_RWREG_065, 0x1 << 3, 0x1 << 3, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_077, 0x3 << SAR_INPUT_SEL_BIT,
		0x2 << SAR_INPUT_SEL_BIT, true }, 0, 0 },
};

const struct reg_seq_config hp_impdet_vpout_table[] = {
	{ { CODEC_ANA_RWREG_065, 0x1 << 5, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_065, 0x1, 0x1, true }, MSLEEP, 10 },
	{ { CODEC_ANA_RWREG_065, 0x1, 0, true }, MSLEEP, 10 },
	{ { CODEC_ANA_RWREG_065, 0x1 << 5, 0x1 << 5, true }, 0, 0 },
};

const struct reg_seq_config disable_get_res_table[] = {
	{ { DACL_POST_PGA_CTRL1_REG, 0xff, 0x88, false }, 0, 0 },
	{ { DACR_POST_PGA_CTRL1_REG, 0xff, 0x88, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_065, 0x1 << 5, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_065, 0x3 << 1, 0, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_065, 0x1, 0x1, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_065, 0x1 << 3, 0, true }, 0, 0 },
};

const struct reg_seq_config enable_res_calib_table[] = {
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_RES_CALIB_EN_BIT,
		0x1 << MBHD_RES_CALIB_EN_BIT, true }, 0, 0 },
	{ { CODEC_ANA_RWREG_077, 0x3 << SAR_INPUT_SEL_BIT, 0, true }, 0, 0 },
};

const struct reg_seq_config disable_res_calib_table[] = {
	{ { CODEC_ANA_RWREG_080, 0x1 << MBHD_RES_CALIB_EN_BIT, 0, true }, 0, 0 },
};
#endif /* __HI6405_HIGH_RES_CFG_H__ */

