/*
 * hi6555v3_driver codec driver.
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555v3.h"
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
#include <linux/pm_runtime.h>
#include <sound/hisi_pa.h>

#include "hi6xxx_mbhc.h"
#include "hi6555v3_asp_reg_def.h"
#include "hi6555v3_pmu_reg_def.h"

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hicodec_debug.h"
#endif

#include "audio_log.h"
#include "hi6555_i2s.h"
#include "hi6555_codec.h"
#include "hi6555_utils.h"
#include "hi6555_store.h"
#include "hi6555_smpa_path.h"

/*lint -e548*/
#define LOG_TAG "hi6555v3"

static struct snd_soc_codec *soc_codec;

#define I2S1_TDM_FS_MASK 0xC00000
#define FS_VOICE_L_DLINK_MASK 0xC0000000
#define FS_VOICE_L_DLINK_VALUE 0x0
#define FS_VOICE_R_DLINK_MASK 0x3
#define FS_VOICE_R_DLINK_VALUE 0x0
#define BT_L_RX_PGA_GAIN_MASK 0xFF000000
#define BT_L_RX_PGA_GAIN_VALUE 0x89000000
#define BT_R_RX_PGA_GAIN_MASK 0xFF000000
#define BT_R_RX_PGA_GAIN_VALUE 0x89000000
#define FS_VOICE_R_DN_SRCUP_IN_MASK 0xC
#define FS_VOICE_R_DN_SRCUP_IN_VALUE 0x0
#define FS_I2S2_TX_L_SRCDN_DOUT_MASK 0x380000
#define FS_I2S2_TX_L_SRCDN_DOUT_VALUE 0x80000
#define I2S2_TX_R_SRCDN_DOUT_FS_MASK 0x38000
#define I2S2_TX_R_SRCDN_DOUT_FS_VALUE 0x10000
#define DIG_IO_DS_CODEC_MASK 0xF
#define ANA_DACLR_EN_MASK   0x3
#define ANA_CLASSD_EN_MASK  0x9
#define MUTE_DAC_VALUE_MASK  0xE00000
#define MUTE_DAC_VALUE       0xE00000
#define FS_ECHO_L_UPLINK_MASK 0x1C0000
#define FS_ECHO_L_UPLINK_VALUE 0xC0000
#define FS_ECHO_R_UPLINK_MASK 0x38000
#define FS_ECHO_R_UPLINK_VALUE 0x18000

#define DEFAULT_SIF_DRIVING_FORCE 5
#define MAX_SIF_DRIVING_FORCE 0xF
#define LDO8_VOLTAGE 1800000
#define CLASSD_DEFAULT_VOLTAGE    4500

#define CLASSD_MIN_VOLTAGE        4500  /* min voltage config */
#define CLASSD_MAX_VOLTAGE        5500  /* here we set 5.5V as the max value */
#define CLASSD_VOLTAGE_DIV        100


enum classd_supply_type {
	CLASSD_SUPPLY_SCHARGER,
	CLASSD_SUPPLY_GPIO,
	CLASSD_SUPPLY_INVALID,
};

struct hi6555v3_customize {
	unsigned int mute_dacl_reg_val;
	unsigned int mute_dacr_reg_val;
	int classd_scharger_voltage;
	enum classd_supply_type supply_type;
	struct regulator *schg_regulator;
	signed char gain_change_db;
	unsigned int usb_ana_ldo_need_control;
	struct regulator *usb_ana_ldo;
};

static struct hi6555v3_customize codec_customize;

static int classd_schg_voltage_get(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = NULL;

	IN_FUNCTION;

	codec = snd_soc_kcontrol_codec(kcontrol);

	ucontrol->value.integer.value[0] = codec_customize.classd_scharger_voltage;

	OUT_FUNCTION;

	return 0;
}

static int classd_schg_voltage_put(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	int classd_voltage;
	struct snd_soc_codec *codec = NULL;

	IN_FUNCTION;

	codec = snd_soc_kcontrol_codec(kcontrol);

	if (codec_customize.supply_type != CLASSD_SUPPLY_SCHARGER) {
		AUDIO_LOGE("class supply type is not sharger, CLASSD_SCHG_VOLTAGE_CONFIG should not exist in path xml");
		return -ENXIO;
	}

	/* The default voltage is 4.5,set by pmu.
	 * And here set range 4.5~5.2
	 */
	classd_voltage = ucontrol->value.integer.value[0];
	if ((classd_voltage % CLASSD_VOLTAGE_DIV) ||
		(classd_voltage < CLASSD_MIN_VOLTAGE) ||
		(classd_voltage > CLASSD_MAX_VOLTAGE)) {
		AUDIO_LOGE("classd voltage config: %d in xml is not support",
			classd_voltage);
		return -EINVAL;
	}

	codec_customize.classd_scharger_voltage = classd_voltage;
		AUDIO_LOGI("classd voltage config: %d ok", classd_voltage);

	OUT_FUNCTION;

	return 0;
}

#define CUSTOMER_KCONTROLS \
	SOC_SINGLE_EXT("CLASSD_SCHG_VOLTAGE_CONFIG", VIR1_ADDR, REG_BIT0, 0xffff, 0, \
		classd_schg_voltage_get, classd_schg_voltage_put), \

static const struct snd_kcontrol_new hi6555v3_kcontrols[] = {
	CUSTOMER_KCONTROLS
};

static void hplr_pga_power_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	snd_soc_write(codec, CODEC_ANA_RW5_REG, 0x40);
	snd_soc_write(codec, CODEC_ANA_RW5_REG, 0x4C);
	msleep(30);
	snd_soc_write(codec, CODEC_ANA_RW5_REG, 0x7C);
	snd_soc_write(codec, CODEC_ANA_RW19_REG, 0x8C);
	snd_soc_write(codec, CODEC_ANA_RW5_REG, 0xFC);
	snd_soc_write(codec, CODEC_ANA_RW24_REG, 0x36);
	snd_soc_write(codec, CODEC_ANA_RW20_REG, 0x00);

	OUT_FUNCTION;
}

