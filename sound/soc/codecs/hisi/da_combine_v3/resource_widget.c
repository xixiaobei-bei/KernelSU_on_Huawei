/*
 * resource_widget.c
 *
 * pll resource widget for codec driver
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#include "resource_widget.h"

#include <sound/core.h>
#include <sound/tlv.h>
#include <linux/io.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/version.h>

#include <linux/hisi/audio_log.h>

#include "asoc_adapter.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v3_regs.h"

#include "huawei_platform/audio/anc_hs_interface.h"

#define GPIO_PULL_UP                  1
#define GPIO_PULL_DOWN                0
#define CHIP_EC_VERISON               1
#define ADC_MADIN_TYPE                0x8
#define CHIPID_REG_NUM                6
#define PMU_MCLK_VAL_NUM              5
#define CHECK_PLL48K_LOCK_TIME        5
#define CHECK_PLL44K1_LOCK_TIME       5
#define CHECK_PLLMAD_LOCK_TIME        10
#define MIC_FIXED_NUM                 4


struct mic_power_config {
	unsigned int discharge_bit;
	unsigned int powerdn_bit;
	unsigned int mic_type;
};

static const struct mic_power_config mic_power_regs[] = {
	{DA_COMBINE_V3_MAINMIC_DISCHG_EN_BIT, DA_COMBINE_V3_MAINMIC_PD_BIT, MAIN_MIC},
	{DA_COMBINE_V3_AUXMIC_DISCHG_EN_BIT, DA_COMBINE_V3_AUXMIC_PD_BIT, AUX_MIC},
	{DA_COMBINE_V3_MIC3_DISCHG_EN_BIT, DA_COMBINE_V3_MIC3_PD_BIT, MIC3},
	{DA_COMBINE_V3_MIC4_DISCHG_EN_BIT, DA_COMBINE_V3_MIC4_PD_BIT, MIC4},
};

static enum da_combine_pll_type pll_for_reg_access(struct snd_soc_component *codec,
	unsigned int reg)
{
	if ((reg >= 0x20007200 && reg <= 0x20007fff) ||
		(reg >= 0x20007041 && reg <= 0x2000704c))
		return PLL_HIGH;
	else
		return PLL_NONE;
}

static void main_pll_init(struct snd_soc_component *codec)
{
	/* main pll config */
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG196, 0x24);
	/* mad pll config */
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG197, 0xC0);
	/* main pll trk config */
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG198, 0x88);
	/* pll ibias enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG195,
		1 << DA_COMBINE_V3_PLL_IBIAS_GEN_BIT, 0);
}

static void pllmad_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG10, 0x7);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_WORD_REG1, 0xBB);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_WORF_REG2, 0x80);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG1, 0xC7);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG2, 0x6);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG3, 0xC0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG4, 0x18);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG5, 0x40);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAD_PLL_LOCK_CFG_REG, 0x40);
}

static void pll48k_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG2, 0x7C);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_DIV_REG, 0x14);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG3, 0x4F);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_WORD_REG1, 0x5C);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_WORF_REG2, 0x24);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_FILTER_REG, 0x56);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG4, 0x59);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG5, 0x0);

	if (snd_soc_component_read32(codec, DA_COMBINE_V3_EC_VERSION_REG) & 0x1)
		snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG6, 0x0);
	else
		snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG6, 0x20);

	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_TRACK_REG1, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_TRACK_REG2, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_TRACK_REG3, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG7, 0xA0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG8, 0x40);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG9, 0x8);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_LOCK_CFG_REG, 0x40);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN1_PLL_PHASE_CFG_REG, 0x0);
}

static void pll44k1_config(struct snd_soc_component *codec)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG2, 0x7E);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_DIV_REG, 0x10);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG3, 0x4E);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_WORD_REG1, 0xB3);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_WORF_REG2, 0x34);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_FILTER_REG, 0x56);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG4, 0x59);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG5, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG6, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_TRACK_REG1, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_TRACK_REG2, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_TRACK_REG3, 0x0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG7, 0xA0);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG8, 0x40);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG9, 0x8);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_LOCK_CFG_REG, 0x40);
	snd_soc_component_write(codec, DA_COMBINE_V3_MAIN2_PLL_PHASE_CFG_REG, 0x0);
}

static void clk_cfg_init(struct snd_soc_component *codec)
{
	/* clear pll unlock state enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_PLL_LOCK_CFG_REG,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT, 1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT);
	/* pll lock state check enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_PLL_LOCK_CFG_REG,
		1 << DA_COMBINE_V3_PLL_LOCK_CHECK_BIT, 1 << DA_COMBINE_V3_PLL_LOCK_CHECK_BIT);
	/* 12M288&6M144 clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_AUDIO_CLK_CFG_REG,
		0x3 << DA_COMBINE_V3_ADUIO_12M288CLK_EN_BIT,
		0x3 << DA_COMBINE_V3_ADUIO_12M288CLK_EN_BIT);
	/* clk source select -> main1 pll */
	da_combine_update_bits(codec, DA_COMBINE_V3_PATH_CLK_SEL_REG, 0xFF, 0);
}

static unsigned int get_pmu_mclk_val(unsigned int mclk_addr)
{
	if (mclk_addr == 0)
		return 0;

	return pmic_read_reg(mclk_addr);
}

