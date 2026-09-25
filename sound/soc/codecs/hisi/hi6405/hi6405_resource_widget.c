/*
 * hi6405_resource_widget.c -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "hi6405_resource_widget.h"

#include <sound/core.h>
#include <sound/tlv.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"
#include "linux/hisi/hi64xx/hi6405.h"
#include "linux/hisi/hi64xx/hi6405_regs.h"
#include "linux/hisi/hi64xx/hi6405_type.h"
#include "huawei_platform/power/vsys_switch/vsys_switch.h"

static void mic_control_sc_frequency(int event,
	struct hi6405_platform_data *platform_data)
{
	struct hi6405_board_cfg board_cfg = platform_data->board_config;

	if (!board_cfg.mic_control_sc_freq_enable)
		return;
	if (platform_data->is_madswitch_on || platform_data->is_callswitch_on)
		return;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		vsys_switch_set_sc_frequency_mode(0); /* fixed mode */
		AUDIO_LOGI("micbias1 event 1, set sc frequency mode 0");
		break;
	case SND_SOC_DAPM_POST_PMD:
		vsys_switch_set_sc_frequency_mode(1); /* auto mode */
		AUDIO_LOGI("micbias1 event 0, set sc frequency mode 1");
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static const struct mic_mux_node hsmic_mux_info[HSMIC_MUX_TOTAL_MIC] = {
	{
		.type = HS_MIC,
		.reg_value = 0x01
	},
	{
		.type = MAIN_MIC,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node auxmic_mux_info[AUXMIC_MUX_TOTAL_MIC] = {
	{
		.type = AUX_MIC,
		.reg_value = 0x01
	},
	{
		.type = MAIN_MIC,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic3_mux_info[MIC3_MUX_TOTAL_MIC] = {
	{
		.type = MIC3,
		.reg_value = 0x01
	},
	{
		.type = MIC4,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic4_mux_info[MIC4_MUX_TOTAL_MIC] = {
	{
		.type = MIC4,
		.reg_value = 0x01
	},
	{
		.type = MIC3,
		.reg_value = 0x04
	}
};

static const struct mic_mux_node mic5_mux_info[MIC5_MUX_TOTAL_MIC] = {
	{
		.type = MIC5,
		.reg_value = 0x01
	}
};

static enum hi64xx_pll_type pll_for_reg_access(struct snd_soc_codec *codec, unsigned int reg)
{
	if ((reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_DIG + ADDR_DIG_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_DIG + ADDR_DIG_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + ADDR_CFG_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + ADDR_CFG_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO1 + ADDR_GPIO1_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO1 + ADDR_GPIO1_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO2 + ADDR_GPIO2_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO2 + ADDR_GPIO2_OFFSET_END))) {
		return PLL_HIGH;
	} else {
		return PLL_NONE;
	}
}

static enum codec_mic_pin_type get_one_mic_mux_state(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int offset,
	const struct mic_mux_node *mic_mux, unsigned int total_mic)
{
	unsigned int raw_mux_reg;
	unsigned int mux_reg;
	unsigned int i;
	enum codec_mic_pin_type ret = NONE;

	raw_mux_reg = snd_soc_read(codec, reg);
	mux_reg = (raw_mux_reg >> offset) & LOWEST_FOUR_BITS_MASK;
	for (i = 0; i < total_mic; i++) {
		if (mux_reg == mic_mux[i].reg_value) {
			ret = mic_mux[i].type;
			break;
		}
	}
	return ret;
}

static void get_all_mic_mux_state(struct snd_soc_codec *codec,
	enum codec_mic_pin_type *mic_mux)
{
	/* HS Mic PGA MAX */
	mic_mux[ADC0] = get_one_mic_mux_state(codec,
		ANA_HSMIC_MUX_AND_PGA, HSMIC_MUX_OFFSET,
		hsmic_mux_info, HSMIC_MUX_TOTAL_MIC);
	/* Aux Mic PGA MAX */
	mic_mux[ADC1] = get_one_mic_mux_state(codec,
		ANA_AUXMIC_MUX_AND_PGA, AUXMIC_MUX_OFFSET,
		auxmic_mux_info, AUXMIC_MUX_TOTAL_MIC);
	/* Mic3 PGA MAX */
	mic_mux[ADC2] = get_one_mic_mux_state(codec,
		ANA_MIC3_MUX_AND_PGA, MIC3_MUX_OFFSET,
		mic3_mux_info, MIC3_MUX_TOTAL_MIC);
	/* Mic4 PGA MAX */
	mic_mux[ADC3] = get_one_mic_mux_state(codec,
		ANA_MIC4_MUX_AND_PGA, MIC4_MUX_OFFSET,
		mic4_mux_info, MIC4_MUX_TOTAL_MIC);
	/* Mic5 PGA MAX */
	mic_mux[ADC4] = get_one_mic_mux_state(codec,
		ANA_MIC5_MUX_AND_PGA, MIC5_MUX_OFFSET,
		mic5_mux_info, MIC5_MUX_TOTAL_MIC);
}

static unsigned int get_calib_value_by_mic_mux(
	struct hi6405_platform_data *data, enum codec_mic_pin_type mic_mux)
{
	unsigned int calib_value;

	switch (mic_mux) {
	case HS_MIC:
		calib_value = MIC_CALIB_NONE_VALUE;
		break;
	case MAIN_MIC:
		calib_value = data->mic_calib_value[CODEC_CALIB_MAIN_MIC];
		break;
	case AUX_MIC:
		calib_value = data->mic_calib_value[CODEC_CALIB_AUX_MIC];
		break;
	case MIC3:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC3];
		break;
	case MIC4:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC4];
		break;
	case MIC5:
		calib_value = data->mic_calib_value[CODEC_CALIB_MIC5];
		break;
	default:
		calib_value = MIC_CALIB_NONE_VALUE;
		break;
	}

	/* in case the calib value is malicious modified */
	if (calib_value < MIC_CALIB_MIN_VALUE ||
		calib_value > MIC_CALIB_MAX_VALUE)
		calib_value = MIC_CALIB_NONE_VALUE;

	return calib_value;
}

static void set_calib_by_adc_mux(struct hi6405_platform_data *data,
	unsigned int adc_type, enum codec_mic_pin_type *mic_mux,
	unsigned int reg)
{
	unsigned int calib_value;

	if (adc_type < TOTAL_MIC_ADC_TYPE) {
		calib_value = get_calib_value_by_mic_mux(data,
			mic_mux[adc_type]);
		snd_soc_write(data->codec, reg, calib_value);
		AUDIO_LOGI("adc %d set mic %u calib value as: %u\n",
			adc_type, mic_mux[adc_type], calib_value);
	}
}

static void set_mic_calib_value(struct hi6405_platform_data *data)
{
	unsigned int adc_type;
	unsigned int adc0_mux;
	unsigned int adc1_mux;
	unsigned int adc2_mux;
	enum codec_mic_pin_type mic_mux[TOTAL_MIC_ADC_TYPE] = { NONE };

	/* ADC0L/R mux reg */
	adc0_mux = snd_soc_read(data->codec, SC_CODEC_MUX_CTRL15_REG);
	/* ADC1L/R mux reg */
	adc1_mux = snd_soc_read(data->codec, SC_CODEC_MUX_CTRL16_REG);
	/* ADC2L mux reg */
	adc2_mux = snd_soc_read(data->codec, SC_CODEC_MUX_CTRL17_REG);

	get_all_mic_mux_state(data->codec, mic_mux);

	/* ADC0L */
	adc_type = (adc0_mux >> ADC0L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC0L_PGA_CTRL1_REG);
	/* ADC0R */
	adc_type = (adc0_mux >> ADC0R_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC0R_PGA_CTRL1_REG);
	/* ADC1L */
	adc_type = (adc1_mux >> ADC1L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC1L_PGA_CTRL1_REG);
	/* ADC1R */
	adc_type = (adc1_mux >> ADC1R_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC1R_PGA_CTRL1_REG);
	/* ADC2L */
	adc_type = (adc2_mux >> ADC2L_DIN_SEL_OFFSET) & LOWEST_FOUR_BITS_MASK;
	set_calib_by_adc_mux(data, adc_type, mic_mux, ADC2L_PGA_CTRL1_REG);
}

static void pllmad_config(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, CODEC_ANA_RWREG_209, 0xBB);
	snd_soc_write(codec, CODEC_ANA_RWREG_210, 0x80);
	snd_soc_write(codec, CODEC_ANA_RWREG_203, 0x59);
	snd_soc_write(codec, CODEC_ANA_RWREG_213, 0x28);
	snd_soc_write(codec, CODEC_ANA_RWREG_205, 0x08);
	snd_soc_write(codec, CODEC_ANA_RWREG_147, 0xC8);
	snd_soc_write(codec, CODEC_ANA_RWREG_204, 0xF9);
}

static void pll48k_config(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, CODEC_ANA_RWREG_154, 0x11);
	snd_soc_write(codec, CODEC_ANA_RWREG_156, 0xEB);
	snd_soc_write(codec, CODEC_ANA_RWREG_157, 0x80);
	snd_soc_write(codec, CODEC_ANA_RWREG_150, 0x51);
	snd_soc_write(codec, CODEC_ANA_RWREG_152, 0x40);
	snd_soc_write(codec, CODEC_ANA_RWREG_159, 0xAC);
}

static void pll44k1_config(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	snd_soc_write(codec, CODEC_ANA_RWREG_177, 0x0E);
	snd_soc_write(codec, CODEC_ANA_RWREG_179, 0xB3);
	snd_soc_write(codec, CODEC_ANA_RWREG_180, 0x30);
	snd_soc_write(codec, CODEC_ANA_RWREG_175, 0x00);
	snd_soc_write(codec, CODEC_ANA_RWREG_182, 0x9A);
	snd_soc_write(codec, CODEC_ANA_RWREG_176, 0x81);
	snd_soc_write(codec, CODEC_ANA_RWREG_173, 0x40);
	snd_soc_write(codec, CODEC_ANA_RWREG_171, 0x0F);

	OUT_FUNCTION;
}

static const struct reg_seq_config buck_init_regs[] = {
	{ { CODEC_ANA_RWREG_235, 0, 0x10, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_224, 0, 0x66, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_217, 0, 0x74, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_218, 0, 0x20, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_219, 0, 0x07, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_220, 0, 0x05, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_221, 0, 0x03, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_222, 0, 0x02, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_223, 0, 0x01, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_225, 0, 0x33, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_226, 0, 0x39, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_227, 0, 0x74, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_228, 0, 0x20, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_229, 0, 0x78, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_230, 0, 0x58, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_231, 0, 0x38, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_232, 0, 0x1A, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_233, 0, 0x0C, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_234, 0, 0x01, false }, 0, 0 },

	{ { CODEC_ANA_RWREG_128, 0, 0x07, false }, 0, 0 },
	{ { CODEC_ANA_RWREG_129, 0, 0x07, false }, 0, 0 },
	/* fix high-low temperature and ATE */
	{ { CODEC_ANA_RWREG_090, 0, 0xDC, false }, 0, 0 },
};

static void buck_init(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	write_reg_seq_array(codec, buck_init_regs, ARRAY_SIZE(buck_init_regs));

	OUT_FUNCTION;
}

static void pll_lock_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_165,
		MASK_ON_BIT(MAIN1_LOCK_DET_MODE_LEN, MAIN1_LOCK_DET_MODE_OFFSET),
		MAIN1_LOCK_DET_MODE_ONLY_F << MAIN1_LOCK_DET_MODE_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_166,
		MASK_ON_BIT(MAIN1_LCK_THR_LEN, MAIN1_LCK_THR_OFFSET),
		MAIN1_LCK_THR_4 << MAIN1_LCK_THR_OFFSET);

	hi64xx_update_bits(codec, CODEC_ANA_RWREG_188,
		MASK_ON_BIT(MAIN2_LOCK_DET_MODE_LEN, MAIN2_LOCK_DET_MODE_OFFSET),
		MAIN2_LOCK_DET_MODE_ONLY_F << MAIN2_LOCK_DET_MODE_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_189,
		MASK_ON_BIT(MAIN2_LCK_THR_LEN, MAIN2_LCK_THR_OFFSET),
		MAIN2_LCK_THR_4 << MAIN2_LCK_THR_OFFSET);
}

