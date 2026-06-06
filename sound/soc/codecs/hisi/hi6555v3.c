/*
 * hi6555v3_driver codec driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/clk.h>
#include <linux/timer.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/tlv.h>
#include <linux/spinlock.h>
#include <linux/printk.h>
#include <linux/regulator/consumer.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/hisi/drv_pmic_if.h>
#include <soc_crgperiph_interface.h>
#include <linux/version.h>

#include "hi6555v3.h"
#include <linux/hisi/hi6xxx/hi6xxx_mbhc.h>
#include "hi6555v3_asp_reg_def.h"
#include "hi6555v3_pmu_reg_def.h"
#include "hi6555v3_i2s.h"

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hi6555v3_debug.h"
#endif
/*lint -e548*/
#define LOG_TAG "hi6555v3"

static struct snd_soc_codec *soc_codec = NULL;

#define HAC_ENABLE                   1
#define GPIO_LEVEL_HIGH              1
#define GPIO_LEVEL_LOW               0
#define PGA_GAIN_STEP                200 /* unit:0.01db */
#define CLEAR_FIFO_DELAY_LEN_MS      1
#define DEFAULT_SIF_DRIVING_FORCE 5
#define MAX_SIF_DRIVING_FORCE 0xF
#define LDO8_VOLTAGE 1800000

static unsigned int hac_en_gpio = ARCH_NR_GPIOS;
static int hac_switch = 0;

static int hac_gpio_output_set(int hac_cmd)
{
	int ret;
	if (!gpio_is_valid(hac_en_gpio)) {
		HI6555V3_LOGE("Failed to get the hac gpio\n");
		return -EFAULT;
	}

	if (HAC_ENABLE == hac_cmd) {
		HI6555V3_LOGI("Enable hac gpio %u\n", hac_en_gpio);
		ret = gpio_direction_output(hac_en_gpio, GPIO_LEVEL_HIGH);
	} else {
		HI6555V3_LOGI("Disable hac gpio %u\n", hac_en_gpio);
		ret = gpio_direction_output(hac_en_gpio, GPIO_LEVEL_LOW);
	}

	return ret;
}

static const char * const hac_switch_text[] = {"OFF", "ON"};

static const struct soc_enum hac_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hac_switch_text), hac_switch_text),
};

int scharger_flash_bst_vo_config(int config_voltage)
{
	return 0;
}

static int hac_switch_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	if (NULL == kcontrol || NULL == ucontrol) {
		HI6555V3_LOGE("input pointer is null\n");
		return -EFAULT;
	}

	ucontrol->value.integer.value[0] = hac_switch;

	return 0;
}

static int hac_switch_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int ret;

	if (NULL == kcontrol || NULL == ucontrol) {
		HI6555V3_LOGE("input pointer is null\n");
		return -EFAULT;
	}

	hac_switch = ucontrol->value.integer.value[0];
	ret = hac_gpio_output_set(hac_switch);

	return ret;
}

static enum hi6555v3_classd_supply_type _get_classd_supply_type(struct snd_soc_codec *codec)
{
	unsigned int value = CLASSD_SUPPLY_INVALID;
	struct device_node *np = NULL;
	WARN_ON(!codec->dev);
	WARN_ON(!codec->dev->of_node);
	np = codec->dev->of_node;

	IN_FUNCTION;

	if (of_property_read_u32(np, "classd_supply_type", &value)) {
		HI6555V3_LOGW("can not find classd_supply_type,pls check if not use classd\n");
		return CLASSD_SUPPLY_INVALID;
	}

	if (value >= CLASSD_SUPPLY_INVALID) {
		HI6555V3_LOGE("find invalied classd_supply_type %u\n", value);
		return CLASSD_SUPPLY_INVALID;
	}

	HI6555V3_LOGI("classd_supply_type is %u\n", value);

	OUT_FUNCTION;
	return value;
}

static int classd_schg_voltage_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = NULL;
	struct hi6555v3_priv *priv = NULL;

	IN_FUNCTION;
	if (!kcontrol || !ucontrol) {
		HI6555V3_LOGE("input pointer is null\n");
		return -EFAULT;
	}

	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	ucontrol->value.integer.value[0] = priv->classd_scharger_voltage;

	OUT_FUNCTION;
	return 0;
}

static int classd_schg_voltage_put(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int ret;
	int classd_voltage;
	struct snd_soc_codec *codec = NULL;
	struct hi6555v3_priv *priv = NULL;

	IN_FUNCTION;
	if (!kcontrol || !ucontrol) {
		HI6555V3_LOGE("input pointer is null\n");
		return -EFAULT;
	}

	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	if (CLASSD_SUPPLY_SCHARGER != priv->classd_supply_type) {
		HI6555V3_LOGE("class supply type is not sharger, CLASSD_SCHG_VOLTAGE_CONFIG should not exist in path xml\n");
		return -ENXIO;
	}

	/* The default voltage is 4.5,set by pmu.
	 * And here set range 4.5~5.2
	 */
	classd_voltage = ucontrol->value.integer.value[0];
	if ((classd_voltage % CLASSD_VOLTAGE_DIV)
		|| (classd_voltage < CLASSD_MIN_VOLTAGE)
		|| (classd_voltage > CLASSD_MAX_VOLTAGE)){
		HI6555V3_LOGE("classd voltage config:%d in xml is not support\n", classd_voltage);
		return -EINVAL;
	}

	ret = scharger_flash_bst_vo_config(classd_voltage);
	if (ret) {
		HI6555V3_LOGI("classd voltage config:%d fail:%d\n", classd_voltage, ret);
		return ret;
	}

	priv->classd_scharger_voltage = classd_voltage;
	HI6555V3_LOGI("classd voltage config:%d ok\n", classd_voltage);

	OUT_FUNCTION;
	return ret;
}

/* MAINPGA GAIN(MAINPGA_GAIN<2:0>)£»
 * range:0dB~14dB¡£
 * step:2dB¡£
 * 000: 0dB£»
 * 001: 2dB£»
 * 010: 4dB£»
 * 011: 6dB£»
 * 100: 8dB£»
 * 101: 10dB£»
 * 110: 12dB£»
 * 111: 14dB£» */
static DECLARE_TLV_DB_SCALE(hi6555v3_ana_mainpga_gain_tlv, 0, PGA_GAIN_STEP, 0);

/* AUXPGA GAIN(AUXPGA_GAIN<2:0>)£»
 * range:0dB~14dB¡£
 * step:2dB¡£
 * 000: 0dB£»
 * 001: 2dB£»
 * 010: 4dB£»
 * 011: 6dB£»
 * 100: 8dB£»
 * 101: 10dB£»
 * 110: 12dB£»
 * 111: 14dB£» */
static DECLARE_TLV_DB_SCALE(hi6555v3_ana_auxpga_gain_tlv, 0, PGA_GAIN_STEP, 0);

/* MIC3PGA GAIN(MIC3PGA_GAIN<2:0>)£»
 * range:0dB~14dB¡£
 * step:2dB¡£
 * 000: 0dB£»
 * 001: 2dB£»
 * 010: 4dB£»
 * 011: 6dB£»
 * 100: 8dB£»
 * 101: 10dB£»
 * 110: 12dB£»
 * 111: 14dB£» */
static DECLARE_TLV_DB_SCALE(hi6555v3_ana_mic3pga_gain_tlv, 0, PGA_GAIN_STEP, 0);

#define HI6555V3_FIFO_FS_KCONTROLS	\
/*{*/																							\
	SOC_SINGLE("FS_3RDCODEC_L_DLINK", 	FS_CTRL2_REG, FS_CODEC3_L_DLINK_OFFSET, 	0x7, 0),	\
	SOC_SINGLE("FS_3RDCODEC_R_DLINK", 	FS_CTRL2_REG, FS_CODEC3_R_DLINK_OFFSET,		0x7, 0),	\
	SOC_SINGLE("FS_VOICE_L_DLINK",		FS_CTRL0_REG, FS_VOICE_L_DLINK_OFFSET,		0x3, 0),	\
	SOC_SINGLE("FS_VOICE_R_DLINK",		FS_CTRL2_REG, FS_VOICE_R_DLINK_OFFSET, 		0x3, 0),	\
	SOC_SINGLE("FS_AUDIO_L_UPLINK",		FS_CTRL0_REG, FS_AUDIO_L_UPLINK_OFFSET, 	0x1, 0),	\
	SOC_SINGLE("FS_AUDIO_R_UPLINK",		FS_CTRL0_REG, FS_AUDIO_R_UPLINK_OFFSET, 	0x1, 0),	\
	SOC_SINGLE("FS_VOICE_L_UPLINK",		FS_CTRL0_REG, FS_VOICE_L_UPLINK_OFFSET, 	0x3, 0),	\
	SOC_SINGLE("FS_VOICE_R_UPLINK", 	FS_CTRL0_REG, FS_VOICE_R_UPLINK_OFFSET, 	0x3, 0),	\
	SOC_SINGLE("FS_MIC3_UPLINK", 		FS_CTRL0_REG, FS_MIC3_UPLINK_OFFSET, 		0x7, 0),	\
	SOC_SINGLE("FS_ECHO_L_UPLINK",		FS_CTRL0_REG, FS_ECHO_L_UPLINK_OFFSET, 		0x7, 0),	\
	SOC_SINGLE("FS_ECHO_R_UPLINK",		FS_CTRL0_REG, FS_ECHO_R_UPLINK_OFFSET, 		0x7, 0),	\
	SOC_SINGLE("FS_ULTR_UPLINK",		FS_CTRL3_REG, FS_ULTR_UPLINK_OFFSET, 		0x3, 0),	\
	SOC_SINGLE("FS_ULTR_DLINK",			FS_CTRL3_REG, FS_ULTR_DLINK_OFFSET, 		0x3, 0),	\
	SOC_SINGLE("FS_SPA1_UPLINK",		FS_CTRL3_REG, FS_SPA1_UPLINK_OFFSET, 		0x1, 0),	\
	SOC_SINGLE("FS_SPA3_UPLINK",		FS_CTRL3_REG, FS_SPA3_UPLINK_OFFSET, 		0x1, 0)		\
/*}*/																							\

#define HI6555V3_I2S_KCONTROLS	\
/*{*/																						     \
	SOC_SINGLE("FS_I2S1",			  		I2S1_CTRL_REG,	 	 FS_I2S1_OFFSET,			0x7, 0),  \
	SOC_SINGLE("I2S1_TX_CLK_SEL", 	  		I2S1_CTRL_REG,		 I2S1_TX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S1_RX_CLK_SEL", 	  		I2S1_CTRL_REG, 	 	 I2S1_RX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S1_MST_SLV",		  		I2S1_CTRL_REG,     	 I2S1_MST_SLV_OFFSET,       0x1, 0), \
	SOC_SINGLE("I2S1_DIRECT_LOOP",	  		I2S1_CTRL_REG,	 	 I2S1_DIRECT_LOOP_OFFSET,   0x3, 0), \
	SOC_SINGLE("I2S1_FUNC_MODE", 	  		I2S1_CTRL_REG, 	 	 I2S1_FUNC_MODE_OFFSET,     0x7, 0), \
	SOC_SINGLE("I2S1_FRAME_MODE", 	  		I2S1_CTRL_REG, 	 	 I2S1_FRAME_MODE_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S1_CODEC_IO_WORDLENGTH", 	I2S1_CTRL_REG, 	 	 I2S1_CODEC_IO_WORDLENGTH_OFFSET,     0x3, 0), \
	SOC_SINGLE("FS_I2S2", 			  		I2S2_PCM_CTRL_REG, 	 FS_I2S2_OFFSET, 	 		0x7, 0), 	\
	SOC_SINGLE("I2S2_TX_CLK_SEL", 	  		I2S2_PCM_CTRL_REG, 	 I2S2_TX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S2_RX_CLK_SEL", 	 		I2S2_PCM_CTRL_REG, 	 I2S2_RX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S2_MST_SLV",        		I2S2_PCM_CTRL_REG,   I2S2_MST_SLV_OFFSET,       0x1, 0), \
	SOC_SINGLE("I2S2_FUNC_MODE",			I2S2_PCM_CTRL_REG, 	 I2S2_FUNC_MODE_OFFSET,     0x7, 0), \
	SOC_SINGLE("I2S2_FRAME_MODE", 	  		I2S2_PCM_CTRL_REG, 	 I2S2_FRAME_MODE_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S2_DIRECT_LOOP",	  		I2S2_PCM_CTRL_REG,	 I2S2_DIRECT_LOOP_OFFSET,	0x3, 0), \
	SOC_SINGLE("I2S2_CODEC_IO_WORDLENGTH",	I2S2_PCM_CTRL_REG, 	 I2S2_CODEC_IO_WORDLENGTH_OFFSET,     0x3, 0), \
	SOC_SINGLE("FS_I2S3", 				  	I2S3_PCM_CTRL_REG, 	 FS_I2S3_OFFSET, 			0x7, 0),  \
	SOC_SINGLE("I2S3_TX_CLK_SEL", 	  		I2S3_PCM_CTRL_REG, 	 I2S3_TX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S3_RX_CLK_SEL", 	  		I2S3_PCM_CTRL_REG, 	 I2S3_RX_CLK_SEL_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S3_MST_SLV",		  		I2S3_PCM_CTRL_REG,   I2S3_MST_SLV_OFFSET,       0x1, 0), \
	SOC_SINGLE("I2S3_DIRECT_LOOP",	  		I2S3_PCM_CTRL_REG,	 I2S3_DIRECT_LOOP_OFFSET,   0x3, 0),  \
	SOC_SINGLE("I2S3_FUNC_MODE", 	  		I2S3_PCM_CTRL_REG, 	 I2S3_FUNC_MODE_OFFSET,     0x7, 0), \
	SOC_SINGLE("I2S3_FRAME_MODE", 	  		I2S3_PCM_CTRL_REG, 	 I2S3_FRAME_MODE_OFFSET,    0x1, 0), \
	SOC_SINGLE("I2S3_CODEC_IO_WORDLENGTH",	I2S3_PCM_CTRL_REG, 	 I2S3_CODEC_IO_WORDLENGTH_OFFSET,     0x3, 0), \
	SOC_SINGLE("I2S1_TDM_TX_CLK_SEL", 	  	I2S1_TDM_CTRL0_REG,	 S1_TDM_TX_CLK_SEL_OFFSET,  0x1, 0), \
	SOC_SINGLE("I2S1_TDM_RX_CLK_SEL", 	  	I2S1_TDM_CTRL0_REG,	 S1_TDM_RX_CLK_SEL_OFFSET,  0x1, 0), \
	SOC_SINGLE("I2S1_TDM_MST_SLV",		  	I2S1_TDM_CTRL1_REG,	 S1_TDM_MST_SLV_OFFSET,     0x1, 0), \
	SOC_SINGLE("I2S1_TDM_DIRECT_LOOP",	 	I2S1_TDM_CTRL0_REG,  S1_TDM_DIRECT_LOOP_OFFSET, 0x3, 0), \
	SOC_SINGLE("I2S1_TDM_FRAME_MODE", 	  	I2S1_TDM_CTRL0_REG,  S1_TDM_FRAME_MODE_OFFSET,  0x7, 0) \
/*}*/																						     \

