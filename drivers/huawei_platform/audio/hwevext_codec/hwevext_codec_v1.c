/*
 * hwevext_codec_v1.c
 *
 * hwevext codec v1 driver
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/regmap.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
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

#include "hwevext_codec_v1.h"
#include "hwevext_mbhc.h"
#include "hwevext_codec_info.h"
#include "hwevext_i2c_ops.h"

#define HWLOG_TAG hwevext_codec
HWLOG_REGIST();

#define IN_FUNCTION   hwlog_info("%s function comein\n", __func__)
#define OUT_FUNCTION  hwlog_info("%s function comeout\n", __func__)

struct codec_init_regs {
	unsigned int addr;
	unsigned int value;
	unsigned int sleep_us;
};

struct hwevext_codec_v1_priv {
	struct clk *mclk;
	struct regmap *regmap;
	struct snd_soc_component *component;
	struct hwevext_mbhc_priv *mbhc_data;
	struct i2c_client *i2c;
	/* init regs */
	struct hwevext_reg_ctl_sequence *init_regs_seq;
	int hs_en_gpio;
	int hs_en_value;
};

/*
 * hwevext_codec_v1 controls
 */
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(dac_vol_left_tlv, -9600, 50, 1);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(dac_vol_right_tlv, -9600, 50, 1);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(adc_vol_tlv, -9600, 50, 1);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(alc_max_gain_tlv, -650, 150, 0);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(alc_min_gain_tlv, -1200, 150, 0);
static const SNDRV_CTL_TLVD_DECLARE_DB_SCALE(alc_target_tlv, -1650, 150, 0);
static const SNDRV_CTL_TLVD_DECLARE_DB_RANGE(hpmixer_gain_tlv,
	0, 4, TLV_DB_SCALE_ITEM(-1200, 150, 0),
	8, 11, TLV_DB_SCALE_ITEM(-450, 150, 0),
);

static const SNDRV_CTL_TLVD_DECLARE_DB_RANGE(adc_pga_gain_tlv,
	0, 0, TLV_DB_SCALE_ITEM(-350, 0, 0),
	1, 1, TLV_DB_SCALE_ITEM(0, 0, 0),
	2, 2, TLV_DB_SCALE_ITEM(250, 0, 0),
	3, 3, TLV_DB_SCALE_ITEM(450, 0, 0),
	4, 4, TLV_DB_SCALE_ITEM(700, 0, 0),
	5, 5, TLV_DB_SCALE_ITEM(1000, 0, 0),
	6, 6, TLV_DB_SCALE_ITEM(1300, 0, 0),
	7, 7, TLV_DB_SCALE_ITEM(1600, 0, 0),
	8, 8, TLV_DB_SCALE_ITEM(1800, 0, 0),
	9, 9, TLV_DB_SCALE_ITEM(2100, 0, 0),
	10, 10, TLV_DB_SCALE_ITEM(2400, 0, 0),
);

static const SNDRV_CTL_TLVD_DECLARE_DB_RANGE(hpout_vol_tlv,
	0, 0, TLV_DB_SCALE_ITEM(-4800, 0, 0),
	1, 3, TLV_DB_SCALE_ITEM(-2400, 1200, 0),
);

static const char * const ng_type_txt[] = {
	"Constant PGA Gain",
	"Mute ADC Output"
};

static const struct soc_enum ng_type =
	SOC_ENUM_SINGLE(HWEVEXT_CODEC_V1_ADC_ALC_NG, 6, 2, ng_type_txt);

static const char * const adcpol_txt[] = { "Normal", "Invert" };
static const struct soc_enum adcpol =
	SOC_ENUM_SINGLE(HWEVEXT_CODEC_V1_ADC_MUTE, 1, 2, adcpol_txt);
static const char * const dacpol_txt[] = {
	"Normal",
	"R Invert",
	"L Invert",
	"L + R Invert"
};

static const struct soc_enum dacpol =
	SOC_ENUM_SINGLE(HWEVEXT_CODEC_V1_DAC_SET1, 0, 4, dacpol_txt);


static int headset_switch_hs_gpio(struct hwevext_codec_v1_priv *hwevext_codec)
{
	if (hwevext_codec->hs_en_gpio < 0  ||
		!gpio_is_valid(hwevext_codec->hs_en_gpio)) {
		hwlog_err("%s: headset enable gpio:%d is invaild\n",
			__func__, hwevext_codec->hs_en_gpio);
		return -EFAULT;
	}

	hwlog_info("%s: headset enable gpio set %d\n",
		__func__, hwevext_codec->hs_en_value);
	gpio_set_value(hwevext_codec->hs_en_gpio, hwevext_codec->hs_en_value);
	return 0;
}

static const char * const headset_switch_text[] = {"OFF", "ON"};

static const struct soc_enum headset_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(headset_switch_text),
	headset_switch_text),
};

static int headset_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *component = NULL;
	struct hwevext_codec_v1_priv *hwevext_codec = NULL;

	if (ucontrol == NULL || kcontrol == NULL) {
		hwlog_err("%s: input pointer is null\n", __func__);
		return -EINVAL;
	}

	component = snd_soc_kcontrol_component(kcontrol);
	if (component == NULL) {
		hwlog_info("%s: component is invalid\n", __func__);
		return -EINVAL;
	}

	hwevext_codec = snd_soc_component_get_drvdata(component);
	if (hwevext_codec == NULL) {
		hwlog_info("%s: hwevext_codec is invalid\n", __func__);
		return -EINVAL;
	}

	ucontrol->value.integer.value[0] = hwevext_codec->hs_en_value;

	return 0;
}

