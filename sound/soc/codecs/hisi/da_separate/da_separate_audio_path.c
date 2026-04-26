/*
 * da_separate_audio_path.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "da_separate_audio_path.h"
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/version.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>

#include "asoc_adapter.h"
#include "linux/hisi/audio_log.h"
#include "da_separate_utils.h"
#include "da_separate_type.h"
#include "da_separate_mbhc.h"

#define LOG_TAG "DA_separate_audio"

static DECLARE_TLV_DB_SCALE(analog_mainpga_gain_tlv, 0, PGA_GAIN_STEP, 0);
static DECLARE_TLV_DB_SCALE(analog_auxpga_gain_tlv, 0, PGA_GAIN_STEP, 0);
static DECLARE_TLV_DB_SCALE(analog_mic3pga_gain_tlv, 0, PGA_GAIN_STEP, 0);

static int hac_switch;
static const char * const hac_switch_text[] = { "OFF", "ON" };
static const struct soc_enum hac_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hac_switch_text), hac_switch_text),
};

static int hac_switch_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EFAULT;
	}

	ucontrol->value.integer.value[0] = hac_switch;

	return 0;
}

static int hac_gpio_output_set(int hac_cmd, unsigned int hac_gpio)
{
	int ret;

	if (!gpio_is_valid(hac_gpio)) {
		AUDIO_LOGE("Failed to get the hac gpio");
		return -EFAULT;
	}

	if (hac_cmd == HAC_ENABLE) {
		AUDIO_LOGI("Enable hac gpio %u", hac_gpio);
		ret = gpio_direction_output(hac_gpio, GPIO_LEVEL_HIGH);
	} else {
		AUDIO_LOGI("Disable hac gpio %u", hac_gpio);
		ret = gpio_direction_output(hac_gpio, GPIO_LEVEL_LOW);
	}

	return ret;
}

static int hac_switch_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	struct snd_soc_component *codec = NULL;
	struct platform_data *priv = NULL;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input pointer is null");
		return -EFAULT;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	priv = snd_soc_component_get_drvdata(codec);

	hac_switch = ucontrol->value.integer.value[0];
	ret = hac_gpio_output_set(hac_switch, priv->hac_en_gpio);

	return ret;
}

static const char * const au_pa_pga_cfg_text[] = { "OFF", "ON" };
static const struct soc_enum au_pa_pga_cfg_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(au_pa_pga_cfg_text), au_pa_pga_cfg_text),
};

static int au_pa_pga_cfg_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input para is NULL");
		return -EFAULT;
	}
	return 0;
}

static int au_pa_pga_cfg_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_component *codec = NULL;
	int state;

	if (kcontrol == NULL || ucontrol == NULL) {
		AUDIO_LOGE("input para is NULL");
		return -EFAULT;
	}

	codec = snd_soc_kcontrol_component(kcontrol);
	state = ucontrol->value.integer.value[0];

	if (state > 0) {
		snd_soc_component_update_bits(codec, VOICE_R_DN_PGA_CTRL_REG,
			MASK_ON_BIT(VOICE_R_DN_PGA_CFG_LEN, VOICE_R_DN_PGA_CFG_OFFSET),
			MASK_ON_BIT(VOICE_R_DN_PGA_CFG_LEN, VOICE_R_DN_PGA_CFG_OFFSET));
		snd_soc_component_update_bits(codec, VOICE_R_DN_PGA_CTRL_REG,
			MASK_ON_BIT(VOICE_R_DN_PGA_FADE_IN_LEN, VOICE_R_DN_PGA_FADE_IN_OFFSET),
			0xc << VOICE_R_DN_PGA_FADE_IN_OFFSET);
		snd_soc_component_update_bits(codec, VOICE_R_DN_PGA_CTRL_REG,
			MASK_ON_BIT(VOICE_R_DN_PGA_FADE_OUT_LEN, VOICE_R_DN_PGA_FADE_OUT_OFFSET),
			0xa << VOICE_R_DN_PGA_FADE_OUT_OFFSET);
		snd_soc_component_update_bits(codec, VOICE_R_DN_PGA_CTRL_REG,
			BIT(VOICE_R_DN_PGA_NOISE_EN_OFFSET) |
			BIT(VOICE_R_DN_PGA_LINEAR_SEL_OFFSET),
			BIT(VOICE_R_DN_PGA_NOISE_EN_OFFSET) |
			BIT(VOICE_R_DN_PGA_LINEAR_SEL_OFFSET));
	}
	AUDIO_LOGI("au_pa pga cfg state change to %d", state);
	return 0;
}

#define AUDIO_FIFO_FS_KCONTROLS \
	SOC_SINGLE("FS_AUDIO_L_UPLINK",  FS_CTRL0_REG,  FS_AUDIO_L_UPLINK_OFFSET, 0x1, 0), \
	SOC_SINGLE("FS_AUDIO_R_UPLINK",  FS_CTRL0_REG,  FS_AUDIO_R_UPLINK_OFFSET, 0x1, 0), \
	SOC_SINGLE("FS_MIC3_UPLINK",     FS_CTRL0_REG,  FS_MIC3_UPLINK_OFFSET,    0x7, 0), \
	SOC_SINGLE("FS_ECHO_L_UPLINK",   FS_CTRL0_REG,  FS_ECHO_L_UPLINK_OFFSET,  0x7, 0), \
	SOC_SINGLE("FS_ECHO_R_UPLINK",   FS_CTRL0_REG,  FS_ECHO_R_UPLINK_OFFSET,  0x7, 0), \

#define AUDIO_SRC_KCONTROLS \
	SOC_SINGLE("AUDIO_L_UP_SRC5_MODE",  SRCUP_CTRL_REG,   AUDIO_L_UP_SRCUP_SRC_MODE_OFFSET,  0x7, 0), \
	SOC_SINGLE("AUDIO_L_UP_SRC5",       FS_CTRL0_REG,     FS_AUDIO_L_UP_SRCUP_IN_OFFSET,     0x3, 0), \
	SOC_SINGLE("AUDIO_R_UP_SRC6_MODE",  SRCUP_CTRL_REG,   AUDIO_R_UP_SRCUP_SRC_MODE_OFFSET,  0x7, 0), \
	SOC_SINGLE("AUDIO_R_UP_SRC6",       FS_CTRL0_REG,     FS_AUDIO_R_UP_SRCUP_IN_OFFSET,     0x3, 0), \
	SOC_SINGLE("MIC3_UP_SRC9_MODE",     SRCDN_CTRL_REG,   MIC3_UP_SRCDN_SRC_MODE_OFFSET,     0x7, 0), \
	SOC_SINGLE("MIC3_UP_SRC9",          FS_CTRL1_REG,     FS_MIC3_UP_SRCDN_OUT_OFFSET,       0x3, 0), \
	SOC_SINGLE("ECHO_L_UP_SRC10_MODE",  SRCDN_CTRL_REG,   ECHO_L_UP_SRCDN_SRC_MODE_OFFSET,   0x7, 0), \
	SOC_SINGLE("ECHO_L_UP_SRC10",       FS_CTRL1_REG,     FS_ECHO_L_UP_SRCDN_OUT_OFFSET,     0x3, 0), \
	SOC_SINGLE("ECHO_R_UP_SRC11_MODE",  SRCDN_CTRL_REG,   ECHO_R_UP_SRCDN_SRC_MODE_OFFSET,   0x7, 0), \
	SOC_SINGLE("ECHO_R_UP_SRC11",       FS_CTRL1_REG,     FS_ECHO_R_UP_SRCDN_OUT_OFFSET,     0x3, 0), \
	SOC_SINGLE("DACL_SRC2_MODE",        SRCUP_CTRL_REG,   DACL_SRCUP_SRC_MODE_OFFSET,        0x7, 0), \
	SOC_SINGLE("DACL_SRC2",             FS_CTRL0_REG,     FS_DACL_SRCUP_IN_OFFSET,           0x3, 0), \
	SOC_SINGLE("DACR_SRC4_MODE",        SRCUP_CTRL_REG,   DACR_SRCUP_SRC_MODE_OFFSET,        0x7, 0), \
	SOC_SINGLE("DACR_SRC4",             FS_CTRL0_REG,     FS_DACR_SRCUP_IN_OFFSET,           0x3, 0), \

#define AUDIO_PGA_KCONTROLS \
	SOC_SINGLE("AUDIO_L_DN_PGA_GAIN",    AUDIO_L_DN_PGA_CTRL_REG,  AUDIO_L_DN_PGA_GAIN_OFFSET,   0xff, 0), \
	SOC_SINGLE("AUDIO_L_DN_PGA_BYPASS",  AUDIO_L_DN_PGA_CTRL_REG,  AUDIO_L_DN_PGA_BYPASS_OFFSET, 0x1,  0), \
	SOC_SINGLE("AUDIO_R_DN_PGA_GAIN",    AUDIO_R_DN_PGA_CTRL_REG,  AUDIO_R_DN_PGA_GAIN_OFFSET,   0xff, 0), \
	SOC_SINGLE("AUDIO_R_DN_PGA_BYPASS",  AUDIO_R_DN_PGA_CTRL_REG,  AUDIO_R_DN_PGA_BYPASS_OFFSET, 0x1,  0), \
	SOC_SINGLE("AUDIO_L_UP_PGA_GAIN",    AUDIO_L_UP_PGA_CTRL_REG,  AUDIO_L_UP_PGA_GAIN_OFFSET,   0xff, 0), \
	SOC_SINGLE("AUDIO_L_UP_PGA_BYPASS",  AUDIO_L_UP_PGA_CTRL_REG,  AUDIO_L_UP_PGA_BYPASS_OFFSET, 0x1,  0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_GAIN",    AUDIO_R_UP_PGA_CTRL_REG,  AUDIO_R_UP_PGA_GAIN_OFFSET,   0xff, 0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_BYPASS",  AUDIO_R_UP_PGA_CTRL_REG,  AUDIO_R_UP_PGA_BYPASS_OFFSET, 0x1,  0), \
	SOC_SINGLE("MIC3_UP_PGA_GAIN",       MIC3_UP_PGA_CTRL_REG,     MIC3_UP_PGA_GAIN_OFFSET,      0xff, 0), \
	SOC_SINGLE("MIC3_UP_PGA_BYPASS",     MIC3_UP_PGA_CTRL_REG,     MIC3_UP_PGA_BYPASS_OFFSET,    0x1,  0), \
	SOC_SINGLE("IN3_AUDIO_DL_PGA",       DACL_MIXER4_CTRL0_REG,    DACL_MIXER4_GAIN3_OFFSET,     0x3,  0), \
	SOC_SINGLE("IN3_AUDIO_DR_PGA",       DACR_MIXER4_CTRL0_REG,    DACR_MIXER4_GAIN3_OFFSET,     0x3,  0), \
	SOC_SINGLE("ADCL_CIC_GAIN",          ADC_FILTER_CTRL_REG,      ADCL_CIC_GAIN_OFFSET,         0x3f, 0), \
	SOC_SINGLE("ADCR_CIC_GAIN",          ADC_FILTER_CTRL_REG,      ADCR_CIC_GAIN_OFFSET,         0x3f, 0), \
	SOC_SINGLE("ADC_MIC3_CIC_GAIN",      ADC_FILTER_CTRL_REG,      ADC_MIC3_CIC_GAIN_OFFSET,     0x3f, 0), \
	SOC_SINGLE("SIDETONE_PGA_BYPASS",    SIDETONE_PGA_CTRL_REG,    SIDETONE_PGA_BYPASS_OFFSET,   0x1,  0), \
	SOC_SINGLE("SIDETONE_PGA",           SIDETONE_PGA_CTRL_REG,    SIDETONE_PGA_GAIN_OFFSET,     0xff, 0), \
	SOC_SINGLE("IN4_SIDETONE_DL_PGA",    DACL_MIXER4_CTRL0_REG,    DACL_MIXER4_GAIN4_OFFSET,     0x3,  0), \
	SOC_SINGLE("IN4_SIDETONE_DR_PGA",    DACR_MIXER4_CTRL0_REG,    DACR_MIXER4_GAIN4_OFFSET,     0x3,  0), \

#define ANALOG_KCONTROLS \
	SOC_SINGLE("ANA_HPL_PGA_MUTE",           CODEC_ANA_RW5_REG,         HPL_MUTE_OFFSET,                 0x1, 0), \
	SOC_SINGLE("ANA_HPL_PGA",                CODEC_ANA_RW21_REG,        HSL_GAIN_OFFSET,                 0xF, 0), \
	SOC_SINGLE("ANA_HPR_PGA_MUTE",           CODEC_ANA_RW5_REG,         HPR_MUTE_OFFSET,                 0x1, 0), \
	SOC_SINGLE("ANA_HPR_PGA",                CODEC_ANA_RW21_REG,        HSR_GAIN_OFFSET,                 0xF, 0), \
	SOC_SINGLE("ANA_SPK_PGA_MUTE",           CLASSD_MUTE_REG,           CLASSD_MUTE_OFFSET,              0x1, 0), \
	SOC_SINGLE("ANA_SPK_PGA",                CLASSD_GAIN_REG,           CLASSD_GAIN_OFFSET,              0x3, 0), \
	SOC_SINGLE("ANA_EAR_PGA_MUTE",           CODEC_ANA_RW6_REG,         EAR_MUTE_OFFSET,                 0x1, 0), \
	SOC_SINGLE("ANA_EAR_PGA",                CODEC_ANA_RW26_REG,        EAR_GAIN_OFFSET,                 0xF, 0), \
	SOC_SINGLE("ANA_LINEOUT_PGA_MUTE",       CODEC_ANA_RW6_REG,         LOUT_MUTE_OFFSET,                0x1, 0), \
	SOC_SINGLE("ANA_LINEOUT_PGA",            CODEC_ANA_RW28_REG,        LOUT_GAIN_OFFSET,                0xF, 0), \
	SOC_SINGLE("ANA_MAINMIC_PGA_MUTE",       CODEC_ANA_RW2_REG,         MAINPGA_MUTE_OFFSET,             0x1, 0), \
	SOC_SINGLE("ANA_MAINPGA_BOOST",          CODEC_ANA_RW9_REG,         MAINPGA_BOOST_OFFSET,            0x1, 0), \
	SOC_SINGLE_TLV("ANA_MAINMIC_PGA",        CODEC_ANA_RW9_REG,         MAINPGA_GAIN_OFFSET,             0x7, 0, analog_mainpga_gain_tlv), \
	SOC_SINGLE("ANA_AUXMIC_PGA_MUTE",        CODEC_ANA_RW2_REG,         AUXPGA_MUTE_OFFSET,              0x1, 0), \
	SOC_SINGLE("ANA_AUXPGA_BOOST",           CODEC_ANA_RW10_REG,        AUXPGA_BOOST_OFFSET,             0x1, 0), \
	SOC_SINGLE_TLV("ANA_AUXMIC_PGA",         CODEC_ANA_RW10_REG,        AUXPGA_GAIN_OFFSET,              0x7, 0, analog_auxpga_gain_tlv), \
	SOC_SINGLE("ANA_MIC3_PGA_MUTE",          CODEC_ANA_RW2_REG,         MIC3PGA_MUTE_OFFSET,             0x1, 0),\
	SOC_SINGLE("ANA_MIC3PGA_BOOST",          CODEC_ANA_RW11_REG,        ADC3PGA_BOOST_OFFSET,            0x1, 0),\
	SOC_SINGLE_TLV("ANA_MIC3_PGA",           CODEC_ANA_RW11_REG,        MIC3PGA_GAIN_OFFSET,             0x7, 0, analog_mic3pga_gain_tlv), \
	SOC_SINGLE("ANA_MIXER_EAR_L_PGA",        CODEC_ANA_RW25_REG,        CR_MIXER_EP_DACL_OFFSET,         0x1, 0), \
	SOC_SINGLE("ANA_MIXER_EAR_R_PGA",        CODEC_ANA_RW25_REG,        CR_MIXER_EP_DACR_OFFSET,         0x1, 0), \
	SOC_SINGLE("ANA_MIXER_EAR_ULTR_PGA",     CODEC_ANA_RW25_REG,        CR_MIXER_EP_DAC_ULTRA_OFFSET,    0x1, 0), \
	SOC_SINGLE("ANA_MIXER_LINEOUT_L_PGA",    CODEC_ANA_RW27_REG,        LOMIX_BOOSTGAIN_DACL_OFFSET,     0x1, 0), \
	SOC_SINGLE("ANA_MIXER_LINEOUT_R_PGA",    CODEC_ANA_RW27_REG,        LOMIX_BOOSTGAIN_DACR_OFFSET,     0x1, 0), \
	SOC_SINGLE("ANA_MIXER_LINEOUT_ULTR_PGA", CODEC_ANA_RW27_REG,        LOMIX_BOOSTGAIN_DACULTRA_OFFSET, 0x1, 0), \

#define CLASSD_KCONTROLS \
	SOC_SINGLE("CLASSD_P_SEL",   CLASSD_P_SEL_REG,  CLASSD_P_SEL_OFFSET,  0x3,  0), \
	SOC_SINGLE("CLASSD_N_SEL",   CLASSD_N_SEL_REG,  CLASSD_N_SEL_OFFSET,  0x3,  0), \

#define CUSTOMER_KCONTROLS \
	SOC_ENUM_EXT("HAC", hac_switch_enum[0], hac_switch_get, hac_switch_put), \
	SOC_ENUM_EXT("HISIPA_PGA_CFG", au_pa_pga_cfg_enum[0], au_pa_pga_cfg_get, au_pa_pga_cfg_put), \


static const struct snd_kcontrol_new audio_controls[] = {
	AUDIO_FIFO_FS_KCONTROLS
	AUDIO_SRC_KCONTROLS
	AUDIO_PGA_KCONTROLS
	ANALOG_KCONTROLS
	CLASSD_KCONTROLS
	CUSTOMER_KCONTROLS
};

static const struct snd_kcontrol_new ear_mixer[] = {
	SOC_DAPM_SINGLE("DACL", CODEC_ANA_RW25_REG,
		MIXOUT_EAR_DACL_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DACR", CODEC_ANA_RW25_REG,
		MIXOUT_EAR_DACR_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DAC_ULTR", CODEC_ANA_RW25_REG,
		MIXOUT_EAR_DAC_ULTRA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new analog_sif_en_mixer[] = {
	SOC_DAPM_SINGLE("ANA_SIFL_EN", VIR0_ADDR, REG_BIT16, 1, 0),
	SOC_DAPM_SINGLE("ANA_SIFR_EN", VIR0_ADDR, REG_BIT17, 1, 0),
	SOC_DAPM_SINGLE("ANA_MUX3_EN", VIR0_ADDR, REG_BIT18, 1, 0),
	SOC_DAPM_SINGLE("ANA_MUX4_EN", VIR0_ADDR, REG_BIT19, 1, 0),
	SOC_DAPM_SINGLE("ANA_MUX5_EN", VIR0_ADDR, REG_BIT20, 1, 0),
	SOC_DAPM_SINGLE("ANA_SIF_ULTR_EN", VIR0_ADDR, REG_BIT30, 1, 0),
	SOC_DAPM_SINGLE("ANA_SIF_MAD_EN", VIR1_ADDR, REG_BIT6, 1, 0),
};

static const struct snd_kcontrol_new lineout_mixer[] = {
	SOC_DAPM_SINGLE("DACL", CODEC_ANA_RW27_REG,
		LOMIX_SEL_DACL_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DACR", CODEC_ANA_RW27_REG,
		LOMIX_SEL_DACR_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DAC_ULTR", CODEC_ANA_RW27_REG,
		LOMIX_SEL_DAC_ULTRA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new analog_adc1_mixer[] = {
	SOC_DAPM_SINGLE("DACL", CODEC_ANA_RW12_REG,
		ADCL_MIXIN_DACL_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DACR", CODEC_ANA_RW12_REG,
		ADCL_MIXIN_DACR_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("MAIN MIC", CODEC_ANA_RW12_REG,
		ADCL_MIXIN_MICPGA1_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("AUX MIC", CODEC_ANA_RW12_REG,
		ADCL_MIXIN_MICPGA2_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new analog_adc2_mixer[] = {
	SOC_DAPM_SINGLE("DACL", CODEC_ANA_RW14_REG,
		ADCR_MIXIN_DACL_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DACR", CODEC_ANA_RW14_REG,
		ADCR_MIXIN_DACR_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("MAIN MIC", CODEC_ANA_RW14_REG,
		ADCR_MIXIN_MICPGA1_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("AUX MIC", CODEC_ANA_RW14_REG,
		ADCR_MIXIN_MICPGA2_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new analog_adc3_mixer[] = {
	SOC_DAPM_SINGLE("DACL", CODEC_ANA_RW16_REG,
		ADC3_MIXIN_DACL_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("DACR", CODEC_ANA_RW16_REG,
		ADC3_MIXIN_ULTRA_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("MAIN MIC", CODEC_ANA_RW16_REG,
		ADC3_MIXIN_MICPGA1_OFFSET, 1, 0),
	SOC_DAPM_SINGLE("MIC3", CODEC_ANA_RW16_REG,
		ADC3_MIXIN_MICPGA3_OFFSET, 1, 0),
};

static const char *const audio_ul_mux_texts[] = {
	"DACL_MIXER",
	"ADCL",
	"S2_L_SRC",
};

static const struct soc_enum audio_ul_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_AUDIO_L_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(audio_ul_mux_texts), audio_ul_mux_texts);

static const struct snd_kcontrol_new audio_ul_mux =
	SOC_DAPM_ENUM("Mux", audio_ul_mux_enum);

static const char *const audio_ur_mux_texts[] = {
	"DACR_MIXER",
	"ADCR",
	"S2_R_SRC",
};

static const struct soc_enum audio_ur_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_AUDIO_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(audio_ur_mux_texts), audio_ur_mux_texts);

static const struct snd_kcontrol_new audio_ur_mux =
	SOC_DAPM_ENUM("Mux", audio_ur_mux_enum);

static const char *const mic3_up_mux_texts[] = {
	"ADC3/ADC2/ADC1",
	"ADC MIC3_SRC",
};

static const struct soc_enum mic3_up_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, MIC3_UP_AFIFO_DIN_SEL_OFFSET,
		ARRAY_SIZE(mic3_up_mux_texts), mic3_up_mux_texts);

static const struct snd_kcontrol_new mic3_up_mux =
	SOC_DAPM_ENUM("Mux", mic3_up_mux_enum);

static const char *const adc1_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_L",
};

static const struct soc_enum adc1_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADCL_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc1_mux_texts), adc1_mux_texts);

static const struct snd_kcontrol_new adc1_mux =
	SOC_DAPM_ENUM("Mux", adc1_mux_enum);

static const char *const adc2_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_R",
};

static const struct soc_enum adc2_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADCR_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc2_mux_texts), adc2_mux_texts);

static const struct snd_kcontrol_new adc2_mux =
	SOC_DAPM_ENUM("Mux", adc2_mux_enum);

static const char *const adc3_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC2_L",
};

static const struct soc_enum adc3_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADC_MIC3_DIN_SEL_OFFSET,
		ARRAY_SIZE(adc3_mux_texts), adc3_mux_texts);

static const struct snd_kcontrol_new adc3_mux =
	SOC_DAPM_ENUM("Mux", adc3_mux_enum);


static const char *const analog_dacl_mux_texts[] = {
	"SIFL",
	"ADC1",
	"ADC2",
	"ADC3",
};

static const struct soc_enum analog_dacl_mux_enum =
	SOC_ENUM_SINGLE(DAC_CHAN_CTRL_REG, DACL_DIN_SEL_OFFSET,
		ARRAY_SIZE(analog_dacl_mux_texts), analog_dacl_mux_texts);

static const struct snd_kcontrol_new analog_dacl_mux =
	SOC_DAPM_ENUM("Mux", analog_dacl_mux_enum);

static const char *const analog_dacr_mux_texts[] = {
	"SIFR",
	"ADC1",
	"ADC2",
	"ADC3",
};

static const struct soc_enum analog_dacr_mux_enum =
	SOC_ENUM_SINGLE(DAC_CHAN_CTRL_REG, DACR_DIN_SEL_OFFSET,
		ARRAY_SIZE(analog_dacr_mux_texts), analog_dacr_mux_texts);

static const struct snd_kcontrol_new analog_dacr_mux =
	SOC_DAPM_ENUM("Mux", analog_dacr_mux_enum);

static const char *const analog_mux3_texts[] = {
	"ADC1",
	"SIFL",
	"SIFR",
};

static const struct soc_enum analog_mux3_enum =
	SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADCL_DOUT_SEL_OFFSET,
		ARRAY_SIZE(analog_mux3_texts), analog_mux3_texts);

static const struct snd_kcontrol_new analog_mux3 =
	SOC_DAPM_ENUM("Mux", analog_mux3_enum);

static const char *const analog_mux4_texts[] = {
	"ADC2",
	"SIFL",
	"SIFR",
};

static const struct soc_enum analog_mux4_enum =
	SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADCR_DOUT_SEL_OFFSET,
		ARRAY_SIZE(analog_mux4_texts), analog_mux4_texts);

static const struct snd_kcontrol_new analog_mux4 =
	SOC_DAPM_ENUM("Mux", analog_mux4_enum);

static const char *const analog_mux5_texts[] = {
	"ADC3",
	"SIFL",
	"SIFR",
};

static const struct soc_enum analog_mux5_enum =
	SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADC_MIC3_DOUT_SEL_OFFSET,
		ARRAY_SIZE(analog_mux5_texts), analog_mux5_texts);

static const struct snd_kcontrol_new analog_mux5 =
	SOC_DAPM_ENUM("Mux", analog_mux5_enum);

static const char *const main_hp_mic_mux_texts[] = {
	"MUTE",
	"MAIN_MIC",
	"HP_MIC",
};

static const struct soc_enum main_hp_mic_mux_enum =
	SOC_ENUM_SINGLE(CODEC_ANA_RW9_REG, MAINPGA_SEL_OFFSET,
		ARRAY_SIZE(main_hp_mic_mux_texts), main_hp_mic_mux_texts);

static const struct snd_kcontrol_new main_hp_mic_mux =
	SOC_DAPM_ENUM("Mux", main_hp_mic_mux_enum);

static const char *const src9_mux_texts[] = {
	"BYPASS",
	"SRC9",
};

static const struct soc_enum src9_mux_enum =
	SOC_ENUM_SINGLE(CODEC_CLK_EN1_REG, MIC3_UP_SRCDN_CLKEN_OFFSET,
		ARRAY_SIZE(src9_mux_texts), src9_mux_texts);

static const struct snd_kcontrol_new src9_mux =
	SOC_DAPM_ENUM("Mux", src9_mux_enum);

static const char *const src10_mux_texts[] = {
	"BYPASS",
	"SRC10",
};

static const struct soc_enum src10_mux_enum =
	SOC_ENUM_SINGLE(CODEC_CLK_EN1_REG, ECHO_L_UP_SRCDN_CLKEN_OFFSET,
		ARRAY_SIZE(src10_mux_texts), src10_mux_texts);

static const struct snd_kcontrol_new src10_mux =
	SOC_DAPM_ENUM("Mux", src10_mux_enum);


static const char *const src11_mux_texts[] = {
	"BYPASS",
	"SRC11",
};

static const struct soc_enum src11_mux_enum =
	SOC_ENUM_SINGLE(CODEC_CLK_EN1_REG, ECHO_R_UP_SRCDN_CLKEN_OFFSET,
		ARRAY_SIZE(src11_mux_texts), src11_mux_texts);

static const struct snd_kcontrol_new src11_mux =
	SOC_DAPM_ENUM("Mux", src11_mux_enum);

static const char *const echo_l_mux_texts[] = {
	"DACL_MIXER",
	"S3_L",
	"ADC_SPAI"
};

static const struct soc_enum echo_l_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_ECHO_L_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(echo_l_mux_texts), echo_l_mux_texts);

static const struct snd_kcontrol_new echo_l_mux =
	SOC_DAPM_ENUM("Mux", echo_l_mux_enum);

static const char *const echo_r_mux_texts[] = {
	"DACR_MIXER",
	"S3_R",
	"ADC_SPAV",
};

static const struct soc_enum echo_r_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_ECHO_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(echo_r_mux_texts), echo_r_mux_texts);

static const struct snd_kcontrol_new echo_r_mux =
	SOC_DAPM_ENUM("Mux", echo_r_mux_enum);

static const char *const side_tone_mux_texts[] = {
	"ADC_MIC3_I",
	"ADCR_I",
	"ADCL_I",
};

static const struct soc_enum side_tone_mux_enum =
	SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, SIDETONE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(side_tone_mux_texts), side_tone_mux_texts);

static const struct snd_kcontrol_new side_tone_mux =
	SOC_DAPM_ENUM("Mux", side_tone_mux_enum);

static const char *const ultr_adc_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_L",
	"DMIC1_R",
	"DMIC2_L",
};

static const struct soc_enum ultr_adc_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, ADC_ULTR_DIN_SEL_OFFSET,
		ARRAY_SIZE(ultr_adc_mux_texts), ultr_adc_mux_texts);

static const struct snd_kcontrol_new ultr_adc_mux = SOC_DAPM_ENUM("Mux", ultr_adc_mux_enum);

static const char * const dacl_vib_mux_texts[] = {
	"SRC_OFF",
	"SRC_ON",
};

static const struct soc_enum dacl_vib_mux_enum =
	SOC_ENUM_SINGLE(CODEC_CLK_EN0_REG, DACL_SRCUP_CLKEN_OFFSET,
	ARRAY_SIZE(dacl_vib_mux_texts), dacl_vib_mux_texts);

static const struct snd_kcontrol_new dacl_vib_mux = SOC_DAPM_ENUM("Mux", dacl_vib_mux_enum);

static const struct snd_kcontrol_new analog_headset_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT9, 1, 0),
};

static const struct snd_kcontrol_new analog_spk_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", CLASSD_EN_REG,
		CLASSD_EN_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new analog_rcv_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", CODEC_ANA_RW6_REG, EAR_PD_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new analog_lout_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", CODEC_ANA_RW6_REG, LOUT_PD_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new echo_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT6, 1, 0),
};

static const struct snd_kcontrol_new audio_ul_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT2, 1, 0),
};

static const struct snd_kcontrol_new audio_ur_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT3, 1, 0),
};

static const struct snd_kcontrol_new analog_dacl_switch[] = {
	SOC_DAPM_SINGLE("DACL", VIR0_ADDR, REG_BIT0, 1, 0),
};

static const struct snd_kcontrol_new analog_dacr_switch[] = {
	SOC_DAPM_SINGLE("DACR", VIR0_ADDR, REG_BIT1, 1, 0),
};

static const struct snd_kcontrol_new mic3_up_fifo_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR0_ADDR, REG_BIT8, 1, 0),
};

static const struct snd_kcontrol_new mic3_bias1_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT7, 1, 0),
};

static const struct snd_kcontrol_new mic3_bias2_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT8, 1, 0),
};

static const struct snd_kcontrol_new auxmic_bias1_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT9, 1, 0),
};

static const struct snd_kcontrol_new auxmic_bias2_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT10, 1, 0),
};

static const struct snd_kcontrol_new audio_carkit_switch[] = {
	SOC_DAPM_SINGLE("ENABLE", VIR1_ADDR, REG_BIT11, 1, 0),
};

static int audio_dn_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_L_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_R_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_DN_PGA_CLKEN_OFFSET) |
			BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int dacl_filter_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/sdm clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACL_FILTER_CLKEN_OFFSET), BIT(DACL_FILTER_CLKEN_OFFSET));
		if (priv->classh_clk_enable)
			snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(CLASSH_CLKEN_OFFSET), BIT(CLASSH_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		if (priv->classh_clk_enable)
			snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(CLASSH_CLKEN_OFFSET), 0);
		/* filter/sdm clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACL_FILTER_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int sdml_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* src16 enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACL_UP16_CLKEN_OFFSET), BIT(DACL_UP16_CLKEN_OFFSET));

		/* sif smt dacl enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(SDM_L_CLKEN_OFFSET), BIT(SDM_L_CLKEN_OFFSET));
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DACL_EN_OFFSET) | BIT(SDM_L_CALT_VLD_OFFSET),
			BIT(SIF_MST_DACL_EN_OFFSET) | BIT(SDM_L_CALT_VLD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* src16 disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACL_UP16_CLKEN_OFFSET), 0);

		/* sif smt dacl disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(SDM_L_CLKEN_OFFSET), 0);
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DACL_EN_OFFSET) | BIT(SDM_L_CALT_VLD_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int dacr_filter_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* src/filter/src16/sdm clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACR_SRCUP_CLKEN_OFFSET) | BIT(DACR_FILTER_CLKEN_OFFSET) |
			BIT(DACR_UP16_CLKEN_OFFSET) | BIT(SDM_R_CLKEN_OFFSET),
			BIT(DACR_SRCUP_CLKEN_OFFSET) | BIT(DACR_FILTER_CLKEN_OFFSET) |
			BIT(DACR_UP16_CLKEN_OFFSET) | BIT(SDM_R_CLKEN_OFFSET)); /* lint !e648 */
		/* sif smt dacr enable */
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DACR_EN_OFFSET) | BIT(SDM_R_CALT_VLD_OFFSET),
			BIT(SIF_MST_DACR_EN_OFFSET) | BIT(SDM_R_CALT_VLD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* src/filter/src16/sdm clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(DACR_SRCUP_CLKEN_OFFSET) | BIT(DACR_FILTER_CLKEN_OFFSET) |
			BIT(DACR_UP16_CLKEN_OFFSET) | BIT(SDM_R_CLKEN_OFFSET), 0); /* lint !e648 */

		/* sif smt dacr disable */
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_DACR_EN_OFFSET) | BIT(SDM_R_CALT_VLD_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adcl_filter_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADCL_FILTER_CLKEN_OFFSET) | BIT(AUDIO_L_UP_PGA_CLKEN_OFFSET),
			BIT(ADCL_FILTER_CLKEN_OFFSET) | BIT(AUDIO_L_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADCL_FILTER_CLKEN_OFFSET) |
			BIT(AUDIO_L_UP_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adcr_filter_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADCR_FILTER_CLKEN_OFFSET) | BIT(AUDIO_R_UP_PGA_CLKEN_OFFSET),
			BIT(ADCR_FILTER_CLKEN_OFFSET) | BIT(AUDIO_R_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk  disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADCR_FILTER_CLKEN_OFFSET) |
			BIT(AUDIO_R_UP_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adc_mic3_filter_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADC_MIC3_FILTER_CLKEN_OFFSET) | BIT(MIC3_UP_PGA_CLKEN_OFFSET),
			BIT(ADC_MIC3_FILTER_CLKEN_OFFSET) | BIT(MIC3_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ADC_MIC3_FILTER_CLKEN_OFFSET) | BIT(MIC3_UP_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct reg_seq_config hplr_pga_up_regs[] = {
	{ { CODEC_ANA_RW20_REG, 0, 0x88, false }, 0, 0 },
	{ { CODEC_ANA_RW24_REG, 0, 0x36, false }, 0, 0 },
	{ { CODEC_ANA_RW19_REG, 0, 0x8C, false }, 0, 0 },
	{ { CODEC_ANA_RW5_REG, 0, 0x7C, false }, RANGE_SLEEP, 5000 },

	{ { CODEC_ANA_RW19_REG, 0, 0xCC, false }, 0, 0 },
	{ { CODEC_ANA_RW5_REG, 0, 0x4C, false }, 0, 0 },
	{ { CODEC_ANA_RW24_REG, 0, 0x16, false }, 0, 0 },
	{ { CODEC_ANA_RW5_REG, 0, 0x40, false }, MSLEEP, 90000 },

	{ { CODEC_ANA_RW5_REG, 0, 0x0, false }, 0, 0 },
};

static const struct reg_seq_config hplr_pga_down_regs[] = {
	{ { CODEC_ANA_RW5_REG, 0, 0x40, false }, 0, 0 },
	{ { CODEC_ANA_RW5_REG, 0, 0x4C, false }, MSLEEP, 30000 },

	{ { CODEC_ANA_RW5_REG, 0, 0x7C, false }, 0, 0 },
	{ { CODEC_ANA_RW19_REG, 0, 0x8C, false }, 0, 0 },

	{ { CODEC_ANA_RW5_REG, 0, 0xFC, false }, 0, 0 },
	{ { CODEC_ANA_RW24_REG, 0, 0x36, false }, 0, 0 },
	{ { CODEC_ANA_RW20_REG, 0, 0x0, false }, 0, 0 },
};

static int hplr_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);
	unsigned int val_l;
	unsigned int val_r;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* if hp path contain DAC DL/DR MIXER, mute audio DL&DR to avoid headphone pop */
		val_l = snd_soc_component_read32(codec, DACL_MIXER4_CTRL0_REG);
		val_l = val_l & BIT(DACL_MIXER4_IN3_MUTE_OFFSET);
		val_r = snd_soc_component_read32(codec, DACR_MIXER4_CTRL0_REG);
		val_r = val_r & BIT(DACR_MIXER4_IN3_MUTE_OFFSET);

		if (!val_l)
			snd_soc_component_update_bits(codec, DACL_MIXER4_CTRL0_REG,
				BIT(DACL_MIXER4_IN3_MUTE_OFFSET),
				BIT(DACL_MIXER4_IN3_MUTE_OFFSET));

		if (!val_r)
			snd_soc_component_update_bits(codec, DACR_MIXER4_CTRL0_REG,
				BIT(DACR_MIXER4_IN3_MUTE_OFFSET),
				BIT(DACR_MIXER4_IN3_MUTE_OFFSET));

		if (!val_l || !val_r)
			usleep_range(5000, 5500);

		da_separate_reg_seq_write_array(codec, hplr_pga_up_regs,
			ARRAY_SIZE(hplr_pga_up_regs));

		/* if hp path contain DAC DL/DR MIXER, unmute audio DL&DR to avoid headphone pop */
		if (!val_l)
			snd_soc_component_update_bits(codec, DACL_MIXER4_CTRL0_REG,
				BIT(DACL_MIXER4_IN3_MUTE_OFFSET), 0);
		if (!val_r)
			snd_soc_component_update_bits(codec, DACR_MIXER4_CTRL0_REG,
				BIT(DACR_MIXER4_IN3_MUTE_OFFSET), 0);

		if (priv->ops.codec_customized_func)
			priv->ops.codec_customized_func(priv, DA_SEPARATE_AUDIO_PATH,
				USB_ANA_LDO_CTL, STATE_ENABLE);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		if (priv->ops.codec_customized_func)
			priv->ops.codec_customized_func(priv, DA_SEPARATE_AUDIO_PATH,
				USB_ANA_LDO_CTL, STATE_DISABLE);

		da_separate_reg_seq_write_array(codec, hplr_pga_down_regs,
			ARRAY_SIZE(hplr_pga_down_regs));
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int side_tone_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* pga/fifo/src clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(SIDETONE_PGA_CLKEN_OFFSET),
			BIT(SIDETONE_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(SIDETONE_PGA_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adc3_pga_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGI("power mode event: %d", event);
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_ADC_MIC3_EN_OFFSET),
			BIT(SIF_MST_ADC_MIC3_EN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_component_update_bits(codec, DAC_FILTER_CTRL_REG,
			BIT(SIF_MST_ADC_MIC3_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	return 0;
}

static int analog_sif_mixer_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* codec ana/sif en */
		snd_soc_component_update_bits(codec, CLK_EN_CFG_REG,
			BIT(CODEC_ANA_EN_OFFSET) | BIT(SIF_EN_OFFSET),
			BIT(CODEC_ANA_EN_OFFSET) | BIT(SIF_EN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* codec ana/sif dis */
		snd_soc_component_update_bits(codec, CLK_EN_CFG_REG,
			BIT(CODEC_ANA_EN_OFFSET) | BIT(SIF_EN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int audio_ul_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int audio_ur_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET),
			BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int mic3_up_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(MIC3_UP_AFIFO_CLKEN_OFFSET), 0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(MIC3_UP_AFIFO_CLKEN_OFFSET),
			BIT(MIC3_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN0_REG,
			BIT(MIC3_UP_AFIFO_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int analog_dacl_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* dem/dacl clk enable */
		snd_soc_component_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DACL_DWA_EN_OFFSET), BIT(DACL_DWA_EN_OFFSET));

		snd_soc_component_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACL_PD_OFFSET), 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* dem/dacl clk disable */
		snd_soc_component_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DACL_DWA_EN_OFFSET), 0);

		snd_soc_component_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACL_PD_OFFSET), BIT(DACL_PD_OFFSET));
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int analog_dacr_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* dem/dacr clk enable */
		snd_soc_component_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DACR_DWA_EN_OFFSET), BIT(DACR_DWA_EN_OFFSET));
		snd_soc_component_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACR_PD_OFFSET), 0);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* dem/dacr clk disable */
		snd_soc_component_update_bits(codec, DAC_CHAN_CTRL_REG,
			BIT(DACR_DWA_EN_OFFSET), 0);
		snd_soc_component_update_bits(codec, CODEC_ANA_RW4_REG,
			BIT(DACR_PD_OFFSET), BIT(DACR_PD_OFFSET));
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int analog_spk_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		snd_soc_component_update_bits(codec, CLASSD_MIN_TN_REG,
			0x7 << CLASSD_MIN_TN_OFFSET, 0x4 << CLASSD_MIN_TN_OFFSET);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		snd_soc_component_update_bits(codec, CLASSD_MIN_TN_REG,
			0x7 << CLASSD_MIN_TN_OFFSET, 0x0);

		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int echo_fifo_switch_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET) |
			BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET),
			0);

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET) |
			BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET),
			BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET) |
			BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		snd_soc_component_update_bits(codec, CODEC_CLK_EN1_REG,
			BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET) |
			BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET),
			0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int micbias1_mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW7_REG,
			MICBISA1_SET_MASK, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW7_REG,
			MICBISA1_SET_MASK, 0xff);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}
	OUT_FUNCTION;

	return 0;
}

