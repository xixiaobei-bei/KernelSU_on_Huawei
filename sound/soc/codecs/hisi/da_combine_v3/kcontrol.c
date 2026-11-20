/*
 * kcontrol.c
 *
 * kcontrol
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

#include "kcontrol.h"

#include <linux/gpio.h>

#include <sound/core.h>
#include <sound/tlv.h>

#include <linux/hisi/audio_log.h>

#include "linux/hisi/da_combine/da_combine_v3_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "type.h"

/*
 * MAIN MIC GAIN volume control:
 * from 0 to 36 dB in 2 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(main_mic_tlv, 0, 200, 0);

/*
 * AUX MIC GAIN volume control:
 * from 0 to 36 dB in 2 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(aux_mic_tlv, 0, 200, 0);

/*
 * LINEINR MIC GAIN volume control:
 * from -20 to 36 dB in 2 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(lineinr_mic_tlv, -2000, 200, 0);/*lint !e570*/

/*
 * LINEINL MIC GAIN volume control:
 * from -20 to 36 dB in 2 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(lineinl_mic_tlv, -2000, 200, 0);/*lint !e570*/

/*
 * LOL PGA GAIN volume control:
 * from -21 to 6 dB in 1.5 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(lol_pga_tlv, -2100, 150, 0);/*lint !e570*/

/*
 * LOR PGA GAIN volume control:
 * from -21 to 6 dB in 1.5 dB steps
 * MAX VALUE is 18
 */
static DECLARE_TLV_DB_SCALE(lor_pga_tlv, -2100, 150, 0);/*lint !e570*/

/*
 * EP PGA GAIN volume control:
 * from -21 to 6 dB in 1.5 dB steps
 * MAX VALUE is 18
 */

static DECLARE_TLV_DB_SCALE(mad_tlv, 0, 200, 0);

static const char * const bt_tri_text[] = {"BT", "PA", "Invalid"};
static const struct soc_enum bt_tri_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(bt_tri_text), bt_tri_text),
};

#ifdef AUDIO_FACTORY_MODE
static const char * const micbias_text[] = { "OFF", "ON" };

static const struct soc_enum micbias_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(micbias_text), micbias_text),
};

static int main_micbias_status_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -EINVAL;
	}

	AUDIO_LOGI("main micbias val:%d", priv->mainmicbias_val);
	ucontrol->value.integer.value[0] = priv->mainmicbias_val;

	return 0;
}

static int main_micbias_status_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -EINVAL;
	}

	priv->mainmicbias_val = ucontrol->value.integer.value[0];
	if (priv->mainmicbias_val == 1) { /* 1 mean ON define in micbias_text */
		AUDIO_LOGI("mainmic bias on");
		da_combine_resmgr_request_micbias(priv->resmgr);
		if (priv->board_config.micbias_modify)
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
				1 << DA_COMBINE_V3_MIC4_PD_BIT, 0);
		else
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
				1 << DA_COMBINE_V3_MAINMIC_PD_BIT, 0);
	} else { /* 0 mean OFF define in micbias_text */
		AUDIO_LOGI("mainmic bias off");
		if (priv->board_config.micbias_modify)
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
				1 << DA_COMBINE_V3_MIC4_PD_BIT,
				1 << DA_COMBINE_V3_MIC4_PD_BIT);
		else
			da_combine_update_bits(codec, DA_COMBINE_V3_ANALOG_REG089,
				1 << DA_COMBINE_V3_MAINMIC_PD_BIT,
				1 << DA_COMBINE_V3_MAINMIC_PD_BIT);
		da_combine_resmgr_release_micbias(priv->resmgr);
	}

	return 0;
}
#endif

static int bt_tri_status_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -EINVAL;
	}

	if (!gpio_is_valid(priv->board_config.bt_tri_gpio)) {
		AUDIO_LOGI("bt tri gpio = %d is invalid\n",
			priv->board_config.bt_tri_gpio);
		ucontrol->value.integer.value[0] = BT_TRI_INVALID;
		return 0;
	}

	ucontrol->value.integer.value[0] =
		gpio_get_value(priv->board_config.bt_tri_gpio);

	AUDIO_LOGI("bt tri gpio = %d, value = %ld\n",
		priv->board_config.bt_tri_gpio,
		ucontrol->value.integer.value[0]);

	return 0;
}

