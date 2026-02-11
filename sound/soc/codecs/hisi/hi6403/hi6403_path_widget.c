/*
 * hi6403_path_widget.c
 *
 * path widget for codec driver
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

#include "hi6403_path_widget.h"

#include <sound/core.h>

#include <linux/hisi/audio_log.h>

#include "linux/hisi/hi64xx/hi6403_regs.h"
#include "linux/hisi/hi64xx/hi64xx_utils.h"

#define INPUT_WIDGET \
	SND_SOC_DAPM_INPUT("LINEINL"), \
	SND_SOC_DAPM_INPUT("LINEINR"), \
	SND_SOC_DAPM_INPUT("LINEOUTL LOOP"), \
	SND_SOC_DAPM_INPUT("LINEOUTR LOOP"), \
	SND_SOC_DAPM_INPUT("HSMIC"), \
	SND_SOC_DAPM_INPUT("MAINMIC"), \
	SND_SOC_DAPM_INPUT("AUXMIC"), \
	SND_SOC_DAPM_INPUT("MIC3"), \
	SND_SOC_DAPM_INPUT("MIC4"), \
	SND_SOC_DAPM_INPUT("S1L IN"), \
	SND_SOC_DAPM_INPUT("S1R IN"), \
	SND_SOC_DAPM_INPUT("S2L IN"), \
	SND_SOC_DAPM_INPUT("S2R IN"), \
	SND_SOC_DAPM_INPUT("S3L IN"), \
	SND_SOC_DAPM_INPUT("S3R IN"), \
	SND_SOC_DAPM_INPUT("S4L IN"), \
	SND_SOC_DAPM_INPUT("S4R IN"), \
	SND_SOC_DAPM_INPUT("DIGMIC0"), \
	SND_SOC_DAPM_INPUT("DIGMIC1"), \
	SND_SOC_DAPM_INPUT("SLIM UP IN"), \
	SND_SOC_DAPM_INPUT("ECL IN"), \
	SND_SOC_DAPM_INPUT("ECR IN"), \
	SND_SOC_DAPM_INPUT("AUDIO IN"), \
	SND_SOC_DAPM_INPUT("VOICE IN"), \
	SND_SOC_DAPM_INPUT("HP HIGH IN"), \
	SND_SOC_DAPM_INPUT("IR TX IN"), \
	SND_SOC_DAPM_INPUT("I2S2 BLUETOOTH LOOP IN"), \
	SND_SOC_DAPM_INPUT("PLL IN"), \
	SND_SOC_DAPM_INPUT("ANC CORE IN"), \

#define OUTPUT_WIDGET \
	SND_SOC_DAPM_OUTPUT("EAR OUT"), \
	SND_SOC_DAPM_OUTPUT("HPL OUT"), \
	SND_SOC_DAPM_OUTPUT("HPR OUT"), \
	SND_SOC_DAPM_OUTPUT("LINEOUT L"), \
	SND_SOC_DAPM_OUTPUT("LINEOUT R"), \
	SND_SOC_DAPM_OUTPUT("S1L OUT"), \
	SND_SOC_DAPM_OUTPUT("S1R OUT"), \
	SND_SOC_DAPM_OUTPUT("S2L OUT"), \
	SND_SOC_DAPM_OUTPUT("S2R OUT"), \
	SND_SOC_DAPM_OUTPUT("S3L OUT"), \
	SND_SOC_DAPM_OUTPUT("S3R OUT"), \
	SND_SOC_DAPM_OUTPUT("S4L OUT"), \
	SND_SOC_DAPM_OUTPUT("S4R OUT"), \
	SND_SOC_DAPM_OUTPUT("U3L OUT"), \
	SND_SOC_DAPM_OUTPUT("U4R OUT"), \
	SND_SOC_DAPM_OUTPUT("ECL OUT"), \
	SND_SOC_DAPM_OUTPUT("ECR OUT"), \
	SND_SOC_DAPM_OUTPUT("SLIM UP OUT"), \
	SND_SOC_DAPM_OUTPUT("MAD OUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO OUT"), \
	SND_SOC_DAPM_OUTPUT("VOICE OUT"), \
	SND_SOC_DAPM_OUTPUT("HP HIGH OUT"), \
	SND_SOC_DAPM_OUTPUT("IR RX8 OUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2 BLUETOOTH LOOP OUT"), \
	SND_SOC_DAPM_OUTPUT("PLL OUT"), \
	SND_SOC_DAPM_OUTPUT("ANC CORE OUT"), \

/* DACL MIXER */
static const struct snd_kcontrol_new dacl_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1L SWITCH",
		HI6403_DACL_MIXER_SEL_0, HI6403_DACL_MIXER_S1L_BIT, 1, 1),
	SOC_DAPM_SINGLE("S2L SWITCH",
		HI6403_DACL_MIXER_SEL_0, HI6403_DACL_MIXER_S2L_BIT, 1, 1),
	SOC_DAPM_SINGLE("MDM SWITCH",
		HI6403_DACL_MIXER_SEL_0, HI6403_DACL_MIXER_MDM_BIT, 1, 1),
	SOC_DAPM_SINGLE("S1R SWITCH",
		HI6403_DACL_MIXER_SEL_1, HI6403_DACL_MIXER_S1R_BIT, 1, 1),
};

/* DACR MIXER */
static const struct snd_kcontrol_new dacr_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1R SWITCH",
		HI6403_DACR_MIXER_SEL_0, HI6403_DACR_MIXER_S1R_BIT, 1, 1),
	SOC_DAPM_SINGLE("S2R SWITCH",
		HI6403_DACR_MIXER_SEL_0, HI6403_DACR_MIXER_S2R_BIT, 1, 1),
	SOC_DAPM_SINGLE("MDM SWITCH",
		HI6403_DACR_MIXER_SEL_0, HI6403_DACR_MIXER_MDM_BIT, 1, 1),
	SOC_DAPM_SINGLE("S1L SWITCH",
		HI6403_DACR_MIXER_SEL_1, HI6403_DACR_MIXER_S1L_BIT, 1, 1),
};

