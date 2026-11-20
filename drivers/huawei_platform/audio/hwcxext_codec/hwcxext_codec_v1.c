/*
 * hwcxext_codec_v1.c
 *
 * hwcxext codec v1 driver
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#include <linux/module.h>

#include <linux/delay.h>
#include <linux/mutex.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <huawei_platform/log/hw_log.h>

#include <dsm/dsm_pub.h>
#ifdef CONFIG_HUAWEI_DSM_AUDIO
#include <dsm_audio/dsm_audio.h>
#endif
#ifdef CONFIG_HWEVEXT_EXTERN_ADC
#include <huawei_platform/audio/hwevext_adc.h>
#endif

#include "hwcxext_codec_v1.h"
#include "hwcxext_mbhc.h"
#include "hwcxext_codec_info.h"

#define HWLOG_TAG hwcxext_codec
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

#define MAX_TIMES_HS_RECOGNIZE 12
#define HS_RECOGNIZE_BEGIN_DELAY 60
#define HS_RECOGNIZE_PROCESS_DELAY 20
#define HS_RECOGNIZE_LIMIT_TIME_MS 900

#ifndef UNUSED_PARAMETER
#define UNUSED_PARAMETER(x) (void)(x)
#endif

struct codec_init_regs {
	unsigned int addr;
	unsigned int value;
	unsigned int sleep_us;
};

/*
 * DAC/ADC Volume
 *
 * max : 74 : 0 dB
 * ( in 1 dB  step )
 * min : 0 : -74 dB
 */
static const DECLARE_TLV_DB_SCALE(adc_tlv, -7400, 100, 0);
static const DECLARE_TLV_DB_SCALE(dac_tlv, -7400, 100, 0);
static const DECLARE_TLV_DB_SCALE(boost_tlv, 0, 1200, 0);

static void hwcxext_codec_v1_snd_soc_update_bits(
	struct snd_soc_component *component,
	unsigned int reg, unsigned int mask, unsigned int value)
{
	int ret;

	ret = snd_soc_component_update_bits(component, reg, mask, value);
	if (ret < 0)
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_WRITE, ret);
}

static int hwcxext_codec_v1_i2c_dump_regs(
	struct hwcxext_codec_regmap_cfg *cfg)
{
	int i;
	unsigned int *regs = NULL;
	unsigned int value;

	if (IS_ERR_OR_NULL(cfg))
		return -EINVAL;

	if (IS_ERR_OR_NULL(cfg->regmap))
		return -EINVAL;

	regs = cfg->reg_readable;
	if (regs == NULL || cfg->num_readable <= 0) {
		hwlog_info("%s: reg_readable is not set\n", __func__);
		return -EINVAL;
	}

	for (i = 0; i < cfg->num_readable; i++) {
		regmap_read(cfg->regmap, regs[i], &value);
		hwlog_info("%s: read reg 0x%08x=0x%08x\n",
			__func__, regs[i], value);
	}

	return 0;
}

static void hwcxext_codec_v1_dump_regs(struct hwcxext_mbhc_priv *mbhc)
{
	struct hwcxext_codec_v1_priv *hwcxext_codec = NULL;

	if (mbhc == NULL) {
		hwlog_err("%s: params is inavild\n", __func__);
		return;
	}

	hwcxext_codec = snd_soc_component_get_drvdata(mbhc->component);
	if (hwcxext_codec == NULL) {
		hwlog_err("%s: hwcxext_codec is inavild\n", __func__);
		return;
	}

	hwcxext_codec_v1_i2c_dump_regs(hwcxext_codec->regmap_cfg);
}

static const struct snd_kcontrol_new hwcxext_codec_v1_snd_controls[] = {
	SOC_DOUBLE_R_TLV("PortD Boost Volume",
		HWCXEXT_CODEC_V1_PORTD_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTD_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortC Boost Volume",
		HWCXEXT_CODEC_V1_PORTC_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTC_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortB Boost Volume",
		HWCXEXT_CODEC_V1_PORTB_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTB_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortF Boost Volume",
		HWCXEXT_CODEC_V1_PORTF_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTF_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortH Boost Volume",
		HWCXEXT_CODEC_V1_PORTH_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTH_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortK Boost Volume",
		HWCXEXT_CODEC_V1_PORTK_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_PORTK_AMP_GAIN_RIGHT, 0, 3, 0, boost_tlv),
	SOC_DOUBLE_R_TLV("PortB ADC1 Volume",
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_LEFT_0,
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_RIGHT_0, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("PortD ADC1 Volume",
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_LEFT_1,
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_RIGHT_1, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("PortC ADC1 Volume",
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_LEFT_2,
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_RIGHT_2, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("PortH ADC1 Volume",
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_LEFT_3,
		HWCXEXT_CODEC_V1_ADC1_AMP_GAIN_RIGHT_3, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("PortF ADC3 Volume",
		HWCXEXT_CODEC_V1_ADC3_AMP_GAIN_LEFT_0,
		HWCXEXT_CODEC_V1_ADC3_AMP_GAIN_RIGHT_0, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("PortK ADC4 Volume",
		HWCXEXT_CODEC_V1_ADC4_AMP_GAIN_LEFT_0,
		HWCXEXT_CODEC_V1_ADC4_AMP_GAIN_RIGHT_0, 0, 0x4a, 0, adc_tlv),
	SOC_DOUBLE_R_TLV("DAC1 Volume",
		HWCXEXT_CODEC_V1_DAC1_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC1_AMP_GAIN_RIGHT, 0, 0x4a, 0, dac_tlv),
	SOC_DOUBLE_R("DAC1 Switch", HWCXEXT_CODEC_V1_DAC1_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC1_AMP_GAIN_RIGHT, 7,  1, 0),
	SOC_DOUBLE_R_TLV("DAC2 Volume",
		HWCXEXT_CODEC_V1_DAC2_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC2_AMP_GAIN_RIGHT, 0, 0x4a, 0, dac_tlv),
	SOC_DOUBLE_R("DAC2 Switch", HWCXEXT_CODEC_V1_DAC2_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC2_AMP_GAIN_RIGHT, 7,  1, 0),
	SOC_DOUBLE_R_TLV("DAC4 Volume",
		HWCXEXT_CODEC_V1_DAC4_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC4_AMP_GAIN_RIGHT, 0, 0x4a, 0, dac_tlv),
	SOC_DOUBLE_R_TLV("DAC5 Volume",
		HWCXEXT_CODEC_V1_DAC5_AMP_GAIN_LEFT,
		HWCXEXT_CODEC_V1_DAC5_AMP_GAIN_RIGHT, 0, 0x4a, 0, dac_tlv),
	SOC_SINGLE("PortA HP Amp Switch",
		HWCXEXT_CODEC_V1_PORTA_PIN_CTRL, 7, 1, 0),
};

/* dapm kcontrol define */
static const struct snd_kcontrol_new portaouten_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTA_PIN_CTRL, 6, 1, 0);

static const struct snd_kcontrol_new portgouten_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTG_PIN_CTRL, 6, 1, 0);

static const struct snd_kcontrol_new porteouten_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTE_PIN_CTRL, 6, 1, 0);

static const struct snd_kcontrol_new portjouten_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTJ_PIN_CTRL, 6, 1, 0);

static const struct snd_kcontrol_new portbinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTB_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new portcinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTC_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new portdinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTD_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new portfinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTF_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new porthinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTH_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new portkinen_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_PORTK_PIN_CTRL, 5, 1, 0);