static int bt_tri_status_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;
	int val = 0;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -EINVAL;
	}

	if (!gpio_is_valid(priv->board_config.bt_tri_gpio)) {
		AUDIO_LOGE("bt tri gpio = %d is invalid\n",
			priv->board_config.bt_tri_gpio);
		return -EINVAL;
	}

	val = ucontrol->value.integer.value[0];
	if (val != BT_TRI_BT && val != BT_TRI_PA) {
		AUDIO_LOGE("set status value invalid, val = %d\n", val);
		return val;
	}

	gpio_set_value(priv->board_config.bt_tri_gpio, val);
	AUDIO_LOGI("bt tri gpio %u,, val = %d\n",
		priv->board_config.bt_tri_gpio, val);

	return val;
}

#ifdef CONFIG_HAC_SUPPORT
/* hac status */
#define HAC_ENABLE                   1
#define HAC_DISABLE                  0

static const char * const hac_switch_text[] = { "OFF", "ON" };

static const struct soc_enum hac_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hac_switch_text),
		hac_switch_text),
};

static int hac_status_get(struct snd_kcontrol *kcontrol,
		struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}
	codec = snd_soc_kcontrol_component(kcontrol);
	priv = snd_soc_component_get_drvdata(codec);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		AUDIO_LOGE("hac gpio = %d is invalid",
			priv->board_config.hac_gpio);
		return -EINVAL;
	}
	ret = gpio_get_value(priv->board_config.hac_gpio);
	AUDIO_LOGI("hac gpio = %d, value = %d",
		priv->board_config.hac_gpio, ret);
	ucontrol->value.integer.value[0] = ret;

	return 0;
}

static int hac_status_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}
	codec = snd_soc_kcontrol_component(kcontrol);
	priv = snd_soc_component_get_drvdata(codec);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		AUDIO_LOGE("hac gpio = %d is invalid",
			priv->board_config.hac_gpio);
		return -EINVAL;
	}
	ret = ucontrol->value.integer.value[0];
	if (ret == HAC_ENABLE)
		gpio_set_value(priv->board_config.hac_gpio, HAC_ENABLE);
	else
		gpio_set_value(priv->board_config.hac_gpio, HAC_DISABLE);

	return ret;
}
#endif

#ifdef CONFIG_RCV_TDD_SUPPORT
static const char * const rcv_tdd_switch_text[] = {"OFF", "ON"};

static const struct soc_enum rcv_tdd_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(rcv_tdd_switch_text),
		rcv_tdd_switch_text),
};

static int rcv_tdd_status_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	if (codec == NULL) {
		AUDIO_LOGE("pointer is null");
		return -EINVAL;
	}

	priv = snd_soc_component_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return -EINVAL;
	}

	if (!gpio_is_valid(priv->board_config.rcv_tdd_gpio)) {
		AUDIO_LOGI("bt tri gpio = %d is invalid",
			priv->board_config.rcv_tdd_gpio);
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] =
		gpio_get_value(priv->board_config.rcv_tdd_gpio);
	AUDIO_LOGI("rcv tdd gpio = %d, value = %d",
		priv->board_config.bt_tri_gpio,
		ucontrol->value.integer.value[0]);

	return 0;
}

static int rcv_tdd_status_set(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct da_combine_v3_platform_data *priv = NULL;
	struct snd_soc_component *codec = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EINVAL;
	}
	codec = snd_soc_kcontrol_component(kcontrol);
	priv = snd_soc_component_get_drvdata(codec);

	if (!gpio_is_valid(priv->board_config.rcv_tdd_gpio)) {
		AUDIO_LOGE("hac gpio = %d is invalid",
			priv->board_config.rcv_tdd_gpio);
		return -EINVAL;
	}
	ret = ucontrol->value.integer.value[0];
	if (ret == RCV_TDD_ENABLE)
		gpio_set_value(priv->board_config.rcv_tdd_gpio,
			RCV_TDD_ENABLE);
	else
		gpio_set_value(priv->board_config.rcv_tdd_gpio,
			RCV_TDD_DISABLE);

	return ret;
}
#endif