static void ana_dac_power_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	/* close dac */
	snd_soc_update_bits(codec, DAC_CHAN_CTRL_REG,
		BIT(DACL_DWA_EN_OFFSET), 0);
	snd_soc_update_bits(codec, CODEC_ANA_RW4_REG,
		BIT(DACL_PD_OFFSET), BIT(DACL_PD_OFFSET));
	snd_soc_update_bits(codec, DAC_CHAN_CTRL_REG,
		BIT(DACR_DWA_EN_OFFSET), 0);
	snd_soc_update_bits(codec, CODEC_ANA_RW4_REG,
		BIT(DACR_PD_OFFSET), BIT(DACR_PD_OFFSET));

	/* close  daclr-d */
	snd_soc_update_bits(codec, CLK_EN_CFG_REG, ANA_DACLR_EN_MASK, 0x0);

	OUT_FUNCTION;
}

static void headphone_pop_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	hplr_pga_power_off(codec);
	ana_dac_power_off(codec);

	OUT_FUNCTION;
}

static void classd_pop_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	if (codec_customize.supply_type >= CLASSD_SUPPLY_INVALID) {
		AUDIO_LOGI("no need to pop off");
		return;
	}

	snd_soc_update_bits(codec, CTRL_REG_CLASSD_CTRL0_REG,
		ANA_CLASSD_EN_MASK, 0x9);

	OUT_FUNCTION;
}

static void set_power_codec(struct snd_soc_codec *codec, int on)
{
	IN_FUNCTION;

	if (on) {
		snd_soc_update_bits(codec, CODEC_ANA_RW10_REG,
			BIT(DISCHARGE_DISABLE_OFFSET), BIT(DISCHARGE_DISABLE_OFFSET));
#ifndef AUDIO_FACTORY_MODE
		snd_soc_update_bits(codec, CODEC_ANA_RW12_REG,
			BIT(IDEL_TONE_CTRL_OFFSET), BIT(IDEL_TONE_CTRL_OFFSET));
#endif
		snd_soc_update_bits(codec, CODEC_ANA_RW3_REG,
			BIT(ADCL_MUTE_OFFSET) | BIT(ADCR_MUTE_OFFSET) |
			BIT(ADC3_MUTE_OFFSET), BIT(ADCL_MUTE_OFFSET) |
			BIT(ADCR_MUTE_OFFSET) | BIT(ADC3_MUTE_OFFSET));
		snd_soc_update_bits(codec, CODEC_ANA_RW22_REG,
			BIT(STB_CTRL_SEC_OFFSET) | BIT(STB_CTRL_OFFSET),
			BIT(STB_CTRL_SEC_OFFSET) | BIT(STB_CTRL_OFFSET));
		snd_soc_update_bits(codec, CODEC_ANA_RW23_REG,
			BIT(STB_1N_OFFSET) | BIT(STB_N12DB_GAIN_OFFSET),
			BIT(STB_1N_OFFSET) | BIT(STB_N12DB_GAIN_OFFSET));
		snd_soc_write(codec, CODEC_ANA_RW1_REG, 0x07);
		msleep(6);
		snd_soc_write(codec, CODEC_ANA_RW1_REG, 0x03);
		snd_soc_write(codec, CODEC_ANA_RW1_REG, 0x01);
	} else {
		snd_soc_write(codec, CODEC_ANA_RW1_REG, 0x04);
	}

	OUT_FUNCTION;
}

static void set_ibias(struct snd_soc_codec *codec, bool enable)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	AUDIO_LOGD("Begin, en=%d", enable);

	if (!priv->ibias.ibias_hsmicbias_en && priv->ibias.ibias_work == 0) {
		if (enable) {
			AUDIO_LOGI("codec power up");
			set_power_codec(codec, true);
		} else {
			AUDIO_LOGI("codec power down");
			set_power_codec(codec, false);
		}
	}
	AUDIO_LOGD("End");
}

static void hi6555v3_enable_hsd(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, CODEC_ANA_RW32_REG, 0x1);
}

static void hi6555v3_set_ibias_hsmicbias(struct snd_soc_codec *codec, bool enable)
{
	struct platform_data *priv = NULL;

	IN_FUNCTION;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	priv = snd_soc_codec_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv pointer is null");
		return;
	}

	mutex_lock(&priv->ibias.ibias_mutex);
	if (enable) {
		if (!priv->ibias.ibias_hsmicbias_en) {
			set_ibias(codec, true);
			priv->ibias.ibias_hsmicbias_en = true;
		}
	} else {
		if (priv->ibias.ibias_hsmicbias_en) {
			priv->ibias.ibias_hsmicbias_en = false;
			set_ibias(codec, false);
		}
	}
	mutex_unlock(&priv->ibias.ibias_mutex);

	OUT_FUNCTION;
}

static void reset_pmu_codec(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	/* smartstar codec reset */
	snd_soc_write(codec, CTRL_REG_PMU_SOFT_RST_REG, 0x22);
	usleep_range(10000, 11000);
	snd_soc_write(codec, CTRL_REG_PMU_SOFT_RST_REG, 0x2F);

	OUT_FUNCTION;
}

static void reset_asp_codec(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	snd_soc_write(codec, R_RST_CTRLEN, BIT(RST_EN_CODEC_N));
	snd_soc_write(codec, R_RST_CTRLDIS, BIT(RST_DISEN_CODEC_N));
	snd_soc_write(codec, R_GATE_EN, BIT(GT_CODEC_CLK));
	snd_soc_update_bits(codec, R_CODEC_DMA_SEL,
		BIT(CODEC_DMA_SEL), BIT(CODEC_DMA_SEL));

	OUT_FUNCTION;
}