static int micbias2_mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW7_REG, MICBISA2_SET_MASK, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_ANA_RW7_REG, MICBISA2_SET_MASK, 0xff);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}
	OUT_FUNCTION;

	return 0;
}

static int hpmicbias_mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);
	struct platform_data *priv = snd_soc_component_get_drvdata(codec);
	unsigned int irq_mask;

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* to avoid irq while MBHD_COMP power up, mask all COMP irq */
		irq_mask = snd_soc_component_read32(codec, ANA_IRQM_REG0_REG);
		snd_soc_component_update_bits(codec, ANA_IRQM_REG0_REG,
			irq_mask | IRQ_MSK_COMP, irq_mask | IRQ_MSK_COMP);
		da_separate_mbhc_set_micbias(priv->mbhc, true);
		msleep(25);

		/* when pwr up finished clean it and cancel mask */
		snd_soc_component_write(codec, ANA_IRQ_REG0_REG, IRQ_MSK_COMP);
		snd_soc_component_update_bits(codec, ANA_IRQM_REG0_REG, IRQ_MSK_BTN_NOR, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		da_separate_mbhc_set_micbias(priv->mbhc, false);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int dmic1_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DMIC1_CLKEN_OFFSET), BIT(DMIC1_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DMIC1_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int dmic2_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DMIC2_CLKEN_OFFSET), BIT(DMIC2_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_component_update_bits(codec, CODEC_CLK_EN3_REG,
			BIT(DMIC2_CLKEN_OFFSET), 0);
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int adc_drv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		usleep_range(10000, 11000);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		break;
	default:
		AUDIO_LOGE("power mode event err: %d", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

#define AUDIO_INPUT_OUTPUT_WIDGET \
	SND_SOC_DAPM_INPUT("AUDIO_DLINK INPUT"), \
	SND_SOC_DAPM_INPUT("SMT_ANA_DAC_SDATA INPUT"), \
	SND_SOC_DAPM_INPUT("ADC1 INPUT"), \
	SND_SOC_DAPM_INPUT("ADC2 INPUT"), \
	SND_SOC_DAPM_INPUT("ADC3 INPUT"), \
	SND_SOC_DAPM_INPUT("MAINMIC INPUT"), \
	SND_SOC_DAPM_INPUT("HPMIC INPUT"), \
	SND_SOC_DAPM_INPUT("AUXMIC INPUT"), \
	SND_SOC_DAPM_INPUT("MIC3 INPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_DLINK OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_UL OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_UR OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("MIC3_UP OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("SMT_HP_LR OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("ANA_EAR OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("ANA_SIF OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("ECHO OUTPUT"), \
	SND_SOC_DAPM_INPUT("DMIC1 INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC2 INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_CARKIT_DLINK INPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_CARKIT_DLINK OUTPUT"), \


#define AUDIO_PGA_WIDGET \
	SND_SOC_DAPM_PGA_E("AUDIO_DN PGA", SND_SOC_NOPM, 0, 0, NULL, 0, \
		audio_dn_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("DACL_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		dacl_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("SDML", SND_SOC_NOPM, 0, 0, NULL, 0, \
		sdml_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("DACR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		dacr_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("ADCL_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		adcl_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("ADCR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		adcr_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("ADC_MIC3_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0, \
		adc_mic3_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("HP_LR PGA", SND_SOC_NOPM, 0, 0, NULL, 0, \
		hplr_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA("ANA_MAIN PGA", CODEC_ANA_RW2_REG, MAINPGA_PD_OFFSET, 1, NULL, 0), \
	SND_SOC_DAPM_PGA("ANA_AUX PGA", CODEC_ANA_RW2_REG, AUXPGA_PD_OFFSET, 1, NULL, 0), \
	SND_SOC_DAPM_PGA("ANA_MIC3 PGA", CODEC_ANA_RW2_REG, MIC3PGA_PD_OFFSET, 1, NULL, 0), \
	SND_SOC_DAPM_PGA("AUDIO_UP_SRC5_EN PGA", CODEC_CLK_EN1_REG, AUDIO_L_UP_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA("AUDIO_UP_SRC6_EN PGA", CODEC_CLK_EN1_REG, AUDIO_R_UP_SRCUP_CLKEN_OFFSET, 0, NULL, 0), \
	SND_SOC_DAPM_PGA_E("SIDE_TONE_EN PGA", SND_SOC_NOPM, 0, 0, NULL, 0, \
		side_tone_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_PGA_E("ADC3 PGA", SND_SOC_NOPM, 0, 0, NULL, 0, \
		adc3_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \

#define AUDIO_MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("ANA_EAR MIXER", CODEC_ANA_RW6_REG, MIXOUT_EAR_PD_OFFSET, 1, \
		ear_mixer, ARRAY_SIZE(ear_mixer)), \
	SND_SOC_DAPM_MIXER("ANA_LINEOUT MIXER", CODEC_ANA_RW6_REG, LOMIXER_PD_OFFSET, 1, \
		lineout_mixer, ARRAY_SIZE(lineout_mixer)), \
	SND_SOC_DAPM_MIXER("ANA_ADC1 MIXER", CODEC_ANA_RW3_REG, ADCL_PD_OFFSET, 1, \
		analog_adc1_mixer, ARRAY_SIZE(analog_adc1_mixer)), \
	SND_SOC_DAPM_MIXER("ANA_ADC2 MIXER", CODEC_ANA_RW3_REG, ADCR_PD_OFFSET, 1, \
		analog_adc2_mixer, ARRAY_SIZE(analog_adc2_mixer)), \
	SND_SOC_DAPM_MIXER("ANA_ADC3 MIXER", CODEC_ANA_RW3_REG, ADC3_PD_OFFSET, 1, \
		analog_adc3_mixer, ARRAY_SIZE(analog_adc3_mixer)), \
	SND_SOC_DAPM_MIXER_E("ANA_SIF_EN MIXER", SND_SOC_NOPM, 0, 0, \
		analog_sif_en_mixer, ARRAY_SIZE(analog_sif_en_mixer), \
		analog_sif_mixer_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define AUDIO_MUX_WIDGET \
	SND_SOC_DAPM_MUX("AUDIO_UL MUX", SND_SOC_NOPM, 0, 0, &audio_ul_mux), \
	SND_SOC_DAPM_MUX("AUDIO_UR MUX", SND_SOC_NOPM, 0, 0, &audio_ur_mux), \
	SND_SOC_DAPM_MUX("MIC3_UP MUX", SND_SOC_NOPM, 0, 0, &mic3_up_mux), \
	SND_SOC_DAPM_MUX("ADC1 MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADCL_EN_OFFSET, 0, &adc1_mux), \
	SND_SOC_DAPM_MUX("ADC2 MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADCR_EN_OFFSET, 0, &adc2_mux), \
	SND_SOC_DAPM_MUX("ADC3 MUX", SND_SOC_NOPM, 0, 0, &adc3_mux), \
	SND_SOC_DAPM_MUX("ANA_DAC_L MUX", CLK_EN_CFG_REG, DACL_EN_OFFSET, 0, &analog_dacl_mux), \
	SND_SOC_DAPM_MUX("ANA_DAC_R MUX", CLK_EN_CFG_REG, DACR_EN_OFFSET, 0, &analog_dacr_mux), \
	SND_SOC_DAPM_MUX("ANA_MUX3 MUX", CLK_EN_CFG_REG, ADCL_EN_OFFSET, 0, &analog_mux3), \
	SND_SOC_DAPM_MUX("ANA_MUX4 MUX", CLK_EN_CFG_REG, ADCR_EN_OFFSET, 0, &analog_mux4), \
	SND_SOC_DAPM_MUX("ANA_MUX5 MUX", CLK_EN_CFG_REG, ADC_MIC3_EN_OFFSET, 0, &analog_mux5), \
	SND_SOC_DAPM_MUX("MAIN_HP_MIC MUX", SND_SOC_NOPM, 0, 0, &main_hp_mic_mux), \
	SND_SOC_DAPM_MUX("SIDE_TONE MUX", SND_SOC_NOPM, 0, 0, &side_tone_mux), \
	SND_SOC_DAPM_MUX("MIC3_SRC9 MUX", SND_SOC_NOPM, 0, 0, &src9_mux), \
	SND_SOC_DAPM_MUX("ECHO_SRC10 MUX", SND_SOC_NOPM, 0, 0, &src10_mux), \
	SND_SOC_DAPM_MUX("ECHO_SRC11 MUX", SND_SOC_NOPM, 0, 0, &src11_mux), \
	SND_SOC_DAPM_MUX("ECHO_L MUX", SND_SOC_NOPM, 0, 0, &echo_l_mux), \
	SND_SOC_DAPM_MUX("ECHO_R MUX", SND_SOC_NOPM, 0, 0, &echo_r_mux), \
	SND_SOC_DAPM_MUX("ADC_ULTR MUX", SND_SOC_NOPM, 0, 0, &ultr_adc_mux), \
	SND_SOC_DAPM_MUX("DAC_DL_VIB MUX", SND_SOC_NOPM, 0, 0, &dacl_vib_mux), \

#define AUDIO_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("AUDIO_UL_FIFO SWITCH", SND_SOC_NOPM, 0, 0, audio_ul_fifo_switch, \
		audio_ul_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIO_UR_FIFO SWITCH", SND_SOC_NOPM, 0, 0, audio_ur_fifo_switch, \
		audio_ur_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MIC3_UP_FIFO SWITCH", SND_SOC_NOPM, 0, 0, mic3_up_fifo_switch, \
		mic3_up_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_L SWITCH", SND_SOC_NOPM, 0, 0, analog_dacl_switch, \
		analog_dacl_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_R SWITCH", SND_SOC_NOPM, 0, 0, analog_dacr_switch, \
		analog_dacr_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH("ANA_HEADSET SWITCH", SND_SOC_NOPM, 0, 0, analog_headset_switch), \
	SND_SOC_DAPM_SWITCH_E("ANA_SPK SWITCH", CLASSD_DRV_EN_REG, CLASSD_DRV_EN_OFFSET, 0, analog_spk_switch, \
		analog_spk_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH("ANA_RCV SWITCH", CODEC_ANA_RW6_REG, EPVCM_PD_OFFSET, 1, analog_rcv_switch), \
	SND_SOC_DAPM_SWITCH("ANA_LOUT SWITCH", SND_SOC_NOPM, 0, 0, analog_lout_switch), \
	SND_SOC_DAPM_SWITCH_E("ECHO_FIFO SWITCH", SND_SOC_NOPM, 0, 0, echo_fifo_switch, \
		echo_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH("MIC3_BIAS1 SWITCH", SND_SOC_NOPM, 0, 0, mic3_bias1_switch), \
	SND_SOC_DAPM_SWITCH("MIC3_BIAS2 SWITCH", SND_SOC_NOPM, 0, 0, mic3_bias2_switch), \
	SND_SOC_DAPM_SWITCH("AUXMIC_BIAS1 SWITCH", SND_SOC_NOPM, 0, 0, auxmic_bias1_switch), \
	SND_SOC_DAPM_SWITCH("AUXMIC_BIAS2 SWITCH", SND_SOC_NOPM, 0, 0, auxmic_bias2_switch), \
	SND_SOC_DAPM_SWITCH("AUDIO_CARKIT_SWITCH", SND_SOC_NOPM, 0, 0, audio_carkit_switch), \

#define AUDIO_MIC_WIDGET \
	SND_SOC_DAPM_MIC("MICBIAS1 MIC", micbias1_mic_event), \
	SND_SOC_DAPM_MIC("MICBIAS2 MIC", micbias2_mic_event), \
	SND_SOC_DAPM_MIC("HPMICBIAS MIC", hpmicbias_mic_event), \
	SND_SOC_DAPM_MIC("DMIC1", dmic1_event), \
	SND_SOC_DAPM_MIC("DMIC2", dmic2_event), \

#define AUDIO_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC1 DRV", CODEC_ANA_RW3_REG, ADCL_MUTE_OFFSET, 1, NULL, 0, \
		adc_drv_power_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC2 DRV", CODEC_ANA_RW3_REG, ADCR_MUTE_OFFSET, 1, NULL, 0, \
		adc_drv_power_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC3 DRV", CODEC_ANA_RW3_REG, ADC3_MUTE_OFFSET, 1, NULL, 0, \
		adc_drv_power_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \

static const struct snd_soc_dapm_widget audio_dapm_widgets[] = {
	AUDIO_INPUT_OUTPUT_WIDGET
	AUDIO_PGA_WIDGET
	AUDIO_MIXER_WIDGET
	AUDIO_MUX_WIDGET
	AUDIO_SWITCH_WIDGET
	AUDIO_MIC_WIDGET
	AUDIO_DRV_WIDGET
};

#define ASP_AUDIO_PLAY \
	{ "AUDIO_DN PGA",         NULL,           "AUDIO_DLINK INPUT" }, \
	{ "DAC_DL MIXER",         "IN3_AUDIO_DL", "AUDIO_DN PGA" }, \
	{ "DAC_DR MIXER",         "IN3_AUDIO_DR", "AUDIO_DN PGA" }, \
	{ "DAC_DL MUX",           "DACL_MIXER",   "DAC_DL MIXER" }, \
	{ "DAC_DR MUX",           "DACR_MIXER",   "DAC_DR MIXER" }, \
	{ "DAC_DL_VIB MUX",       "SRC_ON",       "DAC_DL MUX" }, \
	{ "DAC_DL_VIB MUX",       "SRC_OFF",      "DAC_DL MUX" }, \
	{ "DACL_FILTER",          NULL,           "DAC_DL_VIB MUX" }, \
	{ "DACR_FILTER",          NULL,           "DAC_DR MUX" }, \
	{ "SDML",                 NULL,           "DACL_FILTER" }, \
	{ "AD_DA_LOOP MUX",       NULL,           "SDML" }, \
	{ "AD_DA_LOOP MUX",       "NORMAL",       "DACL_FILTER" }, \
	{ "AD_DA_LOOP MUX",       "NORMAL",       "DACR_FILTER" }, \
	{ "AD_DA_LOOP MUX",       "MIC3_DA_LOOP", "DACL_FILTER" }, \
	{ "AD_DA_LOOP MUX",       "MIC3_DA_LOOP", "DACR_FILTER" }, \
	{ "SIF_EN MIXER",         "SIF_DACL_EN",  "AD_DA_LOOP MUX" }, \
	{ "SIF_EN MIXER",         "SIF_DACR_EN",  "AD_DA_LOOP MUX" }, \
	{ "AUDIO_DLINK OUTPUT",   NULL,           "SIF_EN MIXER" }, \

#define PMU_AUDIO_PLAY_HEADPHONE \
	{ "ANA_SIF_EN MIXER",    "ANA_SIFL_EN",  "SMT_ANA_DAC_SDATA INPUT" }, \
	{ "ANA_SIF_EN MIXER",    "ANA_SIFR_EN",  "SMT_ANA_DAC_SDATA INPUT" }, \
	{ "ANA_DAC_L MUX",       "SIFL",         "ANA_SIF_EN MIXER" }, \
	{ "ANA_DAC_R MUX",       "SIFR",         "ANA_SIF_EN MIXER" }, \
	{ "ANA_DAC_L MUX",       "ADC1",         "ANA_MUX3 MUX" }, \
	{ "ANA_DAC_L MUX",       "ADC2",         "ANA_MUX4 MUX" }, \
	{ "ANA_DAC_L MUX",       "ADC3",         "ANA_MUX5 MUX" }, \
	{ "ANA_DAC_R MUX",       "ADC1",         "ANA_MUX3 MUX" }, \
	{ "ANA_DAC_R MUX",       "ADC2",         "ANA_MUX4 MUX" }, \
	{ "ANA_DAC_R MUX",       "ADC3",         "ANA_MUX5 MUX" }, \
	{ "ANA_DAC_L SWITCH",    "DACL",         "ANA_DAC_L MUX" }, \
	{ "ANA_DAC_R SWITCH",    "DACR",         "ANA_DAC_R MUX" }, \
	{ "ADC_LOOP MUX",        "NORMAL",       "ANA_DAC_L SWITCH" }, \
	{ "ADC_LOOP MUX",        "NORMAL",       "ANA_DAC_R SWITCH" }, \
	{ "HP_LR PGA",           NULL,           "ADC_LOOP MUX" }, \
	{ "ANA_HEADSET SWITCH",  "ENABLE",       "HP_LR PGA" }, \
	{ "SMT_HP_LR OUTPUT",    NULL,           "ANA_HEADSET SWITCH" }, \

#define PMU_AUDIO_PLAY_SPEAKER \
	{ "ANA_EAR MIXER",     "DACL",    "ADC_LOOP MUX" }, \
	{ "ANA_EAR MIXER",     "DACR",    "ADC_LOOP MUX" }, \
	{ "ANA_LINEOUT MIXER", "DACL",    "ADC_LOOP MUX" }, \
	{ "ANA_LINEOUT MIXER", "DACR",    "ADC_LOOP MUX" }, \
	{ "ANA_SPK SWITCH",    "ENABLE",  "ANA_LINEOUT MIXER" }, \
	{ "ANA_RCV SWITCH",    "ENABLE",  "ANA_EAR MIXER" }, \
	{ "ANA_LOUT SWITCH",   "ENABLE",  "ANA_LINEOUT MIXER" }, \
	{ "ANA_EAR OUTPUT",    NULL,      "ANA_SPK SWITCH" }, \
	{ "ANA_EAR OUTPUT",    NULL,      "ANA_RCV SWITCH" }, \
	{ "ANA_EAR OUTPUT",    NULL,      "ANA_LOUT SWITCH" }, \

#define ASP_AUDIO_CAP \
	{ "SIF_EN MIXER",          "SIF_ADCL_EN",  "ADC1 INPUT" }, \
	{ "SIF_EN MIXER",          "SIF_ADCR_EN",  "ADC2 INPUT" }, \
	{ "SIF_EN MIXER",          "SIF_ADC3_EN",  "ADC3 INPUT" }, \
	{ "DA_AD_LOOP MUX",        "NORMAL",       "SIF_EN MIXER" }, \
	{ "ADC1 MUX",              "ADC1",         "DA_AD_LOOP MUX" }, \
	{ "ADC1 MUX",              "ADC3",         "DA_AD_LOOP MUX" }, \
	{ "ADC2 MUX",              "ADC2",         "DA_AD_LOOP MUX" }, \
	{ "ADC2 MUX",              "ADC3",         "SIF_EN MIXER" }, \
	{ "ADCL_FILTER",           NULL,           "ADC1 MUX" }, \
	{ "ADCR_FILTER",           NULL,           "ADC2 MUX" }, \
	{ "AUDIO_UL MUX",          "ADCL",         "ADCL_FILTER" }, \
	{ "AUDIO_UR MUX",          "ADCR",         "ADCR_FILTER" }, \
	{ "AUDIO_UL MUX",          "DACL_MIXER",   "DAC_DL MUX" }, \
	{ "AUDIO_UR MUX",          "DACR_MIXER",   "DAC_DR MUX" }, \
	{ "AUDIO_UL_FIFO SWITCH",  "ENABLE",       "AUDIO_UL MUX" }, \
	{ "AUDIO_UR_FIFO SWITCH",  "ENABLE",       "AUDIO_UR MUX" }, \
	{ "AUDIO_UL OUTPUT",       NULL,           "AUDIO_UL_FIFO SWITCH" }, \
	{ "AUDIO_UR OUTPUT",       NULL,           "AUDIO_UR_FIFO SWITCH" }, \
	{ "ADC3 MUX",              "ADC3",         "SIF_EN MIXER" }, \
	{ "ADC3 MUX",              "ADC2",         "DA_AD_LOOP MUX" }, \
	{ "ADC3 MUX",              "ADC1",         "DA_AD_LOOP MUX" }, \
	{ "ADC3 PGA",              NULL,           "ADC3 MUX" }, \
	{ "ADC_MIC3_FILTER",       NULL,           "ADC3 PGA" }, \
	{ "MIC3_SRC9 MUX",         "BYPASS",       "ADC_MIC3_FILTER" }, \
	{ "MIC3_SRC9 MUX",         "SRC9",         "ADC_MIC3_FILTER" }, \
	{ "MIC3_UP MUX",           "ADC MIC3_SRC", "MIC3_SRC9 MUX" }, \
	{ "MIC3_UP_FIFO SWITCH",   "ENABLE",       "MIC3_UP MUX" }, \
	{ "MIC3_UP OUTPUT",        NULL,           "MIC3_UP_FIFO SWITCH" }, \

#define PMU_AUDIO_CAP \
	{ "MAINMIC INPUT",     NULL,           "MICBIAS1 MIC" }, \
	{ "AUXMIC_BIAS1 SWITCH", "ENABLE",     "MICBIAS1 MIC" }, \
	{ "AUXMIC INPUT",      NULL,           "AUXMIC_BIAS1 SWITCH" }, \
	{ "AUXMIC_BIAS2 SWITCH", "ENABLE",     "MICBIAS2 MIC" }, \
	{ "AUXMIC INPUT",      NULL,           "AUXMIC_BIAS2 SWITCH" }, \
	{ "HPMIC INPUT",       NULL,           "HPMICBIAS MIC" }, \
	{ "MIC3_BIAS2 SWITCH", "ENABLE",       "MICBIAS2 MIC" }, \
	{ "MIC3 INPUT",        NULL,           "MIC3_BIAS2 SWITCH" }, \
	{ "MIC3_BIAS1 SWITCH", "ENABLE",       "MICBIAS1 MIC" }, \
	{ "MIC3 INPUT",        NULL,           "MIC3_BIAS1 SWITCH" }, \
	{ "MAIN_HP_MIC MUX",   "MAIN_MIC",     "MAINMIC INPUT" }, \
	{ "ANA_MAIN PGA",      NULL,           "MAIN_HP_MIC MUX" }, \
	{ "MAIN_HP_MIC MUX",   "HP_MIC",       "HPMIC INPUT" }, \
	{ "ANA_AUX PGA",       NULL,           "AUXMIC INPUT" }, \
	{ "ANA_MIC3 PGA",      NULL,           "MIC3 INPUT" }, \
	{ "ANA_ADC1 MIXER",    "DACL",         "ADC_LOOP MUX" }, \
	{ "ANA_ADC1 MIXER",    "DACR",         "ADC_LOOP MUX" }, \
	{ "ANA_ADC1 MIXER",    "MAIN MIC",     "ANA_MAIN PGA" }, \
	{ "ANA_ADC1 MIXER",    "AUX MIC",      "ANA_AUX PGA" }, \
	{ "ANA_ADC1 MIXER",    "DACR",         "ANA_MIC3 PGA" }, \
	{ "ANA_ADC2 MIXER",    "DACL",         "ADC_LOOP MUX" }, \
	{ "ANA_ADC2 MIXER",    "DACR",         "ADC_LOOP MUX" }, \
	{ "ANA_ADC2 MIXER",    "MAIN MIC",     "ANA_MAIN PGA" }, \
	{ "ANA_ADC2 MIXER",    "AUX MIC",      "ANA_AUX PGA" }, \
	{ "ANA_ADC2 MIXER",    "DACL",         "ANA_MIC3 PGA" }, \
	{ "ANA_ADC1 DRV",      NULL,           "ANA_ADC1 MIXER" }, \
	{ "ANA_ADC2 DRV",      NULL,           "ANA_ADC2 MIXER" }, \
	{ "ANA_MUX3 MUX",      "ADC1",         "ANA_ADC1 DRV" }, \
	{ "ANA_MUX4 MUX",      "ADC2",         "ANA_ADC2 DRV" }, \
	{ "ANA_MUX3 MUX",      "ADC1",         "ADC_LOOP MUX" }, \
	{ "ANA_MUX4 MUX",      "ADC2",         "ADC_LOOP MUX" }, \
	{ "ANA_ADC3 MIXER",    "MIC3",         "ANA_MIC3 PGA" }, \
	{ "ANA_ADC3 MIXER",    "MAIN MIC",     "ANA_MAIN PGA" }, \
	{ "ANA_ADC3 MIXER",    "DACL",         "ANA_AUX PGA" }, \
	{ "ANA_ADC3 DRV",      NULL,           "ANA_ADC3 MIXER" }, \
	{ "ANA_MUX5 MUX",      "ADC3",         "ANA_ADC3 DRV" }, \
	{ "ANA_SIF_EN MIXER",  "ANA_MUX3_EN",  "ANA_MUX3 MUX" }, \
	{ "ANA_SIF_EN MIXER",  "ANA_MUX4_EN",  "ANA_MUX4 MUX" }, \
	{ "ANA_SIF_EN MIXER",  "ANA_MUX5_EN",  "ANA_MUX5 MUX" }, \
	{ "ANA_SIF OUTPUT",    NULL,           "ANA_SIF_EN MIXER" }, \

#define PLL \
	{ "DAC_DL MIXER",         NULL,   "PLL" }, \
	{ "DAC_DR MIXER",         NULL,   "PLL" }, \
	{ "AUDIO_UL_FIFO SWITCH", NULL,   "PLL" }, \
	{ "AUDIO_UR_FIFO SWITCH", NULL,   "PLL" }, \
	{ "MIC3_UP_FIFO SWITCH",  NULL,   "PLL" }, \
	{ "ECHO_FIFO SWITCH",     NULL,   "PLL" }, \
	{ "AUDIO_CARKIT_SWITCH",  NULL,   "PLL" }, \
	{ "ANA_SIF_EN MIXER",     NULL,   "SMT_IBIAS SUPPLY" }, \
	{ "ANA_SPK SWITCH",       NULL,   "CLASSD SUPPLY" }, \

#define ASP_OM \
	{ "AD_DA_LOOP MUX",  "AD_DA_LOOP",             "DACL_FILTER" }, \
	{ "AD_DA_LOOP MUX",  "AD_DA_LOOP",             "DACR_FILTER" }, \
	{ "DA_AD_LOOP MUX",  "DALR_ADLR_LOOP",         "SIF_EN MIXER" }, \
	{ "DA_AD_LOOP MUX",  "DA_SOUT_AD_SIN_LOOP",    "SIF_EN MIXER" }, \
	{ "DA_AD_LOOP MUX",  "DA_SOUT_MIC3_SIN_LOOP",  "SIF_EN MIXER" }, \

#define PMU_OM \
	{ "ADC_LOOP MUX",      "DA_AD_LOOP",    "ANA_DAC_L SWITCH" }, \
	{ "ADC_LOOP MUX",      "DA_AD_LOOP",    "ANA_DAC_R SWITCH" }, \
	{ "ADC_LOOP MUX",      "DA_MIC3_LOOP",  "ANA_DAC_L SWITCH" }, \
	{ "ADC_LOOP MUX",      "DA_MIC3_LOOP",  "ANA_DAC_R SWITCH" }, \
	{ "DAC_LOOPBACK MUX",  "MIC3_SDIN",     "ANA_MUX3 MUX" }, \
	{ "DAC_LOOPBACK MUX",  "MIC3_SDIN",     "ANA_MUX4 MUX" }, \
	{ "DAC_LOOPBACK MUX",  "MIC3_SDIN",     "ANA_MUX5 MUX" }, \
	{ "DAC_LOOPBACK MUX",  "DOUT_SDIN",     "ANA_MUX3 MUX" }, \
	{ "DAC_LOOPBACK MUX",  "DOUT_SDIN",     "ANA_MUX4 MUX" }, \
	{ "DAC_LOOPBACK MUX",  "DOUT_SDIN",     "ANA_MUX5 MUX" }, \
	{ "ANA_SIF_EN MIXER",  NULL,            "DAC_LOOPBACK MUX" }, \
	{ "ANA_SIF_EN MIXER",  NULL,            "DAC_LOOPBACK MUX" }, \

#define ASP_AUDIO_ECHO \
	{ "ECHO_SRC10 MUX",     "BYPASS",     "DAC_DL MIXER" }, \
	{ "ECHO_SRC11 MUX",     "BYPASS",     "DAC_DR MIXER" }, \
	{ "ECHO_SRC10 MUX",     "SRC10",      "DAC_DL MIXER" }, \
	{ "ECHO_SRC11 MUX",     "SRC11",      "DAC_DR MIXER" }, \
	{ "ECHO_L MUX",         "DACL_MIXER", "ECHO_SRC10 MUX" }, \
	{ "ECHO_R MUX",         "DACR_MIXER", "ECHO_SRC11 MUX" }, \
	{ "ECHO_FIFO SWITCH",   "ENABLE",     "ECHO_L MUX" }, \
	{ "ECHO_FIFO SWITCH",   "ENABLE",     "ECHO_R MUX" }, \
	{ "ECHO OUTPUT",        NULL,         "ECHO_FIFO SWITCH" }, \

#define DMIC_CAP \
	{ "DMIC1",         NULL,       "DMIC1 INPUT" }, \
	{ "DMIC2",         NULL,       "DMIC2 INPUT" }, \
	{ "ADC1 MUX",      "DMIC1_L",  "DMIC1" }, \
	{ "ADC2 MUX",      "DMIC1_R",  "DMIC1" }, \
	{ "ADC3 MUX",      "DMIC2_L",  "DMIC2" }, \
	{ "ADC_ULTR MUX",  "DMIC1_L",  "DMIC1" }, \
	{ "ADC_ULTR MUX",  "DMIC1_R",  "DMIC1" }, \
	{ "ADC_ULTR MUX",  "DMIC2_L",  "DMIC2" }, \

#define SIDE_TONE_AUDIO \
	{ "SIDE_TONE MUX",    "ADCL_I",            "ADCL_FILTER" }, \
	{ "SIDE_TONE MUX",    "ADCR_I",            "ADCL_FILTER" }, \
	{ "SIDE_TONE MUX",    "ADC_MIC3_I",        "ADCL_FILTER" }, \
	{ "SIDE_TONE_EN PGA", NULL,                "SIDE_TONE MUX" }, \
	{ "DAC_DL MIXER",     "IN4_SIDE_TONE_L",   "SIDE_TONE_EN PGA" }, \
	{ "DAC_DR MIXER",     "IN4_SIDE_TONE_R",   "SIDE_TONE_EN PGA" }, \

#define AUDIO_CARKIT \
	{ "AUDIO_DN PGA",              NULL,      "AUDIO_CARKIT_DLINK INPUT" }, \
	{ "AUDIO_CARKIT_SWITCH",       "ENABLE",  "AUDIO_DN PGA" }, \
	{ "AUDIO_CARKIT_DLINK OUTPUT", NULL,      "AUDIO_CARKIT_SWITCH" }, \

static const struct snd_soc_dapm_route audio_route_map[] = {
	ASP_AUDIO_PLAY
	PMU_AUDIO_PLAY_HEADPHONE
	PMU_AUDIO_PLAY_SPEAKER
	ASP_AUDIO_CAP
	PMU_AUDIO_CAP
	PLL
	ASP_OM
	PMU_OM
	ASP_AUDIO_ECHO
	DMIC_CAP
	SIDE_TONE_AUDIO
	AUDIO_CARKIT
};

static struct snd_codec_resource audio_resource = {
	audio_controls,
	ARRAY_SIZE(audio_controls),
	audio_dapm_widgets,
	ARRAY_SIZE(audio_dapm_widgets),
	audio_route_map,
	ARRAY_SIZE(audio_route_map),
};

int add_audio(struct snd_soc_component *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	return da_separate_add_codec_resource(codec, &audio_resource);
}