/* DACSL MIXER */
static const struct snd_kcontrol_new dacsl_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACL SWITCH",
		HI6403_DACSL_MIXER_SEL_0, HI6403_DACSL_MIXER_DACL_BIT, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE SWITCH",
		HI6403_DACSL_MIXER_SEL_1,
		HI6403_DACSL_MIXER_SIDETONE_BIT, 1, 1),
	SOC_DAPM_SINGLE("DSP IF8 OL SWITCH",
		HI6403_DACSL_MIXER_SEL_0, HI6403_DACSL_MIXER_OL_BIT, 1, 1),
	SOC_DAPM_SINGLE("MDM SWITCH",
		HI6403_DACSL_MIXER_SEL_0, HI6403_DACSL_MIXER_MDM_BIT, 1, 1),
};

/* DACSR MIXER */
static const struct snd_kcontrol_new dacsr_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACR SWITCH",
		HI6403_DACSR_MIXER_SEL_0, HI6403_DACSR_MIXER_DACR_BIT, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE SWITCH",
		HI6403_DACSR_MIXER_SEL_1,
		HI6403_DACSR_MIXER_SIDETONE_BIT, 1, 1),
	SOC_DAPM_SINGLE("DSP IF8 OR SWITCH",
		HI6403_DACSR_MIXER_SEL_0, HI6403_DACSR_MIXER_OR_BIT, 1, 1),
	SOC_DAPM_SINGLE("MDM SWITCH",
		HI6403_DACSR_MIXER_SEL_0, HI6403_DACSR_MIXER_MDM_BIT, 1, 1),
};

/* DACL SRC MIXER */
static const struct snd_kcontrol_new dacl_src_mixer_controls[] = {
	SOC_DAPM_SINGLE("DSP IF8 SWITCH",
		HI6403_DACSL_SRC_MIXER_SEL, HI6403_DACSL_MIXER_0_BIT, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE SWITCH",
		HI6403_DACSL_SRC_MIXER_SEL, HI6403_DACSL_MIXER_1_BIT, 1, 1),
};

/* DACR SRC MIXER */
static const struct snd_kcontrol_new dacr_src_mixer_controls[] = {
	SOC_DAPM_SINGLE("SIDETONE SWITCH",
		HI6403_DACSR_SRC_MIXER_SEL, HI6403_DACSR_MIXER_1_BIT, 1, 1),
	SOC_DAPM_SINGLE("DSP IF8 SWITCH",
		HI6403_DACSR_SRC_MIXER_SEL, HI6403_DACSR_MIXER_0_BIT, 1, 1),
};

/* DACL FLT MIXER */
static const struct snd_kcontrol_new dacl_flt_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACL SRC SWITCH",
		HI6403_DACL_FLT_MIXER_SEL, HI6403_DACL_FLT_MIXER_0_BIT, 1, 1),
	SOC_DAPM_SINGLE("S1L SWITCH",
		HI6403_DACL_FLT_MIXER_SEL, HI6403_DACL_FLT_MIXER_1_BIT, 1, 1),
};

/* DACR FLT MIXER */
static const struct snd_kcontrol_new dacr_flt_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1R SWITCH",
		HI6403_DACR_FLT_MIXER_SEL, HI6403_DACR_FLT_MIXER_1_BIT, 1, 1),
	SOC_DAPM_SINGLE("DACR SRC SWITCH",
		HI6403_DACR_FLT_MIXER_SEL, HI6403_DACR_FLT_MIXER_0_BIT, 1, 1),
};