static void asp_pmu_reg_init(void)
{
	IN_FUNCTION;

	/* disable mixer */
	snd_soc_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG,
			BIT(DACL_MIXER4_IN1_MUTE_OFFSET) | BIT(DACL_MIXER4_IN2_MUTE_OFFSET) |
			BIT(DACL_MIXER4_IN3_MUTE_OFFSET) | BIT(DACL_MIXER4_IN4_MUTE_OFFSET),
			BIT(DACL_MIXER4_IN1_MUTE_OFFSET) | BIT(DACL_MIXER4_IN2_MUTE_OFFSET) |
			BIT(DACL_MIXER4_IN3_MUTE_OFFSET) | BIT(DACL_MIXER4_IN4_MUTE_OFFSET));

	snd_soc_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG,
			BIT(DACR_MIXER4_IN1_MUTE_OFFSET) | BIT(DACR_MIXER4_IN2_MUTE_OFFSET) |
			BIT(DACR_MIXER4_IN3_MUTE_OFFSET) | BIT(DACR_MIXER4_IN4_MUTE_OFFSET),
			BIT(DACR_MIXER4_IN1_MUTE_OFFSET) | BIT(DACR_MIXER4_IN2_MUTE_OFFSET) |
			BIT(DACR_MIXER4_IN3_MUTE_OFFSET) | BIT(DACR_MIXER4_IN4_MUTE_OFFSET));

	snd_soc_update_bits(soc_codec, CODEC_ANA_RW12_REG,
		BIT(ADCL_MIXIN_DACL_OFFSET) | BIT(ADCL_MIXIN_DACR_OFFSET) |
		BIT(ADCL_MIXIN_MICPGA1_OFFSET) | BIT(ADCL_MIXIN_MICPGA2_OFFSET), 0);

	snd_soc_update_bits(soc_codec, CODEC_ANA_RW14_REG,
		BIT(ADCR_MIXIN_DACL_OFFSET) | BIT(ADCR_MIXIN_DACR_OFFSET) |
		BIT(ADCR_MIXIN_MICPGA1_OFFSET) | BIT(ADCR_MIXIN_MICPGA2_OFFSET), 0);

	snd_soc_update_bits(soc_codec, CODEC_ANA_RW16_REG,
		BIT(ADC3_MIXIN_DACL_OFFSET) | BIT(ADC3_MIXIN_ULTRA_OFFSET) |
		BIT(ADC3_MIXIN_MICPGA1_OFFSET) | BIT(ADC3_MIXIN_MICPGA3_OFFSET), 0);

	snd_soc_update_bits(soc_codec, CODEC_ANA_RW20_REG,
		BIT(MIXOUT_HSL_DACL_OFFSET) | BIT(MIXOUT_HSL_DACR_OFFSET) |
		BIT(MIXOUT_HSR_DACL_OFFSET) | BIT(MIXOUT_HSR_DACR_OFFSET), 0);

	snd_soc_update_bits(soc_codec, CODEC_ANA_RW25_REG,
		BIT(MIXOUT_EAR_DACL_OFFSET) | BIT(MIXOUT_EAR_DACR_OFFSET) |
		BIT(MIXOUT_EAR_DAC_ULTRA_OFFSET) | BIT(CR_MIXER_EP_DACL_OFFSET) |
		BIT(CR_MIXER_EP_DACR_OFFSET) | BIT(CR_MIXER_EP_DAC_ULTRA_OFFSET), 0);

	snd_soc_update_bits(soc_codec, CTRL_REG_CLASSD_CTRL0_REG,
		BIT(CTRL_REG_CLASSD_MUTE_SEL_OFFSET),
		BIT(CTRL_REG_CLASSD_MUTE_SEL_OFFSET));

	OUT_FUNCTION;
}

static void classh_clk_enable_init(struct snd_soc_codec *codec)
{
	struct device_node *np = codec->dev->of_node;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	if (of_property_read_bool(np, "classh_clk_enable"))
		priv->classh_clk_enable = true;
	else
		priv->classh_clk_enable = false;

	AUDIO_LOGI("The value of classh_clk_enable is %d\n",
		priv->classh_clk_enable);
}

static void asp_soc_reg_init(void)
{
	IN_FUNCTION;

	hisi_pa_asp_soc_reg_init(soc_codec);

	/* set i2s1 tdm mode fs */
	snd_soc_update_bits(soc_codec, FS_CTRL3_REG,
		I2S1_TDM_FS_MASK, I2S1_TDM_FS_MASK);

	/* set voice dl fs */
	snd_soc_update_bits(soc_codec, FS_CTRL0_REG,
		FS_VOICE_L_DLINK_MASK, FS_VOICE_L_DLINK_VALUE);

	/* set voice dr fs */
	snd_soc_update_bits(soc_codec, FS_CTRL2_REG,
		FS_VOICE_R_DLINK_MASK, FS_VOICE_R_DLINK_VALUE);

	/* set bt l rx pga gain */
	snd_soc_update_bits(soc_codec, BT_L_RX_PGA_CTRL_REG,
		BT_L_RX_PGA_GAIN_MASK, BT_L_RX_PGA_GAIN_VALUE);

	/* set bt r rx pga gain */
	snd_soc_update_bits(soc_codec, BT_R_RX_PGA_CTRL_REG,
		BT_R_RX_PGA_GAIN_MASK, BT_R_RX_PGA_GAIN_VALUE);

	/* set i2s1 tdm tx sel */
	snd_soc_update_bits(soc_codec, I2S1_TDM_CTRL0_REG,
		BIT(S1_TDM_TX_CLK_SEL_OFFSET), 0);

	/* set voice r dn src in fs */
	snd_soc_update_bits(soc_codec, FS_CTRL2_REG,
		FS_VOICE_R_DN_SRCUP_IN_MASK, FS_VOICE_R_DN_SRCUP_IN_VALUE);

	/* set i2s2 txl dout fs */
	snd_soc_update_bits(soc_codec, FS_CTRL3_REG,
		FS_I2S2_TX_L_SRCDN_DOUT_MASK, FS_I2S2_TX_L_SRCDN_DOUT_VALUE);

	/* set i2s2 txr dout fs */
	snd_soc_update_bits(soc_codec, FS_CTRL3_REG,
		I2S2_TX_R_SRCDN_DOUT_FS_MASK, I2S2_TX_R_SRCDN_DOUT_FS_VALUE);

	/* set echo l uplink fs */
	snd_soc_update_bits(soc_codec, FS_CTRL0_REG,
		FS_ECHO_L_UPLINK_MASK, FS_ECHO_L_UPLINK_VALUE);

	/* set echo r uplink fs */
	snd_soc_update_bits(soc_codec, FS_CTRL0_REG,
		FS_ECHO_R_UPLINK_MASK, FS_ECHO_R_UPLINK_VALUE);

	OUT_FUNCTION;
}

