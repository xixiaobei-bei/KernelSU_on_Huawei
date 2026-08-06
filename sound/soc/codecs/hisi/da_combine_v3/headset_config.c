/*
 * headset_config.c
 *
 * headset init config
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

#include "headset_config.h"

#include <sound/core.h>
#include <sound/tlv.h>
#include <linux/version.h>

#include <linux/hisi/audio_log.h>

#include "asoc_adapter.h"
#include "linux/hisi/da_combine/da_combine_v3_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "path_widget.h"
#include "resource_widget.h"
#include "switch_widget.h"


#define HS_VOLTAGE_COEFFICIENT_VAL   1800
#define HS_VOLUME_VAL_LOW_THRESHOLD  100
#define HS_VOLUME_VAL_HIGH_THRESHOLD 300
#define HS_MAX_SARADC_VAL            0xFF
#define CHECK_SARADC_READY_TIMES     3
#define INVALID_REG_VALUE            0xFFFFFFFF

static struct da_combine_hs_cfg hs_cfg = {0};

void hp_classh_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	if (platform_data->board_config.hp_high_low_change_enable) {
		headphone_low_mode_shadow_config(codec);
		/* headphone mode select low mode */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT,
			0 << DA_COMBINE_V3_HP_SRB_CTRL_BIT);
		/* headphone feed back disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_FB_EN_BIT, 0 << DA_COMBINE_V3_HP_FB_EN_BIT);
		/* headphone feed back mode config */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT,
			0 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT);
		/* headphone ib05 current init */
		snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x22);
		/* enable the config for shadow gain */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG, 0x1, 0x1);
		/* enable the trigger for shadow gain */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
			0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
			0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
	} else {
		headphone_high_mode_shadow_config(codec);
		/* headphone ib05 current init */
		snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x66);
		/* headphone feed back mode config */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT,
			0x1 << DA_COMBINE_V3_HP_FB_STB_CTRL_BIT);
		/* headphone feed back enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_FB_EN_BIT, 0x1 << DA_COMBINE_V3_HP_FB_EN_BIT);
		/* headphone mode select high mode */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG48,
			0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT,
			0x1 << DA_COMBINE_V3_HP_SRB_CTRL_BIT);
	}
}

static void hs_mbhc_on(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	/* mask btn irqs */
	da_combine_irq_mask_btn_irqs(priv->mbhc);

	/* saradc cfg */
	snd_soc_component_write(codec,  DA_COMBINE_V3_SAR_CONFIG_REG, 0x7C);
	/* mbhc on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_MBHC_ON_BIT, 0);

	msleep(30);
	/* unmask btn irqs */
	da_combine_irq_unmask_btn_irqs(priv->mbhc);

	msleep(120);
}