#define HI6555V3_SRC_KCONTROLS	\
/*{*/																						         \
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_MODE",SRCUP_CTRL_REG, 	VOICE_L_DN_SRCUP_SRC_MODE_OFFSET,	0x7, 0), \
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_DIN", FS_CTRL2_REG, 		FS_VOICE_L_DN_SRCUP_IN_OFFSET, 		0x3, 0), \
	SOC_SINGLE("VOICE_L_DN_SRCUP_SRC1_DOUT",FS_CTRL2_REG, 		FS_VOICE_L_DN_SRCUP_DOUT_OFFSET, 	0x3, 0), \
	SOC_SINGLE("DACL_SRC2_MODE", 			SRCUP_CTRL_REG, 	DACL_SRCUP_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("DACL_SRC2", 				FS_CTRL0_REG, 		FS_DACL_SRCUP_IN_OFFSET, 		 	0x3, 0), \
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_MODE",SRCUP_CTRL_REG, 	VOICE_R_DN_SRCUP_SRC_MODE_OFFSET, 	0x7, 0), \
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_DIN", FS_CTRL2_REG, 		FS_VOICE_R_DN_SRCUP_IN_OFFSET, 		0x3, 0), \
	SOC_SINGLE("VOICE_R_DN_SRCUP_SRC3_DOUT",FS_CTRL2_REG, 		FS_VOICE_R_DN_SRCUP_DOUT_OFFSET, 	0x3, 0), \
	SOC_SINGLE("DACR_SRC4_MODE", 			SRCUP_CTRL_REG, 	DACR_SRCUP_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("DACR_SRC4", 				FS_CTRL0_REG, 		FS_DACR_SRCUP_IN_OFFSET, 		 	0x3, 0), \
	SOC_SINGLE("AUDIO_L_UP_SRC5_MODE", 		SRCUP_CTRL_REG, 	AUDIO_L_UP_SRCUP_SRC_MODE_OFFSET, 	0x7, 0), \
	SOC_SINGLE("AUDIO_L_UP_SRC5", 			FS_CTRL0_REG, 		FS_AUDIO_L_UP_SRCUP_IN_OFFSET, 	 	0x3, 0), \
	SOC_SINGLE("AUDIO_R_UP_SRC6_MODE",		SRCUP_CTRL_REG, 	AUDIO_R_UP_SRCUP_SRC_MODE_OFFSET, 	0x7, 0), \
	SOC_SINGLE("AUDIO_R_UP_SRC6", 			FS_CTRL0_REG, 		FS_AUDIO_R_UP_SRCUP_IN_OFFSET, 	 	0x3, 0), \
	SOC_SINGLE("VOICE_L_UP_SRC7_MODE",		SRCDN_CTRL_REG, 	VOICE_L_UP_SRCDN_SRC_MODE_OFFSET, 	0x7, 0), \
	SOC_SINGLE("VOICE_L_UP_SRC7", 			FS_CTRL1_REG, 		FS_VOICE_L_UP_SRCDN_OUT_OFFSET, 	0x3, 0), \
	SOC_SINGLE("VOICE_R_UP_SRC8_MODE",		SRCDN_CTRL_REG, 	VOICE_R_UP_SRCDN_SRC_MODE_OFFSET, 	0x7, 0), \
	SOC_SINGLE("VOICE_R_UP_SRC8", 			FS_CTRL1_REG, 		FS_VOICE_R_UP_SRCDN_OUT_OFFSET, 	0x3, 0), \
	SOC_SINGLE("MIC3_UP_SRC9_MODE", 		SRCDN_CTRL_REG, 	MIC3_UP_SRCDN_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("MIC3_UP_SRC9",				FS_CTRL1_REG, 		FS_MIC3_UP_SRCDN_OUT_OFFSET, 	 	0x3, 0), \
	SOC_SINGLE("ECHO_L_UP_SRC10_MODE",		SRCDN_CTRL_REG, 	ECHO_L_UP_SRCDN_SRC_MODE_OFFSET,  	0x7, 0), \
	SOC_SINGLE("ECHO_L_UP_SRC10", 			FS_CTRL1_REG, 		FS_ECHO_L_UP_SRCDN_OUT_OFFSET, 	 	0x3, 0), \
	SOC_SINGLE("ECHO_R_UP_SRC11_MODE",		SRCDN_CTRL_REG, 	ECHO_R_UP_SRCDN_SRC_MODE_OFFSET,  	0x7, 0), \
	SOC_SINGLE("ECHO_R_UP_SRC11", 			FS_CTRL1_REG, 		FS_ECHO_R_UP_SRCDN_OUT_OFFSET, 	 	0x3, 0), \
	SOC_SINGLE("BT_L_RX_SRCUP_SRC12_MODE",	SRCUP_CTRL_REG, BT_L_RX_SRCUP_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_L_RX_SRCUP_IN_SRC12", 	FS_CTRL1_REG, 	FS_BT_L_RX_SRCUP_IN_OFFSET, 		0x7, 0), \
	SOC_SINGLE("BT_L_RX_SRCUP_OUT_SRC12", 	FS_CTRL1_REG, 	FS_BT_L_RX_SRCUP_OUT_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCDN_SRC13_MODE",	SRCUP_CTRL_REG, BT_R_RX_SRCUP_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCUP_IN_SRC13", 	FS_CTRL1_REG, 	FS_BT_R_RX_SRCUP_IN_OFFSET, 		0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCUP_OUT_SRC13", 	FS_CTRL1_REG, 	FS_BT_R_RX_SRCUP_OUT_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_L_RX_SRCDN_SRC14_MODE",	SRCDN_CTRL_REG, BT_L_RX_SRCDN_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_L_RX_SRCDN_IN_SRC14", 	FS_CTRL2_REG, 	FS_BT_L_RX_SRCDN_IN_OFFSET, 		0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCUP_OUT_SRC14", 	FS_CTRL2_REG, 	FS_BT_L_RX_SRCDN_OUT_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCDN_SRC15_MODE",	SRCDN_CTRL_REG, BT_R_RX_SRCDN_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCDN_IN_SRC15", 	FS_CTRL2_REG, 	FS_BT_R_RX_SRCDN_IN_OFFSET, 		0x7, 0), \
	SOC_SINGLE("BT_R_RX_SRCDN_OUT_SRC15", 	FS_CTRL2_REG, 	FS_BT_R_RX_SRCDN_OUT_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_L_TX_SRCDN_SRC16_MODE", 	SRCDN_CTRL_REG, BT_L_TX_SRCDN_SRC_MODE_OFFSET, 	 	0x7, 0), \
	SOC_SINGLE("BT_L_TX_SRCDN_OUT_SRC16",	FS_CTRL3_REG,	FS_I2S2_TX_L_SRCDN_DOUT_OFFSET,		0x7, 0), \
	SOC_SINGLE("BT_R_TX_SRCDN_SRC17_MODE",	SRCDN_CTRL1_REG,BT_TX_R_SRCDN_SRC_MODE_OFFSET, 		0x7, 0), \
	SOC_SINGLE("BT_R_TX_SRCDN_IN_SRC17", 	FS_CTRL3_REG, 	FS_I2S2_TX_R_SRCDN_DIN_OFFSET, 		0x1, 0), \
	SOC_SINGLE("BT_R_TX_SRCDN_OUT_SRC17", 	FS_CTRL3_REG, 	FS_I2S2_TX_R_SRCDN_DOUT_OFFSET, 	0x7, 0), \
	SOC_SINGLE("CODEC3_L_DN_PGA_SRC", 		FS_CTRL3_REG, 	FS_CODEC3_L_DN_PGA_OFFSET, 			0x7, 0), \
	SOC_SINGLE("CODEC3_R_DN_PGA_SRC", 		FS_CTRL3_REG, 	FS_CODEC3_R_DN_PGA_OFFSET, 			0x7, 0), \
	SOC_SINGLE("ULTR_DN_PGA_SRC", 			FS_CTRL3_REG, 	FS_ULTR_DN_PGA_OFFSET, 				0x3, 0) \
/*}*/																							         \

#define HI6555V3_PGA_KCONTROLS	\
/*{*/																						    \
	SOC_SINGLE("VOICE_L_DN_PGA_GAIN", 		VOICE_L_DN_PGA_CTRL_REG, 	VOICE_L_DN_PGA_GAIN_OFFSET, 	0xff, 0),\
	SOC_SINGLE("VOICE_L_DN_PGA_BYPASS", 	VOICE_L_DN_PGA_CTRL_REG, 	VOICE_L_DN_PGA_BYPASS_OFFSET,	0x1,  0),\
	SOC_SINGLE("VOICE_R_DN_PGA_GAIN", 		VOICE_R_DN_PGA_CTRL_REG, 	VOICE_R_DN_PGA_GAIN_OFFSET, 	0xff, 0),\
	SOC_SINGLE("VOICE_R_DN_PGA_BYPASS", 	VOICE_R_DN_PGA_CTRL_REG, 	VOICE_R_DN_PGA_BYPASS_OFFSET,	0x1,  0),\
	SOC_SINGLE("AUDIO_L_DN_PGA_GAIN", 		AUDIO_L_DN_PGA_CTRL_REG, 	AUDIO_L_DN_PGA_GAIN_OFFSET, 	0xff, 0),\
	SOC_SINGLE("AUDIO_L_DN_PGA_BYPASS", 	AUDIO_L_DN_PGA_CTRL_REG, 	AUDIO_L_DN_PGA_BYPASS_OFFSET,	0x1,  0),\
	SOC_SINGLE("AUDIO_R_DN_PGA_GAIN", 		AUDIO_R_DN_PGA_CTRL_REG, 	AUDIO_R_DN_PGA_GAIN_OFFSET, 	0xff, 0),\
	SOC_SINGLE("AUDIO_R_DN_PGA_BYPASS", 	AUDIO_R_DN_PGA_CTRL_REG, 	AUDIO_R_DN_PGA_BYPASS_OFFSET,	0x1,  0),\
	SOC_SINGLE("SIDETONE_PGA_BYPASS",		SIDETONE_PGA_CTRL_REG, 	 	SIDETONE_PGA_BYPASS_OFFSET, 	0x1,  0),\
	SOC_SINGLE("SIDETONE_PGA",				SIDETONE_PGA_CTRL_REG, 	 	SIDETONE_PGA_GAIN_OFFSET, 		0xff, 0),\
	SOC_SINGLE("ULTR_DN_PGA_BYPASS",		ULTR_DN_PGA_CTRL_REG, 	 	ULTR_DN_PGA_BYPASS_OFFSET, 		0x1,  0),\
	SOC_SINGLE("ULTR_DN_PGA",				ULTR_DN_PGA_CTRL_REG, 	 	ULTR_DN_PGA_GAIN_OFFSET, 		0xff, 0),\
	SOC_SINGLE("CODEC3_L_DN_PGA_BYPASS", 	CODEC3_L_DN_PGA_CTRL_REG, 	CODEC3_L_DN_PGA_BYPASS_OFFSET, 	0x1,  0),\
	SOC_SINGLE("CODEC3_L_DN_PGA",			CODEC3_L_DN_PGA_CTRL_REG, 	CODEC3_L_DN_PGA_GAIN_OFFSET, 	0xff, 0),\
	SOC_SINGLE("CODEC3_R_DN_PGA_BYPASS",	CODEC3_R_DN_PGA_CTRL_REG, 	CODEC3_R_DN_PGA_BYPASS_OFFSET, 	0x1,  0),\
	SOC_SINGLE("CODEC3_R_DN_PGA",			CODEC3_R_DN_PGA_CTRL_REG, 	CODEC3_R_DN_PGA_GAIN_OFFSET, 	0xff, 0), \
	SOC_SINGLE("BT_L_RX_PGA_GAIN",    		BT_L_RX_PGA_CTRL_REG,  		BT_L_RX_PGA_GAIN_OFFSET,   		0xff, 0), \
	SOC_SINGLE("BT_L_RX_PGA_BYPASS", 		BT_L_RX_PGA_CTRL_REG, 		BT_L_RX_PGA_BYPASS_OFFSET, 		0x1, 0), \
	SOC_SINGLE("BT_R_RX_PGA_GAIN",    		BT_R_RX_PGA_CTRL_REG,  		BT_R_RX_PGA_GAIN_OFFSET,   		0xff, 0), \
	SOC_SINGLE("BT_R_RX_PGA_BYPASS", 		BT_R_RX_PGA_CTRL_REG, 		BT_R_RX_PGA_BYPASS_OFFSET, 		0x1, 0), \
	SOC_SINGLE("AUDIO_L_UP_PGA_GAIN", 		AUDIO_L_UP_PGA_CTRL_REG, 	AUDIO_L_UP_PGA_GAIN_OFFSET,   	0xff, 0), \
	SOC_SINGLE("AUDIO_L_UP_PGA_BYPASS", 	AUDIO_L_UP_PGA_CTRL_REG, 	AUDIO_L_UP_PGA_BYPASS_OFFSET, 	0x1,  0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_GAIN", 		AUDIO_R_UP_PGA_CTRL_REG, 	AUDIO_R_UP_PGA_GAIN_OFFSET,   	0xff, 0), \
	SOC_SINGLE("AUDIO_R_UP_PGA_BYPASS", 	AUDIO_R_UP_PGA_CTRL_REG, 	AUDIO_R_UP_PGA_BYPASS_OFFSET, 	0x1,  0), \
	SOC_SINGLE("MIC3_UP_PGA_GAIN",			MIC3_UP_PGA_CTRL_REG, 	 	MIC3_UP_PGA_GAIN_OFFSET, 	   	0xff,  0),\
	SOC_SINGLE("MIC3_UP_PGA_BYPASS", 		MIC3_UP_PGA_CTRL_REG, 	 	MIC3_UP_PGA_BYPASS_OFFSET,    	0x1,  0),  \
	SOC_SINGLE("ADCL_CIC_GAIN", 			ADC_FILTER_CTRL_REG, 		ADCL_CIC_GAIN_OFFSET,    		0x3f, 0), \
	SOC_SINGLE("ADCR_CIC_GAIN", 			ADC_FILTER_CTRL_REG, 		ADCR_CIC_GAIN_OFFSET,    		0x3f, 0), \
	SOC_SINGLE("ADC_MIC3_CIC_GAIN", 		ADC_FILTER_CTRL_REG, 		ADC_MIC3_CIC_GAIN_OFFSET,		0x3f, 0), \
	SOC_SINGLE("SPAV_ADC_CIC_GAIN", 		SPAIV_ADC_FILTER_REG, 		ADC_SPAV_CIC_GAIN_OFFSET,   	0x3f, 0), \
	SOC_SINGLE("SPAI_ADC_CIC_GAIN", 		SPAIV_ADC_FILTER_REG, 		ADC_SPAI_CIC_GAIN_OFFSET,   	0x3f, 0), \
	SOC_SINGLE("ADC_ULTR_CIC_GAIN", 		ADC_ULTR_FILTER_CTRL_REG, 	ADC_ULTR_CIC_GAIN_OFFSET,   	0x3f, 0),  \
	SOC_SINGLE("IN1_S2_L_PGA",		  		DACL_MIXER4_CTRL0_REG, 		DACL_MIXER4_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN2_VOICE_DL_PGA",	  		DACL_MIXER4_CTRL0_REG, 		DACL_MIXER4_GAIN2_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN3_AUDIO_DL_PGA",	  		DACL_MIXER4_CTRL0_REG, 		DACL_MIXER4_GAIN3_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN4_SIDETONE_DL_PGA", 		DACL_MIXER4_CTRL0_REG, 		DACL_MIXER4_GAIN4_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN1_S2_R_PGA",		  		DACR_MIXER4_CTRL0_REG, 		DACR_MIXER4_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN2_VOICE_DR_PGA",	  		DACR_MIXER4_CTRL0_REG, 		DACR_MIXER4_GAIN2_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN3_AUDIO_DR_PGA",	  		DACR_MIXER4_CTRL0_REG, 		DACR_MIXER4_GAIN3_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN4_SIDETONE_DR_PGA", 		DACR_MIXER4_CTRL0_REG,		DACR_MIXER4_GAIN4_OFFSET, 		0x3, 0),  \
	SOC_SINGLE("DACML_MIXER2_GAIN1", 		DACML_MIXER2_CTRL_REG, 		DACML_MIXER2_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("DACML_MIXER2_GAIN2", 		DACML_MIXER2_CTRL_REG, 		DACML_MIXER2_GAIN2_OFFSET, 		0x3, 0), \
	SOC_SINGLE("DACMR_MIXER2_GAIN1", 		DACMR_MIXER2_CTRL_REG, 		DACMR_MIXER2_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("DACMR_MIXER2_GAIN2", 		DACMR_MIXER2_CTRL_REG, 		DACMR_MIXER2_GAIN2_OFFSET, 		0x3, 0), \
	SOC_SINGLE("TX_MIXER2_GAIN1_PGA", 		BT_TX_MIXER2_CTRL_REG, 		BT_TX_MIXER2_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("TX_MIXER2_GAIN2_PGA", 		BT_TX_MIXER2_CTRL_REG, 		BT_TX_MIXER2_GAIN2_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN1_DACL_SRC_OUT_PGA",		ULTR_MIXER2_CTRL_REG, 		ULTR_MIXER2_GAIN1_OFFSET, 		0x3, 0), \
	SOC_SINGLE("IN2_ULTR_D_PGA",	 		ULTR_MIXER2_CTRL_REG, 		ULTR_MIXER2_GAIN2_OFFSET, 		0x3, 0) \
/*}*/																							\

#define HI6555V3_ANA_KCONTROLS	\
/*{*/																									     \
	SOC_SINGLE("ANA_HPL_PGA_MUTE",		CODEC_ANA_RW5_REG, 			HPL_MUTE_OFFSET, 0x1, 0),\
	SOC_SINGLE("ANA_HPL_PGA", 			CODEC_ANA_RW21_REG, 		HSL_GAIN_OFFSET, 0xF, 0),\
	SOC_SINGLE("ANA_HPR_PGA_MUTE",		CODEC_ANA_RW5_REG, 			HPR_MUTE_OFFSET, 0x1, 0),\
	SOC_SINGLE("ANA_HPR_PGA", 			CODEC_ANA_RW21_REG, 		HSR_GAIN_OFFSET, 0xF, 0),\
	SOC_SINGLE("ANA_SPK_PGA_MUTE", 		CTRL_REG_CLASSD_CTRL0_REG, 	CTRL_REG_CLASSD_MUTE_OFFSET, 0x1, 0),\
	SOC_SINGLE("ANA_SPK_PGA", 			CTRL_REG_CLASSD_CTRL0_REG, 	CTRL_REG_CLASSD_GAIN_OFFSET, 0x3, 0),\
	SOC_SINGLE("ANA_EAR_PGA_MUTE",		CODEC_ANA_RW6_REG, 			EAR_MUTE_OFFSET,           0x1, 0),\
	SOC_SINGLE("ANA_EAR_PGA", 			CODEC_ANA_RW26_REG, 		EAR_GAIN_OFFSET,           0xF, 0),\
	SOC_SINGLE("ANA_LINEOUT_PGA_MUTE",	CODEC_ANA_RW6_REG, 			LOUT_MUTE_OFFSET,          0x1, 0),\
	SOC_SINGLE("ANA_LINEOUT_PGA", 		CODEC_ANA_RW28_REG, 		LOUT_GAIN_OFFSET,          0xF, 0),\
	SOC_SINGLE("ANA_MAINMIC_PGA_MUTE",	CODEC_ANA_RW2_REG, 			MAINPGA_MUTE_OFFSET,       0x1, 0),\
	SOC_SINGLE("ANA_MAINPGA_BOOST",	    CODEC_ANA_RW9_REG, 			MAINPGA_BOOST_OFFSET,      0x1, 0),\
	SOC_SINGLE_TLV("ANA_MAINMIC_PGA", 	CODEC_ANA_RW9_REG, 			MAINPGA_GAIN_OFFSET, 0x7, 0, hi6555v3_ana_mainpga_gain_tlv),\
	SOC_SINGLE("ANA_AUXMIC_PGA_MUTE", 	CODEC_ANA_RW2_REG, 			AUXPGA_MUTE_OFFSET,        0x1, 0),\
	SOC_SINGLE("ANA_AUXPGA_BOOST",		CODEC_ANA_RW10_REG, 		AUXPGA_BOOST_OFFSET,      0x1, 0),\
	SOC_SINGLE_TLV("ANA_AUXMIC_PGA", 	CODEC_ANA_RW10_REG, 		AUXPGA_GAIN_OFFSET,  0x7, 0, hi6555v3_ana_auxpga_gain_tlv),\
	SOC_SINGLE("ANA_MIC3_PGA_MUTE", 	CODEC_ANA_RW2_REG, 			MIC3PGA_MUTE_OFFSET,       0x1, 0),\
	SOC_SINGLE("ANA_MIC3PGA_BOOST",	    CODEC_ANA_RW11_REG, 		ADC3PGA_BOOST_OFFSET,      0x3, 0),\
	SOC_SINGLE_TLV("ANA_MIC3_PGA",		CODEC_ANA_RW11_REG, 		MIC3PGA_GAIN_OFFSET, 0x7, 0, hi6555v3_ana_mic3pga_gain_tlv),\
	SOC_SINGLE("ANA_MIXER_EAR_L_PGA", 	CODEC_ANA_RW25_REG, 		CR_MIXER_EP_DACL_OFFSET,0x1, 0),\
	SOC_SINGLE("ANA_MIXER_EAR_R_PGA", 	CODEC_ANA_RW25_REG, 		CR_MIXER_EP_DACR_OFFSET,0x1, 0), \
	SOC_SINGLE("ANA_MIXER_EAR_ULTR_PGA", 		CODEC_ANA_RW25_REG, 				CR_MIXER_EP_DAC_ULTRA_OFFSET,0x1, 0),\
	SOC_SINGLE("ANA_MIXER_LINEOUT_L_PGA", 			CODEC_ANA_RW27_REG, 			LOMIX_BOOSTGAIN_DACL_OFFSET,0x1, 0),\
	SOC_SINGLE("ANA_MIXER_LINEOUT_R_PGA", 			CODEC_ANA_RW27_REG, 			LOMIX_BOOSTGAIN_DACR_OFFSET,0x1, 0),\
	SOC_SINGLE("ANA_MIXER_LINEOUT_ULTR_PGA", 		CODEC_ANA_RW27_REG, 			LOMIX_BOOSTGAIN_DACULTRA_OFFSET,0x1, 0)\
/*}*/																								         \

#define HI6555V3_CLASSD_KCONTROLS	\
/*{*/								    									  \
    SOC_SINGLE("CLASSD_P_SEL", 			CTRL_REG_CLASSD_CTRL1_REG,  CTRL_REG_CLASSD_P_SEL_OFFSET, 0x3, 0),\
    SOC_SINGLE("CLASSD_N_SEL", 			CTRL_REG_CLASSD_CTRL1_REG,  CTRL_REG_CLASSD_N_SEL_OFFSET, 0x3, 0)\
/*}*/

#define HI6555V3_CUSTOMER	\
/*{*/								    									  \
	SOC_ENUM_EXT("HAC",   hac_switch_enum[0], hac_switch_get, hac_switch_put), \
	SOC_SINGLE_EXT("CLASSD_SCHG_VOLTAGE_CONFIG", HI6555V3_DDR_CODEC_VIR1_ADDR, CODEC_REG_BIT0, 0xffff, 0, \
				classd_schg_voltage_get, classd_schg_voltage_put) \
/*}*/																	\

#define HI6555V3_CLASSH		\
/*{*/								    									  \
	SOC_SINGLE("CLASSH_BYPASS",				CLASSH_CTRL1_REG,		CLASSH_BYPASS_OFFSET,		0x1,	0), \
	SOC_SINGLE("CLASSH_FALL_DELAY",			CLASSH_CTRL1_REG,		FALL_DELAY_DF_CFG_OFFSET,	0x7fff,	0),	\
	SOC_SINGLE("CLASSH_DO_CFG",				CLASSH_CTRL1_REG,		DO_CFG_OFFSET,				0xf,	0),	\
	SOC_SINGLE("CLASSH_FALL_DELAY_EN",		CLASSH_CTRL1_REG,		FALL_DELAY_EN_OFFSET,		0x1,	0),	\
	SOC_SINGLE("CLASSH_DN_CFG",				CLASSH_CTRL1_REG,		DN_CFG_OFFSET,				0xf,	0),	\
	SOC_SINGLE("CLASSH_FALL_STEP",			CLASSH_CTRL2_REG,		FALL_STEP_OFFSET,			0x3f,	0),	\
	SOC_SINGLE("CLASSH_REG_VCTRL1",			CLASSH_CTRL2_REG,		REG_VCTRL1_OFFSET,			0xff,	0),	\
	SOC_SINGLE("CLASSH_EN",					CLASSH_CTRL2_REG,		CLASSH_EN_OFFSET,			0x1,	0),	\
	SOC_SINGLE("CLASSH_DYM_EN",				CLASSH_CTRL2_REG,		CLASSH_DYM_EN_OFFSET,		0x1,	0),	\
	SOC_SINGLE("CLASSH_PGA_GAIN",			CLASSH_CTRL2_REG,		CLASSH_PGA_GAIN_OFFSET,		0x1ff,	0)	\
/*}*/																	\

#define HI6555V3_LUT	\
/*{*/								    									  \
	SOC_SINGLE("LUT_REG0_3",	LUT_REG0_3_REG,			LUT_REG0_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG4_7",	LUT_REG4_7_REG,			LUT_REG4_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG8_11",	LUT_REG8_11_REG,		LUT_REG8_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG12_15",	LUT_REG12_15_REG,		LUT_REG12_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG16_19",	LUT_REG16_19_REG,		LUT_REG16_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG20_23",	LUT_REG20_23_REG,		LUT_REG20_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG24_27",	LUT_REG24_27_REG,		LUT_REG24_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG28_31",	LUT_REG28_31_REG,		LUT_REG28_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG32_35",	LUT_REG32_35_REG,		LUT_REG32_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG36_39",	LUT_REG36_39_REG,		LUT_REG36_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG40_43",	LUT_REG40_43_REG,		LUT_REG40_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG44_47",	LUT_REG44_47_REG,		LUT_REG44_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG48_51",	LUT_REG48_51_REG,		LUT_REG48_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG52_55",	LUT_REG52_55_REG,		LUT_REG52_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG56_59",	LUT_REG56_59_REG,		LUT_REG56_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG60_63",	LUT_REG60_63_REG,		LUT_REG60_OFFSET,	0x7fffffff,		0), \
	SOC_SINGLE("LUT_REG64_67",	LUT_REG64_67_REG,		LUT_REG64_OFFSET,	0x7fffffff,		0)	\
/*}*/																	\

static const struct snd_kcontrol_new hi6555v3_snd_controls[] = {
	HI6555V3_FIFO_FS_KCONTROLS,
	HI6555V3_I2S_KCONTROLS,
	HI6555V3_SRC_KCONTROLS,
	HI6555V3_PGA_KCONTROLS,
	HI6555V3_ANA_KCONTROLS,
	HI6555V3_CLASSD_KCONTROLS,
	HI6555V3_CUSTOMER,
	HI6555V3_CLASSH,
	HI6555V3_LUT,
};

static int hi6555v3_audio_dl_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_DN_PGA_CLKEN_OFFSET) | BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));

		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_DN_PGA_CLKEN_OFFSET) | BIT(AUDIO_L_DN_AFIFO_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_audio_dr_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_DN_PGA_CLKEN_OFFSET) | BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo/pga clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_DN_PGA_CLKEN_OFFSET) | BIT(AUDIO_R_DN_AFIFO_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
static int hi6555v3_side_tone_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* pga/fifo/src clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(SIDETONE_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(SIDETONE_PGA_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int hi6555v3_voice_dl_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo/src clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_DN_PGA_CLKEN_OFFSET)
						| BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET) | BIT(VOICE_L_DN_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_DN_PGA_CLKEN_OFFSET)
						  | BIT(VOICE_L_DN_AFIFO_CLKEN_OFFSET) | BIT(VOICE_L_DN_SRCUP_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_voice_dr_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* pga/fifo/src clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(VOICE_R_DN_PGA_CLKEN_OFFSET)
						  | BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET) | BIT(VOICE_R_DN_SRCUP_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(VOICE_R_DN_PGA_CLKEN_OFFSET)
						  | BIT(VOICE_R_DN_AFIFO_CLKEN_OFFSET)  | BIT(VOICE_R_DN_SRCUP_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_audio_ul_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_L_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_audio_ur_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;
	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(AUDIO_R_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
static int hi6555v3_voice_ul_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_L_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_voice_ur_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(VOICE_R_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}


static int hi6555v3_mic3_up_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(MIC3_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(MIC3_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(MIC3_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_echo_l_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_L_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_echo_r_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ECHO_R_UP_AFIFO_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ultr_dn_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_DN_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_DN_AFIFO_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ultr_up_fifo_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* clear fifo */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_UP_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		/* fifo clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* fifo clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ULTR_UP_AFIFO_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adc_mic3_filter_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(ADC_MIC3_FILTER_CLKEN_OFFSET) | BIT(MIC3_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ADC_MIC3_FILTER_CLKEN_OFFSET) | BIT(MIC3_UP_PGA_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adcl_filter_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(ADCL_FILTER_CLKEN_OFFSET) | BIT(AUDIO_L_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ADCL_FILTER_CLKEN_OFFSET) | BIT(AUDIO_L_UP_PGA_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adcr_filter_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/pga clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(ADCR_FILTER_CLKEN_OFFSET) | BIT(AUDIO_R_UP_PGA_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/pga clk  disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(ADCR_FILTER_CLKEN_OFFSET) | BIT(AUDIO_R_UP_PGA_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}
static int hi6555v3_dacl_filter_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter/sdm clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(DACL_FILTER_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter/sdm clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(DACL_FILTER_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_sdml_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* src16 enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(DACL_UP16_CLKEN_OFFSET));

		/* sif smt dacl enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(SDM_L_CLKEN_OFFSET));
		hi6555v3_set_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DACL_EN_OFFSET) | BIT(SDM_L_CALT_VLD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* src16 disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(DACL_UP16_CLKEN_OFFSET));

		/* sif smt dacl disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(SDM_L_CLKEN_OFFSET));
		hi6555v3_clr_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DACL_EN_OFFSET) | BIT(SDM_L_CALT_VLD_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_dacr_filter_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* src/filter/src16/sdm clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(DACR_SRCUP_CLKEN_OFFSET) | BIT(DACR_FILTER_CLKEN_OFFSET)
						   |BIT(DACR_UP16_CLKEN_OFFSET) | BIT(SDM_R_CLKEN_OFFSET));/*lint !e648*/

		/* sif smt dacr enable */
		hi6555v3_set_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DACR_EN_OFFSET) | BIT(SDM_R_CALT_VLD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* src/filter/src16/sdm clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(DACR_SRCUP_CLKEN_OFFSET) | BIT(DACR_FILTER_CLKEN_OFFSET)
						   |BIT(DACR_UP16_CLKEN_OFFSET) | BIT(SDM_R_CLKEN_OFFSET));/*lint !e648*/

		/* sif smt dacr disable */
		hi6555v3_clr_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DACR_EN_OFFSET) | BIT(SDM_R_CALT_VLD_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_dacl_ultr_filter_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* ultr dacl configuration */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(DACML_CLKEN_OFFSET));
		hi6555v3_set_reg_bits(DACML_FILTER_CTRL_REG, BIT(DACML_HBF1I_BYPASS_EN_OFFSET));
		hi6555v3_clr_reg_bits(DACML_FILTER_CTRL_REG, BIT(DACML_HBF2I_BYPASS_EN_OFFSET) | BIT(DACML_COMPI_BYPASS_EN_OFFSET));

		/* src ultr mux */
		hi6555v3_clr_reg_bits(CODEC_DIN_MUX1_REG, BIT(ULTR_UP16_DIN_SEL_OFFSET));

		/* src up and sdm clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(DACML_UP16_CLKEN_OFFSET) | BIT(SDM_ULTR_CLKEN_OFFSET));

		/* sdm calt enable */
		hi6555v3_set_reg_bits(DACML_FILTER_CTRL_REG, BIT(SDM_ULTR_CALT_VLD_OFFSET));

		/* sdm dither disable */
		hi6555v3_clr_reg_bits(DACML_FILTER_CTRL_REG, BIT(SDM_ULTR_DITHER_OFFSET));

		/* sif smt dac ultr enable */
		hi6555v3_set_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DAC_ULTR_EN_OFFSET));

		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* ultr dacl disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(DACML_CLKEN_OFFSET));
		hi6555v3_clr_reg_bits(DACML_FILTER_CTRL_REG, BIT(DACML_HBF1I_BYPASS_EN_OFFSET));

		/* src up and sdm clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(DACML_UP16_CLKEN_OFFSET) | BIT(SDM_ULTR_CLKEN_OFFSET));

		/* sdm calt disable */
		hi6555v3_clr_reg_bits(DACML_FILTER_CTRL_REG, BIT(SDM_ULTR_CALT_VLD_OFFSET));

		/* sif smt dac ultr disable */
		hi6555v3_clr_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_MST_DAC_ULTR_EN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adc_ultr_filter_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_ULTR_CLKEN_OFFSET));
		/* hbfvd filter bypass */
		hi6555v3_set_reg_bits(ADC_ULTR_FILTER_CTRL_REG, BIT(ADC_ULTR_HBFVD_BYPASS_EN_OFFSET));
		/* hpf hbf2d filter work */
		hi6555v3_clr_reg_bits(ADC_ULTR_FILTER_CTRL_REG, BIT(ADC_ULTR_HPF_BYPASS_EN_OFFSET) | BIT(ADC_ULTR_HBF2D_BYPASS_EN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_ULTR_CLKEN_OFFSET));
		/* hbfvd filter work */
		hi6555v3_clr_reg_bits(ADC_ULTR_FILTER_CTRL_REG, BIT(ADC_ULTR_HBFVD_BYPASS_EN_OFFSET));
		/* hpf hbf2d filter bypass */
		hi6555v3_set_reg_bits(ADC_ULTR_FILTER_CTRL_REG, BIT(ADC_ULTR_HPF_BYPASS_EN_OFFSET) | BIT(ADC_ULTR_HBF2D_BYPASS_EN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adc_spai_filter_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_SPI_CLKEN_OFFSET));
		/* ADC SPAI MUX */
		hi6555v3_set_reg_bits(CODEC_DIN_MUX1_REG, BIT(SPAI_ADC_DIN_SEL_OFFSET));
		/* SIF ADC_SPA enable */
		hi6555v3_set_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_SPA_ADC_EN_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_SPI_CLKEN_OFFSET));
		/* ADC SPAI MUX */
		hi6555v3_clr_reg_bits(CODEC_DIN_MUX1_REG, BIT(SPAI_ADC_DIN_SEL_OFFSET));
		/* SIF ADC_SPA disable */
		hi6555v3_clr_reg_bits(DAC_FILTER_CTRL_REG, BIT(SIF_SPA_ADC_EN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_adc_spav_filter_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* filter clk enable */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_SPV_CLKEN_OFFSET));
		/* ADC SPAV MUX */
		hi6555v3_clr_reg_bits(CODEC_DIN_MUX1_REG, BIT(SPAV_ADC_DIN_SEL_OFFSET));

		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* filter clk disable */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(ADC_SPV_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ana_dacl_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* dem/dacl clk enable */
		hi6555v3_set_reg_bits(DAC_CHAN_CTRL_REG, BIT(DACL_DWA_EN_OFFSET));

		hi6555v3_clr_reg_bits(CODEC_ANA_RW4_REG, BIT(DACL_PD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* dem/dacl clk disable */
		hi6555v3_clr_reg_bits(DAC_CHAN_CTRL_REG, BIT(DACL_DWA_EN_OFFSET));

		hi6555v3_set_reg_bits(CODEC_ANA_RW4_REG, BIT(DACL_PD_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ana_dacr_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/* dem/dacr clk enable */
		hi6555v3_set_reg_bits(DAC_CHAN_CTRL_REG, BIT(DACR_DWA_EN_OFFSET));

		hi6555v3_clr_reg_bits(CODEC_ANA_RW4_REG, BIT(DACR_PD_OFFSET));
		break;
	case SND_SOC_DAPM_PRE_PMD:
		/* dem/dacr clk disable */
		hi6555v3_clr_reg_bits(DAC_CHAN_CTRL_REG, BIT(DACR_DWA_EN_OFFSET));

		hi6555v3_set_reg_bits(CODEC_ANA_RW4_REG, BIT(DACR_PD_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ana_dac_ultr_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		hi6555v3_set_reg_bits(CLK_EN_CFG_REG, BIT(DAC_ULTR_EN_OFFSET));
		hi6555v3_set_reg_bits(DAC_CHAN_CTRL_REG, BIT(DAC_ULTR_DWA_EN_OFFSET));
		hi6555v3_clr_reg_bits(CODEC_ANA_RW4_REG, BIT(DACS_PD_OFFSET));

		break;
	case SND_SOC_DAPM_PRE_PMD:
		hi6555v3_clr_reg_bits(CLK_EN_CFG_REG, BIT(DAC_ULTR_EN_OFFSET));
		hi6555v3_clr_reg_bits(DAC_CHAN_CTRL_REG, BIT(DAC_ULTR_DWA_EN_OFFSET));
		hi6555v3_set_reg_bits(CODEC_ANA_RW4_REG, BIT(DACS_PD_OFFSET));

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_ana_spk_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		hi6555v3_reg_write(codec, CTRL_REG_CLASSD_CTRL2_REG, 0x40);

		break;
	case SND_SOC_DAPM_PRE_PMD:
		hi6555v3_reg_write(codec, CTRL_REG_CLASSD_CTRL2_REG, 0x0);

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static void hi6555v3_hplr_pga_power_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x40);
	hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x4C);
	msleep(30);
	hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x7C);
	hi6555v3_reg_write(codec, CODEC_ANA_RW19_REG, 0x8C);
	hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0xFC);
	hi6555v3_reg_write(codec, CODEC_ANA_RW24_REG, 0x36);
	hi6555v3_reg_write(codec, CODEC_ANA_RW20_REG, 0x00);

	OUT_FUNCTION;
}

static void hi6555v3_ana_dac_power_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	/* close dac */
	hi6555v3_ana_dacl_switch_event((struct snd_soc_dapm_widget *)NULL,
				(struct snd_kcontrol *)NULL, SND_SOC_DAPM_PRE_PMD);
	hi6555v3_ana_dacr_switch_event((struct snd_soc_dapm_widget *)NULL,
				(struct snd_kcontrol *)NULL, SND_SOC_DAPM_PRE_PMD);

	/* close  daclr-d */
	hi6555v3_reg_update(CLK_EN_CFG_REG, ANA_DACLR_EN_MASK, 0x0);

	OUT_FUNCTION;
}

static void hi6555v3_headphone_pop_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	hi6555v3_hplr_pga_power_off(codec);
	hi6555v3_ana_dac_power_off(codec);

	OUT_FUNCTION;
}