static bool pll48k_check(struct snd_soc_component *codec)
{
	int i = 0;
	unsigned int regval[CHIPID_REG_NUM];
	unsigned int mclk_value[PMU_MCLK_VAL_NUM];
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	udelay(200);
	/* check pll48k lock state */
	while (!((snd_soc_component_read32(codec, DA_COMBINE_V3_PLL_STATUS_REG) &
		(0x1 << DA_COMBINE_V3_PLL48K_LOCK_BIT)) &&
		(snd_soc_component_read32(codec, DA_COMBINE_VERSION_REG) == DA_COMBINE_VERSION_CS))) {
		udelay(200);
		i++;
		if (i == CHECK_PLL48K_LOCK_TIME) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}

	regval[0] = snd_soc_component_read32(codec, DA_COMBINE_V3_PLL_STATUS_REG);
	mclk_value[0] = get_pmu_mclk_val(priv->board_config.pmu_mclk_addr);
	regval[1] = snd_soc_component_read32(codec, DA_COMBINE_VERSION_REG);
	mclk_value[1] = get_pmu_mclk_val(priv->board_config.pmu_mclk_addr);
	regval[2] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG0);
	mclk_value[2] = get_pmu_mclk_val(priv->board_config.pmu_mclk_addr);
	regval[3] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG1);
	mclk_value[3] = get_pmu_mclk_val(priv->board_config.pmu_mclk_addr);
	regval[4] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG2);
	mclk_value[4] = get_pmu_mclk_val(priv->board_config.pmu_mclk_addr);
	regval[5] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG3);

#ifdef CONFIG_AUDIO_DEBUG
	AUDIO_LOGI("check time is %d, pllstate:0x%x, version:0x%x, chipid0:0x%x,\
		chipid1:0x%x, chipid2:0x%x, chipid3:0x%x",
		i, regval[0], regval[1], regval[2], regval[3], regval[4], regval[5]);
#endif
	AUDIO_LOGI("before version:0x%x, before chipid0-3: 0x%x, 0x%x, 0x%x, 0x%x",
		mclk_value[0], mclk_value[1], mclk_value[2], mclk_value[3], mclk_value[4]);

	da_combine_irq_enable_irq(priv->irqmgr, IRQ_PLL_UNLOCK);
	return true;
}

static int pll48k_turn_on(struct snd_soc_component *codec)
{
	AUDIO_LOGI("turn on pll 48k");
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_RUN_START_BIT,
		1 << DA_COMBINE_V3_MAIN1_PLL_RUN_START_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_PD_BIT, 0);
	udelay(20);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_RST_BIT,
		1 << DA_COMBINE_V3_MAIN1_PLL_RST_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_ISO_18_BIT,
		1 << DA_COMBINE_V3_MAIN1_ISO_18_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_ISO_12_BIT,
		1 << DA_COMBINE_V3_MAIN1_ISO_12_BIT);

	return 0;
}

static int pll48k_turn_off(struct snd_soc_component *codec)
{
	unsigned int regval[5];
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	da_combine_irq_disable_irq(priv->irqmgr, IRQ_PLL_UNLOCK);

	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_PD_BIT,
		1 << DA_COMBINE_V3_MAIN1_PLL_PD_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_ISO_18_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_ISO_12_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_RST_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN1_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN1_PLL_RUN_START_BIT, 0);

	regval[0] = snd_soc_component_read32(codec, DA_COMBINE_VERSION_REG);
	regval[1] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG0);
	regval[2] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG1);
	regval[3] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG2);
	regval[4] = snd_soc_component_read32(codec, DA_COMBINE_CHIP_ID_REG3);

	AUDIO_LOGI("48k off, version:0x%x", regval[0]);
#ifdef CONFIG_AUDIO_DEBUG
	AUDIO_LOGI("chipid0:0x%x, chipid1:0x%x, chipid2:0x%x, chipid3:0x%x",
		regval[1], regval[2], regval[3], regval[4]);
#endif
	/* clear pll unlock state enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_PLL_LOCK_CFG_REG,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT);

	return 0;
}

static bool pll44k1_check(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);
	int i = 0;

	udelay(200);
	/* check pll44k1 lock state */
	while (!(snd_soc_component_read32(codec, DA_COMBINE_V3_PLL_STATUS_REG) &
		(0x1 << DA_COMBINE_V3_PLL44K1_LOCK_BIT))) {
		udelay(200);
		i++;
		if (i == CHECK_PLL44K1_LOCK_TIME) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}

	AUDIO_LOGI("check time is %d", i);
	da_combine_irq_enable_irq(priv->irqmgr, IRQ_PLL44K1_UNLOCK);

	return true;
}

static int pll44k1_turn_on(struct snd_soc_component *codec)
{
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_RUN_START_BIT,
		1 << DA_COMBINE_V3_MAIN2_PLL_RUN_START_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_PD_BIT, 0);
	udelay(20);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_RST_BIT, 1 << DA_COMBINE_V3_MAIN2_PLL_RST_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_ISO_18_BIT, 1 << DA_COMBINE_V3_MAIN2_ISO_18_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_ISO_12_BIT, 1 << DA_COMBINE_V3_MAIN2_ISO_12_BIT);

	return 0;
}

static int pll44k1_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	da_combine_irq_disable_irq(priv->irqmgr, IRQ_PLL44K1_UNLOCK);

	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_ISO_18_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_ISO_12_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_RST_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_PD_BIT,
		1 << DA_COMBINE_V3_MAIN2_PLL_PD_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAIN2_PLL_CFG_REG1,
		1 << DA_COMBINE_V3_MAIN2_PLL_RUN_START_BIT, 0);
	AUDIO_LOGI("44k1 off");
	/* clear pll unlock state enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_PLL_LOCK_CFG_REG,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT);

	return 0;
}

static bool pllmad_check(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);
	int i = 0;

	usleep_range(5000, 5500);
	/* check pllmad lock state */
	while (!(snd_soc_component_read32(codec, DA_COMBINE_V3_PLL_STATUS_REG) &
		(0x1 << DA_COMBINE_V3_PLLMAD_LOCK_BIT))) {
		usleep_range(5000, 5500);
		i++;
		if (i == CHECK_PLLMAD_LOCK_TIME) {
			AUDIO_LOGE("check time is %d", i);
			return false;
		}
	}

	AUDIO_LOGI("check time is %d", i);
	da_combine_irq_enable_irq(priv->irqmgr, IRQ_PLLMAD_UNLOCK);

	return true;
}