static void hs_mbhc_off(struct snd_soc_component *codec)
{
	/* eco off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG093,
		1 << DA_COMBINE_MICBIAS_ECO_ON_BIT, 0);
	AUDIO_LOGI("eco disable\n");

	/* mbhc cmp off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		1 << DA_COMBINE_MBHC_ON_BIT, 1 << DA_COMBINE_MBHC_ON_BIT);
}

static void hs_enable_hsdet(struct snd_soc_component *codec,
	struct da_combine_mbhc_config mbhc_config)
{
	snd_soc_component_write(codec, DA_COMBINE_V3_HP_DET_CFG_REG, mbhc_config.hs_cfg[HS_CTRL]);
}

static void headphone_resdet_enable(struct snd_soc_component *codec,
	bool enable)
{
	if (enable) {
		/* imp_det enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_IMP_DET_CTRL_REG,
			0x1 << DA_COMBINE_V3_IMP_EN_BIT, 0x1 << DA_COMBINE_V3_IMP_EN_BIT);
		/* imp_det clk enable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
			0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT,
			0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT);

		/* dac pga : -40dB -> 0dB */
		snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG, 0x78);
		snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG, 0x78);
		msleep(32);

		/* sar clk -> 32k */
		da_combine_update_bits(codec, DA_COMBINE_V3_SAR_CONFIG_REG,
			0x7 << DA_COMBINE_V3_SAR_CLK_SEL_BIT,
			0x3 << DA_COMBINE_V3_SAR_CLK_SEL_BIT);
		/* sar input -> hpl */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_DET_CFG_REG,
			0x3 << DA_COMBINE_V3_SAR_INPUT_SEL_BIT,
			0x2 << DA_COMBINE_V3_SAR_INPUT_SEL_BIT);
		/* sar ref -> dvdd1.8 */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_DET_CFG_REG,
			0x7 << DA_COMBINE_V3_SAR_REF_SEL_BIT,
			0x4 << DA_COMBINE_V3_SAR_REF_SEL_BIT);
		/* sar rst -> work */
		da_combine_update_bits(codec, DA_COMBINE_V3_SAR_CONFIG_REG,
			0x1 << DA_COMBINE_V3_SAR_RST_BIT,
			0x1 << DA_COMBINE_V3_SAR_RST_BIT);
		/* sar power on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
			0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT,
			0x0 << DA_COMBINE_V3_MBHD_SAR_PD_BIT);
		/* det_pd on */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG3,
			0x1 << DA_COMBINE_V3_HP_RDET_PD_BIT,
			0x0 << DA_COMBINE_V3_HP_RDET_PD_BIT);
	} else {
		/* det_pd off */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG3,
			0x1 << DA_COMBINE_V3_HP_RDET_PD_BIT,
			0x1 << DA_COMBINE_V3_HP_RDET_PD_BIT);
		/* sar power off */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
			0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT,
			0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT);

		/* sar ref -> micbias2.7 */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_DET_CFG_REG,
			0x7 << DA_COMBINE_V3_SAR_REF_SEL_BIT,
			0x6 << DA_COMBINE_V3_SAR_REF_SEL_BIT);
		/* sar input -> mbhc */
		da_combine_update_bits(codec, DA_COMBINE_V3_HP_DET_CFG_REG,
			0x3 << DA_COMBINE_V3_SAR_INPUT_SEL_BIT,
			0x0 << DA_COMBINE_V3_SAR_INPUT_SEL_BIT);
		/* sar clk -> 32k */
		da_combine_update_bits(codec, DA_COMBINE_V3_SAR_CONFIG_REG,
			0x7 << DA_COMBINE_V3_SAR_CLK_SEL_BIT,
			0x3 << DA_COMBINE_V3_SAR_CLK_SEL_BIT);

		/* dac pga : 0dB -> -40dB */
		snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG, 0x28);
		snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG, 0x28);
		msleep(32);

		/* imp_det clk disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
			0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT,
			0x0 << DA_COMBINE_V3_HP_DET_CLKEN_BIT);
		/* imp_det disable */
		da_combine_update_bits(codec, DA_COMBINE_V3_IMP_DET_CTRL_REG,
			0x1 << DA_COMBINE_V3_IMP_EN_BIT, 0x0 << DA_COMBINE_V3_IMP_EN_BIT);
	}
}

static unsigned int get_voltage_value(struct snd_soc_component *codec,
	unsigned int voltage_coefficient)
{
	int retry = CHECK_SARADC_READY_TIMES;
	unsigned int sar_value = 0;
	unsigned int voltage_value = 0;
	uint64_t sar_to_valtage = 0;

	/* saradc on */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT, 0);
	/* start saradc */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		0x1 << DA_COMBINE_V3_SARADC_START_BIT, 0x1 << DA_COMBINE_V3_SARADC_START_BIT);

	while (retry--) {
		usleep_range(1000, 1100);
		if (da_combine_check_saradc_ready_detection(codec)) {
			sar_value = snd_soc_component_read32(codec, DA_COMBINE_V3_SAR_VALUE_REG);
			AUDIO_LOGI("saradc value for imp is %#x\n", sar_value);
			break;
		}
	}
	if (retry < 0)
		AUDIO_LOGE("get saradc err\n");

	/* end saradc */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		0x1 << DA_COMBINE_V3_SARADC_START_BIT, 0x0 << DA_COMBINE_V3_SARADC_START_BIT);
	/* saradc pd */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
		0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT, 0x1 << DA_COMBINE_V3_MBHD_SAR_PD_BIT);

	sar_to_valtage = (uint64_t)sar_value * voltage_coefficient / HS_MAX_SARADC_VAL;
	if (sar_to_valtage > INVALID_REG_VALUE)
		voltage_value = INVALID_REG_VALUE;
	else
		voltage_value = sar_to_valtage;

	return voltage_value;
}

static unsigned int get_volume_vlaue(struct snd_soc_component *codec)
{
	unsigned int volume_value = 0;

	volume_value = get_voltage_value(codec,
		HS_VOLTAGE_COEFFICIENT_VAL);
	if (volume_value < HS_VOLUME_VAL_LOW_THRESHOLD ||
		volume_value > HS_VOLUME_VAL_HIGH_THRESHOLD) {
		AUDIO_LOGI("voltage value is %d, invalid value, res selected is %#x\n",
			volume_value, snd_soc_component_read32(codec, DA_COMBINE_V3_ANALOG_REG50));
		volume_value = 0;
	}

	return volume_value;
}