static void hi6555v3_classd_pop_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	hi6555v3_reg_update(CTRL_REG_CLASSD_CTRL0_REG, ANA_CLASSD_EN_MASK, 0x9);

	OUT_FUNCTION;
}

static int hi6555v3_hplr_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	unsigned int val_l,val_r;

	IN_FUNCTION;

	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		/*if hp path contain DAC DL/DR MIXER, mute audio DL&DR to avoid headphone pop*/
		val_l = hi6555v3_reg_read(soc_codec, DACL_MIXER4_CTRL0_REG);
		val_l = val_l & BIT(DACL_MIXER4_IN3_MUTE_OFFSET);
		val_r = hi6555v3_reg_read(soc_codec, DACR_MIXER4_CTRL0_REG);
		val_r = val_r & BIT(DACR_MIXER4_IN3_MUTE_OFFSET);

		if (!val_l) {
			hi6555v3_set_reg_bits(DACL_MIXER4_CTRL0_REG, BIT(DACL_MIXER4_IN3_MUTE_OFFSET));
		}
		if (!val_r) {
			hi6555v3_set_reg_bits(DACR_MIXER4_CTRL0_REG, BIT(DACR_MIXER4_IN3_MUTE_OFFSET));
		}
		if(!val_l || !val_r) {
			msleep(5);
		}

		hi6555v3_reg_write(codec, CODEC_ANA_RW20_REG, 0x88);
		hi6555v3_reg_write(codec, CODEC_ANA_RW24_REG, 0x36);
		hi6555v3_reg_write(codec, CODEC_ANA_RW19_REG, 0x8C);
		hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x7C);
		msleep(5);
		hi6555v3_reg_write(codec, CODEC_ANA_RW19_REG, 0xCC);
		hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x4C);
		hi6555v3_reg_write(codec, CODEC_ANA_RW24_REG, 0x16);
		hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x40);
		msleep(90);
		hi6555v3_reg_write(codec, CODEC_ANA_RW5_REG, 0x00);

		/*if hp path contain DAC DL/DR MIXER, unmute audio DL&DR to avoid headphone pop*/
		if (!val_l) {
			hi6555v3_clr_reg_bits(DACL_MIXER4_CTRL0_REG, BIT(DACL_MIXER4_IN3_MUTE_OFFSET));
		}
		if (!val_r) {
			hi6555v3_clr_reg_bits(DACR_MIXER4_CTRL0_REG, BIT(DACR_MIXER4_IN3_MUTE_OFFSET));
		}
		break;
	case SND_SOC_DAPM_PRE_PMD:
		hi6555v3_hplr_pga_power_off(codec);

		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static void hi6555v3_power_codec(struct snd_soc_codec *codec, int on)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);

	if (on) {
		hi6555v3_set_reg_bits(CODEC_ANA_RW10_REG,
			BIT(DISCHARGE_DISABLE_OFFSET));
#ifndef AUDIO_FACTORY_MODE
		hi6555v3_set_reg_bits(CODEC_ANA_RW12_REG, BIT(IDEL_TONE_CTRL_OFFSET));
#endif
		hi6555v3_set_reg_bits(CODEC_ANA_RW3_REG, 
			BIT(ADCL_MUTE_OFFSET) | BIT(ADCR_MUTE_OFFSET) | BIT(ADC3_MUTE_OFFSET));
		hi6555v3_set_reg_bits(CODEC_ANA_RW22_REG,
			BIT(STB_CTRL_SEC_OFFSET) | BIT(STB_CTRL_OFFSET));
		hi6555v3_set_reg_bits(CODEC_ANA_RW23_REG,
			BIT(STB_1N_OFFSET) | BIT(STB_N12DB_GAIN_OFFSET));
		hi6555v3_reg_write(codec, CODEC_ANA_RW1_REG, 0x07);
		msleep(6);
		hi6555v3_reg_write(codec, CODEC_ANA_RW1_REG, 0x03);
		hi6555v3_reg_write(codec, CODEC_ANA_RW1_REG, 0x01);
	} else {
		hi6555v3_reg_write(codec, CODEC_ANA_RW1_REG, 0x04);
	}

	OUT_FUNCTION;
}

static int hi6555v3_ana_sif_mixer_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/*codec ana/sif en*/
		hi6555v3_set_reg_bits(CLK_EN_CFG_REG, BIT(CODEC_ANA_EN_OFFSET) |BIT(SIF_EN_OFFSET));

		break;
	case SND_SOC_DAPM_POST_PMD:
		/*codec ana/sif dis*/
		hi6555v3_clr_reg_bits(CLK_EN_CFG_REG, BIT(CODEC_ANA_EN_OFFSET) |BIT(SIF_EN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static void hi6555v3_enable_ibias(struct snd_soc_codec *codec, bool enable)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	HI6555V3_LOGD("Begin, en=%d\n", enable);
	WARN_ON(!priv);

	if (!priv->ibias_hsmicbias_en && 0 == priv->ibias_work) {
		if (enable) {
			HI6555V3_LOGI("codec power up\n");
			hi6555v3_power_codec(codec, true);
		} else {
			HI6555V3_LOGI("codec power down\n");
			hi6555v3_power_codec(codec, false);
		}
	}
	HI6555V3_LOGD("End\n");
}

void ibias_hsmicbias_enable(struct snd_soc_codec *codec, bool enable)
{
	struct hi6555v3_priv *priv = NULL;

	IN_FUNCTION;

	if (!codec) {
		HI6555V3_LOGE("codec pointer is null\n");
		return;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (!priv) {
		HI6555V3_LOGE("priv pointer is null\n");
		return;
	}

	mutex_lock(&priv->ibias_mutex);
	if (enable) {
		if (!priv->ibias_hsmicbias_en) {
			hi6555v3_enable_ibias(codec, true);
			priv->ibias_hsmicbias_en = true;
		}
	} else {
		if (priv->ibias_hsmicbias_en) {
			priv->ibias_hsmicbias_en = false;
			hi6555v3_enable_ibias(codec, false);
		}
	}
	mutex_unlock(&priv->ibias_mutex);

	OUT_FUNCTION;
}

static void hi6555v3_ibias_work_enable(struct snd_soc_codec *codec, bool enable)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);

	mutex_lock(&priv->ibias_mutex);
	if (enable) {
		if (0 == priv->ibias_work) {
			hi6555v3_enable_ibias(codec, true);
		}

		WARN_ON(MAX_INT32 == priv->ibias_work);
		++priv->ibias_work;
	} else {
		if (priv->ibias_work <= 0) {
			HI6555V3_LOGE("ibias_work is %d, fail to disable ibias\n", priv->ibias_work);
			mutex_unlock(&priv->ibias_mutex);
			return;
		}

		--priv->ibias_work;

		if (0 == priv->ibias_work) {
			hi6555v3_enable_ibias(codec, false);
		}
	}
	mutex_unlock(&priv->ibias_mutex);

	OUT_FUNCTION;
}

static int hi6555v3_smt_ibias_supply_power_mode_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_ibias_work_enable(soc_codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_ibias_work_enable(soc_codec, false);
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_pll_supply_power_mode_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6555v3_priv *priv;
	int ret = 0;

	IN_FUNCTION;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	if (UDP_PLATFORM == priv->platform_type) {
		switch (event) {
		case SND_SOC_DAPM_PRE_PMU:
			priv->have_dapm = true;
			break;
		case SND_SOC_DAPM_POST_PMD:
			priv->have_dapm = false;
			break;
		default:
			HI6555V3_LOGE("power mode event err : %d\n", event);
			break;
		}
	} else {
		HI6555V3_LOGI("fpga clk is always on\n");
	}

	OUT_FUNCTION;

	return ret;
}

static int hi6555v3_dmic1_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(DMIC1_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(DMIC1_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}
	OUT_FUNCTION;
	return 0;
}

static int hi6555v3_dmic2_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(DMIC2_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(DMIC2_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}
	OUT_FUNCTION;
	return 0;
}

int hi6555v3_micbias1_mic_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_reg_update(CODEC_ANA_RW7_REG, MICBISA1_SET_MASK, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_reg_update(CODEC_ANA_RW7_REG, MICBISA1_SET_MASK, 0xff);
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}
	OUT_FUNCTION;
	return 0;
}

int hi6555v3_micbias2_mic_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_reg_update(CODEC_ANA_RW7_REG, MICBISA2_SET_MASK, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_reg_update(CODEC_ANA_RW7_REG, MICBISA2_SET_MASK, 0xff);
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}
	OUT_FUNCTION;
	return 0;
}

int hi6555v3_hpmicbias_mic_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6555v3_priv *priv;
	unsigned int irq_mask;

	IN_FUNCTION;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* to avoid irq while MBHD_COMP power up, mask all COMP irq */
		irq_mask = hi6555v3_reg_read(codec, ANA_IRQM_REG0_REG);
		hi6555v3_set_reg_bits(ANA_IRQM_REG0_REG, irq_mask | IRQ_MSK_COMP);
		hi6xxx_hs_micbias_dapm_enable(priv->mbhc, true);
		msleep(25);

		/* when pwr up finished clean it and cancel mask */
		hi6555v3_reg_write(codec, ANA_IRQ_REG0_REG, IRQ_MSK_COMP);
		hi6555v3_clr_reg_bits(ANA_IRQM_REG0_REG, IRQ_MSK_BTN_NOR);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6xxx_hs_micbias_dapm_enable(priv->mbhc, false);
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}
	OUT_FUNCTION;
	return 0;
}

int hi6555v3_adc_drv_power_mode_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_POST_PMU:
		msleep(10);
		break;
	case SND_SOC_DAPM_PRE_PMD:
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static int hi6555v3_i2s1_i2s_tx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable i2s1 mode */
		hi6555v3_clr_reg_bits(I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET));
		/* enable i2s1 tx */
		hi6555v3_set_reg_bits(I2S1_CTRL_REG, BIT(I2S1_IF_TX_ENA_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable i2s1 tx */
		hi6555v3_clr_reg_bits(I2S1_CTRL_REG, BIT(I2S1_IF_TX_ENA_OFFSET));
		break;
	default:
		HI6555V3_LOGE("i2s1_clk_i2s_tx_switch event err : %d\n", event);
		break;
	}

	return 0;
}

static int hi6555v3_i2s1_i2s_rx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/*enable i2s1 rx */
		hi6555v3_set_reg_bits(I2S1_CTRL_REG, BIT(I2S1_IF_RX_ENA_OFFSET));

		/*enable smartpa feedback SPA1 SPA3*/
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA3_UP_AFIFO_CLKEN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/*disable smartpa feedback SPA1 SPA3 */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(SPA1_UP_AFIFO_CLKEN_OFFSET) | BIT(SPA3_UP_AFIFO_CLKEN_OFFSET));

		/*disable i2s1 rx */
		hi6555v3_clr_reg_bits(I2S1_CTRL_REG, BIT(I2S1_IF_RX_ENA_OFFSET));
		break;
	default:
		HI6555V3_LOGE("i2s1_i2s_rx_switch event err : %d\n", event);
		break;
	}

	return 0;
}

static int hi6555v3_i2s1_tdm_rx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable smartpa feedback SPA1 SPA2 SPA3 SPA4 fifo switch */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG,
				BIT(SPA1_UP_AFIFO_CLKEN_OFFSET)
			   |BIT(SPA2_UP_AFIFO_CLKEN_OFFSET)
			   |BIT(SPA3_UP_AFIFO_CLKEN_OFFSET)
			   |BIT(SPA4_UP_AFIFO_CLKEN_OFFSET));
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* disable smartpa feedback SPA1 SPA2 SPA3 SPA4 fifo switch */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG,
			  BIT(SPA1_UP_AFIFO_CLKEN_OFFSET)
			 |BIT(SPA2_UP_AFIFO_CLKEN_OFFSET)
			 |BIT(SPA3_UP_AFIFO_CLKEN_OFFSET)
			 |BIT(SPA4_UP_AFIFO_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("i2s1_i2s_rx_switch event err : %d\n", event);
		break;
	}

	return 0;
}

static int hi6555v3_i2s1_tdm_tx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/*enable tdm mode */
		hi6555v3_set_reg_bits(I2S1_TDM_CTRL1_REG, BIT(S1_I2S_TDM_MODE_OFFSET));

		/*enable tdm tx */
		hi6555v3_set_reg_bits(I2S1_TDM_CTRL0_REG, BIT(S1_TDM_IF_EN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/*disable tdm tx */
		hi6555v3_clr_reg_bits(I2S1_TDM_CTRL0_REG, BIT(S1_TDM_IF_EN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("i2s1_clk_tdm_tx_switch_event event err : %d\n", event);
		break;
	}

	return 0;
}

static int hi6555v3_third_codec_dl_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:

		/*enable 3rd codec dlr fifo*/
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET));
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));

		udelay(CLEAR_FIFO_DELAY_LEN_MS);

		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET));
		hi6555v3_set_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));

		/* enable pga */
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(CODEC3_L_DN_PGA_CLKEN_OFFSET));
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(CODEC3_R_DN_PGA_CLKEN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/*disable 3rd codec dlr fifo*/
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_R_DN_AFIFO_CLKEN_OFFSET));
		hi6555v3_clr_reg_bits(CODEC_CLK_EN1_REG, BIT(CODEC3_L_DN_AFIFO_CLKEN_OFFSET));

		/* disable pga */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(CODEC3_L_DN_PGA_CLKEN_OFFSET));
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(CODEC3_R_DN_PGA_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("3rdCodec_dlr_fifo_i2s3_tx_switch event err : %d\n", event);
		break;
	}

	return 0;
}

