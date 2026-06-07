/*
 * hi6403_headset_config.c
 *
 * headset init config
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include "hi6403_headset_config.h"

#include <sound/core.h>
#include <sound/tlv.h>

#include <linux/hisi/audio_log.h>

#include "linux/hisi/hi64xx/hi6403_regs.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"
#include "hi6403_path_widget.h"
#include "hi6403_resource_widget.h"
#include "hi6403_switch_widget.h"

#define HS_VOLTAGE_COEFFICIENT_VAL   1800
#define HS_VOLUME_VAL_LOW_THRESHOLD  100
#define HS_VOLUME_VAL_HIGH_THRESHOLD 300
#define HS_MAX_SARADC_VAL            0xFF

static struct hi64xx_hs_cfg hi6403_hs_cfg = {0};

void hi6403_hp_classh_init(struct snd_soc_codec *codec,
	struct hi6403_platform_data *platform_data)
{
	if (platform_data->board_config.hp_high_low_change_enable == true) {
		hi6403_headphone_low_mode_shadow_config(codec);
		/* headphone mode select low mode */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_SRB_CTRL_BIT,
			0 << HI6403_HP_SRB_CTRL_BIT);
		/* headphone feed back disable */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_FB_EN_BIT, 0 << HI6403_HP_FB_EN_BIT);
		/* headphone feed back mode config */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_FB_STB_CTRL_BIT,
			0 << HI6403_HP_FB_STB_CTRL_BIT);
		/* headphone ib05 current init */
		snd_soc_write(codec, HI6403_ANALOG_REG73, 0x22);
		/* enable the config for shadow gain */
		hi64xx_update_bits(codec, HI6403_HP_SW_CTRL10_REG, 0x1, 0x1);
		/* enable the trigger for shadow gain */
		hi64xx_update_bits(codec, HI6403_HP_SW_CTRL10_REG,
			0x1 << HI6403_HP_SW_TRIGGER_BIT,
			0x1 << HI6403_HP_SW_TRIGGER_BIT);
	} else {
		hi6403_headphone_high_mode_shadow_config(codec);
		/* headphone ib05 current init */
		snd_soc_write(codec, HI6403_ANALOG_REG73, 0x66);
		/* headphone feed back mode config */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_FB_STB_CTRL_BIT,
			0x1 << HI6403_HP_FB_STB_CTRL_BIT);
		/* headphone feed back enable */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_FB_EN_BIT, 0x1 << HI6403_HP_FB_EN_BIT);
		/* headphone mode select high mode */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG48,
			0x1 << HI6403_HP_SRB_CTRL_BIT,
			0x1 << HI6403_HP_SRB_CTRL_BIT);
	}
}

static void hi6403_hs_mbhc_on(struct snd_soc_codec *codec)
{
	struct hi6403_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	/* mask btn irqs */
	hi64xx_irq_mask_btn_irqs(priv->mbhc);

	/* saradc cfg */
	snd_soc_write(codec,  HI6403_SAR_CONFIG_REG, 0x7C);
	/* mbhc on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		1 << HI64xx_MBHC_ON_BIT, 0);

	msleep(30);
	/* unmask btn irqs */
	hi64xx_irq_unmask_btn_irqs(priv->mbhc);

	msleep(120);
}

static void hi6403_hs_mbhc_off(struct snd_soc_codec *codec)
{
	/* eco off */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG093,
		1 << HI64xx_MICBIAS_ECO_ON_BIT, 0);
	AUDIO_LOGI("eco disable\n");

	/* mbhc cmp off */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		1 << HI64xx_MBHC_ON_BIT, 1 << HI64xx_MBHC_ON_BIT);
}

static void hi6403_hs_enable_hsdet(struct snd_soc_codec *codec,
	struct hi64xx_mbhc_config mbhc_config)
{
	snd_soc_write(codec, HI6403_HP_DET_CFG_REG, mbhc_config.hs_ctrl);
}