static const struct snd_kcontrol_new i2sadc1l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 0, 1, 0);

static const struct snd_kcontrol_new i2sadc1r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 1, 1, 0);

static const struct snd_kcontrol_new i2sadc2l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 2, 1, 0);

static const struct snd_kcontrol_new i2sadc2r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 3, 1, 0);

static const struct snd_kcontrol_new i2sadc3l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 4, 1, 0);

static const struct snd_kcontrol_new i2sadc3r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL2, 5, 1, 0);

static const struct snd_kcontrol_new i2sadc4l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL7, 5, 1, 0);

static const struct snd_kcontrol_new i2sadc4r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL7, 6, 1, 0);

static const struct snd_kcontrol_new i2sdac1l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL3, 0, 1, 0);

static const struct snd_kcontrol_new i2sdac1r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL3, 1, 1, 0);

static const struct snd_kcontrol_new i2sdac2l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL3, 2, 1, 0);

static const struct snd_kcontrol_new i2sdac2r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL3, 3, 1, 0);

static const struct snd_kcontrol_new i2sdac4l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL10, 20, 1, 0);

static const struct snd_kcontrol_new i2sdac4r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL10, 21, 1, 0);

static const struct snd_kcontrol_new i2sdac5l_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL10, 22, 1, 0);

static const struct snd_kcontrol_new i2sdac5r_ctl =
	SOC_DAPM_SINGLE("Switch", HWCXEXT_CODEC_V1_I2SPCM_CONTROL10, 23, 1, 0);

static const char * const dac_enum_text[] = {
	"DAC1 Switch", "DAC2 Switch",
};

static const struct soc_enum porta_dac_enum =
	SOC_ENUM_SINGLE(HWCXEXT_CODEC_V1_PORTA_CONNECTION_SELECT,
		0, 2, dac_enum_text);

static const struct snd_kcontrol_new porta_mux =
	SOC_DAPM_ENUM("PortA Mux", porta_dac_enum);

static const struct soc_enum portg_dac_enum =
	SOC_ENUM_SINGLE(HWCXEXT_CODEC_V1_PORTG_CONNECTION_SELECT,
		0, 2, dac_enum_text);

static const struct snd_kcontrol_new portg_mux =
	SOC_DAPM_ENUM("PortG Mux", portg_dac_enum);

static const char * const adc1in_sel_text[] = {
	"PortB Switch", "PortD Switch", "PortC Switch",
	"Widget15 Switch", "PortH Switch",
};

static const struct soc_enum adc1in_sel_enum =
	SOC_ENUM_SINGLE(HWCXEXT_CODEC_V1_ADC1_CONNECTION_SELECT,
		0, 5, adc1in_sel_text);

static const struct snd_kcontrol_new adc1_mux =
	SOC_DAPM_ENUM("ADC1 Mux", adc1in_sel_enum);

static const char * const adc2in_sel_text[] = {
	"PortC Switch", "PortH Switch", "Widget15 Switch",
};

static const struct soc_enum adc2in_sel_enum =
	SOC_ENUM_SINGLE(HWCXEXT_CODEC_V1_ADC2_CONNECTION_SELECT,
	0, 3, adc2in_sel_text);

static const struct snd_kcontrol_new adc2_mux =
	SOC_DAPM_ENUM("ADC2 Mux", adc2in_sel_enum);

static const struct snd_kcontrol_new wid15_mix[] = {
	SOC_DAPM_SINGLE("DAC1L Switch",
		HWCXEXT_CODEC_V1_MIXER_AMP_GAIN_LEFT_0, 7, 1, 1),
	SOC_DAPM_SINGLE("DAC1R Switch",
		HWCXEXT_CODEC_V1_MIXER_AMP_GAIN_RIGHT_0, 7, 1, 1),
	SOC_DAPM_SINGLE("DAC2L Switch",
		HWCXEXT_CODEC_V1_MIXER_AMP_GAIN_LEFT_1, 7, 1, 1),
	SOC_DAPM_SINGLE("DAC2R Switch",
		HWCXEXT_CODEC_V1_MIXER_AMP_GAIN_RIGHT_1, 7, 1, 1),
};

static int hwcxext_codec_v1_afg_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_AFG_POWER_STATE,
			0xff, 0x00);

		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DIGITAL_BIOS_TEST0_LSB,
			0x10, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DIGITAL_BIOS_TEST0_LSB,
			0x10, 0x10);

		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_AFG_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}
	return 0;
}