#define S2_CFG_KCONTROLS \
	SOC_SINGLE("S2 IF FS CFG", \
		DA_COMBINE_V3_S2_CLK_CFG_REG, DA_COMBINE_V3_S2_CLK_SEL_BIT, 7, 0), \
	SOC_SINGLE("S2 DSP IF IN FS", \
		DA_COMBINE_V3_S2_CLK_CFG_REG1, DA_COMBINE_V3_DSP_IF_IN_FS_CFG, 7, 0), \
	SOC_SINGLE("S2 DSP IF OUT FS", \
		DA_COMBINE_V3_S2_CLK_CFG_REG1, DA_COMBINE_V3_DSP_IF_OUT_FS_CFG, 7, 0), \
	SOC_SINGLE("S2 IF FUNC MODE CFG", \
		DA_COMBINE_V3_S2_CFG_REG, DA_COMBINE_V3_S2_MODE_CFG_BIT, 5, 0), \
	SOC_SINGLE("S2 IF FRAME MODE CFG", \
		DA_COMBINE_V3_S2_CFG_REG_H, DA_COMBINE_V3_S2_FRAME_MODE_BIT, 1, 0), \
	SOC_SINGLE("S2 RX CLK SEL", \
		DA_COMBINE_V3_S2_CFG_REG_H, DA_COMBINE_V3_S2_RX_CLK_SEL_BIT, 1, 0), \
	SOC_SINGLE("S2 TX CLK SEL", \
		DA_COMBINE_V3_S2_CFG_REG_H, DA_COMBINE_V3_S2_TX_CLK_SEL_BIT, 1, 0), \

#define S3_CFG_KCONTROLS \
	SOC_SINGLE("S3 CLK IF EN", \
		DA_COMBINE_V3_S3_CLK_CFG_REG, DA_COMBINE_V3_S3_CLK_EN_BIT, 1, 0), \
	SOC_SINGLE("S3 CLK FS", \
		DA_COMBINE_V3_S3_CLK_CFG_REG, DA_COMBINE_V3_S3_CLK_FS_SEL_BIT, 7, 0), \
	SOC_SINGLE("S3 OUT LR CLK FS", \
		DA_COMBINE_V3_S3_CLK_CFG_REG1, DA_COMBINE_V3_S3_LR_CLK_FS_BIT, 7, 0), \
	SOC_SINGLE("S3 IF RX EN", \
		DA_COMBINE_V3_S3_CFG_REG_H, DA_COMBINE_V3_S3_IF_RX_EN_BIT, 1, 0), \
	SOC_SINGLE("S3 IF TX EN", \
		DA_COMBINE_V3_S3_CFG_REG_H, DA_COMBINE_V3_S3_IF_TX_EN_BIT, 1, 0), \

#define S_1_3_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("S1 IL PGA GAIN", \
		DA_COMBINE_V3_S1_PGA_IL_GAIN_CFG_REG, DA_COMBINE_V3_S1_PGA_IL_GAIN_BIT, 255, 0), \
	SOC_SINGLE("S1 IR PGA GAIN", \
		DA_COMBINE_V3_S1_PGA_IR_GAIN_CFG_REG, DA_COMBINE_V3_S1_PGA_IR_GAIN_BIT, 255, 0), \
	SOC_SINGLE("S2 IL PGA GAIN", \
		DA_COMBINE_V3_S2_PGA_IL_GAIN_CFG_REG, DA_COMBINE_V3_S2_PGA_IL_GAIN_BIT, 255, 0), \
	SOC_SINGLE("S2 IR PGA GAIN", \
		DA_COMBINE_V3_S2_PGA_IR_GAIN_CFG_REG, DA_COMBINE_V3_S2_PGA_IR_GAIN_BIT, 255, 0), \
	SOC_SINGLE("S3 IL PGA GAIN", \
		DA_COMBINE_V3_S3_PGA_IL_GAIN_CFG_REG, DA_COMBINE_V3_S3_PGA_IL_GAIN_BIT, 255, 0), \
	SOC_SINGLE("S3 IR PGA GAIN", \
		DA_COMBINE_V3_S3_PGA_IR_GAIN_CFG_REG, DA_COMBINE_V3_S3_PGA_IR_GAIN_BIT, 255, 0), \