static void hi6403_headphone_resdet_enable(struct snd_soc_codec *codec,
	bool enable)
{
	if (enable) {
		/* imp_det enable */
		hi64xx_update_bits(codec, HI6403_IMP_DET_CTRL_REG,
			0x1 << HI6403_IMP_EN_BIT, 0x1 << HI6403_IMP_EN_BIT);
		/* imp_det clk enable */
		hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG05,
			0x1 << HI6403_HP_DET_CLKEN_BIT,
			0x1 << HI6403_HP_DET_CLKEN_BIT);

		/* dac pga : -40dB -> 0dB */
		snd_soc_write(codec, HI6403_DACL_PGA_GAIN_CFG_REG, 0x78);
		snd_soc_write(codec, HI6403_DACR_PGA_GAIN_CFG_REG, 0x78);
		msleep(32);

		/* sar clk -> 32k */
		hi64xx_update_bits(codec, HI6403_SAR_CONFIG_REG,
			0x7 << HI6403_SAR_CLK_SEL_BIT,
			0x3 << HI6403_SAR_CLK_SEL_BIT);
		/* sar input -> hpl */
		hi64xx_update_bits(codec, HI6403_HP_DET_CFG_REG,
			0x3 << HI6403_SAR_INPUT_SEL_BIT,
			0x2 << HI6403_SAR_INPUT_SEL_BIT);
		/* sar ref -> dvdd1.8 */
		hi64xx_update_bits(codec, HI6403_HP_DET_CFG_REG,
			0x7 << HI6403_SAR_REF_SEL_BIT,
			0x4 << HI6403_SAR_REF_SEL_BIT);
		/* sar rst -> work */
		hi64xx_update_bits(codec, HI6403_SAR_CONFIG_REG,
			0x1 << HI6403_SAR_RST_BIT,
			0x1 << HI6403_SAR_RST_BIT);
		/* sar power on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
			0x1 << HI6403_MBHD_SAR_PD_BIT,
			0x0 << HI6403_MBHD_SAR_PD_BIT);
		/* det_pd on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
			0x1 << HI6403_HP_RDET_PD_BIT,
			0x0 << HI6403_HP_RDET_PD_BIT);
	} else {
		/* det_pd off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
			0x1 << HI6403_HP_RDET_PD_BIT,
			0x1 << HI6403_HP_RDET_PD_BIT);
		/* sar power off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
			0x1 << HI6403_MBHD_SAR_PD_BIT,
			0x1 << HI6403_MBHD_SAR_PD_BIT);

		/* sar ref -> micbias2.7 */
		hi64xx_update_bits(codec, HI6403_HP_DET_CFG_REG,
			0x7 << HI6403_SAR_REF_SEL_BIT,
			0x6 << HI6403_SAR_REF_SEL_BIT);
		/* sar input -> mbhc */
		hi64xx_update_bits(codec, HI6403_HP_DET_CFG_REG,
			0x3 << HI6403_SAR_INPUT_SEL_BIT,
			0x0 << HI6403_SAR_INPUT_SEL_BIT);
		/* sar clk -> 32k */
		hi64xx_update_bits(codec, HI6403_SAR_CONFIG_REG,
			0x7 << HI6403_SAR_CLK_SEL_BIT,
			0x3 << HI6403_SAR_CLK_SEL_BIT);

		/* dac pga : 0dB -> -40dB */
		snd_soc_write(codec, HI6403_DACL_PGA_GAIN_CFG_REG, 0x28);
		snd_soc_write(codec, HI6403_DACR_PGA_GAIN_CFG_REG, 0x28);
		msleep(32);

		/* imp_det clk disable */
		hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG05,
			0x1 << HI6403_HP_DET_CLKEN_BIT,
			0x0 << HI6403_HP_DET_CLKEN_BIT);
		/* imp_det disable */
		hi64xx_update_bits(codec, HI6403_IMP_DET_CTRL_REG,
			0x1 << HI6403_IMP_EN_BIT, 0x0 << HI6403_IMP_EN_BIT);
	}
}