static int hwcxext_codec_v1_portb_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTB_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTB_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portc_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTC_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTC_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portd_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTD_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTD_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_porth_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTH_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTH_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portf_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTF_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTF_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portk_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTK_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTK_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_widget15_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_MIXER_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_MIXER_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_adc1_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC1_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC1_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_adc2_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC2_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC2_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_adc3_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC3_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC3_POWER_STATE,
			0xff, 0x03);
	break;
	default:
		hwlog_err("%s: event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_adc4_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC4_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ADC4_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

int hwcxext_codec_v1_headset_micbias_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* PortD Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTD_PIN_CTRL,
			0x04, 0x04);
		/* Headset Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ANALOG_TEST11,
			0x02, 0x02);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* Headset Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_ANALOG_TEST11,
			0x02, 0x00);
		/* PortD Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTD_PIN_CTRL,
			0x04, 0x00);
		break;
	default:
		hwlog_warn("event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

int hwcxext_codec_v1_portb_micbias_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* PortB Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTB_PIN_CTRL,
			0x04, 0x04);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* PortB Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTB_PIN_CTRL,
			0x04, 0x00);
		break;
	default:
		hwlog_warn("event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

int hwcxext_codec_v1_portc_micbias_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* PortC Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTC_PIN_CTRL,
			0x04, 0x04);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* PortC Mic Bias */
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTC_PIN_CTRL,
			0x04, 0x00);
		break;
	default:
		hwlog_warn("event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_porta_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTA_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTA_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portg_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTG_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTG_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_porte_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTE_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTE_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_portj_power_event(
	struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTJ_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_PORTJ_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: power mode event err : %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_dac1_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC1_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC1_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_dac2_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC2_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC2_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_dac4_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC4_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC4_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

static int hwcxext_codec_v1_dac5_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC5_POWER_STATE,
			0xff, 0x00);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hwcxext_codec_v1_snd_soc_update_bits(component,
			HWCXEXT_CODEC_V1_DAC5_POWER_STATE,
			0xff, 0x03);
		break;
	default:
		hwlog_err("%s: event err: %d\n", __func__, event);
		break;
	}

	return 0;
}