static int headset_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct snd_soc_component *component = NULL;
	struct hwevext_codec_v1_priv *hwevext_codec = NULL;

	if (ucontrol == NULL || kcontrol == NULL) {
		hwlog_err("%s: input pointer is null\n", __func__);
		return -EINVAL;
	}

	component = snd_soc_kcontrol_component(kcontrol);
	if (component == NULL) {
		hwlog_info("%s: component is invalid\n", __func__);
		return -EINVAL;
	}

	hwevext_codec = snd_soc_component_get_drvdata(component);
	if (hwevext_codec == NULL) {
		hwlog_info("%s: hwevext_codec is invalid\n", __func__);
		return -EINVAL;
	}

	hwevext_codec->hs_en_value = ucontrol->value.integer.value[0];

	ret = headset_switch_hs_gpio(hwevext_codec);
	if (ret != 0)
		hwlog_err("%s: switch headset enable gpio failed\n", __func__);

	return ret;
}

static const struct snd_kcontrol_new hwevext_codec_v1_snd_controls[] = {
	SOC_DOUBLE_TLV("Headphone Playback Volume",
		HWEVEXT_CODEC_V1_CPHP_ICAL_VOL,
		4, 0, 3, 1, hpout_vol_tlv),
	SOC_DOUBLE_TLV("Headphone Mixer Volume", HWEVEXT_CODEC_V1_HPMIX_VOL,
		4, 0, 11, 0, hpmixer_gain_tlv),

	SOC_ENUM("Playback Polarity", dacpol),
	SOC_SINGLE_TLV("DAC Playback left Volume", HWEVEXT_CODEC_V1_DAC_VOLL,
		0, 0xc0, 1, dac_vol_left_tlv),
	SOC_SINGLE_TLV("DAC Playback right Volume", HWEVEXT_CODEC_V1_DAC_VOLR,
		0, 0xc0, 1, dac_vol_right_tlv),
	SOC_SINGLE("DAC Soft Ramp Switch", HWEVEXT_CODEC_V1_DAC_SET1, 4, 1, 1),
	SOC_SINGLE("DAC Soft Ramp Rate", HWEVEXT_CODEC_V1_DAC_SET1, 2, 4, 0),
	SOC_SINGLE("DAC Notch Filter Switch",
		HWEVEXT_CODEC_V1_DAC_SET2, 6, 1, 0),
	SOC_SINGLE("DAC Double Fs Switch", HWEVEXT_CODEC_V1_DAC_SET2, 7, 1, 0),
	SOC_SINGLE("DAC Stereo Enhancement",
		HWEVEXT_CODEC_V1_DAC_SET3, 0, 7, 0),
	SOC_SINGLE("DAC Mono Mix Switch", HWEVEXT_CODEC_V1_DAC_SET3, 3, 1, 0),

	SOC_ENUM("Capture Polarity", adcpol),
	SOC_SINGLE("Mic Boost Switch", HWEVEXT_CODEC_V1_ADC_D2SEPGA, 0, 1, 0),
	SOC_SINGLE_TLV("ADC Capture Volume", HWEVEXT_CODEC_V1_ADC_VOLUME,
		0, 0xc0, 1, adc_vol_tlv),
	SOC_SINGLE_TLV("ADC PGA Gain Volume", HWEVEXT_CODEC_V1_ADC_PGAGAIN,
		4, 10, 0, adc_pga_gain_tlv),
	SOC_SINGLE("ADC Soft Ramp Switch", HWEVEXT_CODEC_V1_ADC_MUTE, 4, 1, 0),
	SOC_SINGLE("ADC Double Fs Switch", HWEVEXT_CODEC_V1_ADC_DMIC, 4, 1, 0),

	SOC_SINGLE("ALC Capture Switch", HWEVEXT_CODEC_V1_ADC_ALC1, 6, 1, 0),
	SOC_SINGLE_TLV("ALC Capture Max Volume",
		HWEVEXT_CODEC_V1_ADC_ALC1, 0, 28, 0, alc_max_gain_tlv),
	SOC_SINGLE_TLV("ALC Capture Min Volume",
		HWEVEXT_CODEC_V1_ADC_ALC2, 0, 28, 0, alc_min_gain_tlv),
	SOC_SINGLE_TLV("ALC Capture Target Volume",
		HWEVEXT_CODEC_V1_ADC_ALC3, 4, 10, 0, alc_target_tlv),
	SOC_SINGLE("ALC Capture Hold Time",
		HWEVEXT_CODEC_V1_ADC_ALC3, 0, 10, 0),
	SOC_SINGLE("ALC Capture Decay Time",
		HWEVEXT_CODEC_V1_ADC_ALC4, 4, 10, 0),
	SOC_SINGLE("ALC Capture Attack Time",
		HWEVEXT_CODEC_V1_ADC_ALC4, 0, 10, 0),
	SOC_SINGLE("ALC Capture Noise Gate Switch", HWEVEXT_CODEC_V1_ADC_ALC_NG,
		5, 1, 0),
	SOC_SINGLE("ALC Capture Noise Gate Threshold",
		HWEVEXT_CODEC_V1_ADC_ALC_NG, 0, 31, 0),
	SOC_ENUM("ALC Capture Noise Gate Type", ng_type),
	SOC_ENUM_EXT("HeadSet Swtich", headset_switch_enum[0],
		headset_switch_get, headset_switch_put),
};

/* Analog Input Mux */
static const char * const hwevext_codec_v1_analog_in_txt[] = {
		"lin1-rin1",
		"lin2-rin2",
		"lin1-rin1 with 20db Boost",
		"lin2-rin2 with 20db Boost"
};
static const unsigned int hwevext_codec_v1_analog_in_values[] = { 0, 1, 2, 3 };
static const struct soc_enum hwevext_codec_v1_analog_input_enum =
	SOC_VALUE_ENUM_SINGLE(HWEVEXT_CODEC_V1_ADC_PDN_LINSEL, 4, 3,
		ARRAY_SIZE(hwevext_codec_v1_analog_in_txt),
		hwevext_codec_v1_analog_in_txt,
		hwevext_codec_v1_analog_in_values);
static const struct snd_kcontrol_new hwevext_codec_v1_analog_in_mux_controls =
	SOC_DAPM_ENUM("Route", hwevext_codec_v1_analog_input_enum);