/* MADPGA MIXER  */
static const struct snd_kcontrol_new mad_pga_mixer_controls[] = {
	SOC_DAPM_SINGLE("MIC4 SWITCH",
		HI6403_ANALOG_REG83, HI6403_MAD_MIC4_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC3 SWITCH",
		HI6403_ANALOG_REG83, HI6403_MAD_MIC3_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("AUXMIC SWITCH",
		HI6403_ANALOG_REG83, HI6403_MAD_AUXMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MAINMIC SWITCH",
		HI6403_ANALOG_REG83, HI6403_MAD_MAINMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("HPMIC SWITCH",
		HI6403_ANALOG_REG84, HI6403_MAD_HPMIC_SEL_BIT, 1, 0),
};

#define MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("DACL MIXER", \
		HI6403_DAC_MIXER_CLK_REG, HI6403_DACL_MIXER_EN_BIT, 0, \
		dacl_mixer_controls, \
		ARRAY_SIZE(dacl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR MIXER", \
		HI6403_DAC_MIXER_CLK_REG, HI6403_DACR_MIXER_EN_BIT, 0, \
		dacr_mixer_controls, \
		ARRAY_SIZE(dacr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL SRC MIXER", \
		HI6403_DAC_SRC_MIXER_CLK_REG, HI6403_DACL_SRC_MIXER_EN_BIT, 0, \
		dacl_src_mixer_controls, \
		ARRAY_SIZE(dacl_src_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR SRC MIXER", \
		HI6403_DAC_SRC_MIXER_CLK_REG, HI6403_DACR_SRC_MIXER_EN_BIT, 0, \
		dacr_src_mixer_controls, \
		ARRAY_SIZE(dacr_src_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACSL MIXER", \
		HI6403_DAC_MIXER_CLK_REG, HI6403_DACSL_MIXER_EN_BIT, 0, \
		dacsl_mixer_controls, \
		ARRAY_SIZE(dacsl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACSR MIXER", \
		HI6403_DAC_MIXER_CLK_REG, HI6403_DACSR_MIXER_EN_BIT, 0, \
		dacsr_mixer_controls, \
		ARRAY_SIZE(dacsr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL FLT MIXER", \
		HI6403_DAC_SRC_MIXER_CLK_REG, HI6403_DACL_FLT_MIXER_EN_BIT, 0, \
		dacl_flt_mixer_controls, \
		ARRAY_SIZE(dacl_flt_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR FLT MIXER", \
		HI6403_DAC_SRC_MIXER_CLK_REG, HI6403_DACR_FLT_MIXER_EN_BIT, 0, \
		dacr_flt_mixer_controls, \
		ARRAY_SIZE(dacr_flt_mixer_controls)), \
	SND_SOC_DAPM_MIXER("MAD PGA MIXER", \
		SND_SOC_NOPM, 0, 0, \
		mad_pga_mixer_controls, \
		ARRAY_SIZE(mad_pga_mixer_controls)), \

/* MUX CONTROL */
/* MUX OF DACL SRC IN 0x7215 */
static const char * const dacl_src_mux_texts[] = {
	"DSP IF8 OL",
	"DACL",
};
static const struct soc_enum dacl_src_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DAC_SRC_MUX_SEL, HI6403_DACL_SRC_MUX_BIT,
		ARRAY_SIZE(dacl_src_mux_texts), dacl_src_mux_texts);
static const struct snd_kcontrol_new dapm_dacl_src_mux_controls =
	SOC_DAPM_ENUM("Mux", dacl_src_mux_enum);

/* MUX OF DACR SRC IN 0x7215 */
static const char * const dacr_src_mux_texts[] = {
	"DSP IF8 OR",
	"DACR",
};
static const struct soc_enum dacr_src_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DAC_SRC_MUX_SEL, HI6403_DACR_SRC_MUX_BIT,
		ARRAY_SIZE(dacr_src_mux_texts), dacr_src_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_src_mux_controls =
	SOC_DAPM_ENUM("Mux", dacr_src_mux_enum);

/* MUX OF DACL PGA IN 0x7289 */
static const char * const dacl_pga_mux_texts[] = {
	"DACL_384",
	"ADC1L",
	"HP RES",
};
static const struct soc_enum dacl_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACL_PGA_MUX_SEL, HI6403_DAC_PGA_MUX_BIT,
		ARRAY_SIZE(dacl_pga_mux_texts), dacl_pga_mux_texts);
static const struct snd_kcontrol_new dapm_dacl_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", dacl_pga_mux_enum);

/* MUX OF DACR PGA IN 0x728a */
static const char * const dacr_pga_mux_texts[] = {
	"DACR_384",
	"ADC1R",
	"HP RES",
};
static const struct soc_enum dacr_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACR_PGA_MUX_SEL, HI6403_DAC_PGA_MUX_BIT,
		ARRAY_SIZE(dacr_pga_mux_texts), dacr_pga_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", dacr_pga_mux_enum);

/* MUX FOR DACL SDM45 0x733c */
static const char * const dacl_sdm45_mux_texts[] = {
	"L UP",
	"DSDL",
	"IO TEST",
	"ANC",
};
static const struct soc_enum dacl_sdm45_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACL_SDM45_MUX_SEL, HI6403_DAC_SDM45_MUX_BIT,
		ARRAY_SIZE(dacl_sdm45_mux_texts), dacl_sdm45_mux_texts);
static const struct snd_kcontrol_new dapm_dacl_sdm45_mux_controls =
	SOC_DAPM_ENUM("Mux", dacl_sdm45_mux_enum);

/* MUX FOR DACR SDM45 0x7340 */
static const char * const dacr_sdm45_mux_texts[] = {
	"R UP",
	"DSDR",
	"IO TEST",
	"ANC",
};
static const struct soc_enum dacr_sdm45_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACR_SDM45_MUX_SEL, HI6403_DAC_SDM45_MUX_BIT,
		ARRAY_SIZE(dacr_sdm45_mux_texts), dacr_sdm45_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_sdm45_mux_controls =
	SOC_DAPM_ENUM("Mux", dacr_sdm45_mux_enum);

/* ADCL0 MUX 0x7294 */
static const char * const adc_mux_texts[] = {
	"MAINMIC",
	"AUXMIC",
	"LINEINL",
	"LINEINR",
	"DMIC0L",
	"DMIC0R",
	"DMIC1L",
	"DMIC1R",
	"MAD",
};
static const struct soc_enum adcl0_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ADC0_MUX_SEL, HI6403_ADCL0_MUX_BIT,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);
static const struct snd_kcontrol_new dapm_adcl0_mux_controls =
	SOC_DAPM_ENUM("Mux", adcl0_mux_enum);

/* ADCR0 MUX 0x7294 */
static const struct soc_enum adcr0_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ADC0_MUX_SEL, HI6403_ADCR0_MUX_BIT,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);
static const struct snd_kcontrol_new dapm_adcr0_mux_controls =
	SOC_DAPM_ENUM("Mux", adcr0_mux_enum);

/* ADCL1 MUX 0x7295 */
static const struct soc_enum adcl1_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ADC1_MUX_SEL, HI6403_ADCL1_MUX_BIT,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);
static const struct snd_kcontrol_new dapm_adcl1_mux_controls =
	SOC_DAPM_ENUM("Mux", adcl1_mux_enum);

/* ADCR0 MUX 0x7295 */
static const struct soc_enum adcr1_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ADC1_MUX_SEL, HI6403_ADCR1_MUX_BIT,
		ARRAY_SIZE(adc_mux_texts), adc_mux_texts);
static const struct snd_kcontrol_new dapm_adcr1_mux_controls =
	SOC_DAPM_ENUM("Mux", adcr1_mux_enum);

/* MIC1 MUX 0x7208 */
static const char * const mic1_mux_texts[] = {
	"ADC0L",
	"ADC1L",
	"DACL_48",
	"S2L",
	"S3L",
	"S4L",
	"ADC0R",
	"ADC1R",
};
static const struct soc_enum mic1_mux_enum =
	SOC_ENUM_SINGLE(HI6403_MIC12_MUX_SEL, HI6403_MIC1_MUX_BIT,
		ARRAY_SIZE(mic1_mux_texts), mic1_mux_texts);