static unsigned int calc_res(struct snd_soc_component *codec,
	unsigned int voltage_value, unsigned int num)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);
	unsigned int res_value = 0;
	unsigned int eq_res_value = 0;

	if (voltage_value == 0)
		return FAKE_RES_VALUE;

	eq_res_value = (1 << num) * priv->res_value * IMP_RANGE_VALUE /
		voltage_value / CURRENT_RATIO_VALUE;

	if (eq_res_value >= FB_RES_VALUE) {
		AUDIO_LOGE("calculate res value is %#x, invalid value\n",
			eq_res_value);
		res_value = FAKE_RES_VALUE;
	} else {
		res_value = FB_RES_VALUE * eq_res_value /
			(FB_RES_VALUE - eq_res_value);
	}

	return res_value;
}

static unsigned int headphone_resvalue_calc(struct snd_soc_component *codec)
{
	unsigned int res_value = 0;
	unsigned int volume_value[IMP_RES_NUM] = {0};
	unsigned int calc_res_value = 0;
	unsigned int i = 0;
	unsigned int data_num = 0;

	for (i = 0; i < IMP_RES_NUM; i++) {
		/* select different res */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG50,
			0x7 << DA_COMBINE_V3_DETRES_CHAIN_SEL_BIT,
			i << DA_COMBINE_V3_DETRES_CHAIN_SEL_BIT);
		/* reset imp */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG50,
			0x1 << DA_COMBINE_V3_DETRES_RST_BIT,
			0x1 << DA_COMBINE_V3_DETRES_RST_BIT);
		usleep_range(100, 120);
		/* unreset imp */
		da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG50,
			0x1 << DA_COMBINE_V3_DETRES_RST_BIT,
			0x0 << DA_COMBINE_V3_DETRES_RST_BIT);
		usleep_range(100, 120);

		volume_value[i] = get_volume_vlaue(codec);

		/* get sum of valid res value */
		if (volume_value[i] != 0) {
			calc_res_value =
				calc_res(codec, volume_value[i], i);
			AUDIO_LOGI("afer calculated res[%d] is %d\n",
				i, calc_res_value);
			if (calc_res_value != FAKE_RES_VALUE) {
				data_num++;
				res_value += calc_res_value;
			}
		}
	}

	if (data_num != 0) {
		res_value = res_value / data_num;
		AUDIO_LOGI("final res is %d\n", res_value);
	} else {
		res_value = HS_RES_DEGAULT_VALUE;
		AUDIO_LOGE("no valid res value! config as 32ohm\n");
	}

	return res_value;
}

static unsigned int get_resvalue(struct snd_soc_component *codec)
{
	unsigned int res_value = 0;

	headphone_resdet_enable(codec, true);

	msleep(40);

	res_value = headphone_resvalue_calc(codec);

	headphone_resdet_enable(codec, false);

	return res_value;
}

static void hs_res_det_cfg(struct snd_soc_component *codec)
{
	/* ib05_hp 5uA */
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG73, 0x66);
	/* headphone res select -> 1.25k */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG38,
		0x7 << DA_COMBINE_V3_HP_RES_CT_BIT, 0x2 << DA_COMBINE_V3_HP_RES_CT_BIT);
	/* headphone 0x20007130 config */
	snd_soc_component_write(codec, DA_COMBINE_V3_ANALOG_REG48, 0xCE);
	/* hpl_fb off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG43,
		0x7 << DA_COMBINE_V3_HPL_POP_PULL_5_3_BIT,
		0x0 << DA_COMBINE_V3_HPL_POP_PULL_5_3_BIT);
	/* hpr_fb off */
	da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG44,
		0x7 << DA_COMBINE_V3_HPR_POP_PULL_5_3_BIT,
		0x0 << DA_COMBINE_V3_HPR_POP_PULL_5_3_BIT);
}