static unsigned int hi6403_get_voltage_value(struct snd_soc_codec *codec,
	unsigned int voltage_coefficient)
{
	int retry = 3;
	unsigned int sar_value = 0;
	unsigned int voltage_value = 0;

	/* saradc on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		0x1 << HI6403_MBHD_SAR_PD_BIT, 0);
	/* start saradc */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		0x1 << HI6403_SARADC_START_BIT, 0x1 << HI6403_SARADC_START_BIT);

	while (retry--) {
		usleep_range(1000, 1100);
		if (hi64xx_check_saradc_ready_detect(codec)) {
			sar_value = snd_soc_read(codec, HI6403_SAR_VALUE_REG);
			AUDIO_LOGI("saradc value for imp is %#x\n", sar_value);
			break;
		}
	}
	if (retry < 0)
		AUDIO_LOGE("get saradc err\n");

	/* end saradc */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		0x1 << HI6403_SARADC_START_BIT, 0x0 << HI6403_SARADC_START_BIT);
	/* saradc pd */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG089,
		0x1 << HI6403_MBHD_SAR_PD_BIT, 0x1 << HI6403_MBHD_SAR_PD_BIT);

	voltage_value = sar_value * voltage_coefficient / HS_MAX_SARADC_VAL;

	return voltage_value;
}

static unsigned int hi6403_get_volume_vlaue(struct snd_soc_codec *codec)
{
	unsigned int volume_value = 0;

	volume_value = hi6403_get_voltage_value(codec,
		HS_VOLTAGE_COEFFICIENT_VAL);
	if (volume_value < HS_VOLUME_VAL_LOW_THRESHOLD ||
		volume_value > HS_VOLUME_VAL_HIGH_THRESHOLD) {
		AUDIO_LOGI("voltage value is %d, invalid value, res selected is %#x\n",
			volume_value, snd_soc_read(codec, HI6403_ANALOG_REG50));
		volume_value = 0;
	}

	return volume_value;
}

static unsigned int hi6403_calc_res(struct snd_soc_codec *codec,
	unsigned int voltage_value, unsigned int num)
{
	struct hi6403_platform_data *priv = snd_soc_codec_get_drvdata(codec);
	unsigned int res_value = 0;
	unsigned int eq_res_value = 0;

	if (voltage_value == 0)
		return HI6403_FAKE_RES_VALUE;

	eq_res_value = (1 << num) * priv->res_value * HI6403_IMP_RANGE_VALUE /
		voltage_value / HI6403_CURRENT_RATIO_VALUE;

	if (eq_res_value >= HI6403_FB_RES_VALUE) {
		AUDIO_LOGE("calculate res value is %#x, invalid value\n",
			eq_res_value);
		res_value = HI6403_FAKE_RES_VALUE;
	} else {
		res_value = HI6403_FB_RES_VALUE * eq_res_value /
			(HI6403_FB_RES_VALUE - eq_res_value);
	}

	return res_value;
}

static unsigned int hi6403_headphone_resvalue_calc(struct snd_soc_codec *codec)
{
	unsigned int res_value = 0;
	unsigned int volume_value[HI6403_IMP_RES_NUM] = {0};
	unsigned int calc_res_value = 0;
	unsigned int i = 0;
	unsigned int data_num = 0;

	for (i = 0; i < HI6403_IMP_RES_NUM; i++) {
		/* select different res */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG50,
				0x7 << HI6403_DETRES_CHAIN_SEL_BIT,
				i << HI6403_DETRES_CHAIN_SEL_BIT);
		/* reset imp */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG50,
				0x1 << HI6403_DETRES_RST_BIT,
				0x1 << HI6403_DETRES_RST_BIT);
		usleep_range(100, 120);
		/* unreset imp */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG50,
				0x1 << HI6403_DETRES_RST_BIT,
				0x0 << HI6403_DETRES_RST_BIT);
		usleep_range(100, 120);

		volume_value[i] = hi6403_get_volume_vlaue(codec);

		/* get sum of valid res value */
		if (volume_value[i] != 0) {
			calc_res_value =
				hi6403_calc_res(codec, volume_value[i], i);
			AUDIO_LOGI("afer calculated res[%d] is %d\n",
				i, calc_res_value);
			if (calc_res_value != HI6403_FAKE_RES_VALUE) {
				data_num++;
				res_value += calc_res_value;
			}
		}
	}

	if (data_num != 0) {
		res_value = res_value / data_num;
		AUDIO_LOGI("final res is %d\n", res_value);
	} else {
		res_value = HI6403_HS_RES_DEGAULT_VALUE;
		AUDIO_LOGE("no valid res value! config as 32ohm\n");
	}

	return res_value;
}