#define DAC_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("DACL PGA GAIN", \
		DA_COMBINE_V3_DACL_PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_DACL_PGA_GAIN_BIT, 255, 0), \
	SOC_SINGLE("DACR PGA GAIN", \
		DA_COMBINE_V3_DACR_PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_DACR_PGA_GAIN_BIT, 255, 0), \
	SOC_SINGLE("SIDE PGA GAIN", \
		DA_COMBINE_V3_SIDE_PGA_GAIN_CFG_REG, DA_COMBINE_V3_SIDE_PGA_GAIN_BIT, 255, 0), \
	SOC_SINGLE("DACL MIXER S1L GAIN", \
		DA_COMBINE_V3_DACL_MIXER_GAIN_CFG_REG, DA_COMBINE_V3_DACL_MIXER_S1L_GAIN_BIT, 3, 0), \
	SOC_SINGLE("DACL MIXER S1R GAIN", \
		DA_COMBINE_V3_DACL_MIXER_GAIN_CFG_REG, DA_COMBINE_V3_DACL_MIXER_S1R_GAIN_BIT, 3, 0), \
	SOC_SINGLE("DACR MIXER S1L GAIN", \
		DA_COMBINE_V3_DACR_MIXER_GAIN_CFG_REG, DA_COMBINE_V3_DACR_MIXER_S1L_GAIN_BIT, 3, 0), \
	SOC_SINGLE("DACR MIXER S1R GAIN", \
		DA_COMBINE_V3_DACR_MIXER_GAIN_CFG_REG, DA_COMBINE_V3_DACR_MIXER_S1R_GAIN_BIT, 3, 0), \

#define ADC_05PGA_GAIN_KCONTROLS \
	SOC_SINGLE("ADC0L 05PGA GAIN", \
		DA_COMBINE_V3_ADC0L_05PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_ADC0L_05PGA_GAIN_VIRTUAL_BIT, 0x7e, 0), \
	SOC_SINGLE("ADC0R 05PGA GAIN", \
		DA_COMBINE_V3_ADC0R_05PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_ADC0R_05PGA_GAIN_VIRTUAL_BIT, 0x7e, 0), \
	SOC_SINGLE("ADC1L 05PGA GAIN", \
		DA_COMBINE_V3_ADC1L_05PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_ADC1L_05PGA_GAIN_VIRTUAL_BIT, 0x7e, 0), \
	SOC_SINGLE("ADC1R 05PGA GAIN", \
		DA_COMBINE_V3_ADC1R_05PGA_GAIN_VIRTUAL_REG, DA_COMBINE_V3_ADC1R_05PGA_GAIN_VIRTUAL_BIT, 0x7e, 0), \