struct hi6405_ana_clk_sel_reg {
	enum hi64xx_pll_type pll_type;
	unsigned int ana_mic_val; /* mic select */
	unsigned int ana_tx_val; /* tx_hp select */
};

static const struct hi6405_ana_clk_sel_reg hi6405_ana_clk_sel_reg_val[] = {
	{ PLL_LOW, 0x3f, 0x3 }, /* mad pll */
	{ PLL_HIGH, 0x0, 0x0 }, /* main1 pll */
	{ PLL_44_1, 0x0, 0x2 },  /* main2 pll */
};

static void select_ana_clk_source(struct snd_soc_codec *codec, int pll_type)
{
	size_t i;
	size_t array_size = ARRAY_SIZE(hi6405_ana_clk_sel_reg_val);

	if (pll_type < PLL_LOW || pll_type >= PLL_MAX) {
		AUDIO_LOGE("error clk sourec type %d", pll_type);
		return;
	}
	AUDIO_LOGI("set ana clk source pll_type %d", pll_type);

	for (i = 0; i < array_size; i++) {
		if (pll_type == hi6405_ana_clk_sel_reg_val[i].pll_type) {
			hi64xx_update_bits(codec, CODEC_ANA_RWREG_140, 0x3f,
				hi6405_ana_clk_sel_reg_val[i].ana_mic_val << CODEC_CLK_ANA_MAD_SEL_OFFSET);
			hi64xx_update_bits(codec, CODEC_ANA_RWREG_141,
				MASK_ON_BIT(CODEC_ANA_CODEC_CLK_ANA_TX_SEL_LEN, CODEC_ANA_CODEC_CLK_ANA_TX_SEL_OFFSET),
				hi6405_ana_clk_sel_reg_val[i].ana_tx_val << CODEC_ANA_CODEC_CLK_ANA_TX_SEL_OFFSET);
			break;
		}
	}
}