/* dapm widgets */
static const struct snd_soc_dapm_widget hwcxext_codec_v1_dapm_widgets[] = {
	/* Playback */
	SND_SOC_DAPM_INPUT("In AIF"),

	SND_SOC_DAPM_SWITCH("I2S DAC1L", SND_SOC_NOPM, 0, 0, &i2sdac1l_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC1R", SND_SOC_NOPM, 0, 0, &i2sdac1r_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC2L", SND_SOC_NOPM, 0, 0, &i2sdac2l_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC2R", SND_SOC_NOPM, 0, 0, &i2sdac2r_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC4L", SND_SOC_NOPM, 0, 0, &i2sdac4l_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC4R", SND_SOC_NOPM, 0, 0, &i2sdac4r_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC5L", SND_SOC_NOPM, 0, 0, &i2sdac5l_ctl),
	SND_SOC_DAPM_SWITCH("I2S DAC5R", SND_SOC_NOPM, 0, 0, &i2sdac5r_ctl),

	SND_SOC_DAPM_DAC_E("DAC1", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_dac1_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_DAC_E("DAC2", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_dac2_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_DAC_E("DAC4", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_dac4_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_DAC_E("DAC5", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_dac5_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_MUX("PortA Mux", SND_SOC_NOPM, 0, 0, &porta_mux),
	SND_SOC_DAPM_MUX("PortG Mux", SND_SOC_NOPM, 0, 0, &portg_mux),

	/* SUPPLY */
	SND_SOC_DAPM_SUPPLY("PortA Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_porta_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SUPPLY("PortG Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portg_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SUPPLY("PortE Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_porte_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SUPPLY("PortJ Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portj_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_SWITCH("PortA Out En", SND_SOC_NOPM, 0, 0,
		&portaouten_ctl),
	SND_SOC_DAPM_SWITCH("PortG Out En", SND_SOC_NOPM, 0, 0,
		&portgouten_ctl),
	SND_SOC_DAPM_SWITCH("PortE Out En", SND_SOC_NOPM, 0, 0,
		&porteouten_ctl),
	SND_SOC_DAPM_SWITCH("PortJ Out En", SND_SOC_NOPM, 0, 0,
		&portjouten_ctl),
	SND_SOC_DAPM_OUTPUT("PORTA"),
	SND_SOC_DAPM_OUTPUT("PORTG"),
	SND_SOC_DAPM_OUTPUT("PORTE"),
	SND_SOC_DAPM_OUTPUT("PORTJ"),

	/* Capture */
	SND_SOC_DAPM_OUTPUT("Out AIF"),

	SND_SOC_DAPM_SWITCH("I2S ADC1L", SND_SOC_NOPM, 0, 0, &i2sadc1l_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC1R", SND_SOC_NOPM, 0, 0, &i2sadc1r_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC2L", SND_SOC_NOPM, 0, 0, &i2sadc2l_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC2R", SND_SOC_NOPM, 0, 0, &i2sadc2r_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC3L", SND_SOC_NOPM, 0, 0, &i2sadc3l_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC3R", SND_SOC_NOPM, 0, 0, &i2sadc3r_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC4L", SND_SOC_NOPM, 0, 0, &i2sadc4l_ctl),
	SND_SOC_DAPM_SWITCH("I2S ADC4R", SND_SOC_NOPM, 0, 0, &i2sadc4r_ctl),

	/* ADC */
	SND_SOC_DAPM_ADC_E("ADC1", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_adc1_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_ADC_E("ADC2", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_adc2_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_ADC_E("ADC3", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_adc3_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_ADC_E("ADC4", NULL, SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_adc4_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),
	SND_SOC_DAPM_MUX("ADC1 Mux", SND_SOC_NOPM, 0, 0, &adc1_mux),
	SND_SOC_DAPM_MUX("ADC2 Mux", SND_SOC_NOPM, 0, 0, &adc2_mux),

	/* SUPPLY */
	SND_SOC_DAPM_SUPPLY("AFG Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_afg_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortB Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portb_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortC Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portc_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortD Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portd_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortH Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_porth_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortF Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portf_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("PortK Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_portk_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_SUPPLY("Widget15 Power", SND_SOC_NOPM, 0, 0,
		hwcxext_codec_v1_widget15_power_event,
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),

	SND_SOC_DAPM_MIXER("Widget15 Mixer", SND_SOC_NOPM, 0, 0,
		wid15_mix, ARRAY_SIZE(wid15_mix)),
	SND_SOC_DAPM_SWITCH("PortB In En", SND_SOC_NOPM, 0, 0, &portbinen_ctl),
	SND_SOC_DAPM_SWITCH("PortC In En", SND_SOC_NOPM, 0, 0, &portcinen_ctl),
	SND_SOC_DAPM_SWITCH("PortD In En", SND_SOC_NOPM, 0, 0, &portdinen_ctl),
	SND_SOC_DAPM_SWITCH("PortH In En", SND_SOC_NOPM, 0, 0, &porthinen_ctl),
	SND_SOC_DAPM_SWITCH("PortF In En", SND_SOC_NOPM, 0, 0, &portfinen_ctl),
	SND_SOC_DAPM_SWITCH("PortK In En", SND_SOC_NOPM, 0, 0, &portkinen_ctl),

	/* MICBIAS */
	SND_SOC_DAPM_MIC("Headset Bias", hwcxext_codec_v1_headset_micbias_event),
	SND_SOC_DAPM_MIC("PortB Mic Bias", hwcxext_codec_v1_portb_micbias_event),
	SND_SOC_DAPM_MIC("PortC Mic Bias", hwcxext_codec_v1_portc_micbias_event),

	SND_SOC_DAPM_INPUT("PORTB"),
	SND_SOC_DAPM_INPUT("PORTC"),
	SND_SOC_DAPM_INPUT("PORTD"),
	SND_SOC_DAPM_INPUT("PORTH"),
	SND_SOC_DAPM_INPUT("PORTF"),
	SND_SOC_DAPM_INPUT("PORTK"),
};

static const struct snd_soc_dapm_route hwcxext_codec_v1_dapm_routes[] = {
	/* Playback */
	{"In AIF", NULL, "AFG Power"},
	{"I2S DAC1L", "Switch", "In AIF"},
	{"I2S DAC1R", "Switch", "In AIF"},
	{"I2S DAC2L", "Switch", "In AIF"},
	{"I2S DAC2R", "Switch", "In AIF"},
	{"I2S DAC4L", "Switch", "In AIF"},
	{"I2S DAC4R", "Switch", "In AIF"},
	{"I2S DAC5L", "Switch", "In AIF"},
	{"I2S DAC5R", "Switch", "In AIF"},
	{"DAC1", NULL, "I2S DAC1L"},
	{"DAC1", NULL, "I2S DAC1R"},
	{"DAC2", NULL, "I2S DAC2L"},
	{"DAC2", NULL, "I2S DAC2R"},
	{"DAC4", NULL, "I2S DAC4L"},
	{"DAC4", NULL, "I2S DAC4R"},
	{"DAC5", NULL, "I2S DAC5L"},
	{"DAC5", NULL, "I2S DAC5R"},
	{"PortA Mux", "DAC1 Switch", "DAC1"},
	{"PortA Mux", "DAC2 Switch", "DAC2"},
	{"PortG Mux", "DAC1 Switch", "DAC1"},
	{"PortG Mux", "DAC2 Switch", "DAC2"},
	{"Widget15 Mixer", "DAC1L Switch", "DAC1"},
	{"Widget15 Mixer", "DAC1R Switch", "DAC2"},
	{"Widget15 Mixer", "DAC2L Switch", "DAC1"},
	{"Widget15 Mixer", "DAC2R Switch", "DAC2"},
	{"PortE Out En", "Switch", "DAC4"},
	{"PortJ Out En", "Switch", "DAC5"},
	{"Widget15 Mixer", NULL, "Widget15 Power"},
	{"PortA Out En", "Switch", "PortA Mux"},
	{"PortG Out En", "Switch", "PortG Mux"},
	{"PortA Mux", NULL, "PortA Power"},
	{"PortG Mux", NULL, "PortG Power"},
	{"PortA Out En", NULL, "PortA Power"},
	{"PortG Out En", NULL, "PortG Power"},
	{"PortE Out En", NULL, "PortE Power"},
	{"PortJ Out En", NULL, "PortJ Power"},
	{"PORTA", NULL, "PortA Out En"},
	{"PORTG", NULL, "PortG Out En"},
	{"PORTE", NULL, "PortE Out En"},
	{"PORTJ", NULL, "PortJ Out En"},

	/* Capture */
	{"PORTD", NULL, "Headset Bias"},
	{"PortB In En", "Switch", "PORTB"},
	{"PortC In En", "Switch", "PORTC"},
	{"PortD In En", "Switch", "PORTD"},
	{"PortF In En", "Switch", "PORTF"},
	{"PortK In En", "Switch", "PORTK"},
	{"ADC1 Mux", "PortB Switch", "PortB In En"},
	{"ADC1 Mux", "PortC Switch", "PortC In En"},
	{"ADC1 Mux", "PortD Switch", "PortD In En"},
	{"ADC1 Mux", "Widget15 Switch", "Widget15 Mixer"},
	{"ADC2 Mux", "PortC Switch", "PortC In En"},
	{"ADC2 Mux", "Widget15 Switch", "Widget15 Mixer"},
	{"ADC1", NULL, "ADC1 Mux"},
	{"ADC2", NULL, "ADC2 Mux"},
	{"ADC3", NULL, "PortF In En"},
	{"ADC4", NULL, "PortK In En"},
	{"I2S ADC1L", "Switch", "ADC1"},
	{"I2S ADC1R", "Switch", "ADC1"},
	{"I2S ADC2L", "Switch", "ADC2"},
	{"I2S ADC2R", "Switch", "ADC2"},
	{"I2S ADC3L", "Switch", "ADC3"},
	{"I2S ADC3R", "Switch", "ADC3"},
	{"I2S ADC4L", "Switch", "ADC4"},
	{"I2S ADC4R", "Switch", "ADC4"},
	{"Out AIF", NULL, "I2S ADC1L"},
	{"Out AIF", NULL, "I2S ADC1R"},
	{"Out AIF", NULL, "I2S ADC2L"},
	{"Out AIF", NULL, "I2S ADC2R"},
	{"Out AIF", NULL, "I2S ADC3L"},
	{"Out AIF", NULL, "I2S ADC3R"},
	{"Out AIF", NULL, "I2S ADC4L"},
	{"Out AIF", NULL, "I2S ADC4R"},
	{"Out AIF", NULL, "AFG Power"},
	{"PortB In En", NULL, "PortB Power"},
	{"PortC In En", NULL, "PortC Power"},
	{"PortD In En", NULL, "PortD Power"},
	{"PortF In En", NULL, "PortF Power"},
	{"PortK In En", NULL, "PortK Power"},
};


static int hwcxext_codec_v1_set_dai_sysclk(
	struct snd_soc_dai *codec_dai,
	int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int hwcxext_codec_v1_set_dai_fmt(struct snd_soc_dai *codec_dai,
	unsigned int fmt)
{
	return 0;
}

static int hwcxext_codec_v1_pcm_hw_params(
	struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int hwcxext_codec_v1_set_dai_bclk_ratio(
	struct snd_soc_dai *dai,
	unsigned int ratio)
{
	return 0;
}

static const struct snd_soc_dai_ops hwevext_codec_v1_ops = {
	.hw_params = hwcxext_codec_v1_pcm_hw_params,
	.set_fmt = hwcxext_codec_v1_set_dai_fmt,
	.set_sysclk = hwcxext_codec_v1_set_dai_sysclk,
	.set_bclk_ratio = hwcxext_codec_v1_set_dai_bclk_ratio,
};

static struct snd_soc_dai_driver hwcxext_codec_v1_dai = {
	.name = "HWCXEXT HiFi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = HWCXEXT_CODEC_V1_RATES,
		.formats = HWCXEXT_CODEC_V1_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = HWCXEXT_CODEC_V1_RATES,
		.formats = HWCXEXT_CODEC_V1_FORMATS,
	},
	.ops = &hwevext_codec_v1_ops,
	.symmetric_rates = 1,
};

static bool hwcxext_codec_v1_mbhc_check_headset_in(
	struct hwcxext_mbhc_priv *mbhc)
{
	int ret;
	unsigned int flags;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return false;
	}

	ret = snd_soc_component_read(mbhc->component,
		HWCXEXT_CODEC_V1_PORTA_PIN_SENSE, &flags);
	if (ret) {
		hwlog_err("%s: read reg err:%d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		return false;
	}
	flags = flags >> HWCXEXT_CODEC_V1_HP_INSERTED_BIT;

	if (flags & HWCXEXT_CODEC_V1_FLAG_HP_INSERTED) {
		hwlog_info("%s:headset is %s, gpio flags(0x%x): %#04x\n",
			__func__, "plugin",
			HWCXEXT_CODEC_V1_PORTA_PIN_SENSE, flags);
		return true;
	}

	hwlog_info("%s:headset is %s, gpio flags(0x%x): %#04x\n",
		__func__, "plugout",
		HWCXEXT_CODEC_V1_PORTA_PIN_SENSE, flags);

	return false;
}

static void hwcxext_codec_v1_enable_micbias_for_hs_detect(
	struct hwcxext_mbhc_priv *mbhc)
{
	UNUSED_PARAMETER(mbhc);
}

static void hwcxext_codec_v1_disable_micbias_for_hs_detect(
	struct hwcxext_mbhc_priv *mbhc)
{
	IN_FUNCTION;
	/* disable manual mode, disable micbias in mic */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_CODEC_TEST39,
		0x83f, 0x000);
}

static void hwcxext_codec_v1_enable_jack_detect(struct hwcxext_mbhc_priv *mbhc,
	bool support_usb_switch)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return;
	}

	IN_FUNCTION;
	dapm = snd_soc_component_get_dapm(mbhc->component);
	/* No-sticky input type */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_AFG_GPIO_STICKY_MASK,
		0xff, 0x1f);

	/* Use GPOI0 as interrupt pin */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_UM_INTERRUPT_CRTL,
		0x10, 0x10);

	if (!support_usb_switch)
		/* Enables unsolitited message on PortA */
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_PORTA_UNSOLICITED_RESPONSE,
			0xff, 0x80);

	/* support both nokia and apple headset set. Monitor time = 275 ms */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_DIGITAL_TEST15,
		0xff, 0x87);

	/* Disable TIP detection */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_ANALOG_TEST12,
		0xfff, 0x300);

	/* Switch MusicD3Live pin to GPIO */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_DIGITAL_TEST1,
		0xff, 0);
}

static unsigned int hwcxext_codec_v1_get_hs_type(struct hwcxext_mbhc_priv *mbhc)
{
	unsigned int hs_type;
	int ret;

	ret = snd_soc_component_read(mbhc->component,
		HWCXEXT_CODEC_V1_DIGITAL_TEST11, &hs_type);
	if (ret) {
		hwlog_err("%s: read reg err:%d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		return AUDIO_JACK_NONE;
	}

	hwlog_info("%s: hs_type 0x%x\n", __func__, hs_type);
	hs_type = hs_type >> 8;
	if (hs_type & 0x8) {
		hwlog_info("%s: headphone is 4 pole\n", __func__);
		return AUDIO_JACK_HEADSET;
	} else if (hs_type & 0x4) {
		hwlog_info("%s: headphone is invert 4 pole\n", __func__);
		return AUDIO_JACK_INVERT;
	} else {
		hwlog_info("%s: headphone is 3 pole\n", __func__);
		return AUDIO_JACK_HEADPHONE;
	}
}

static unsigned int hwcxext_codec_v1_repeat_get_correct_hs_type(
	struct hwcxext_mbhc_priv *mbhc,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	unsigned int type;
	unsigned int count = 0;
	unsigned long timeout;
	unsigned int pre_type = AUDIO_JACK_NONE;

	timeout = jiffies + msecs_to_jiffies(HS_RECOGNIZE_LIMIT_TIME_MS);
	msleep(HS_RECOGNIZE_BEGIN_DELAY);
	do {
		type = hwcxext_codec_v1_get_hs_type(mbhc);
		if (pre_type == type) {
			count++;
		} else {
			count = 1; // it begin repeat to count
			pre_type = type;
		}
		hwlog_info("%s: type:%d, pre_type:%d, count:%d\n", __func__,
			type, pre_type, count);
		if (time_after(jiffies, timeout)) {
			hwlog_warn("%s: it exceed the limit time\n", __func__);
			break;
		}
		msleep(HS_RECOGNIZE_PROCESS_DELAY);
	} while (count < hwcxext_codec->max_times_hs_recognize);

	hwlog_info("%s: hs type is %u\n", __func__, type);
	return type;
}

static int hwcxext_codec_v1_hs_type_recognize(
	struct hwcxext_mbhc_priv *mbhc)
{
	unsigned int type;
	struct hwcxext_codec_v1_priv *hwcxext_codec = NULL;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return AUDIO_JACK_INVAILD;
	}

	hwcxext_codec = snd_soc_component_get_drvdata(mbhc->component);
	if (hwcxext_codec == NULL) {
		hwlog_err("%s: hwcxext_codec is inavild\n", __func__);
		return AUDIO_JACK_INVAILD;
	}

	type = hwcxext_codec_v1_repeat_get_correct_hs_type(mbhc, hwcxext_codec);
	if (type == AUDIO_JACK_INVERT) {
		hwlog_info("%s: out invert 4 pole is handle as 4 pole\n",
			__func__);
		type = AUDIO_JACK_HEADSET;
		/* disable manual mode, disable micbias in mic */
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST39,
			0x83f, 0x000);
		/* enable manual mode, enable micbias in mic */
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST39,
			0x82f, 0x82f);
	} else if (type == AUDIO_JACK_HEADSET) {
		hwlog_info("%s: out handle as 4 pole\n", __func__);
		/* disable manual mode, disable micbias in mic */
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST39,
			0x83f, 0x000);
		/* enable manual mode, enable micbias in mic */
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST39,
			0x81f, 0x81f);
	}

	/* clear interrupt */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_UM_INTERRUPT_CRTL,
		0x10, 0x10);

	return type;
}