#define MIC_PGA_GAIN_KCONTROLS \
	SOC_SINGLE_TLV("LINEL PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG20, DA_COMBINE_V3_LINEINL_PGA_GAIN_BIT, 28, 0, lineinl_mic_tlv), \
	SOC_SINGLE_TLV("LINER PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG21, DA_COMBINE_V3_LINEINR_PGA_GAIN_BIT, 28, 0, lineinr_mic_tlv), \
	SOC_SINGLE_TLV("AUX PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG22, DA_COMBINE_V3_AUX_PGA_GAIN_BIT, 18, 0, aux_mic_tlv), \
	SOC_SINGLE_TLV("MAIN PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG23, DA_COMBINE_V3_MAIN_PGA_GAIN_BIT, 18, 0, main_mic_tlv), \
	SOC_SINGLE_TLV("LOL PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG63, DA_COMBINE_V3_LOL_GAIN_BIT, 18, 0, lol_pga_tlv), \
	SOC_SINGLE_TLV("LOR PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG64, DA_COMBINE_V3_LOR_GAIN_BIT, 18, 0, lor_pga_tlv), \
	SOC_SINGLE("EP PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG62, DA_COMBINE_V3_EAR_GAIN_BIT, 22, 0), \
	SOC_SINGLE_TLV("MAD PGA GAIN", \
		DA_COMBINE_V3_ANALOG_REG84, DA_COMBINE_V3_MAD_PGA_GAIN_BIT, 18, 0, mad_tlv), \

#define S2_SRC_KCONTROLS \
	SOC_SINGLE("S2 IL SRC MODE", \
		DA_COMBINE_V3_S2_CONTROL_REG, DA_COMBINE_V3_S2IL_SRC_MODE_BIT, 7, 0), \
	SOC_SINGLE("S2 IR SRC MODE", \
		DA_COMBINE_V3_S2_CONTROL_REG, DA_COMBINE_V3_S2IR_SRC_MODE_BIT, 7, 0), \
	SOC_SINGLE("S2 OL SRC MODE", \
		DA_COMBINE_V3_S2_DSPIF_CONTROL_REG, DA_COMBINE_V3_S2OL_SRC_MODE_BIT, 7, 0), \
	SOC_SINGLE("S2 OR SRC MODE", \
		DA_COMBINE_V3_S2_DSPIF_CONTROL_REG, DA_COMBINE_V3_S2OR_SRC_MODE_BIT, 7, 0), \

#define MIC_BOOST_KCONTROLS \
	SOC_SINGLE("MAIN MIC BOOST0", \
		DA_COMBINE_V3_ANALOG_REG31, DA_COMBINE_V3_ADC0L_BOOST_1_BIT, 1, 0), \
	SOC_SINGLE("MAIN MIC BOOST1", \
		DA_COMBINE_V3_ANALOG_REG36, DA_COMBINE_V3_ADC0L_BOOST_2_BIT, 1, 0), \
	SOC_SINGLE("AUX MIC BOOST0", \
		DA_COMBINE_V3_ANALOG_REG31, DA_COMBINE_V3_ADC0R_BOOST_1_BIT, 1, 0), \
	SOC_SINGLE("AUX MIC BOOST1", \
		DA_COMBINE_V3_ANALOG_REG36, DA_COMBINE_V3_ADC0R_BOOST_2_BIT, 1, 0), \
	SOC_SINGLE("ADC MAD BOOST0", \
		DA_COMBINE_V3_ANALOG_REG088, DA_COMBINE_V3_ADC_MAD_BOOST_1_BIT, 1, 0), \
	SOC_SINGLE("ADC MAD BOOST1", \
		DA_COMBINE_V3_ANALOG_REG088, DA_COMBINE_V3_ADC_MAD_BOOST_2_BIT, 1, 0), \
	SOC_SINGLE("LINEINL BOOST0", \
		DA_COMBINE_V3_ANALOG_REG34, DA_COMBINE_V3_ADC1L_BOOST_1_BIT, 1, 0), \
	SOC_SINGLE("LINEINL BOOST1", \
		DA_COMBINE_V3_ANALOG_REG36, DA_COMBINE_V3_ADC1L_BOOST_2_BIT, 1, 0), \
	SOC_SINGLE("LINEINR BOOST0", \
		DA_COMBINE_V3_ANALOG_REG34, DA_COMBINE_V3_ADC1R_BOOST_1_BIT, 1, 0), \
	SOC_SINGLE("LINEINR BOOST1", \
		DA_COMBINE_V3_ANALOG_REG36, DA_COMBINE_V3_ADC1R_BOOST_2_BIT, 1, 0), \

#define BT_TRI_KCONTROLS \
	SOC_ENUM_EXT("BT TRI", bt_tri_enum[0], \
		bt_tri_status_get, \
		bt_tri_status_set), \

static const struct snd_kcontrol_new snd_controls[] = {
	S2_CFG_KCONTROLS
	S3_CFG_KCONTROLS
	S_1_3_PGA_GAIN_KCONTROLS
	DAC_PGA_GAIN_KCONTROLS
	ADC_05PGA_GAIN_KCONTROLS
	MIC_PGA_GAIN_KCONTROLS
	S2_SRC_KCONTROLS
	MIC_BOOST_KCONTROLS
	BT_TRI_KCONTROLS
#ifdef AUDIO_FACTORY_MODE
	SOC_ENUM_EXT("MAIN MICBIAS", micbias_enum[0],
		main_micbias_status_get,
		main_micbias_status_put),
#endif
#ifdef CONFIG_HAC_SUPPORT
	SOC_ENUM_EXT("HAC", hac_switch_enum[0],
		hac_status_get,
		hac_status_set),
#endif
#ifdef CONFIG_RCV_TDD_SUPPORT
	SOC_ENUM_EXT("RCV TDD", rcv_tdd_switch_enum[0],
		rcv_tdd_status_get,
		rcv_tdd_status_set),
#endif
};

int add_kcontrol(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return snd_soc_add_component_controls(codec, snd_controls,
		ARRAY_SIZE(snd_controls));
}