static unsigned int hi6403_get_resvalue(struct snd_soc_codec *codec)
{
	unsigned int res_value = 0;

	hi6403_headphone_resdet_enable(codec, true);

	msleep(40);

	res_value = hi6403_headphone_resvalue_calc(codec);

	hi6403_headphone_resdet_enable(codec, false);

	return res_value;
}

static void hs_res_det_cfg(struct snd_soc_codec *codec)
{
	/* ib05_hp 5uA */
	snd_soc_write(codec, HI6403_ANALOG_REG73, 0x66);
	/* headphone res select -> 1.25k */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG38,
		0x7 << HI6403_HP_RES_CT_BIT, 0x2 << HI6403_HP_RES_CT_BIT);
	/* headphone 0x20007130 config */
	snd_soc_write(codec, HI6403_ANALOG_REG48, 0xCE);
	/* hpl_fb off */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG43,
		0x7 << HI6403_HPL_POP_PULL_5_3_BIT,
		0x0 << HI6403_HPL_POP_PULL_5_3_BIT);
	/* hpr_fb off */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG44,
		0x7 << HI6403_HPR_POP_PULL_5_3_BIT,
		0x0 << HI6403_HPR_POP_PULL_5_3_BIT);
}

static void imp_path_enable(struct snd_soc_codec *codec)
{
	/* dacl src up16 clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG06,
		1 << HI6403_DACL_UP16_CLKEN_BIT,
		1 << HI6403_DACL_UP16_CLKEN_BIT);
	/* dacl sdm clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DACL_SDM_CLKEN_BIT, 1 << HI6403_DACL_SDM_CLKEN_BIT);
	/* dsdl sdm clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DSDL_SDM_CLKEN_BIT, 1 << HI6403_DSDL_SDM_CLKEN_BIT);
	/* dacr src up16 clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG06,
		1 << HI6403_DACR_UP16_CLKEN_BIT,
		1 << HI6403_DACR_UP16_CLKEN_BIT);
	/* dacr sdm clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DACR_SDM_CLKEN_BIT, 1 << HI6403_DACR_SDM_CLKEN_BIT);
	/* dsdr sdm clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DSDR_SDM_CLKEN_BIT, 1 << HI6403_DSDR_SDM_CLKEN_BIT);
	/* hp mux -> sdm */
	hi64xx_update_bits(codec, HI6403_DAC_HP_MUX_SEL, 0x3f, 0x0);
	/* dac select imp */
	hi64xx_update_bits(codec, HI6403_DACL_PGA_MUX_SEL,
		0x3 << HI6403_DAC_PGA_MUX_BIT, 0x2 << HI6403_DAC_PGA_MUX_BIT);
	hi64xx_update_bits(codec, HI6403_DACR_PGA_MUX_SEL,
		0x3 << HI6403_DAC_PGA_MUX_BIT, 0x2 << HI6403_DAC_PGA_MUX_BIT);
	/* up16 mux -> pga */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACL_U16_MUX_BIT, 0x0 << HI6403_DACL_U16_MUX_BIT);
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACR_U16_MUX_BIT, 0x0 << HI6403_DACR_U16_MUX_BIT);

	/* dac pga enable -> -40dB */
	snd_soc_write(codec, HI6403_DACL_PGA_GAIN_CFG_REG, 0x28);
	snd_soc_write(codec, HI6403_DACR_PGA_GAIN_CFG_REG, 0x28);

	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG10,
		0x3 << HI6403_DACL_PGA_CLKEN_BIT,
		0x3 << HI6403_DACL_PGA_CLKEN_BIT);
	/* imp_det clk enable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG05,
		0x1 << HI6403_HP_DET_CLKEN_BIT, 0x1 << HI6403_HP_DET_CLKEN_BIT);
	/* imp_det enable */
	hi64xx_update_bits(codec, HI6403_IMP_DET_CTRL_REG,
		0x1 << HI6403_IMP_EN_BIT, 0x1 << HI6403_IMP_EN_BIT);
	usleep_range(1000, 1100);
	/* imp_det disable */
	hi64xx_update_bits(codec, HI6403_IMP_DET_CTRL_REG,
		0x1 << HI6403_IMP_EN_BIT, 0x0 << HI6403_IMP_EN_BIT);
	usleep_range(1000, 1100);
	/* imp_det clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG05,
		0x1 << HI6403_HP_DET_CLKEN_BIT, 0x0 << HI6403_HP_DET_CLKEN_BIT);

	/* headphone pop on */
	hi6403_headphone_pop_on(codec);
}