static const char * const hwevext_codec_v1_dmic_txt[] = {
		"dmic disable",
		"dmic data at high level",
		"dmic data at low level",
};
static const unsigned int hwevext_codec_v1_dmic_values[] = { 0, 1, 2 };
static const struct soc_enum hwevext_codec_v1_dmic_src_enum =
	SOC_VALUE_ENUM_SINGLE(HWEVEXT_CODEC_V1_ADC_DMIC, 0, 3,
		ARRAY_SIZE(hwevext_codec_v1_dmic_txt),
		hwevext_codec_v1_dmic_txt,
		hwevext_codec_v1_dmic_values);
static const struct snd_kcontrol_new hwevext_codec_v1_dmic_src_controls =
	SOC_DAPM_ENUM("Route", hwevext_codec_v1_dmic_src_enum);

/* hp mixer mux */
static const char * const hwevext_codec_v1_hpmux_texts[] = {
	"lin1-rin1",
	"lin2-rin2",
	"lin-rin with Boost",
	"lin-rin with Boost and PGA"
};

static SOC_ENUM_SINGLE_DECL(hwevext_codec_v1_left_hpmux_enum,
	HWEVEXT_CODEC_V1_HPMIX_SEL,
	4, hwevext_codec_v1_hpmux_texts);

static const struct snd_kcontrol_new hwevext_codec_v1_left_hpmux_controls =
	SOC_DAPM_ENUM("Route", hwevext_codec_v1_left_hpmux_enum);

static SOC_ENUM_SINGLE_DECL(hwevext_codec_v1_right_hpmux_enum,
	HWEVEXT_CODEC_V1_HPMIX_SEL,
	0, hwevext_codec_v1_hpmux_texts);

static const struct snd_kcontrol_new hwevext_codec_v1_right_hpmux_controls =
	SOC_DAPM_ENUM("Route", hwevext_codec_v1_right_hpmux_enum);

/* headphone Output Mixer */
static const struct snd_kcontrol_new hwevext_codec_v1_out_left_mix[] = {
	SOC_DAPM_SINGLE("LLIN Switch", HWEVEXT_CODEC_V1_HPMIX_SWITCH, 6, 1, 0),
	SOC_DAPM_SINGLE("Left DAC Switch",
		HWEVEXT_CODEC_V1_HPMIX_SWITCH, 7, 1, 0),
};
static const struct snd_kcontrol_new hwevext_codec_v1_out_right_mix[] = {
	SOC_DAPM_SINGLE("RLIN Switch",
		HWEVEXT_CODEC_V1_HPMIX_SWITCH, 2, 1, 0),
	SOC_DAPM_SINGLE("Right DAC Switch",
		HWEVEXT_CODEC_V1_HPMIX_SWITCH, 3, 1, 0),
};

/* DAC data source mux */
static const char * const hwevext_codec_v1_dacsrc_texts[] = {
	"LDATA TO LDAC, RDATA TO RDAC",
	"LDATA TO LDAC, LDATA TO RDAC",
	"RDATA TO LDAC, RDATA TO RDAC",
	"RDATA TO LDAC, LDATA TO RDAC",
};

static SOC_ENUM_SINGLE_DECL(hwevext_codec_v1_dacsrc_mux_enum,
	HWEVEXT_CODEC_V1_DAC_SET1,
	6, hwevext_codec_v1_dacsrc_texts);

static const struct snd_kcontrol_new hwevext_codec_v1_dacsrc_mux_controls =
	SOC_DAPM_ENUM("Route", hwevext_codec_v1_dacsrc_mux_enum);


static void hwevext_codec_v1_snd_soc_update_bits(
	struct snd_soc_component *component,
	unsigned int reg, unsigned int mask, unsigned int value)
{
	int ret;

	ret = snd_soc_component_update_bits(component, reg, mask, value);
	if (ret < 0)
		hwevext_i2c_dsm_report(HWEVEXT_I2C_WRITE, ret);
}

static int hwevext_codec_v1_adc_cap_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(w->dapm);

	hwlog_info("%s: event:0x%x\n", __func__, event);
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		hwevext_codec_v1_snd_soc_update_bits(component,
			HWEVEXT_CODEC_V1_SERDATA1,
			0x1 << HWEVEXT_CODEC_V1_SERDATA1_TRI_OFFSET, 0);
		hwevext_codec_v1_snd_soc_update_bits(component,
			HWEVEXT_CODEC_V1_SERDATA_ADC,
			0x1 << HWEVEXT_CODEC_V1_SERDATA_ADC_SDP_MUTE_OFFSET, 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		hwevext_codec_v1_snd_soc_update_bits(component,
			HWEVEXT_CODEC_V1_SERDATA_ADC,
			0x1 << HWEVEXT_CODEC_V1_SERDATA_ADC_SDP_MUTE_OFFSET,
			0x1 << HWEVEXT_CODEC_V1_SERDATA_ADC_SDP_MUTE_OFFSET);
		hwevext_codec_v1_snd_soc_update_bits(component,
			HWEVEXT_CODEC_V1_SERDATA1,
			0x1 << HWEVEXT_CODEC_V1_SERDATA1_TRI_OFFSET,
			0x1 << HWEVEXT_CODEC_V1_SERDATA1_TRI_OFFSET);
		break;
	default:
		hwlog_err("power event err: %d", event);
		break;
	}

	return 0;
}

static const struct snd_kcontrol_new adc_cap_switch[] = {
	SOC_DAPM_SINGLE("switch", SND_SOC_NOPM, 0, 1, 0)
};