static void clk_cfg_init(struct snd_soc_codec *codec)
{
	/* 12M288&6M144 clk enable */
	hi64xx_update_bits(codec, AUD_CLK_EN_REG,
		0x1 << AUD_12M_CLK_EN_OFFSET, 0x1 << AUD_12M_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, AUD_CLK_EN_REG,
		0x1 << AUD_6144K_CLK_EN_OFFSET, 0x1 << AUD_6144K_CLK_EN_OFFSET);
	/* 11M289&5M644 clk enable */
	hi64xx_update_bits(codec, AUD_CLK_EN_REG,
		0x1 << AUD_11M_CLK_EN_OFFSET | 0x1 << AUD_56448K_CLK_EN_OFFSET,
		0x1 << AUD_11M_CLK_EN_OFFSET | 0x1 << AUD_56448K_CLK_EN_OFFSET);
	/* clk source select -> main1 pll */
	select_ana_clk_source(codec, PLL_HIGH);

	/* pll lock mode cfg */
	pll_lock_init(codec);
}

static void enable_regulator(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, CODEC_ANA_RWREG_104, 0x80);/* en GBR */
	usleep_range(10000, 11000);
	snd_soc_write(codec, CODEC_ANA_RWREG_127, 0x40);/* en fast start */
	snd_soc_write(codec, CODEC_ANA_RWREG_130, 0x04);/* cfg LDOTRX to 1.8 */
	snd_soc_write(codec, CODEC_ANA_RWREG_104, 0x84);/* en LDOTRX */

	/* iso_a18_d11/iso_a33_d11 enable */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_049,
		0x1 << AVREF_ESD_SW11_OFFSET, 0x1 << AVREF_ESD_SW11_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_049,
		0x1 << ISO_A18_D11_REG_OFFSET, 0x1 << ISO_A18_D11_REG_OFFSET);
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_049,
		0x1 << ISO_A33_D11_REG_OFFSET, 0x1 << ISO_A33_D11_REG_OFFSET);
}