static const struct snd_kcontrol_new dapm_mic1_mux_controls =
	SOC_DAPM_ENUM("Mux", mic1_mux_enum);

/* MIC2 MUX 0x7208 */
static const char * const mic2_mux_texts[] = {
	"ADC0R",
	"ADC1R",
	"DACR_48",
	"S2R",
	"S3R",
	"S4R",
	"ADC0L",
	"ADC1L",
};
static const struct soc_enum mic2_mux_enum =
	SOC_ENUM_SINGLE(HI6403_MIC12_MUX_SEL, HI6403_MIC2_MUX_BIT,
		ARRAY_SIZE(mic2_mux_texts), mic2_mux_texts);
static const struct snd_kcontrol_new dapm_mic2_mux_controls =
	SOC_DAPM_ENUM("Mux", mic2_mux_enum);

/* MIC3 MUX 0x7209 */
static const char * const mic3_mux_texts[] = {
	"ADC0L",
	"ADC1L",
	"DACL_48",
	"S1L",
	"S2L",
	"S4L",
	"ADC0R",
	"ADC1R",
};
static const struct soc_enum mic3_mux_enum =
	SOC_ENUM_SINGLE(HI6403_MIC34_MUX_SEL, HI6403_MIC3_MUX_BIT,
		ARRAY_SIZE(mic3_mux_texts), mic3_mux_texts);
static const struct snd_kcontrol_new dapm_mic3_mux_controls =
	SOC_DAPM_ENUM("Mux", mic3_mux_enum);

/* MIC4 MUX 0x7209 */
static const char * const mic4_mux_texts[] = {
	"ADC0R",
	"ADC1R",
	"DACR_48",
	"S1R",
	"S2R",
	"S4R",
	"ADC0L",
	"ADC1L",
};
static const struct soc_enum mic4_mux_enum =
	SOC_ENUM_SINGLE(HI6403_MIC34_MUX_SEL, HI6403_MIC4_MUX_BIT,
		ARRAY_SIZE(mic4_mux_texts), mic4_mux_texts);
static const struct snd_kcontrol_new dapm_mic4_mux_controls =
	SOC_DAPM_ENUM("Mux", mic4_mux_enum);

/* BTL MUX 0x720A */
static const char * const btl_mux_texts[] = {
	"ADC0L",
	"ADC1L",
	"DACL_48",
	"S1L",
	"S3L",
	"S4L",
};
static const struct soc_enum btl_mux_enum =
	SOC_ENUM_SINGLE(HI6403_BT_MUX_SEL, HI6403_BTL_MUX_BIT,
		ARRAY_SIZE(btl_mux_texts), btl_mux_texts);
static const struct snd_kcontrol_new dapm_btl_mux_controls =
	SOC_DAPM_ENUM("Mux", btl_mux_enum);

/* BTR MUX 0x720A */
static const char * const btr_mux_texts[] = {
	"ADC0R",
	"ADC1R",
	"DACR_48",
	"S1R",
	"S3R",
	"S4R",
};
static const struct soc_enum btr_mux_enum =
	SOC_ENUM_SINGLE(HI6403_BT_MUX_SEL, HI6403_BTR_MUX_BIT,
		ARRAY_SIZE(btr_mux_texts), btr_mux_texts);
static const struct snd_kcontrol_new dapm_btr_mux_controls =
	SOC_DAPM_ENUM("Mux", btr_mux_enum);

/* M1L MUX 0x720B */
static const char * const m1l_mux_texts[] = {
	"ADC0L",
	"ADC1L",
	"DACSL",
	"DACL_48",
	"S1L",
	"S2L",
	"S3L",
};
static const struct soc_enum m1l_mux_enum =
	SOC_ENUM_SINGLE(HI6403_M1_MUX_SEL, HI6403_M1L_MUX_BIT,
		ARRAY_SIZE(m1l_mux_texts), m1l_mux_texts);
static const struct snd_kcontrol_new dapm_m1l_mux_controls =
	SOC_DAPM_ENUM("Mux", m1l_mux_enum);

/* M1R MUX 0x720B */
static const char * const m1r_mux_texts[] = {
	"ADC0R",
	"ADC1R",
	"DACSR",
	"DACR_48",
	"S1R",
	"S2R",
	"S3R",
};
static const struct soc_enum m1r_mux_enum =
	SOC_ENUM_SINGLE(HI6403_M1_MUX_SEL, HI6403_M1R_MUX_BIT,
		ARRAY_SIZE(m1r_mux_texts), m1r_mux_texts);
static const struct snd_kcontrol_new dapm_m1r_mux_controls =
	SOC_DAPM_ENUM("Mux", m1r_mux_enum);

/* MDM MUX 0x7212 */
static const char * const mdm_mux_texts[] = {
	"S3L",
	"S4L",
};
static const struct soc_enum mdm_mux_enum =
	SOC_ENUM_SINGLE(HI6403_MDM_MUX_SEL, HI6403_MDM_MUX_BIT,
		ARRAY_SIZE(mdm_mux_texts), mdm_mux_texts);
static const struct snd_kcontrol_new dapm_mdm_mux_controls =
	SOC_DAPM_ENUM("Mux", mdm_mux_enum);

/* SIDETONE MUX 0x7212 */
static const char * const sidetone_mux_texts[] = {
	"S1L",
	"S2L",
	"ADC0L",
	"ADC0R",
	"ADC1L",
	"ADC1R",
};
static const struct soc_enum sidetone_mux_enum =
	SOC_ENUM_SINGLE(HI6403_SIDETONE_MUX_SEL, HI6403_SIDETONE_MUX_BIT,
		ARRAY_SIZE(sidetone_mux_texts), sidetone_mux_texts);
static const struct snd_kcontrol_new dapm_sidetone_mux_controls =
	SOC_DAPM_ENUM("Mux", sidetone_mux_enum);