static const struct snd_soc_dapm_widget hwevext_codec_v1_dapm_widgets[] = {
	SND_SOC_DAPM_SUPPLY("Bias", HWEVEXT_CODEC_V1_SYS_PDN, 3, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Analog power",
		HWEVEXT_CODEC_V1_SYS_PDN, 4, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Mic Bias", HWEVEXT_CODEC_V1_SYS_PDN, 5, 1, NULL, 0),

	SND_SOC_DAPM_INPUT("DMIC"),
	SND_SOC_DAPM_INPUT("MIC1"),
	SND_SOC_DAPM_INPUT("MIC2"),

	/* Input Mux */
	SND_SOC_DAPM_MUX("Differential Mux", SND_SOC_NOPM, 0, 0,
		&hwevext_codec_v1_analog_in_mux_controls),

	SND_SOC_DAPM_SUPPLY("ADC Vref", HWEVEXT_CODEC_V1_SYS_PDN, 1, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("ADC bias", HWEVEXT_CODEC_V1_SYS_PDN, 2, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("ADC Clock",
		HWEVEXT_CODEC_V1_CLKMGR_CLKSW, 3, 0, NULL, 0),
	SND_SOC_DAPM_PGA("Line input PGA", HWEVEXT_CODEC_V1_ADC_PDN_LINSEL,
		7, 1, NULL, 0),
	SND_SOC_DAPM_ADC("Mono ADC", NULL,
		HWEVEXT_CODEC_V1_ADC_PDN_LINSEL, 6, 1),
	SND_SOC_DAPM_MUX("Digital Mic Mux", SND_SOC_NOPM, 0, 0,
		&hwevext_codec_v1_dmic_src_controls),

	/* Digital Interface */
	SND_SOC_DAPM_OUTPUT("I2S OUT"),
	SND_SOC_DAPM_INPUT("I2S IN"),

	SND_SOC_DAPM_MUX("DAC Source Mux", SND_SOC_NOPM, 0, 0,
			 &hwevext_codec_v1_dacsrc_mux_controls),

	SND_SOC_DAPM_SUPPLY("DAC Vref", HWEVEXT_CODEC_V1_SYS_PDN, 0, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("DAC Clock",
		HWEVEXT_CODEC_V1_CLKMGR_CLKSW, 2, 0, NULL, 0),
	SND_SOC_DAPM_DAC("Right DAC", NULL, HWEVEXT_CODEC_V1_DAC_PDN, 0, 1),
	SND_SOC_DAPM_DAC("Left DAC", NULL, HWEVEXT_CODEC_V1_DAC_PDN, 4, 1),

	/* Headphone Output Side */
	SND_SOC_DAPM_MUX("Left Headphone Mux", SND_SOC_NOPM, 0, 0,
		&hwevext_codec_v1_left_hpmux_controls),
	SND_SOC_DAPM_MUX("Right Headphone Mux", SND_SOC_NOPM, 0, 0,
		&hwevext_codec_v1_right_hpmux_controls),
	SND_SOC_DAPM_MIXER("Left Headphone Mixer", HWEVEXT_CODEC_V1_HPMIX_PDN,
		5, 1, &hwevext_codec_v1_out_left_mix[0],
		ARRAY_SIZE(hwevext_codec_v1_out_left_mix)),
	SND_SOC_DAPM_MIXER("Right Headphone Mixer", HWEVEXT_CODEC_V1_HPMIX_PDN,
		1, 1, &hwevext_codec_v1_out_right_mix[0],
		ARRAY_SIZE(hwevext_codec_v1_out_right_mix)),
	SND_SOC_DAPM_PGA("Left Headphone Mixer Out", HWEVEXT_CODEC_V1_HPMIX_PDN,
		4, 1, NULL, 0),
	SND_SOC_DAPM_PGA("Right Headphone Mixer Out", HWEVEXT_CODEC_V1_HPMIX_PDN,
		0, 1, NULL, 0),

	SND_SOC_DAPM_OUT_DRV("Left Headphone Charge Pump",
		HWEVEXT_CODEC_V1_CPHP_OUTEN,
		6, 0, NULL, 0),
	SND_SOC_DAPM_OUT_DRV("Right Headphone Charge Pump",
		HWEVEXT_CODEC_V1_CPHP_OUTEN,
		2, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Headphone Charge Pump", HWEVEXT_CODEC_V1_CPHP_PDN2,
		5, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Headphone Charge Pump Clock",
		HWEVEXT_CODEC_V1_CLKMGR_CLKSW, 4, 0, NULL, 0),

	SND_SOC_DAPM_OUT_DRV("Left Headphone Driver",
		HWEVEXT_CODEC_V1_CPHP_OUTEN, 5, 0, NULL, 0),
	SND_SOC_DAPM_OUT_DRV("Right Headphone Driver",
		HWEVEXT_CODEC_V1_CPHP_OUTEN, 1, 0, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Headphone Out",
		HWEVEXT_CODEC_V1_CPHP_PDN1, 2, 1, NULL, 0),

	/* pdn_Lical and pdn_Rical bits are documented as Reserved, but must
	 * be explicitly unset in order to enable HP output
	 */
	SND_SOC_DAPM_SUPPLY("Left Headphone ical", HWEVEXT_CODEC_V1_CPHP_ICAL_VOL,
		7, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("Right Headphone ical", HWEVEXT_CODEC_V1_CPHP_ICAL_VOL,
		3, 1, NULL, 0),

	SND_SOC_DAPM_MIXER_E("ADC_CAP", SND_SOC_NOPM, 0, 0,
		adc_cap_switch, ARRAY_SIZE(adc_cap_switch),
		hwevext_codec_v1_adc_cap_mixer_event,
		SND_SOC_DAPM_PRE_PMD | SND_SOC_DAPM_POST_PMU),

	SND_SOC_DAPM_OUTPUT("HPOL"),
	SND_SOC_DAPM_OUTPUT("HPOR"),
};

static const struct snd_soc_dapm_route hwevext_codec_v1_dapm_routes[] = {
	/* Recording */
	{"MIC1", NULL, "Mic Bias"},
	{"MIC2", NULL, "Mic Bias"},
	{"MIC1", NULL, "Bias"},
	{"MIC2", NULL, "Bias"},
	{"MIC1", NULL, "Analog power"},
	{"MIC2", NULL, "Analog power"},

	{"MIC1", NULL, "ADC Clock"},
	{"MIC2", NULL, "ADC Clock"},
	{"MIC1", NULL, "ADC Vref"},
	{"MIC2", NULL, "ADC Vref"},
	{"MIC1", NULL, "ADC bias"},
	{"MIC2", NULL, "ADC bias"},

	{"Differential Mux", "lin1-rin1", "MIC1"},
	{"Differential Mux", "lin2-rin2", "MIC2"},
	{"Line input PGA", NULL, "Differential Mux"},

	{"Mono ADC", NULL, "Line input PGA"},

	/* It's not clear why, but to avoid recording only silence,
	 * the DAC clock must be running for the ADC to work.
	 */
	{"Mono ADC", NULL, "DAC Clock"},

	{"Digital Mic Mux", "dmic disable", "Mono ADC"},

	{"ADC_CAP", "switch", "Digital Mic Mux"},

	{"I2S OUT", NULL, "ADC_CAP"},

	/* Playback */
	{"DAC Source Mux", "LDATA TO LDAC, RDATA TO RDAC", "I2S IN"},
	{"DAC Source Mux", "LDATA TO LDAC, LDATA TO RDAC", "I2S IN"},
	{"DAC Source Mux", "RDATA TO LDAC, RDATA TO RDAC", "I2S IN"},
	{"DAC Source Mux", "RDATA TO LDAC, LDATA TO RDAC", "I2S IN"},

	{"Left DAC", NULL, "DAC Clock"},
	{"Right DAC", NULL, "DAC Clock"},

	{"Left DAC", NULL, "DAC Vref"},
	{"Right DAC", NULL, "DAC Vref"},

	{"Left DAC", NULL, "DAC Source Mux"},
	{"Right DAC", NULL, "DAC Source Mux"},

	{"Left Headphone Mux", "lin-rin with Boost and PGA", "Line input PGA"},
	{"Right Headphone Mux", "lin-rin with Boost and PGA", "Line input PGA"},

	{"Left Headphone Mixer", "LLIN Switch", "Left Headphone Mux"},
	{"Left Headphone Mixer", "Left DAC Switch", "Left DAC"},

	{"Right Headphone Mixer", "RLIN Switch", "Right Headphone Mux"},
	{"Right Headphone Mixer", "Right DAC Switch", "Right DAC"},

	{"Left Headphone Mixer Out", NULL, "Left Headphone Mixer"},
	{"Right Headphone Mixer Out", NULL, "Right Headphone Mixer"},

	{"Left Headphone Charge Pump", NULL, "Left Headphone Mixer Out"},
	{"Right Headphone Charge Pump", NULL, "Right Headphone Mixer Out"},

	{"Left Headphone Charge Pump", NULL, "Headphone Charge Pump"},
	{"Right Headphone Charge Pump", NULL, "Headphone Charge Pump"},

	{"Left Headphone Charge Pump", NULL, "Headphone Charge Pump Clock"},
	{"Right Headphone Charge Pump", NULL, "Headphone Charge Pump Clock"},

	{"Left Headphone Driver", NULL, "Left Headphone Charge Pump"},
	{"Right Headphone Driver", NULL, "Right Headphone Charge Pump"},

	{"HPOL", NULL, "Left Headphone Driver"},
	{"HPOR", NULL, "Right Headphone Driver"},

	{"HPOL", NULL, "Left Headphone ical"},
	{"HPOR", NULL, "Right Headphone ical"},

	{"Headphone Out", NULL, "Bias"},
	{"Headphone Out", NULL, "Analog power"},
	{"HPOL", NULL, "Headphone Out"},
	{"HPOR", NULL, "Headphone Out"},
};

static int hwevext_codec_v1_set_dai_sysclk(struct snd_soc_dai *codec_dai,
	int clk_id, unsigned int freq, int dir)
{
	return 0;
}

static int hwevext_codec_v1_set_dai_fmt(struct snd_soc_dai *codec_dai,
	unsigned int fmt)
{
	struct snd_soc_component *component = codec_dai->component;
	u8 serdata1 = 0;
	u8 serdata2 = 0;
	u8 clksw;
	u8 mask;

	hwlog_info("%s : fmt:0x%x\n", __func__, fmt);
	if ((fmt & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS) {
		hwlog_err("%s: Codec driver only supports slave mode\n",
			__func__);
		return -EINVAL;
	}

	if ((fmt & SND_SOC_DAIFMT_FORMAT_MASK) != SND_SOC_DAIFMT_I2S) {
		hwlog_err("%s: Codec driver only supports I2S format\n",
			__func__);
		return -EINVAL;
	}

	/* Clock inversion */
	switch (fmt & SND_SOC_DAIFMT_INV_MASK) {
	case SND_SOC_DAIFMT_NB_NF:
		break;
	case SND_SOC_DAIFMT_IB_IF:
		serdata1 |= HWEVEXT_CODEC_V1_SERDATA1_BCLK_INV;
		serdata2 |= HWEVEXT_CODEC_V1_SERDATA2_ADCLRP;
		break;
	case SND_SOC_DAIFMT_IB_NF:
		serdata1 |= HWEVEXT_CODEC_V1_SERDATA1_BCLK_INV;
		break;
	case SND_SOC_DAIFMT_NB_IF:
		serdata2 |= HWEVEXT_CODEC_V1_SERDATA2_ADCLRP;
		break;
	default:
		return -EINVAL;
	}

	mask = HWEVEXT_CODEC_V1_SERDATA1_MASTER |
		HWEVEXT_CODEC_V1_SERDATA1_BCLK_INV;
	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_SERDATA1, mask, serdata1);

	mask = HWEVEXT_CODEC_V1_SERDATA2_FMT_MASK |
		HWEVEXT_CODEC_V1_SERDATA2_ADCLRP;
	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_SERDATA_ADC, mask, serdata2);
	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_SERDATA_DAC, mask, serdata2);

	/* Enable BCLK and MCLK inputs in slave mode */
	clksw = HWEVEXT_CODEC_V1_CLKMGR_CLKSW_MCLK_ON |
		HWEVEXT_CODEC_V1_CLKMGR_CLKSW_BCLK_ON;
	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_CLKMGR_CLKSW, clksw, clksw);

	return 0;
}

static int hwevext_codec_v1_pcm_startup(struct snd_pcm_substream *substream,
	struct snd_soc_dai *dai)
{
	return 0;
}

static int hwevext_codec_v1_pcm_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	u8 wordlen = 0;

	IN_FUNCTION;
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		wordlen = HWEVEXT_CODEC_V1_SERDATA2_LEN_16;
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		wordlen = HWEVEXT_CODEC_V1_SERDATA2_LEN_20;
		break;
	case SNDRV_PCM_FORMAT_S24_LE:
		wordlen = HWEVEXT_CODEC_V1_SERDATA2_LEN_24;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		wordlen = HWEVEXT_CODEC_V1_SERDATA2_LEN_32;
		break;
	default:
		return -EINVAL;
	}

	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_SERDATA_DAC,
		HWEVEXT_CODEC_V1_SERDATA2_LEN_MASK, wordlen);
	hwevext_codec_v1_snd_soc_update_bits(component,
		HWEVEXT_CODEC_V1_SERDATA_ADC,
		HWEVEXT_CODEC_V1_SERDATA2_LEN_MASK, wordlen);
	return 0;
}