static bool pll48k_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int i = 0;
	unsigned int regval;

	usleep_range(3100, 3200);
	/* check pll48k lock state */
	while (!((snd_soc_read(codec, IRQ_ANA_2_REG) & (0x1 << MAIN1_PLL_LOCK_F_OFFSET))
		&& (snd_soc_read(codec, HI64XX_VERSION_REG) == HI64XX_VERSION_CS))) {
		udelay(200);
		if (++i == 50) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_read(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x", i, regval);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	return true;
}

static int pll48k_turn_on(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("turn on pll 48k");
	snd_soc_write(codec, CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_write(codec, CODEC_ANA_RWREG_153, 0x80);
	snd_soc_write(codec, CODEC_ANA_RWREG_150, 0x41);
	snd_soc_write(codec, CODEC_ANA_RWREG_149, 0x10);
	snd_soc_write(codec, CODEC_ANA_RWREG_148, 0x28);
	snd_soc_write(codec, CODEC_ANA_RWREG_151, 0x78);
	udelay(25);
	snd_soc_write(codec, CODEC_ANA_RWREG_149, 0xD0);
	snd_soc_write(codec, CODEC_ANA_RWREG_148, 0xA8);

	select_ana_clk_source(codec, PLL_HIGH);

	return 0;
}

static int pll48k_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;

	if (platform_data == NULL) {
		AUDIO_LOGE("platform data is null");
		return -EINVAL;
	}

	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	snd_soc_write(codec, CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_write(codec, CODEC_ANA_RWREG_151, 0x42);
	snd_soc_write(codec, CODEC_ANA_RWREG_150, 0x51);
	snd_soc_write(codec, CODEC_ANA_RWREG_153, 0x0);
	snd_soc_write(codec, CODEC_ANA_RWREG_149, 0x10);

	regval = snd_soc_read(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("pll 48k off, pll state:0x%x", regval);

	return 0;
}

static bool pll44k1_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval[2] = {0};
	int i = 0;

	usleep_range(3100, 3200);
	/* check pll44k1 lock state */
	while (!(snd_soc_read(codec, IRQ_ANA_2_REG) & (0x1 << MAIN2_PLL_LOCK_F_OFFSET))) {
		udelay(200);
		if (++i == 50) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}

	regval[0] = snd_soc_read(codec, IRQ_ANA_2_REG);
	regval[1] = snd_soc_read(codec, HI64XX_VERSION_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x, version:0x%x",
		i, regval[0], regval[1]);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	return true;
}

static int pll44k1_turn_on(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	AUDIO_LOGI("turn on pll 44.1k");
	/* 44.1 clk enable */
	hi64xx_update_bits(codec, AUD_CLK_EN_REG,
		0x1 << AUD_44P1K_SEL2_OFFSET | 0x1 << AUD_44P1K_SEL0_OFFSET,
		0x1 << AUD_44P1K_SEL2_OFFSET | 0x1 << AUD_44P1K_SEL0_OFFSET);
	/* pll enable */
	snd_soc_write(codec, CODEC_ANA_RWREG_172, 0x10);
	snd_soc_write(codec, CODEC_ANA_RWREG_171, 0x8);
	snd_soc_write(codec, CODEC_ANA_RWREG_174, 0x78);
	udelay(25);
	snd_soc_write(codec, CODEC_ANA_RWREG_172, 0xD0);
	snd_soc_write(codec, CODEC_ANA_RWREG_171, 0x88);

	select_ana_clk_source(codec, PLL_44_1);

	OUT_FUNCTION;

	return 0;
}

static int pll44k1_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;

	IN_FUNCTION;

	AUDIO_LOGI("turn off pll 44.1k");

	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	select_ana_clk_source(codec, PLL_HIGH);
	/* pll disable */
	snd_soc_write(codec, CODEC_ANA_RWREG_171, 0x2F);
	snd_soc_write(codec, CODEC_ANA_RWREG_174, 0x40);
	snd_soc_write(codec, CODEC_ANA_RWREG_172, 0xD0);

	/* 44.1 clk disable */
	hi64xx_update_bits(codec, AUD_CLK_EN_REG,
		0x1 << AUD_44P1K_SEL2_OFFSET | 0x1 << AUD_44P1K_SEL0_OFFSET, 0);

	regval = snd_soc_read(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("44k1 off, pll state:0x%x", regval);

	OUT_FUNCTION;

	return 0;
}

static bool pllmad_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;
	int i = 0;

	usleep_range(5000, 5500);
	while (!(snd_soc_read(codec, IRQ_ANA_2_REG) & (0x1 << MAD_PLL_LOCK_F_OFFSET))) {
		usleep_range(5000, 5500);
		if (++i == 10) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_read(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x", i, regval);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	return true;
}

static int pllmad_turn_on(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("turn on pll mad");
	snd_soc_write(codec, CODEC_ANA_RWREG_204, 0xF8);
	snd_soc_write(codec, CODEC_ANA_RWREG_203, 0x41);
	snd_soc_write(codec, CODEC_ANA_RWREG_147, 0xC0);
	udelay(25);
	snd_soc_write(codec, CODEC_ANA_RWREG_204, 0xF9);
	snd_soc_write(codec, CODEC_ANA_RWREG_203, 0x43);
	hi64xx_update_bits(codec, CLK_SOURCE_SW_REG,
		0x1 << CLK_12288_SW_OFFSET, 0x1 << CLK_12288_SW_OFFSET);
	/* clk source select -> mad pll */
	select_ana_clk_source(codec, PLL_LOW);

	return 0;
}

static int pllmad_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;

	AUDIO_LOGI("turn off pll mad");
	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	hi64xx_update_bits(codec, CLK_SOURCE_SW_REG,
		0x1 << CLK_12288_SW_OFFSET, 0x0);
	snd_soc_write(codec, CODEC_ANA_RWREG_203, 0x59);
	snd_soc_write(codec, CODEC_ANA_RWREG_147, 0xC8);

	regval = snd_soc_read(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("mad off, pll state:0x%x\n", regval);

	return 0;
}

static int enable_supply(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("enable supply");
	snd_soc_write(codec, CODEC_ANA_RWREG_146, 0x19);/* pll ibias en */

	return 0;
}

static int disable_supply(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("disable supply");
	snd_soc_write(codec, CODEC_ANA_RWREG_146, 0x59);/* pll bias disable */

	return 0;
}

static int enable_ibias(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("enable ibias");
	/* ibias on */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_000,
		0x1 << IBIAS_PD_OFFSET, 0);

	return 0;
}

static int disable_ibias(struct snd_soc_codec *codec)
{
	AUDIO_LOGI("disable ibias");

	/* ibias off */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_000,
		0x1 << IBIAS_PD_OFFSET, 0x1 << IBIAS_PD_OFFSET);

	return 0;
}

static int enable_micbias(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	AUDIO_LOGI("hs micbias enable");
	/* mask btn irqs */
	hi64xx_irq_mask_btn_irqs(data->mbhc);
	/* eco off */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_080,
		0x1 << MBHD_ECO_EN_BIT, 0);
	/* mbhc cmp on */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << MBHD_PD_NORMCOMP, 0);
	/* hsmic chg */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_073,
		0x1 << HSMICB2_DSCHG, 0);
	/* hsmic on */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_013,
		0x1 << HS_MICB_PD_BIT, 0);
	msleep(20);
	/* unmask btn irqs */
	hi64xx_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}