static void hi6555v3_asp_reg_init(struct snd_soc_codec *codec)
{
	IN_FUNCTION;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	reset_asp_codec(codec);
	asp_pmu_reg_init();
	asp_soc_reg_init();

	OUT_FUNCTION;
}

static void hsmicbias_force_close(struct snd_soc_codec *codec)
{
	unsigned int hs_irq_mask;

	/* to avoid irq while MBHD_COMP power down, mask all COMP irq */
	hs_irq_mask = snd_soc_read(codec, ANA_IRQM_REG0_REG);
	snd_soc_update_bits(codec, ANA_IRQM_REG0_REG,
		hs_irq_mask | IRQ_MSK_COMP, hs_irq_mask | IRQ_MSK_COMP);

	/* disable NORMAL key detect and identify */
	snd_soc_update_bits(codec, CODEC_ANA_RW8_REG,
		BIT(MBHD_BUFF_PD_OFFSET), BIT(MBHD_BUFF_PD_OFFSET));
	snd_soc_update_bits(codec, CODEC_ANA_RW8_REG,
		BIT(MBHD_COMP_PD_OFFSET), BIT(MBHD_COMP_PD_OFFSET));

	/* hs micbias pd */
	snd_soc_update_bits(codec, CODEC_ANA_RW7_REG,
		BIT(HSMICB_PD_OFFSET), BIT(HSMICB_PD_OFFSET));
	snd_soc_update_bits(codec, CODEC_ANA_RW7_REG,
		BIT(HSMICB_DSCHG_OFFSET), BIT(HSMICB_DSCHG_OFFSET));
	usleep_range(5000, 5500);
	snd_soc_update_bits(codec, CODEC_ANA_RW7_REG,
		BIT(HSMICB_DSCHG_OFFSET), 0);
}

static int audio_codec_register_hisi_pa(struct snd_soc_codec *codec)
{
	int ret = 0;
	const char *hisi_pa = NULL;
	struct device_node *np = codec->dev->of_node;

	if (of_property_read_bool(np, "is_hisi_pa")) {
		ret = of_property_read_string(np, "is_hisi_pa", &hisi_pa);
		if (ret < 0) {
			AUDIO_LOGE("get is_hisi_pa failed");
			return ret;
		}
		if(strcmp(hisi_pa,"true") == 0)
			ret = hisi_pa_set_default_func(codec);
	}
	return ret;
}

void audio_codec_mute_pga(bool mute)
{
	volatile unsigned int val = 0;
	struct platform_data *priv = NULL;
	struct hi6555v3_customize *customize = NULL;

	if (soc_codec == NULL) {
		AUDIO_LOGE("soc_codec is NULL");
		return;
	}

	priv = snd_soc_codec_get_drvdata(soc_codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv is NULL");
		return;
	}

	customize = priv->customized_data;
	if (customize == NULL) {
		AUDIO_LOGE("customize is NULL");
		return;
	}

	if (customize->supply_type != CLASSD_SUPPLY_SCHARGER) {
		AUDIO_LOGI("classd supply type is %d, need not mute/umute", customize->supply_type);
		return;
	}

	if (mute) {
		/* mute audio_dlr_mixer */
		val = snd_soc_read(soc_codec, DACL_MIXER4_CTRL0_REG);
		if (val != INVALID_REG_VALUE) {
			customize->mute_dacl_reg_val = val;
			snd_soc_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, MUTE_DAC_VALUE);
		} else {
			AUDIO_LOGE("read dacl reg fail");
		}

		val = snd_soc_read(soc_codec, DACR_MIXER4_CTRL0_REG);
		if (val != INVALID_REG_VALUE) {
			customize->mute_dacr_reg_val = val;
			snd_soc_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, MUTE_DAC_VALUE);
		} else {
			AUDIO_LOGE("read dacl reg fail");
		}

		AUDIO_LOGI("mute audio_dlr_mixer for flash driver");
	} else {
		/* unmute audio_dlr_mixer */
		if (customize->mute_dacl_reg_val != INVALID_REG_VALUE) {
			snd_soc_update_bits(soc_codec, DACL_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, customize->mute_dacl_reg_val);
			customize->mute_dacl_reg_val = INVALID_REG_VALUE;
		}

		if (customize->mute_dacr_reg_val != INVALID_REG_VALUE) {
			snd_soc_update_bits(soc_codec, DACR_MIXER4_CTRL0_REG, MUTE_DAC_VALUE_MASK, customize->mute_dacr_reg_val);
			customize->mute_dacr_reg_val = INVALID_REG_VALUE;
		}
		AUDIO_LOGI("unmute audio_dlr_mixer for flash driver");
	}
}
EXPORT_SYMBOL_GPL(audio_codec_mute_pga);

static void hi6555v3_shutdown(struct platform_device *dev)
{
	AUDIO_LOGI("Begin");

	if (soc_codec == NULL) {
		AUDIO_LOGE("soc_codec is NULL");
		return;
	}

	/* headset pop process */
	headphone_pop_off(soc_codec);

	/* classd pop process */
	classd_pop_off(soc_codec);

	/* close hsmicbias */
	hsmicbias_force_close(soc_codec);

	/* close ibias and Avref */
	set_power_codec(soc_codec, false);

	/* reset codec */
	snd_soc_write(soc_codec, CTRL_REG_PMU_SOFT_RST_REG, 0x22);

	AUDIO_LOGI("End");
}

static void set_platform_type(struct snd_soc_codec *codec)
{
	struct platform_data *priv = NULL;
	struct device_node *np = codec->dev->of_node;
	unsigned int val = 0;

	priv = snd_soc_codec_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return;
	}

	if (!of_property_read_u32(np, "udp_io_config", &val)) {
		AUDIO_LOGI("udp_io_config is %d", val);
		if (val == 1)
			priv->board_type = UDP_BOARD;
		else
			priv->board_type = FPGA_BOARD;

	} else {
		AUDIO_LOGI("udp_io_config is not configed, set fpga default");
		priv->board_type = FPGA_BOARD;
	}

}