static void imp_path_disable(struct snd_soc_codec *codec)
{
	/* headphone pop off */
	hi6403_headphone_pop_off(codec);

	/* up16 mux -> classH */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACL_U16_MUX_BIT, 0x1 << HI6403_DACL_U16_MUX_BIT);
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACR_U16_MUX_BIT, 0x1 << HI6403_DACR_U16_MUX_BIT);
	/* dac select imp */
	hi64xx_update_bits(codec, HI6403_DACL_PGA_MUX_SEL,
		0x3 << HI6403_DAC_PGA_MUX_BIT, 0x0 << HI6403_DAC_PGA_MUX_BIT);
	hi64xx_update_bits(codec, HI6403_DACR_PGA_MUX_SEL,
		0x3 << HI6403_DAC_PGA_MUX_BIT, 0x0 << HI6403_DAC_PGA_MUX_BIT);
	/* dac pga clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG10,
		0x3 << HI6403_DACL_PGA_CLKEN_BIT,
		0x0 << HI6403_DACL_PGA_CLKEN_BIT);
	/* hp mux -> off */
	hi64xx_update_bits(codec, HI6403_DAC_HP_MUX_SEL, 0x3f, 0x1B);
	/* dacr sdm clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DACR_SDM_CLKEN_BIT, 0);
	/* dsdr sdm clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DSDR_SDM_CLKEN_BIT, 0);
	/* dacr src up16 clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG06,
		1 << HI6403_DACR_UP16_CLKEN_BIT, 0);
	/* dacl sdm clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DACL_SDM_CLKEN_BIT, 0);
	/* dsdl sdm clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG08,
		1 << HI6403_DSDL_SDM_CLKEN_BIT, 0);
	/* dacl src up16 clk disable */
	hi64xx_update_bits(codec, HI6403_DIG_CLK_CFG_REG06,
		1 << HI6403_DACL_UP16_CLKEN_BIT, 0);
}

static void hi6403_hs_path_enable(struct snd_soc_codec *codec)
{
	struct hi6403_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	/* disable the trigger for shadow gain */
	hi64xx_update_bits(codec, HI6403_HP_SW_CTRL10_REG,
			0x1 << HI6403_HP_SW_TRIGGER_BIT,
			0x0 << HI6403_HP_SW_TRIGGER_BIT);
	/* disable the config for shadow gain */
	hi64xx_update_bits(codec, HI6403_HP_SW_CTRL10_REG, 0x1, 0x0);

	/* pll request */
	hi64xx_resmgr_request_pll(priv->resmgr, PLL_HIGH);
	hi6403_request_cp_dp_clk(codec);
	/* headphone config for imp_det */
	hs_res_det_cfg(codec);
	/* path enable */
	imp_path_enable(codec);
}