static int disable_micbias(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	/* mask btn irqs */
	hi64xx_irq_mask_btn_irqs(data->mbhc);
	/* hsmic pd */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_013,
		0x1 << HS_MICB_PD_BIT, 0x1 << HS_MICB_PD_BIT);
	/* hsmic dischg */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_073,
		0x1 << HSMICB2_DSCHG, 0x1 << HSMICB2_DSCHG);
	usleep_range(15000, 16500);
	/* hsmic chg */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_073,
		0x1 << HSMICB2_DSCHG, 0);
	AUDIO_LOGI("hs micbias disable");
	/* eco on & eco auto saradc on */
	/* mbhc cmp off */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_012,
		0x1 << MBHD_PD_NORMCOMP, 0x1 << MBHD_PD_NORMCOMP);
	/* eco on */
	hi64xx_update_bits(codec, CODEC_ANA_RWREG_080,
		0x1 << MBHD_ECO_EN_BIT, 0x1 << MBHD_ECO_EN_BIT);
	AUDIO_LOGI("eco enable");
	msleep(20);

	/* unmask btn irqs */
	hi64xx_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}

static int pll_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_pll(priv->resmgr, PLL_HIGH);
		if (!priv->cdc_ctrl->pm_runtime_support) {
			mdelay(2);
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_HI6405,
				SLIMBUS_FRAMER_CODEC);
			if (ret) {
				AUDIO_LOGE("slimbus switch framer to codec failed\n");
			}
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (!priv->cdc_ctrl->pm_runtime_support) {
			/* slimbus master framer is soc */
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_HI6405,
				SLIMBUS_FRAMER_SOC);
			if (ret) {
				AUDIO_LOGE("slimbus switch framer to soc failed\n");
			}
			mdelay(2);
		}
		hi64xx_resmgr_release_pll(priv->resmgr, PLL_HIGH);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int pll_param_pass(struct snd_soc_dapm_widget *w,
	enum hi64xx_pll_type pll_type, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_pll(platform_data->resmgr, pll_type);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_resmgr_release_pll(platform_data->resmgr, pll_type);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}


static void request_cp1(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp1_num++;
	if (data->cp1_num == 1) {
		/* buck1 enable */
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x1 << BUCK1_ENP_BIT, 0x1 << BUCK1_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);

	OUT_FUNCTION;
}

static void release_cp1(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&data->impdet_dapm_mutex);
	if (data->cp1_num == 0) {
		AUDIO_LOGE("cp1 num is 0 when release");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp1_num--;
	if (data->cp1_num == 0) {
		/* buck1 disable */
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x1 << BUCK1_ENP_BIT, 0);
	}
	mutex_unlock(&data->impdet_dapm_mutex);

	OUT_FUNCTION;
}

static void request_cp2(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp2_num++;
	if (data->cp2_num == 1) {
		/* buck2 enable */
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x1 << CP2_ENP_BIT, 0x1 << CP2_ENP_BIT);
		mdelay(5);
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x3 << LDON_ENP_BIT, 0x3 << LDON_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);

	OUT_FUNCTION;
}

static void release_cp2(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&data->impdet_dapm_mutex);
	if (data->cp2_num == 0) {
		AUDIO_LOGE("cp2 num is 0 when release!");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp2_num--;
	if (data->cp2_num == 0) {
		/* buck2 disable */
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x3 << LDON_ENP_BIT, 0x0 << LDON_ENP_BIT);
		mdelay(5);
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_104,
			0x1 << CP2_ENP_BIT, 0x0 << CP2_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);

	OUT_FUNCTION;
}

static int clk_44k1_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	IN_FUNCTION;

	ret = pll_param_pass(w, PLL_44_1, event);

	OUT_FUNCTION;

	return ret;
}

static int mad_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	IN_FUNCTION;

	ret = pll_param_pass(w, PLL_LOW, event);

	OUT_FUNCTION;

	return ret;
}

void request_dp_clk(struct snd_soc_codec *codec, bool enable)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	mutex_lock(&platform_data->impdet_dapm_mutex);
	if (enable) {
		platform_data->dp_clk_num++;
		if (platform_data->dp_clk_num == 1) {
			hi64xx_update_bits(codec, CODEC_DP_CLKEN_REG,
				0x1 << CODEC_DP_CLKEN_OFFSET,
				0x1 << CODEC_DP_CLKEN_OFFSET);
		}
	} else {
		if (platform_data->dp_clk_num <= 0) {
			mutex_unlock(&platform_data->impdet_dapm_mutex);
			AUDIO_LOGE("dp clk num is 0 when disable");
			return;
		}
		platform_data->dp_clk_num--;
		if (platform_data->dp_clk_num == 0) {
			hi64xx_update_bits(codec, CODEC_DP_CLKEN_REG,
				0x1 << CODEC_DP_CLKEN_OFFSET, 0);
		}
	}
	mutex_unlock(&platform_data->impdet_dapm_mutex);

	OUT_FUNCTION;
}

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
void request_cp_dp_clk(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec is null");
		return;
	}

	/* dp clk enable */
	request_dp_clk(codec, true);
	/* cp1 enable */
	request_cp1(codec);
	/* cp2 enable */
	request_cp2(codec);
}
void release_cp_dp_clk(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec is null");
		return;
	}

	/* cp2 disable */
	release_cp2(codec);
	/* cp1 disable */
	release_cp1(codec);
	/* dp clk disable */
	request_dp_clk(codec, false);
}
#endif