static int asp_resource_init(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	priv->resource.asp_regulator = devm_regulator_get(codec->dev, "asp");
	if (IS_ERR(priv->resource.asp_regulator)) {
		AUDIO_LOGE("get asp regulators err: %pK", priv->resource.asp_regulator);
		ret = PTR_ERR(priv->resource.asp_regulator);
		goto get_asp_regulator_err;
	}

	priv->resource.asp_subsys_clk = devm_clk_get(codec->dev, "clk_asp_subsys");
	if (IS_ERR_OR_NULL(priv->resource.asp_subsys_clk)) {
		AUDIO_LOGE("get clk_asp_subsys err: %pK", priv->resource.asp_subsys_clk);
		ret = PTR_ERR(priv->resource.asp_subsys_clk);
		goto get_asp_subsys_clk_err;
	}

	priv->resource.asp_codec_clk = devm_clk_get(codec->dev, "clk_asp_codec");
	if (IS_ERR_OR_NULL(priv->resource.asp_codec_clk)) {
		AUDIO_LOGE("get clk_asp_codec err: %pK", priv->resource.asp_codec_clk);
		ret = PTR_ERR(priv->resource.asp_codec_clk);
		goto get_asp_codec_clk_err;
	}

	AUDIO_LOGI("asp resource init ok");
	goto end;

get_asp_codec_clk_err:
	priv->resource.asp_codec_clk = NULL;
get_asp_subsys_clk_err:
	priv->resource.asp_subsys_clk = NULL;
get_asp_regulator_err:
	priv->resource.asp_regulator = NULL;
end:
	OUT_FUNCTION;
	return ret;
}

static void asp_resource_deinit(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	priv->resource.asp_regulator = NULL;
	priv->resource.asp_subsys_clk = NULL;
	priv->resource.asp_codec_clk = NULL;

	AUDIO_LOGI("asp resource deinit ok");
}

static void schg_set_classd_supply(struct snd_soc_codec *codec, bool enable)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);
	struct hi6555v3_customize *customize = priv->customized_data;
	struct regulator *regulator_schg = customize->schg_regulator;
	int ret;

	if (IS_ERR(regulator_schg)) {
		AUDIO_LOGE("regulator_schg_boost3 is error: %ld",
			PTR_ERR(regulator_schg));
		return;
	}

	if (enable) {
		ret = regulator_enable(regulator_schg);
		if (ret) {
			AUDIO_LOGE("regulator_schg_boost3 enable fail, ret = %d", ret);
			return;
		}

		AUDIO_LOGI("classd schg regulator enable ok");
	} else {
		if (!regulator_is_enabled(regulator_schg)) {
			AUDIO_LOGW("regulator_schg_boost3 is not enabled when disable");
			return;
		}

		ret = regulator_disable(regulator_schg);
		if (ret) {
			AUDIO_LOGE("regulator_schg_boost3 enable fail, ret = %d", ret);
			return;
		}

		AUDIO_LOGI("classd schg regulator disable ok");
	}
}

static void gpio_set_classd_supply(struct snd_soc_codec *codec, bool enable)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (!gpio_is_valid(priv->gpio_classd)) {
		AUDIO_LOGE("switch classd gpio: %d fail", priv->gpio_classd);
		return;
	}

	if (enable) {
		gpio_set_value(priv->gpio_classd, GPIO_LEVEL_HIGH);
		AUDIO_LOGI("pull up classd gpio ok");
	} else {
		gpio_set_value(priv->gpio_classd, GPIO_LEVEL_LOW);
		AUDIO_LOGI("pull down classd gpio ok");
	}

	OUT_FUNCTION;
}

static void hi6555v3_set_classd_supply(struct snd_soc_codec *codec, bool enable)
{
	enum classd_supply_type supply_type = codec_customize.supply_type;

	if (codec == NULL) {
		AUDIO_LOGE("codec is null");
		return;
	}

	if (supply_type == CLASSD_SUPPLY_SCHARGER)
		schg_set_classd_supply(codec, enable);
	else if (supply_type == CLASSD_SUPPLY_GPIO)
		gpio_set_classd_supply(codec, enable);
}

static int pmu_resource_init(struct snd_soc_codec *codec)
{
	int ret;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	priv->resource.pmu_regulator = devm_regulator_get(codec->dev, "codec_ldo8");
	if (IS_ERR(priv->resource.pmu_regulator)) {
		AUDIO_LOGE("couldn't get pmu regulator ret: %ld", PTR_ERR(priv->resource.pmu_regulator));
		ret = PTR_ERR(priv->resource.pmu_regulator);
		priv->resource.pmu_regulator = NULL;
		return ret;
	}

	ret = regulator_set_voltage(priv->resource.pmu_regulator, LDO8_VOLTAGE, LDO8_VOLTAGE);
	if (ret) {
		AUDIO_LOGE("regulator set pmu regulator voltage failed");
		return ret;
	}

	ret = regulator_enable(priv->resource.pmu_regulator);
	if (ret) {
		AUDIO_LOGE("couldn't enable pmu regulator %d", ret);
		return ret; /* lint !e527 */
	}

	priv->resource.set_classd_supply = hi6555v3_set_classd_supply;
	AUDIO_LOGI("pmu resource init ok");

	OUT_FUNCTION;

	return 0;
}

static void pmu_resource_deinit(struct snd_soc_codec *codec)
{
	int ret;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	if (!IS_ERR(priv->resource.pmu_regulator)) {
		ret = regulator_disable(priv->resource.pmu_regulator);
		if (ret)
			AUDIO_LOGE("couldn't disable pmu %d", ret);
	}

	AUDIO_LOGI("pmu resource deinit ok");
}