static int pllmad_turn_on(struct snd_soc_component *codec)
{
	AUDIO_LOGI("turn on pll mad");
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_RUN_START_BIT,
		1 << DA_COMBINE_V3_MAD_PLL_RUN_START_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_PD_BIT, 0);
	udelay(10);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_RST_BIT, 1 << DA_COMBINE_V3_MAD_PLL_RST_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG6,
		1 << DA_COMBINE_V3_MAD_PLL_ISO_18_BIT,
		1 << DA_COMBINE_V3_MAD_PLL_ISO_18_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_12M288_CLK_SEL_REG, 0x2, 0x2);

	/* adc clk select madpll */
	da_combine_update_bits(codec, DA_COMBINE_V3_PATH_CLK_SEL_REG, 0xFF, 0xFF);

	return 0;
}

static int pllmad_turn_off(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	da_combine_irq_disable_irq(priv->irqmgr, IRQ_PLLMAD_UNLOCK);

	/* adc clk select pll48k */
	da_combine_update_bits(codec, DA_COMBINE_V3_PATH_CLK_SEL_REG, 0xFF, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_12M288_CLK_SEL_REG, 0x2, 0x0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_PD_BIT, 1 << DA_COMBINE_V3_MAD_PLL_PD_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_RST_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG6,
		1 << DA_COMBINE_V3_MAD_PLL_ISO_18_BIT, 0);
	da_combine_update_bits(codec, DA_COMBINE_V3_MAD_PLL_CFG_REG9,
		1 << DA_COMBINE_V3_MAD_PLL_RUN_START_BIT, 0);
	/* clear pll unlock state enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_PLL_LOCK_CFG_REG,
		1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT, 1 << DA_COMBINE_V3_PLL_LOCK_CLR_BIT);

	AUDIO_LOGI("mad off");

	return 0;
}

static int enable_ibias(struct snd_soc_component *codec)
{
	AUDIO_LOGI("enable ibias");
	/* vref_fast on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_VREF_FAST_BIT, 1 << DA_COMBINE_V3_VREF_FAST_BIT);
	/* vref select 1k */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x03 << DA_COMBINE_V3_VREFSEL_BIT, 0x03 << DA_COMBINE_V3_VREFSEL_BIT);
	msleep(30);
	/* vref_fast off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_VREF_FAST_BIT, 0);
	/* vref select 100k */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x03 << DA_COMBINE_V3_VREFSEL_BIT, 0x01 << DA_COMBINE_V3_VREFSEL_BIT);
	/* ibias on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_IBIAS_PD_BIT, 0);

	return 0;
}

static int disable_ibias(struct snd_soc_component *codec)
{
	AUDIO_LOGI("disable ibias");

	/* ibias off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_IBIAS_PD_BIT, 1 << DA_COMBINE_V3_IBIAS_PD_BIT);
	/* vref select pd */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x03 << DA_COMBINE_V3_VREFSEL_BIT, 0);

	return 0;
}

static int enable_ibias_bandgap(struct snd_soc_component *codec)
{
	/* vref_fast on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_VREF_FAST_BIT, 1 << DA_COMBINE_V3_VREF_FAST_BIT);
	/* vref select 1k */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x03 << DA_COMBINE_V3_VREFSEL_BIT, 0x03 << DA_COMBINE_V3_VREFSEL_BIT);
	usleep_range(10000, 11000);
	/* vref_fast off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_VREF_FAST_BIT, 0);
	/* bandgap select & on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0, 0x61, 0x1);
	/* ibias on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_IBIAS_PD_BIT, 0);

	return 0;
}

static int disable_ibias_bandgap(struct snd_soc_component *codec)
{
	/* ibias off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		1 << DA_COMBINE_V3_IBIAS_PD_BIT, 1 << DA_COMBINE_V3_IBIAS_PD_BIT);
	/* vref select pd */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x03 << DA_COMBINE_V3_VREFSEL_BIT, 0);
	/* res select & off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG0,
		0x61, 0x60);

	return 0;
}

static void hs_high_resistance_enable(struct snd_soc_component *codec,
	bool enable)
{
	bool buck1_poweron = false;

	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return;
	}

	/**
	 * buck1 power on, rcv is using, if set the codec hs high resistance,
	 * TYPEC analog hs will recognize as 4-pole when reverse plug-in,
	 * recognizing error.
	 **/
	buck1_poweron = snd_soc_component_read32(codec, DA_COMBINE_V3_ANALOG_REG129) & 0x1;
	if (buck1_poweron) {
		AUDIO_LOGE("buck1 power on, do not set the hs high resistance");
		return;
	}

	if (enable) {
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG43,
			0x3F << DA_COMBINE_V3_HPL_POP_PULL_2_0_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG44,
			0x3F << DA_COMBINE_V3_HPR_POP_PULL_2_0_BIT, 0);
	} else {
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG43,
			0x3F<<DA_COMBINE_V3_HPL_POP_PULL_2_0_BIT,
			0x8<<DA_COMBINE_V3_HPL_POP_PULL_2_0_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG44,
			0x3F<<DA_COMBINE_V3_HPR_POP_PULL_2_0_BIT,
			0x8<<DA_COMBINE_V3_HPR_POP_PULL_2_0_BIT);
	}
}