/* NEARMIC MUX 0x7484 */
static const char * const nearmic_mux_texts[] = {
	"ADC0L",
	"ADC1L",
	"ADC0R",
	"ADC1R",
};
static const struct soc_enum nearmic_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANC_MUX_SEL, HI6403_NEARMIC_MUX_BIT,
		ARRAY_SIZE(nearmic_mux_texts), nearmic_mux_texts);
static const struct snd_kcontrol_new dapm_nearmic_mux_controls =
	SOC_DAPM_ENUM("Mux", nearmic_mux_enum);

/*PDM MUX 0x7297 */
static const char * const pdm_mux_texts[] = {
	"DACL_48/DACR_48",
	"DACL_MIXER/DRCR_MIXER",
};
static const struct soc_enum pdm_mux_enum =
	SOC_ENUM_SINGLE(HI6403_PDM_MUX_SEL, HI6403_PDM_MUX_BIT,
		ARRAY_SIZE(pdm_mux_texts), pdm_mux_texts);
static const struct snd_kcontrol_new dapm_pdm_mux_controls =
	SOC_DAPM_ENUM("Mux", pdm_mux_enum);

/*DACR SDM MUX 0x7342 */
static const char * const dacr_sdm_mux_texts[] = {
	"R UP",
	"ANC",
	"IO TEST",
};
static const struct soc_enum dacr_sdm_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACR_SDM_MUX_SEL, HI6403_DACR_SDM_MUX_BIT,
		ARRAY_SIZE(dacr_sdm_mux_texts), dacr_sdm_mux_texts);
static const struct snd_kcontrol_new dapm_dacr_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dacr_sdm_mux_enum);

/*DACSR SDM MUX 0x7344 */
static const char * const dacsr_sdm_mux_texts[] = {
	"SR UP",
	"ANC",
	"IO TEST",
};
static const struct soc_enum dacsr_sdm_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACSR_SDM_MUX_SEL, HI6403_DACSR_SDM_MUX_BIT,
		ARRAY_SIZE(dacsr_sdm_mux_texts), dacsr_sdm_mux_texts);
static const struct snd_kcontrol_new dapm_dacsr_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", dacsr_sdm_mux_enum);

/*DAC HP MUX 0x7349 */
static const char * const dac_hpl_mux_texts[] = {
	"DACL SDM45",
	"DACR SDM45",
	"HP_SDM_MUX",
	"Off",
};
static const struct soc_enum dac_hpl_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DAC_HP_MUX_SEL, HI6403_DAC_HPL_MUX_BIT,
		ARRAY_SIZE(dac_hpl_mux_texts), dac_hpl_mux_texts);
static const struct snd_kcontrol_new dapm_dac_hpl_mux_controls =
	SOC_DAPM_ENUM("Mux", dac_hpl_mux_enum);

static const char * const dac_hpr_mux_texts[] = {
	"DACR SDM45",
	"DACL SDM45",
	"HP_SDM_MUX",
	"Off",
};
static const struct soc_enum dac_hpr_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DAC_HP_MUX_SEL, HI6403_DAC_HPR_MUX_BIT,
		ARRAY_SIZE(dac_hpr_mux_texts), dac_hpr_mux_texts);
static const struct snd_kcontrol_new dapm_dac_hpr_mux_controls =
	SOC_DAPM_ENUM("Mux", dac_hpr_mux_enum);

/*DACS MUX 0x734a */
static const char * const dacs_mux_texts[] = {
	"DACL SDM",
	"DACSL",
	"DACR SDM",
	"DACSR",
	"EP_SDM_MUX",
	"DSP_IF8",
	"Off",
};
static const struct soc_enum dac_ep_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACS_MUX_SEL, HI6403_DAC_EP_MUX_BIT,
		ARRAY_SIZE(dacs_mux_texts), dacs_mux_texts);
static const struct snd_kcontrol_new dapm_dac_ep_mux_controls =
	SOC_DAPM_ENUM("Mux", dac_ep_mux_enum);

/*DAC LINEOUT L MUX */
static const struct soc_enum dac_ltl_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DACS_MUX_SEL, HI6403_DAC_LTL_MUX_BIT,
		ARRAY_SIZE(dacs_mux_texts), dacs_mux_texts);
static const struct snd_kcontrol_new dapm_dac_ltl_mux_controls =
	SOC_DAPM_ENUM("Mux", dac_ltl_mux_enum);

/*DAC LINEOUT R MUX 0x734b */
static const struct soc_enum dac_ltr_mux_enum =
	SOC_ENUM_SINGLE(HI6403_DAC_LTR_MUX_SEL, HI6403_DAC_LTR_MUX_BIT,
		ARRAY_SIZE(dacs_mux_texts), dacs_mux_texts);
static const struct snd_kcontrol_new dapm_dac_ltr_mux_controls =
	SOC_DAPM_ENUM("Mux", dac_ltr_mux_enum);

/*ANC DATA MUX 0x7484 */
static const char * const anc_farend_mux_texts[] = {
	"DACL_384",
	"DACR_384",
	"DACSL_384",
	"DACSR_384",
};
static const struct soc_enum anc_farend_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANC_MUX_SEL, HI6403_ANC_FAREND_MUX_BIT,
		ARRAY_SIZE(anc_farend_mux_texts), anc_farend_mux_texts);
static const struct snd_kcontrol_new dapm_anc_farend_mux_controls =
	SOC_DAPM_ENUM("Mux", anc_farend_mux_enum);

/*ERROR MIC MUX 0x7484 */
static const char * const exp_mic_mux_texts[] = {
	"L0",
	"R0",
	"L1",
	"R1",
};
static const struct soc_enum error_mic_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANC_MUX_SEL, HI6403_ERROR_MIC_MUX_BIT,
		ARRAY_SIZE(exp_mic_mux_texts), exp_mic_mux_texts);
