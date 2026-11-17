/*
 * resource_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "resource_widget.h"

#include <linux/version.h>
#include <sound/core.h>
#include <sound/tlv.h>

#include "asoc_adapter.h"
#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "huawei_platform/power/vsys_switch/vsys_switch.h"

#define CP2_DELAY_TIME 5

static void mic_control_sc_frequency(int event,
	struct da_combine_v5_platform_data *platform_data)
{
	struct da_combine_v5_board_cfg board_cfg = platform_data->board_config;

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

static enum da_combine_pll_type pll_for_reg_access(struct snd_soc_component *codec, unsigned int reg)
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

static void pllmad_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, CODEC_ANA_RWREG_209, 0xBB);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_210, 0x80);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_203, 0x59);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_213, 0x28);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_205, 0x08);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_147, 0xC8);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_204, 0xF9);
}

static void pll48k_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, CODEC_ANA_RWREG_154, 0x11);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_156, 0xEB);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_157, 0x80);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_150, 0x51);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_152, 0x40);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_159, 0xAC);
}

static void pll44k1_config(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	snd_soc_component_write(codec, CODEC_ANA_RWREG_177, 0x0E);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_179, 0xB3);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_180, 0x30);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_175, 0x00);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_182, 0x9A);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_176, 0x81);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_173, 0x40);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_171, 0x0F);

	AUDIO_LOGD("end");
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

static void buck_init(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	write_reg_seq_array(codec, buck_init_regs, ARRAY_SIZE(buck_init_regs));

	AUDIO_LOGD("end");
}

static void pll_lock_init(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_165,
		MASK_ON_BIT(MAIN1_LOCK_DET_MODE_LEN, MAIN1_LOCK_DET_MODE_OFFSET),
		MAIN1_LOCK_DET_MODE_ONLY_F << MAIN1_LOCK_DET_MODE_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_166,
		MASK_ON_BIT(MAIN1_LCK_THR_LEN, MAIN1_LCK_THR_OFFSET),
		MAIN1_LCK_THR_4 << MAIN1_LCK_THR_OFFSET);

	da_combine_update_bits(codec, CODEC_ANA_RWREG_188,
		MASK_ON_BIT(MAIN2_LOCK_DET_MODE_LEN, MAIN2_LOCK_DET_MODE_OFFSET),
		MAIN2_LOCK_DET_MODE_ONLY_F << MAIN2_LOCK_DET_MODE_OFFSET);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_189,
		MASK_ON_BIT(MAIN2_LCK_THR_LEN, MAIN2_LCK_THR_OFFSET),
		MAIN2_LCK_THR_4 << MAIN2_LCK_THR_OFFSET);
}

struct da_combine_v5_ana_clk_sel_reg {
	enum da_combine_pll_type pll_type;
	unsigned int ana_mic_val; /* mic select */
	unsigned int ana_tx_val; /* tx_hp select */
};

static const struct da_combine_v5_ana_clk_sel_reg da_combine_v5_ana_clk_sel_reg_val[] = {
	{ PLL_LOW, 0x3f, 0x3 }, /* mad pll */
	{ PLL_HIGH, 0x0, 0x0 }, /* main1 pll */
	{ PLL_44_1, 0x0, 0x2 },  /* main2 pll */
};

static void select_ana_clk_source(struct snd_soc_component *codec, int pll_type)
{
	size_t i;
	size_t array_size = ARRAY_SIZE(da_combine_v5_ana_clk_sel_reg_val);

	if (pll_type < PLL_LOW || pll_type >= PLL_MAX) {
		AUDIO_LOGE("error clk sourec type %d", pll_type);
		return;
	}
	AUDIO_LOGI("set ana clk source pll_type %d", pll_type);

	for (i = 0; i < array_size; i++) {
		if (pll_type == da_combine_v5_ana_clk_sel_reg_val[i].pll_type) {
			da_combine_update_bits(codec, CODEC_ANA_RWREG_140, 0x3f,
				da_combine_v5_ana_clk_sel_reg_val[i].ana_mic_val << CODEC_CLK_ANA_MAD_SEL_OFFSET);
			da_combine_update_bits(codec, CODEC_ANA_RWREG_141,
				MASK_ON_BIT(CODEC_ANA_CODEC_CLK_ANA_TX_SEL_LEN, CODEC_ANA_CODEC_CLK_ANA_TX_SEL_OFFSET),
				da_combine_v5_ana_clk_sel_reg_val[i].ana_tx_val << CODEC_ANA_CODEC_CLK_ANA_TX_SEL_OFFSET);
			break;
		}
	}
}