static int dp_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		request_dp_clk(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		request_dp_clk(codec, false);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s1_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S1_CLK_IF_EN_OFFSET,
			0x1 << S1_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S1_CLK_IF_TXRX_EN_OFFSET,
			0x1 << S1_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S1_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S1_CLK_IF_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S2_CLK_IF_EN_OFFSET,
			0x1 << S2_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S2_CLK_IF_TXRX_EN_OFFSET,
			0x1 << S2_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S2_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S2_CLK_IF_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s4_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S4_CLK_IF_EN_OFFSET,
			0x1 << S4_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S4_CLK_IF_TXRX_EN_OFFSET,
			0x1 << S4_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S4_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			0x1 << S4_CLK_IF_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int cp1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		request_cp1(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		release_cp1(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int cp2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		request_cp2(codec);
		usleep_range(5000, 5500);
		break;
	case SND_SOC_DAPM_POST_PMD:
		release_cp2(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_rx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_S2_IF_L_REG,
			0x1 << S2_IF_RX_ENA_OFFSET,
			0x1 << S2_IF_RX_ENA_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_S2_IF_L_REG,
			0x1 << S2_IF_RX_ENA_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s4_rx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, SC_S4_IF_L_REG,
			0x1 << S4_IF_RX_ENA_OFFSET,
			0x1 << S4_IF_RX_ENA_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, SC_S4_IF_L_REG,
			0x1 << S4_IF_RX_ENA_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

#define SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY_S("PLL_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, pll_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("44K1_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, clk_44k1_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("MAD_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, mad_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("DP_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, dp_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S1_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, s1_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S2_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, s2_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S4_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, s4_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP1_SUPPLY", \
		3, SND_SOC_NOPM, 0, 0, cp1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP2_SUPPLY", \
		4, SND_SOC_NOPM, 0, 0, cp2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S2_RX_SUPPLY", \
		2, SND_SOC_NOPM, 0, 0, s2_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S4_RX_SUPPLY", \
		2, SND_SOC_NOPM, 0, 0, s4_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

/* SWITCH - PLL TEST */
static const struct snd_kcontrol_new dapm_pll44k1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLL48K_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_pll48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLL44K1_BIT, 1, 0);
static const struct snd_kcontrol_new dapm_pll32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE", VIRTUAL_DOWN_REG, PLL32K_BIT, 1, 0);

static int pll48k_test_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	pll_param_pass(w, PLL_HIGH, event);

	OUT_FUNCTION;

	return 0;
}

static int pll44k1_test_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	pll_param_pass(w, PLL_44_1, event);

	OUT_FUNCTION;

	return 0;
}

static int pll32k_test_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	pll_param_pass(w, PLL_LOW, event);

	OUT_FUNCTION;

	return 0;
}

#define PLL_WIDGET \
	SND_SOC_DAPM_SWITCH_E("PLL48K_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pll48k_switch_controls, \
		pll48k_test_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLL44K1_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pll44k1_switch_controls, \
		pll44k1_test_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLLMAD_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &dapm_pll32k_switch_controls, \
		pll32k_test_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static int micbias1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			0x1 << ANA_MICBIAS1_DSCHG_OFFSET | 0xF,
			0x0 << ANA_MICBIAS1_DSCHG_OFFSET | ANA_MICBIAS1_ADJ_2_7V);
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			0x1 << ANA_MICBIAS1_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			0x1 << ANA_MICBIAS1_PD_BIT, 0x1 << ANA_MICBIAS1_PD_BIT);
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			0x1 << ANA_MICBIAS1_DSCHG_OFFSET | 0xF,
			0x1 << ANA_MICBIAS1_DSCHG_OFFSET);
		usleep_range(10000, 11000);
		hi64xx_update_bits(codec, ANA_MICBIAS1,
			0x1 << ANA_MICBIAS1_DSCHG_OFFSET | 0xF, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	mic_control_sc_frequency(event, platform_data);

	return 0;
}