static int enable_micbias(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(priv->mbhc);
	/* eco off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_V3_HS_ECO_EN_BIT, 0);
	AUDIO_LOGI("eco disable");
	/* mbhc cmp on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_V3_MBHD_COMP_PD_BIT, 0);
	/* hsmic chg */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_V3_HSMIC_DISCHG_EN_BIT, 0);
	/* hsmic on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_V3_HSMIC_PD_BIT, 0);

	AUDIO_LOGI("hs micbias enable");
	msleep(20);
	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(priv->mbhc);

	return 0;
}

static int disable_micbias(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv =
		snd_soc_component_get_drvdata(codec);

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(priv->mbhc);
	/* hsmic pd */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_V3_HSMIC_PD_BIT, 1 << DA_COMBINE_V3_HSMIC_PD_BIT);
	/* hsmic dischg */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_V3_HSMIC_DISCHG_EN_BIT,
		1 << DA_COMBINE_V3_HSMIC_DISCHG_EN_BIT);
	usleep_range(15000, 16000);
	/* hsmic chg */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_V3_HSMIC_DISCHG_EN_BIT, 0);
	AUDIO_LOGI("hs micbias disable");

	/* eco on & eco auto saradc on */
	/* mbhc cmp off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_V3_MBHD_COMP_PD_BIT,
		1 << DA_COMBINE_V3_MBHD_COMP_PD_BIT);
	/* eco on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_V3_HS_ECO_EN_BIT,
		1 << DA_COMBINE_V3_HS_ECO_EN_BIT);
	AUDIO_LOGI("eco enable");
	msleep(20);
	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(priv->mbhc);

	return 0;
}

static int pll_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret;
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_pll(priv->resmgr, PLL_HIGH);
		if (!priv->cdc_ctrl->pm_runtime_support) {
			mdelay(2);
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_DA_COMBINE_V3,
				SLIMBUS_FRAMER_CODEC);
			if (ret)
				AUDIO_LOGE("slimbus switch framer failed");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (!priv->cdc_ctrl->pm_runtime_support) {
			/* slimbus master framer is soc */
			ret = slimbus_switch_framer(SLIMBUS_DEVICE_DA_COMBINE_V3,
				SLIMBUS_FRAMER_SOC);
			if (ret)
				AUDIO_LOGE("slimbus switch framer to soc failed");
			mdelay(2);
		}
		da_combine_resmgr_release_pll(priv->resmgr, PLL_HIGH);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int mad_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *platform_data =
		snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_resmgr_request_pll(platform_data->resmgr, PLL_LOW);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_resmgr_release_pll(platform_data->resmgr, PLL_LOW);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static void ir_gpio_switch(unsigned int gpio_id, int val)
{
	int ret = 0;

	ret = gpio_request(gpio_id, "da_combine_ir_gpio");
	if (ret != 0) {
		AUDIO_LOGE("gpio request fail %u", gpio_id);
		return;
	}

	ret = gpio_direction_output(gpio_id, val);
	if (ret != 0)
		AUDIO_LOGE("could not set gpio:%u direction out", gpio_id);

	gpio_free(gpio_id);
}

static int ir_tx_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	if (priv->board_config.ir_gpio_id == 0) {
		AUDIO_LOGW("can't get ir switch gpio, maybe not support audio ir transmit");
		return 0;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_106,
			3 << DA_COMBINE_V3_CODEC_ANA_RWREG_106_OCP_BIT,
			2 << DA_COMBINE_V3_CODEC_ANA_RWREG_106_OCP_BIT);
		if (priv->board_config.ear_ir_gpio_id != 0) {
			ir_gpio_switch(priv->board_config.ear_ir_gpio_id,
				GPIO_PULL_DOWN);
			AUDIO_LOGI("ear ir switch gpio %d down",
				priv->board_config.ear_ir_gpio_id);
		}
		ir_gpio_switch(priv->board_config.ir_gpio_id, GPIO_PULL_UP);
		AUDIO_LOGI("ir switch gpio %d up",
			priv->board_config.ir_gpio_id);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_CODEC_ANA_RWREG_106,
			3 << DA_COMBINE_V3_CODEC_ANA_RWREG_106_OCP_BIT,
			0 << DA_COMBINE_V3_CODEC_ANA_RWREG_106_OCP_BIT);
		ir_gpio_switch(priv->board_config.ir_gpio_id, GPIO_PULL_DOWN);
		AUDIO_LOGI("ir switch gpio %d down",
			priv->board_config.ir_gpio_id);
		if (priv->board_config.ear_ir_gpio_id != 0) {
			ir_gpio_switch(priv->board_config.ear_ir_gpio_id,
				GPIO_PULL_UP);
			AUDIO_LOGI("ear ir switch gpio %d up",
				priv->board_config.ear_ir_gpio_id);
		}
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void request_dp_clk(struct snd_soc_component *codec, bool enable)
{
	struct da_combine_v3_platform_data *data = NULL;

	data = snd_soc_component_get_drvdata(codec);
	mutex_lock(&data->impdet_dapm_mutex);
	if (enable) {
		data->dp_clk_num++;
		if (data->dp_clk_num == 1) {
			da_combine_update_bits(codec, DA_COMBINE_V3_CODEC_DPCLK_REG,
				0x1 << DA_COMBINE_V3_CODEC_DPCLK_BIT,
				0x1 << DA_COMBINE_V3_CODEC_DPCLK_BIT);
		}
	} else {
		if (data->dp_clk_num == 0) {
			AUDIO_LOGE("dp clk num err");
			mutex_unlock(&data->impdet_dapm_mutex);
			return;
		}
		data->dp_clk_num--;
		if (data->dp_clk_num == 0) {
			da_combine_update_bits(codec, DA_COMBINE_V3_CODEC_DPCLK_REG,
				0x1 << DA_COMBINE_V3_CODEC_DPCLK_BIT,
				0x0 << DA_COMBINE_V3_CODEC_DPCLK_BIT);
		}
	}
	mutex_unlock(&data->impdet_dapm_mutex);
}

static int dp_clk_supply_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

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