static void clk_cfg_init(struct snd_soc_component *codec)
{
	/* 12M288&6M144 clk enable */
	da_combine_update_bits(codec, AUD_CLK_EN_REG,
		BIT(AUD_12M_CLK_EN_OFFSET), BIT(AUD_12M_CLK_EN_OFFSET));
	da_combine_update_bits(codec, AUD_CLK_EN_REG,
		BIT(AUD_6144K_CLK_EN_OFFSET), BIT(AUD_6144K_CLK_EN_OFFSET));
	/* 11M289&5M644 clk enable */
	da_combine_update_bits(codec, AUD_CLK_EN_REG,
		BIT(AUD_11M_CLK_EN_OFFSET) | BIT(AUD_56448K_CLK_EN_OFFSET),
		BIT(AUD_11M_CLK_EN_OFFSET) | BIT(AUD_56448K_CLK_EN_OFFSET));
	/* clk source select -> main1 pll */
	select_ana_clk_source(codec, PLL_HIGH);

	/* pll lock mode cfg */
	pll_lock_init(codec);
}

static void enable_regulator(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, CODEC_ANA_RWREG_104, 0x80);/* en GBR */
	usleep_range(10000, 11000);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_127, 0x40);/* en fast start */
	snd_soc_component_write(codec, CODEC_ANA_RWREG_130, 0x04);/* cfg LDOTRX to 1.8 */
	snd_soc_component_write(codec, CODEC_ANA_RWREG_104, 0x84);/* en LDOTRX */

	/* iso_a18_d11/iso_a33_d11 enable */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_049,
		BIT(AVREF_ESD_SW11_OFFSET), BIT(AVREF_ESD_SW11_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_049,
		BIT(ISO_A18_D11_REG_OFFSET), BIT(ISO_A18_D11_REG_OFFSET));
	da_combine_update_bits(codec, CODEC_ANA_RWREG_049,
		BIT(ISO_A33_D11_REG_OFFSET), BIT(ISO_A33_D11_REG_OFFSET));
}

static bool pll48k_check(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	int i = 0;
	unsigned int regval;

	usleep_range(3100, 3200);
	/* check pll48k lock state */
	while (!((snd_soc_component_read32(codec, IRQ_ANA_2_REG) & (BIT(MAIN1_PLL_LOCK_F_OFFSET)))
		&& (snd_soc_component_read32(codec, DA_COMBINE_VERSION_REG) == DA_COMBINE_VERSION_CS))) {
		udelay(200);
		if (++i == 50) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x", i, regval);
	da_combine_irq_enable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	return true;
}

int pll48k_turn_on(struct snd_soc_component *codec)
{
	AUDIO_LOGI("turn on pll 48k");
	snd_soc_component_write(codec, CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_153, 0x80);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_150, 0x41);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_149, 0x10);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_148, 0x28);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_151, 0x78);
	udelay(25);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_149, 0xD0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_148, 0xA8);

	select_ana_clk_source(codec, PLL_HIGH);

	return 0;
}

static int pll48k_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	unsigned int regval;

	if (platform_data == NULL) {
		AUDIO_LOGE("platform data is null");
		return -EINVAL;
	}

	da_combine_irq_disable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_151, 0x42);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_150, 0x51);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_153, 0x0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_149, 0x10);

	regval = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("pll 48k off, pll state:0x%x", regval);

	return 0;
}

static bool pll44k1_check(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	unsigned int regval[2] = {0};
	int i = 0;

	usleep_range(3100, 3200);
	/* check pll44k1 lock state */
	while (!(snd_soc_component_read32(codec, IRQ_ANA_2_REG) & (BIT(MAIN2_PLL_LOCK_F_OFFSET)))) {
		udelay(200);
		if (++i == 50) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}

	regval[0] = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	regval[1] = snd_soc_component_read32(codec, DA_COMBINE_VERSION_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x, version:0x%x",
		i, regval[0], regval[1]);
	da_combine_irq_enable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	return true;
}