static void hwcxext_codec_v1_get_btn_type_from_mb_status(
	struct hwcxext_mbhc_priv *mbhc,
	unsigned int mb_status)
{
	if (mb_status & HWCXEXT_CODEC_V1_MB_B1) {
		hwlog_info("%s: btn hook\n", __func__);
		mbhc->get_btn_info.btn_type = BTN_HOOK;
	} else if (mb_status & HWCXEXT_CODEC_V1_MB_B2) {
		hwlog_info("%s: volume up\n", __func__);
		mbhc->get_btn_info.btn_type = BTN_UP;
	} else if (mb_status & HWCXEXT_CODEC_V1_MB_B3) {
		hwlog_info("%s: volume down\n", __func__);
		mbhc->get_btn_info.btn_type = BTN_DOWN;
	} else {
		hwlog_info("%s: btn type is invail status\n", __func__);
		mbhc->get_btn_info.btn_type = BTN_TYPE_INVAILD;
		mbhc->get_btn_info.btn_event = BTN_ENENT_INVAILD;
	}

	mbhc->get_btn_info.btn_press_times = mb_status >> 8;
	hwlog_info("%s: btn_event:%d, btn press_times:%d, btn_type:%d\n",
		__func__, mbhc->get_btn_info.btn_event,
		mbhc->get_btn_info.btn_press_times,
		mbhc->get_btn_info.btn_type);
}