static const struct snd_kcontrol_new dapm_error_mic_mux_controls =
	SOC_DAPM_ENUM("Mux", error_mic_mux_enum);

/*REF MIC MUX 0x7484 */
static const struct soc_enum ref_mic_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANC_MUX_SEL, HI6403_REF_MIC_MUX_BIT,
		ARRAY_SIZE(exp_mic_mux_texts), exp_mic_mux_texts);
static const struct snd_kcontrol_new dapm_ref_mic_mux_controls =
	SOC_DAPM_ENUM("Mux", ref_mic_mux_enum);

/*DSP IF 8 TEST MUX 0x7211 */
static const char * const if8_test_mux_texts[] = {
	"DAC MIXER",
	"ADC",
};
static const struct soc_enum if8_test_mux_enum =
	SOC_ENUM_SINGLE(HI6403_IF8_TEST_MUX_SEL, HI6403_IF8_TEST_MUX_BIT,
		ARRAY_SIZE(if8_test_mux_texts), if8_test_mux_texts);
static const struct snd_kcontrol_new dapm_if8_test_mux_controls =
	SOC_DAPM_ENUM("Mux", if8_test_mux_enum);

/* LINEIN L MUX */
static const char * const lineinl_mux_texts[] = {
	"LINEIN1",
	"LINEIN2",
	"LINEOUT",
	"MIC3",
};
static const struct soc_enum lineinl_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANALOG_REG20, HI6403_LINEINL_SEL_BIT,
		ARRAY_SIZE(lineinl_mux_texts), lineinl_mux_texts);
static const struct snd_kcontrol_new dapm_lineinl_mux_controls =
	SOC_DAPM_ENUM("Mux", lineinl_mux_enum);

/* LINEIN R MUX */
static const char * const lineinr_mux_texts[] = {
	"LINEIN1",
	"LINEIN2",
	"LINEOUT",
	"MIC4",
};
static const struct soc_enum lineinr_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANALOG_REG21, HI6403_LINEINR_SEL_BIT,
		ARRAY_SIZE(lineinr_mux_texts), lineinr_mux_texts);
static const struct snd_kcontrol_new dapm_lineinr_mux_controls =
	SOC_DAPM_ENUM("Mux", lineinr_mux_enum);

/* MAINMIC MUX */
static const char * const mainmic_mux_texts[] = {
	"HSMIC",
	"MAINMIC",
};
static const struct soc_enum mainmic_mux_enum =
	SOC_ENUM_SINGLE(HI6403_ANALOG_REG23, HI6403_MAINMIC_SEL_BIT,
		ARRAY_SIZE(mainmic_mux_texts), mainmic_mux_texts);
static const struct snd_kcontrol_new dapm_mainmic_mux_controls =
	SOC_DAPM_ENUM("Mux", mainmic_mux_enum);

#define MUX_WIDGET \
	SND_SOC_DAPM_MUX("DACL SRC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacl_src_mux_controls), \
	SND_SOC_DAPM_MUX("DACR SRC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_src_mux_controls), \
	SND_SOC_DAPM_MUX("DACL PGA MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacl_pga_mux_controls), \
	SND_SOC_DAPM_MUX("DACR PGA MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_pga_mux_controls), \
	SND_SOC_DAPM_MUX("DACL SDM45 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacl_sdm45_mux_controls), \
	SND_SOC_DAPM_MUX("DACR SDM45 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_sdm45_mux_controls), \
	SND_SOC_DAPM_MUX("ADC0L MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcl0_mux_controls), \
	SND_SOC_DAPM_MUX("ADC0R MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcr0_mux_controls), \
	SND_SOC_DAPM_MUX("ADC1L MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcl1_mux_controls), \
	SND_SOC_DAPM_MUX("ADC1R MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_adcr1_mux_controls), \
	SND_SOC_DAPM_MUX("MIC1 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic1_mux_controls), \
	SND_SOC_DAPM_MUX("MIC2 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic2_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic3_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4 MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mic4_mux_controls), \
	SND_SOC_DAPM_MUX("BTL MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_btl_mux_controls), \
	SND_SOC_DAPM_MUX("BTR MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_btr_mux_controls), \
	SND_SOC_DAPM_MUX("M1L MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_m1l_mux_controls), \
	SND_SOC_DAPM_MUX("M1R MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_m1r_mux_controls), \
	SND_SOC_DAPM_MUX("MDM MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mdm_mux_controls), \
	SND_SOC_DAPM_MUX("SIDETONE MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_sidetone_mux_controls), \
	SND_SOC_DAPM_MUX("NEARMIC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_nearmic_mux_controls), \
	SND_SOC_DAPM_MUX("PDM MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_pdm_mux_controls), \
	SND_SOC_DAPM_MUX("DACR SDM MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacr_sdm_mux_controls), \
	SND_SOC_DAPM_MUX("DACSR SDM MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dacsr_sdm_mux_controls), \
	SND_SOC_DAPM_MUX("DAC HPL MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dac_hpl_mux_controls), \
	SND_SOC_DAPM_MUX("DAC HPR MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dac_hpr_mux_controls), \
	SND_SOC_DAPM_MUX("DAC EP MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dac_ep_mux_controls), \
	SND_SOC_DAPM_MUX("DAC LINEOUTL MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dac_ltl_mux_controls), \
	SND_SOC_DAPM_MUX("DAC LINEOUTR MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_dac_ltr_mux_controls), \
	SND_SOC_DAPM_MUX("ANC FAREND MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_anc_farend_mux_controls), \
	SND_SOC_DAPM_MUX("ERROR MIC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_error_mic_mux_controls), \
	SND_SOC_DAPM_MUX("REF MIC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_ref_mic_mux_controls), \
	SND_SOC_DAPM_MUX("IF8 TEST MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_if8_test_mux_controls), \
	SND_SOC_DAPM_MUX("LINEIN L MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_lineinl_mux_controls), \
	SND_SOC_DAPM_MUX("LINEIN R MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_lineinr_mux_controls), \
	SND_SOC_DAPM_MUX("MAINMIC MUX", \
		SND_SOC_NOPM, 0, 0, \
		&dapm_mainmic_mux_controls), \

void hi6403_headphone_pop_on(struct snd_soc_codec *codec)
{
	struct hi6403_platform_data *priv = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	priv = snd_soc_codec_get_drvdata(codec);

	/* headphone dac vbias power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_DAC_HP_VB_PB_BIT, 0);
	/* headphone dac sec vbias power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_DAC_HP_VB_SEC_PB_BIT, 0);
	/* headphone dac enable */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		0x3 << HI6403_DAC_HPL_PD_BIT, 0);
	/* headphone gen power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_HP_GEN_PD_BIT, 0);
	/* headphone feed back power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
		0x3 << HI6403_HPR_FD_PD_BIT, 0);
	udelay(200);
	/* headphone power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
		0x3 << HI6403_HPR_PD_BIT, 0);
	msleep(120);
	/* headphone pop pull set 0 */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG43,
		0x3F << HI6403_HPL_POP_PULL_2_0_BIT, 0);
	hi64xx_update_bits(codec, HI6403_ANALOG_REG44,
		0x3F << HI6403_HPR_POP_PULL_2_0_BIT, 0);
	snd_soc_write(codec, HI6403_DACL_PGA_GAIN_CFG_REG,
		priv->virtul_dacl_gain);
	snd_soc_write(codec, HI6403_DACR_PGA_GAIN_CFG_REG,
		priv->virtul_dacr_gain);
}