static int hi6555v3_s2_src_i2s2_tx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/*enable s2 src downlink*/
		hi6555v3_set_reg_bits(CODEC_CLK_EN3_REG, BIT(BT_TX_L_SRCDN_CLKEN_OFFSET));

		/*enable i2s2 tx */
		hi6555v3_set_reg_bits(I2S2_PCM_CTRL_REG, BIT(I2S2_IF_TX_ENA_OFFSET));

		break;

	case SND_SOC_DAPM_POST_PMD:
		/*disable i2s2 tx */
		hi6555v3_clr_reg_bits(I2S2_PCM_CTRL_REG, BIT(I2S2_IF_TX_ENA_OFFSET));

		/*disable s2 src downlink*/
		hi6555v3_clr_reg_bits(CODEC_CLK_EN3_REG, BIT(BT_TX_L_SRCDN_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("s2_src_i2s2_clk_tx_switch event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int hi6555v3_i2s2_rx_pga_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/*enable s2 lr uplink pga clk*/
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG, BIT(BT_L_RX_PGA_CLKEN_OFFSET) | BIT(BT_R_RX_PGA_CLKEN_OFFSET));

		/*enable i2s2 rx*/
		hi6555v3_set_reg_bits(I2S2_PCM_CTRL_REG, BIT(I2S2_IF_RX_ENA_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/*disable i2s2 rx*/
		hi6555v3_clr_reg_bits(I2S2_PCM_CTRL_REG, BIT(I2S2_IF_RX_ENA_OFFSET));

		/*disable s2 lr uplink pga clk*/
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG, BIT(BT_L_RX_PGA_CLKEN_OFFSET) | BIT(BT_R_RX_PGA_CLKEN_OFFSET));

		break;
	default:
		HI6555V3_LOGE("i2s2_src_pga_event event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int hi6555v3_s2_src_rx_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* enable s2 lr uplink src clk */
		hi6555v3_set_reg_bits(CODEC_CLK_EN0_REG,
				 BIT(BT_L_RX_SRCDN_CLKEN_OFFSET)
				|BIT(BT_R_RX_SRCDN_CLKEN_OFFSET)
				|BIT(BT_L_RX_SRCUP_CLKEN_OFFSET)
				|BIT(BT_R_RX_SRCUP_CLKEN_OFFSET));
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* disable s2 lr uplink src clk */
		hi6555v3_clr_reg_bits(CODEC_CLK_EN0_REG,
				 BIT(BT_L_RX_SRCDN_CLKEN_OFFSET)
				|BIT(BT_R_RX_SRCDN_CLKEN_OFFSET)
				|BIT(BT_L_RX_SRCUP_CLKEN_OFFSET)
				|BIT(BT_R_RX_SRCUP_CLKEN_OFFSET));
		break;
	default:
		HI6555V3_LOGE("i2s2_src_rx_event event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static int hi6555v3_i2s2_clk_switch_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{

	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(soc_codec);

	IN_FUNCTION;

	WARN_ON(!priv);

	if (FPGA_PLATFORM == priv->platform_type) {
		return 0;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6555v3_i2s2_set_pinctrl_default();
		break;

	case SND_SOC_DAPM_POST_PMD:
		hi6555v3_i2s2_set_pinctrl_idle();
		break;
	default:
		HI6555V3_LOGE("i2s2_clk_switch_event event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;
	return 0;
}

static void _classd_schg_supply_enable(struct hi6555v3_priv *priv)
{
	int ret;
	struct regulator *regulator_schg;

	IN_FUNCTION;

	regulator_schg = priv->regulator_schg_boost3;
	if (IS_ERR(regulator_schg)) {
		HI6555V3_LOGE("regulator_schg_boost3 is error:%ld\n", PTR_ERR(regulator_schg));
		return;
	}

	ret = regulator_enable(regulator_schg);
	if (ret) {
		HI6555V3_LOGE("regulator_schg_boost3 enable fail, ret=%d\n", ret);
		return;
	}

	HI6555V3_LOGI("classd schg regulator enable ok\n");

	OUT_FUNCTION;
}

static void _classd_schg_supply_disable(struct hi6555v3_priv *priv)
{
	int ret;
	struct regulator *regulator_schg;

	IN_FUNCTION;

	regulator_schg = priv->regulator_schg_boost3;
	if (IS_ERR(regulator_schg)) {
		HI6555V3_LOGE("regulator_schg_boost3 is error:%ld\n", PTR_ERR(regulator_schg));
		return;
	}

	if (!regulator_is_enabled(regulator_schg)) {
		HI6555V3_LOGW("regulator_schg_boost3 is not enabled when disable\n");
		return;
	}

	ret = regulator_disable(regulator_schg);
	if (ret) {
		HI6555V3_LOGE("regulator_schg_boost3 enable fail, ret=%d\n", ret);
		return;
	}

	HI6555V3_LOGI("classd schg regulator disable ok\n");

	OUT_FUNCTION;
}

static void _classd_gpio_supply_switch(struct hi6555v3_priv *priv, bool enable)
{
	IN_FUNCTION;

	if (!gpio_is_valid(priv->gpio_classd)) {
		HI6555V3_LOGE("switch classd gpio:%d fail\n", priv->gpio_classd);
		return;
	}

	if (enable) {
		gpio_set_value(priv->gpio_classd, GPIO_LEVEL_HIGH);
		HI6555V3_LOGI("pull up classd gpio ok\n");
	} else {
		gpio_set_value(priv->gpio_classd, GPIO_LEVEL_LOW);
		HI6555V3_LOGI("pull down classd gpio ok\n");
	}

	OUT_FUNCTION;
}

static void _classd_gpio_supply_enable(struct hi6555v3_priv *priv)
{
	IN_FUNCTION;
	_classd_gpio_supply_switch(priv, true);
	OUT_FUNCTION;
}

static void _classd_gpio_supply_disable(struct hi6555v3_priv *priv)
{
	IN_FUNCTION;
	_classd_gpio_supply_switch(priv, false);
	OUT_FUNCTION;
}

static int hi6555v3_classd_supply_power_mode_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6555v3_priv *priv;
	enum hi6555v3_classd_supply_type supply_type;

	IN_FUNCTION;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(!codec);

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	supply_type = priv->classd_supply_type;

	if (supply_type >= CLASSD_SUPPLY_INVALID)
	{
		HI6555V3_LOGE("classd supply type is invalid:%d\n", supply_type);
		return 0;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (CLASSD_SUPPLY_SCHARGER == supply_type) {
			_classd_schg_supply_enable(priv);
		} else if (CLASSD_SUPPLY_GPIO == supply_type) {
			_classd_gpio_supply_enable(priv);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (CLASSD_SUPPLY_SCHARGER == supply_type) {
			_classd_schg_supply_disable(priv);
		} else if (CLASSD_SUPPLY_GPIO == supply_type) {
			_classd_gpio_supply_disable(priv);
		}
		break;
	default:
		HI6555V3_LOGE("power mode event err : %d\n", event);
		break;
	}

	OUT_FUNCTION;

	return 0;
}

static const struct snd_kcontrol_new hi6555v3_ana_dacl_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT0, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_dacr_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACR",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT1, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_dac_ultr_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("DAC_ULTR",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT31, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_headset_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT9, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_spk_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			CTRL_REG_CLASSD_CTRL0_REG, CTRL_REG_REG_CLASSD_EN_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_rcv_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			CODEC_ANA_RW6_REG, EAR_PD_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_lout_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			CODEC_ANA_RW6_REG, LOUT_PD_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_audio_ul_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT2, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_audio_ur_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT3, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_voice_ul_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT4, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_voice_ur_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT5, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_echo_l_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT6, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_echo_r_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT7, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_mic3_up_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT8, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ultr_dn_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT28, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ultr_up_fifo_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("ENABLE",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT29, 1, 0/*NO INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_dac_dl_mixer_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("IN1_S2_L",
			DACL_MIXER4_CTRL0_REG, DACL_MIXER4_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN2_VOICE_DL",
			DACL_MIXER4_CTRL0_REG, DACL_MIXER4_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN3_AUDIO_DL",
			DACL_MIXER4_CTRL0_REG, DACL_MIXER4_IN3_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN4_SIDE_TONE_L",
			DACL_MIXER4_CTRL0_REG, DACL_MIXER4_IN4_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_dac_dr_mixer_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("IN1_S2_R",
			DACR_MIXER4_CTRL0_REG, DACR_MIXER4_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN2_VOICE_DR",
			DACR_MIXER4_CTRL0_REG, DACR_MIXER4_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN3_AUDIO_DR",
			DACR_MIXER4_CTRL0_REG, DACR_MIXER4_IN3_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("IN4_SIDE_TONE_R",
			DACR_MIXER4_CTRL0_REG, DACR_MIXER4_IN4_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_hpl_mixer_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			CODEC_ANA_RW20_REG, MIXOUT_HSL_DACL_OFFSET, 1, 0/* INVERT */),
	SOC_DAPM_SINGLE("DACR",
			CODEC_ANA_RW20_REG, MIXOUT_HSL_DACR_OFFSET, 1, 0/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_hpr_mixer_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			CODEC_ANA_RW20_REG, MIXOUT_HSR_DACL_OFFSET, 1, 0/* INVERT */),
	SOC_DAPM_SINGLE("DACR",
			CODEC_ANA_RW20_REG, MIXOUT_HSR_DACR_OFFSET, 1, 0/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_sif_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("SIF_DACL_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT11, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_DACR_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT12, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_DACUL_EN",
			HI6555V3_DDR_CODEC_VIR1_ADDR, CODEC_REG_BIT2, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_DAC_SPA_EN",
			HI6555V3_DDR_CODEC_VIR1_ADDR, CODEC_REG_BIT3, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_ADCL_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT13, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_ADCR_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT14, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_ADC3_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT15, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("SIF_ADC_SPA_EN",
			HI6555V3_DDR_CODEC_VIR1_ADDR, CODEC_REG_BIT1, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_sif_en_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("ANA_SIFL_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT16, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("ANA_SIFR_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT17, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("ANA_MUX3_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT18, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("ANA_MUX4_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT19, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("ANA_MUX5_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT20, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("ANA_SIF_ULTR_EN",
			HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT30, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ear_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
		CODEC_ANA_RW25_REG, MIXOUT_EAR_DACL_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DACR",
		CODEC_ANA_RW25_REG, MIXOUT_EAR_DACR_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DAC_ULTR",
		CODEC_ANA_RW25_REG, MIXOUT_EAR_DAC_ULTRA_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_lineout_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
		CODEC_ANA_RW27_REG, LOMIX_SEL_DACL_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DACR",
		CODEC_ANA_RW27_REG, LOMIX_SEL_DACR_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DAC_ULTR",
		CODEC_ANA_RW27_REG, LOMIX_SEL_DAC_ULTRA_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_adc1_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			CODEC_ANA_RW12_REG, ADCL_MIXIN_DACL_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DACR",
			CODEC_ANA_RW12_REG, ADCL_MIXIN_DACR_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("MAIN MIC",
			CODEC_ANA_RW12_REG, ADCL_MIXIN_MICPGA1_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("AUX MIC",
			CODEC_ANA_RW12_REG, ADCL_MIXIN_MICPGA2_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_adc2_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			CODEC_ANA_RW14_REG, ADCR_MIXIN_DACL_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DACR",
			CODEC_ANA_RW14_REG, ADCR_MIXIN_DACR_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("MAIN MIC",
			CODEC_ANA_RW14_REG, ADCR_MIXIN_MICPGA1_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("AUX MIC",
			CODEC_ANA_RW14_REG, ADCR_MIXIN_MICPGA2_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ana_adc3_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL",
			CODEC_ANA_RW16_REG, ADC3_MIXIN_DACL_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("DACR",
			CODEC_ANA_RW16_REG, ADC3_MIXIN_ULTRA_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("MAIN MIC",
			CODEC_ANA_RW16_REG, ADC3_MIXIN_MICPGA1_OFFSET, 1, 0/* not INVERT */),
	SOC_DAPM_SINGLE("MIC3",
			CODEC_ANA_RW16_REG, ADC3_MIXIN_MICPGA3_OFFSET, 1, 0/* not INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_s2_mixer_dapm_controls[] = {
	SOC_DAPM_SINGLE("S2_DACL_MIXER",
			BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("S2_DACR_MIXER",
			BT_TX_MIXER2_CTRL_REG, BT_TX_MIXER2_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_ultr_mixer2_dapm_controls[] = {
	SOC_DAPM_SINGLE("DACL_SRC_OUT",
			ULTR_MIXER2_CTRL_REG, ULTR_MIXER2_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("ULTR_D",
			ULTR_MIXER2_CTRL_REG, ULTR_MIXER2_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_dacml_mixer2_dapm_controls[] = {
	SOC_DAPM_SINGLE("VOICE_DL",
			DACML_MIXER2_CTRL_REG, DACML_MIXER2_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("ULTR_D",
			DACML_MIXER2_CTRL_REG, DACML_MIXER2_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const struct snd_kcontrol_new hi6555v3_dacmr_mixer2_dapm_controls[] = {
	SOC_DAPM_SINGLE("VOICE_DR",
			DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_IN1_MUTE_OFFSET, 1, 1/* INVERT */),
	SOC_DAPM_SINGLE("ULTR_D",
			DACMR_MIXER2_CTRL_REG, DACMR_MIXER2_IN2_MUTE_OFFSET, 1, 1/* INVERT */),
};

static const char *hi6555v3_dacl_mux_texts[] = {
	"DACL_MIXER",
	"S2_L",
	"S1_L",
};

static const struct soc_enum hi6555v3_dacl_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, DACL_SRCUP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_dacl_mux_texts), hi6555v3_dacl_mux_texts);

static const struct snd_kcontrol_new hi6555v3_dacl_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_dacl_mux_enum);

static const char *hi6555v3_dacr_mux_texts[] = {
	"DACR_MIXER",
	"S2_R",
	"S1_R",
};

static const struct soc_enum hi6555v3_dacr_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, DACR_SRCUP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_dacr_mux_texts), hi6555v3_dacr_mux_texts);

static const struct snd_kcontrol_new hi6555v3_dacr_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_dacr_mux_enum);

static const char *hi6555v3_adc1_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_L",
};

static const struct soc_enum hi6555v3_adc1_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADCL_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_adc1_mux_texts), hi6555v3_adc1_mux_texts);

static const struct snd_kcontrol_new hi6555v3_adc1_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_adc1_mux_enum);

static const char *hi6555v3_adc2_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_R",
};

static const struct soc_enum hi6555v3_adc2_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADCR_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_adc2_mux_texts), hi6555v3_adc2_mux_texts);

static const struct snd_kcontrol_new hi6555v3_adc2_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_adc2_mux_enum);

static const char *hi6555v3_adc3_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC2_L",
};

static const struct soc_enum hi6555v3_adc3_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, ADC_MIC3_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_adc3_mux_texts), hi6555v3_adc3_mux_texts);

static const struct snd_kcontrol_new hi6555v3_adc3_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_adc3_mux_enum);

static const char *hi6555v3_ultr_dl_mux_texts[] = {
	"ULTR_D_FIFO",
	"ADC_ULTR",
};

static const struct soc_enum hi6555v3_ultr_dl_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, ULTR_DN_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ultr_dl_mux_texts), hi6555v3_ultr_dl_mux_texts);

static const struct snd_kcontrol_new hi6555v3_ultr_dl_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ultr_dl_mux_enum);

static const char *hi6555v3_dac_ultr_mux_texts[] = {
	"ULTR_D",
	"ULTR_MIXER",
};

static const struct soc_enum hi6555v3_dac_ultr_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, DACML_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_dac_ultr_mux_texts), hi6555v3_dac_ultr_mux_texts);

static const struct snd_kcontrol_new hi6555v3_dac_ultr_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_dac_ultr_mux_enum);

static const char *hi6555v3_adc_ultr_mux_texts[] = {
	"ADC1",
	"ADC2",
	"ADC3",
	"DMIC1_L",
	"DMIC1_R",
	"DMIC2_L",
};

static const struct soc_enum hi6555v3_adc_ultr_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, ADC_ULTR_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_adc_ultr_mux_texts), hi6555v3_adc_ultr_mux_texts);

static const struct snd_kcontrol_new hi6555v3_adc_ultr_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_adc_ultr_mux_enum);

static const char *hi6555v3_ana_dacl_mux_texts[] = {
	"SIFL",
	"ADC1",
	"ADC2",
	"ADC3",
};

static const struct soc_enum hi6555v3_ana_dacl_mux_enum =
SOC_ENUM_SINGLE(DAC_CHAN_CTRL_REG, DACL_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ana_dacl_mux_texts), hi6555v3_ana_dacl_mux_texts);

static const struct snd_kcontrol_new hi6555v3_ana_dacl_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ana_dacl_mux_enum);

static const char *hi6555v3_ana_dacr_mux_texts[] = {
	"SIFR",
	"ADC1",
	"ADC2",
	"ADC3",
};

static const struct soc_enum hi6555v3_ana_dacr_mux_enum =
SOC_ENUM_SINGLE(DAC_CHAN_CTRL_REG, DACR_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ana_dacr_mux_texts), hi6555v3_ana_dacr_mux_texts);

static const struct snd_kcontrol_new hi6555v3_ana_dacr_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ana_dacr_mux_enum);

static const char *hi6555v3_audio_ul_mux_texts[] = {
	"DACL_MIXER",
	"ADCL",
	"S2_L_SRC",
};

static const struct soc_enum hi6555v3_audio_ul_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_AUDIO_L_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_audio_ul_mux_texts), hi6555v3_audio_ul_mux_texts);

static const struct snd_kcontrol_new hi6555v3_audio_ul_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_audio_ul_mux_enum);

static const char *hi6555v3_audio_ur_mux_texts[] = {
	"DACR_MIXER",
	"ADCR",
	"S2_R_SRC",
};

static const struct soc_enum hi6555v3_audio_ur_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_AUDIO_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_audio_ur_mux_texts), hi6555v3_audio_ur_mux_texts);

static const struct snd_kcontrol_new hi6555v3_audio_ur_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_audio_ur_mux_enum);

static const char *hi6555v3_side_tone_mux_texts[] = {
	"ADC_MIC3_I",
	"ADCR_I",
	"ADCL_I",
};

static const struct soc_enum hi6555v3_side_tone_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, SIDETONE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_side_tone_mux_texts), hi6555v3_side_tone_mux_texts);

static const struct snd_kcontrol_new hi6555v3_side_tone_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_side_tone_mux_enum);


static const char *hi6555v3_voice_ul_mux[] = {
	"ADCL_SRC",
	"S2_L",
};

static const struct soc_enum hi6555v3_voice_ul_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_VOICE_L_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_voice_ul_mux), hi6555v3_voice_ul_mux);

static const struct snd_kcontrol_new hi6555v3_voice_ul_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_voice_ul_mux_enum);

static const char *hi6555v3_voice_ur_mux[] = {
	"ADCR_SRC",
	"S2_R",
};

static const struct soc_enum hi6555v3_voice_ur_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_VOICE_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_voice_ur_mux), hi6555v3_voice_ur_mux);

static const struct snd_kcontrol_new hi6555v3_voice_ur_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_voice_ur_mux_enum);

static const char *hi6555v3_mic3_up_mux[] = {
	"ADC3/ADC2/ADC1",
	"ADC MIC3_SRC",
};

static const struct soc_enum hi6555v3_mic3_up_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, MIC3_UP_AFIFO_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_mic3_up_mux), hi6555v3_mic3_up_mux);

static const struct snd_kcontrol_new hi6555v3_mic3_up_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_mic3_up_mux_enum);

static const char *hi6555v3_echo_l_mux[] = {
	"DACL_MIXER",
	"S3_L",
	"ADC_SPAI"
};

static const struct soc_enum hi6555v3_echo_l_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_ECHO_L_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_echo_l_mux), hi6555v3_echo_l_mux);

static const struct snd_kcontrol_new hi6555v3_echo_l_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_echo_l_mux_enum);

static const char *hi6555v3_echo_r_mux[] = {
	"DACR_MIXER",
	"S3_R",
	"ADC_SPAV",
};

static const struct soc_enum hi6555v3_echo_r_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX_REG, BM_26TO24_ECHO_R_UP_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_echo_r_mux), hi6555v3_echo_r_mux);

static const struct snd_kcontrol_new hi6555v3_echo_r_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_echo_r_mux_enum);


static const char *hi6555v3_src10_mux[] = {
	"BYPASS",
	"SRC10",
};

static const struct soc_enum hi6555v3_src10_mux_enum =
SOC_ENUM_SINGLE(CODEC_CLK_EN1_REG, ECHO_L_UP_SRCDN_CLKEN_OFFSET,
		ARRAY_SIZE(hi6555v3_src10_mux), hi6555v3_src10_mux);

static const struct snd_kcontrol_new hi6555v3_src10_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_src10_mux_enum);


static const char *hi6555v3_src11_mux[] = {
	"BYPASS",
	"SRC11",
};

static const struct soc_enum hi6555v3_src11_mux_enum =
SOC_ENUM_SINGLE(CODEC_CLK_EN1_REG, ECHO_R_UP_SRCDN_CLKEN_OFFSET,
		ARRAY_SIZE(hi6555v3_src11_mux), hi6555v3_src11_mux);

static const struct snd_kcontrol_new hi6555v3_src11_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_src11_mux_enum);


static const char *hi6555v3_ana_mux3_mux[] = {
	"ADC1",
	"SIFL",
	"SIFR",
};

static const struct soc_enum hi6555v3_ana_mux3_mux_enum =
SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADCL_DOUT_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ana_mux3_mux), hi6555v3_ana_mux3_mux);

static const struct snd_kcontrol_new hi6555v3_ana_mux3_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ana_mux3_mux_enum);

static const char *hi6555v3_ana_mux4_mux[] = {
	"ADC2",
	"SIFL",
	"SIFR",
};

static const struct soc_enum hi6555v3_ana_mux4_mux_enum =
SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADCR_DOUT_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ana_mux4_mux), hi6555v3_ana_mux4_mux);

static const struct snd_kcontrol_new hi6555v3_ana_mux4_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ana_mux4_mux_enum);

static const char *hi6555v3_ana_mux5_mux[] = {
	"ADC3",
	"SIFL",
	"SIFR",
};

static const struct soc_enum hi6555v3_ana_mux5_mux_enum =
SOC_ENUM_SINGLE(ADC_CHAN_CTRL_REG, ADC_MIC3_DOUT_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_ana_mux5_mux), hi6555v3_ana_mux5_mux);

static const struct snd_kcontrol_new hi6555v3_ana_mux5_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ana_mux5_mux_enum);

static const char *hi6555v3_main_hp_mic_mux[] = {
	"MUTE",
	"MAIN_MIC",
	"HP_MIC",
};

static const struct soc_enum hi6555v3_main_hp_mic_mux_enum =
SOC_ENUM_SINGLE(CODEC_ANA_RW9_REG, MAINPGA_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_main_hp_mic_mux), hi6555v3_main_hp_mic_mux);

static const struct snd_kcontrol_new hi6555v3_main_hp_mic_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_main_hp_mic_mux_enum);

static const char *hi6555v3_i2s1_down_l_mux_texts[] = {
	"ADCL",
	"DACL_MIXER",
	"DACML_MIXER2",
};

static const struct soc_enum hi6555v3_i2s1_down_l_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S1_PCM_TX_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_i2s1_down_l_mux_texts), hi6555v3_i2s1_down_l_mux_texts);

static const struct snd_kcontrol_new hi6555v3_i2s1_down_l_mux_dapm_controls =
SOC_DAPM_ENUM("Mux",hi6555v3_i2s1_down_l_mux_enum);

static const char *hi6555v3_i2s1_down_r_mux_texts[] = {
	"ADCR",
	"DACR_MIXER",
	"DACMR_MIXER2",
};

static const struct soc_enum hi6555v3_i2s1_down_r_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S1_PCM_TX_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_i2s1_down_r_mux_texts), hi6555v3_i2s1_down_r_mux_texts);

static const struct snd_kcontrol_new hi6555v3_i2s1_down_r_mux_dapm_controls =
SOC_DAPM_ENUM("Mux",hi6555v3_i2s1_down_r_mux_enum);

static const char *hi6555v3_i2s2_txr_mux_texts[] = {
	"S2_TXL",
	"DACR_MIXER",
};

static const struct soc_enum hi6555v3_i2s2_txr_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S2_TX_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_i2s2_txr_mux_texts), hi6555v3_i2s2_txr_mux_texts);

static const struct snd_kcontrol_new hi6555v3_i2s2_txr_mux_dapm_controls =
SOC_DAPM_ENUM("Mux",hi6555v3_i2s2_txr_mux_enum);

static const char *hi6555v3_i2s2_txl_mux_texts[] = {
	"S2_MIXER",
	"DACL_MIXER",
};

static const struct soc_enum hi6555v3_i2s2_txl_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S2_TX_L_SRCDN_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_i2s2_txl_mux_texts), hi6555v3_i2s2_txl_mux_texts);

static const struct snd_kcontrol_new hi6555v3_i2s2_txl_mux_dapm_controls =
SOC_DAPM_ENUM("Mux",hi6555v3_i2s2_txl_mux_enum);

static const char *hi6555v3_i2s3_s1r_mux_texts[] = {
	"THIRD_CODEC_DL",
	"ADC_ULTR",
};

static const struct soc_enum hi6555v3_i2s3_s1r_mux_enum =
SOC_ENUM_SINGLE(CODEC_DIN_MUX1_REG, I2S3_TX_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6555v3_i2s3_s1r_mux_texts), hi6555v3_i2s3_s1r_mux_texts);