static int micbias2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MICBIAS2,
			0x1 << ANA_MICBIAS2_DSCHG_OFFSET | 0xF,
			0 << ANA_MICBIAS2_DSCHG_OFFSET | ANA_MICBIAS2_ADJ_2_7V);
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			0x1 << ANA_MICBIAS2_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_MICBIAS_ENABLE,
			0x1 << ANA_MICBIAS2_PD_BIT,
			0x1 << ANA_MICBIAS2_PD_BIT);
		hi64xx_update_bits(codec, ANA_MICBIAS2,
			0x1 << ANA_MICBIAS2_DSCHG_OFFSET | 0xF,
			0x1 << ANA_MICBIAS2_DSCHG_OFFSET);
		usleep_range(10000, 11000);
		hi64xx_update_bits(codec, ANA_MICBIAS2,
			0x1 << ANA_MICBIAS2_DSCHG_OFFSET | 0xF, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int hsmicbias_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_micbias(platform_data->resmgr);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_resmgr_release_micbias(platform_data->resmgr);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

#define MICBIAS_WIDGET \
	SND_SOC_DAPM_MIC("MICBIAS1_MIC", micbias1_power_event), \
	SND_SOC_DAPM_MIC("MICBIAS2_MIC", micbias2_power_event), \
	SND_SOC_DAPM_MIC("HSMICBIAS", hsmicbias_power_event), \

static int mad_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MAD_PGA_ENABLE,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1, 0);
		hi64xx_update_bits(codec, ANA_MAD_PGA_SDM,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT, 0);
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_000,
			0x1 << PD_MAD_SDM_VREF_OFFSET, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, CODEC_ANA_RWREG_000,
			0x1 << PD_MAD_SDM_VREF_OFFSET,
			0x1 << PD_MAD_SDM_VREF_OFFSET);
		hi64xx_update_bits(codec, ANA_MAD_PGA_SDM,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT,
			0x1 << ANA_MAD_PGA_PD_OUT | 0x1 << ANA_MAD_PGA_PD_IBIAS |
			0x1 << ANA_MAD_PGA_PD_SDM | 0x1 << ANA_MAD_PGA_MUTE_FLS |
			0x1 << ANA_MAD_PGA_MUTE_DWA_OUT);
		hi64xx_update_bits(codec, ANA_MAD_PGA_ENABLE,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1,
			0x1 << ANA_MAD_PGA_PD | 0x1 << ANA_MAD_PGA_MUTE2 |
			0x1 << ANA_MAD_PGA_MUTE1);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int hsmic_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_HSMIC_PGA_ENABLE,
			0x1 << ANA_HSMIC_PGA_PD | 0x1 << ANA_HSMIC_PGA_MUTE2 |
			0x1 << ANA_HSMIC_PGA_MUTE1 | 0x1 << ANA_HSMIC_FLASH_MUTE |
			0x1 << ANA_HSMIC_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_HSMIC_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_HSMIC_OFFSET,
			0x1 << ANA_PD_OUT_HSMIC_OFFSET);
		hi64xx_update_bits(codec, ANA_HSMIC_PGA_ENABLE,
			0x1 << ANA_HSMIC_PGA_PD | 0x1 << ANA_HSMIC_PGA_MUTE2 |
			0x1 << ANA_HSMIC_PGA_MUTE1 | 0x1 << ANA_HSMIC_FLASH_MUTE |
			0x1 << ANA_HSMIC_PGA_SDM_PD,
			0x1 << ANA_HSMIC_PGA_PD | 0x1 << ANA_HSMIC_PGA_MUTE2 |
			0x1 << ANA_HSMIC_PGA_MUTE1 | 0x1 << ANA_HSMIC_FLASH_MUTE |
			0x1 << ANA_HSMIC_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int auxmic_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_AUXMIC_PGA_ENABLE,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET,
			0x1 << ANA_PD_OUT_AUXMIC_OFFSET);
		hi64xx_update_bits(codec, ANA_AUXMIC_PGA_ENABLE,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD,
			0x1 << ANA_AUXMIC_PGA_PD | 0x1 << ANA_AUXMIC_PGA_MUTE2 |
			0x1 << ANA_AUXMIC_PGA_MUTE1 | 0x1 << ANA_AUXMIC_FLASH_MUTE |
			0x1 << ANA_AUXMIC_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic3_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MIC3_PGA_ENABLE,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC3_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC3_OFFSET,
			0x1 << ANA_PD_OUT_MIC3_OFFSET);
		hi64xx_update_bits(codec, ANA_MIC3_PGA_ENABLE,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD,
			0x1 << ANA_MIC3_PGA_PD | 0x1 << ANA_MIC3_PGA_MUTE2 |
			0x1 << ANA_MIC3_PGA_MUTE1 | 0x1 << ANA_MIC3_FLASH_MUTE |
			0x1 << ANA_MIC3_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic4_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MIC4_PGA_ENABLE,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC4_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC4_OFFSET,
			0x1 << ANA_PD_OUT_MIC4_OFFSET);
		hi64xx_update_bits(codec, ANA_MIC4_PGA_ENABLE,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD,
			0x1 << ANA_MIC4_PGA_PD | 0x1 << ANA_MIC4_PGA_MUTE2 |
			0x1 << ANA_MIC4_PGA_MUTE1 | 0x1 << ANA_MIC4_FLASH_MUTE |
			0x1 << ANA_MIC4_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int mic5_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ANA_MIC5_PGA_ENABLE,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC5_OFFSET, 0x0);
		/* set mic calib values for all valid routes */
		set_mic_calib_value(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, ANA_ADC_SDM,
			0x1 << ANA_PD_OUT_MIC5_OFFSET,
			0x1 << ANA_PD_OUT_MIC5_OFFSET);
		hi64xx_update_bits(codec, ANA_MIC5_PGA_ENABLE,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD,
			0x1 << ANA_MIC5_PGA_PD | 0x1 << ANA_MIC5_PGA_MUTE2 |
			0x1 << ANA_MIC5_PGA_MUTE1 | 0x1 << ANA_MIC5_FLASH_MUTE |
			0x1 << ANA_MIC5_PGA_SDM_PD);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl0_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC0L_EN_OFFSET,
			0x1 << CLK_ADC0L_EN_OFFSET);
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << ADC0L_PGA_CLK_EN_OFFSET,
			0x1 << ADC0L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << ADC0L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC0L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcr0_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC0R_EN_OFFSET,
			0x1 << CLK_ADC0R_EN_OFFSET);
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << ADC0R_PGA_CLK_EN_OFFSET,
			0x1 << ADC0R_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, S1_MIXER_EQ_CLK_EN_REG,
			0x1 << ADC0R_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC0R_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC1L_EN_OFFSET,
			0x1 << CLK_ADC1L_EN_OFFSET);
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_1_REG,
			0x1 << ADC1L_PGA_CLK_EN_OFFSET,
			0x1 << ADC1L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_1_REG,
			0x1 << ADC1L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC1L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcr1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC1R_EN_OFFSET,
			0x1 << CLK_ADC1R_EN_OFFSET);
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_1_REG,
			0x1 << ADC1R_PGA_CLK_EN_OFFSET,
			0x1 << ADC1R_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_1_REG,
			0x1 << ADC1R_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, ADC_DAC_CLK_EN_REG,
			0x1 << CLK_ADC1R_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET,
			0x1 << CLK_ADC2L_EN_OFFSET);
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adcl2_virtbtn_ir_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		AUDIO_LOGW("hi6405_adcl2_virtbtn_ir_power_event 1");
		snd_soc_write(codec, SC_CODEC_MUX_CTRL19_REG, 0x1);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x1 << DSPIF8_DIN_SEL_OFFSET,
			0x1 << DSPIF8_DIN_SEL_OFFSET);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x3 << DSPIF9_DIN_SEL_OFFSET,
			0x0 << DSPIF9_DIN_SEL_OFFSET);
		hi64xx_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET);
		snd_soc_write(codec, SC_CODEC_MUX_CTRL11_REG, 0x1);
		hi64xx_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET,
			0x1 << CLK_ADC2L_EN_OFFSET);
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		AUDIO_LOGW("hi6405_adcl2_virtbtn_ir_power_event 0");
		snd_soc_write(codec, SC_CODEC_MUX_CTRL11_REG, 0x0);
		hi64xx_update_bits(codec, ADC2L_CTRL0_REG,
			0x1 << ADC2L_HPF_BYPASS_EN_OFFSET |
			0x1 << ADC2L_IR2D_BYPASS_EN_OFFSET, 0x0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x1 << DSPIF8_DIN_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, SC_CODEC_MUX_CTRL39_REG,
			0x3 << DSPIF9_DIN_SEL_OFFSET,
			0x0 << DSPIF9_DIN_SEL_OFFSET);
		snd_soc_write(codec, SC_CODEC_MUX_CTRL19_REG, 0x0);
		hi64xx_update_bits(codec, DAC_DP_CLK_EN_2_REG,
			0x1 << ADC2L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, I2S_DSPIF_CLK_EN_REG,
			0x1 << CLK_ADC2L_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_il_pga_power_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *kcontrol, int event)
{
    struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IL_PGA_CLK_EN_OFFSET | 0x1 << S2_IL_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int s2_ir_pga_power_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, S2_DP_CLK_EN_REG,
			0x1 << S2_IR_PGA_CLK_EN_OFFSET | 0x1 << S2_IR_SRC_CLK_EN_OFFSET, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}