static int hwevext_codec_v1_mute(struct snd_soc_dai *dai, int mute)
{
	hwlog_info("%s : mute:%d\n", __func__, mute);

	hwevext_codec_v1_snd_soc_update_bits(dai->component, 
		HWEVEXT_CODEC_V1_DAC_SET1, 0x20,
		mute ? 0x20 : 0);
	return 0;
}

#define HWEVEXT_CODEC_V1_FORMATS (SNDRV_PCM_FMTBIT_S16_LE | \
		SNDRV_PCM_FMTBIT_S20_3LE | \
		SNDRV_PCM_FMTBIT_S24_LE)

static const struct snd_soc_dai_ops hwevext_codec_v1_ops = {
	.startup = hwevext_codec_v1_pcm_startup,
	.hw_params = hwevext_codec_v1_pcm_hw_params,
	.set_fmt = hwevext_codec_v1_set_dai_fmt,
	.set_sysclk = hwevext_codec_v1_set_dai_sysclk,
	.digital_mute = hwevext_codec_v1_mute,
};

static struct snd_soc_dai_driver hwevext_codec_v1_dai = {
	.name = "HWEVEXT HiFi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = HWEVEXT_CODEC_V1_FORMATS,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 1,
		.channels_max = 2,
		.rates = SNDRV_PCM_RATE_8000_48000,
		.formats = HWEVEXT_CODEC_V1_FORMATS,
	},
	.ops = &hwevext_codec_v1_ops,
	.symmetric_rates = 1,
};