static const struct snd_kcontrol_new hi6555v3_i2s3_s1r_mux_dapm_controls =
SOC_DAPM_ENUM("Mux",hi6555v3_i2s3_s1r_mux_enum);

/* sif ad_da loopback mux */
static const char *hi6555v3_ad_da_loopback_mux_texts[] = {
	"NORMAL",
	"AD_DA_LOOP",
	"MIC3_DA_LOOP",
};

static const struct soc_enum hi6555v3_ad_da_loopback_mux_enum =
SOC_ENUM_SINGLE(DAC_FILTER_CTRL_REG, SIF_MST_P2S_LOOPBACK_OFFSET,
		ARRAY_SIZE(hi6555v3_ad_da_loopback_mux_texts), hi6555v3_ad_da_loopback_mux_texts);

static const struct snd_kcontrol_new hi6555v3_ad_da_loopback_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_ad_da_loopback_mux_enum);

/* sif da_ad loopback mux */
static const char *hi6555v3_da_ad_loopback_mux_texts[] = {
	"NORMAL",
	"DALR_ADLR_LOOP",
	"DA_SOUT_AD_SIN_LOOP",
	"DA_SOUT_MIC3_SIN_LOOP",
};

static const struct soc_enum hi6555v3_da_ad_loopback_mux_enum =
SOC_ENUM_SINGLE(DAC_FILTER_CTRL_REG, SIF_MST_S2P_LOOPBACK_OFFSET,
		ARRAY_SIZE(hi6555v3_da_ad_loopback_mux_texts), hi6555v3_da_ad_loopback_mux_texts);

static const struct snd_kcontrol_new hi6555v3_da_ad_loopback_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_da_ad_loopback_mux_enum);

/* dac -> adc  loopback mux */
static const char *hi6555v3_adc_loopback_mux_texts[] = {
	"NORMAL",
	"DA_AD_LOOP",
	"DA_MIC3_LOOP",
};

static const struct soc_enum hi6555v3_adc_loopback_mux_enum =
SOC_ENUM_SINGLE(SIF_LOOPBACK_CFG_REG, ADC_LOOPBACK_OFFSET,
		ARRAY_SIZE(hi6555v3_adc_loopback_mux_texts), hi6555v3_adc_loopback_mux_texts);

static const struct snd_kcontrol_new hi6555v3_adc_loopback_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_adc_loopback_mux_enum);

/* dac_loopback mux */
static const char *hi6555v3_dac_loopback_mux_texts[] = {
	"NORMAL",
	"DOUT_SDIN",
	"MIC3_SDIN",
};

static const struct soc_enum hi6555v3_dac_loopback_mux_enum =
SOC_ENUM_SINGLE(SIF_LOOPBACK_CFG_REG, DAC_LOOPBACK_OFFSET,
		ARRAY_SIZE(hi6555v3_dac_loopback_mux_texts), hi6555v3_dac_loopback_mux_texts);

static const struct snd_kcontrol_new hi6555v3_dac_loopback_mux_dapm_controls =
SOC_DAPM_ENUM("Mux", hi6555v3_dac_loopback_mux_enum);

static const struct snd_kcontrol_new hi6555v3_i2s1_i2s_tx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S_SWITCH_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT21, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s1_tdm_tx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("TDM_SWITCH_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT22, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_s2_src_i2s2_tx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S2_SWITCH_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT23, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s1_i2s_rx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S1_I2S_RX_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT25, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s1_tdm_rx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S1_TDM_RX_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT26, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_s2_src_rx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S2_SRC_RX_EN", HI6555V3_DDR_CODEC_VIR0_ADDR, CODEC_REG_BIT27, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s3_rx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S3_RX_EN", I2S3_PCM_CTRL_REG, I2S3_IF_RX_ENA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s3_tx_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S3_TX_EN", I2S3_PCM_CTRL_REG, I2S3_IF_TX_ENA_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s1_clk_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S1_CLK_EN", CODEC_CLK_EN1_REG, I2S1_TDM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s2_clk_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S2_CLK_EN", CODEC_CLK_EN1_REG, I2S2_PCM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_i2s3_clk_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("I2S3_CLK_EN", CODEC_CLK_EN1_REG, I2S3_PCM_CLKEN_OFFSET, 1, 0),
};

static const struct snd_kcontrol_new hi6555v3_dac_spa_switch_dapm_controls[] = {
	SOC_DAPM_SINGLE("DAC_SPA_EN", DAC_FILTER_CTRL_REG, SIF_SPA_DAC_EN_OFFSET, 1, 0),
};

#define HI6555V3_INPUT_WIDGET	\
	SND_SOC_DAPM_INPUT("AUDIO_DLINK INPUT"),	\
	SND_SOC_DAPM_INPUT("SMT_ANA_DAC_SDATA INPUT"),	\
	SND_SOC_DAPM_INPUT("VOICE_DLINK INPUT"),	\
	SND_SOC_DAPM_INPUT("ADC1 INPUT"),	\
	SND_SOC_DAPM_INPUT("ADC2 INPUT"),	\
	SND_SOC_DAPM_INPUT("ADC3 INPUT"),	\
	SND_SOC_DAPM_INPUT("DMIC1 INPUT"),	\
	SND_SOC_DAPM_INPUT("DMIC2 INPUT"),	\
	SND_SOC_DAPM_INPUT("ADC_SPA INPUT"),	\
	SND_SOC_DAPM_INPUT("ULTR_DLINK INPUT"),	\
	SND_SOC_DAPM_INPUT("MAINMIC INPUT"),	\
	SND_SOC_DAPM_INPUT("HPMIC INPUT"),	\
	SND_SOC_DAPM_INPUT("AUXMIC INPUT"),	\
	SND_SOC_DAPM_INPUT("MIC3 INPUT"),	\
	SND_SOC_DAPM_INPUT("I2S1_2_3 INPUT"),	\