#define PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("MAD_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mad_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("HSMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hsmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("AUXMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		auxmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC3_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic3_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC4_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic4_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC5_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mic5_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR1", \
		1, SND_SOC_NOPM, 0, 0, adcr1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL1", \
		1, SND_SOC_NOPM, 0, 0, adcl1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR0", \
		1, SND_SOC_NOPM, 0, 0, adcr0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL0", \
		1, SND_SOC_NOPM, 0, 0, adcl0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2", \
		1, SND_SOC_NOPM, 0, 0, adcl2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2_VIRTBTN_IR", \
		1, SND_SOC_NOPM, 0, 0, adcl2_virtbtn_ir_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S1_IL_PGA", \
		0, S1_DP_CLK_EN_REG, S1_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S1_IR_PGA", \
		0, S1_DP_CLK_EN_REG, S1_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S2_IL_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		s2_il_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S2_IR_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		s2_ir_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S3_IL_PGA", \
		0, S3_DP_CLK_EN_REG, S3_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S3_IR_PGA", \
		0, S3_DP_CLK_EN_REG, S3_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IL_PGA", \
		0, VIRTUAL_DOWN_REG, S4_IL_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IR_PGA", \
		0, VIRTUAL_DOWN_REG, S4_IR_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("SIDETONE_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		NULL, 0), \

static const struct snd_soc_dapm_widget resource_widgets[] = {
	SUPPLY_WIDGET
	PLL_WIDGET
	MICBIAS_WIDGET
	PGA_WIDGET
};

int hi6405_add_resource_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, resource_widgets,
		ARRAY_SIZE(resource_widgets));
}

int hi6405_resmgr_init(struct hi6405_platform_data *platform_data)
{
	int ret;
	struct resmgr_config cfg = { 0 };

	if (platform_data == NULL) {
		AUDIO_LOGE("platform data parameter is null");
		return -EINVAL;
	}

	cfg.pll_num = 3;
	cfg.pll_sw_mode = MODE_MULTIPLE;
	cfg.pfn_pll_ctrls[PLL_LOW].turn_on = pllmad_turn_on;
	cfg.pfn_pll_ctrls[PLL_LOW].turn_off = pllmad_turn_off;
	cfg.pfn_pll_ctrls[PLL_LOW].is_locked = pllmad_check;
	cfg.pfn_pll_ctrls[PLL_HIGH].turn_on = pll48k_turn_on;
	cfg.pfn_pll_ctrls[PLL_HIGH].turn_off = pll48k_turn_off;
	cfg.pfn_pll_ctrls[PLL_HIGH].is_locked = pll48k_check;
	cfg.pfn_pll_ctrls[PLL_44_1].turn_on = pll44k1_turn_on;
	cfg.pfn_pll_ctrls[PLL_44_1].turn_off = pll44k1_turn_off;
	cfg.pfn_pll_ctrls[PLL_44_1].is_locked = pll44k1_check;
	cfg.enable_supply = enable_supply;
	cfg.disable_supply = disable_supply;
	cfg.enable_ibias = enable_ibias;
	cfg.disable_ibias = disable_ibias;
	cfg.enable_micbias = enable_micbias;
	cfg.disable_micbias = disable_micbias;
	cfg.pll_for_reg_access = pll_for_reg_access;

	ret = hi64xx_resmgr_init(platform_data->codec, platform_data->cdc_ctrl,
		platform_data->irqmgr, &cfg, &platform_data->resmgr);

	return ret;
}

void hi6405_supply_pll_init(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	enable_regulator(codec);
	buck_init(codec);
	pll48k_config(codec);
	pllmad_config(codec);
	pll44k1_config(codec);
	clk_cfg_init(codec);
}