static int hwcxext_codec_v1_btn_type_recognize(
	struct hwcxext_mbhc_priv *mbhc)
{
	int ret;
	unsigned int mb_status;

	if (mbhc == NULL) {
		hwlog_err("%s: params is inavild\n", __func__);
		return -EINVAL;
	}

	ret = snd_soc_component_read(mbhc->component,
		HWCXEXT_CODEC_V1_CODEC_TEST37, &mb_status);
	if (ret) {
		hwlog_err("%s: read reg err:%d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		return -EINVAL;
	}

	/* clear interrupt */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_UM_INTERRUPT_CRTL,
		0x10, 0x10);

	/* clear mb status */
	hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
		HWCXEXT_CODEC_V1_CODEC_TEST37,
		0x7ff, 0x7ff);

	mbhc->get_btn_info.btn_event = BTN_ENENT_INVAILD;
	hwlog_info("%s: mb_status 0x%04x\n", __func__, mb_status);

	if (mb_status & HWCXEXT_CODEC_V1_MB_DN) {
		if (mb_status & HWCXEXT_CODEC_V1_MB_UP) {
			hwlog_info("%s: btn is release\n", __func__);
			mbhc->get_btn_info.btn_event = BTN_RELEASED;
		} else {
			hwlog_info("%s: btn is pressed\n", __func__);
				mbhc->get_btn_info.btn_event = BTN_PRESS;
		}
	}

	if (mbhc->get_btn_info.btn_event == BTN_ENENT_INVAILD) {
		hwlog_info("%s: btn_event is invaild\n", __func__);
		return -EINVAL;
	}

	hwcxext_codec_v1_get_btn_type_from_mb_status(mbhc, mb_status);
	return 0;
}

static void hwcxext_codec_v1_set_hs_detec_restart(
	struct hwcxext_mbhc_priv *mbhc,
	int status)
{
	int ret;
	int restart_status;

	if (mbhc == NULL) {
		hwlog_err("%s: params is inavild\n", __func__);
		return;
	}

	if (status == HS_DETECT_RESTART_BEGIN)
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST38,
			0x20, 0x20);
	else
		hwcxext_codec_v1_snd_soc_update_bits(mbhc->component,
			HWCXEXT_CODEC_V1_CODEC_TEST38,
			0x20, 0x00);

	ret = snd_soc_component_read(mbhc->component,
		HWCXEXT_CODEC_V1_CODEC_TEST38, &restart_status);
	if (ret) {
		hwlog_err("%s: read reg err:%d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		return;
	}

	hwlog_info("%s: restart_status: 0x%x\n", __func__, restart_status);
}

static struct hwcxext_mbhc_cb mbhc_cb = {
	.mbhc_check_headset_in = hwcxext_codec_v1_mbhc_check_headset_in,
	.enable_micbias = hwcxext_codec_v1_enable_micbias_for_hs_detect,
	.disable_micbias = hwcxext_codec_v1_disable_micbias_for_hs_detect,
	.enable_jack_detect = hwcxext_codec_v1_enable_jack_detect,
	.get_hs_type_recognize = hwcxext_codec_v1_hs_type_recognize,
	.get_btn_type_recognize = hwcxext_codec_v1_btn_type_recognize,
	.dump_regs = hwcxext_codec_v1_dump_regs,
	.set_hs_detec_restart = hwcxext_codec_v1_set_hs_detec_restart,
};