static void hwevext_codec_v1_enable_micbias_for_hs_detect(
	struct hwevext_mbhc_priv *mbhc)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return;
	}

	dapm = snd_soc_component_get_dapm(mbhc->component);
	if (IS_ERR_OR_NULL(dapm)) {
		hwlog_err("%s: dapm is NULL\n", __func__);
		return ;
	}

	IN_FUNCTION;

	snd_soc_dapm_mutex_lock(dapm);
	snd_soc_dapm_force_enable_pin_unlocked(dapm, "Bias");
	snd_soc_dapm_force_enable_pin_unlocked(dapm, "Analog power");
	snd_soc_dapm_force_enable_pin_unlocked(dapm, "Mic Bias");
	snd_soc_dapm_sync_unlocked(dapm);
	snd_soc_dapm_mutex_unlock(dapm);

	msleep(20);
}

static void hwevext_codec_v1_disable_micbias_for_hs_detect(
	struct hwevext_mbhc_priv *mbhc)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return;
	}

	dapm = snd_soc_component_get_dapm(mbhc->component);
	if (IS_ERR_OR_NULL(dapm)) {
		hwlog_err("%s: dapm is NULL\n", __func__);
		return ;
	}

	IN_FUNCTION;
	snd_soc_dapm_mutex_lock(dapm);
	snd_soc_dapm_disable_pin_unlocked(dapm, "Mic Bias");
	snd_soc_dapm_disable_pin_unlocked(dapm, "Analog power");
	snd_soc_dapm_disable_pin_unlocked(dapm, "Bias");
	snd_soc_dapm_sync_unlocked(dapm);
	snd_soc_dapm_mutex_unlock(dapm);
}

static bool hwevext_codec_v1_mbhc_check_headset_in(
	struct hwevext_mbhc_priv *mbhc)
{
	int ret;
	unsigned int flags;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return false;
	}

	ret = snd_soc_component_read(mbhc->component,
		HWEVEXT_CODEC_V1_GPIO_FLAG, &flags);
	if (ret) {
		hwlog_err("%s: read reg err:%d\n", __func__, ret);
		return false;
	}

	if (!(flags & HWEVEXT_CODEC_V1_GPIO_FLAG_HP_INSERTED)) {
		hwlog_info("%s:headset is %s, gpio flags(0x%x): %#04x\n",
			__func__, "plugin",
			HWEVEXT_CODEC_V1_GPIO_FLAG, flags);
		return true;
	}

	hwlog_info("%s:headset is %s, gpio flags(0x%x): %#04x\n",
		__func__, "plugout",
		HWEVEXT_CODEC_V1_GPIO_FLAG, flags);

	return false;
}

static unsigned int hwevext_codec_v1_get_hs_status_reg_value(
	struct hwevext_mbhc_priv *mbhc)
{
	unsigned int flags;

	if (IS_ERR_OR_NULL(mbhc)) {
		hwlog_err("%s: mbhc is NULL\n", __func__);
		return 0;
	}

	snd_soc_component_read(mbhc->component,
		HWEVEXT_CODEC_V1_GPIO_FLAG, &flags);
	hwlog_info("%s: gpio flags %#04x\n", __func__, flags);

	return flags;
}

static struct hwevext_mbhc_cb mbhc_cb = {
	.mbhc_check_headset_in = hwevext_codec_v1_mbhc_check_headset_in,
	.enable_micbias = hwevext_codec_v1_enable_micbias_for_hs_detect,
	.disable_micbias = hwevext_codec_v1_disable_micbias_for_hs_detect,
	.get_hs_status_reg_value = hwevext_codec_v1_get_hs_status_reg_value,
};