void hi6403_headphone_pop_off(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	snd_soc_write(codec, HI6403_DACL_PGA_GAIN_CFG_REG, 0xBA);
	snd_soc_write(codec, HI6403_DACR_PGA_GAIN_CFG_REG, 0xBA);
	msleep(85);

	/* headphone pop pull set not 0 */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG43,
		0x3F << HI6403_HPL_POP_PULL_2_0_BIT,
		0x8 << HI6403_HPL_POP_PULL_2_0_BIT);
	hi64xx_update_bits(codec, HI6403_ANALOG_REG44,
		0x3F << HI6403_HPR_POP_PULL_2_0_BIT,
		0x8 << HI6403_HPR_POP_PULL_2_0_BIT);
	/* headphone power down */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
		0x3 << HI6403_HPR_PD_BIT, 0x3 << HI6403_HPR_PD_BIT);
	msleep(50);
	/* headphone feed back power down */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG3,
		0x3 << HI6403_HPR_FD_PD_BIT, 0x3 << HI6403_HPR_FD_PD_BIT);
	udelay(100);

	/* headphone gen power down */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_HP_GEN_PD_BIT, 1 << HI6403_HP_GEN_PD_BIT);
	/* headphone dac disable */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		0x3 << HI6403_DAC_HPL_PD_BIT, 0x3 << HI6403_DAC_HPL_PD_BIT);
	/* headphone dac vbias power down */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_DAC_HP_VB_PB_BIT, 1 << HI6403_DAC_HP_VB_PB_BIT);
	/* headphone dac sec vbias power on */
	hi64xx_update_bits(codec, HI6403_ANALOG_REG2,
		1 << HI6403_DAC_HP_VB_SEC_PB_BIT,
		1 << HI6403_DAC_HP_VB_SEC_PB_BIT);
}

static void classH_headphone_select(struct snd_soc_codec *codec)
{
	/* SRC up16 select dacls pga dout */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACL_S_UP16_DIN_SEL_BIT,
		0 << HI6403_DACL_S_UP16_DIN_SEL_BIT);
	/* select classH path for headphone */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x3 << HI6403_CLASSH_DIN_L_SEL_BIT,
		0x0 << HI6403_CLASSH_DIN_L_SEL_BIT);
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x3 << HI6403_CLASSH_DIN_R_SEL_BIT,
		0x0 << HI6403_CLASSH_DIN_R_SEL_BIT);
	/* select LUT1 */
	hi64xx_update_bits(codec, HI6403_CLASSH_CTRL12,
		0x1 << HI6403_CLASSH_HP_EP_SW_BIT,
		0x1 << HI6403_CLASSH_HP_EP_SW_BIT);
}

static void classH_earphone_select(struct snd_soc_codec *codec)
{
	/* SRC up16 select classH dout L */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x1 << HI6403_DACL_S_UP16_DIN_SEL_BIT,
		0x1 << HI6403_DACL_S_UP16_DIN_SEL_BIT);
	/* select classH path for earphone */
	hi64xx_update_bits(codec, HI6403_CLASSH_HP_EP_CTRL,
		0x3 << HI6403_CLASSH_DIN_L_SEL_BIT,
		0x2 << HI6403_CLASSH_DIN_L_SEL_BIT);
	/* select LUT1B */
	hi64xx_update_bits(codec, HI6403_CLASSH_CTRL12,
		0x1 << HI6403_CLASSH_HP_EP_SW_BIT,
		0x0 << HI6403_CLASSH_HP_EP_SW_BIT);
}

void hi6403_set_classh_config(struct snd_soc_codec *codec,
	enum classh_state classH_state_cfg)
{
	union classh_bit_sel state;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	state.val = (unsigned char)(classH_state_cfg & 0x0F);

	AUDIO_LOGI("ClassH state is %d", state.val);

	if (state.classh_bind.hp_rcv_power == HP_RCV_POWEROFF)
		return;

	if ((state.classh_bind.hp_rcv_power == HP_POWER_ONLY &&
		state.classh.hp_classh == 1) ||
		(state.classh_bind.hp_rcv_power == RCV_POWER_ONLY &&
		state.classh.rcv_classh == 1))
		hi64xx_update_bits(codec, HI6403_CLASS_H_CFG_REG1,
			0x1 << HI6403_CLASS_H_SEL_BIT,
			0x1 << HI6403_CLASS_H_SEL_BIT);
	else
		hi64xx_update_bits(codec, HI6403_CLASS_H_CFG_REG1,
			0x1 << HI6403_CLASS_H_SEL_BIT,
			0x0 << HI6403_CLASS_H_SEL_BIT);