static int hwcxext_codec_v1_init_pmu_audioclk(
	struct snd_soc_component *component,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	bool need_pmu_audioclk = false;
	int ret;

	need_pmu_audioclk = of_property_read_bool(component->dev->of_node,
		"clk_pmuaudioclk");
	if (!need_pmu_audioclk) {
		hwlog_info("%s: it no need pmu audioclk\n", __func__);
		return 0;
	}

	hwcxext_codec->mclk = devm_clk_get(&hwcxext_codec->i2c->dev,
		"clk_pmuaudioclk");
	if (IS_ERR_OR_NULL(hwcxext_codec->mclk)) {
		hwlog_err("%s: unable to get mclk\n", __func__);
		return PTR_ERR(hwcxext_codec->mclk);
	}

	ret = clk_prepare_enable(hwcxext_codec->mclk);
	if (ret) {
		hwlog_err("%s: unable to enable mclk\n", __func__);
		return ret;
	}

	return 0;
}

static void hwcxext_codec_v1_regist_extern_adc(
	struct snd_soc_component *component)
{
#ifdef CONFIG_HWEVEXT_EXTERN_ADC
	if (of_property_read_bool(component->dev->of_node,
		"need_add_adc_kcontrol")) {
		hwlog_info("%s: add adc kcontrol\n", __func__);
		hwevext_adc_add_kcontrol(component);
	}
#endif
}

static int hwcxext_codec_v1_probe(struct snd_soc_component *component)
{
	int ret;
	struct hwcxext_codec_v1_priv *hwcxext_codec =
		snd_soc_component_get_drvdata(component);

	if (hwcxext_codec == NULL) {
		hwlog_info("%s: hwcxext_codec is invalid\n", __func__);
		return -EINVAL;
	}

	hwcxext_codec->component = component;
	IN_FUNCTION;
	ret = hwcxext_codec_v1_init_pmu_audioclk(component, hwcxext_codec);
	if (ret)
		return ret;

	ret = hwcxext_i2c_ops_regs_seq(hwcxext_codec->regmap_cfg,
			hwcxext_codec->init_regs_seq);
	if (ret) {
		hwlog_err("%s: init regs failed\n", __func__);
		return -ENXIO;
	}

	ret = hwcxext_mbhc_init(&hwcxext_codec->i2c->dev, component,
		&hwcxext_codec->mbhc_data, &mbhc_cb);
	if (ret) {
		hwlog_err("%s: hwcxext mbhc init failed\n", __func__);
		return ret;
	}
	hwcxext_codec_v1_regist_extern_adc(component);

	return 0;
}

static void hwcxext_codec_v1_remove(struct snd_soc_component *component)
{
	struct hwcxext_codec_v1_priv *hwcxext_codec =
		snd_soc_component_get_drvdata(component);

	if (hwcxext_codec == NULL) {
		hwlog_info("%s: hwcxext_codec is invalid\n", __func__);
		return;
	}

	clk_disable_unprepare(hwcxext_codec->mclk);
	hwcxext_mbhc_exit(&hwcxext_codec->i2c->dev, hwcxext_codec->mbhc_data);
}

static const struct snd_soc_component_driver soc_component_hwcxext_codec_v1 = {
	.probe			= hwcxext_codec_v1_probe,
	.remove			= hwcxext_codec_v1_remove,
	.controls		= hwcxext_codec_v1_snd_controls,
	.num_controls		= ARRAY_SIZE(hwcxext_codec_v1_snd_controls),
	.dapm_widgets		= hwcxext_codec_v1_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(hwcxext_codec_v1_dapm_widgets),
	.dapm_routes		= hwcxext_codec_v1_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(hwcxext_codec_v1_dapm_routes),
};

struct hwcxext_codec_info_ctl_ops hwcxext_codec_v1_info_ctl_ops = {
	.dump_regs  = hwcxext_codec_v1_i2c_dump_regs,
};

static int hwcxext_codec_v1_get_hardvison_status(
	struct regmap *regmap)
{
	int ret;
	unsigned int vendor_id;
	unsigned int revison_id;