static int pll44k1_turn_on(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	AUDIO_LOGI("turn on pll 44.1k");
	/* 44.1 clk enable */
	da_combine_update_bits(codec, AUD_CLK_EN_REG,
		BIT(AUD_44P1K_SEL2_OFFSET) | BIT(AUD_44P1K_SEL0_OFFSET),
		BIT(AUD_44P1K_SEL2_OFFSET) | BIT(AUD_44P1K_SEL0_OFFSET));
	/* pll enable */
	snd_soc_component_write(codec, CODEC_ANA_RWREG_172, 0x10);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_171, 0x8);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_174, 0x78);
	udelay(25);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_172, 0xD0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_171, 0x88);

	select_ana_clk_source(codec, PLL_44_1);

	AUDIO_LOGD("end");

	return 0;
}

static int pll44k1_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	unsigned int regval;

	AUDIO_LOGD("begin");

	AUDIO_LOGI("turn off pll 44.1k");

	da_combine_irq_disable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	select_ana_clk_source(codec, PLL_HIGH);
	/* pll disable */
	snd_soc_component_write(codec, CODEC_ANA_RWREG_171, 0x2F);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_174, 0x40);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_172, 0xD0);

	/* 44.1 clk disable */
	da_combine_update_bits(codec, AUD_CLK_EN_REG,
		BIT(AUD_44P1K_SEL2_OFFSET) | BIT(AUD_44P1K_SEL0_OFFSET), 0);

	regval = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("44k1 off, pll state:0x%x", regval);

	AUDIO_LOGD("end");

	return 0;
}

static bool pllmad_check(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	unsigned int regval;
	int i = 0;

	usleep_range(5000, 5500);
	while (!(snd_soc_component_read32(codec, IRQ_ANA_2_REG) & (BIT(MAD_PLL_LOCK_F_OFFSET)))) {
		usleep_range(5000, 5500);
		if (++i == 10) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("check time is %d, pll state:0x%x", i, regval);
	da_combine_irq_enable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	return true;
}

static int pllmad_turn_on(struct snd_soc_component *codec)
{
	AUDIO_LOGI("turn on pll mad");
	snd_soc_component_write(codec, CODEC_ANA_RWREG_204, 0xF8);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_203, 0x41);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_147, 0xC0);
	udelay(25);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_204, 0xF9);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_203, 0x43);
	da_combine_update_bits(codec, CLK_SOURCE_SW_REG,
		BIT(CLK_12288_SW_OFFSET), BIT(CLK_12288_SW_OFFSET));
	/* clk source select -> mad pll */
	select_ana_clk_source(codec, PLL_LOW);

	return 0;
}

static int pllmad_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);
	unsigned int regval;

	AUDIO_LOGI("turn off pll mad");
	da_combine_irq_disable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	da_combine_update_bits(codec, CLK_SOURCE_SW_REG,
		BIT(CLK_12288_SW_OFFSET), 0x0);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_203, 0x59);
	snd_soc_component_write(codec, CODEC_ANA_RWREG_147, 0xC8);

	regval = snd_soc_component_read32(codec, IRQ_ANA_2_REG);
	AUDIO_LOGI("mad off, pll state:0x%x\n", regval);

	return 0;
}

int enable_supply(struct snd_soc_component *codec)
{
	AUDIO_LOGI("enable supply");
	snd_soc_component_write(codec, CODEC_ANA_RWREG_146, 0x19);/* pll ibias en */

	return 0;
}

static int disable_supply(struct snd_soc_component *codec)
{
	AUDIO_LOGI("disable supply");
	snd_soc_component_write(codec, CODEC_ANA_RWREG_146, 0x59);/* pll bias disable */

	return 0;
}

int enable_ibias(struct snd_soc_component *codec)
{
	AUDIO_LOGI("enable ibias");
	/* ibias on */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_000,
		BIT(IBIAS_PD_OFFSET), 0);

	return 0;
}

static int disable_ibias(struct snd_soc_component *codec)
{
	AUDIO_LOGI("disable ibias");

	/* ibias off */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_000,
		BIT(IBIAS_PD_OFFSET), BIT(IBIAS_PD_OFFSET));

	return 0;
}