#define HI6555V3_OUTPUT_WIDGET	\
	SND_SOC_DAPM_OUTPUT("AUDIO_DLINK OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("AUDIO_UL OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("AUDIO_UR OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("VOICE_UL OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("VOICE_UR OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("MIC3_UP OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("ECHO OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("ULTR_UP OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("ULTR_DLINK OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("DAC_SPA OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("SMT_HP_LR OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("ANA_EAR OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("ANA_SIF OUTPUT"),	\
	SND_SOC_DAPM_OUTPUT("I2S1_2_3 OUTPUT"),	\

#define HI6555V3_PGA_WIDGET	\
	SND_SOC_DAPM_PGA_E("AUDIO_DL PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_audio_dl_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("AUDIO_DR PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_audio_dr_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("DACL_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_dacl_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("SDML", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_sdml_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("DACR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_dacr_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("DACL_ULTR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_dacl_ultr_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADC_ULTR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_adc_ultr_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADC_SPAI_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_adc_spai_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADC_SPAV_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_adc_spav_filter_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADCL_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_adcl_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADCR_FILTER", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_adcr_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("ADC_MIC3_FILTER", SND_SOC_NOPM,	0, 0, NULL, 0,	\
			hi6555v3_adc_mic3_filter_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("HP_LR PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_hplr_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("I2S2_RX PGA", SND_SOC_NOPM, 0, 0,	NULL, 0,	\
			hi6555v3_i2s2_rx_pga_event,	(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_PGA_E("VOICE_DL PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_voice_dl_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("VOICE_DR PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_voice_dr_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("SIDE_TONE_EN PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_side_tone_pga_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_PGA_E("THIRD_CODEC_DL PGA", SND_SOC_NOPM, 0, 0, NULL, 0,	\
			hi6555v3_third_codec_dl_pga_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_PGA("AUDIO_UP_SRC5_EN PGA", CODEC_CLK_EN1_REG,	AUDIO_L_UP_SRCUP_CLKEN_OFFSET, 0, NULL, 0),	\
	SND_SOC_DAPM_PGA("AUDIO_UP_SRC6_EN PGA", CODEC_CLK_EN1_REG,	AUDIO_R_UP_SRCUP_CLKEN_OFFSET,	0, NULL, 0),	\
	SND_SOC_DAPM_PGA("VOICE_UP_SRC7_EN PGA", CODEC_CLK_EN1_REG, VOICE_L_UP_SRCDN_CLKEN_OFFSET, 0, NULL, 0),	\
	SND_SOC_DAPM_PGA("VOICE_UP_SRC8_EN PGA", CODEC_CLK_EN1_REG, VOICE_R_UP_SRCDN_CLKEN_OFFSET, 0, NULL, 0),	\
	SND_SOC_DAPM_PGA("ADC_MIC3_SRC9_EN PGA", CODEC_CLK_EN1_REG,	MIC3_UP_SRCDN_CLKEN_OFFSET, 0, NULL, 0),	\
	SND_SOC_DAPM_PGA("ANA_MAIN PGA", CODEC_ANA_RW2_REG, MAINPGA_PD_OFFSET, 1, NULL, 0),	\
	SND_SOC_DAPM_PGA("ANA_AUX PGA", CODEC_ANA_RW2_REG, AUXPGA_PD_OFFSET, 1, NULL,0),	\
	SND_SOC_DAPM_PGA("ANA_MIC3 PGA", CODEC_ANA_RW2_REG, MIC3PGA_PD_OFFSET, 1, NULL, 0),	\
	SND_SOC_DAPM_PGA("ULTR_DLINK PGA", CODEC_CLK_EN3_REG, ULTR_DN_PGA_CLKEN_OFFSET, 0, NULL, 0),	\

#define HI6555V3_MIXER_WIDGET	\
	SND_SOC_DAPM_MIXER("DAC_DL MIXER", CODEC_CLK_EN0_REG, DACL_MIXER4_CLKEN_OFFSET, 0,	\
			hi6555v3_dac_dl_mixer_en_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_dac_dl_mixer_en_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("DAC_DR MIXER", CODEC_CLK_EN0_REG, DACR_MIXER4_CLKEN_OFFSET, 0,	\
			hi6555v3_dac_dr_mixer_en_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_dac_dr_mixer_en_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("SIF_EN MIXER", CODEC_CLK_EN1_REG, SIF_MST_CLKEN_OFFSET, 0,	\
			hi6555v3_sif_en_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_sif_en_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ANA_EAR MIXER", CODEC_ANA_RW6_REG, MIXOUT_EAR_PD_OFFSET, 1,	\
			hi6555v3_ear_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_ear_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ANA_LINEOUT MIXER", CODEC_ANA_RW6_REG, LOMIXER_PD_OFFSET, 1,	\
			hi6555v3_lineout_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_lineout_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("S2_DOWNLINK_EN MIXER", CODEC_CLK_EN0_REG, BT_TX_MIXER2_CLKEN_OFFSET, 0,	\
			hi6555v3_s2_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_s2_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ULTR_DLINK_EN MIXER", CODEC_CLK_EN3_REG, ULTR_MIXER2_CLKEN_OFFSET, 0,	\
			hi6555v3_ultr_mixer2_dapm_controls, ARRAY_SIZE(hi6555v3_ultr_mixer2_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("DACML_EN MIXER", CODEC_CLK_EN3_REG, DACML_MIXER2_CLKEN_OFFSET, 0,	\
			hi6555v3_dacml_mixer2_dapm_controls, ARRAY_SIZE(hi6555v3_dacml_mixer2_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("DACMR_EN MIXER", CODEC_CLK_EN3_REG, DACMR_MIXER2_CLKEN_OFFSET, 0,	\
			hi6555v3_dacmr_mixer2_dapm_controls, ARRAY_SIZE(hi6555v3_dacmr_mixer2_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ANA_ADC1 MIXER", CODEC_ANA_RW3_REG, ADCL_PD_OFFSET, 1,	\
			hi6555v3_ana_adc1_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_ana_adc1_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ANA_ADC2 MIXER", CODEC_ANA_RW3_REG, ADCR_PD_OFFSET, 1,	\
			hi6555v3_ana_adc2_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_ana_adc2_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER("ANA_ADC3 MIXER", CODEC_ANA_RW3_REG, ADC3_PD_OFFSET, 1,	\
			hi6555v3_ana_adc3_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_ana_adc3_mixer_dapm_controls)),	\
	SND_SOC_DAPM_MIXER_E("ANA_SIF_EN MIXER", SND_SOC_NOPM, 0, 0,	\
			hi6555v3_ana_sif_en_mixer_dapm_controls, ARRAY_SIZE(hi6555v3_ana_sif_en_mixer_dapm_controls),	\
			hi6555v3_ana_sif_mixer_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\

#define HI6555V3_MUX_WIDGET	\
	SND_SOC_DAPM_MUX("I2S1_S1L MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_i2s1_down_l_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("I2S1_S1R MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_i2s1_down_r_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("I2S2_TXL MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_i2s2_txl_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("I2S2_TXR MUX", CODEC_CLK_EN3_REG, BT_TX_R_SRCDN_CLKEN_OFFSET, 0, &hi6555v3_i2s2_txr_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("I2S3_S1R MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_i2s3_s1r_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("DAC_DL MUX", CODEC_CLK_EN0_REG, DACL_SRCUP_CLKEN_OFFSET, 0, &hi6555v3_dacl_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("DAC_DR MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_dacr_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("AUDIO_UL MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_audio_ul_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("AUDIO_UR MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_audio_ur_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("SIDE_TONE MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_side_tone_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("VOICE_UL MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_voice_ul_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("VOICE_UR MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_voice_ur_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("MIC3_UP MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_mic3_up_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ECHO_L MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_echo_l_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ECHO_R MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_echo_r_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ECHO_SRC10 MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_src10_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ECHO_SRC11 MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_src11_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("AD_DA_LOOP MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_ad_da_loopback_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("DA_AD_LOOP MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_da_ad_loopback_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ADC_LOOP MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_adc_loopback_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("DAC_LOOPBACK MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_dac_loopback_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ADC1 MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADCL_EN_OFFSET, 0, &hi6555v3_adc1_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ADC2 MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADCR_EN_OFFSET, 0,&hi6555v3_adc2_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ADC3 MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADC_MIC3_EN_OFFSET, 0, &hi6555v3_adc3_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ULTR_DL MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_ultr_dl_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("DAC_ULTR MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_dac_ultr_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ADC_ULTR MUX", DAC_FILTER_CTRL_REG, SIF_MST_ADC_MIC3_EN_OFFSET, 0, &hi6555v3_adc_ultr_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ANA_DAC_L MUX", CLK_EN_CFG_REG, DACL_EN_OFFSET, 0, &hi6555v3_ana_dacl_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ANA_DAC_R MUX", CLK_EN_CFG_REG, DACR_EN_OFFSET, 0, &hi6555v3_ana_dacr_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ANA_MUX3 MUX", CLK_EN_CFG_REG, ADCL_EN_OFFSET, 0, &hi6555v3_ana_mux3_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ANA_MUX4 MUX", CLK_EN_CFG_REG, ADCR_EN_OFFSET, 0, &hi6555v3_ana_mux4_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("ANA_MUX5 MUX", CLK_EN_CFG_REG, ADC_MIC3_EN_OFFSET, 0, &hi6555v3_ana_mux5_mux_dapm_controls),	\
	SND_SOC_DAPM_MUX("MAIN_HP_MIC MUX", SND_SOC_NOPM, 0, 0, &hi6555v3_main_hp_mic_mux_dapm_controls),	\

#define HI6555V3_SWITCH_WIDGET	\
	SND_SOC_DAPM_SWITCH_E("AUDIO_UL_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_audio_ul_fifo_switch_dapm_controls,	\
			hi6555v3_audio_ul_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("AUDIO_UR_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_audio_ur_fifo_switch_dapm_controls,	\
			hi6555v3_audio_ur_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("VOICE_UL_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_voice_ul_fifo_switch_dapm_controls,	\
			hi6555v3_voice_ul_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("VOICE_UR_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_voice_ur_fifo_switch_dapm_controls,	\
			hi6555v3_voice_ur_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("MIC3_UP_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_mic3_up_fifo_switch_dapm_controls,		\
			hi6555v3_mic3_up_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("ECHO_L_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_echo_l_fifo_switch_dapm_controls,	\
			hi6555v3_echo_l_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("ECHO_R_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_echo_r_fifo_switch_dapm_controls,	\
			hi6555v3_echo_r_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("ULTR_DLINK_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ultr_dn_fifo_switch_dapm_controls,	\
			hi6555v3_ultr_dn_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("ULTR_UP_FIFO SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ultr_up_fifo_switch_dapm_controls,	\
			hi6555v3_ultr_up_fifo_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("I2S1_I2S_TX SWITCH", SND_SOC_NOPM, 0, 0,hi6555v3_i2s1_i2s_tx_switch_dapm_controls,	\
			hi6555v3_i2s1_i2s_tx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("I2S1_TDM_TX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s1_tdm_tx_switch_dapm_controls,	\
			hi6555v3_i2s1_tdm_tx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH("I2S1_CLK SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s1_clk_switch_dapm_controls),	\
	SND_SOC_DAPM_SWITCH_E("I2S1_I2S_RX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s1_i2s_rx_switch_dapm_controls,	\
			hi6555v3_i2s1_i2s_rx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("I2S1_TDM_RX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s1_tdm_rx_switch_dapm_controls,	\
			hi6555v3_i2s1_tdm_rx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("S2_SRC_I2S2_TX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_s2_src_i2s2_tx_switch_dapm_controls,	\
			hi6555v3_s2_src_i2s2_tx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH_E("S2_SRC_RX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_s2_src_rx_switch_dapm_controls,	\
			hi6555v3_s2_src_rx_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),		\
	SND_SOC_DAPM_SWITCH_E("I2S2_CLK SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s2_clk_switch_dapm_controls,	\
			hi6555v3_i2s2_clk_switch_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SWITCH("I2S3_CLK SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s3_clk_switch_dapm_controls),	\
	SND_SOC_DAPM_SWITCH("I2S3_RX SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_i2s3_rx_switch_dapm_controls),		\
	SND_SOC_DAPM_SWITCH("I2S3_TX SWITCH", SND_SOC_NOPM,	0, 0, hi6555v3_i2s3_tx_switch_dapm_controls),		\
	SND_SOC_DAPM_SWITCH("DAC_SPA SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_dac_spa_switch_dapm_controls),	\
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_L SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ana_dacl_switch_dapm_controls,	\
			hi6555v3_ana_dacl_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),		\
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_R SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ana_dacr_switch_dapm_controls,	\
			hi6555v3_ana_dacr_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),		\
	SND_SOC_DAPM_SWITCH_E("ANA_DAC_ULTR SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ana_dac_ultr_switch_dapm_controls,	\
			hi6555v3_ana_dac_ultr_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_SWITCH("ANA_HEADSET SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ana_headset_switch_dapm_controls),	\
	SND_SOC_DAPM_SWITCH_E("ANA_SPK SWITCH", CTRL_REG_CLASSD_CTRL0_REG, CTRL_REG_CLASSD_DRV_EN_OFFSET, 0, hi6555v3_ana_spk_switch_dapm_controls, \
			hi6555v3_ana_spk_switch_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)), \
	SND_SOC_DAPM_SWITCH("ANA_RCV SWITCH", CODEC_ANA_RW6_REG, EPVCM_PD_OFFSET, 1, hi6555v3_ana_rcv_switch_dapm_controls),		\
	SND_SOC_DAPM_SWITCH("ANA_LOUT SWITCH", SND_SOC_NOPM, 0, 0, hi6555v3_ana_lout_switch_dapm_controls),		\

#define HI6555V3_SUPPLY_WIDGET	\
	SND_SOC_DAPM_SUPPLY("SMT_IBIAS SUPPLY", SND_SOC_NOPM, 0, 0,	\
			hi6555v3_smt_ibias_supply_power_mode_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SUPPLY("PLL", SND_SOC_NOPM, 0, 0,	\
			hi6555v3_pll_supply_power_mode_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\
	SND_SOC_DAPM_SUPPLY("CLASSD SUPPLY", SND_SOC_NOPM, 0, 0,	\
			hi6555v3_classd_supply_power_mode_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),	\

#define HI6555V3_MIC_WIDGET	\
	SND_SOC_DAPM_MIC("DMIC1", hi6555v3_dmic1_event),	\
	SND_SOC_DAPM_MIC("DMIC2", hi6555v3_dmic2_event),	\
	SND_SOC_DAPM_MIC("MICBIAS1 MIC", hi6555v3_micbias1_mic_event),	\
	SND_SOC_DAPM_MIC("MICBIAS2 MIC", hi6555v3_micbias2_mic_event),	\
	SND_SOC_DAPM_MIC("HPMICBIAS MIC", hi6555v3_hpmicbias_mic_event),	\

#define HI6555V3_DRV_WIDGET	\
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC1 DRV", CODEC_ANA_RW3_REG, ADCL_MUTE_OFFSET, 1, NULL, 0,	\
		hi6555v3_adc_drv_power_mode_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC2 DRV", CODEC_ANA_RW3_REG, ADCR_MUTE_OFFSET, 1, NULL, 0,	\
		hi6555v3_adc_drv_power_mode_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\
	SND_SOC_DAPM_OUT_DRV_E("ANA_ADC3 DRV",	CODEC_ANA_RW3_REG, ADC3_PD_OFFSET, 1, NULL, 0,	\
		hi6555v3_adc_drv_power_mode_event, (SND_SOC_DAPM_POST_PMU | SND_SOC_DAPM_PRE_PMD)),	\

static const struct snd_soc_dapm_widget hi6555v3_dapm_widgets[] = {
	HI6555V3_INPUT_WIDGET
	HI6555V3_OUTPUT_WIDGET
	HI6555V3_PGA_WIDGET
	HI6555V3_MIXER_WIDGET
	HI6555V3_MUX_WIDGET
	HI6555V3_SWITCH_WIDGET
	HI6555V3_SUPPLY_WIDGET
	HI6555V3_MIC_WIDGET
	HI6555V3_DRV_WIDGET
};

/*****************************************************************************
  Route Map
 *****************************************************************************/
static const struct snd_soc_dapm_route hi6555v3_route_map[] = {
	/* asp_codec audio_play */
	{"AUDIO_DR PGA",               				NULL,                  			"AUDIO_DLINK INPUT"},
	{"AUDIO_DL PGA",               				NULL,                  			"AUDIO_DLINK INPUT"},
	{"DAC_DL MIXER",            	  			"IN3_AUDIO_DL",  				"AUDIO_DL PGA"},
	{"DAC_DR MIXER",            	  			"IN3_AUDIO_DR",  				"AUDIO_DR PGA"},
	{"DAC_DL MIXER",							"IN4_SIDE_TONE_L",				"SIDE_TONE_EN PGA"},
	{"DAC_DR MIXER",							"IN4_SIDE_TONE_R",				"SIDE_TONE_EN PGA"},
	{"DAC_DL MUX",	            	  			"DACL_MIXER",  					"DAC_DL MIXER"},
	{"DAC_DR MUX",	            	  			"DACR_MIXER",  					"DAC_DR MIXER"},
	{"DACL_FILTER",     	  					NULL,  							"DAC_DL MUX"},
	{"DACR_FILTER",     	  					NULL,  							"DAC_DR MUX"},
	{"SDML",									NULL,							"DACL_FILTER"},
	{"AD_DA_LOOP MUX",							NULL,							"SDML"},
	{"AD_DA_LOOP MUX",							"NORMAL",						"DACL_FILTER"},
	{"AD_DA_LOOP MUX",							"NORMAL",						"DACR_FILTER"},
	{"AD_DA_LOOP MUX",							"AD_DA_LOOP",					"DACL_FILTER"},
	{"AD_DA_LOOP MUX",							"AD_DA_LOOP",					"DACR_FILTER"},
	{"AD_DA_LOOP MUX",							"MIC3_DA_LOOP",					"DACL_FILTER"},
	{"AD_DA_LOOP MUX",							"MIC3_DA_LOOP",					"DACR_FILTER"},
	{"SIF_EN MIXER",							"SIF_DACL_EN",					"AD_DA_LOOP MUX"},
	{"SIF_EN MIXER",							"SIF_DACR_EN",					"AD_DA_LOOP MUX"},
	{"AUDIO_DLINK OUTPUT",                      NULL,                         	"SIF_EN MIXER"},

	/* asp_codec - echo */
	{"SIF_EN MIXER",	            			"SIF_ADC_SPA_EN",				"ADC_SPA INPUT"},
	{"ADC_SPAI_FILTER",	            			NULL,							"SIF_EN MIXER"},
	{"ADC_SPAV_FILTER",	            			NULL,							"SIF_EN MIXER"},
	{"ECHO_SRC10 MUX",	            			"BYPASS",						"DAC_DL MIXER"},
	{"ECHO_SRC11 MUX",	            			"BYPASS",						"DAC_DR MIXER"},
	{"ECHO_SRC10 MUX",	            			"SRC10",						"DAC_DL MIXER"},
	{"ECHO_SRC11 MUX",	            			"SRC11",						"DAC_DR MIXER"},
	{"ECHO_L MUX",	            				"DACL_MIXER",					"ECHO_SRC10 MUX"},
	{"ECHO_R MUX",	            				"DACR_MIXER",					"ECHO_SRC11 MUX"},
	{"ECHO_L MUX",	            				"ADC_SPAI",						"ADC_SPAI_FILTER"},
	{"ECHO_R MUX",	            				"ADC_SPAV",						"ADC_SPAV_FILTER"},
	{"ECHO_L_FIFO SWITCH",	            		"ENABLE",						"ECHO_L MUX"},
	{"ECHO_R_FIFO SWITCH",	            		"ENABLE",						"ECHO_R MUX"},
	{"ECHO OUTPUT",	            				NULL,  							"ECHO_L_FIFO SWITCH"},
	{"ECHO OUTPUT",	            				NULL,  							"ECHO_R_FIFO SWITCH"},

	/* asp_codec - voice downlink */
	{"VOICE_DL PGA",               				NULL,                  			"VOICE_DLINK INPUT"},
	{"VOICE_DR PGA",               				NULL,                  			"VOICE_DLINK INPUT"},
	{"DAC_DL MIXER",            	  			"IN2_VOICE_DL",  				"VOICE_DL PGA"},
	{"DAC_DR MIXER",            	  			"IN2_VOICE_DR",  				"VOICE_DR PGA"},

	/* ana - head_phone audio_play */
	{"ANA_SIF_EN MIXER",						"ANA_SIFL_EN",                 	"SMT_ANA_DAC_SDATA INPUT"},
	{"ANA_SIF_EN MIXER",						"ANA_SIFR_EN",                  "SMT_ANA_DAC_SDATA INPUT"},
	{"ANA_DAC_L MUX",							"SIFL",                    		"ANA_SIF_EN MIXER"},
	{"ANA_DAC_R MUX",							"SIFR",                    		"ANA_SIF_EN MIXER"},
	{"ANA_DAC_L MUX",							"ADC1",							"ANA_MUX3 MUX"},
	{"ANA_DAC_L MUX",							"ADC2",							"ANA_MUX4 MUX"},
	{"ANA_DAC_L MUX",							"ADC3",							"ANA_MUX5 MUX"},
	{"ANA_DAC_R MUX",							"ADC1",							"ANA_MUX3 MUX"},
	{"ANA_DAC_R MUX",							"ADC2",							"ANA_MUX4 MUX"},
	{"ANA_DAC_R MUX",							"ADC3",							"ANA_MUX5 MUX"},
	{"ANA_DAC_L SWITCH",						"DACL",                    		"ANA_DAC_L MUX"},
	{"ANA_DAC_R SWITCH",						"DACR",                    		"ANA_DAC_R MUX"},
	{"ADC_LOOP MUX",							"NORMAL",						"ANA_DAC_L SWITCH"},
	{"ADC_LOOP MUX",							"NORMAL",						"ANA_DAC_R SWITCH"},
	{"ADC_LOOP MUX",							"DA_AD_LOOP",					"ANA_DAC_L SWITCH"},
	{"ADC_LOOP MUX",							"DA_AD_LOOP",					"ANA_DAC_R SWITCH"},
	{"ADC_LOOP MUX",							"DA_MIC3_LOOP",					"ANA_DAC_L SWITCH"},
	{"ADC_LOOP MUX",							"DA_MIC3_LOOP",					"ANA_DAC_R SWITCH"},
	{"HP_LR PGA",								NULL,                    		"ADC_LOOP MUX"},
	{"ANA_HEADSET SWITCH",						"ENABLE",                  		"HP_LR PGA"},
	{"SMT_HP_LR OUTPUT",						NULL,                    		"ANA_HEADSET SWITCH"},

	/* ana - ear audio_play */
	{"ANA_EAR MIXER",							"DACL",                    		"ADC_LOOP MUX"},
	{"ANA_EAR MIXER",							"DACR",                    		"ADC_LOOP MUX"},
	{"ANA_LINEOUT MIXER",						"DACL",                    		"ADC_LOOP MUX"},
	{"ANA_LINEOUT MIXER",						"DACR",                    		"ADC_LOOP MUX"},
	{"ANA_SPK SWITCH",							"ENABLE",                  		"ANA_LINEOUT MIXER"},
	{"ANA_RCV SWITCH",							"ENABLE",                  		"ANA_EAR MIXER"},
	{"ANA_LOUT SWITCH",							"ENABLE",              			"ANA_LINEOUT MIXER"},
	{"ANA_EAR OUTPUT",							NULL,                  			"ANA_SPK SWITCH"},
	{"ANA_EAR OUTPUT",							NULL,                  			"ANA_RCV SWITCH"},
	{"ANA_EAR OUTPUT",							NULL,                  			"ANA_LOUT SWITCH"},

	/* asp_codec - capture */
	{"SIF_EN MIXER",                          	"SIF_ADCL_EN",                  "ADC1 INPUT"},
	{"SIF_EN MIXER",                          	"SIF_ADCR_EN",                  "ADC2 INPUT"},
	{"DA_AD_LOOP MUX",							"NORMAL",						"SIF_EN MIXER"},
	{"DA_AD_LOOP MUX",							"DALR_ADLR_LOOP",				"SIF_EN MIXER"},
	{"DA_AD_LOOP MUX",							"DA_SOUT_AD_SIN_LOOP",			"SIF_EN MIXER"},
	{"DA_AD_LOOP MUX",                          "DA_SOUT_MIC3_SIN_LOOP",        "SIF_EN MIXER"},
	{"ADC1 MUX",								"ADC1",							"DA_AD_LOOP MUX"},
	{"ADC2 MUX",								"ADC2",							"DA_AD_LOOP MUX"},
	{"ADCL_FILTER",             				NULL,                  			"ADC1 MUX"},
	{"ADCR_FILTER",            		 			NULL,                  			"ADC2 MUX"},
	{"AUDIO_UL MUX",            				"ADCL",                  		"ADCL_FILTER"},
	{"AUDIO_UR MUX",             				"ADCR",                  		"ADCR_FILTER"},
	{"AUDIO_UL MUX",							"DACL_MIXER",					"DAC_DL MIXER"},
	{"AUDIO_UR MUX",							"DACR_MIXER",					"DAC_DR MIXER"},
	{"SIDE_TONE MUX",							"ADCL_I",						"ADCL_FILTER"},
	{"SIDE_TONE MUX",							"ADCR_I",						"ADCL_FILTER"},
	{"SIDE_TONE MUX",							"ADC_MIC3_I",					"ADCL_FILTER"},
	{"SIDE_TONE_EN PGA",						NULL,							"SIDE_TONE MUX"},
	{"I2S1_S1L MUX",							"ADCL",							"ADCL_FILTER"},
	{"I2S1_S1R MUX",							"ADCR",							"ADCR_FILTER"},
	{"AUDIO_UL_FIFO SWITCH",	            	"ENABLE",  						"AUDIO_UL MUX"},
	{"AUDIO_UR_FIFO SWITCH",	            	"ENABLE",  						"AUDIO_UR MUX"},
	{"AUDIO_UL OUTPUT",	      					NULL,  							"AUDIO_UL_FIFO SWITCH"},
	{"AUDIO_UR OUTPUT",	   				       	NULL,  							"AUDIO_UR_FIFO SWITCH"},

	/* asp_codec - mic3 */
	{"SIF_EN MIXER",                          	"SIF_ADC3_EN",                  "ADC3 INPUT"},
	{"ADC3 MUX",                           		"ADC3",                         "SIF_EN MIXER"},
	{"ADC_MIC3_FILTER",             			NULL,                  			"ADC3 MUX"},
	{"ADC_MIC3_SRC9_EN PGA",             		NULL,                 			"ADC_MIC3_FILTER"},
	{"MIC3_UP MUX",             				"ADC MIC3_SRC",                 "ADC_MIC3_SRC9_EN PGA"},
	{"MIC3_UP_FIFO SWITCH",	            		"ENABLE",  						"MIC3_UP MUX"},
	{"MIC3_UP OUTPUT",	   				       	NULL,  							"MIC3_UP_FIFO SWITCH"},

	/* asp_codec voice_uplink */
	{"VOICE_UP_SRC7_EN PGA",             		NULL,                  			"ADCL_FILTER"},
	{"VOICE_UP_SRC8_EN PGA",             		NULL,                  			"ADCR_FILTER"},
	{"VOICE_UL MUX",            	  			"ADCL_SRC",  					"VOICE_UP_SRC7_EN PGA"},
	{"VOICE_UR MUX",            	  			"ADCR_SRC",  					"VOICE_UP_SRC8_EN PGA"},
	{"VOICE_UL_FIFO SWITCH",	            	"ENABLE",  						"VOICE_UL MUX"},
	{"VOICE_UR_FIFO SWITCH",	            	"ENABLE",  						"VOICE_UR MUX"},
	{"VOICE_UL OUTPUT",	            			NULL,  							"VOICE_UL_FIFO SWITCH"},
	{"VOICE_UR OUTPUT",	            			NULL,  							"VOICE_UR_FIFO SWITCH"},

	/* ana - main\aux */
	{"MAINMIC INPUT",             				NULL,                  			"MICBIAS1 MIC"},
	{"AUXMIC INPUT",             				NULL,                  			"MICBIAS1 MIC"},
	{"MAIN_HP_MIC MUX",							"MAIN_MIC",						"MAINMIC INPUT"},
	{"ANA_MAIN PGA",             				NULL,                  			"MAIN_HP_MIC MUX"},
	{"ANA_AUX PGA",             				NULL,                  			"AUXMIC INPUT"},
	{"ANA_ADC1 MIXER",							"DACL",							"ANA_MAIN PGA"},
	{"ANA_ADC1 MIXER",							"DACR",							"ANA_MAIN PGA"},
	{"ANA_ADC1 MIXER",							"MAIN MIC",						"ANA_MAIN PGA"},
	{"ANA_ADC1 MIXER",							"AUX MIC",						"ANA_AUX PGA"},
	{"ANA_ADC2 MIXER",							"DACL",							"ANA_MAIN PGA"},
	{"ANA_ADC2 MIXER",							"DACR",							"ANA_MAIN PGA"},
	{"ANA_ADC2 MIXER",             				"MAIN MIC",                  	"ANA_MAIN PGA"},
	{"ANA_ADC2 MIXER",             				"AUX MIC",                  	"ANA_AUX PGA"},
	{"ANA_ADC1 DRV",							NULL,							"ANA_ADC1 MIXER"},
	{"ANA_ADC2 DRV",							NULL,							"ANA_ADC2 MIXER"},
	{"ANA_MUX3 MUX",							"ADC1",							"ANA_ADC1 DRV"},
	{"ANA_MUX4 MUX",							"ADC2",							"ANA_ADC2 DRV"},
	{"ANA_MUX3 MUX",							"ADC1",							"ADC_LOOP MUX"},
	{"ANA_MUX4 MUX",							"ADC2",							"ADC_LOOP MUX"},
	{"ANA_SIF_EN MIXER",						"ANA_MUX3_EN",					"ANA_MUX3 MUX"},
	{"ANA_SIF_EN MIXER",						"ANA_MUX4_EN",					"ANA_MUX4 MUX"},
	{"ANA_SIF OUTPUT",             				NULL,                  			"ANA_SIF_EN MIXER"},

	/* ana - mic3 */
	{"MIC3 INPUT",             					NULL,                  			"MICBIAS2 MIC"},
	{"ANA_MIC3 PGA",             				NULL,                  			"MIC3 INPUT"},
	{"ANA_ADC3 MIXER",             				"MIC3",                  		"ANA_MIC3 PGA"},
	{"ANA_ADC3 DRV",							NULL,							"ANA_ADC3 MIXER"},
	{"ANA_MUX5 MUX",             				"ADC3",                  		"ANA_ADC3 DRV"},
	{"ANA_SIF_EN MIXER",             			"ANA_MUX5_EN",                	"ANA_MUX5 MUX"},

	/* ana- headphone */
	{"HPMIC INPUT",             				NULL,                  			"HPMICBIAS MIC"},
	{"MAIN_HP_MIC MUX",             			"HP_MIC",                  		"HPMIC INPUT"},

	/* pll */
	{"DAC_DL MIXER",							NULL,                    		"PLL"},
	{"DAC_DR MIXER",							NULL,                    		"PLL"},

	{"AUDIO_UL_FIFO SWITCH",					NULL,                    		"PLL"},
	{"AUDIO_UR_FIFO SWITCH",					NULL,                    		"PLL"},
	{"MIC3_UP_FIFO SWITCH",						NULL,                    		"PLL"},
	{"ANA_SIF_EN MIXER",						NULL,                    		"SMT_IBIAS SUPPLY"},
	{"ANA_SPK SWITCH",							NULL,                    		"CLASSD SUPPLY"},

	/*I2s1_audio_voice_down_link i2s mode */
	{"I2S1_S1L MUX",							"DACL_MIXER",					"DAC_DL MIXER"},
	{"I2S1_S1R MUX",							"DACR_MIXER",					"DAC_DR MIXER"},

	{"I2S1_I2S_TX SWITCH",						"I2S_SWITCH_EN",				"I2S1_S1L MUX"},
	{"I2S1_I2S_TX SWITCH",						"I2S_SWITCH_EN",				"I2S1_S1R MUX"},
	{"I2S1_CLK SWITCH",							"I2S1_CLK_EN",					"I2S1_I2S_TX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S1_CLK SWITCH"},

	{"DACML_EN MIXER",							"ULTR_D",						"ULTR_DL MUX"},
	{"DACML_EN MIXER",							"VOICE_DL",						"VOICE_DL PGA"},
	{"I2S1_S1L MUX",							"DACML_MIXER2",					"DACML_EN MIXER"},

	{"DACMR_EN MIXER",							"ULTR_D",						"ULTR_DL MUX"},
	{"DACMR_EN MIXER",							"VOICE_DR",						"VOICE_DR PGA"},
	{"I2S1_S1R MUX",							"DACMR_MIXER2",					"DACMR_EN MIXER"},

	/*I2s1_audio_voice_down_link tdm mode */
	{"I2S1_TDM_TX SWITCH",						"TDM_SWITCH_EN",				"I2S1_S1L MUX"},
	{"I2S1_TDM_TX SWITCH",						"TDM_SWITCH_EN",				"I2S1_S1R MUX"},
	{"I2S1_CLK SWITCH",							"I2S1_CLK_EN",					"I2S1_TDM_TX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S1_CLK SWITCH"},

	/*I2S1 i2s smartpa feedback*/
	{"I2S1_I2S_RX SWITCH",						"I2S1_I2S_RX_EN",				"I2S1_2_3 INPUT"},
	{"I2S1_CLK SWITCH",							"I2S1_CLK_EN",					"I2S1_I2S_RX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S1_CLK SWITCH"},

	/*I2S1 tdm smartpa feedback*/
	{"I2S1_TDM_RX SWITCH",						"I2S1_TDM_RX_EN",				"I2S1_2_3 INPUT"},
	{"I2S1_CLK SWITCH",							"I2S1_CLK_EN",					"I2S1_TDM_RX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S1_CLK SWITCH"},

	/*3rdCodec i2s3 downlink*/
	{"THIRD_CODEC_DL PGA",						NULL,							"I2S1_2_3 INPUT"},
	{"I2S3_S1R MUX",							"THIRD_CODEC_DL",				"THIRD_CODEC_DL PGA"},
	{"I2S3_S1R MUX",							"ADC_ULTR",						"ADC_ULTR_FILTER"},
	{"I2S3_TX SWITCH",							"I2S3_TX_EN",					"I2S3_S1R MUX"},
	{"I2S3_CLK SWITCH",							"I2S3_CLK_EN",					"I2S3_TX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S3_CLK SWITCH"},

	/*3rdCodec i2s1 downlink*/
	{"THIRD_CODEC_DL PGA",						NULL,							"I2S1_2_3 INPUT"},
	{"AUDIO_DL PGA",							NULL,							"I2S1_2_3 INPUT"},
	{"AUDIO_DR PGA",							NULL,							"I2S1_2_3 INPUT"},
	{"I2S1_TDM_TX SWITCH",						"TDM_SWITCH_EN",				"AUDIO_DL PGA"},
	{"I2S1_TDM_TX SWITCH",						"TDM_SWITCH_EN",				"AUDIO_DR PGA"},
	{"I2S1_TDM_TX SWITCH",						"TDM_SWITCH_EN",				"THIRD_CODEC_DL PGA"},

	/*I2S2 DOWNLINK*/
	{"S2_DOWNLINK_EN MIXER",					"S2_DACL_MIXER",				"DAC_DL MIXER"},
	{"S2_DOWNLINK_EN MIXER",					"S2_DACR_MIXER",				"DAC_DR MIXER"},
	{"I2S2_TXL MUX",							"S2_MIXER",						"S2_DOWNLINK_EN MIXER"},
	{"I2S2_TXL MUX",							"DACL_MIXER",					"DAC_DL MIXER"},
	{"I2S2_TXR MUX",							"DACR_MIXER",					"DAC_DR MIXER"},
	{"I2S2_TXR MUX",							"S2_TXL",						"I2S2_TXL MUX"},
	{"S2_SRC_I2S2_TX SWITCH",					"I2S2_SWITCH_EN",				"I2S2_TXL MUX"},
	{"S2_SRC_I2S2_TX SWITCH",					"I2S2_SWITCH_EN",				"I2S2_TXR MUX"},
	{"I2S2_CLK SWITCH",							"I2S2_CLK_EN",					"S2_SRC_I2S2_TX SWITCH"},
	{"I2S1_2_3 OUTPUT",							NULL,							"I2S2_CLK SWITCH"},

	/*I2S2 UPLINK*/
	{"I2S2_CLK SWITCH",							"I2S2_CLK_EN",					"I2S1_2_3 INPUT"},
	{"I2S2_RX PGA",								NULL,							"I2S2_CLK SWITCH"},
	{"VOICE_UL MUX",							"S2_L",							"I2S2_RX PGA"},
	{"VOICE_UR MUX",							"S2_R",							"I2S2_RX PGA"},
	{"DAC_DL MUX",								"S2_L",							"I2S2_RX PGA"},
	{"DAC_DR MUX",								"S2_R",							"I2S2_RX PGA"},
	{"DAC_DL MIXER",				  			"IN1_S2_L",  					"I2S2_RX PGA"},
	{"DAC_DR MIXER",				  			"IN1_S2_R",  					"I2S2_RX PGA"},

	{"AUDIO_UP_SRC5_EN PGA",					NULL,							"I2S2_RX PGA"},
	{"AUDIO_UP_SRC6_EN PGA",					NULL,							"I2S2_RX PGA"},
	{"AUDIO_UL MUX",							"S2_L_SRC",						"AUDIO_UP_SRC5_EN PGA"},
	{"AUDIO_UR MUX",							"S2_R_SRC",						"AUDIO_UP_SRC6_EN PGA"},

	{"S2_SRC_RX SWITCH",						"I2S2_SRC_RX_EN",				"I2S2_RX PGA"},
	{"VOICE_UL MUX",							"S2_L",							"S2_SRC_RX SWITCH"},
	{"VOICE_UR MUX",							"S2_R",							"S2_SRC_RX SWITCH"},
	{"DAC_DL MUX",								"S2_L",							"S2_SRC_RX SWITCH"},
	{"DAC_DR MUX",								"S2_R",							"S2_SRC_RX SWITCH"},
	{"DAC_DL MIXER",							"IN1_S2_L",						"S2_SRC_RX SWITCH"},
	{"DAC_DR MIXER",							"IN1_S2_R",						"S2_SRC_RX SWITCH"},

	{"AUDIO_UP_SRC5_EN PGA",					NULL,							"S2_SRC_RX SWITCH"},
	{"AUDIO_UP_SRC6_EN PGA",					NULL,							"S2_SRC_RX SWITCH"},
	{"AUDIO_UL MUX",							"S2_L_SRC",						"AUDIO_UP_SRC5_EN PGA"},
	{"AUDIO_UR MUX",							"S2_R_SRC",						"AUDIO_UP_SRC6_EN PGA"},

	/*I2S3 UPLINK*/
	{"I2S3_RX SWITCH",							"I2S3_RX_EN",					"I2S1_2_3 INPUT"},
	{"I2S3_CLK SWITCH",							"I2S3_CLK_EN",					"I2S3_RX SWITCH"},
	{"ECHO_L MUX",								"S3_L",							"I2S3_CLK SWITCH"},
	{"ECHO_R MUX",								"S3_R",							"I2S3_CLK SWITCH"},

	/*dac adc loop*/
	{"DAC_LOOPBACK MUX",						"MIC3_SDIN",					"ANA_MUX3 MUX"},
	{"DAC_LOOPBACK MUX",						"MIC3_SDIN",					"ANA_MUX4 MUX"},
	{"DAC_LOOPBACK MUX",						"MIC3_SDIN",					"ANA_MUX5 MUX"},

	{"DAC_LOOPBACK MUX",						"DOUT_SDIN",					"ANA_MUX3 MUX"},
	{"DAC_LOOPBACK MUX",						"DOUT_SDIN",					"ANA_MUX4 MUX"},
	{"DAC_LOOPBACK MUX",						"DOUT_SDIN",					"ANA_MUX5 MUX"},

	{"ANA_SIF_EN MIXER",						NULL,							"DAC_LOOPBACK MUX"},
	{"ANA_SIF_EN MIXER",						NULL,							"DAC_LOOPBACK MUX"},

	/*dmic route*/
	{"DMIC1",									NULL,							"DMIC1 INPUT"},
	{"DMIC2",									NULL,							"DMIC2 INPUT"},
	{"ADC1 MUX",								"DMIC1_L",						"DMIC1"},
	{"ADC2 MUX",								"DMIC1_R",						"DMIC1"},
	{"ADC3 MUX",								"DMIC2_L",						"DMIC2"},
	{"ADC_ULTR MUX",							"DMIC1_L",						"DMIC1"},
	{"ADC_ULTR MUX",							"DMIC1_R",						"DMIC1"},
	{"ADC_ULTR MUX",							"DMIC2_L",						"DMIC2"},

	/* asp_codec ultr_uplink */
	{"SIF_EN MIXER",                          	"SIF_ADC3_EN",                  "ADC3 INPUT"},
	{"ADC_ULTR MUX",                           	"ADC3",                     	"SIF_EN MIXER"},
	{"ADC_ULTR MUX",                            "ADC3",                         "DA_AD_LOOP MUX"},
	{"ADC_ULTR_FILTER",             			NULL,                  			"ADC_ULTR MUX"},
	{"ULTR_UP_FIFO SWITCH",             		"ENABLE",                  		"ADC_ULTR_FILTER"},
	{"ULTR_UP OUTPUT",	            			NULL,  							"ULTR_UP_FIFO SWITCH"},

	/* ultr soc downlink */
	{"ULTR_DLINK_FIFO SWITCH",					"ENABLE",						"ULTR_DLINK INPUT"},
	{"ULTR_DL MUX",								"ULTR_D_FIFO",					"ULTR_DLINK_FIFO SWITCH"},
	{"ULTR_DL MUX",								"ADC_ULTR",						"ADC_ULTR MUX"},
	{"ULTR_DLINK PGA",							NULL,							"ULTR_DL MUX"},
	{"ULTR_DLINK_EN MIXER",						"ULTR_D",						"ULTR_DLINK PGA"},
	{"ULTR_DLINK_EN MIXER",						"DACL_SRC_OUT",					"DAC_DL MUX"},
	{"DAC_ULTR MUX", 							"ULTR_MIXER", 					"ULTR_DLINK_EN MIXER"},
	{"DAC_ULTR MUX",							"ULTR_D",						"ULTR_DLINK PGA"},
	{"DACL_ULTR_FILTER", 						NULL, 							"DAC_ULTR MUX"},
	{"SIF_EN MIXER",							"SIF_DACUL_EN",					"DACL_ULTR_FILTER"},
	{"ULTR_DLINK OUTPUT",					  	NULL,							"SIF_EN MIXER"},

	/* ultr ana downlink */
	{"ANA_SIF_EN MIXER",						"ANA_SIF_ULTR_EN",          	"SMT_ANA_DAC_SDATA INPUT"},
	{"ANA_DAC_ULTR SWITCH",						"DAC_ULTR",						"ANA_SIF_EN MIXER"},
	{"ANA_EAR MIXER",							"DAC_ULTR",						"ANA_DAC_ULTR SWITCH"},
	{"ANA_LINEOUT MIXER",						"DAC_ULTR",						"ANA_DAC_ULTR SWITCH"},

	/* dac spa */
	{"DAC_SPA SWITCH",							"DAC_SPA_EN",					"DACL_FILTER"},
	{"SIF_EN MIXER",							"SIF_DAC_SPA_EN",				"DAC_SPA SWITCH"},
	{"DAC_SPA OUTPUT",							NULL,							"SIF_EN MIXER"},
};

void audio_codec_mute_pga(bool mute)
{
	volatile unsigned int val = 0;
	struct hi6555v3_priv *priv = NULL;

	IN_FUNCTION;

	if (!soc_codec) {
		HI6555V3_LOGE("soc_codec is NULL\n");
		return;
	}

	priv = snd_soc_codec_get_drvdata(soc_codec);
	if (!priv) {
		HI6555V3_LOGE("priv is NULL\n");
		return;
	}

	if(CLASSD_SUPPLY_SCHARGER != priv->classd_supply_type) {
		HI6555V3_LOGI("classd supply type is %d, need not mute/umute\n", priv->classd_supply_type);
		return;
	}

	if (mute) {
		/*mute audio_dlr_mixer*/
		val = hi6555v3_reg_read(soc_codec, DACL_MIXER4_CTRL0_REG);
		if (INVALID_REG_VALUE != val) {
			priv->mute_dacl_reg_val = val;
			hi6555v3_reg_update(DACL_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, MUTE_DAC_VALUE);
		} else {
			HI6555V3_LOGE("read dacl reg fail\n");
		}

		val = hi6555v3_reg_read(soc_codec, DACR_MIXER4_CTRL0_REG);
		if (INVALID_REG_VALUE != val) {
			priv->mute_dacr_reg_val = val;
			hi6555v3_reg_update(DACR_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, MUTE_DAC_VALUE);
		} else {
			HI6555V3_LOGE("read dacl reg fail\n");
		}

		HI6555V3_LOGI("mute audio_dlr_mixer for flash driver\n");
	} else {
		/*unmute audio_dlr_mixer*/
		if (INVALID_REG_VALUE != priv->mute_dacl_reg_val) {
			hi6555v3_reg_update(DACL_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, priv->mute_dacl_reg_val);
			priv->mute_dacl_reg_val = INVALID_REG_VALUE;
		}

		if (INVALID_REG_VALUE != priv->mute_dacr_reg_val) {
			hi6555v3_reg_update(DACR_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, priv->mute_dacr_reg_val);
			priv->mute_dacr_reg_val = INVALID_REG_VALUE;
		}

		HI6555V3_LOGI("unmute audio_dlr_mixer for flash driver\n");
	}

	return;
}
EXPORT_SYMBOL_GPL(audio_codec_mute_pga);

static void _hi6555v3_reset_pmu_codec(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	/* smartstar codec reset */
	hi6555v3_reg_write(codec, CTRL_REG_PMU_SOFT_RST_REG, 0x22);
	msleep(10);
	hi6555v3_reg_write(codec, CTRL_REG_PMU_SOFT_RST_REG, 0x2F);

	OUT_FUNCTION;
}

static void _hi6555v3_reset_asp_codec(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	hi6555v3_reg_write(codec, R_RST_CTRLEN, BIT(RST_EN_CODEC_N));
	hi6555v3_reg_write(codec, R_RST_CTRLDIS, BIT(RST_DISEN_CODEC_N));
	hi6555v3_reg_write(codec, R_GATE_EN, BIT(GT_CODEC_CLK));
	hi6555v3_set_reg_bits(R_CODEC_DMA_SEL, BIT(CODEC_DMA_SEL));

	OUT_FUNCTION;
}

static void _hi6555v3_asp_pmu_reg_init(void)
{
	IN_FUNCTION;

	/* disable mixer */
	hi6555v3_set_reg_bits(DACL_MIXER4_CTRL0_REG, BIT(DACL_MIXER4_IN1_MUTE_OFFSET) | BIT(DACL_MIXER4_IN2_MUTE_OFFSET)
					   |BIT(DACL_MIXER4_IN3_MUTE_OFFSET) | BIT(DACL_MIXER4_IN4_MUTE_OFFSET));

	hi6555v3_set_reg_bits(DACR_MIXER4_CTRL0_REG, BIT(DACR_MIXER4_IN1_MUTE_OFFSET) | BIT(DACR_MIXER4_IN2_MUTE_OFFSET)
					   |BIT(DACR_MIXER4_IN3_MUTE_OFFSET) | BIT(DACR_MIXER4_IN4_MUTE_OFFSET));

	hi6555v3_clr_reg_bits(CODEC_ANA_RW12_REG, BIT(ADCL_MIXIN_DACL_OFFSET) | BIT(ADCL_MIXIN_DACR_OFFSET)
					   |BIT(ADCL_MIXIN_MICPGA1_OFFSET) | BIT(ADCL_MIXIN_MICPGA2_OFFSET));

	hi6555v3_clr_reg_bits(CODEC_ANA_RW14_REG, BIT(ADCR_MIXIN_DACL_OFFSET) | BIT(ADCR_MIXIN_DACR_OFFSET)
					   |BIT(ADCR_MIXIN_MICPGA1_OFFSET) | BIT(ADCR_MIXIN_MICPGA2_OFFSET));

	hi6555v3_clr_reg_bits(CODEC_ANA_RW16_REG, BIT(ADC3_MIXIN_DACL_OFFSET) | BIT(ADC3_MIXIN_ULTRA_OFFSET)
					   |BIT(ADC3_MIXIN_MICPGA1_OFFSET) | BIT(ADC3_MIXIN_MICPGA3_OFFSET));

	hi6555v3_clr_reg_bits(CODEC_ANA_RW20_REG, BIT(MIXOUT_HSL_DACL_OFFSET) | BIT(MIXOUT_HSL_DACR_OFFSET)
					   |BIT(MIXOUT_HSR_DACL_OFFSET) | BIT(MIXOUT_HSR_DACR_OFFSET));

	hi6555v3_clr_reg_bits(CODEC_ANA_RW25_REG, BIT(MIXOUT_EAR_DACL_OFFSET) | BIT(MIXOUT_EAR_DACR_OFFSET)
					   |BIT(MIXOUT_EAR_DAC_ULTRA_OFFSET) | BIT(CR_MIXER_EP_DACL_OFFSET)
					   |BIT(CR_MIXER_EP_DACR_OFFSET) | BIT(CR_MIXER_EP_DAC_ULTRA_OFFSET));

	hi6555v3_set_reg_bits(CTRL_REG_CLASSD_CTRL0_REG, BIT(CTRL_REG_CLASSD_MUTE_SEL_OFFSET));

	OUT_FUNCTION;
}

static void _hi6555v3_asp_soc_reg_init(void)
{
	IN_FUNCTION;

	/* class h bypass */
	hi6555v3_set_reg_bits(CLASSH_CTRL1_REG, BIT(CLASSH_BYPASS_OFFSET));

	/* set i2s1 tdm mode fs */
	hi6555v3_reg_update(FS_CTRL3_REG, I2S1_TDM_FS_MASK, I2S1_TDM_FS_MASK);

	/* set voice dl fs */
	hi6555v3_reg_update(FS_CTRL0_REG, FS_VOICE_L_DLINK_MASK, FS_VOICE_L_DLINK_VALUE);

	/* set voice dr fs */
	hi6555v3_reg_update(FS_CTRL2_REG, FS_VOICE_R_DLINK_MASK, FS_VOICE_R_DLINK_VALUE);

	/* set bt l rx pga gain */
	hi6555v3_reg_update(BT_L_RX_PGA_CTRL_REG, BT_L_RX_PGA_GAIN_MASK, BT_L_RX_PGA_GAIN_VALUE);

	/* set bt r rx pga gain */
	hi6555v3_reg_update(BT_R_RX_PGA_CTRL_REG, BT_R_RX_PGA_GAIN_MASK, BT_R_RX_PGA_GAIN_VALUE);

	/* set i2s1 tdm tx sel */
	hi6555v3_clr_reg_bits(I2S1_TDM_CTRL0_REG, BIT(S1_TDM_TX_CLK_SEL_OFFSET));

	/* set voice r dn src in fs */
	hi6555v3_reg_update(FS_CTRL2_REG, FS_VOICE_R_DN_SRCUP_IN_MASK, FS_VOICE_R_DN_SRCUP_IN_VALUE);

	/* set i2s2 txl dout fs */
	hi6555v3_reg_update(FS_CTRL3_REG, FS_I2S2_TX_L_SRCDN_DOUT_MASK, FS_I2S2_TX_L_SRCDN_DOUT_VALUE);

	/* set i2s2 txr dout fs */
	hi6555v3_reg_update(FS_CTRL3_REG, I2S2_TX_R_SRCDN_DOUT_FS_MASK, I2S2_TX_R_SRCDN_DOUT_FS_VALUE);

	OUT_FUNCTION;
}

static void _hi6555v3_asp_reg_init(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	_hi6555v3_reset_asp_codec(codec);

	_hi6555v3_asp_pmu_reg_init();

	_hi6555v3_asp_soc_reg_init();

	OUT_FUNCTION;
}

static void _hi6555v3_hsmicbias_force_close(struct snd_soc_codec *codec)
{
	unsigned int hs_irq_mask = 0;

	/* to avoid irq while MBHD_COMP power down, mask all COMP irq */
	hs_irq_mask = snd_soc_read(codec, ANA_IRQM_REG0_REG);
	hi6555v3_set_reg_bits(ANA_IRQM_REG0_REG, hs_irq_mask | IRQ_MSK_COMP);

	/* disable NORMAL key detect and identify */
	hi6555v3_set_reg_bits(CODEC_ANA_RW8_REG, BIT(MBHD_BUFF_PD_OFFSET));
	hi6555v3_set_reg_bits(CODEC_ANA_RW8_REG, BIT(MBHD_COMP_PD_OFFSET));

	/* hs micbias pd */
	hi6555v3_set_reg_bits(CODEC_ANA_RW7_REG, BIT(HSMICB_PD_OFFSET));
	hi6555v3_set_reg_bits(CODEC_ANA_RW7_REG, BIT(HSMICB_DSCHG_OFFSET));
	msleep(5);
	hi6555v3_clr_reg_bits(CODEC_ANA_RW7_REG, BIT(HSMICB_DSCHG_OFFSET));
}

static int hi6555v3_startup(struct snd_pcm_substream *substream,
		struct snd_soc_dai *dai)
{
	/* Inherit the previous version policy, not implemented */
	return 0;
}

static int hi6555v3_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params, struct snd_soc_dai *dai)
{
	/* Inherit the previous version policy, not implemented */
	return 0;
}

static void hi6555v3_shutdown(struct platform_device *dev)
{
	struct hi6555v3_priv *priv = NULL;

	HI6555V3_LOGI("Begin\n");

	if (!soc_codec) {
		HI6555V3_LOGE("soc_codec is NULL\n");
		return;
	}

	priv = snd_soc_codec_get_drvdata(soc_codec);
	if (!priv) {
		HI6555V3_LOGE("priv is NULL\n");
		return;
	}

	/* headset pop process */
	hi6555v3_headphone_pop_off(soc_codec);

	/* classd pop process */
	hi6555v3_classd_pop_off(soc_codec);

	/* close hsmicbias */
	_hi6555v3_hsmicbias_force_close(soc_codec);

	/* close ibias and Avref */
	hi6555v3_power_codec(soc_codec, false);

	/* reset codec */
	hi6555v3_reg_write(soc_codec, CTRL_REG_PMU_SOFT_RST_REG, 0x22);

	HI6555V3_LOGI("End\n");
}

struct snd_soc_dai_ops hi6555v3_dai_ops = {
	.startup    = hi6555v3_startup,
	.hw_params  = hi6555v3_hw_params,
};

struct snd_soc_dai_driver hi6555v3_dai[] = {
	{
		.name = "hi6555c-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = HI6555V3_PB_MIN_CHANNELS,
			.channels_max = HI6555V3_PB_MAX_CHANNELS,
			.rates = HI6555V3_RATES,
			.formats = HI6555V3_FORMATS
		},
		.capture = {
			.stream_name = "Capture",
			.channels_min = HI6555V3_CP_MIN_CHANNELS,
			.channels_max = HI6555V3_CP_MAX_CHANNELS,
			.rates = HI6555V3_RATES,
			.formats = HI6555V3_FORMATS
		},
		.ops = &hi6555v3_dai_ops,
	},
};

static void _set_platform_type(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = NULL;
	struct device_node *np = codec->dev->of_node;
	unsigned int val = 0;

	priv = snd_soc_codec_get_drvdata(codec);
	if (!priv) {
		HI6555V3_LOGE("priv is null\n");
		return;
	}

	if (!of_property_read_u32(np, "udp_io_config", &val)) {
		HI6555V3_LOGI("udp_io_config is %d\n", val);
		if (val == 1) {
			priv->platform_type = UDP_PLATFORM;
		} else {
			priv->platform_type = FPGA_PLATFORM;
		}
	} else {
		HI6555V3_LOGI("udp_io_config is not configed, set fpga default\n");
		priv->platform_type = FPGA_PLATFORM;
	}

	return;
}

static int _asp_resource_init(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	priv->asp_regulator = devm_regulator_get(codec->dev, "asp");
	if (IS_ERR(priv->asp_regulator)) {
		HI6555V3_LOGE("get asp regulators err:%pK\n", priv->asp_regulator);
		ret = PTR_ERR(priv->asp_regulator);
		goto get_asp_regulator_err;
	}

	priv->asp_subsys_clk = devm_clk_get(codec->dev, "clk_asp_subsys");
	if (IS_ERR_OR_NULL(priv->asp_subsys_clk)) {
		HI6555V3_LOGE("get clk_asp_subsys err:%pK\n", priv->asp_subsys_clk);
		ret = PTR_ERR(priv->asp_subsys_clk);
		goto get_asp_subsys_clk_err;
	}

	priv->asp_49m_clk = devm_clk_get(codec->dev, "clk_asp_codec");
	if (IS_ERR_OR_NULL(priv->asp_49m_clk)) {
		HI6555V3_LOGE("get clk_asp_codec err:%pK\n", priv->asp_49m_clk);
		ret = PTR_ERR(priv->asp_49m_clk);
		goto get_asp_49m_clk_err;
	}

	HI6555V3_LOGI("asp resource init ok\n");
	goto end;

get_asp_49m_clk_err:
	priv->asp_49m_clk = NULL;
get_asp_subsys_clk_err:
	priv->asp_subsys_clk = NULL;
get_asp_regulator_err:
	priv->asp_regulator = NULL;
end:
	OUT_FUNCTION;
	return ret;
}

static void _asp_resource_deinit(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);
	priv->asp_regulator = NULL;
	priv->asp_subsys_clk = NULL;
	priv->asp_49m_clk = NULL;

	HI6555V3_LOGI("asp resource deinit ok\n");
}

static int _asp_regulator_enable(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);
	WARN_ON(!priv->asp_regulator);

	if (priv->regulator_pu_count > 0) {
		priv->regulator_pu_count++;
		HI6555V3_LOGI("asp regulator is already enable, regulator_pu_count %d\n", priv->regulator_pu_count);
		return 0;
	}

	ret = regulator_enable(priv->asp_regulator);
	if (ret) {
		HI6555V3_LOGE("couldn't enable asp_cfg_regu regulator, ret = %d\n", ret);
		return ret;/*lint !e527*/
	}

	priv->regulator_pu_count++;

	HI6555V3_LOGI("asp regulator enable done, regulator_pu_count %d\n", priv->regulator_pu_count);

	OUT_FUNCTION;

	return ret;
}

static void _asp_regulator_disable(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);

	if (priv->regulator_pu_count > 1) {
		priv->regulator_pu_count--;
		HI6555V3_LOGI("asp regulator others in use, regulator_pu_count %d\n", priv->regulator_pu_count);
		return;
	}

	if (!IS_ERR(priv->asp_regulator)) {
		if (regulator_disable(priv->asp_regulator)) {
			HI6555V3_LOGE("couldn't disable asp_regulator\n");
		}
		HI6555V3_LOGI("asp_regulator disable ok\n");
	}

	priv->regulator_pu_count--;

	HI6555V3_LOGI("asp regulator disable done, regulator_pu_count %d\n", priv->regulator_pu_count);

	OUT_FUNCTION;
}

static int _asp_clk_enable(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);
	WARN_ON(!priv->asp_subsys_clk);
	WARN_ON(!priv->asp_49m_clk);

	if (priv->clk_pu_count > 0) {
		priv->clk_pu_count++;
		HI6555V3_LOGI("asp_subsys_clk is already enable, clk_pu_count %d\n", priv->clk_pu_count);
		return 0;
	}

	ret = clk_prepare_enable(priv->asp_subsys_clk);
	if (ret) {
		HI6555V3_LOGE("asp_subsys_clk :clk prepare enable failed, ret = %d\n", ret);
		return ret;/*lint !e527*/
	}

	ret = clk_prepare_enable(priv->asp_49m_clk);
	if (ret) {
		HI6555V3_LOGE("asp_49m_clk clk en fail, ret = %d\n", ret);
		clk_disable_unprepare(priv->asp_subsys_clk);
		return ret;/*lint !e527*/
	}

	priv->clk_pu_count++;
	HI6555V3_LOGI("asp clk enable ok, clk_pu_count: %d\n", priv->clk_pu_count);

	OUT_FUNCTION;

	return ret;
}

static void _asp_clk_disable(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->clk_pu_count > 1) {
		priv->clk_pu_count--;
		HI6555V3_LOGI("asp_subsys_clk others in use, clk_pu_count %d\n", priv->clk_pu_count);
		return;
	}

	if (!IS_ERR_OR_NULL(priv->asp_49m_clk)) {
		clk_disable_unprepare(priv->asp_49m_clk);
		HI6555V3_LOGI("asp_49m_clk disable ok\n");
	}

	if (!IS_ERR_OR_NULL(priv->asp_subsys_clk)) {
		clk_disable_unprepare(priv->asp_subsys_clk);
		HI6555V3_LOGI("asp_subsys_clk disable ok\n");
	}

	priv->clk_pu_count--;
	HI6555V3_LOGI("asp clk disable ok, clk_pu_count: %d\n", priv->clk_pu_count);

	OUT_FUNCTION;
}

static int _asp_resource_enable(struct snd_soc_codec *codec)
{
	int ret;

	IN_FUNCTION;

	ret = _asp_regulator_enable(codec);
	if (ret) {
		HI6555V3_LOGE("couldn't enable asp regulator, ret = %d\n", ret);
		goto end;
	}

	ret = _asp_clk_enable(codec);
	if (ret) {
		HI6555V3_LOGE("asp clk enable failed, ret = %d\n", ret);
		goto clk_enable_err;
	}

	HI6555V3_LOGI("asp resource enable ok\n");

	goto end;

clk_enable_err:
	_asp_regulator_disable(codec);

end:
	OUT_FUNCTION;

	return ret;
}

static void _asp_resource_disable(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	_asp_regulator_disable(codec);
	_asp_clk_disable(codec);

	OUT_FUNCTION;
}

static int _pmu_resource_init(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	priv->codec_ldo8 = devm_regulator_get(codec->dev, "codec_ldo8");
	if (IS_ERR(priv->codec_ldo8)) {
		HI6555V3_LOGE("couldn't get ldo8 regulator ret:%ld\n", PTR_ERR(priv->codec_ldo8));
		ret = PTR_ERR(priv->codec_ldo8);
		priv->codec_ldo8 = NULL;
		return ret;
	}

	ret = regulator_set_voltage(priv->codec_ldo8, LDO8_VOLTAGE, LDO8_VOLTAGE);
	if (ret) {
		HI6555V3_LOGE("regulator set codec_ldo8 voltage failed\n");
		return ret;
	}

	ret = regulator_enable(priv->codec_ldo8);
	if (ret) {
		HI6555V3_LOGE("couldn't enable codec_ldo8 regulators %d\n", ret);
		return ret;/*lint !e527*/
	}

	HI6555V3_LOGI("pmu resource init ok\n");

	OUT_FUNCTION;

	return 0;
}

static void _pmu_resource_deinit(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	if (!IS_ERR(priv->codec_ldo8)) {
		ret = regulator_disable(priv->codec_ldo8);
		if (ret) {
			HI6555V3_LOGE("couldn't disable codec_ldo8 %d\n", ret);
		}
	}

	HI6555V3_LOGI("pmu resource deinit ok\n");
}

static int _pinctrl_init(struct snd_soc_codec *codec)
{
	int ret;
	struct device *dev = codec->dev;
	struct hi6555v3_priv *priv;
	priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (FPGA_PLATFORM == priv->platform_type) {
		hi6555v3_reg_write(codec, 0xFFF11714, 0x2); /* sif_do0 GPIO_194 */
		hi6555v3_reg_write(codec, 0xFFF11718, 0x2); /* sif_di0 GPIO_193 */
		hi6555v3_reg_write(codec, 0xFFF1171c, 0x2); /* sif_sync GPIO_204 */
		hi6555v3_reg_write(codec, 0xFFF1172c, 0x2); /* sif_do1 GPIO_210 */
		hi6555v3_reg_write(codec, 0xFFF11758, 0x3); /* dsd_clk GPIO_221, used as sif_clk */
		hi6555v3_reg_write(codec, 0xFFF11760, 0x2); /* sif_di1 GPIO_223 */

		return 0;
	}

	/* I2S1 & I2S2 & SIF & claasd bst_5v_en iomux config */
	priv->pctrl = pinctrl_get(dev);
	if (IS_ERR(priv->pctrl)) {
		priv->pctrl = NULL;
		HI6555V3_LOGE("could not get pinctrl\n");
		return -EFAULT;
	}

	priv->pin_default = pinctrl_lookup_state(priv->pctrl, PINCTRL_STATE_DEFAULT);
	if (IS_ERR(priv->pin_default)) {
		HI6555V3_LOGE("could not get default state (%li)\n", PTR_ERR(priv->pin_default));
		priv->pin_default = NULL;
		goto pinctrl_operation_err;
	}

	priv->pin_idle = pinctrl_lookup_state(priv->pctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(priv->pin_idle)) {
		HI6555V3_LOGE("could not get idle state (%li)\n", PTR_ERR(priv->pin_idle));
		priv->pin_idle = NULL;
		goto pinctrl_operation_err;
	}

	ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
	if (ret) {
		HI6555V3_LOGE("could not set pins to default state, ret=%d\n", ret);
		goto pinctrl_operation_err;
	}

	HI6555V3_LOGI("pinctrl init ok!\n");

	OUT_FUNCTION;
	return 0;

pinctrl_operation_err:
	pinctrl_put(priv->pctrl);
	priv->pctrl = NULL;
	return -EFAULT;
}

static void _pinctrl_deinit(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (FPGA_PLATFORM == priv->platform_type) {
		return;
	}

	pinctrl_put(priv->pctrl);
	priv->pctrl = NULL;
	HI6555V3_LOGI("pinctrl deinit ok\n");

	OUT_FUNCTION;
}

static void _classd_schg_init(struct hi6555v3_priv *priv)
{
	struct snd_soc_codec *codec = priv->codec;
	struct device *dev  = NULL;

	IN_FUNCTION;
	WARN_ON(!codec);
	dev = codec->dev;

	priv->regulator_schg_boost3 = devm_regulator_get(dev, "schg_boost3");
	if (IS_ERR(priv->regulator_schg_boost3)) {
		HI6555V3_LOGE("get regulator_schg_boost3 failed. error:%ld\n", PTR_ERR(priv->regulator_schg_boost3));
		return;
	}

	HI6555V3_LOGI("get regulator_schg_boost3 ok\n");
	OUT_FUNCTION;
}

static void _classd_schg_deinit(struct hi6555v3_priv *priv)
{
	IN_FUNCTION;

	if (IS_ERR(priv->regulator_schg_boost3)) {
		HI6555V3_LOGE("regulator_schg_boost3 is error:%ld\n", PTR_ERR(priv->regulator_schg_boost3));
		return;
	}

	devm_regulator_put(priv->regulator_schg_boost3);
	priv->regulator_schg_boost3 = NULL;
	HI6555V3_LOGI("put regulator_schg_boost3 ok\n");
	OUT_FUNCTION;
}

static void _classd_gpio_init(struct hi6555v3_priv *priv)
{
	int ret;
	unsigned int val = ARCH_NR_GPIOS;
	struct snd_soc_codec *codec = priv->codec;
	struct device *dev  = NULL;
	struct device_node *np  = NULL;

	IN_FUNCTION;
	WARN_ON(!codec);
	WARN_ON(!codec->dev);
	WARN_ON(!codec->dev->of_node);
	dev = codec->dev;
	np = dev->of_node;

	priv->gpio_classd = ARCH_NR_GPIOS;
	ret = of_property_read_u32(np, "gpio_classd", &val);
	if (ret) {
		HI6555V3_LOGE("get gpio_classd failed. error:%d\n", ret);
		return;
	}

	if (!gpio_is_valid(val)) {
		HI6555V3_LOGE("classd gpio:%d is invalied\n", val);
		return;
	}

	if (gpio_request(val, "classd_gpio")) {
		HI6555V3_LOGE("request classd_gpio:%d failed\n", val);
		return;
	}

	priv->gpio_classd = val;
	if (gpio_direction_output(priv->gpio_classd, GPIO_LEVEL_LOW)) {
		gpio_free(priv->gpio_classd);
		priv->gpio_classd = ARCH_NR_GPIOS;
		HI6555V3_LOGE("classd_gpio:%d pull down failed\n", val);
		return;
	}

	HI6555V3_LOGI("classd gpio init ok\n");
	OUT_FUNCTION;
}

static void _classd_gpio_deinit(struct hi6555v3_priv *priv)
{
	IN_FUNCTION;

	if (!gpio_is_valid(priv->gpio_classd)) {
		HI6555V3_LOGW("classd gpio is invalied\n");
		return;
	}

	gpio_free(priv->gpio_classd);
	priv->gpio_classd = ARCH_NR_GPIOS;

	HI6555V3_LOGI("classd gpio deinit ok\n");
	OUT_FUNCTION;
	return;
}

static void _classd_supply_init(struct snd_soc_codec *codec)
{
	enum hi6555v3_classd_supply_type supply_type;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;
	WARN_ON(!priv);

	supply_type = _get_classd_supply_type(codec);
	switch (supply_type) {
	case CLASSD_SUPPLY_SCHARGER:
		_classd_schg_init(priv);
		break;
	case CLASSD_SUPPLY_GPIO:
		_classd_gpio_init(priv);
		break;
	default:
		HI6555V3_LOGW("classd supply init fail. error type is %d!", supply_type);
		break;
	}

	priv->classd_supply_type = supply_type;
	OUT_FUNCTION;
}

static void _classd_supply_deinit(struct snd_soc_codec *codec)
{
	int supply_type;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;
	WARN_ON(!priv);

	supply_type = priv->classd_supply_type;
	switch (supply_type) {
	case CLASSD_SUPPLY_SCHARGER:
		_classd_schg_deinit(priv);
		break;
	case CLASSD_SUPPLY_GPIO:
		_classd_gpio_deinit(priv);
		break;
	default:
		HI6555V3_LOGW("classd supply deinit fail. error type is %d!", supply_type);
		break;
	}

	OUT_FUNCTION;
}

static void _pm_runtime_init(struct snd_soc_codec *codec)
{
	struct device_node *np = codec->dev->of_node;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);
	if (of_property_read_bool(np, "pm_runtime_support")) {
		priv->pm_runtime_support = true;
	} else {
		priv->pm_runtime_support = false;
	}

	HI6555V3_LOGI("pm_runtime_support:%d\n", priv->pm_runtime_support);

	if (priv->pm_runtime_support) {
		pm_runtime_use_autosuspend(codec->dev);
		pm_runtime_set_autosuspend_delay(codec->dev, 200); /* 200ms */
		pm_runtime_set_active(codec->dev);
		pm_runtime_enable(codec->dev);
	}

	OUT_FUNCTION;
}

static void _pm_runtime_deinit(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	WARN_ON(!priv);

	if (priv->pm_runtime_support) {
		pm_runtime_resume(codec->dev);
		pm_runtime_disable(codec->dev);
		pm_runtime_set_suspended(codec->dev);
	}

	OUT_FUNCTION;
}

static void _cfg_sif_driving_force(struct snd_soc_codec *codec)
{
	unsigned int value = DEFAULT_SIF_DRIVING_FORCE;
	unsigned int dts_cfg = 0;
	struct device_node *np;
	WARN_ON(!codec->dev);
	WARN_ON(!codec->dev->of_node);
	np = codec->dev->of_node;

	IN_FUNCTION;

	if (!of_property_read_u32(np, "sif_driving_force", &dts_cfg)) {
		if (dts_cfg <= MAX_SIF_DRIVING_FORCE) {
			value = dts_cfg;
		}
	}

	hi6555v3_reg_update(CTRL_REG_DIG_IO_DS_CODEC_CFG_REG, DIG_IO_DS_CODEC_MASK, value);

	HI6555V3_LOGI("sif_driving_force is %u\n", value);

	OUT_FUNCTION;
}

static int hi6555v3_resource_init(struct snd_soc_codec *codec)
{
	int ret;
	struct hi6555v3_priv *priv = NULL;

	IN_FUNCTION;

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	_set_platform_type(codec);

	ret = _pmu_resource_init(codec);
	if (ret) {
		HI6555V3_LOGE("pum codec resource init error, ret = %d\n", ret);
		goto end;
	}

	ret = _asp_resource_init(codec);
	if (ret) {
		HI6555V3_LOGE("asp resource init error, ret = %d\n", ret);
		goto asp_resource_init_err;
	}

	ret = _asp_resource_enable(codec);
	if (ret) {
		HI6555V3_LOGE("asp resource enable error, ret = %d\n", ret);
		goto asp_resource_enable_err;
	}

	ret = _pinctrl_init(codec);
	if (ret) {
		HI6555V3_LOGE("pinctrl init error, ret=%d\n", ret);
		goto pinctrl_init_err;
	}

	_classd_supply_init(codec);
	_pm_runtime_init(codec);
	_cfg_sif_driving_force(codec);
	_hi6555v3_reset_pmu_codec(codec);
	_hi6555v3_asp_reg_init(codec);
	HI6555V3_LOGI("resource init ok\n");

	goto end;

pinctrl_init_err:
	_asp_resource_disable(codec);

asp_resource_enable_err:
	_asp_resource_deinit(codec);

asp_resource_init_err:
	_pmu_resource_deinit(codec);

end:
	OUT_FUNCTION;

	return ret;
}

static void hi6555v3_resource_deinit(struct snd_soc_codec *codec)
{
	_pm_runtime_deinit(codec);
	_classd_supply_deinit(codec);
	_pinctrl_deinit(codec);
	_asp_resource_disable(codec);
	_asp_resource_deinit(codec);
	_pmu_resource_deinit(codec);

	HI6555V3_LOGI("resource deinit ok\n");

	return;
}

/*lint -e429*/
static int hi6555v3_priv_init(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = NULL;
	struct device *dev = codec->dev;
	unsigned int i;

	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (!priv) {
		HI6555V3_LOGE("priv devm_kzalloc failed\n");
		return -ENOMEM;
	}

	snd_soc_codec_set_drvdata(codec, priv);

	soc_codec = codec;
	priv->codec = codec;

	/* virtual codec register init */
	for (i = HI6555V3_VIR0; i < HI6555V3_VIR_CNT; i++) {
		priv->v_codec_reg[i] = 0;
	}

	priv->regulator_pu_count = 0;

	priv->mute_dacl_reg_val = INVALID_REG_VALUE;
	priv->mute_dacr_reg_val = INVALID_REG_VALUE;

	priv->classd_scharger_voltage = CLASSD_DEFAULT_VOLTAGE;

	spin_lock_init(&priv->lock);
	mutex_init(&priv->ibias_mutex);

	HI6555V3_LOGI("priv init ok\n");

	return 0;
}
/*lint +e429*/

static void hi6555v3_priv_deinit(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv) {
		mutex_destroy(&priv->ibias_mutex);
		snd_soc_codec_set_drvdata(codec, NULL);
	}

	soc_codec = NULL;

	HI6555V3_LOGI("priv deinit ok\n");

	OUT_FUNCTION;
}

static int hi6555v3_hac_init(struct snd_soc_codec *codec)
{
	unsigned int val = 0;
	struct device_node *np = codec->dev->of_node;

	if (!of_property_read_u32(np, "hisilicon,hac_gpio", &val)) {
		hac_en_gpio = val;
		HI6555V3_LOGI("hac gpio num is %d.\n",hac_en_gpio);
	} else {
		hac_en_gpio = ARCH_NR_GPIOS;
		HI6555V3_LOGI("hac is not support.\n");
		return 0;
	}

	if (gpio_request(hac_en_gpio, "hac_en_gpio")) {
		hac_en_gpio = ARCH_NR_GPIOS;
		HI6555V3_LOGE("hac_en_gpio request failed\n");
		return -EFAULT;
	}

	if (gpio_direction_output(hac_en_gpio, 0)) {
		gpio_free(hac_en_gpio);
		hac_en_gpio = ARCH_NR_GPIOS;
		HI6555V3_LOGE("hac_en_gpio set output failed\n");
		return -EFAULT;
	}

	return 0;
}

static int hi6555v3_codec_probe(struct snd_soc_codec *codec)
{
	struct snd_soc_dapm_context *dapm;
	struct hi6555v3_priv *priv;
	int ret;

	HI6555V3_LOGI("Begin\n");

	if (!codec) {
		HI6555V3_LOGE("codec is null");
		return -EINVAL;
	}

	dapm = snd_soc_codec_get_dapm(codec);
	if (!dapm) {
		HI6555V3_LOGE("dapm is null");
		return -EINVAL;
	}

	ret = hi6555v3_priv_init(codec);
	if (ret) {
		HI6555V3_LOGE("hi6555v3_priv_init err, ret=%d\n", ret);
		goto end;
	}

	ret = hi6555v3_resource_init(codec);
	if (ret) {
		HI6555V3_LOGE("hi6555v3_resource_init err, ret=%d\n", ret);
		goto resource_init_err;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);
	ret = hi6xxx_mbhc_init(codec, &priv->mbhc);
	if (ret) {
		HI6555V3_LOGE("hi6xxx_mbhc_init err, ret=%d\n", ret);
		goto mbhc_init_err;
	}

	ret = snd_soc_dapm_sync(dapm);
	if (ret) {
		HI6555V3_LOGE("dapm sync error, errornum = %d\n", ret);
		goto dapm_sync_err;
	}
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_init(codec, &hi6555v3_dump_info);
#endif

	ret = hi6555v3_hac_init(codec);
	if (ret) {
		HI6555V3_LOGE("hi6555v3_hac_init error, errornum = %d\n", ret);
		goto hac_init_err;
	}

	goto end;

hac_init_err:
#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

dapm_sync_err:
	hi6xxx_mbhc_deinit(priv->mbhc);

mbhc_init_err:
	hi6555v3_resource_deinit(codec);

resource_init_err:
	hi6555v3_priv_deinit(codec);

end:
	HI6555V3_LOGI("End\n");

	return ret;
}

static int hi6555v3_codec_remove(struct snd_soc_codec *codec)
{
	struct hi6555v3_priv *priv;

	IN_FUNCTION;

	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	hi6xxx_mbhc_deinit(priv->mbhc);
	hi6555v3_resource_deinit(codec);
	hi6555v3_priv_deinit(codec);

	if (gpio_is_valid(hac_en_gpio)) {
		gpio_free(hac_en_gpio);
		hac_en_gpio = ARCH_NR_GPIOS;
	}

	OUT_FUNCTION;

	return 0;
}

static struct snd_soc_codec_driver soc_codec_dev_hi6555v3 = {
	.probe    = hi6555v3_codec_probe,
	.remove  = hi6555v3_codec_remove,
	.read      = hi6555v3_reg_read,
	.write     = hi6555v3_reg_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	.component_driver = {
		.controls = hi6555v3_snd_controls,
		.num_controls = ARRAY_SIZE(hi6555v3_snd_controls),
		.dapm_widgets = hi6555v3_dapm_widgets,
		.num_dapm_widgets = ARRAY_SIZE(hi6555v3_dapm_widgets),
		.dapm_routes = hi6555v3_route_map,
		.num_dapm_routes = ARRAY_SIZE(hi6555v3_route_map),
	},
#else
	.controls = hi6555v3_snd_controls,
	.num_controls = ARRAY_SIZE(hi6555v3_snd_controls),
	.dapm_widgets = hi6555v3_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(hi6555v3_dapm_widgets),
	.dapm_routes = hi6555v3_route_map,
	.num_dapm_routes = ARRAY_SIZE(hi6555v3_route_map),
#endif
	.idle_bias_off = true
};

static int hi6555v3_probe(struct platform_device *pdev)
{
	int ret;
	HI6555V3_LOGI("Begin\n");
	WARN_ON(!pdev);

	ret = hi6555v3_base_addr_map(pdev);
	if (ret) {
		HI6555V3_LOGE("hi6555v3_base_addr_map failed! err code 0x%x\n", ret);
		hi6555v3_base_addr_unmap();
		return ret;
	}

	ret = snd_soc_register_codec(&pdev->dev, &soc_codec_dev_hi6555v3, hi6555v3_dai, ARRAY_SIZE(hi6555v3_dai));
	if (ret) {
		HI6555V3_LOGE("snd_soc_register_codec failed! err code 0x%x\n", ret);
		hi6555v3_base_addr_unmap();
	}

	HI6555V3_LOGI("End\n");
	return ret;
}

/* will not be called */
static int hi6555v3_remove(struct platform_device *pdev)
{
	HI6555V3_LOGI("Begin\n");
	WARN_ON(!pdev);

	snd_soc_unregister_codec(&pdev->dev);
	hi6555v3_base_addr_unmap();
	HI6555V3_LOGI("End\n");

	return 0;
}

static const struct of_device_id hi6555v3_codec_match[] = {
	{ .compatible = "hisilicon,hi6555v3-codec", },
	{},
};

#ifdef CONFIG_PM
int hi6555v3_runtime_suspend(struct device *dev)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(soc_codec);

	HI6555V3_LOGI("begin\n");

	if (!priv) {
		HI6555V3_LOGE("get drvdata failed\n");
		return -EINVAL;
	}

	if (UDP_PLATFORM == priv->platform_type) {
		ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
		if (ret) {
			HI6555V3_LOGE("could not set pins to idle state, ret = %d\n", ret);
			return ret;
		}
	}

	_asp_clk_disable(soc_codec);

	HI6555V3_LOGI("end\n");
	return 0;
}

int hi6555v3_runtime_resume(struct device *dev)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(soc_codec);

	HI6555V3_LOGI("begin\n");

	if (!priv) {
		HI6555V3_LOGE("get drvdata failed\n");
		return -EINVAL;
	}

	ret = _asp_clk_enable(soc_codec);
	if (ret) {
		HI6555V3_LOGE("could not enable asp resource, ret = %d\n", ret);
		return ret;
	}

	if (UDP_PLATFORM == priv->platform_type) {
		ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
		if (ret) {
			HI6555V3_LOGE("could not set pins to default state, ret = %d\n", ret);
			return ret;
		}
	}

	HI6555V3_LOGI("end\n");
	return 0;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int hi6555v3_suspend(struct device *dev)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(soc_codec);

	HI6555V3_LOGI("begin\n");

	if (!dev) {
		HI6555V3_LOGE("device is null\n");
		return -EINVAL;
	}

	if (!priv) {
		HI6555V3_LOGE("get drvdata failed\n");
		return -EINVAL;
	}

	if (priv->pm_runtime_support) {
		ret = pm_runtime_get_sync(dev);
		HI6555V3_LOGD("pm suspend ret:%d\n", ret);
		if (ret < 0) {
			HI6555V3_LOGE("pm suspend error, ret:%d\n", ret);
			return ret;
		}
	}

	HI6555V3_LOGD("suspend usage_count:0x%x status:0x%x disable_depth:%d asp_subsys_clk:%d, asp_49m_clk:%d\n",
			atomic_read(&(dev->power.usage_count)),
			dev->power.runtime_status,
			dev->power.disable_depth,
			clk_get_enable_count(priv->asp_subsys_clk),
			clk_get_enable_count(priv->asp_49m_clk));


	if (!priv->have_dapm) {
		/* set pin to low power mode */
		if (UDP_PLATFORM == priv->platform_type) {
			ret = pinctrl_select_state(priv->pctrl, priv->pin_idle);
			if (ret) {
				HI6555V3_LOGE("could not set pins to idle state, ret=%d\n", ret);
				return ret;
			}
		}
		_asp_resource_disable(soc_codec);
		priv->asp_pd = true;

		HI6555V3_LOGI("suspend without dapm\n");
	}

	HI6555V3_LOGI("end\n");
	return 0;
}

static int hi6555v3_resume(struct device *dev)
{
	int ret;
	struct hi6555v3_priv *priv = snd_soc_codec_get_drvdata(soc_codec);

	HI6555V3_LOGI("resume begin\n");

	if (!dev) {
		HI6555V3_LOGE("device is null\n");
		return -EINVAL;
	}

	if (!priv) {
		HI6555V3_LOGE("get drvdata failed\n");
		return -EINVAL;
	}

	if (priv->asp_pd) {
		ret = _asp_resource_enable(soc_codec);
		if (ret) {
			HI6555V3_LOGE("asp resource enable error, ret: %d\n", ret);
			return ret;
		}

		priv->asp_pd = false;

		_hi6555v3_asp_reg_init(soc_codec);

		if (UDP_PLATFORM == priv->platform_type) {
			ret = pinctrl_select_state(priv->pctrl, priv->pin_default);
			if (ret) {
				HI6555V3_LOGE("could not set pins to default state, ret=%d\n", ret);
				return ret;
			}
		}
		HI6555V3_LOGI("resume from asp pd\n");
	}

	if (priv->pm_runtime_support) {
		pm_runtime_mark_last_busy(dev);
		pm_runtime_put_autosuspend(dev);

		pm_runtime_disable(dev);
		pm_runtime_set_active(dev);
		pm_runtime_enable(dev);
	}

	HI6555V3_LOGD("resume usage_count:0x%x status:0x%x disable_depth:%d asp_subsys_clk:%d, asp_49m_clk:%d\n",
			atomic_read(&(dev->power.usage_count)),
			dev->power.runtime_status,
			dev->power.disable_depth,
			clk_get_enable_count(priv->asp_subsys_clk),
			clk_get_enable_count(priv->asp_49m_clk));

	HI6555V3_LOGI("resume end\n");
	return 0;
}
#endif

struct snd_soc_codec *hi6555v3_get_codec(void)
{
	return soc_codec;
}

const struct dev_pm_ops hi6555v3_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(hi6555v3_suspend, hi6555v3_resume)
	SET_RUNTIME_PM_OPS(hi6555v3_runtime_suspend, hi6555v3_runtime_resume, NULL)
};

static struct platform_driver hi6555v3_driver = {
	.driver = {
		.name  = "hi6555v3-codec",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hi6555v3_codec_match),
		.pm = &hi6555v3_pm_ops,
	},
	.probe  = hi6555v3_probe,
	.remove = hi6555v3_remove,
	.shutdown = hi6555v3_shutdown,
};

static int __init hi6555v3_codec_init(void)
{
	HI6555V3_LOGI("Begin\n");
	return platform_driver_register(&hi6555v3_driver);
}
module_init(hi6555v3_codec_init);

static void __exit hi6555v3_codec_exit(void)
{
	HI6555V3_LOGI("Begin\n");
	platform_driver_unregister(&hi6555v3_driver);
}
module_exit(hi6555v3_codec_exit);

MODULE_DESCRIPTION("ASoC hi6555v3 driver");
MODULE_AUTHOR("xieliyin <xieliyin@huawei.com>");
MODULE_LICENSE("GPL");