static int s2_rx_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			0x1 << DA_COMBINE_V3_S2_RX_EN_BIT,
			0x1 << DA_COMBINE_V3_S2_RX_EN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_CFG_REG,
			0x1 << DA_COMBINE_V3_S2_RX_EN_BIT, 0);
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_CLK_CFG_REG,
			1 << DA_COMBINE_V3_S4_CLKEN_BIT,
			1 << DA_COMBINE_V3_S4_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_CFG_REG,
			1 << DA_COMBINE_V3_S4_RX_EN_BIT,
			1 << DA_COMBINE_V3_S4_RX_EN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_CLK_CFG_REG,
			1 << DA_COMBINE_V3_S4_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_S4_CFG_REG,
			1 << DA_COMBINE_V3_S4_RX_EN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static void request_cp1(struct snd_soc_component *codec, bool enable)
{
	struct da_combine_v3_platform_data *data = NULL;

	data = snd_soc_component_get_drvdata(codec);
	mutex_lock(&data->impdet_dapm_mutex);
	if (enable) {
		data->cp1_num++;
		if (data->cp1_num == 1) {
			/* buck1 enable */
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG129,
				1 << DA_COMBINE_V3_BUCK1_ENP_BIT,
				1 << DA_COMBINE_V3_BUCK1_ENP_BIT);
			/* classH enable */
			da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
				1 << DA_COMBINE_V3_CLASSH_CLKEN_BIT,
				1 << DA_COMBINE_V3_CLASSH_CLKEN_BIT);
		}
	} else {
		if (data->cp1_num == 0) {
			AUDIO_LOGE("cp1 num err");
			mutex_unlock(&data->impdet_dapm_mutex);
			return;
		}
		data->cp1_num--;
		if (data->cp1_num == 0) {
			/* classH disable */
			da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
				1 << DA_COMBINE_V3_CLASSH_CLKEN_BIT, 0);
			/* buck1 disable */
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG129,
				1 << DA_COMBINE_V3_BUCK1_ENP_BIT, 0);
		}
	}
	mutex_unlock(&data->impdet_dapm_mutex);
}

static void request_cp2(struct snd_soc_component *codec, bool enable)
{
	struct da_combine_v3_platform_data *data = NULL;

	data = snd_soc_component_get_drvdata(codec);
	mutex_lock(&data->impdet_dapm_mutex);
	if (enable) {
		data->cp2_num++;
		if (data->cp2_num == 1) {
			/* buck2 enable */
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG129,
				1 << DA_COMBINE_V3_BUCK2_ENP_BIT,
				1 << DA_COMBINE_V3_BUCK2_ENP_BIT);
		}
	} else {
		if (data->cp2_num == 0) {
			AUDIO_LOGE("cp2 num err");
			mutex_unlock(&data->impdet_dapm_mutex);
			return;
		}
		data->cp2_num--;
		if (data->cp2_num == 0) {
			/* buck2 disable */
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG129,
				1 << DA_COMBINE_V3_BUCK2_ENP_BIT, 0);
		}
	}
	mutex_unlock(&data->impdet_dapm_mutex);
}

static int cp1_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		request_cp1(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		request_cp1(codec, false);
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	AUDIO_LOGI("power event: 0x%x", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		request_cp2(codec, true);
		usleep_range(5000, 5500);
		break;
	case SND_SOC_DAPM_POST_PMD:
		request_cp2(codec, false);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

#define SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY_S("PLL CLK", \
		0, SND_SOC_NOPM, 0, 0, pll_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("MAD CLK", \
		0, SND_SOC_NOPM, 0, 0, mad_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("IR TX CLK", \
		0, SND_SOC_NOPM, 0, 0, ir_tx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("DP CLK", \
		1, SND_SOC_NOPM, 0, 0, dp_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S2 IF CLK", \
		1, DA_COMBINE_V3_S2_CLK_CFG_REG, DA_COMBINE_V3_S2_CLKEN_BIT, 0, NULL, 0), \
	SND_SOC_DAPM_SUPPLY_S("S2 RX", \
		2, SND_SOC_NOPM, 0, 0, s2_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S4 RX", \
		2, SND_SOC_NOPM, 0, 0, s4_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP1", \
		3, SND_SOC_NOPM, 0, 0, cp1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP2", \
		4, SND_SOC_NOPM, 0, 0, cp2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static void set_allmic_gain_zero(struct da_combine_v3_platform_data *data)
{
	snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC0L_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC0R_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC1L_05PGA_GAIN_REG, 0x78);
	snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC1R_05PGA_GAIN_REG, 0x78);
}

static void anc_hs_control_charge(struct snd_soc_component *codec, bool enable)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

#ifdef CONFIG_ANC_HS_INTERFACE
	if (!check_anc_hs_interface_support())
		return;

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(priv->mbhc);
	if (enable) {
		AUDIO_LOGI("anc hs control charge enable");
		anc_hs_interface_force_charge(ANC_HS_ENABLE_CHARGE);
	} else {
		AUDIO_LOGI("anc hs control charge disable");
		anc_hs_interface_force_charge(ANC_HS_DISABLE_CHARGE);
	}
	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(priv->mbhc);
#endif

	OUT_FUNCTION;
}

static int hsmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* set pga gain 0 */
		set_allmic_gain_zero(data);
		/* stop anc charge */
		anc_hs_control_charge(codec, false);
		/* headset micbias on */
		da_combine_resmgr_request_micbias(data->resmgr);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* headset micbias off */
		da_combine_resmgr_release_micbias(data->resmgr);
		/* resume anc charge */
		anc_hs_control_charge(codec, true);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static void write_mic_fixed_value(struct da_combine_v3_platform_data *data,
	unsigned int din_sel, unsigned int reg, unsigned int type)
{
	if (din_sel == ADC_MADIN_TYPE || din_sel == type)
		snd_soc_component_write(data->codec, reg,
			data->mic_fixed_value[type]);
}

static void set_mic_gain_fixed(struct da_combine_v3_platform_data *data)
{
	unsigned int i;
	union sc_adc_ctrl_reg adc0_mux;
	union sc_adc_ctrl_reg adc1_mux;
	unsigned int micpd_reg =
		snd_soc_component_read32(data->codec, DA_COMBINE_V3_ANALOG_REG089);

	/* adcx_mux means ADCxL/R mux reg */
	adc0_mux = (union sc_adc_ctrl_reg)snd_soc_component_read32(data->codec, DA_COMBINE_V3_ADC0_MUX_SEL);
	adc1_mux = (union sc_adc_ctrl_reg)snd_soc_component_read32(data->codec, DA_COMBINE_V3_ADC1_MUX_SEL);
	AUDIO_LOGI("adc0_mux = 0x%x, adc1_mux = 0x%x, micpd_reg = 0x%x",
		adc0_mux, adc1_mux, micpd_reg);

	for (i = 0; i < MIC_FIXED_NUM; i++) {
		if ((micpd_reg & (1 << i)) == 0) {
			write_mic_fixed_value(data, adc0_mux.reg.r_din_sel,
				DA_COMBINE_V3_ADC0L_05PGA_GAIN_REG, i);
			write_mic_fixed_value(data, adc0_mux.reg.l_din_sel,
				DA_COMBINE_V3_ADC0R_05PGA_GAIN_REG, i);
			write_mic_fixed_value(data, adc1_mux.reg.r_din_sel,
				DA_COMBINE_V3_ADC1L_05PGA_GAIN_REG, i);
			write_mic_fixed_value(data, adc1_mux.reg.r_din_sel,
				DA_COMBINE_V3_ADC1R_05PGA_GAIN_REG, i);
		}
	}
}

static void set_mic_gain_zero(struct da_combine_v3_platform_data *data, enum mic_type type)
{
	union sc_adc_ctrl_reg dc0_mux =
		(union sc_adc_ctrl_reg)snd_soc_component_read32(data->codec, DA_COMBINE_V3_ADC0_MUX_SEL);
	union sc_adc_ctrl_reg dc1_mux =
		(union sc_adc_ctrl_reg)snd_soc_component_read32(data->codec, DA_COMBINE_V3_ADC1_MUX_SEL);

	if (dc0_mux.reg.r_din_sel == type)
		snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC0L_05PGA_GAIN_REG, 0x78);
	if (dc0_mux.reg.l_din_sel == type)
		snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC0R_05PGA_GAIN_REG, 0x78);
	if (dc1_mux.reg.r_din_sel == type)
		snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC1L_05PGA_GAIN_REG, 0x78);
	if (dc1_mux.reg.l_din_sel == type)
		snd_soc_component_write(data->codec, DA_COMBINE_V3_ADC1R_05PGA_GAIN_REG, 0x78);
}