int enable_hsmicbias(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGI("hs micbias enable");
	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(data->mbhc);
	/* eco off */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_080,
		BIT(MBHD_ECO_EN_BIT), 0);
	/* mbhc cmp on */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012,
		BIT(MBHD_PD_NORMCOMP), 0);
	/* hsmic chg */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_073,
		BIT(HSMICB2_DSCHG), 0);
	/* hsmic on */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_013,
		BIT(HS_MICB_PD_BIT), 0);
	msleep(20);
	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}

static int disable_hsmicbias(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(data->mbhc);
	/* hsmic pd */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_013,
		BIT(HS_MICB_PD_BIT), BIT(HS_MICB_PD_BIT));
	/* hsmic dischg */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_073,
		BIT(HSMICB2_DSCHG), BIT(HSMICB2_DSCHG));
	usleep_range(15000, 16500);
	/* hsmic chg */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_073,
		BIT(HSMICB2_DSCHG), 0);
	AUDIO_LOGI("hs micbias disable");
	/* eco on & eco auto saradc on */
	/* mbhc cmp off */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_012,
		BIT(MBHD_PD_NORMCOMP), BIT(MBHD_PD_NORMCOMP));
	/* eco on */
	da_combine_update_bits(codec, CODEC_ANA_RWREG_080,
		BIT(MBHD_ECO_EN_BIT), BIT(MBHD_ECO_EN_BIT));
	AUDIO_LOGI("eco enable");
	msleep(20);

	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}



static int pll_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *priv = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_pll(priv->resmgr, PLL_HIGH);
		if (!priv->cdc_ctrl->pm_runtime_support) {
			mdelay(2);
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_DA_COMBINE_V5,
				SLIMBUS_FRAMER_CODEC);
			if (ret)
				AUDIO_LOGE("slimbus switch framer failed\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (!priv->cdc_ctrl->pm_runtime_support) {
			/* slimbus master framer is soc */
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_DA_COMBINE_V5,
				SLIMBUS_FRAMER_SOC);
			if (ret)
				AUDIO_LOGE("slimbus switch framer to soc failed\n");
			mdelay(2);
		}
		da_combine_resmgr_release_pll(priv->resmgr, PLL_HIGH);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int pll_param_pass(struct snd_soc_dapm_widget *w,
	enum da_combine_pll_type pll_type, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_pll(platform_data->resmgr, pll_type);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_resmgr_release_pll(platform_data->resmgr, pll_type);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static void cp1_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		BIT(BUCK1_ENP_BIT), BIT(BUCK1_ENP_BIT));
}

static void cp1_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		BIT(BUCK1_ENP_BIT), 0);
}

static void cp2_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		BIT(CP2_ENP_BIT), BIT(CP2_ENP_BIT));
	mdelay(CP2_DELAY_TIME);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		0x3 << LDON_ENP_BIT, 0x3 << LDON_ENP_BIT);

}

static void cp2_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		0x3 << LDON_ENP_BIT, 0x0 << LDON_ENP_BIT);
	mdelay(CP2_DELAY_TIME);
	da_combine_update_bits(codec, CODEC_ANA_RWREG_104,
		BIT(CP2_ENP_BIT), 0x0 << CP2_ENP_BIT);
}

static void request_cp1(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp1_num++;
	/* buck1 enable */
	if (data->cp1_num == 1)
		cp1_enable(codec);
	mutex_unlock(&data->impdet_dapm_mutex);

	AUDIO_LOGD("end");
}

static void release_cp1(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	mutex_lock(&data->impdet_dapm_mutex);
	if (data->cp1_num == 0) {
		AUDIO_LOGE("cp1 num is 0 when release");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp1_num--;
	/* buck1 disable */
	if (data->cp1_num == 0)
		cp1_disable(codec);
	mutex_unlock(&data->impdet_dapm_mutex);

	AUDIO_LOGD("end");
}

static void request_cp2(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp2_num++;
	/* buck2 enable */
	if (data->cp2_num == 1)
		cp2_enable(codec);
	mutex_unlock(&data->impdet_dapm_mutex);

	AUDIO_LOGD("end");
}

static void release_cp2(struct snd_soc_component *codec)
{
	struct da_combine_v5_platform_data *data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	mutex_lock(&data->impdet_dapm_mutex);
	if (data->cp2_num == 0) {
		AUDIO_LOGE("cp2 num is 0 when release!");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp2_num--;
	/* buck2 disable */
	if (data->cp2_num == 0)
		cp2_disable(codec);
	mutex_unlock(&data->impdet_dapm_mutex);

	AUDIO_LOGD("end");
}

static int clk_44k1_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	AUDIO_LOGD("begin");

	ret = pll_param_pass(w, PLL_44_1, event);

	AUDIO_LOGD("end");

	return ret;
}

static int mad_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	AUDIO_LOGD("begin");

	ret = pll_param_pass(w, PLL_LOW, event);

	AUDIO_LOGD("end");

	return ret;
}