static void hi6403_hs_res_detect(struct snd_soc_codec *codec)
{
	struct hi6403_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	unsigned int res_value = hi6403_get_resvalue(codec);

	if (res_value < HI6403_RES_THRESHOLD) {
		priv->hs_high_pga_gain = HI6403_HS_HIGH_0_GAIN;
		priv->hs_low_pga_gain = HI6403_HS_LOW_0_GAIN;
	} else {
		priv->hs_high_pga_gain = HI6403_HS_HIGH_1_GAIN;
		priv->hs_low_pga_gain = HI6403_HS_LOW_1_GAIN;
	}
}

static void hi6403_hs_path_disable(struct snd_soc_codec *codec)
{
	struct hi6403_platform_data *priv = snd_soc_codec_get_drvdata(codec);

	/* path enable */
	imp_path_disable(codec);
	hi6403_release_cp_dp_clk(codec);
	/* pll request */
	hi64xx_resmgr_release_pll(priv->resmgr, PLL_HIGH);

	/* classA/B -> classH  */
	hi64xx_update_bits(codec, HI6403_CLASS_H_CFG_REG1,
		0x1 << HI6403_CLASS_H_SEL_BIT, 0x1 << HI6403_CLASS_H_SEL_BIT);

	hi6403_hp_classh_init(codec, priv);
}

static struct hs_mbhc_reg hi6403_hs_mbhc_reg = {
	.irq_source_reg = CODEC_BASE_ADDR + HI6403_PLL44K1_PLLMAD_STATE_REG,
	.irq_mbhc_2_reg = CODEC_BASE_ADDR + HI6403_IRQ_REG2_REG,
};

static struct hs_mbhc_func hi6403_hs_mbhc_func = {
	.hs_mbhc_on =  hi6403_hs_mbhc_on,
	.hs_get_voltage = hi6403_get_voltage_value,
	.hs_enable_hsdet = hi6403_hs_enable_hsdet,
	.hs_mbhc_off =  hi6403_hs_mbhc_off,
};

static struct hs_res_detect_func hi6403_hs_res_detect_func = {
	.hs_res_detect = hi6403_hs_res_detect,
	.hs_path_enable = hi6403_hs_path_enable,
	.hs_path_disable = hi6403_hs_path_disable,
};

static struct hs_res_detect_func hi6403_hs_res_detect_func_null = {
	.hs_res_detect = NULL,
	.hs_path_enable = NULL,
	.hs_path_disable = NULL,
};

int hi6403_hs_cfg_init(struct snd_soc_codec *codec,
	struct hi6403_platform_data *platform_data)
{
	int ret;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	if (platform_data == NULL) {
		AUDIO_LOGE("platform data parameter is null");
		return -EINVAL;
	}

	hi6403_hs_cfg.mbhc_reg = &hi6403_hs_mbhc_reg;
	hi6403_hs_cfg.mbhc_func = &hi6403_hs_mbhc_func;

	if (platform_data->board_config.hp_res_detect_enable)
		hi6403_hs_cfg.res_detect_func = &hi6403_hs_res_detect_func;
	else
		hi6403_hs_cfg.res_detect_func = &hi6403_hs_res_detect_func_null;

	ret = hi64xx_mbhc_init(codec, platform_data->node, &hi6403_hs_cfg,
		platform_data->resmgr, platform_data->irqmgr,
		&platform_data->mbhc);
	if (ret != 0) {
		memset(&hi6403_hs_cfg, 0, sizeof(hi6403_hs_cfg)); /* unsafe_function_ignore: memset */
		AUDIO_LOGE("hi6403 mbhc init fail. err codec is %x\n", ret);
	}

	return ret;
}

void hi6403_hs_cfg_deinit(struct hi64xx_mbhc *mbhc)
{
	if (mbhc == NULL) {
		AUDIO_LOGE("mbhc parameter is null");
		return;
	}

	hi64xx_mbhc_deinit(mbhc);
	memset(&hi6403_hs_cfg, 0, sizeof(hi6403_hs_cfg)); /* unsafe_function_ignore: memset */
}