static int pinctrl_init(struct snd_soc_codec *codec)
{
	int ret;
	struct device *dev = codec->dev;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->board_type == FPGA_BOARD) {
		snd_soc_write(codec, 0xFFF11714, 0x2); /* sif_do0 GPIO_194 */
		snd_soc_write(codec, 0xFFF11718, 0x2); /* sif_di0 GPIO_193 */
		snd_soc_write(codec, 0xFFF1171c, 0x2); /* sif_sync GPIO_204 */
		snd_soc_write(codec, 0xFFF1172c, 0x2); /* sif_do1 GPIO_210 */
		snd_soc_write(codec, 0xFFF11758, 0x3); /* dsd_clk GPIO_221, used as sif_clk */
		snd_soc_write(codec, 0xFFF11760, 0x2); /* sif_di1 GPIO_223 */

		return 0;
	}

	/* I2S1 & I2S2 & SIF & claasd bst_5v_en iomux config */
	priv->resource.pin.pctrl = pinctrl_get(dev);
	if (IS_ERR(priv->resource.pin.pctrl)) {
		priv->resource.pin.pctrl = NULL;
		AUDIO_LOGE("could not get pinctrl");
		return -EFAULT;
	}

	priv->resource.pin.pin_default = pinctrl_lookup_state(priv->resource.pin.pctrl,
		PINCTRL_STATE_DEFAULT);
	if (IS_ERR(priv->resource.pin.pin_default)) {
		AUDIO_LOGE("could not get default state (%li)",
			PTR_ERR(priv->resource.pin.pin_default));
		goto pinctrl_operation_err;
	}

	priv->resource.pin.pin_idle = pinctrl_lookup_state(priv->resource.pin.pctrl, PINCTRL_STATE_IDLE);
	if (IS_ERR(priv->resource.pin.pin_idle)) {
		AUDIO_LOGE("could not get idle state (%li)", PTR_ERR(priv->resource.pin.pin_idle));
		goto pinctrl_operation_err;
	}

	ret = pinctrl_select_state(priv->resource.pin.pctrl, priv->resource.pin.pin_default);
	if (ret) {
		AUDIO_LOGE("could not set pins to default state, ret = %d", ret);
		goto pinctrl_operation_err;
	}

	AUDIO_LOGI("pinctrl init ok");

	OUT_FUNCTION;
	return 0;

pinctrl_operation_err:
	pinctrl_put(priv->resource.pin.pctrl);
	priv->resource.pin.pctrl = NULL;
	priv->resource.pin.pin_default = NULL;
	priv->resource.pin.pin_idle = NULL;

	return -EFAULT;
}

static void pinctrl_deinit(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->board_type == FPGA_BOARD)
		return;

	pinctrl_put(priv->resource.pin.pctrl);
	priv->resource.pin.pctrl = NULL;
	priv->resource.pin.pin_default = NULL;
	priv->resource.pin.pin_idle = NULL;
	AUDIO_LOGI("pinctrl deinit ok");

	OUT_FUNCTION;
}

static void classd_schg_init(struct platform_data *priv)
{
	struct device *dev = priv->codec->dev;

	IN_FUNCTION;

	codec_customize.schg_regulator = devm_regulator_get(dev, "schg_boost3");
	if (IS_ERR(codec_customize.schg_regulator)) {
		AUDIO_LOGE("get schg_boost3 regulator failed. error: %ld",
			PTR_ERR(codec_customize.schg_regulator));
		return;
	}

	AUDIO_LOGI("get schg_regulator ok");
	OUT_FUNCTION;
}

static void classd_schg_deinit(struct platform_data *priv)
{
	IN_FUNCTION;

	if (IS_ERR(codec_customize.schg_regulator)) {
		AUDIO_LOGE("schg regulator is error:%ld",
			PTR_ERR(codec_customize.schg_regulator));
		return;
	}

	devm_regulator_put(codec_customize.schg_regulator);
	codec_customize.schg_regulator = NULL;
	AUDIO_LOGI("put schg regulator ok");
	OUT_FUNCTION;
}

static void classd_gpio_init(struct platform_data *priv)
{
	int ret;
	unsigned int val = ARCH_NR_GPIOS;
	struct device_node *np  = priv->codec->dev->of_node;

	IN_FUNCTION;

	priv->gpio_classd = ARCH_NR_GPIOS;
	ret = of_property_read_u32(np, "gpio_classd", &val);
	if (ret) {
		AUDIO_LOGE("get gpio_classd failed. error: %d", ret);
		return;
	}

	if (!gpio_is_valid(val)) {
		AUDIO_LOGE("classd gpio: %d is invalied", val);
		return;
	}

	if (gpio_request(val, "classd_gpio")) {
		AUDIO_LOGE("request classd_gpio: %d failed", val);
		return;
	}

	priv->gpio_classd = val;
	if (gpio_direction_output(priv->gpio_classd, GPIO_LEVEL_LOW)) {
		gpio_free(priv->gpio_classd);
		priv->gpio_classd = ARCH_NR_GPIOS;
		AUDIO_LOGE("classd_gpio: %d pull down failed", val);
		return;
	}

	AUDIO_LOGI("classd gpio init ok");
	OUT_FUNCTION;
}

static void classd_gpio_deinit(struct platform_data *priv)
{
	IN_FUNCTION;

	if (!gpio_is_valid(priv->gpio_classd)) {
		AUDIO_LOGW("classd gpio%d is invalied", priv->gpio_classd);
		return;
	}

	gpio_free(priv->gpio_classd);
	priv->gpio_classd = ARCH_NR_GPIOS;

	AUDIO_LOGI("classd gpio deinit ok");
	OUT_FUNCTION;

}

static enum classd_supply_type get_classd_supply_type(const struct snd_soc_codec *codec)
{
	unsigned int value = CLASSD_SUPPLY_INVALID;
	struct device_node *np = codec->dev->of_node;

	IN_FUNCTION;

	if (of_property_read_u32(np, "classd_supply_type", &value)) {
		AUDIO_LOGW("can not find classd_supply_type,pls check if not use classd");
		return CLASSD_SUPPLY_INVALID;
	}

	if (value >= CLASSD_SUPPLY_INVALID) {
		AUDIO_LOGE("find invalied classd_supply_type %u", value);
		return CLASSD_SUPPLY_INVALID;
	}

	AUDIO_LOGI("classd_supply_type is %u", value);

	OUT_FUNCTION;

	return value;
}

static void classd_supply_init(struct snd_soc_codec *codec)
{
	enum classd_supply_type supply_type = codec_customize.supply_type;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	switch (supply_type) {
	case CLASSD_SUPPLY_SCHARGER:
		classd_schg_init(priv);
		break;
	case CLASSD_SUPPLY_GPIO:
		classd_gpio_init(priv);
		break;
	default:
		AUDIO_LOGW("classd supply init fail. error type is %d", supply_type);
		break;
	}

	OUT_FUNCTION;
}