static void imp_path_enable(struct snd_soc_component *codec)
{
	/* dacl src up16 clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
		1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT,
		1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT);
	/* dacl sdm clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT, 1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT);
	/* dsdl sdm clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT, 1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT);
	/* dacr src up16 clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
		1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT,
		1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT);
	/* dacr sdm clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT, 1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT);
	/* dsdr sdm clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT, 1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT);
	/* hp mux -> sdm */
	da_combine_update_bits(codec, DA_COMBINE_V3_DAC_HP_MUX_SEL, 0x3f, 0x0);
	/* dac select imp */
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_PGA_MUX_SEL,
		0x3 << DA_COMBINE_V3_DAC_PGA_MUX_BIT, 0x2 << DA_COMBINE_V3_DAC_PGA_MUX_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_PGA_MUX_SEL,
		0x3 << DA_COMBINE_V3_DAC_PGA_MUX_BIT, 0x2 << DA_COMBINE_V3_DAC_PGA_MUX_BIT);
	/* up16 mux -> pga */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_HP_EP_CTRL,
		0x1 << DA_COMBINE_V3_DACL_U16_MUX_BIT, 0x0 << DA_COMBINE_V3_DACL_U16_MUX_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_HP_EP_CTRL,
		0x1 << DA_COMBINE_V3_DACR_U16_MUX_BIT, 0x0 << DA_COMBINE_V3_DACR_U16_MUX_BIT);

	/* dac pga enable -> -40dB */
	snd_soc_component_write(codec, DA_COMBINE_V3_DACL_PGA_GAIN_CFG_REG, 0x28);
	snd_soc_component_write(codec, DA_COMBINE_V3_DACR_PGA_GAIN_CFG_REG, 0x28);

	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
		0x3 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT,
		0x3 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT);
	/* imp_det clk enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
		0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT, 0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT);
	/* imp_det enable */
	da_combine_update_bits(codec, DA_COMBINE_V3_IMP_DET_CTRL_REG,
		0x1 << DA_COMBINE_V3_IMP_EN_BIT, 0x1 << DA_COMBINE_V3_IMP_EN_BIT);
	usleep_range(1000, 1100);
	/* imp_det disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_IMP_DET_CTRL_REG,
		0x1 << DA_COMBINE_V3_IMP_EN_BIT, 0x0 << DA_COMBINE_V3_IMP_EN_BIT);
	usleep_range(1000, 1100);
	/* imp_det clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG05,
		0x1 << DA_COMBINE_V3_HP_DET_CLKEN_BIT, 0x0 << DA_COMBINE_V3_HP_DET_CLKEN_BIT);

	/* headphone pop on */
	headphone_pop_on(codec);
}

static void imp_path_disable(struct snd_soc_component *codec)
{
	/* headphone pop off */
	headphone_pop_off(codec);

	/* up16 mux -> classH */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_HP_EP_CTRL,
		0x1 << DA_COMBINE_V3_DACL_U16_MUX_BIT, 0x1 << DA_COMBINE_V3_DACL_U16_MUX_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASSH_HP_EP_CTRL,
		0x1 << DA_COMBINE_V3_DACR_U16_MUX_BIT, 0x1 << DA_COMBINE_V3_DACR_U16_MUX_BIT);
	/* dac select imp */
	da_combine_update_bits(codec, DA_COMBINE_V3_DACL_PGA_MUX_SEL,
		0x3 << DA_COMBINE_V3_DAC_PGA_MUX_BIT, 0x0 << DA_COMBINE_V3_DAC_PGA_MUX_BIT);
	da_combine_update_bits(codec, DA_COMBINE_V3_DACR_PGA_MUX_SEL,
		0x3 << DA_COMBINE_V3_DAC_PGA_MUX_BIT, 0x0 << DA_COMBINE_V3_DAC_PGA_MUX_BIT);
	/* dac pga clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG10,
		0x3 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT,
		0x0 << DA_COMBINE_V3_DACL_PGA_CLKEN_BIT);
	/* hp mux -> off */
	da_combine_update_bits(codec, DA_COMBINE_V3_DAC_HP_MUX_SEL, 0x3f, 0x1B);
	/* dacr sdm clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DACR_SDM_CLKEN_BIT, 0);
	/* dsdr sdm clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DSDR_SDM_CLKEN_BIT, 0);
	/* dacr src up16 clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
		1 << DA_COMBINE_V3_DACR_UP16_CLKEN_BIT, 0);
	/* dacl sdm clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DACL_SDM_CLKEN_BIT, 0);
	/* dsdl sdm clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG08,
		1 << DA_COMBINE_V3_DSDL_SDM_CLKEN_BIT, 0);
	/* dacl src up16 clk disable */
	da_combine_update_bits(codec, DA_COMBINE_V3_DIG_CLK_CFG_REG06,
		1 << DA_COMBINE_V3_DACL_UP16_CLKEN_BIT, 0);
}