void dp_clk_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_DP_CLKEN_REG,
		BIT(CODEC_DP_CLKEN_OFFSET),
		BIT(CODEC_DP_CLKEN_OFFSET));
}

static void dp_clk_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, CODEC_DP_CLKEN_REG,
		BIT(CODEC_DP_CLKEN_OFFSET), 0);
}

void request_dp_clk(struct snd_soc_component *codec, bool enable)
{
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");

	mutex_lock(&platform_data->impdet_dapm_mutex);
	if (enable) {
		platform_data->dp_clk_num++;
		if (platform_data->dp_clk_num == 1)
			dp_clk_enable(codec);
	} else {
		if (platform_data->dp_clk_num <= 0) {
			mutex_unlock(&platform_data->impdet_dapm_mutex);
			AUDIO_LOGE("dp clk num is 0 when disable");
			return;
		}
		platform_data->dp_clk_num--;
		if (platform_data->dp_clk_num == 0)
			dp_clk_disable(codec);
	}
	mutex_unlock(&platform_data->impdet_dapm_mutex);

	AUDIO_LOGD("end");
}

#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
void request_cp_dp_clk(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	/* dp clk enable */
	request_dp_clk(codec, true);
	/* cp1 enable */
	request_cp1(codec);
	/* cp2 enable */
	request_cp2(codec);
}
void release_cp_dp_clk(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

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

	AUDIO_LOGD("end");

	return 0;
}

static int s1_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S1_CLK_IF_EN_OFFSET),
			BIT(S1_CLK_IF_EN_OFFSET));
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S1_CLK_IF_TXRX_EN_OFFSET),
			BIT(S1_CLK_IF_TXRX_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S1_CLK_IF_TXRX_EN_OFFSET), 0);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S1_CLK_IF_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int s2_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S2_CLK_IF_EN_OFFSET),
			BIT(S2_CLK_IF_EN_OFFSET));
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S2_CLK_IF_TXRX_EN_OFFSET),
			BIT(S2_CLK_IF_TXRX_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S2_CLK_IF_TXRX_EN_OFFSET), 0);
		da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
			BIT(S2_CLK_IF_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static void s4_if_clk_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
		BIT(S4_CLK_IF_EN_OFFSET),
		BIT(S4_CLK_IF_EN_OFFSET));
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
		BIT(S4_CLK_IF_TXRX_EN_OFFSET),
		BIT(S4_CLK_IF_TXRX_EN_OFFSET));

	AUDIO_LOGD("end");
}

static void s4_if_clk_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
		BIT(S4_CLK_IF_TXRX_EN_OFFSET), 0);
	da_combine_update_bits(codec, SC_CODEC_MUX_CTRL26_REG,
		BIT(S4_CLK_IF_EN_OFFSET), 0);

	AUDIO_LOGD("end");
}

static int s4_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		s4_if_clk_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		s4_if_clk_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static int cp1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

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

	AUDIO_LOGD("end");

	return 0;
}

static int cp2_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

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

	AUDIO_LOGD("end");

	return 0;
}

int s2_rx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			BIT(S2_IF_RX_ENA_OFFSET),
			BIT(S2_IF_RX_ENA_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, SC_S2_IF_L_REG,
			BIT(S2_IF_RX_ENA_OFFSET), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static void s4_rx_enable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, SC_S4_IF_L_REG,
		BIT(S4_IF_RX_ENA_OFFSET),
		BIT(S4_IF_RX_ENA_OFFSET));
}

static void s4_rx_disable(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, SC_S4_IF_L_REG,
		BIT(S4_IF_RX_ENA_OFFSET), 0);
}