static void classd_supply_deinit(struct snd_soc_codec *codec)
{
	int supply_type = codec_customize.supply_type;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	switch (supply_type) {
	case CLASSD_SUPPLY_SCHARGER:
		classd_schg_deinit(priv);
		break;
	case CLASSD_SUPPLY_GPIO:
		classd_gpio_deinit(priv);
		break;
	default:
		AUDIO_LOGW("classd supply deinit fail. error type is %d", supply_type);
		break;
	}

	OUT_FUNCTION;
}

static void hac_gpio_init(struct snd_soc_codec *codec)
{
	unsigned int val = 0;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);
	struct device_node *np = codec->dev->of_node;

	if (!of_property_read_u32(np, "hisilicon,hac_gpio", &val)) {
		priv->hac_en_gpio = val;
		AUDIO_LOGI("hac gpio num is %d", priv->hac_en_gpio);
	} else {
		priv->hac_en_gpio = ARCH_NR_GPIOS;
		AUDIO_LOGI("hac is not support.");
		return;
	}

	if (gpio_request(priv->hac_en_gpio, "hac_en_gpio")) {
		priv->hac_en_gpio = ARCH_NR_GPIOS;
		AUDIO_LOGE("hac_en_gpio request failed");
		return;
	}

	if (gpio_direction_output(priv->hac_en_gpio, 0)) {
		gpio_free(priv->hac_en_gpio);
		priv->hac_en_gpio = ARCH_NR_GPIOS;
		AUDIO_LOGE("hac_en_gpio set output failed");
		return;
	}
}

static void hac_gpio_deinit(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	if (gpio_is_valid(priv->hac_en_gpio)) {
		gpio_free(priv->hac_en_gpio);
		priv->hac_en_gpio = ARCH_NR_GPIOS;
	}
}

static void usb_ana_ldo_ctl(struct platform_data *priv, bool enable)
{
	struct hi6555v3_customize *customize = priv->customized_data;
	int cur_state;

	if (!customize->usb_ana_ldo) {
		AUDIO_LOGE("usb_ana_ldo regulators is NULL\n");
		return;
	}
	cur_state = regulator_is_enabled(customize->usb_ana_ldo);

	if (enable && cur_state <= 0) {
		if (regulator_enable(customize->usb_ana_ldo)) {
			AUDIO_LOGE("couldn't enable usb_ana_ldo regulators\n");
			return;
		}
		AUDIO_LOGI("enable usb_ana_ldo regulators already succ\n");
	} else if (!enable && cur_state > 0) {
		if (regulator_disable(customize->usb_ana_ldo)) {
			AUDIO_LOGE("couldn't disable usb_ana_ldo\n");
			return;
		}
		AUDIO_LOGI("disable usb_ana_ldo regulators already succ\n");
	}
}

static void usb_ana_ldo_resource_init(struct snd_soc_codec *codec)
{
	unsigned int val = 0;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);
	struct device_node *np = codec->dev->of_node;
	struct hi6555v3_customize *customize = priv->customized_data;

	if (!of_property_read_u32(np, "usb_ana_ldo_need_control", &val))
		customize->usb_ana_ldo_need_control = val;
	else
		customize->usb_ana_ldo_need_control = 0;

	customize->usb_ana_ldo = NULL;
	if (customize->usb_ana_ldo_need_control > 0) {
		customize->usb_ana_ldo = devm_regulator_get(codec->dev, "usb_ana_ldo");
		if (IS_ERR(customize->usb_ana_ldo)) {
			AUDIO_LOGE("couldn't get usb_ana_ldo regulator, %ld\n",
				PTR_ERR(customize->usb_ana_ldo));
			customize->usb_ana_ldo = NULL;
			return;
		}
	}
	AUDIO_LOGI("ldo_need_ctl = %u, ldo resource init ok\n",
		customize->usb_ana_ldo_need_control);
}

static void pm_runtime_init(struct snd_soc_codec *codec)
{
	struct device_node *np = codec->dev->of_node;
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (of_property_read_bool(np, "pm_runtime_support"))
		priv->pm.pm_runtime_support = true;
	else
		priv->pm.pm_runtime_support = false;

	AUDIO_LOGI("pm_runtime_support: %d", priv->pm.pm_runtime_support);

	if (priv->pm.pm_runtime_support) {
		pm_runtime_use_autosuspend(codec->dev);
		pm_runtime_set_autosuspend_delay(codec->dev, 200); /* 200ms */
		pm_runtime_set_active(codec->dev);
		pm_runtime_enable(codec->dev);
	}

	OUT_FUNCTION;
}

static void pm_runtime_deinit(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv->pm.pm_runtime_support) {
		pm_runtime_resume(codec->dev);
		pm_runtime_disable(codec->dev);
		pm_runtime_set_suspended(codec->dev);
	}

	OUT_FUNCTION;
}

static void cfg_sif_driving_force(struct snd_soc_codec *codec)
{
	unsigned int value = DEFAULT_SIF_DRIVING_FORCE;
	unsigned int dts_cfg = 0;
	struct device_node *np = codec->dev->of_node;

	IN_FUNCTION;

	if (!of_property_read_u32(np, "sif_driving_force", &dts_cfg)) {
		if (dts_cfg <= MAX_SIF_DRIVING_FORCE)
			value = dts_cfg;
	}

	snd_soc_update_bits(codec, CTRL_REG_DIG_IO_DS_CODEC_CFG_REG,
		DIG_IO_DS_CODEC_MASK, value);

	AUDIO_LOGI("sif_driving_force is %u", value);

	OUT_FUNCTION;
}