static void mic_poweron_enable(struct snd_soc_dapm_widget *w,
	int event, const struct mic_power_config *mic_reg)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct da_combine_v3_platform_data *data = snd_soc_component_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (mic_reg->mic_type == MAIN_MIC) {
#ifdef CONFIG_ANC_HS_INTERFACE
			if (check_anc_hs_interface_support())
				anc_hs_interface_5v_control(ANC_HS_ENABLE_5VOLTATE);
#endif
		}
		/* mainmic discharge disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
			1 << mic_reg->discharge_bit, 0);
		/* mainmic power on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
			1 << mic_reg->powerdn_bit, 0);
		/* set mainmic fixed value to pga used by mainmic */
		set_mic_gain_fixed(data);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* mainmic power off */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
			1 << mic_reg->powerdn_bit, 1 << mic_reg->powerdn_bit);
		/* mainmic discharge enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
			1 << mic_reg->discharge_bit,
			1 << mic_reg->discharge_bit);
		usleep_range(10000, 11000);
		/* mainmic discharge disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
			1 << mic_reg->discharge_bit, 0);
		/* set 0 to pga */
		set_mic_gain_zero(data, mic_reg->mic_type);

		if (mic_reg->mic_type == MAIN_MIC) {
#ifdef CONFIG_ANC_HS_INTERFACE
			if (check_anc_hs_interface_support())
				anc_hs_interface_5v_control(ANC_HS_DISABLE_5VOLTATE);
#endif
		}
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static int mainmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	mic_poweron_enable(w, event, &mic_power_regs[MAIN_MIC]);

	return 0;
}

static int auxmic_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	mic_poweron_enable(w, event, &mic_power_regs[AUX_MIC]);

	return 0;
}

static int mic3_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	mic_poweron_enable(w, event, &mic_power_regs[MIC3]);

	return 0;
}

static int mic4_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	mic_poweron_enable(w, event, &mic_power_regs[MIC4]);

	return 0;
}

#define MICBIAS_WIDGET \
	SND_SOC_DAPM_MIC("HS MIC", hsmic_power_event), \
	SND_SOC_DAPM_MIC("MAIN MIC", mainmic_power_event), \
	SND_SOC_DAPM_MIC("AUX MIC", auxmic_power_event), \
	SND_SOC_DAPM_MIC("MIC3 IN", mic3_power_event), \
	SND_SOC_DAPM_MIC("MIC4 IN", mic4_power_event), \