static int s4_rx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		s4_rx_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		s4_rx_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

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
	AUDIO_LOGD("begin");

	pll_param_pass(w, PLL_HIGH, event);

	AUDIO_LOGD("end");

	return 0;
}

static int pll44k1_test_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	AUDIO_LOGD("begin");

	pll_param_pass(w, PLL_44_1, event);

	AUDIO_LOGD("end");

	return 0;
}

static int pll32k_test_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	AUDIO_LOGD("begin");

	pll_param_pass(w, PLL_LOW, event);

	AUDIO_LOGD("end");

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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MICBIAS1,
			BIT(ANA_MICBIAS1_DSCHG_OFFSET) | 0xF,
			0x0 << ANA_MICBIAS1_DSCHG_OFFSET | ANA_MICBIAS1_ADJ_2_7V);
		da_combine_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS1_PD_BIT), 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS1_PD_BIT), BIT(ANA_MICBIAS1_PD_BIT));
		da_combine_update_bits(codec, ANA_MICBIAS1,
			BIT(ANA_MICBIAS1_DSCHG_OFFSET) | 0xF,
			BIT(ANA_MICBIAS1_DSCHG_OFFSET));
		usleep_range(10000, 11000);
		da_combine_update_bits(codec, ANA_MICBIAS1,
			BIT(ANA_MICBIAS1_DSCHG_OFFSET) | 0xF, 0);
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, ANA_MICBIAS2,
			BIT(ANA_MICBIAS2_DSCHG_OFFSET) | 0xF,
			0 << ANA_MICBIAS2_DSCHG_OFFSET | ANA_MICBIAS2_ADJ_2_7V);
		da_combine_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS2_PD_BIT), 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, ANA_MICBIAS_ENABLE,
			BIT(ANA_MICBIAS2_PD_BIT),
			BIT(ANA_MICBIAS2_PD_BIT));
		da_combine_update_bits(codec, ANA_MICBIAS2,
			BIT(ANA_MICBIAS2_DSCHG_OFFSET) | 0xF,
			BIT(ANA_MICBIAS2_DSCHG_OFFSET));
		usleep_range(10000, 11000);
		da_combine_update_bits(codec, ANA_MICBIAS2,
			BIT(ANA_MICBIAS2_DSCHG_OFFSET) | 0xF, 0);
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGD("begin");
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_micbias(platform_data->resmgr);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_resmgr_release_micbias(platform_data->resmgr);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

#define MICBIAS_WIDGET \
	SND_SOC_DAPM_MIC("MICBIAS1_MIC", micbias1_power_event), \
	SND_SOC_DAPM_MIC("MICBIAS2_MIC", micbias2_power_event), \
	SND_SOC_DAPM_MIC("HSMICBIAS", hsmicbias_power_event), \

static const struct snd_soc_dapm_widget resource_widgets[] = {
	PLL_WIDGET
	MICBIAS_WIDGET
	SUPPLY_WIDGET
};

static const struct snd_soc_dapm_widget resource_widgets_for_single_kcontrol[] = {
	MICBIAS_WIDGET
	SUPPLY_WIDGET
};

int da_combine_v5_add_resource_widgets(struct snd_soc_component *codec, bool single_kcontrol)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	if (single_kcontrol)
		return snd_soc_dapm_new_controls(dapm, resource_widgets_for_single_kcontrol,
			ARRAY_SIZE(resource_widgets_for_single_kcontrol));
	else
		return snd_soc_dapm_new_controls(dapm, resource_widgets,
			ARRAY_SIZE(resource_widgets));
}

int da_combine_v5_resmgr_init(struct da_combine_v5_platform_data *platform_data)
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
	cfg.enable_micbias = enable_hsmicbias;
	cfg.disable_micbias = disable_hsmicbias;
	cfg.pll_for_reg_access = pll_for_reg_access;

	ret = da_combine_resmgr_init(platform_data->codec, platform_data->cdc_ctrl,
		platform_data->irqmgr, &cfg, &platform_data->resmgr);

	return ret;
}

void da_combine_v5_supply_pll_init(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	enable_regulator(codec);
	buck_init(codec);
	pll48k_config(codec);
	pllmad_config(codec);
	pll44k1_config(codec);
	clk_cfg_init(codec);
}