static int hwevext_codec_v1_parse_heaset_swtich(
	struct hwevext_codec_v1_priv *hwevext_codec)
{
	int ret;
	const char *gpio_en_str = "headset_switch_en";
	struct snd_soc_component *component = hwevext_codec->component;

	hwevext_codec->hs_en_gpio =
		of_get_named_gpio(component->dev->of_node, gpio_en_str, 0);
	if (hwevext_codec->hs_en_gpio < 0) {
		hwlog_debug("%s: get_named_gpio:%s failed, %d\n", __func__,
			gpio_en_str, hwevext_codec->hs_en_gpio);
		ret = of_property_read_u32(component->dev->of_node, gpio_en_str,
			(u32 *)&(hwevext_codec->hs_en_gpio));
		if (ret < 0) {
			hwlog_info("%s: of_property_read_u32 gpio failed, %d\n",
				__func__, ret);
			hwevext_codec->hs_en_gpio = -EINVAL;
			return 0;
		}
	}

	if (!gpio_is_valid(hwevext_codec->hs_en_gpio)) {
		hwlog_err("%s: hs_en_gpio gpio %d invalid\n", __func__,
		hwevext_codec->hs_en_gpio);
		return 0;
	}

	ret = gpio_request(hwevext_codec->hs_en_gpio, "hwevext_hs_en");
	if (ret < 0) {
		hwevext_codec->hs_en_gpio = -EINVAL;
		hwlog_err("%s: gpio_request ret %d invalid\n", __func__, ret);
		return -EFAULT;
	}

	hwevext_codec->hs_en_value = 0;
	if (gpio_direction_output(hwevext_codec->hs_en_gpio, 0) != 0) {
		gpio_free(hwevext_codec->hs_en_gpio);
		hwevext_codec->hs_en_gpio = -EINVAL;
		hwlog_err("%s: headset enable gpio set output failed\n",
			__func__);
		return -EFAULT;
	}
	return 0;
}

static int hwevext_codec_v1_init_pmu_audioclk(
	struct snd_soc_component *component,
	struct hwevext_codec_v1_priv *hwevext_codec)
{
	bool need_pmu_audioclk = false;
	int ret;

	need_pmu_audioclk = of_property_read_bool(component->dev->of_node,
		"clk_pmuaudioclk");
	if (!need_pmu_audioclk) {
		hwlog_info("%s: it no need pmu audioclk\n", __func__);
		return 0;
	}

	hwevext_codec->mclk = devm_clk_get(&hwevext_codec->i2c->dev,
		"clk_pmuaudioclk");
	if (IS_ERR_OR_NULL(hwevext_codec->mclk)) {
		hwlog_err("%s: unable to get mclk\n", __func__);
		return PTR_ERR(hwevext_codec->mclk);
	}

	ret = clk_prepare_enable(hwevext_codec->mclk);
	if (ret) {
		hwlog_err("%s: unable to enable mclk\n", __func__);
		return ret;
	}

	return 0;
}