	if (state.classh_bind.hp_rcv_power == RCV_POWER_ONLY &&
		state.classh.rcv_classh == 1)
		classH_earphone_select(codec);
	else
		classH_headphone_select(codec);
}

static void headphone_power_param_pass(struct hi6403_platform_data *data,
	struct snd_soc_codec *codec, bool *hs_pwron_flag, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		*hs_pwron_flag = true;
		if (data->hsl_power_on && data->hsr_power_on) {
			data->rcv_hp_classh_state =
				(unsigned int)data->rcv_hp_classh_state |
				HP_POWER_STATE;
			hi6403_set_classh_config(codec, data->rcv_hp_classh_state);
			hi6403_headphone_pop_on(codec);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (data->hsl_power_on && data->hsr_power_on) {
			hi6403_headphone_pop_off(codec);
			data->rcv_hp_classh_state =
				(unsigned int)data->rcv_hp_classh_state &
				(~HP_POWER_STATE);
			hi6403_set_classh_config(codec, data->rcv_hp_classh_state);
		}
		*hs_pwron_flag = false;
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}
}

static int headphonel_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6403_platform_data *data =
		snd_soc_codec_get_drvdata(codec);

	headphone_power_param_pass(data, codec,
		&data->hsl_power_on, event);

	return 0;
}

static int headphoner_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6403_platform_data *data =
		snd_soc_codec_get_drvdata(codec);

	headphone_power_param_pass(data, codec,
		&data->hsr_power_on, event);

	return 0;
}

static int earphone_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6403_platform_data *priv =
		snd_soc_codec_get_drvdata(codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		priv->rcv_hp_classh_state =
			(unsigned int)priv->rcv_hp_classh_state |
				RCV_POWER_STATE;
		hi6403_set_classh_config(codec, priv->rcv_hp_classh_state);

		/* earphone dac on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_EAR_PD_BIT, 0);
		/* earphone pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_EAR_PD_BIT, 0);
		/* earphone mute off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG62,
			1 << HI6403_EAR_MUTE_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* earphone mute on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG62,
		 1 << HI6403_EAR_MUTE_BIT, 1 << HI6403_EAR_MUTE_BIT);
		/* earphone pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_EAR_PD_BIT, 1 << HI6403_EAR_PD_BIT);
		/* earphone dac pb */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_EAR_PD_BIT, 1 << HI6403_DAC_EAR_PD_BIT);

		priv->rcv_hp_classh_state =
			(unsigned int)priv->rcv_hp_classh_state &
			(~RCV_POWER_STATE);
		hi6403_set_classh_config(codec, priv->rcv_hp_classh_state);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int lineoutl_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* lineoutl dac on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_LOL_PD_BIT, 0);
		/* lineoutl pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_LOL_PD_BIT, 0);
		/* lineoutl mute off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG63,
			1 << HI6403_LOL_MUTE_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* lineoutl mute off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG63,
			1 << HI6403_LOL_MUTE_BIT,
			1 << HI6403_LOL_MUTE_BIT);
		/* lineoutl pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_LOL_PD_BIT, 1 << HI6403_LOL_PD_BIT);
		/* lineoutl dac pb */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_LOL_PD_BIT,
			1 << HI6403_DAC_LOL_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int lineoutr_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* lineoutr dac on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_LOR_PD_BIT, 0);
		/* lineoutr pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_LOL_PD_BIT, 0);
		/* lineoutr mute off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG64,
			1 << HI6403_LOR_MUTE_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* lineoutr mute off */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG64,
			1 << HI6403_LOR_MUTE_BIT,
			1 << HI6403_LOR_MUTE_BIT);
		/* lineoutr pga on */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_LOL_PD_BIT, 1 << HI6403_LOL_PD_BIT);
		/* lineoutr dac pb */
		hi64xx_update_bits(codec, HI6403_ANALOG_REG4,
			1 << HI6403_DAC_LOR_PD_BIT,
			1 << HI6403_DAC_LOR_PD_BIT);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return 0;
}

static int ec_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6403,
			SLIMBUS_TRACK_ECREF, NULL);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6403,
			SLIMBUS_TRACK_ECREF, NULL);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	return ret;
}

#define OUT_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV("S2 TX", \
		HI6403_S2_CFG_REG, HI6403_S2_TX_EN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("S4 TX", \
		HI6403_S4_CFG_REG, HI6403_S4_TX_EN_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV_E("HPL DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		headphonel_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("HPR DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		headphoner_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("EP DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		earphone_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("LOL DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		lineoutl_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("LOR DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		lineoutr_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV("U1 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_1_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U2 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_2_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U3 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_3_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U4 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_4_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U5 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_5_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U6 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_6_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV_E("U7 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, \
		HI6403_SLIM_UP_CLKEN_7_BIT, 0, NULL, 0, \
		ec_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV("U8 DRV", \
		HI6403_SLIM_UP_CLKEN_REG, HI6403_SLIM_UP_CLKEN_8_BIT, 0, \
		NULL, 0), \

static const struct snd_soc_dapm_widget path_widgets[] = {
	INPUT_WIDGET
	OUTPUT_WIDGET
	MIXER_WIDGET
	MUX_WIDGET
	OUT_DRV_WIDGET
#ifdef CONFIG_BUCKBOOST
	SND_SOC_DAPM_INPUT("AUXMIC PWM IN"),
#endif
#ifdef CONFIG_BUCKBOOST
	SND_SOC_DAPM_OUTPUT("AUXMIC PWM OUT"),
#endif
};

int hi6403_add_path_widgets(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, path_widgets,
		ARRAY_SIZE(path_widgets));
}