static void hs_path_enable(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	/* disable the trigger for shadow gain */
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG,
		0x1 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT,
		0x0 << DA_COMBINE_V3_HP_SW_TRIGGER_BIT);
	/* disable the config for shadow gain */
	da_combine_update_bits(codec, DA_COMBINE_V3_HP_SW_CTRL10_REG, 0x1, 0x0);

	/* pll request */
	da_combine_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	da_combine_v3_request_cp_dp_clk(codec);
	/* headphone config for imp_det */
	hs_res_det_cfg(codec);
	/* path enable */
	imp_path_enable(codec);
}

static void hs_res_detect(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	unsigned int res_value = get_resvalue(codec);

	if (res_value < RES_THRESHOLD) {
		priv->hs_high_pga_gain = HS_HIGH_0_GAIN;
		priv->hs_low_pga_gain = HS_LOW_0_GAIN;
	} else {
		priv->hs_high_pga_gain = HS_HIGH_1_GAIN;
		priv->hs_low_pga_gain = HS_LOW_1_GAIN;
	}
}

static void hs_path_disable(struct snd_soc_component *codec)
{
	struct da_combine_v3_platform_data *priv = snd_soc_component_get_drvdata(codec);

	/* path enable */
	imp_path_disable(codec);
	da_combine_v3_release_cp_dp_clk(codec);
	/* pll request */
	da_combine_resmgr_release_pll(priv->resmgr, PLL_HIGH);

	/* classA/B -> classH  */
	da_combine_update_bits(codec, DA_COMBINE_V3_CLASS_H_CFG_REG1,
		0x1 << DA_COMBINE_V3_CLASS_H_SEL_BIT, 0x1 << DA_COMBINE_V3_CLASS_H_SEL_BIT);

	hp_classh_init(codec, priv);
}

static struct hs_mbhc_reg hs_mbhc_reg = {
	.irq_source_reg = CODEC_BASE_ADDR + DA_COMBINE_V3_PLL44K1_PLLMAD_STATE_REG,
	.irq_mbhc_2_reg = CODEC_BASE_ADDR + DA_COMBINE_V3_IRQ_REG2_REG,
};

static struct hs_mbhc_func hs_mbhc_func = {
	.hs_mbhc_on =  hs_mbhc_on,
	.hs_get_voltage = get_voltage_value,
	.hs_enable_hsdet = hs_enable_hsdet,
	.hs_mbhc_off =  hs_mbhc_off,
};

static struct hs_res_detect_func hs_res_detect_func = {
	.hs_res_detect = hs_res_detect,
	.hs_path_enable = hs_path_enable,
	.hs_path_disable = hs_path_disable,
};

static struct hs_res_detect_func hs_res_detect_func_null = {
	.hs_res_detect = NULL,
	.hs_path_enable = NULL,
	.hs_path_disable = NULL,
};

static void generate_mbhc_cfg(struct da_combine_v3_platform_data *data,
	struct da_combine_mbhc_cfg *mbhc_cfg)
{
	mbhc_cfg->codec = data->codec;
	mbhc_cfg->irqmgr = data->irqmgr;
	mbhc_cfg->node = data->node;
	mbhc_cfg->resmgr = data->resmgr;
	mbhc_cfg->mbhc = &(data->mbhc);
}

int hs_cfg_init(struct snd_soc_component *codec,
	struct da_combine_v3_platform_data *platform_data)
{
	int ret;
	struct da_combine_mbhc_cfg mbhc;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	if (platform_data == NULL) {
		AUDIO_LOGE("platform data parameter is null");
		return -EINVAL;
	}

	hs_cfg.mbhc_reg = &hs_mbhc_reg;
	hs_cfg.mbhc_func = &hs_mbhc_func;

	if (platform_data->board_config.hp_res_detect_enable)
		hs_cfg.res_detect_func = &hs_res_detect_func;
	else
		hs_cfg.res_detect_func = &hs_res_detect_func_null;

	generate_mbhc_cfg(platform_data, &mbhc);

	ret = da_combine_mbhc_init(&mbhc, &hs_cfg);
	if (ret != 0) {
		memset(&hs_cfg, 0, sizeof(hs_cfg)); /* unsafe_function_ignore: memset */
		AUDIO_LOGE("mbhc init fail. err is %x\n", ret);
	}

	return ret;
}

void hs_cfg_deinit(struct da_combine_mbhc *mbhc)
{
	if (mbhc == NULL) {
		AUDIO_LOGE("mbhc parameter is null");
		return;
	}

	da_combine_mbhc_deinit(mbhc);
	memset(&hs_cfg, 0, sizeof(hs_cfg)); /* unsafe_function_ignore: memset */
}