static void hwevext_codec_v1_regist_extern_adc(
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

static int hwevext_codec_v1_probe(struct snd_soc_component *component)
{
	int ret;
	unsigned int flags;
	struct hwevext_codec_v1_priv *hwevext_codec =
		snd_soc_component_get_drvdata(component);

	if (hwevext_codec == NULL) {
		hwlog_info("%s: hwevext_codec is invalid\n", __func__);
		return -EINVAL;
	}

	hwevext_codec->component = component;
	IN_FUNCTION;
	ret = hwevext_codec_v1_init_pmu_audioclk(component, hwevext_codec);
	if (ret)
		return ret;

	ret = hwevext_codec_v1_parse_heaset_swtich(hwevext_codec);
	if (ret) {
		hwlog_err("%s: parse heaset swtich failed:%d\n", __func__, ret);
		return ret;
	}

	ret = hwevext_i2c_ops_regs_seq(hwevext_codec->init_regs_seq);
	if (ret) {
		hwlog_err("%s: init regs failed\n", __func__);
		return -ENXIO;
	}

	ret = hwevext_mbhc_init(&hwevext_codec->i2c->dev, component,
		&hwevext_codec->mbhc_data, &mbhc_cb);
	if (ret) {
		hwlog_err("%s: hwevext mbhc init failed\n", __func__);
		return ret;
	}
	hwevext_codec_v1_regist_extern_adc(component);
	snd_soc_component_read(component, HWEVEXT_CODEC_V1_GPIO_FLAG, &flags);
	hwlog_info("%s: reg:0x%x = %#04x\n", __func__,
		HWEVEXT_CODEC_V1_GPIO_FLAG, flags);

	return 0;
}

static void hwevext_codec_v1_remove(struct snd_soc_component *component)
{
	struct hwevext_codec_v1_priv *hwevext_codec =
		snd_soc_component_get_drvdata(component);

	if (hwevext_codec == NULL) {
		hwlog_info("%s: hwevext_codec is invalid\n", __func__);
		return;
	}

	clk_disable_unprepare(hwevext_codec->mclk);
	hwevext_mbhc_exit(&hwevext_codec->i2c->dev, hwevext_codec->mbhc_data);
}

static const struct snd_soc_component_driver soc_component_hwevext_codec_v1 = {
	.probe			= hwevext_codec_v1_probe,
	.remove			= hwevext_codec_v1_remove,
	.controls		= hwevext_codec_v1_snd_controls,
	.num_controls		= ARRAY_SIZE(hwevext_codec_v1_snd_controls),
	.dapm_widgets		= hwevext_codec_v1_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(hwevext_codec_v1_dapm_widgets),
	.dapm_routes		= hwevext_codec_v1_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(hwevext_codec_v1_dapm_routes),
};

static bool hwevext_codec_v1_volatile_register(struct device *dev,
	unsigned int reg)
{
	switch (reg) {
	case HWEVEXT_CODEC_V1_GPIO_FLAG:
		return 1;
	default:
		return 0;
	}
}

static const struct regmap_config hwevext_codec_v1_regmap = {
	.reg_bits = 8,
	.val_bits = 8,
	.max_register = 0x53,
	.volatile_reg = &hwevext_codec_v1_volatile_register,
	.cache_type = REGCACHE_NONE,
};

static int hwevext_codec_v1_i2c_dump_regs(struct regmap *regmap)
{
	unsigned int regs;
	unsigned int value;

	if (regmap == NULL) {
		hwlog_err("%s: regmap is NULL\n", __func__);
		return -EINVAL;
	}

	for (regs = HWEVEXT_CODEC_V1_RESET;
		regs <= HWEVEXT_CODEC_V1_TEST3; regs++) {
		regmap_read(regmap, regs, &value);
		hwlog_info("%s: read reg 0x%x=0x%x\n", __func__, regs, value);
	}

	return 0;
}

struct hwevext_codec_info_ctl_ops info_ctl_ops = {
	.dump_regs  = hwevext_codec_v1_i2c_dump_regs,
};

static int hwevext_codec_v1_i2c_probe(struct i2c_client *i2c_client,
	const struct i2c_device_id *id)
{
	struct device *dev = &i2c_client->dev;
	struct hwevext_codec_v1_priv *hwevext_codec = NULL;
	int ret;
	unsigned int status;

	IN_FUNCTION;
	hwevext_codec = devm_kzalloc(dev, sizeof(struct hwevext_codec_v1_priv),
		GFP_KERNEL);
	if (hwevext_codec == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c_client, hwevext_codec);
	hwevext_codec->i2c = i2c_client;
	dev_set_name(&i2c_client->dev, "%s", "hwevext_codec");

	hwevext_codec->regmap = regmap_init_i2c(i2c_client,
		&hwevext_codec_v1_regmap);
	if (IS_ERR_OR_NULL(hwevext_codec->regmap)) {
		hwlog_err("Failed to init regmap: %d\n", __func__, ret);
		ret = -ENOMEM;
		goto hwevext_codec_v1_i2c_probe_err1;
	}

	ret = regmap_read(hwevext_codec->regmap,
		HWEVEXT_CODEC_V1_RESET, &status);
	if (ret) {
		hwlog_err("Failed to get chip status: %d\n", __func__, ret);
		hwevext_i2c_dsm_report(HWEVEXT_I2C_READ, ret);
		ret = -ENXIO;
		goto hwevext_codec_v1_i2c_probe_err2;
	}
	hwlog_info("%s: get chip status: 0x%x\n", __func__, status);

	ret = hwevext_i2c_parse_dt_reg_ctl(i2c_client,
		&hwevext_codec->init_regs_seq, "init_regs");
	if (ret < 0)
		goto hwevext_codec_v1_i2c_probe_err2;

	ret = devm_snd_soc_register_component(&i2c_client->dev,
		&soc_component_hwevext_codec_v1,
			&hwevext_codec_v1_dai, 1);
	if (ret < 0) {
		hwlog_err("%s: failed to register codec: %d\n", __func__, ret);
#ifdef CONFIG_HUAWEI_DSM_AUDIO
		audio_dsm_report_info(AUDIO_CODEC,
			DSM_HIFI_AK4376_CODEC_PROBE_ERR,
			"extern codec snd_soc_register_codec fail %d\n", ret);
#endif
		goto hwevext_codec_v1_i2c_probe_err3;
	}
	hwevext_i2c_set_reg_value_mask(hwevext_codec_v1_regmap.val_bits);
	hwevext_i2c_ops_store_regmap(hwevext_codec->regmap);
	hwevext_codec_register_info_ctl_ops(&info_ctl_ops);
	hwevext_codec_info_store_regmap(hwevext_codec->regmap);
	hwevext_codec_info_set_ctl_support(true);
	return 0;

hwevext_codec_v1_i2c_probe_err3:
	hwevext_i2c_free_reg_ctl(hwevext_codec->init_regs_seq);
hwevext_codec_v1_i2c_probe_err2:
	regmap_exit(hwevext_codec->regmap);
hwevext_codec_v1_i2c_probe_err1:
	devm_kfree(dev, hwevext_codec);
	return ret;
}

static int hwevext_codec_v1_i2c_remove(struct i2c_client *client)
{
	struct hwevext_codec_v1_priv *hwevext_codec = i2c_get_clientdata(client);
	if (hwevext_codec == NULL) {
		hwlog_err("%s: hwevext_codec invalid\n", __func__);
		return -EINVAL;
	}
	hwevext_i2c_free_reg_ctl(hwevext_codec->init_regs_seq);
	hwevext_codec_info_set_ctl_support(false);
	snd_soc_unregister_codec(&client->dev);
	if (hwevext_codec->regmap)
		regmap_exit(hwevext_codec->regmap);
	return 0;
}

static const struct i2c_device_id hwevext_codec_v1_i2c_id[] = {
	{"hwevext_codec_v1", 0 },
	{}
};
MODULE_DEVICE_TABLE(i2c, hwevext_codec_v1_i2c_id);

static const struct of_device_id hwevext_codec_v1_of_match[] = {
	{ .compatible = "huawei,hwevext_codec_v1", },
	{},
};
MODULE_DEVICE_TABLE(of, hwevext_codec_v1_of_match);

static struct i2c_driver hwevext_codec_v1_i2c_driver = {
	.driver = {
		.name = "hwevext_codec",
		.owner = THIS_MODULE,
		.of_match_table	= of_match_ptr(hwevext_codec_v1_of_match),
	},
	.probe		= hwevext_codec_v1_i2c_probe,
	.remove		= hwevext_codec_v1_i2c_remove,
	.id_table	= hwevext_codec_v1_i2c_id,
};

static int __init hwevext_codec_v1_i2c_init(void)
{
	IN_FUNCTION;
	return i2c_add_driver(&hwevext_codec_v1_i2c_driver);
}

static void __exit hwevext_codec_v1_i2c_exit(void)
{
	IN_FUNCTION;
	i2c_del_driver(&hwevext_codec_v1_i2c_driver);
}

module_init(hwevext_codec_v1_i2c_init);
module_exit(hwevext_codec_v1_i2c_exit);

MODULE_DESCRIPTION("hwevext codec v1 driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
MODULE_LICENSE("GPL v2");