static int resource_init(struct snd_soc_codec *codec)
{
	int ret;

	IN_FUNCTION;

	set_platform_type(codec);

	ret = pmu_resource_init(codec);
	if (ret) {
		AUDIO_LOGE("pum codec resource init error, ret = %d", ret);
		goto end;
	}

	ret = asp_resource_init(codec);
	if (ret) {
		AUDIO_LOGE("asp resource init error, ret = %d", ret);
		goto asp_resource_init_err;
	}

	ret = hi6555_asp_resource_enable(codec);
	if (ret) {
		AUDIO_LOGE("asp resource enable error, ret = %d", ret);
		goto asp_resource_enable_err;
	}

	ret = pinctrl_init(codec);
	if (ret) {
		AUDIO_LOGE("pinctrl init error, ret = %d", ret);
		goto pinctrl_init_err;
	}

	classd_supply_init(codec);
	classh_clk_enable_init(codec);
	hac_gpio_init(codec);
	usb_ana_ldo_resource_init(codec);
	pm_runtime_init(codec);
	cfg_sif_driving_force(codec);
	reset_pmu_codec(codec);
	hi6555v3_asp_reg_init(codec);
	AUDIO_LOGI("resource init ok");

	goto end;

pinctrl_init_err:
	hi6555_asp_resource_disable(codec);

asp_resource_enable_err:
	asp_resource_deinit(codec);

asp_resource_init_err:
	pmu_resource_deinit(codec);

end:
	OUT_FUNCTION;

	return ret;
}

static void resource_deinit(struct snd_soc_codec *codec)
{
	pm_runtime_deinit(codec);
	hac_gpio_deinit(codec);
	classd_supply_deinit(codec);
	pinctrl_deinit(codec);
	hi6555_asp_resource_disable(codec);
	asp_resource_deinit(codec);
	pmu_resource_deinit(codec);

	AUDIO_LOGI("resource deinit ok");
}

static const char *name_list[] = {
	"audio",
	"ultr",
	"voice",
	"smpa",
	"third_codec",
	"btfm",
};

static void customized_data_init(struct hi6555v3_customize *data)
{
	int ret;

	codec_customize.mute_dacl_reg_val = INVALID_REG_VALUE;
	codec_customize.mute_dacr_reg_val = INVALID_REG_VALUE;
	codec_customize.classd_scharger_voltage = CLASSD_DEFAULT_VOLTAGE;
	codec_customize.supply_type = get_classd_supply_type(soc_codec);

	ret = snd_soc_add_codec_controls(soc_codec, hi6555v3_kcontrols, ARRAY_SIZE(hi6555v3_kcontrols));
	if (ret != 0)
		AUDIO_LOGE("add kcontrols failed, ret: %d", ret);
}

/* lint -e429 */
static int priv_data_init(struct snd_soc_codec *codec)
{
	struct platform_data *priv = NULL;
	unsigned int i;

	priv = snd_soc_codec_get_drvdata(codec);
	if (priv == NULL) {
		AUDIO_LOGE("priv is null");
		return -ENOMEM;
	}

	soc_codec = codec;
	priv->codec = codec;

	/* virtual codec register init */
	for (i = VIR0; i < VIR_CNT; i++)
		priv->virtual_reg[i] = 0;

	priv->pm.regulator_pu_count = 0;
	priv->ibias.set_ibias = set_ibias;
	spin_lock_init(&priv->reg_access_lock);
	mutex_init(&priv->ibias.ibias_mutex);

	priv->path_name = name_list;
	priv->path_name_num = ARRAY_SIZE(name_list);

	customized_data_init(&codec_customize);
	priv->customized_data = &codec_customize;

	AUDIO_LOGI("priv init ok");

	return 0;
}

/* lint +e429 */
static void priv_data_deinit(struct snd_soc_codec *codec)
{
	struct platform_data *priv = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION;

	if (priv != NULL) {
		mutex_destroy(&priv->ibias.ibias_mutex);
		snd_soc_codec_set_drvdata(codec, NULL);
	}

	soc_codec = NULL;

	AUDIO_LOGI("priv deinit ok");

	OUT_FUNCTION;
}

static int hi6555v3_codec_init(struct snd_soc_codec *codec)
{
	int ret;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	ret = priv_data_init(codec);
	if (ret) {
		AUDIO_LOGE("priv_data_init err, ret=%d", ret);
		return ret;
	}

	ret = resource_init(codec);
	if (ret) {
		AUDIO_LOGE("resource_init err, ret=%d", ret);
		goto resource_init_err;
	}

	ret = audio_codec_register_hisi_pa(codec);
	if (ret) {
		AUDIO_LOGE("register hisi pa error, ret = %d\n", ret);
		goto register_hisi_pa_err;
	}

	return 0;

register_hisi_pa_err:
	resource_deinit(codec);
resource_init_err:
	priv_data_deinit(codec);
	return ret;
}

static void hi6555v3_codec_deinit(struct snd_soc_codec *codec)
{
	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return;
	}

	resource_deinit(codec);
	priv_data_deinit(codec);
}

static void process_audio_path(struct platform_data *priv,
	u32 key, u32 val)
{
	switch (key) {
	case USB_ANA_LDO_CTL:
		if (val == STATE_ENABLE)
			usb_ana_ldo_ctl(priv, true);
		else if (val == STATE_DISABLE)
			usb_ana_ldo_ctl(priv, false);

		break;
	default:
		break;
	}

}

static void hi6555v3_codec_customized_func(struct platform_data *priv,
	u32 sub_module, u32 key, u32 val)
{
	switch (sub_module) {
	case HI6555_AUDIO_PATH:
		process_audio_path(priv, key, val);
		break;
	default:
		break;
	}
	AUDIO_LOGI("sub_module = %u, key = %u, val = %u\n",
		sub_module, key, val);
}

static struct hi6xxx_mbhc_ops hi6555v3_mbhc_ops = {
	hi6555v3_set_ibias_hsmicbias,
	hi6555v3_enable_hsd,
};

static struct codec_ops hi6555v3_codec_ops = {
	hi6555v3_codec_init,
	hi6555v3_codec_deinit,
	hi6555v3_shutdown,
	hi6555v3_asp_reg_init,
	&hi6555v3_mbhc_ops,
	hi6555v3_codec_customized_func,
};

static int __init hi6555v3_codec_data_init(void)
{
	hi6555_codec_driver_init(&hi6555v3_codec_ops);

	return 0;
}

arch_initcall(hi6555v3_codec_data_init);