static int mad_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mad pga on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG83,
			1 << DA_COMBINE_V3_MAD_PGA_PD_BIT, 0);
		/* mad pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG84,
			1 << DA_COMBINE_V3_MAD_MUTE1_BIT, 0);
		/* mad pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG84,
			1 << DA_COMBINE_V3_MAD_MUTE2_BIT, 0);
		/* mad adc on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG83,
			1 << DA_COMBINE_V3_MAD_ADC_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* mad adc off */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG83,
			1 << DA_COMBINE_V3_MAD_ADC_PD_BIT,
			1 << DA_COMBINE_V3_MAD_ADC_PD_BIT);
		/* mad pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG84,
			1 << DA_COMBINE_V3_MAD_MUTE2_BIT,
			1 << DA_COMBINE_V3_MAD_MUTE2_BIT);
		/* mad pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG84,
			1 << DA_COMBINE_V3_MAD_MUTE1_BIT,
			1 << DA_COMBINE_V3_MAD_MUTE1_BIT);
		/* mad pga pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG83,
			1 << DA_COMBINE_V3_MAD_PGA_PD_BIT,
			1 << DA_COMBINE_V3_MAD_PGA_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int main_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* main pga on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_MAIN_PGA_PD_BIT, 0);
		/* main pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_MAIN_MUTE1_BIT, 0);
		/* main pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG23,
			1 << DA_COMBINE_V3_MAIN_MUTE2_BIT, 0);
		/* adc0l on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC0L_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* adc0l off */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC0L_PD_BIT,
			1 << DA_COMBINE_V3_ADC0L_PD_BIT);
		/* main pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG23,
			1 << DA_COMBINE_V3_MAIN_MUTE2_BIT,
			1 << DA_COMBINE_V3_MAIN_MUTE2_BIT);
		/* main pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_MAIN_MUTE1_BIT,
			1 << DA_COMBINE_V3_MAIN_MUTE1_BIT);
		/* main pga pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_MAIN_PGA_PD_BIT,
			1 << DA_COMBINE_V3_MAIN_PGA_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int aux_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* aux pga on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_AUX_PGA_PD_BIT, 0);
		/* aux pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG22,
			1 << DA_COMBINE_V3_AUX_MUTE1_BIT, 0);
		/* aux pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG22,
			1 << DA_COMBINE_V3_AUX_MUTE2_BIT, 0);
		/* adc0r on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC0R_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* adc0r pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC0R_PD_BIT,
			1 << DA_COMBINE_V3_ADC0R_PD_BIT);
		/* aux pga mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG22,
			1 << DA_COMBINE_V3_AUX_MUTE2_BIT,
			1 << DA_COMBINE_V3_AUX_MUTE2_BIT);
		/* aux pga mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG22,
			1 << DA_COMBINE_V3_AUX_MUTE1_BIT,
			1 << DA_COMBINE_V3_AUX_MUTE1_BIT);
		/* aux pga pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_AUX_PGA_PD_BIT,
			1 << DA_COMBINE_V3_AUX_PGA_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int lineinl_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* lineinl pga on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_LINEINL_PGA_PD_BIT, 0);
		/* lineinl mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_LINEINL_MUTE1_BIT, 0);
		/* lineinl mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG20,
			1 << DA_COMBINE_V3_LINEINL_MUTE2_BIT, 0);
		/* adc1l on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC1L_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* adc1l pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC1L_PD_BIT, 1 << DA_COMBINE_V3_ADC1L_PD_BIT);
		/* lineinl mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG20,
			1 << DA_COMBINE_V3_LINEINL_MUTE2_BIT,
			1 << DA_COMBINE_V3_LINEINL_MUTE2_BIT);
		/* lineinl mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_LINEINL_MUTE1_BIT,
			1 << DA_COMBINE_V3_LINEINL_MUTE1_BIT);
		/* lineinl pga pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_LINEINL_PGA_PD_BIT,
			1 << DA_COMBINE_V3_LINEINL_PGA_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int lineinr_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* lineinr pga on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_LINEINR_PGA_PD_BIT, 0);
		/* lineinr mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_LINEINR_MUTE1_BIT, 0);
		/* lineinr mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG21,
			1 << DA_COMBINE_V3_LINEINR_MUTE2_BIT, 0);
		/* adc1r on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC1R_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* adc1r pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_ADC1R_PD_BIT, 1 << DA_COMBINE_V3_ADC1R_PD_BIT);
		/* lineinr mute2 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG21,
			1 << DA_COMBINE_V3_LINEINR_MUTE2_BIT,
			1 << DA_COMBINE_V3_LINEINR_MUTE2_BIT);
		/* lineinr mute1 */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG24,
			1 << DA_COMBINE_V3_LINEINR_MUTE1_BIT,
			1 << DA_COMBINE_V3_LINEINR_MUTE1_BIT);
		/* lineinr pga pd */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG1,
			1 << DA_COMBINE_V3_LINEINR_PGA_PD_BIT,
			1 << DA_COMBINE_V3_LINEINR_PGA_PD_BIT);
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
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IL_PGA_CLKEN_BIT),
			BIT(DA_COMBINE_V3_S2_IL_PGA_CLKEN_BIT));
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IL_SRC_CLKEN_BIT),
			BIT(DA_COMBINE_V3_S2_IL_SRC_CLKEN_BIT));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IL_SRC_CLKEN_BIT), 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IL_PGA_CLKEN_BIT), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int s2_ir_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IR_PGA_CLKEN_BIT),
			BIT(DA_COMBINE_V3_S2_IR_PGA_CLKEN_BIT));
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IR_SRC_CLKEN_BIT),
			BIT(DA_COMBINE_V3_S2_IR_SRC_CLKEN_BIT));
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IR_SRC_CLKEN_BIT), 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_S2_PORT_CLKEN_REG,
			BIT(DA_COMBINE_V3_S2_IR_PGA_CLKEN_BIT), 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacl_flt_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacl src clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			0x3 << DA_COMBINE_V3_DACL_SRC_CLKEN_BIT,
			0x3 << DA_COMBINE_V3_DACL_SRC_CLKEN_BIT);
		/* dacl clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACL_CLKEN_BIT, 1 << DA_COMBINE_V3_DACL_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacl clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACL_CLKEN_BIT, 0);
		/* dacl src clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			0x3 << DA_COMBINE_V3_DACL_SRC_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacr_flt_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacr clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACR_CLKEN_BIT, 1 << DA_COMBINE_V3_DACR_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacr clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACR_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacl_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacl pga clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT);
		/* dacl src up16 clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT);
		/* dacl sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT);
		/* dsdl sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacl sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT, 0);
		/* dsdl sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT, 0);
		/* dacl src up16 clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT, 0);
		/* dacl pga clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacr_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacr pga clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_DACR_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACR_PGA_CLKEN_BIT);
		/* dacr src up16 clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT);
		/* dacr sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT);
		/* dsdr sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacr sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT, 0);
		/* dsdr sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT, 0);
		/* dacr src up16 clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT, 0);
		/* dacrl pga clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_DACR_PGA_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacls_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacls pga enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG11,
			1 << DA_COMBINE_V3_DACLS_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACLS_PGA_CLKEN_BIT);
		/* dacls clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACLS_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACLS_CLKEN_BIT);
		/* dacls src up16 clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACLS_UP16_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACLS_UP16_CLKEN_BIT);
		/* dacls sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACLS_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACLS_SDM_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacls sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACLS_SDM_CLKEN_BIT, 0);
		/* dacls src up16 clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACLS_UP16_CLKEN_BIT, 0);
		/* dacls clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACLS_CLKEN_BIT, 0);
		/* dacls pga disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG11,
			1 << DA_COMBINE_V3_DACLS_PGA_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int dacrs_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* dacrs clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACRS_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACRS_CLKEN_BIT);
		/* dacrs src up16 clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACRS_UP16_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACRS_UP16_CLKEN_BIT);
		/* dacrs sdm clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACRS_SDM_CLKEN_BIT,
			1 << DA_COMBINE_V3_DACRS_SDM_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacrs sdm clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
			1 << DA_COMBINE_V3_DACRS_SDM_CLKEN_BIT, 0);
		/* dacrs rsrc up16 clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_DACRS_UP16_CLKEN_BIT, 0);
		/* dacrs clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_DACRS_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adc1r_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC1R_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC1R_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			1 << DA_COMBINE_V3_ADC1R_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC1R_PGA_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			1 << DA_COMBINE_V3_ADC1R_PGA_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC1R_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adc1l_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC1L_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC1L_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			1 << DA_COMBINE_V3_ADC1L_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC1L_PGA_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG07,
			1 << DA_COMBINE_V3_ADC1L_PGA_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC1L_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adc0r_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC0R_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC0R_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_ADC0R_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC0R_PGA_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_ADC0R_PGA_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC0R_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int adc0l_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC0L_CLKEN_BIT,
			1 << DA_COMBINE_V3_ADC0L_CLKEN_BIT);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1<<DA_COMBINE_V3_ADC0L_PGA_CLKEN_BIT,
			1<<DA_COMBINE_V3_ADC0L_PGA_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
			1 << DA_COMBINE_V3_ADC0L_PGA_CLKEN_BIT, 0);
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG09,
			1 << DA_COMBINE_V3_ADC0L_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int sidetone_pga_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* sidetone pga & srcclk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_SIDE_PGA_CLKEN_BIT,
			1 << DA_COMBINE_V3_SIDE_PGA_CLKEN_BIT);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* sidetone pga & src clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
			1 << DA_COMBINE_V3_SIDE_PGA_CLKEN_BIT, 0);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

#define PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("MAD PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		mad_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MAIN PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		main_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("AUX PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		aux_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("LINEINL PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		lineinl_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("LINEINR PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		lineinr_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S1 IL PGA", \
		0, DA_COMBINE_V3_S1_PORT_CLKEN_REG, DA_COMBINE_V3_S1_IL_PGA_CLKEN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S1 IR PGA", \
		0, DA_COMBINE_V3_S1_PORT_CLKEN_REG, DA_COMBINE_V3_S1_IR_PGA_CLKEN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S2 IL PGA", \
		0, SND_SOC_NOPM, 0, 0, s2_il_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S2 IR PGA", \
		0, SND_SOC_NOPM, 0, 0, s2_ir_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S3 IL PGA", \
		0, DA_COMBINE_V3_S3_PORT_CLKEN_REG, DA_COMBINE_V3_S3_IL_PGA_CLKEN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S3 IR PGA", \
		0, DA_COMBINE_V3_S3_PORT_CLKEN_REG, DA_COMBINE_V3_S3_IR_PGA_CLKEN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("DACL FLT PGA", \
		1, SND_SOC_NOPM, 0, 0, dacl_flt_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("DACR FLT PGA", \
		1, SND_SOC_NOPM, 0, 0, dacr_flt_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("DACL PGA", \
		1, SND_SOC_NOPM, 0, 0, dacl_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("DACR PGA", \
		1, SND_SOC_NOPM, 0, 0, dacr_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("DACSL PGA", \
		1, SND_SOC_NOPM, 0, 0, dacls_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("DACSR PGA", \
		1, SND_SOC_NOPM, 0, 0, dacrs_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC1R", \
		1, SND_SOC_NOPM, 0, 0, adc1r_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC1L", \
		1, SND_SOC_NOPM, 0, 0, adc1l_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC0R", \
		1, SND_SOC_NOPM, 0, 0, adc0r_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADC0L", \
		1, SND_SOC_NOPM, 0, 0, adc0l_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("SIDETONE", \
		1, SND_SOC_NOPM, 0, 0, sidetone_pga_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget resource_widgets[] = {
	SUPPLY_WIDGET
	MICBIAS_WIDGET
	PGA_WIDGET
};

int add_resource_widgets(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, resource_widgets,
		ARRAY_SIZE(resource_widgets));
}

void da_combine_v3_request_cp_dp_clk(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("null");
		return;
	}

	/* dp clk enable */
	request_dp_clk(codec, true);
	/* cp1 enable */
	request_cp1(codec, true);
	/* cp2 enable */
	request_cp2(codec, true);
}
void da_combine_v3_release_cp_dp_clk(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("null");
		return;
	}

	/* cp2 disable */
	request_cp2(codec, false);
	/* cp1 disable */
	request_cp1(codec, false);
	/* dp clk disable */
	request_dp_clk(codec, false);
}

int resmgr_init(struct da_combine_v3_platform_data *platform_data)
{
	int ret;
	unsigned int val = 0;
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
	cfg.enable_micbias = enable_micbias;
	cfg.disable_micbias = disable_micbias;
	cfg.pll_for_reg_access = pll_for_reg_access;

	val = hi_cdcctrl_reg_read(platform_data->cdc_ctrl,
			DA_COMBINE_V3_EC_VERSION_REG | BASE_OFFSET);
	if ((val & CHIP_EC_VERISON) != 0) {
		cfg.enable_ibias = enable_ibias;
		cfg.disable_ibias = disable_ibias;
	} else {
		cfg.enable_ibias = enable_ibias_bandgap;
		cfg.disable_ibias = disable_ibias_bandgap;
	}

	cfg.da_combine_hs_high_resistance_enable = hs_high_resistance_enable;

	ret = da_combine_resmgr_init(platform_data->codec, platform_data->cdc_ctrl,
		platform_data->irqmgr, &cfg, &platform_data->resmgr);

	return ret;
}

void supply_pll_init(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return;
	}

	main_pll_init(codec);
	pll48k_config(codec);
	pllmad_config(codec);
	pll44k1_config(codec);
	clk_cfg_init(codec);
}