	ret = regmap_read(regmap,
		HWCXEXT_CODEC_V1_VENDOR_ID, &vendor_id);
	if (ret) {
		hwlog_err("Failed to get vendor_id: %d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		ret = -ENXIO;
		goto hwcxext_codec_v1_get_hardvison_status_err;
	}

	ret = regmap_read(regmap,
		HWCXEXT_CODEC_V1_REVISION_ID, &revison_id);
	if (ret) {
		hwlog_err("Failed to get revison_id: %d\n", __func__, ret);
		hwcxext_i2c_dsm_report(HWCXEXT_I2C_READ, ret);
		ret = -ENXIO;
		goto hwcxext_codec_v1_get_hardvison_status_err;
	}
	hwlog_info("%s: get chip version: %08x,%08x\n",
		__func__, vendor_id, revison_id);
	return 0;

hwcxext_codec_v1_get_hardvison_status_err:
	return ret;
}

static void hwcxext_codec_v1_parse_boost_en_gpio(struct device *dev,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	int ret;
	const char *power_boost_en_gpio_str = "power_boost_en_gpio";

	hwcxext_codec->power_boost_en_gpio = of_get_named_gpio(dev->of_node,
		power_boost_en_gpio_str, 0);
	if (hwcxext_codec->power_boost_en_gpio < 0) {
		hwlog_debug("%s: get_named_gpio failed, %d\n", __func__,
			hwcxext_codec->power_boost_en_gpio);
		ret = of_property_read_u32(dev->of_node, power_boost_en_gpio_str,
			(u32 *)&hwcxext_codec->power_boost_en_gpio);
		if (ret < 0) {
			hwlog_err("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			goto hwcxext_codec_v1_parse_boost_en_gpio_err;
		}
	}

	if (gpio_request((unsigned int)hwcxext_codec->power_boost_en_gpio,
		"power_boost_en_gpio") < 0) {
		hwlog_err("%s: gpio%d request failed\n", __func__,
			hwcxext_codec->power_boost_en_gpio);
		goto hwcxext_codec_v1_parse_boost_en_gpio_err;
	}
	gpio_direction_output(hwcxext_codec->power_boost_en_gpio, 1);
	return;

hwcxext_codec_v1_parse_boost_en_gpio_err:
	hwcxext_codec->power_boost_en_gpio = -EINVAL;
}

static void hwcxext_codec_v1_parse_max_times_hs_recognize(
	struct device *dev,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	unsigned int temp;
	const char *max_times_str = "max_times_hs_recognize";

	if (!of_property_read_u32(dev->of_node,	max_times_str, &temp)) {
		hwcxext_codec->max_times_hs_recognize = temp;
		hwlog_info("%s: max_times_hs_recognize: %d\n", __func__,
			hwcxext_codec->max_times_hs_recognize);
	} else {
		hwcxext_codec->max_times_hs_recognize = MAX_TIMES_HS_RECOGNIZE;
	}
}

static int hwcxext_codec_v1_register_component(struct device *dev)
{
	int ret;

	ret = devm_snd_soc_register_component(dev,
		&soc_component_hwcxext_codec_v1, &hwcxext_codec_v1_dai, 1);
	if (ret) {
		hwlog_err("%s: failed to register codec: %d\n", __func__, ret);
#ifdef CONFIG_HUAWEI_DSM_AUDIO
		audio_dsm_report_info(AUDIO_CODEC,
			DSM_HIFI_AK4376_CODEC_PROBE_ERR,
			"extern codec snd_soc_register_codec fail %d\n", ret);
#endif
		return -EFAULT;
	}

	return 0;
}

static int hwcxext_codec_v1_i2c_probe(struct i2c_client *i2c_client,
	const struct i2c_device_id *id)
{
	struct device *dev = &i2c_client->dev;
	struct hwcxext_codec_v1_priv *hwcxext_codec = NULL;
	int ret;

	IN_FUNCTION;
	hwcxext_codec = devm_kzalloc(dev, sizeof(struct hwcxext_codec_v1_priv),
		GFP_KERNEL);
	if (hwcxext_codec == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c_client, hwcxext_codec);
	hwcxext_codec->i2c = i2c_client;
	dev_set_name(&i2c_client->dev, "%s", "hwcxext_codec");
	hwcxext_codec_v1_parse_boost_en_gpio(dev, hwcxext_codec);
	hwcxext_codec_v1_parse_max_times_hs_recognize(dev, hwcxext_codec);
	ret = hwcxext_i2c_regmap_init(i2c_client, hwcxext_codec);
	if (ret < 0)
		goto hwcxext_codec_v1_i2c_probe_err1;

	ret = hwcxext_codec_v1_get_hardvison_status(
			hwcxext_codec->regmap_cfg->regmap);
	if (ret < 0)
		goto hwcxext_codec_v1_i2c_probe_err1;

	ret = hwcxext_i2c_parse_dt_reg_ctl(i2c_client,
		&hwcxext_codec->init_regs_seq, "init_regs");
	if (ret < 0)
		goto hwcxext_codec_v1_i2c_probe_err2;

	ret = hwcxext_codec_v1_register_component(&i2c_client->dev);
	if (ret < 0)
		goto hwcxext_codec_v1_i2c_probe_err3;

	hwcxext_codec_register_info_ctl_ops(&hwcxext_codec_v1_info_ctl_ops);
	hwcxext_codec_info_store_regmap(hwcxext_codec->regmap_cfg);
	hwcxext_codec_info_set_ctl_support(true);
	return 0;

hwcxext_codec_v1_i2c_probe_err3:
	hwcxext_i2c_free_reg_ctl(&hwcxext_codec->init_regs_seq);
hwcxext_codec_v1_i2c_probe_err2:
	hwcxext_i2c_regmap_deinit(hwcxext_codec->regmap_cfg);
hwcxext_codec_v1_i2c_probe_err1:
	devm_kfree(dev, hwcxext_codec);
	return ret;
}

static int hwcxext_codec_v1_i2c_remove(struct i2c_client *client)
{
	struct hwcxext_codec_v1_priv *hwcxext_codec = i2c_get_clientdata(client);

	if (hwcxext_codec == NULL) {
		hwlog_err("%s: hwcxext_codec invalid\n", __func__);
		return -EINVAL;
	}

	hwcxext_i2c_free_reg_ctl(&hwcxext_codec->init_regs_seq);
	hwcxext_codec_info_set_ctl_support(false);
	snd_soc_unregister_codec(&client->dev);
	hwcxext_i2c_regmap_deinit(hwcxext_codec->regmap_cfg);
	return 0;
}

static const struct i2c_device_id hwcxext_codec_v1_i2c_id[] = {
	{"hwcxext_codec_v1", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, hwcxext_codec_v1_i2c_id);

static const struct of_device_id hwcxext_codec_v1_of_match[] = {
	{ .compatible = "huawei,hwcxext_codec_v1", },
	{},
};
MODULE_DEVICE_TABLE(of, hwcxext_codec_v1_of_match);

static struct i2c_driver hwcxext_codec_v1_i2c_driver = {
	.driver = {
		.name = "hwcxext_codec",
		.owner = THIS_MODULE,
		.of_match_table	= of_match_ptr(hwcxext_codec_v1_of_match),
	},
	.probe		= hwcxext_codec_v1_i2c_probe,
	.remove		= hwcxext_codec_v1_i2c_remove,
	.id_table	= hwcxext_codec_v1_i2c_id,
};

static int __init hwcxext_codec_v1_i2c_init(void)
{
	IN_FUNCTION;
	return i2c_add_driver(&hwcxext_codec_v1_i2c_driver);
}

static void __exit hwcxext_codec_v1_i2c_exit(void)
{
	IN_FUNCTION;
	i2c_del_driver(&hwcxext_codec_v1_i2c_driver);
}

module_init(hwcxext_codec_v1_i2c_init);
module_exit(hwcxext_codec_v1_i2c_exit);

MODULE_DESCRIPTION("hwcxext codec v1 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
