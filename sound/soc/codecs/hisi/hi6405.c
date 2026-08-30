/*
 * hi6405.c -- ALSA SoC hi6405 codec driver
 *
 * Copyright (c) 2018 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/init.h>
#include <linux/input.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/regulator/consumer.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_irq.h>
#include <linux/of_platform.h>
#include <linux/wakelock.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/tlv.h>
#include <sound/soc.h>
#include <sound/jack.h>
#include <asm/io.h>
#include <linux/gpio.h>
#include <linux/version.h>
#include <linux/hisi/hi64xx/hi64xx_compat.h>
#include <linux/hisi/hi64xx/hi64xx_utils.h>
#include <linux/hisi/hi64xx/hi64xx_resmgr.h>
#include <linux/hisi/hi64xx/hi64xx_vad.h>
#include <linux/hisi/hi64xx/hi64xx_mbhc.h>
#include <linux/hisi/hi64xx/hi6405_regs.h>
#include <linux/hisi/hi64xx/hi6405.h>
#include <linux/hisi/hi64xx_hifi_misc.h>
#include "slimbus.h"
#include "slimbus_6405.h"

#include "hi6405_hifi_config.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
#include "hi6405_debug.h"
#endif

#define LOG_TAG "hi6405"
#define MBHC_VOLTAGE_COEFFICIENT_MIN 1600
#define MBHC_VOLTAGE_COEFFICIENT_DEFAULT 2700
#define MBHC_VOLTAGE_COEFFICIENT_MAX 2800
#define DEFAULT_ADC_PGA_GAIN 0x78
#define DEFAULT_DACLR_MIXER_PGA_GAIN 0xFF
#define HI6405_MAX_VAL_ON_BIT(bit) ((1<<bit) - 1)
#define HI6405_MASK_ON_BIT(bit,offset) (((1<<bit) - 1)<<offset)
#define HI6405_IRQ_REQUEST_ERR_PRINT    HI6405_LOGE("hi64xx_irq_request_irq fail. err code is %x", ret);

#ifdef CONFIG_HAC_SUPPORT
/*hac status*/
#define HAC_ENABLE                   1
#define HAC_DISABLE                  0
#endif

#ifdef CONFIG_HISI_DIEID
#define CODEC_DIEID_BUF (60)
#define CODEC_DIEID_TEM_BUF (4)
#endif

#define PLL_DATA_ALL_NUM     128
#define PLL_DATA_GROUP_NUM   8
#define PLL_DATA_BUF_SIZE    10

enum track_state {
	TRACK_FREE = 0,
	TRACK_STARTUP = 1,
};

enum classh_state {
	hp_classh_state  = 0x1,  /* hp high mode(classAB) = 0 low mode(classh) = 1 */
	rcv_classh_state = 0x2,  /* classh_rcv_hp_switch true = 1 false =0 */
	hp_power_state   = 0x4,  /* hp power on = 1 power off = 0 */
	rcv_power_state  = 0x8,  /* rcv power on = 1 power off = 0 */
};

enum HI6405_SAMPLE_RATE {
	SAMPLE_RATE_8K = 0,
	SAMPLE_RATE_16K = 1,
	SAMPLE_RATE_32K = 2,
	SAMPLE_RATE_48K = 4,
	SAMPLE_RATE_96K = 5,
	SAMPLE_RATE_192K = 6,
};

enum HI6405_SRC_MODE {
	SRC_MODE_1_5 = 4,
	SRC_MODE_2 = 3,
	SRC_MODE_3 = 0,
	SRC_MODE_6 = 2,
	SRC_MODE_12 = 1,
};

enum hi6405_irq_type {
	HI6405_IRQ_BUNK1_OCP = 40,
	HI6405_IRQ_BUNK1_SCP = 41,
	HI6405_IRQ_CP1_SHORT = 42,
	HI6405_IRQ_CP2_SHORT = 43,
	HI6405_IRQ_LDO_AVDD18_OCP = 44,
	HI6405_IRQ_LDOP_OCP = 45,
	HI6405_IRQ_LDON_OCP = 46,
};

/*
* HSMIC PGA GAIN volume control:
* from 0 to 30 dB in 2 dB steps
* MAX VALUE is 15
*/
static DECLARE_TLV_DB_SCALE(hsmic_pga_tlv, 0, 200, 0);

/*
* AUX PGA GAIN volume control:
* from 0 to 30 dB in 2 dB steps
* MAX VALUE is 15
*/
static DECLARE_TLV_DB_SCALE(auxmic_pga_tlv, 0, 200, 0);

/*
* MIC3 PGA GAIN volume control:
* from 0 to 30 dB in 2 dB steps
* MAX VALUE is 15
*/
static DECLARE_TLV_DB_SCALE(mic3_pga_tlv, 0, 200, 0);

/*
* MIC4 MIC GAIN volume control:
* from 0 to 30 dB in 2 dB steps
* MAX VALUE is 15
*/
static DECLARE_TLV_DB_SCALE(mic4_pga_tlv, 0, 200, 0);

/*
* MIC5 PGA GAIN volume control:
* from 0 to 30 dB in 2 dB steps
* MAX VALUE is 15
*/
static DECLARE_TLV_DB_SCALE(mic5_pga_tlv, 0, 200, 0);

/*
* MAD PGA GAIN volume control:
* from 0 to 24 dB in 2 dB steps
* MAX VALUE is 12
*/

static DECLARE_TLV_DB_SCALE(mad_pga_tlv, 0, 200, 0);

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
static struct hicodec_dump_reg_entry hi6405_dump_table[] = {
	{"PAGE IO", HI6405_DBG_PAGE_IO_CODEC_START, HI6405_DBG_PAGE_IO_CODEC_END, 4},
	{"PAGE CFG", HI6405_DBG_PAGE_CFG_CODEC_START, HI6405_DBG_PAGE_CFG_CODEC_END, 1},
	{"PAGE ANA", HI6405_DBG_PAGE_ANA_CODEC_START, HI6405_DBG_PAGE_ANA_CODEC_END, 1},
	{"PAGE DIG", HI6405_DBG_PAGE_DIG_CODEC_START, HI6405_DBG_PAGE_DIG_CODEC_END, 1},
};

static struct hicodec_dump_reg_info hi6405_dump_info = {
	.entry = hi6405_dump_table,
	.count = sizeof(hi6405_dump_table) / sizeof(struct hicodec_dump_reg_entry),
};
#endif

struct hi6405_board_cfg {
	int mic_num;
	bool classh_rcv_hp_switch;
#ifdef CONFIG_HAC_SUPPORT
	int hac_gpio;
#endif
	bool wakeup_hisi_algo_support;
};

struct hi6405_platform_data {
	struct snd_soc_codec *codec;
	struct device_node *node;
	struct hi_cdc_ctrl *cdc_ctrl;
	struct hi64xx_resmgr *resmgr;
	struct hi64xx_irq *irqmgr;
	struct hi64xx_mbhc *mbhc;
	struct hi64xx_mbhc_config mbhc_config;
	struct hi6405_board_cfg board_config;
	spinlock_t v_rw_lock;
	struct mutex impdet_dapm_mutex;
	unsigned int virtual_reg[HI6405_VIR_CNT];
	bool hsl_power_on;
	bool hsr_power_on;
	int dp_clk_num;
	int cp1_num;
	int cp2_num;
	slimbus_track_param_t voice_up_params;
	slimbus_track_param_t voice_down_params;
	slimbus_track_param_t play_params;
	slimbus_track_param_t capture_params;
	slimbus_track_param_t pa_iv_params;
	slimbus_track_param_t soundtrigger_params;
	enum track_state voiceup_state;
	enum track_state audioup_4mic_state;
	enum classh_state rcv_hp_classh_state;
};

static const struct of_device_id hi6405_platform_match[] = {
	{ .compatible = "hisilicon,hi6405-codec", },
	{},
};

struct snd_soc_codec *hi6405_codec = NULL;

static int hi6405_slimbus_param_pass(struct snd_soc_codec *codec, slimbus_track_type_t track,
							slimbus_track_param_t *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (params == NULL) {
			snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x44);
		} else {
			if (params->rate == SLIMBUS_SAMPLE_RATE_96K) {
				snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x55);
			} else if (params->rate == SLIMBUS_SAMPLE_RATE_192K) {
				snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x66);
			} else if (params->rate == SLIMBUS_SAMPLE_RATE_384K) {
				snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x77);
			} else {
				snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x44);
			}
		}

		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, track, params);
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, track, params);
		snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x44);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		ret = -1;
		break;
	}

	return ret;
}

static int hi6405_pll_param_pass(struct snd_soc_dapm_widget *w,
						enum hi64xx_pll_type pll_type, int event)
{
	struct hi6405_platform_data *platform_data = NULL;
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(NULL == codec);

	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_pll(platform_data->resmgr, pll_type);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_resmgr_release_pll(platform_data->resmgr, pll_type);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_pll_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == priv);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_pll(priv->resmgr, PLL_HIGH);
		if (!priv->cdc_ctrl->pm_runtime_support) {
			/* slimbus master framer is hi6405 */
			mdelay(2);
			slimbus_switch_framer(SLIMBUS_DEVICE_HI6405, SLIMBUS_FRAMER_CODEC);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (!priv->cdc_ctrl->pm_runtime_support) {
			/* slimbus master framer is soc */
			slimbus_switch_framer(SLIMBUS_DEVICE_HI6405, SLIMBUS_FRAMER_SOC);
			mdelay(2);
		}
		hi64xx_resmgr_release_pll(priv->resmgr, PLL_HIGH);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_44k1_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	IN_FUNCTION
	ret = hi6405_pll_param_pass(w, PLL_44_1, event);
	OUT_FUNCTION
	return ret;
}

static int hi6405_mad_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	int ret;

	IN_FUNCTION
	ret = hi6405_pll_param_pass(w, PLL_LOW, event);
	OUT_FUNCTION
	return ret;
}

static void hi6405_request_dp_clk(struct snd_soc_codec *codec, bool enable)
{
	struct hi6405_platform_data *platform_data = NULL;

	IN_FUNCTION
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	mutex_lock(&platform_data->impdet_dapm_mutex);
	if (enable) {
		platform_data->dp_clk_num++;
		if (1 == platform_data->dp_clk_num) {
			hi64xx_update_bits(codec, HI6405_CODEC_DP_CLKEN_REG,
						0x1<<HI6405_CODEC_DP_CLKEN_OFFSET, 0x1<<HI6405_CODEC_DP_CLKEN_OFFSET);
		}
	} else {
		if (0 == platform_data->dp_clk_num) {
			HI6405_LOGE("dp clk num is 0 when disable");
			mutex_unlock(&platform_data->impdet_dapm_mutex);
			return;
		}
		platform_data->dp_clk_num--;
		if (0 == platform_data->dp_clk_num) {
			hi64xx_update_bits(codec, HI6405_CODEC_DP_CLKEN_REG,
						0x1<<HI6405_CODEC_DP_CLKEN_OFFSET, 0);
		}
	}
	mutex_unlock(&platform_data->impdet_dapm_mutex);
	OUT_FUNCTION
	return;
}

static int hi6405_dp_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_request_dp_clk(codec, true);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6405_request_dp_clk(codec, false);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_s1_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S1_CLK_IF_EN_OFFSET, 1<<HI6405_S1_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S1_CLK_IF_TXRX_EN_OFFSET, 1<<HI6405_S1_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S1_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S1_CLK_IF_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_s2_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S2_CLK_IF_EN_OFFSET, 1<<HI6405_S2_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET, 1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S2_CLK_IF_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_s4_if_clk_supply_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S4_CLK_IF_EN_OFFSET, 1<<HI6405_S4_CLK_IF_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S4_CLK_IF_TXRX_EN_OFFSET, 1<<HI6405_S4_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S4_CLK_IF_TXRX_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG,
				1<<HI6405_S4_CLK_IF_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static void hi6405_request_cp1(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(!codec);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!data);

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp1_num++;
	if (1 == data->cp1_num) {
		/* buck1 enable */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				1<<HI6405_BUCK1_ENP_BIT, 1<<HI6405_BUCK1_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);
	OUT_FUNCTION
}

static void hi6405_release_cp1(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(!codec);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!data);

	mutex_lock(&data->impdet_dapm_mutex);
	if (0 == data->cp1_num) {
		HI6405_LOGE("cp1 num is 0 when release\n");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp1_num--;
	if (0 == data->cp1_num) {
		/* buck1 disable */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				1<<HI6405_BUCK1_ENP_BIT, 0);
	}
	mutex_unlock(&data->impdet_dapm_mutex);
	OUT_FUNCTION
}

static void hi6405_request_cp2(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(!codec);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!data);

	mutex_lock(&data->impdet_dapm_mutex);
	data->cp2_num++;
	if (1 == data->cp2_num) {
		/* buck2 enable */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				1<<HI6405_CP2_ENP_BIT, 1<<HI6405_CP2_ENP_BIT);
		mdelay(5);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				3<<HI6405_LDON_ENP_BIT, 3<<HI6405_LDON_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);
	OUT_FUNCTION
	return;
}

static void hi6405_release_cp2(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(!codec);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!data);

	mutex_lock(&data->impdet_dapm_mutex);
	if (0 == data->cp2_num) {
		HI6405_LOGE("cp2 num is 0 when release!\n");
		mutex_unlock(&data->impdet_dapm_mutex);
		return;
	}
	data->cp2_num--;
	if (0 == data->cp2_num) {
		/* buck2 disable */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				3<<HI6405_LDON_ENP_BIT, 0<<HI6405_LDON_ENP_BIT);
		mdelay(5);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_104,
				1<<HI6405_CP2_ENP_BIT, 0<<HI6405_CP2_ENP_BIT);
	}
	mutex_unlock(&data->impdet_dapm_mutex);
	OUT_FUNCTION
	return;
}

static int hi6405_cp1_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x\n", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_request_cp1(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6405_release_cp1(codec);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_cp2_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	HI6405_LOGI("power event: 0x%x\n", event);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_request_cp2(codec);
		msleep(5);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6405_release_cp2(codec);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

#ifdef CONFIG_HAC_SUPPORT
static const char * const hac_switch_text[] = {"OFF", "ON"};

static const struct soc_enum hac_switch_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(hac_switch_text), hac_switch_text),
};

static int hi6405_hac_status_get(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (NULL == kcontrol || NULL == ucontrol){
		HI6405_LOGE("input pointer is null\n");
		return -1;
	}
	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		HI6405_LOGE("hac gpio = %d is invalid\n",priv->board_config.hac_gpio);
		return -1;
	}
	ret = gpio_get_value(priv->board_config.hac_gpio);
	HI6405_LOGI("hac gpio = %d, value = %d\n", priv->board_config.hac_gpio, ret);
	ucontrol->value.integer.value[0] = ret;
	return 0;
}

static int hi6405_hac_status_set(struct snd_kcontrol *kcontrol,
					struct snd_ctl_elem_value *ucontrol)
{
	int ret = 0;
	struct hi6405_platform_data *priv = NULL;
	struct snd_soc_codec *codec = NULL;

	if (NULL == kcontrol || NULL == ucontrol) {
		HI6405_LOGE("input pointer is null\n");
		return -1;
	}
	codec = snd_soc_kcontrol_codec(kcontrol);
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	if (!gpio_is_valid(priv->board_config.hac_gpio)) {
		HI6405_LOGE("hac gpio = %d is invalid\n", priv->board_config.hac_gpio);
		return -1;
	}
	ret = ucontrol->value.integer.value[0];
	if (HAC_ENABLE == ret) {
		gpio_set_value(priv->board_config.hac_gpio, HAC_ENABLE);
	} else {
		gpio_set_value(priv->board_config.hac_gpio, HAC_DISABLE);
	}
	return ret;
}

int hi6405_hac_gpio_init(int hac_gpio)
{
	if (!gpio_is_valid(hac_gpio)) {
		HI6405_LOGE("hac Value is not valid.\n");
		return -1;
	}
	if (gpio_request(hac_gpio, "hac_en_gpio")) {
		HI6405_LOGE("hac gpio request failed!\n");
		return -1;
	}
	if (gpio_direction_output(hac_gpio, 0)) {
		HI6405_LOGE("hac gpio set output failed!\n");
		return -1;
	}
	return 0;
}
#endif

static int hi6405_s2_rx_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(!codec);
	IN_FUNCTION

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG,
				1<<HI6405_S2_IF_RX_ENA_OFFSET, 1<<HI6405_S2_IF_RX_ENA_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG,
				1<<HI6405_S2_IF_RX_ENA_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_s4_rx_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(!codec);
	IN_FUNCTION

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_L_REG,
				1<<HI6405_S4_IF_RX_ENA_OFFSET, 1<<HI6405_S4_IF_RX_ENA_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_L_REG,
				1<<HI6405_S4_IF_RX_ENA_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static void hi6405_classH_headphone_select(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	/* classH clk en */
	hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			0x1<<HI6405_CLK_CLASSH_EN_OFFSET, 0x1<<HI6405_CLK_CLASSH_EN_OFFSET);
	/* classA/B -> classH */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL6_REG,
			0x1<<HI6405_CLASSH_EN_OFFSET, 0x1<<HI6405_CLASSH_EN_OFFSET);

	/* classh_vth_ng_cfg=72dB */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_VTH_NG_CFG_LEN, HI6405_CLASSH_VTH_NG_CFG_OFFSET),
			HI6405_CLASSH_VTH_NG_CFG_72DB);
	/* dacl/r_up16_din_sel=classh */
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL33_REG,
			0x1<<HI6405_DACL_UP16_DIN_SEL_OFFSET |
			0x1<<HI6405_DACR_UP16_DIN_SEL_OFFSET,
			0x1<<HI6405_DACL_UP16_DIN_SEL_OFFSET |
			0x1<<HI6405_DACR_UP16_DIN_SEL_OFFSET);
	/* classh_hp_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL1_REG,
			0x1<<HI6405_CLASSH_HP_EN_OFFSET, 0x1<<HI6405_CLASSH_HP_EN_OFFSET);
	/* classh_ng_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL8_REG,
			0x1<<HI6405_CLASSH_NG_EN_OFFSET, 0x1<<HI6405_CLASSH_NG_EN_OFFSET);
	/* dacl/r_up16_din_sel=classh */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL8_REG,
			0x1<<HI6405_CLASSH_NG_EN_OFFSET, 0x1<<HI6405_CLASSH_NG_EN_OFFSET);
	/* cp_pop_clk_pd */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140,
			0x1<<HI6405_CODEC_ANA_CTCM_CHOP_BPS_OFFSET, 0x1<<HI6405_CODEC_ANA_CTCM_CHOP_BPS_OFFSET);
	/* rst_clk_calib_pop_ng_lp */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_056,
			0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET, 0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_056,
			0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET, 0);

	OUT_FUNCTION
}

static void hi6405_classAB_headphone_select(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	/* classH clk must en */
	hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			0x1<<HI6405_CLK_CLASSH_EN_OFFSET, 0x1<<HI6405_CLK_CLASSH_EN_OFFSET);
	/* classH -> classA/B */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL6_REG,
			0x1<<HI6405_CLASSH_EN_OFFSET, 0x0<<HI6405_CLASSH_EN_OFFSET);

	/* classh_vth_ng_cfg=72dB */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_VTH_NG_CFG_LEN, HI6405_CLASSH_VTH_NG_CFG_OFFSET),
			HI6405_CLASSH_VTH_NG_CFG_72DB);
	/* dacl/r_up16_din_sel=pga */
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL33_REG,
			0x1<<HI6405_DACL_UP16_DIN_SEL_OFFSET |
			0x1<<HI6405_DACR_UP16_DIN_SEL_OFFSET,
			0);
	/* classh_hp_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL1_REG,
			0x1<<HI6405_CLASSH_HP_EN_OFFSET, 0x0<<HI6405_CLASSH_HP_EN_OFFSET);
	/* classh_ng_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL8_REG,
			0x1<<HI6405_CLASSH_NG_EN_OFFSET, 0x1<<HI6405_CLASSH_NG_EN_OFFSET);

	/* cp_pop_clk_pd */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140,
			0x1<<HI6405_CODEC_ANA_CTCM_CHOP_BPS_OFFSET, 0x1<<HI6405_CODEC_ANA_CTCM_CHOP_BPS_OFFSET);
	/* rst_clk_calib_pop_ng_lp */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_056,
			0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET, 0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_056,
			0x1<<HI6405_CODEC_ANA_RST_CLK_POP_OFFSET, 0);

	OUT_FUNCTION
}

static void hi6405_classH_earpiece_select(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	/* classH clk en */
	hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			0x1<<HI6405_CLK_CLASSH_EN_OFFSET, 0x1<<HI6405_CLK_CLASSH_EN_OFFSET);

	/* classh_ep_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL1_REG,
			0x1<<HI6405_CLASSH_EP_EN_OFFSET, 0x1<<HI6405_CLASSH_EP_EN_OFFSET);

	/* classA/B -> classH */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL6_REG,
			0x1<<HI6405_CLASSH_EN_OFFSET, 0x1<<HI6405_CLASSH_EN_OFFSET);

	/* dacls_up16_din_sel classh */
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL33_REG,
			0x1<<HI6405_DACL_S_UP16_DIN_SEL_OFFSET,
			0x1<<HI6405_DACL_S_UP16_DIN_SEL_OFFSET);

	/* fall delay counter=16384 */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_VTH_NG_CFG_LEN, HI6405_CLASSH_VTH_NG_CFG_OFFSET),
			0);
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_DF_CFG_LEN, HI6405_CLASSH_DF_CFG_OFFSET),
			HI6405_CLASSH_DF_CFG_16384);

	OUT_FUNCTION
}

static void hi6405_classAB_earpiece_select(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	/* classH clk en */
	hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			0x1<<HI6405_CLK_CLASSH_EN_OFFSET, 0x1<<HI6405_CLK_CLASSH_EN_OFFSET);

	/* classh_ep_en */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL1_REG,
			0x1<<HI6405_CLASSH_EP_EN_OFFSET, 0x1<<HI6405_CLASSH_EP_EN_OFFSET);

	/* classH -> classAB */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL6_REG,
			0x1<<HI6405_CLASSH_EN_OFFSET, 0);
	/* dacls_up16_din_sel classh */
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL33_REG,
			0x1<<HI6405_DACL_S_UP16_DIN_SEL_OFFSET,
			0);

	/* fall delay counter=16384 */
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_VTH_NG_CFG_LEN, HI6405_CLASSH_VTH_NG_CFG_OFFSET),
			0);
	hi64xx_update_bits(codec, HI6405_CLASSH_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_CLASSH_DF_CFG_LEN, HI6405_CLASSH_DF_CFG_OFFSET),
			HI6405_CLASSH_DF_CFG_16384);

	OUT_FUNCTION
}

static void hi6405_set_classh_config(struct snd_soc_codec *codec, enum classh_state classh_state_cfg)
{
	unsigned char state = classh_state_cfg & 0x0F;

	HI6405_LOGI("ClassH state is %d", state);

	if (state & hp_power_state) {
		if (state & hp_classh_state)
			hi6405_classH_headphone_select(codec);
		else
			hi6405_classAB_headphone_select(codec);
	}

	if (state & rcv_power_state) {
		if (state & rcv_classh_state)
			hi6405_classH_earpiece_select(codec);
		else
			hi6405_classAB_earpiece_select(codec);
	}
}

static void hi6405_headphone_pop_on(struct snd_soc_codec *codec)
{
	IN_FUNCTION

	/* hp sdm clk dis */
	hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_HP_SDM_L_CLK_EN_OFFSET | 1<<HI6405_HP_SDM_R_CLK_EN_OFFSET,
			0);

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x7A);/* pop ramp */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_141, 0x32);/* tx_hp */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140, 0xC0, 0x80); /* chop dis and cp_pop */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_056, 0x4A);/* tx_hp default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_056, 0xA);/* tx_hp pop work */

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x7A);/* tx_hp default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x7A);/* tx_hp default */

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_056, 0xA);/* tx_hp pop work */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_055, 0xF);/* TX_HP VBR_Gain:0db */

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_011, 0x0);/* CPGND */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_057, 0x1);/* tx_hp hl default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_057, 0x1);/* tx_hp hl default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0x10);/* TX_HP_offset default*/
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0x18);/* TX_HP_offset 768k*/
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0x18);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0x8);/* OFFSET calibration work */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0xE);
	mdelay(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_058, 0x8);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x7F);/* TX_HPL default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x3F);/* TX_HPR */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x5F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x1F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_066, 0x89);/* pop default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x7A);/* pop ramp default */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_066, 0x89);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_066, 0x9);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_066, 0x1);

	/* sequence and delay time can not modify for pop */
	mdelay(20);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x4B);
	mdelay(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x4F);
	mdelay(20);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x43);
	usleep_range(200,210);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0xB);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0xF);
	mdelay(20);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x3);
	mdelay(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x1A);

	/* hp sdm clk en */
	hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_HP_SDM_L_CLK_EN_OFFSET | 1<<HI6405_HP_SDM_R_CLK_EN_OFFSET,
			1<<HI6405_HP_SDM_L_CLK_EN_OFFSET | 1<<HI6405_HP_SDM_R_CLK_EN_OFFSET);

	OUT_FUNCTION
}

static void hi6405_headphone_pop_off(struct snd_soc_codec *codec)
{
	IN_FUNCTION

	/* hp sdm clk dis */
	hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_HP_SDM_L_CLK_EN_OFFSET | 1<<HI6405_HP_SDM_R_CLK_EN_OFFSET,
			0);

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_067, 0x7A);

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x4B);
	usleep_range(100, 110);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x4F);
	mdelay(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x5F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x0B);
	usleep_range(100, 110);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x0F);
	mdelay(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x1F);
	usleep_range(100, 110);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_09, 0x7F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x3F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_010, 0x7F);

	OUT_FUNCTION
}

static void hi6405_headphone_power_param_pass(struct hi6405_platform_data *data,
					struct snd_soc_codec *codec, bool *hs_pwron_flag, int event)
{
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		*hs_pwron_flag = true;
		if (data->hsl_power_on && data->hsr_power_on) {
			data->rcv_hp_classh_state |= hp_power_state;
			hi6405_set_classh_config(codec, data->rcv_hp_classh_state);
			hi6405_headphone_pop_on(codec);
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (data->hsl_power_on && data->hsr_power_on) {
			hi6405_headphone_pop_off(codec);
			data->rcv_hp_classh_state &= ~hp_power_state;
			hi6405_set_classh_config(codec, data->rcv_hp_classh_state);
		}
		*hs_pwron_flag = false;
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return;
}

static int hi6405_headphone_l_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(codec == NULL);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(data == NULL);

	hi6405_headphone_power_param_pass(data, codec, &data->hsl_power_on, event);

	OUT_FUNCTION
	return 0;
}

static int hi6405_headphone_r_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *data = NULL;

	IN_FUNCTION
	WARN_ON(codec == NULL);
	data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(data == NULL);

	hi6405_headphone_power_param_pass(data, codec, &data->hsr_power_on, event);

	OUT_FUNCTION
	return 0;
}

static int hi6405_ir_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_LRN_OFFSET, 0x1<<HI6405_CODEC_PD_INF_LRN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_EMS_OFFSET, 0<<HI6405_CODEC_PD_INF_EMS_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_051,
			HI6405_MASK_ON_BIT(HI6405_CODEC_FIR_OUT_CTRL_LEN, HI6405_CODEC_FIR_OUT_CTRL_OFFSET)
			| HI6405_MASK_ON_BIT(HI6405_CODEC_FIR_ATE_CTRL_LEN, HI6405_CODEC_FIR_ATE_CTRL_OFFSET),
			HI6405_CODEC_FIR_OUT_X15<<HI6405_CODEC_FIR_OUT_CTRL_OFFSET
			| HI6405_CODEC_FIR_ATE_XF<<HI6405_CODEC_FIR_ATE_CTRL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_051,
			HI6405_MASK_ON_BIT(HI6405_CODEC_FIR_OUT_CTRL_LEN, HI6405_CODEC_FIR_OUT_CTRL_OFFSET)
			| HI6405_MASK_ON_BIT(HI6405_CODEC_FIR_ATE_CTRL_LEN, HI6405_CODEC_FIR_ATE_CTRL_OFFSET),
			HI6405_CODEC_FIR_OUT_NON<<HI6405_CODEC_FIR_OUT_CTRL_OFFSET
			| HI6405_CODEC_FIR_ATE_X1<<HI6405_CODEC_FIR_ATE_CTRL_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_EMS_OFFSET, 0x1<<HI6405_CODEC_PD_INF_EMS_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	OUT_FUNCTION
	return ret;
}

static int hi6405_s2_tx_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1<<HI6405_S2_OL_PGA_CLK_EN_OFFSET | 1<<HI6405_S2_OR_PGA_CLK_EN_OFFSET,
			1<<HI6405_S2_OL_PGA_CLK_EN_OFFSET | 1<<HI6405_S2_OR_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1<<HI6405_S2_OL_PGA_CLK_EN_OFFSET | 1<<HI6405_S2_OR_PGA_CLK_EN_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	OUT_FUNCTION
	return ret;
}

static int hi6405_earpiece_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	struct hi6405_platform_data *priv = NULL;

	IN_FUNCTION
	WARN_ON(!codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		priv->rcv_hp_classh_state |= rcv_power_state;
		hi6405_set_classh_config(codec, priv->rcv_hp_classh_state);

		/* earpiece dac on */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPDAC_OFFSET, 0);
		/* earpiece pga on */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPPGA_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPINT_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPVB_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_RSUB_EP_OFFSET, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* earpiece pga off */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_RSUB_EP_OFFSET, 1<<HI6405_CODEC_ANA_PD_RSUB_EP_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPVB_OFFSET, 1<<HI6405_CODEC_ANA_PD_EPVB_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPINT_OFFSET, 1<<HI6405_CODEC_ANA_PD_EPINT_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPPGA_OFFSET, 1<<HI6405_CODEC_ANA_PD_EPPGA_OFFSET);
		/* earpiece dac pb */
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_014, 1<<HI6405_CODEC_ANA_PD_EPDAC_OFFSET, 1<<HI6405_CODEC_ANA_PD_EPDAC_OFFSET);

		priv->rcv_hp_classh_state &= ~rcv_power_state;
		hi6405_set_classh_config(codec, priv->rcv_hp_classh_state);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_play48k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;
	IN_FUNCTION

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_IR_SRC_CLK_EN_OFFSET,
			1<<HI6405_S1_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_IR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL38_REG,
			1<<HI6405_DACSL_MIXER4_DVLD_SEL_OFFSET,
			1<<HI6405_DACSL_MIXER4_DVLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL38_REG,
			1<<HI6405_DACSL_MIXER4_DVLD_SEL_OFFSET,
			0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET  | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0);
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_IR_SRC_CLK_EN_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return ret;
}

static int hi6405_play96k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;
	IN_FUNCTION

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET  | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	OUT_FUNCTION
	return ret;
}

static int hi6405_play192k_path_config(struct snd_soc_codec *codec, int event)
{
	int ret = 0;
	IN_FUNCTION

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL34_REG,
			1<<HI6405_DACL_POST_MIXER2_DVLD_SEL_OFFSET | 1<<HI6405_DACR_POST_MIXER2_DVLD_SEL_OFFSET,
			1<<HI6405_DACL_POST_MIXER2_DVLD_SEL_OFFSET | 1<<HI6405_DACR_POST_MIXER2_DVLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL34_REG,
			1<<HI6405_DACL_POST_MIXER2_DVLD_SEL_OFFSET | 1<<HI6405_DACR_POST_MIXER2_DVLD_SEL_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return ret;
}

static int hi6405_play44k1_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	//priv->play_params.rate = SLIMBUS_SAMPLE_RATE_44100;

	ret = hi6405_slimbus_param_pass(codec, SLIMBUS_6405_TRACK_AUDIO_PLAY, &priv->play_params, event);
	if (ret){
		HI6405_LOGW("slimbus fail!");
		return ret;
	}
	ret = hi6405_play48k_path_config(codec, event);

	OUT_FUNCTION
	return ret;
}

static int hi6405_play48k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	ret = hi6405_slimbus_param_pass(codec, SLIMBUS_6405_TRACK_AUDIO_PLAY, NULL, event);

	if (ret){
		HI6405_LOGW("slimbus fail!");
		return ret;
	}
	ret = hi6405_play48k_path_config(codec, event);

	OUT_FUNCTION
	return ret;
}

static int hi6405_play96k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_96K;

	ret = hi6405_slimbus_param_pass(codec, SLIMBUS_6405_TRACK_DIRECT_PLAY, &priv->play_params, event);
	if (ret){
		HI6405_LOGW("slimbus fail!");
		return ret;
	}
	ret = hi6405_play96k_path_config(codec, event);

	OUT_FUNCTION
	return ret;
}

static int hi6405_play192k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_192K;

	ret = hi6405_slimbus_param_pass(codec, SLIMBUS_6405_TRACK_DIRECT_PLAY, &priv->play_params, event);
	if (ret){
		HI6405_LOGW("slimbus fail!");
		return ret;
	}
	hi6405_play192k_path_config(codec, event);

	OUT_FUNCTION
	return ret;
}

static int hi6405_play384k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_384K;

	ret = hi6405_slimbus_param_pass(codec, SLIMBUS_6405_TRACK_DIRECT_PLAY, &priv->play_params, event);

	OUT_FUNCTION
	return ret;
}

static int hi6405_hp_high_level_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;

	IN_FUNCTION
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == priv);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* hpdac high performance */
		priv->rcv_hp_classh_state &= ~hp_classh_state;
		hi6405_set_classh_config(codec, priv->rcv_hp_classh_state);
		break;

	case SND_SOC_DAPM_POST_PMD:
		/* hpdac lower power */
		priv->rcv_hp_classh_state |= hp_classh_state;
		hi6405_set_classh_config(codec, priv->rcv_hp_classh_state);
		break;
	default:
		HI6405_LOGW("power event err:%d\n", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_hp_concurrency_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;

	IN_FUNCTION
	WARN_ON(codec == NULL);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(priv == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1 << HI6405_S2_IL_SRC_CLK_EN_OFFSET |
			1 << HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			1 << HI6405_S2_IL_SRC_CLK_EN_OFFSET |
			1 << HI6405_S2_IR_SRC_CLK_EN_OFFSET);
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret)
			HI6405_LOGE("slimbus track activate 4pa iv err\n");
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_PLAY_D3D4, &priv->play_params);
		if (ret)
			HI6405_LOGE("slimbus track activate 4pa iv err\n");
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1 << HI6405_S2_IL_SRC_CLK_EN_OFFSET |
			1 << HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			0x0);
		break;
	default:
		HI6405_LOGW("power event err:%d\n", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_pll48k_test_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION
	hi6405_pll_param_pass(w, PLL_HIGH, event);
	OUT_FUNCTION
	return 0;
}

static int hi6405_pll44k1_test_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION
	hi6405_pll_param_pass(w, PLL_44_1, event);
	OUT_FUNCTION
	return 0;
}

static int hi6405_pll32k_test_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	IN_FUNCTION
	hi6405_pll_param_pass(w, PLL_LOW, event);
	OUT_FUNCTION
	return 0;
}


static int hi6405_s2up_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	IN_FUNCTION
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			  1<<HI6405_S2_OL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_OR_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			  1<<HI6405_S2_OL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_OR_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			  1<<HI6405_S2_OL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_OR_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET
			| 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_iv_dspif_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* on bit match */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET,
			0);

		/* src 96k-48k */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL8_REG,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET);

		/* 32bit */
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0xff);

		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SPA_OL_SRC_DIN_SEL_LEN, HI6405_SPA_OL_SRC_DIN_SEL_OFFSET) |
			HI6405_MASK_ON_BIT(HI6405_SPA_OR_SRC_DIN_SEL_LEN, HI6405_SPA_OR_SRC_DIN_SEL_OFFSET),
			1<<HI6405_SPA_OL_SRC_DIN_SEL_OFFSET |
			1<<HI6405_SPA_OR_SRC_DIN_SEL_OFFSET);

		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x0);

		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET,
			0x0);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL8_REG,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET,
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET,
			1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_iv_2pa_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	struct hi6405_platform_data *priv = NULL;
	int ret = 0;
	IN_FUNCTION
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);
	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 2;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* on bit match */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			0x1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET,
			0x0);
		/* src 96k-48k */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL8_REG,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET);

		/* 32bit */
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x3<<HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET);

		/* sel iv */
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL37_REG, 0xA5);

		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL37_REG, 0x5);

		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0);

		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			1<<HI6405_S4_OL_SRC_CLK_EN_OFFSET |
			1<<HI6405_S4_OR_SRC_CLK_EN_OFFSET,
			0x0);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL8_REG,
			1<<HI6405_S4_OL_SRC_DIN_VLD_SEL_OFFSET |
			1<<HI6405_S4_OR_SRC_DIN_VLD_SEL_OFFSET,
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			0x1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET,
			0x1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET);

		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_fm_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL9_REG,
			1<<HI6405_DAC_MIXER4_VLD_SEL_OFFSET  | 1<<HI6405_DAC_PRE_MIXER2_VLD_SEL_OFFSET,
			0);
		hi64xx_update_bits(codec, HI6405_S2_DP_CLK_EN_REG,
			1<<HI6405_S2_IL_SRC_CLK_EN_OFFSET | 1<<HI6405_S2_IR_SRC_CLK_EN_OFFSET,
			0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}
static int hi6405_dacl_post_mixer_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* filter */
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACL_EN_OFFSET,
			1<<HI6405_CLK_DACL_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACL_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacr_post_mixer_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* filter */
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACR_EN_OFFSET,
			1<<HI6405_CLK_DACR_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACR_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacsl_mixer_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* filter clk*/
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACSL_EN_OFFSET,
			1<<HI6405_CLK_DACSL_EN_OFFSET);
		/* hbf1i_bypass_en */
		hi64xx_update_bits(codec, HI6405_DACSL_CTRL_REG, 1<<HI6405_DACSL_HBF1I_BYPASS_EN_OFFSET,
			1<<HI6405_DACSL_HBF1I_BYPASS_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DACSL_CTRL_REG, 1<<HI6405_DACSL_HBF1I_BYPASS_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG, 1<<HI6405_CLK_DACSL_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacsr_mixer_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		break;
	case SND_SOC_DAPM_POST_PMD:
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacl_pga_mux_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* pga */
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACL_PRE_PGA_CLK_EN_OFFSET,
			1<<HI6405_DACL_PRE_PGA_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACL_POST_PGA_CLK_EN_OFFSET,
			1<<HI6405_DACL_POST_PGA_CLK_EN_OFFSET);

		/* src */
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG, 1<<HI6405_CLK_DACL_UP16_EN_OFFSET,
			1<<HI6405_CLK_DACL_UP16_EN_OFFSET);
		/* sdm valid */
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL0_REG, 1<<HI6405_HP_SDM_L_DEM_DSEN_OFFSET,
			1<<HI6405_HP_SDM_L_DEM_DSEN_OFFSET);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL7_REG, HI6405_HP_SDM_L_UP16_DELAY_MASK,
			HI6405_HP_SDM_L_UP16_DELAY_VAULE);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL8_REG, HI6405_HP_SDM_L_DELAY_MASK,
			HI6405_HP_SDM_L_DELAY_VALUE);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_HP_SDM_L_CALT_VLD_LEN, HI6405_HP_SDM_L_CALT_VLD_OFFSET),
			HI6405_HP_SDM_L_CELLSEL_MODE_2<<HI6405_HP_SDM_L_CELLSEL_MODE_OFFSET);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL12_REG, 1<<HI6405_HP_SDM_L_CALT_VLD_OFFSET,
			1<<HI6405_HP_SDM_L_CALT_VLD_OFFSET);
		/* dacl dc */
		hi64xx_update_bits(codec, HI6405_DACL_DC_M_REG, HI6405_DACL_DC_M_REG_MASK,
			HI6405_DACL_DC_M_REG_VALUE);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* dacl dc */
		hi64xx_update_bits(codec, HI6405_DACL_DC_M_REG, HI6405_DACL_DC_M_REG_MASK, 0);
		/* sdm default */
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL12_REG, 1<<HI6405_HP_SDM_L_CALT_VLD_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_HP_SDM_L_CALT_VLD_LEN, HI6405_HP_SDM_L_CALT_VLD_OFFSET),
			0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL8_REG, HI6405_HP_SDM_L_DELAY_MASK, 0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL7_REG, HI6405_HP_SDM_L_UP16_DELAY_MASK,	0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_L_CTRL0_REG, 1<<HI6405_HP_SDM_L_DEM_DSEN_OFFSET,
			1<<HI6405_HP_SDM_L_DEM_DSEN_OFFSET);
		/* src */
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG, 1<<HI6405_CLK_DACL_UP16_EN_OFFSET, 0);
		/* pga */
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACL_POST_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACL_PRE_PGA_CLK_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacr_pga_mux_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* pga */
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACR_PRE_PGA_CLK_EN_OFFSET,
			1<<HI6405_DACR_PRE_PGA_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACR_POST_PGA_CLK_EN_OFFSET,
			1<<HI6405_DACR_POST_PGA_CLK_EN_OFFSET);

		/* src */
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			1<<HI6405_CLK_DACR_UP16_EN_OFFSET, 1<<HI6405_CLK_DACR_UP16_EN_OFFSET);
		/* sdm */
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL0_REG,
			1<<HI6405_HP_SDM_R_DEM_DSEN_OFFSET,
			1<<HI6405_HP_SDM_R_DEM_DSEN_OFFSET);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL7_REG, HI6405_HP_SDM_R_UP16_DELAY_MASK,
			HI6405_HP_SDM_R_UP16_DELAY_VAULE);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL8_REG, HI6405_HP_SDM_R_DELAY_MASK,
			HI6405_HP_SDM_R_DELAY_VALUE);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_HP_SDM_R_CALT_VLD_LEN, HI6405_HP_SDM_R_CALT_VLD_OFFSET),
			HI6405_HP_SDM_R_CELLSEL_MODE_2<<HI6405_HP_SDM_R_CELLSEL_MODE_OFFSET);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL12_REG, 1<<HI6405_HP_SDM_R_CALT_VLD_OFFSET,
			1<<HI6405_HP_SDM_R_CALT_VLD_OFFSET);
		/* dacr dc */
		hi64xx_update_bits(codec, HI6405_DACR_DC_M_REG, HI6405_DACR_DC_M_REG_MASK,
			HI6405_DACR_DC_M_REG_VALUE);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DACR_DC_M_REG, HI6405_DACR_DC_M_REG_MASK, 0);
		/* sdm */
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL12_REG, 1<<HI6405_HP_SDM_R_CALT_VLD_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL9_REG,
			HI6405_MASK_ON_BIT(HI6405_HP_SDM_R_CALT_VLD_LEN, HI6405_HP_SDM_R_CALT_VLD_OFFSET),
			0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL8_REG, HI6405_HP_SDM_R_DELAY_MASK, 0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL7_REG, HI6405_HP_SDM_R_UP16_DELAY_MASK,	0);
		hi64xx_update_bits(codec, HI6405_HP_SDM_R_CTRL0_REG, 1<<HI6405_HP_SDM_R_DEM_DSEN_OFFSET,
			1<<HI6405_HP_SDM_R_DEM_DSEN_OFFSET);
		/* src */
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG, 1<<HI6405_CLK_DACR_UP16_EN_OFFSET, 0);
		/* pga */
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACR_POST_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_MISC_CLK_EN_REG, 1<<HI6405_DACR_PRE_PGA_CLK_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int hi6405_dacsl_pga_mux_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* pga */
		hi64xx_update_bits(codec, HI6405_DMIC_CLK_EN_REG, 1<<HI6405_DACL_S_PGA_CLK_EN_OFFSET,
			1<<HI6405_DACL_S_PGA_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DACSL_PGA_CTRL0_REG, 1<<HI6405_DACSL_PGA_BYPASS_OFFSET,
			1<<HI6405_DACSL_PGA_BYPASS_OFFSET);
		/* src */
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG, 1<<HI6405_CLK_DACL_S_UP16_EN_OFFSET,
			1<<HI6405_CLK_DACL_S_UP16_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG, 1<<HI6405_CLK_DACL_S_UP16_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_DACSL_PGA_CTRL0_REG, 1<<HI6405_DACSL_PGA_BYPASS_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_DMIC_CLK_EN_REG, 1<<HI6405_DACL_S_PGA_CLK_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

static int HI6405_ep_l_sdm_mux_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);

	IN_FUNCTION
	WARN_ON(!codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* sdm CAL en */
		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMMD_SEL_LEN, HI6405_EP_SDM_L_DEMMD_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMMD_SEL_CAL<<HI6405_EP_SDM_L_DEMMD_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMHI_SEL_LEN, HI6405_EP_SDM_L_DEMHI_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMHI_SEL_CAL<<HI6405_EP_SDM_L_DEMHI_SEL_OFFSET);

		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL2_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMLO_SEL_LEN, HI6405_EP_SDM_L_DEMLO_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMHO_SEL_CAL<<HI6405_EP_SDM_L_DEMLO_SEL_OFFSET);
		/* sdm valid */
		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL3_REG, 1<<HI6405_EP_SDM_L_CALT_VLD_OFFSET,
			1<<HI6405_EP_SDM_L_CALT_VLD_OFFSET);

		/* bypass pga */
		hi64xx_update_bits(codec, HI6405_DACSL_MIXER4_PGA_CTRL0_REG,
			1<<HI6405_DACSL_MIXER4_PGA_BYPASS_OFFSET,
			1<<HI6405_DACSL_MIXER4_PGA_BYPASS_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DACSL_MIXER4_PGA_CTRL0_REG,
			1<<HI6405_DACSL_MIXER4_PGA_BYPASS_OFFSET,
			0);

		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL3_REG, 1<<HI6405_EP_SDM_L_CALT_VLD_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL2_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMLO_SEL_LEN, HI6405_EP_SDM_L_DEMLO_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMHO_SEL_DEFAULT<<HI6405_EP_SDM_L_DEMLO_SEL_OFFSET);

		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMHI_SEL_LEN, HI6405_EP_SDM_L_DEMHI_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMHI_SEL_DEFAULT<<HI6405_EP_SDM_L_DEMHI_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_EP_SDM_L_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_EP_SDM_L_DEMMD_SEL_LEN, HI6405_EP_SDM_L_DEMMD_SEL_OFFSET),
			HI6405_EP_SDM_L_DEMMD_SEL_DEFAULT<<HI6405_EP_SDM_L_DEMMD_SEL_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	OUT_FUNCTION
	return 0;
}

/* SWITCH - AUDIODOWN */
static const struct snd_kcontrol_new hi6405_dapm_play44k1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY44K1_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_play48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY48K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_play96k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY96K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_play192k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY192K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_play384k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY384K_BIT, 1, 0);

static const struct snd_kcontrol_new hi6405_dapm_hp_high_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_HP_HIGH_BIT, 1, 0);

static const struct snd_kcontrol_new hi6405_dapm_hp_concurrency_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_HP_CONCURRENCY_BIT, 1, 0);

/* SWITCH - PLL TEST */
static const struct snd_kcontrol_new hi6405_dapm_pll44k1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLL48K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_pll48k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLL44K1_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_pll32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLL32K_BIT, 1, 0);

/* SWITCH - S1-S4 */
static const struct snd_kcontrol_new hi6405_dapm_s2_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_S2_OL_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s2_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_S2_OR_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s4_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_S4_OL_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s4_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_S4_OR_BIT, 1, 0);

/* SWITCH - VIR */
static const struct snd_kcontrol_new hi6405_dapm_play384k_vir_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_PLAY384K_VIR_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_iv_dspif_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_IV_DSPIF_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_iv_2pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_IV_2PA_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_iv_4pa_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_IV_4PA_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_ir_emission_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_IR_EMISSION_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_ec_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_EC_BIT, 1, 0);

/* MIXER */
static const struct snd_kcontrol_new hi6405_dsdl_mixer_controls[] = {
	SOC_DAPM_SINGLE("PGA",
		HI6405_DSDL_MIXER2_CTRL1_REG, HI6405_DSDL_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACL_384K",
		HI6405_DSDL_MIXER2_CTRL1_REG, HI6405_DSDL_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dsdr_mixer_controls[] = {
	SOC_DAPM_SINGLE("PGA",
		HI6405_DSDR_MIXER2_CTRL1_REG, HI6405_DSDR_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACR_384K",
		HI6405_DSDR_MIXER2_CTRL1_REG, HI6405_DSDR_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacl_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1_L",
		HI6405_DACL_MIXER4_CTRL1_REG, HI6405_DACL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S2_L",
		HI6405_DACL_MIXER4_CTRL1_REG, HI6405_DACL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S3_L",
		HI6405_DACL_MIXER4_CTRL1_REG, HI6405_DACL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_R",
		HI6405_DACL_MIXER4_CTRL1_REG, HI6405_DACL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacr_mixer_controls[] = {
	SOC_DAPM_SINGLE("S1_R",
		HI6405_DACR_MIXER4_CTRL1_REG, HI6405_DACR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S2_R",
		HI6405_DACR_MIXER4_CTRL1_REG, HI6405_DACR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		HI6405_DACR_MIXER4_CTRL1_REG, HI6405_DACR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_L",
		HI6405_DACR_MIXER4_CTRL1_REG, HI6405_DACR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacl_pre_mixer_controls[] = {
	SOC_DAPM_SINGLE("MUX9",
		HI6405_DACL_PRE_MIXER2_CTRL0_REG, HI6405_DACL_PRE_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		HI6405_DACL_PRE_MIXER2_CTRL0_REG, HI6405_DACL_PRE_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacr_pre_mixer_controls[] = {
	SOC_DAPM_SINGLE("MUX10",
		HI6405_DACR_PRE_MIXER2_CTRL0_REG, HI6405_DACR_PRE_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		HI6405_DACR_PRE_MIXER2_CTRL0_REG, HI6405_DACR_PRE_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacl_post_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACLSRC",
		HI6405_DACL_POST_MIXER2_CTRL0_REG, HI6405_DACL_POST_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_L",
		HI6405_DACL_POST_MIXER2_CTRL0_REG, HI6405_DACL_POST_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacr_post_mixer_controls[] = {
	SOC_DAPM_SINGLE("DACRSRC",
		HI6405_DACR_POST_MIXER2_CTRL0_REG, HI6405_DACR_POST_MIXER2_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("S1_R",
		HI6405_DACR_POST_MIXER2_CTRL0_REG, HI6405_DACR_POST_MIXER2_IN2_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacsl_mixer_controls[] = {
	SOC_DAPM_SINGLE("UTW",
		HI6405_DACSL_MIXER4_CTRL1_REG, HI6405_DACSL_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACSL_PGA",
		HI6405_DACSL_MIXER4_CTRL1_REG, HI6405_DACSL_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		HI6405_DACSL_MIXER4_CTRL1_REG, HI6405_DACSL_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		HI6405_DACSL_MIXER4_CTRL1_REG, HI6405_DACSL_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

static const struct snd_kcontrol_new hi6405_dacsr_mixer_controls[] = {
	SOC_DAPM_SINGLE("UTW",
		HI6405_DACSR_MIXER4_CTRL1_REG, HI6405_DACSR_MIXER4_IN1_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("DACSR_PGA",
		HI6405_DACSR_MIXER4_CTRL1_REG, HI6405_DACSR_MIXER4_IN2_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("MDM",
		HI6405_DACSR_MIXER4_CTRL1_REG, HI6405_DACSR_MIXER4_IN3_MUTE_OFFSET, 1, 1),
	SOC_DAPM_SINGLE("SIDETONE",
		HI6405_DACSR_MIXER4_CTRL1_REG, HI6405_DACSR_MIXER4_IN4_MUTE_OFFSET, 1, 1),
};

/* S1_IL_MUX*/
static const char *hi6405_dapm_s1_il_mux_texts[] = {
	"S1_INL",
	"D1",
};
static const struct soc_enum hi6405_dapm_s1_il_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL0_REG, HI6405_S1_IL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_s1_il_mux_texts), hi6405_dapm_s1_il_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_s1_il_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_s1_il_mux_enum);

/* S1_IR_MUX*/
static const char *hi6405_dapm_s1_ir_mux_texts[] = {
	"S1_INR",
	"D2",
};
static const struct soc_enum hi6405_dapm_s1_ir_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL0_REG, HI6405_S1_IR_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_s1_ir_mux_texts), hi6405_dapm_s1_ir_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_s1_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_s1_ir_mux_enum);

/* S2_IL_MUX*/
static const char *hi6405_dapm_s2_il_mux_texts[] = {
	"S2_INL",
	"D3",
};
static const struct soc_enum hi6405_dapm_s2_il_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL0_REG, HI6405_S2_IL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_s2_il_mux_texts), hi6405_dapm_s2_il_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_s2_il_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_s2_il_mux_enum);

/* S2_IR_MUX*/
static const char *hi6405_dapm_s2_ir_mux_texts[] = {
	"S2_INR",
	"D4",
};
static const struct soc_enum hi6405_dapm_s2_ir_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL0_REG, HI6405_S2_IR_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_s2_ir_mux_texts), hi6405_dapm_s2_ir_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_s2_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_s2_ir_mux_enum);

static const char *hi6405_dapm_dsp_if8_mux_texts[] = {
	"DAC_MIXER",
	"ADC",
	"HP_SDM_L",
	"EP_SDM_L",
	"CLASSH_DEBUG",
	"DSDL",
};
static const struct soc_enum hi6405_dapm_dsp_if8_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL11_REG, HI6405_DSPIF8_TEST_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dsp_if8_mux_texts), hi6405_dapm_dsp_if8_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dsp_if8_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dsp_if8_mux_enum);

static const char *hi6405_dapm_dacl_pre_mux_texts[] = {
	"DSP_IF8_OL",
	"DACL_MIXER",
};
static const struct soc_enum hi6405_dapm_dacl_pre_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL9_REG, HI6405_DACL_PRE_MIXER2_IN1_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dacl_pre_mux_texts), hi6405_dapm_dacl_pre_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dacl_pre_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dacl_pre_mux_enum);

static const char *hi6405_dapm_dacr_pre_mux_texts[] = {
	"DSP_IF8_OR",
	"DACR_MIXER",
};
static const struct soc_enum hi6405_dapm_dacr_pre_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL9_REG, HI6405_DACR_PRE_MIXER2_IN1_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dacr_pre_mux_texts), hi6405_dapm_dacr_pre_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dacr_pre_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dacr_pre_mux_enum);

static const char *hi6405_dapm_dacl_pga_mux_texts[] = {
	"DACL_384K",
	"ADCL1",
	"HP_RES_DET",
	"S1_L",
};
static const struct soc_enum hi6405_dapm_dacl_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL12_REG, HI6405_DACL_PRE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dacl_pga_mux_texts), hi6405_dapm_dacl_pga_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dacl_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dacl_pga_mux_enum);

static const char *hi6405_dapm_dacr_pga_mux_texts[] = {
	"DACR_384K",
	"ADCR1",
	"HP_RES_DET",
	"S1_R",
};
static const struct soc_enum hi6405_dapm_dacr_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL12_REG, HI6405_DACR_PRE_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dacr_pga_mux_texts), hi6405_dapm_dacr_pga_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dacr_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dacr_pga_mux_enum);

static const char *hi6405_dapm_hp_l_sdm_mux_texts[] = {
	"DACL_UP16",
	"DSD_L",
	"ANC_L",
	"IO_TEST",
};
static const struct soc_enum hi6405_dapm_hp_l_sdm_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL13_REG, HI6405_HP_SDM_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_hp_l_sdm_mux_texts), hi6405_dapm_hp_l_sdm_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_hp_l_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_hp_l_sdm_mux_enum);

static const char *hi6405_dapm_hp_r_sdm_mux_texts[] = {
	"DACR_UP16",
	"DSD_R",
	"ANC_R",
	"IO_TEST",
};
static const struct soc_enum hi6405_dapm_hp_r_sdm_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL13_REG, HI6405_HP_SDM_R_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_hp_r_sdm_mux_texts), hi6405_dapm_hp_r_sdm_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_hp_r_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_hp_r_sdm_mux_enum);

static const char *hi6405_dapm_ep_l_sdm_mux_texts[] = {
	"DACSL_UP16",
	"ANC_L",
	"ANC_R",
	"IO_TEST",
};
static const struct soc_enum hi6405_dapm_ep_l_sdm_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL13_REG, HI6405_EP_SDM_L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_ep_l_sdm_mux_texts), hi6405_dapm_ep_l_sdm_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_ep_l_sdm_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_ep_l_sdm_mux_enum);

static const char *hi6405_dapm_dacsl_pag_mux_texts[] = {
	"DACSL_384K",
	"HP_RES_DET",
};
static const struct soc_enum hi6405_dapm_dacsl_pag_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL38_REG, HI6405_DACSL_PGA_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_dacsl_pag_mux_texts), hi6405_dapm_dacsl_pag_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_dacsl_pag_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_dacsl_pag_mux_enum);

static const char *hi6405_dapm_hp_dac_l_mux_texts[] = {
	"HP_L_SDM",
	"HP_R_SDM",
	"HP_ICELL_CAB",
	"HP_SDM_MUX",
	"DSPIF8",
};
static const struct soc_enum hi6405_dapm_hp_dac_l_mux_enum =
	SOC_ENUM_SINGLE(HI6405_DAC_DATA_SEL_CTRL0_REG, HI6405_DAC0_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_hp_dac_l_mux_texts), hi6405_dapm_hp_dac_l_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_hp_dac_l_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_hp_dac_l_mux_enum);

static const char *hi6405_dapm_hp_dac_r_mux_texts[] = {
	"HP_R_SDM",
	"HP_L_SDM",
	"HP_ICELL_CAB",
	"HP_SDM_MUX",
	"DSPIF8",
};
static const struct soc_enum hi6405_dapm_hp_dac_r_mux_enum =
	SOC_ENUM_SINGLE(HI6405_DAC_DATA_SEL_CTRL0_REG, HI6405_DAC1_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_hp_dac_r_mux_texts), hi6405_dapm_hp_dac_r_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_hp_dac_r_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_hp_dac_r_mux_enum);

static const char *hi6405_dapm_ep_dac_mux_texts[] = {
	"EP_L_SDM",
	"AP_ICELL_CAB",
	"EP_SDM_MUX",
	"DSPIF8",
};
static const struct soc_enum hi6405_dapm_ep_dac_mux_enum =
	SOC_ENUM_SINGLE(HI6405_DAC_DATA_SEL_CTRL1_REG, HI6405_DAC2_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_ep_dac_mux_texts), hi6405_dapm_ep_dac_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_ep_dac_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_ep_dac_mux_enum);

static const char *hi6405_dapm_ir_mux_texts[] = {
	"DSPIF8",
	"IR_REG_CTRL",
	"IO_TEST_IN",
	"OTHERS",
};
static const struct soc_enum hi6405_dapm_ir_mux_enum =
	SOC_ENUM_SINGLE(HI6405_IR_SOURCE_CTRL_REG, HI6405_IR_SOURCE_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_ir_mux_texts), hi6405_dapm_ir_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_ir_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_ir_mux_enum);

static const char *hi6405_dapm_mdm_mux_texts[] = {
	"S3_L",
	"S3_R",
};
static const struct soc_enum hi6405_dapm_mdm_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL14_REG, HI6405_MDM_SEL_OFFSET,
		ARRAY_SIZE(hi6405_dapm_mdm_mux_texts), hi6405_dapm_mdm_mux_texts);
static const struct snd_kcontrol_new hi6405_dapm_mdm_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_dapm_mdm_mux_enum);

static int hi6405_micbias1_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(platform_data == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS1,
			1<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET | 0xF,
			0<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET | HI6405_ANA_MICBIAS1_ADJ_2_7V);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS_ENABLE, 1<<HI6405_ANA_MICBIAS1_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS_ENABLE,
			1<<HI6405_ANA_MICBIAS1_PD_BIT, 1<<HI6405_ANA_MICBIAS1_PD_BIT);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS1,
			1<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET | 0xF,
			1<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET);
		msleep(10);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS1,
			1<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET | 0xF, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_micbias2_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(platform_data == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS2,
			1<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET | 0xF,
			0<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET | HI6405_ANA_MICBIAS2_ADJ_2_7V);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS_ENABLE, 1<<HI6405_ANA_MICBIAS2_PD_BIT, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS_ENABLE,
			1<<HI6405_ANA_MICBIAS2_PD_BIT, 1<<HI6405_ANA_MICBIAS2_PD_BIT);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS2,
			1<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET | 0xF,
			1<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET);
		msleep(10);
		hi64xx_update_bits(codec, HI6405_ANA_MICBIAS2,
			1<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET | 0xF, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_hsmicbias_power_event(struct snd_soc_dapm_widget *w,
		struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(platform_data == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_resmgr_request_micbias(platform_data->resmgr);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_resmgr_release_micbias(platform_data->resmgr);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return 0;
}

static int hi6405_hsmic_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_HSMIC_PGA_ENABLE,
			1<<HI6405_ANA_HSMIC_PGA_PD | 1<<HI6405_ANA_HSMIC_PGA_MUTE2 |
			1<<HI6405_ANA_HSMIC_PGA_MUTE1 | 1<<HI6405_ANA_HSMIC_FLASH_MUTE |
			1<<HI6405_ANA_HSMIC_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_HSMIC_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_HSMIC_OFFSET, 1<<HI6405_ANA_PD_OUT_HSMIC_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_HSMIC_PGA_ENABLE,
			1<<HI6405_ANA_HSMIC_PGA_PD | 1<<HI6405_ANA_HSMIC_PGA_MUTE2 |
			1<<HI6405_ANA_HSMIC_PGA_MUTE1 | 1<<HI6405_ANA_HSMIC_FLASH_MUTE |
			1<<HI6405_ANA_HSMIC_PGA_SDM_PD,
			1<<HI6405_ANA_HSMIC_PGA_PD | 1<<HI6405_ANA_HSMIC_PGA_MUTE2 |
			1<<HI6405_ANA_HSMIC_PGA_MUTE1 | 1<<HI6405_ANA_HSMIC_FLASH_MUTE |
			1<<HI6405_ANA_HSMIC_PGA_SDM_PD);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return 0;
}

static int hi6405_mad_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MAD_PGA_ENABLE,
			1<<HI6405_ANA_MAD_PGA_PD | 1<<HI6405_ANA_MAD_PGA_MUTE2 | 1<<HI6405_ANA_MAD_PGA_MUTE1, 0);
		hi64xx_update_bits(codec, HI6405_ANA_MAD_PGA_SDM,
			1<<HI6405_ANA_MAD_PGA_PD_OUT | 1<<HI6405_ANA_MAD_PGA_PD_IBIAS |
			1<<HI6405_ANA_MAD_PGA_PD_SDM | 1<<HI6405_ANA_MAD_PGA_MUTE_FLS |
			1<<HI6405_ANA_MAD_PGA_MUTE_DWA_OUT, 0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_000,
			0x1<<HI6405_PD_MAD_SDM_VREF_OFFSET, 0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_000,
			0x1<<HI6405_PD_MAD_SDM_VREF_OFFSET, 0x1<<HI6405_PD_MAD_SDM_VREF_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_MAD_PGA_SDM,
			1<<HI6405_ANA_MAD_PGA_PD_OUT | 1<<HI6405_ANA_MAD_PGA_PD_IBIAS |
			1<<HI6405_ANA_MAD_PGA_PD_SDM | 1<<HI6405_ANA_MAD_PGA_MUTE_FLS |
			1<<HI6405_ANA_MAD_PGA_MUTE_DWA_OUT,
			1<<HI6405_ANA_MAD_PGA_PD_OUT | 1<<HI6405_ANA_MAD_PGA_PD_IBIAS |
			1<<HI6405_ANA_MAD_PGA_PD_SDM | 1<<HI6405_ANA_MAD_PGA_MUTE_FLS |
			1<<HI6405_ANA_MAD_PGA_MUTE_DWA_OUT);
		hi64xx_update_bits(codec, HI6405_ANA_MAD_PGA_ENABLE,
			1<<HI6405_ANA_MAD_PGA_PD | 1<<HI6405_ANA_MAD_PGA_MUTE2 | 1<<HI6405_ANA_MAD_PGA_MUTE1,
			1<<HI6405_ANA_MAD_PGA_PD | 1<<HI6405_ANA_MAD_PGA_MUTE2 | 1<<HI6405_ANA_MAD_PGA_MUTE1);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_auxmic_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_AUXMIC_PGA_ENABLE,
			1<<HI6405_ANA_AUXMIC_PGA_PD | 1<<HI6405_ANA_AUXMIC_PGA_MUTE2 |
			1<<HI6405_ANA_AUXMIC_PGA_MUTE1 | 1<<HI6405_ANA_AUXMIC_FLASH_MUTE |
			1<<HI6405_ANA_AUXMIC_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_AUXMIC_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_AUXMIC_OFFSET, 1<<HI6405_ANA_PD_OUT_AUXMIC_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_AUXMIC_PGA_ENABLE,
			1<<HI6405_ANA_AUXMIC_PGA_PD | 1<<HI6405_ANA_AUXMIC_PGA_MUTE2 |
			1<<HI6405_ANA_AUXMIC_PGA_MUTE1 | 1<<HI6405_ANA_AUXMIC_FLASH_MUTE |
			1<<HI6405_ANA_AUXMIC_PGA_SDM_PD,
			1<<HI6405_ANA_AUXMIC_PGA_PD | 1<<HI6405_ANA_AUXMIC_PGA_MUTE2 |
			1<<HI6405_ANA_AUXMIC_PGA_MUTE1 | 1<<HI6405_ANA_AUXMIC_FLASH_MUTE |
			1<<HI6405_ANA_AUXMIC_PGA_SDM_PD);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_mic3_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MIC3_PGA_ENABLE,
			1<<HI6405_ANA_MIC3_PGA_PD | 1<<HI6405_ANA_MIC3_PGA_MUTE2 |
			1<<HI6405_ANA_MIC3_PGA_MUTE1 | 1<<HI6405_ANA_MIC3_FLASH_MUTE |
			1<<HI6405_ANA_MIC3_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC3_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC3_OFFSET, 1<<HI6405_ANA_PD_OUT_MIC3_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_MIC3_PGA_ENABLE,
			1<<HI6405_ANA_MIC3_PGA_PD | 1<<HI6405_ANA_MIC3_PGA_MUTE2 |
			1<<HI6405_ANA_MIC3_PGA_MUTE1 | 1<<HI6405_ANA_MIC3_FLASH_MUTE |
			1<<HI6405_ANA_MIC3_PGA_SDM_PD,
			1<<HI6405_ANA_MIC3_PGA_PD | 1<<HI6405_ANA_MIC3_PGA_MUTE2 |
			1<<HI6405_ANA_MIC3_PGA_MUTE1 | 1<<HI6405_ANA_MIC3_FLASH_MUTE |
			1<<HI6405_ANA_MIC3_PGA_SDM_PD);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_mic4_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MIC4_PGA_ENABLE,
			1<<HI6405_ANA_MIC4_PGA_PD | 1<<HI6405_ANA_MIC4_PGA_MUTE2 |
			1<<HI6405_ANA_MIC4_PGA_MUTE1 | 1<<HI6405_ANA_MIC4_FLASH_MUTE |
			1<<HI6405_ANA_MIC4_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC4_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC4_OFFSET, 1<<HI6405_ANA_PD_OUT_MIC4_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_MIC4_PGA_ENABLE,
			1<<HI6405_ANA_MIC4_PGA_PD | 1<<HI6405_ANA_MIC4_PGA_MUTE2 |
			1<<HI6405_ANA_MIC4_PGA_MUTE1 | 1<<HI6405_ANA_MIC4_FLASH_MUTE |
			1<<HI6405_ANA_MIC4_PGA_SDM_PD,
			1<<HI6405_ANA_MIC4_PGA_PD | 1<<HI6405_ANA_MIC4_PGA_MUTE2 |
			1<<HI6405_ANA_MIC4_PGA_MUTE1 | 1<<HI6405_ANA_MIC4_FLASH_MUTE |
			1<<HI6405_ANA_MIC4_PGA_SDM_PD);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_mic5_pga_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ANA_MIC5_PGA_ENABLE,
			1<<HI6405_ANA_MIC5_PGA_PD | 1<<HI6405_ANA_MIC5_PGA_MUTE2 |
			1<<HI6405_ANA_MIC5_PGA_MUTE1 | 1<<HI6405_ANA_MIC5_FLASH_MUTE |
			1<<HI6405_ANA_MIC5_PGA_SDM_PD, 0);
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC5_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_ADC_SDM,
			1<<HI6405_ANA_PD_OUT_MIC5_OFFSET, 1<<HI6405_ANA_PD_OUT_MIC5_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_MIC5_PGA_ENABLE,
			1<<HI6405_ANA_MIC5_PGA_PD | 1<<HI6405_ANA_MIC5_PGA_MUTE2 |
			1<<HI6405_ANA_MIC5_PGA_MUTE1 | 1<<HI6405_ANA_MIC5_FLASH_MUTE |
			1<<HI6405_ANA_MIC5_PGA_SDM_PD,
			1<<HI6405_ANA_MIC5_PGA_PD | 1<<HI6405_ANA_MIC5_PGA_MUTE2 |
			1<<HI6405_ANA_MIC5_PGA_MUTE1 | 1<<HI6405_ANA_MIC5_FLASH_MUTE |
			1<<HI6405_ANA_MIC5_PGA_SDM_PD);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcl0_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC0L_EN_OFFSET, 1<<HI6405_CLK_ADC0L_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			1<<HI6405_ADC0L_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC0L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			1<<HI6405_ADC0L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC0L_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcr0_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC0R_EN_OFFSET, 1<<HI6405_CLK_ADC0R_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			1<<HI6405_ADC0R_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC0R_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			1<<HI6405_ADC0R_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC0R_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcl1_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC1L_EN_OFFSET, 1<<HI6405_CLK_ADC1L_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_1_REG,
			1<<HI6405_ADC1L_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC1L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_1_REG,
			1<<HI6405_ADC1L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC1L_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcr1_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC1R_EN_OFFSET, 1<<HI6405_CLK_ADC1R_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_1_REG,
			1<<HI6405_ADC1R_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC1R_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_1_REG,
			1<<HI6405_ADC1R_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_ADC_DAC_CLK_EN_REG,
			1<<HI6405_CLK_ADC1R_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcl2_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	WARN_ON(w == NULL);
	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(codec == NULL);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			1<<HI6405_CLK_ADC2L_EN_OFFSET, 1<<HI6405_CLK_ADC2L_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			1<<HI6405_CLK_ADC2L_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

/* U1 U2 selesct for wakeup app may died */
static void hi6405_u12_select_pga(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
		0x3<<HI6405_SLIM_UP12_DATA_SEL_OFFSET, 0x0<<HI6405_SLIM_UP12_DATA_SEL_OFFSET);
}

static void hi6405_u12_select_dspif(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
		0x3<<HI6405_SLIM_UP12_DATA_SEL_OFFSET, 0x1<<HI6405_SLIM_UP12_DATA_SEL_OFFSET);
}

static int hi6405_audioup_2mic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;
	int ret = 0;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		platform_data->capture_params.channels = 2;
		/* src in && out sample rate */
		snd_soc_write(codec, HI6405_SRC_VLD_CTRL2_REG, 0x1C);
		/* src disable clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			0x1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET, 0x0);
		/* src down 2 mode */
		hi64xx_update_bits(codec, HI6405_MIC1_SRC_CTRL_REG,
			0x7<<HI6405_MIC1_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC1_SRC_MODE_OFFSET);
		hi64xx_update_bits(codec, HI6405_MIC2_SRC_CTRL_REG,
			0x7<<HI6405_MIC2_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC2_SRC_MODE_OFFSET);
		/* src enable clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET,
			1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET);

		hi6405_u12_select_pga(codec);
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_CAPTURE, &platform_data->capture_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate capture err \n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		if (TRACK_FREE == platform_data->audioup_4mic_state) {
			hi6405_u12_select_dspif(codec);
			ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_CAPTURE, NULL);
			if (ret) {
				HI6405_LOGE("slimbus_track_deactivate capture err \n");
			}
			/* src power off */
			hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
				1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET, 0x0);
			hi64xx_update_bits(codec, HI6405_MIC2_SRC_CTRL_REG,
				0x7<<HI6405_MIC2_SRC_MODE_OFFSET, 0x0);
			hi64xx_update_bits(codec, HI6405_MIC1_SRC_CTRL_REG,
				0x7<<HI6405_MIC1_SRC_MODE_OFFSET, 0x0);
			snd_soc_write(codec, HI6405_SRC_VLD_CTRL2_REG, 0x0);
		}
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return ret;
}

static int hi6405_audioup_4mic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;
	int ret = 0;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	if (platform_data->voiceup_state && (4 == platform_data->voice_up_params.channels)) {
		platform_data->capture_params.channels = 2;
	} else {
		platform_data->capture_params.channels = 4;
	}

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		if (4 == platform_data->capture_params.channels) {
			hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
						0x1<<HI6405_RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0x1<<HI6405_RST_5MIC_S1_ACCESS_IRQ_OFFSET);
		}
		snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_3_REG, 0x44);
		/* src in && out sample rate */
		snd_soc_write(codec, HI6405_SRC_VLD_CTRL2_REG, 0x1C);
		/* src clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			0x1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET |
			0x1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET, 0x0);
		/* src down 2 mode */
		hi64xx_update_bits(codec, HI6405_MIC1_SRC_CTRL_REG,
			0x7<<HI6405_MIC1_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC1_SRC_MODE_OFFSET);
		hi64xx_update_bits(codec, HI6405_MIC2_SRC_CTRL_REG,
			0x7<<HI6405_MIC2_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC2_SRC_MODE_OFFSET);
		/* src clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET,
			1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET);
		if (4 == platform_data->capture_params.channels) {
			snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_4_REG, 0x44);
			snd_soc_write(codec, HI6405_SRC_VLD_CTRL3_REG, 0xC);
			snd_soc_write(codec, HI6405_SRC_VLD_CTRL4_REG, 0xC);
			hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
				0x1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET |
				0x1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET, 0x0);
			hi64xx_update_bits(codec, HI6405_MIC3_SRC_CTRL_REG,
				0x7<<HI6405_MIC3_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC3_SRC_MODE_OFFSET);
			hi64xx_update_bits(codec, HI6405_MIC4_SRC_CTRL_REG,
				0x7<<HI6405_MIC4_SRC_MODE_OFFSET, SRC_MODE_2<<HI6405_MIC4_SRC_MODE_OFFSET);
			hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
				1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET,
				1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET);
		}
		hi6405_u12_select_pga(codec);

		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_CAPTURE, &platform_data->capture_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate capture err\n");
		}

		if (4 == platform_data->capture_params.channels)
			hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
						0x1<<HI6405_RST_5MIC_S1_ACCESS_IRQ_OFFSET, 0);

		platform_data->audioup_4mic_state = TRACK_STARTUP;
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi6405_u12_select_dspif(codec);

		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_CAPTURE, &platform_data->capture_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_deactivate capture err \n");
		}
		/* src power off */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC1_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC2_SRC_CLK_EN_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_MIC1_SRC_CTRL_REG,
			0x7<<HI6405_MIC1_SRC_MODE_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_MIC2_SRC_CTRL_REG,
			0x7<<HI6405_MIC2_SRC_MODE_OFFSET, 0x0);
		snd_soc_write(codec, HI6405_SRC_VLD_CTRL2_REG, 0x0);
		if (4 == platform_data->capture_params.channels) {
			hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
				1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET, 0x0);
			hi64xx_update_bits(codec, HI6405_MIC3_SRC_CTRL_REG,
				0x7<<HI6405_MIC3_SRC_MODE_OFFSET, 0x0);
			hi64xx_update_bits(codec, HI6405_MIC4_SRC_CTRL_REG,
				0x7<<HI6405_MIC4_SRC_MODE_OFFSET, 0x0);
			snd_soc_write(codec, HI6405_SRC_VLD_CTRL3_REG, 0x0);
			snd_soc_write(codec, HI6405_SRC_VLD_CTRL4_REG, 0x0);
		}
		platform_data->audioup_4mic_state = TRACK_FREE;
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_audioup_5mic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec = snd_soc_dapm_to_codec(w->dapm);
	int ret = 0;

	IN_FUNCTION
	WARN_ON(!codec);

	/* 0x20007033 0x0 */
	/* 0x200075ce 0x30 */

	OUT_FUNCTION
	return ret;
}

static void hi6405_up_src_pre_pmu(struct snd_soc_codec *codec, unsigned int channel,
	enum HI6405_SAMPLE_RATE sample_rate, enum HI6405_SRC_MODE src_mode)
{
	/* src in && out sample rate */
	hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL7_REG,
		0x1<<HI6405_S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1<<HI6405_S3_OL_SRC_DIN_VLD_SEL_OFFSET,
		0x1<<HI6405_S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1<<HI6405_S3_OL_SRC_DIN_VLD_SEL_OFFSET);
	hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL7_REG,
		0x7<<HI6405_S3_O_SRC_DOUT_VLD_SEL_OFFSET,
		sample_rate<<HI6405_S3_O_SRC_DOUT_VLD_SEL_OFFSET);
	/* src clk */
	hi64xx_update_bits(codec, HI6405_S3_DP_CLK_EN_REG,
		1<<HI6405_S3_OL_SRC_CLK_EN_OFFSET | 1<<HI6405_S3_OR_SRC_CLK_EN_OFFSET, 0x0);
	/* src down xx mode */
	hi64xx_update_bits(codec, HI6405_S3_OL_SRC_CTRL_REG,
		0x7<<HI6405_S3_OL_SRC_MODE_OFFSET, src_mode<<HI6405_S3_OL_SRC_MODE_OFFSET);
	hi64xx_update_bits(codec, HI6405_S3_OR_SRC_CTRL_REG,
		0x7<<HI6405_S3_OR_SRC_MODE_OFFSET, src_mode<<HI6405_S3_OR_SRC_MODE_OFFSET);
	/* src clk */
	hi64xx_update_bits(codec, HI6405_S3_DP_CLK_EN_REG,
		1<<HI6405_S3_OL_SRC_CLK_EN_OFFSET | 1<<HI6405_S3_OR_SRC_CLK_EN_OFFSET,
		1<<HI6405_S3_OL_SRC_CLK_EN_OFFSET | 1<<HI6405_S3_OR_SRC_CLK_EN_OFFSET);
	/* slimbus sample rate */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_5_REG,
		sample_rate<<HI6405_FS_SLIM_UP6_OFFSET |
		sample_rate<<HI6405_FS_SLIM_UP5_OFFSET);
	if (4 == channel) {
		/* mic3/4 in&&out sample rate */
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL3_REG,
			0x7<<HI6405_MIC3_SRC_DOUT_VLD_SEL_OFFSET, sample_rate<<HI6405_MIC3_SRC_DOUT_VLD_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL4_REG,
			0x7<<HI6405_MIC4_SRC_DOUT_VLD_SEL_OFFSET, sample_rate<<HI6405_MIC4_SRC_DOUT_VLD_SEL_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		/* u3/u4 src mode */
		hi64xx_update_bits(codec, HI6405_MIC3_SRC_CTRL_REG,
			0x7<<HI6405_MIC3_SRC_MODE_OFFSET, src_mode<<HI6405_MIC3_SRC_MODE_OFFSET);
		hi64xx_update_bits(codec, HI6405_MIC4_SRC_CTRL_REG,
			0x7<<HI6405_MIC4_SRC_MODE_OFFSET, src_mode<<HI6405_MIC4_SRC_MODE_OFFSET);
		/* u3/u4 clk clk */
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET,
			1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET);

		/* u3/u4 slimbus sample rate */
		snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_4_REG,
			sample_rate<<HI6405_FS_SLIM_UP4_OFFSET |
			sample_rate<<HI6405_FS_SLIM_UP3_OFFSET);
	}
}

static void hi6405_up_src_post_pmu(struct snd_soc_codec *codec, unsigned int channel)
{
	/* slimbus sample rate */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_5_REG,
		0x0<<HI6405_FS_SLIM_UP6_OFFSET | 0x0<<HI6405_FS_SLIM_UP5_OFFSET);
	/* src power off */
	hi64xx_update_bits(codec, HI6405_S3_DP_CLK_EN_REG,
		1<<HI6405_S3_OL_SRC_CLK_EN_OFFSET | 1<<HI6405_S3_OR_SRC_CLK_EN_OFFSET, 0x0);
	hi64xx_update_bits(codec, HI6405_S3_OL_SRC_CTRL_REG,
		0x7<<HI6405_S3_OL_SRC_MODE_OFFSET, 0x0);
	hi64xx_update_bits(codec, HI6405_S3_OR_SRC_CTRL_REG,
		0x7<<HI6405_S3_OR_SRC_MODE_OFFSET, 0x0);
	hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL7_REG,
		0x7<<HI6405_S3_O_SRC_DOUT_VLD_SEL_OFFSET, 0);
	hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL7_REG,
		0x1<<HI6405_S3_OR_SRC_DIN_VLD_SEL_OFFSET | 0x1<<HI6405_S3_OL_SRC_DIN_VLD_SEL_OFFSET, 0x0);
	if (4 == channel) {
		hi64xx_update_bits(codec, HI6405_S1_DP_CLK_EN_REG,
			1<<HI6405_S1_MIC4_SRC_CLK_EN_OFFSET | 1<<HI6405_S1_MIC3_SRC_CLK_EN_OFFSET, 0x0);
		snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_4_REG,
			0x4<<HI6405_FS_SLIM_UP4_OFFSET | 0x4<<HI6405_FS_SLIM_UP3_OFFSET);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL3_REG,
			0x7<<HI6405_MIC3_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_SRC_VLD_CTRL4_REG,
			0x7<<HI6405_MIC4_SRC_DOUT_VLD_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_MIC3_SRC_CTRL_REG,
			0x7<<HI6405_MIC3_SRC_MODE_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_MIC4_SRC_CTRL_REG,
			0x7<<HI6405_MIC4_SRC_MODE_OFFSET, 0x0);
	}
}


static void hi6405_u56_select_dspif(struct snd_soc_codec *codec)
{
	/* U5 U6 selesct dspif for wakeup app may died */
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL22_REG,
		0x3<<HI6405_SLIM_UP6_DATA_SEL_OFFSET, 0x1<<HI6405_SLIM_UP6_DATA_SEL_OFFSET);
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
		0x3<<HI6405_SLIM_UP5_DATA_SEL_OFFSET, 0x1<<HI6405_SLIM_UP5_DATA_SEL_OFFSET);
}

static void hi6405_u56_select_pga(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL22_REG,
		0x3<<HI6405_SLIM_UP6_DATA_SEL_OFFSET, 0x0<<HI6405_SLIM_UP6_DATA_SEL_OFFSET);
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
		0x3<<HI6405_SLIM_UP5_DATA_SEL_OFFSET, 0x0<<HI6405_SLIM_UP5_DATA_SEL_OFFSET);
}

static int hi6405_voice_slimbus_active(struct snd_soc_codec *codec, struct hi6405_platform_data *platform_data)
{
	int ret;

	hi6405_u56_select_pga(codec);
	if (4 == platform_data->voice_up_params.channels) {
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
					0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET, 0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET);
	}

	/* slimbus voice active */
	ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP, &platform_data->voice_up_params);
	if (ret) {
		HI6405_LOGE("slimbus_track_activate voice up err\n");
	}

	if (4 == platform_data->voice_up_params.channels) {
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
					0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET, 0);
	}

	platform_data->voiceup_state = TRACK_STARTUP;

	return ret;
}

static int hi6405_voice_slimbus_deactive(struct snd_soc_codec *codec, struct hi6405_platform_data *platform_data)
{
	int ret;

	/* slimbus voice deactive */
	ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_UP, NULL);
	if (ret) {
		HI6405_LOGE("slimbus_track_deactivate voice up err\n");
	}

	platform_data->voiceup_state = TRACK_FREE;
	hi6405_u56_select_dspif(codec);

	return ret;
}

static int hi6405_voice8k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;
	int ret = 0;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_8K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_8K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_up_src_pre_pmu(codec, platform_data->voice_up_params.channels, SAMPLE_RATE_8K, SRC_MODE_12);
		ret = hi6405_voice_slimbus_active(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_active voice err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = hi6405_voice_slimbus_deactive(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_deactive voice err\n");
		}
		hi6405_up_src_post_pmu(codec, platform_data->voice_up_params.channels);

		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_voice16k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;
	int ret = 0;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_16K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_up_src_pre_pmu(codec, platform_data->voice_up_params.channels, SAMPLE_RATE_16K, SRC_MODE_6);
		ret = hi6405_voice_slimbus_active(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_active voice err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = hi6405_voice_slimbus_deactive(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_deactive voice err\n");
		}
		hi6405_up_src_post_pmu(codec, platform_data->voice_up_params.channels);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_voice32k_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;
	int ret = 0;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->voice_up_params.rate = SLIMBUS_SAMPLE_RATE_32K;
	platform_data->voice_down_params.rate = SLIMBUS_SAMPLE_RATE_32K;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_up_src_pre_pmu(codec, platform_data->voice_up_params.channels, SAMPLE_RATE_32K, SRC_MODE_3);
		ret = hi6405_voice_slimbus_active(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_active voice err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = hi6405_voice_slimbus_deactive(codec, platform_data);
		if (ret) {
			HI6405_LOGE("hi6405_voice_slimbus_deactive voice err\n");
		}
		hi6405_up_src_post_pmu(codec, platform_data->voice_up_params.channels);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_madswitch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* mad src enable */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG, 1<<HI6405_S1_MAD_SRC_CLK_EN_OFFSET, 1<<HI6405_S1_MAD_SRC_CLK_EN_OFFSET);
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL21_REG,
			0x3<<HI6405_S1_O_DSP_IF_DIN_SEL_OFFSET, 0x2<<HI6405_S1_O_DSP_IF_DIN_SEL_OFFSET);
		hi6405_u12_select_dspif(codec);
		hi6405_u56_select_dspif(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* s1_o_dsp_if_din_sel->mad_buffer out */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL21_REG,
			0x3<<HI6405_S1_O_DSP_IF_DIN_SEL_OFFSET, 0x0);
		hi6405_u12_select_pga(codec);
		hi6405_u56_select_pga(codec);
		/* mad src disable */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG, 1<<HI6405_S1_MAD_SRC_CLK_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_virtbtn_active_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		HI6405_LOGW("hi6405_virtbtn_active_switch_power_event 1");
		hi64xx_update_bits(codec, HI6405_DSPIF_VLD_CTRL4_REG,
			0x7<<HI6405_DSPIF8_DIN_VLD_SEL_OFFSET, 0x4<<HI6405_DSPIF8_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		HI6405_LOGW("hi6405_virtbtn_active_switch_power_event 0");
		hi64xx_update_bits(codec, HI6405_DSPIF_VLD_CTRL4_REG,
			0x7<<HI6405_DSPIF8_DIN_VLD_SEL_OFFSET, 0x0<<HI6405_DSPIF8_DIN_VLD_SEL_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_virtbtn_passive_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;


	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		HI6405_LOGW("hi6405_virtbtn_passive_switch_power_event 1");
		hi64xx_update_bits(codec, HI6405_APB_CLK_CFG_REG,
			1<<HI6405_GPIO_PCLK_EN_OFFSET, 1<<HI6405_GPIO_PCLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DSPIF_VLD_CTRL5_REG,
			0x7<<HI6405_DSPIF9_DIN_VLD_SEL_OFFSET, 0x0<<HI6405_DSPIF9_DIN_VLD_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		HI6405_LOGW("hi6405_virtbtn_passive_switch_power_event 0");
		hi64xx_update_bits(codec, HI6405_DSPIF_VLD_CTRL5_REG,
			0x7<<HI6405_DSPIF9_DIN_VLD_SEL_OFFSET, 0x0<<HI6405_DSPIF9_DIN_VLD_SEL_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_ir_study_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_EMS_OFFSET, 0x1<<HI6405_CODEC_PD_INF_EMS_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_LRN_OFFSET, 0<<HI6405_CODEC_PD_INF_LRN_OFFSET);
		hi64xx_update_bits(codec, HI6405_ANA_IR_EN,
			0x3<<HI6405_ANA_IR_INF_SEL_OFFSET, 1<<HI6405_ANA_IR_INF_SEL_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		hi64xx_update_bits(codec, HI6405_ANA_IR_EN,
			0x3<<HI6405_ANA_IR_INF_SEL_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_PD_INF_LRN_OFFSET, 0x1<<HI6405_CODEC_PD_INF_LRN_OFFSET);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_adcl2_virtbtn_ir_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		HI6405_LOGW("hi6405_adcl2_virtbtn_ir_power_event 1");
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL19_REG, 0x1);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL39_REG,
			0x1<<HI6405_DSPIF8_DIN_SEL_OFFSET, 0x1<<HI6405_DSPIF8_DIN_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL39_REG,
			0x3<<HI6405_DSPIF9_DIN_SEL_OFFSET, 0x0<<HI6405_DSPIF9_DIN_SEL_OFFSET);
		hi64xx_update_bits(codec, HI6405_ADC2L_CTRL0_REG,
			1<<HI6405_ADC2L_HPF_BYPASS_EN_OFFSET | 1<<HI6405_ADC2L_IR2D_BYPASS_EN_OFFSET,
			1<<HI6405_ADC2L_HPF_BYPASS_EN_OFFSET | 1<<HI6405_ADC2L_IR2D_BYPASS_EN_OFFSET);
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL11_REG, 0x1);
		hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			1<<HI6405_CLK_ADC2L_EN_OFFSET, 1<<HI6405_CLK_ADC2L_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET, 1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		HI6405_LOGW("hi6405_adcl2_virtbtn_ir_power_event 0");
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL11_REG, 0x0);
		hi64xx_update_bits(codec, HI6405_ADC2L_CTRL0_REG,
			1<<HI6405_ADC2L_HPF_BYPASS_EN_OFFSET | 1<<HI6405_ADC2L_IR2D_BYPASS_EN_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL39_REG,
			0x1<<HI6405_DSPIF8_DIN_SEL_OFFSET, 0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL39_REG,
			0x3<<HI6405_DSPIF9_DIN_SEL_OFFSET, 0x0<<HI6405_DSPIF9_DIN_SEL_OFFSET);
		snd_soc_write(codec, HI6405_SC_CODEC_MUX_CTRL19_REG, 0x0);
		hi64xx_update_bits(codec, HI6405_DAC_DP_CLK_EN_2_REG,
			1<<HI6405_ADC2L_PGA_CLK_EN_OFFSET, 0);
		hi64xx_update_bits(codec, HI6405_I2S_DSPIF_CLK_EN_REG,
			1<<HI6405_CLK_ADC2L_EN_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}

static int hi6405_soundtrigger_u5u6_power_event(struct snd_soc_codec *codec, slimbus_track_param_t *params, int event)
{
	int ret = 0;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_up_src_pre_pmu(codec, 2, SAMPLE_RATE_16K, SRC_MODE_6);
		hi6405_u56_select_pga(codec);
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_SOUND_TRIGGER, params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate sound trigger err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* slimbus soundtrigger deactive */
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_SOUND_TRIGGER, NULL);
		if (ret) {
			HI6405_LOGE("slimbus_track_deactivate sound trigger err\n");
		}
		hi6405_u56_select_dspif(codec);
		hi6405_up_src_post_pmu(codec, 2);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_soundtrigger_onemic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->soundtrigger_params.channels = 1;

	return hi6405_soundtrigger_u5u6_power_event(codec, &platform_data->soundtrigger_params, event);
}

static int hi6405_soundtrigger_dualmic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->soundtrigger_params.channels = 2;

	return hi6405_soundtrigger_u5u6_power_event(codec, &platform_data->soundtrigger_params, event);
}

static int hi6405_soundtrigger_4mic_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *platform_data;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	platform_data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == platform_data);

	platform_data->soundtrigger_params.channels = 4;

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		hi6405_up_src_pre_pmu(codec, 4, SAMPLE_RATE_16K, SRC_MODE_6);
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET, 0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET);
		hi6405_u56_select_pga(codec);
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_SOUND_TRIGGER, &platform_data->soundtrigger_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate sound trigger err \n");
		}
		hi64xx_update_bits(codec, HI6405_S1_MIXER_EQ_CLK_EN_REG,
			0x1<<HI6405_RST_5MIC_S3_ACCESS_IRQ_OFFSET, 0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* slimbus soundtrigger deactive */
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_SOUND_TRIGGER, NULL);
		if (ret) {
			HI6405_LOGE("slimbus_track_deactivate sound trigger err \n");
		}
		hi6405_u56_select_dspif(codec);
		hi6405_up_src_post_pmu(codec, 4);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_ec_switch_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* close u7 src clk */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
		1 << HI6405_S4_SPA_R_SRC_CLK_EN_OFFSET, 0x0);

		/* sel u7 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			3 << HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET, 0x0);

		/* config u7 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			7 << HI6405_FS_SLIM_UP7_OFFSET, 0x4),

		/* active ec */
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_ECREF, NULL);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate ec err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_ECREF, NULL);
		if (ret) {
			HI6405_LOGE("slimbus_track_deactivate ec err\n");
		}

		/* sel reg default config */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			7 << HI6405_FS_SLIM_UP7_OFFSET, 0x4),

		/* sel reg default config */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			3 << HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET, 0x0);

		/* sel reg default config */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
		1 << HI6405_S4_SPA_R_SRC_CLK_EN_OFFSET, 0x0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return ret;
}

static int hi6405_i2s2_bluetooth_loop_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_codec *codec;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* s2 func mode  PCM STD */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x7<<HI6405_S2_FUNC_MODE_OFFSET, 2<<HI6405_S2_FUNC_MODE_OFFSET);
		/* s2 direct loop Sdin->Sdout */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x3<<HI6405_S2_DIRECT_LOOP_OFFSET, 1<<HI6405_S2_DIRECT_LOOP_OFFSET);
		/* s2 mater mode */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_MST_SLV_OFFSET, 0<<HI6405_S2_MST_SLV_OFFSET);
		/* s2 if rx en */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_IF_RX_ENA_OFFSET, 1<<HI6405_S2_IF_RX_ENA_OFFSET);
		/* s2 if tx en */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_IF_TX_ENA_OFFSET, 1<<HI6405_S2_IF_TX_ENA_OFFSET);
		/* s2 clk if en */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG, 0x1<<HI6405_S2_CLK_IF_EN_OFFSET, 1<<HI6405_S2_CLK_IF_EN_OFFSET);
		/* s2 freq */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL23_REG, 0x7<<HI6405_FS_S2_OFFSET, 0<<HI6405_FS_S2_OFFSET);
		/* s2 frame mode */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_H_REG, 0x1<<HI6405_S2_FRAME_MODE_OFFSET, 0<<HI6405_S2_FRAME_MODE_OFFSET);
		/* s2_clk_if_txrx_en */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG, 0x1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET, 1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* s2_clk_if_txrx_en */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG, 0x1<<HI6405_S2_CLK_IF_TXRX_EN_OFFSET, 0);
		/* s2 frame mode */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_H_REG, 0x1<<HI6405_S2_FRAME_MODE_OFFSET, 0);
		/* s2 freq */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL23_REG, 0x7<<HI6405_FS_S2_OFFSET, 0);
		/* s2 clk if en */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL26_REG, 0x1<<HI6405_S2_CLK_IF_EN_OFFSET, 0);
		/* s2 if tx en */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_IF_TX_ENA_OFFSET, 0);
		/* s2 if rx en */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_IF_RX_ENA_OFFSET, 0);
		/* s2 mater mode */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x1<<HI6405_S2_MST_SLV_OFFSET, 0);
		/* s2 direct loop Sdin->Sdout */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x3<<HI6405_S2_DIRECT_LOOP_OFFSET, 0);
		/* s2 func mode  PCM STD */
		hi64xx_update_bits(codec, HI6405_SC_S2_IF_L_REG, 0x7<<HI6405_S2_FUNC_MODE_OFFSET, 0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}

	return 0;
}


static int hi6405_moveup_2pa_tdm_iv_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *priv = NULL;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 2;
	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->play_params.channels = 2;
	HI6405_LOGI("power event : %d\n", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* bypass S4 UP bit match */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_O_BITMATCH_BYP_EN_LEN, HI6405_S4_O_BITMATCH_BYP_EN_OFFSET),
			1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET);
		/* close u7 src clk */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_SPA_R_SRC_CLK_EN_LEN, HI6405_S4_SPA_R_SRC_CLK_EN_OFFSET),
			0x0);
		/* sel u7 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP7_DATA_SEL_LEN, HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET);
		/* config u7 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP7_LEN, HI6405_FS_SLIM_UP7_OFFSET),
			0x4<<HI6405_FS_SLIM_UP7_OFFSET),

		/* close u8 src clk */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_SPA_L_SRC_CLK_EN_LEN, HI6405_S4_SPA_L_SRC_CLK_EN_OFFSET),
			0x0);
		/* sel u8 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP8_DATA_SEL_LEN, HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET);
		/* config u8 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP8_LEN, HI6405_FS_SLIM_UP8_OFFSET),
			0x4<<HI6405_FS_SLIM_UP8_OFFSET);

		/* 32bit */
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x3<<HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET);
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);//D5 D6
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D5 D6 err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* sel reg default config */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP7_DATA_SEL_LEN, HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP8_DATA_SEL_LEN, HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x0);
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_2PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D5 D6 err\n");
		}
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return ret;
}

static int hi6405_moveup_4pa_tdm_iv_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *priv = NULL;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	priv->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->pa_iv_params.channels = 4;
	priv->play_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	priv->play_params.channels = 2;
	HI6405_LOGI("power event : %d\n", event);
	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* sel u5 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_UP5_DATA_SEL_LEN, HI6405_SLIM_UP5_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_UP5_DATA_SEL_OFFSET);
		/* config u5 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_5_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP5_LEN, HI6405_FS_SLIM_UP5_OFFSET),
			0x4<<HI6405_FS_SLIM_UP5_OFFSET),

		/* sel u6 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL22_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_UP6_DATA_SEL_LEN, HI6405_SLIM_UP6_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_UP6_DATA_SEL_OFFSET);
		/* config u6 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_5_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP6_LEN, HI6405_FS_SLIM_UP6_OFFSET),
			0x4<<HI6405_FS_SLIM_UP6_OFFSET);

		/* bypass S4 UP bit match */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL1_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_O_BITMATCH_BYP_EN_LEN, HI6405_S4_O_BITMATCH_BYP_EN_OFFSET),
			1<<HI6405_S4_O_BITMATCH_BYP_EN_OFFSET);
		/* close u7 src clk */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_SPA_R_SRC_CLK_EN_LEN, HI6405_S4_SPA_R_SRC_CLK_EN_OFFSET),
			0x0);
		/* sel u7 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP7_DATA_SEL_LEN, HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET);
		/* config u7 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP7_LEN, HI6405_FS_SLIM_UP7_OFFSET),
			0x4<<HI6405_FS_SLIM_UP7_OFFSET),

		/* close u8 src clk */
		hi64xx_update_bits(codec, HI6405_S4_DP_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_SPA_L_SRC_CLK_EN_LEN, HI6405_S4_SPA_L_SRC_CLK_EN_OFFSET),
			0x0);
		/* sel u8 data source */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP8_DATA_SEL_LEN, HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET),
			0x2<<HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET);
		/* config u8 48k */
		hi64xx_update_bits(codec, HI6405_SC_FS_SLIM_CTRL_6_REG,
			HI6405_MASK_ON_BIT(HI6405_FS_SLIM_UP8_LEN, HI6405_FS_SLIM_UP8_OFFSET),
			0x4<<HI6405_FS_SLIM_UP8_OFFSET);

		/* 32bit */
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x3<<HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET);

		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_PLAY, &priv->play_params);//D1 D2
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D1 D2 err\n");
		}
		ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);//D5 D6
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D5 D6 err\n");
		}
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* sel reg default config */
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL25_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_UP5_DATA_SEL_LEN, HI6405_SLIM_UP5_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL22_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_UP6_DATA_SEL_LEN, HI6405_SLIM_UP6_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP7_DATA_SEL_LEN, HI6405_SLIM_SW_UP7_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SLIM_SW_UP8_DATA_SEL_LEN, HI6405_SLIM_SW_UP8_DATA_SEL_OFFSET),
			0x0);
		hi64xx_update_bits(codec, HI6405_SC_S4_IF_H_REG,
			HI6405_MASK_ON_BIT(HI6405_S4_CODEC_IO_WORDLENGTH_LEN, HI6405_S4_CODEC_IO_WORDLENGTH_OFFSET),
			0x0);
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_4PA_IV, &priv->pa_iv_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate 4pa iv err\n");
		}
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_VOICE_DOWN, &priv->play_params);//D5 D6
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D5 D6 err\n");
		}
		ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405,
			SLIMBUS_6405_TRACK_AUDIO_PLAY, &priv->play_params);
		if (ret) {
			HI6405_LOGE("slimbus_track_activate D1 D2 err\n");
		}
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return ret;
}

static int hi6405_tdm_audio_pa_down_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *priv = NULL;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	HI6405_LOGI("power event : %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dn1 dn2,it is defalut value*/
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SPA_OL_SRC_DIN_SEL_LEN, HI6405_SPA_OL_SRC_DIN_SEL_OFFSET) |
			HI6405_MASK_ON_BIT(HI6405_SPA_OR_SRC_DIN_SEL_LEN, HI6405_SPA_OR_SRC_DIN_SEL_OFFSET),
			0x0);
		break;
	case SND_SOC_DAPM_POST_PMD:
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return ret;

}

static int hi6405_tdm_audio_pa_down_loop_power_event(struct snd_soc_dapm_widget *w,
			struct snd_kcontrol *kcontrol, int event)
{
	int ret = 0;
	struct snd_soc_codec *codec;
	struct hi6405_platform_data *priv = NULL;

	codec = snd_soc_dapm_to_codec(w->dapm);
	WARN_ON(NULL == codec);
	priv = snd_soc_codec_get_drvdata(codec);
	WARN_ON(!priv);

	HI6405_LOGI("power event : %d\n", event);

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* config SPA_OL/OR(if7) select dspif out put*/
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SPA_OL_SRC_DIN_SEL_LEN, HI6405_SPA_OL_SRC_DIN_SEL_OFFSET) |
			HI6405_MASK_ON_BIT(HI6405_SPA_OR_SRC_DIN_SEL_LEN, HI6405_SPA_OR_SRC_DIN_SEL_OFFSET),
			0x2<<HI6405_SPA_OR_SRC_DIN_SEL_OFFSET|0x2<<HI6405_SPA_OL_SRC_DIN_SEL_OFFSET);
		/* dspif7_tdm_trans clk on*/
		hi64xx_update_bits(codec, HI6405_DMIC_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_DSPIF7_TDM_TRANS_CLK_EN_LEN, HI6405_DSPIF7_TDM_TRANS_CLK_EN_OFFSET),
			0x1<<HI6405_DSPIF7_TDM_TRANS_CLK_EN_OFFSET);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* config SPA_OL/OR(if7) ,select dn1 dn2,it is defalut value*/
		hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
			HI6405_MASK_ON_BIT(HI6405_SPA_OL_SRC_DIN_SEL_LEN, HI6405_SPA_OL_SRC_DIN_SEL_OFFSET) |
			HI6405_MASK_ON_BIT(HI6405_SPA_OR_SRC_DIN_SEL_LEN, HI6405_SPA_OR_SRC_DIN_SEL_OFFSET),
			0x0);
		/* dspif7_tdm_trans clk close*/
		hi64xx_update_bits(codec, HI6405_DMIC_CLK_EN_REG,
			HI6405_MASK_ON_BIT(HI6405_DSPIF7_TDM_TRANS_CLK_EN_LEN, HI6405_DSPIF7_TDM_TRANS_CLK_EN_OFFSET),
			0x0);
		break;
	default :
		HI6405_LOGW("power event err : %d", event);
		break;
	}
	return ret;

}

static const char *hi6405_hsmic_pga_mux_texts[] = {
	"reserve0",
	"HSMIC",
	"HSMIC_CPLS",
	"reserve3",
	"MAINMIC",
	"reserve5",
	"reserve6",
	"reserve7",
	"MAINMIC_CPLS",
};

static const struct soc_enum hi6405_hsmic_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_ANA_HSMIC_MUX_AND_PGA, HI6405_ANA_HSMIC_MUX_BIT,
		ARRAY_SIZE(hi6405_hsmic_pga_mux_texts), hi6405_hsmic_pga_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_hsmic_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_hsmic_pga_mux_enum);

static const char *hi6405_auxmic_pga_mux_texts[] = {
	"reserve0",
	"AUXMIC",
	"AUXMIC_CPLS",
	"reserve3",
	"MAINMIC",
	"reserve5",
	"reserve6",
	"reserve7",
	"MAINMIC_CPLS",
};

static const struct soc_enum hi6405_auxmic_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_ANA_AUXMIC_MUX_AND_PGA, HI6405_ANA_AUXMIC_MUX_BIT,
		ARRAY_SIZE(hi6405_auxmic_pga_mux_texts), hi6405_auxmic_pga_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_auxmic_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_auxmic_pga_mux_enum);

static const char *hi6405_mic3_pga_texts[] = {
	"reserve0",
	"MIC3",
	"MIC3_CPLS",
	"reserve3",
	"MIC4",
	"reserve5",
	"reserve6",
	"reserve7",
	"MIC4_CPLS",
};

static const struct soc_enum hi6405_mic3_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_ANA_MIC3_MUX_AND_PGA, HI6405_ANA_MIC3_MUX_BIT,
		ARRAY_SIZE(hi6405_mic3_pga_texts), hi6405_mic3_pga_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic3_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic3_pga_mux_enum);

static const char *hi6405_mic4_pga_texts[] = {
	"reserve0",
	"MIC4",
	"MIC4_CPLS",
	"reserve3",
	"MIC3",
	"reserve5",
	"reserve6",
	"reserve7",
	"MIC3_CPLS",
};

static const struct soc_enum hi6405_mic4_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_ANA_MIC4_MUX_AND_PGA, HI6405_ANA_MIC4_MUX_BIT,
		ARRAY_SIZE(hi6405_mic4_pga_texts), hi6405_mic4_pga_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic4_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic4_pga_mux_enum);

static const char *hi6405_mic5_pga_texts[] = {
	"IR",
	"MIC5",
	"MIC5_CPLS",
};

static const struct soc_enum hi6405_mic5_pga_mux_enum =
	SOC_ENUM_SINGLE(HI6405_ANA_MIC5_MUX_AND_PGA, HI6405_ANA_MIC5_MUX_BIT,
		ARRAY_SIZE(hi6405_mic5_pga_texts), hi6405_mic5_pga_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic5_pga_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic5_pga_mux_enum);

static const struct snd_kcontrol_new hi6405_mad_pga_mixer_controls[] = {
	SOC_DAPM_SINGLE("MAINMIC_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_MAINMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("AUXMIC_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_AUXMIC_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC3_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_MIC3_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC4_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_MIC4_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("MIC5_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_MIC5_SEL_BIT, 1, 0),
	SOC_DAPM_SINGLE("HSMIC_ENABLE",
		HI6405_ANA_MAD_PGA_MIXER, HI6405_ANA_MAD_PGA_HSMIC_SEL_BIT, 1, 0),
};

static const char *hi6405_adc_mux_texts[] = {
	"ADC0",
	"ADC1",
	"ADC2",
	"ADC3",
	"ADC4",
	"DMIC0L",
	"DMIC0R",
	"DMIC1L",
	"DMIC1R",
	"ADC_MAD",
};

static const struct soc_enum hi6405_adcl0_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL15_REG, HI6405_ADC0L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_adc_mux_texts), hi6405_adc_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_adcl0_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_adcl0_mux_enum);

static const struct soc_enum hi6405_adcr0_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL15_REG, HI6405_ADC0R_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_adc_mux_texts), hi6405_adc_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_adcr0_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_adcr0_mux_enum);

static const struct soc_enum hi6405_adcl1_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL16_REG, HI6405_ADC1L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_adc_mux_texts), hi6405_adc_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_adcl1_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_adcl1_mux_enum);

static const struct soc_enum hi6405_adcr1_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL16_REG, HI6405_ADC1R_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_adc_mux_texts), hi6405_adc_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_adcr1_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_adcr1_mux_enum);

static const struct soc_enum hi6405_adcl2_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL17_REG, HI6405_ADC2L_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_adc_mux_texts), hi6405_adc_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_adcl2_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_adcl2_mux_enum);

static const char *hi6405_mic1_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S2_L",
	"S3_L",
	"S4_L",
	"ADCR0",
	"ADCR1",
	"ADCL2",
};

static const struct soc_enum hi6405_mic1_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL29_REG, HI6405_MIC_1_SEL_OFFSET,
		ARRAY_SIZE(hi6405_mic1_mux_texts), hi6405_mic1_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic1_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic1_mux_enum);

static const char *hi6405_mic2_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S2_R",
	"S3_R",
	"S4_R",
	"ADCL0",
	"ADCL1",
	"ADCL2",
};

static const struct soc_enum hi6405_mic2_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL29_REG, HI6405_MIC_2_SEL_OFFSET,
		ARRAY_SIZE(hi6405_mic2_mux_texts), hi6405_mic2_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic2_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic2_mux_enum);

static const char *hi6405_mic3_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S1_L",
	"S2_L",
	"S4_L",
	"ADCR0",
	"ADCR1",
	"ADCL2",
};

static const struct soc_enum hi6405_mic3_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL28_REG, HI6405_MIC_3_SEL_OFFSET,
		ARRAY_SIZE(hi6405_mic3_mux_texts), hi6405_mic3_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic3_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic3_mux_enum);

static const char *hi6405_mic4_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S1_R",
	"S2_R",
	"S4_R",
	"ADCL0",
	"ADCL1",
	"ADCL2",
};

static const struct soc_enum hi6405_mic4_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL28_REG, HI6405_MIC_4_SEL_OFFSET,
		ARRAY_SIZE(hi6405_mic4_mux_texts), hi6405_mic4_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic4_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic4_mux_enum);

static const char *hi6405_mic5_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"ADCR0",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum hi6405_mic5_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL27_REG, HI6405_MIC_5_SEL_OFFSET,
		ARRAY_SIZE(hi6405_mic5_mux_texts), hi6405_mic5_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_mic5_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_mic5_mux_enum);

static const char *hi6405_sidetone_mux_texts[] = {
	"S1_L",
	"S2_L",
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2"
};

static const struct soc_enum hi6405_sidetone_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL14_REG, HI6405_SIDE_SRC_DIN_SEL_OFFSET,
		ARRAY_SIZE(hi6405_sidetone_mux_texts), hi6405_sidetone_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_sidetone_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_sidetone_mux_enum);

static const char *hi6405_m1_l_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_S",
	"DACL_48",
	"S1_L",
	"S2_L",
	"S3_L",
	"ADCL2",
};

static const struct soc_enum hi6405_m1_l_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL32_REG, HI6405_M1L_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_m1_l_mux_texts), hi6405_m1_l_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_m1_l_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_m1_l_mux_enum);

static const char *hi6405_m1_r_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_S",
	"DACR_48",
	"S1_R",
	"S2_R",
	"S3_R",
	"ADCL2",
};

static const struct soc_enum hi6405_m1_r_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL32_REG, HI6405_M1R_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_m1_r_mux_texts), hi6405_m1_r_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_m1_r_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_m1_r_mux_enum);

static const char *hi6405_bt_l_mux_texts[] = {
	"ADCL0",
	"ADCL1",
	"DACL_48",
	"S1_L",
	"S3_L",
	"S4_L",
	"ADCL2",
};

static const struct soc_enum hi6405_bt_l_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL31_REG, HI6405_BTL_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_bt_l_mux_texts), hi6405_bt_l_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_bt_l_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_bt_l_mux_enum);

static const char *hi6405_bt_r_mux_texts[] = {
	"ADCR0",
	"ADCR1",
	"DACR_48",
	"S1_R",
	"S3_R",
	"S4_R",
	"ADCL2",
};

static const struct soc_enum hi6405_bt_r_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL31_REG, HI6405_BTR_DATA_SEL_OFFSET,
		ARRAY_SIZE(hi6405_bt_r_mux_texts), hi6405_bt_r_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_bt_r_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_bt_r_mux_enum);

static const char *hi6405_us_r1_mux_texts[] = {
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum hi6405_us_r1_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL30_REG, HI6405_ULTRASONIC_RX_MUX1_SEL_OFFSET,
		ARRAY_SIZE(hi6405_us_r1_mux_texts), hi6405_us_r1_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_us_r1_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_us_r1_mux_enum);

static const char *hi6405_us_r2_mux_texts[] = {
	"ADCL0",
	"ADCR0",
	"ADCL1",
	"ADCR1",
	"ADCL2",
	"DACL_S",
	"DACR_S",
};

static const struct soc_enum hi6405_us_r2_mux_enum =
	SOC_ENUM_SINGLE(HI6405_SC_CODEC_MUX_CTRL30_REG, HI6405_ULTRASONIC_RX_MUX2_SEL_OFFSET,
		ARRAY_SIZE(hi6405_us_r2_mux_texts), hi6405_us_r2_mux_texts);

static const struct snd_kcontrol_new hi6405_dapm_us_r2_mux_controls =
	SOC_DAPM_ENUM("Mux", hi6405_us_r2_mux_enum);

static const struct snd_kcontrol_new hi6405_dapm_mad_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_MAD_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s1_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_S1_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s1_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_S1_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s3_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_S3_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_s3_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_S3_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_u3_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_U3_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_u4_or_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_U4_OR_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_u5_ol_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_U5_OL_SRC_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_us_r1_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_US_R1_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_us_r2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_US_R2_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_audioup_2mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_AUDIOUP_2MIC_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_audioup_4mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_AUDIOUP_4MIC_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_audioup_5mic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_AUDIOUP_5MIC_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_voice8k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_VOICE8K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_voice16k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_VOICE16K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_voice32k_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_VOICE32K_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_ir_env_study_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_IR_ENV_STUDY_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_soundtrigger_onemic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_SOUNDTRIGGER_ONE_MIC_EN_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_soundtrigger_dualmic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_SOUNDTRIGGER_DUAL_MIC_EN_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_soundtrigger_multimic_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_SOUNDTRIGGER_MULTI_MIC_EN_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_auxmic_hsmicbias_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_AUXMIC_HSMICBIAS_EN_MM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_auxmic_micbias2_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_AUXMIC_MICBIAS2_EN_MM_BIT, 1, 0);
/* I2S2 bluetooth LOOP ENABLE*/
static const struct snd_kcontrol_new hi6405_dapm_i2s2_bluetooth_loop_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_I2S2_BLUETOOTH_LOOP_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_moveup_2pa_tdm_iv_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_MOVEUP_2PA_TDM_IV_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_moveup_4pa_tdm_iv_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_MOVEUP_4PA_TDM_IV_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_tdm_audio_pa_down_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_TDM_AUDIO_PA_TDM_DOWN_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_tdm_audio_pa_down_loop_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_TDM_AUDIO_PA_TDM_DN_LOOP_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_fm_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_DOWN_REG, HI6405_FM_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_virtbtn_active_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_VIRTUAL_BTN_ACTIVE_BIT, 1, 0);
static const struct snd_kcontrol_new hi6405_dapm_virtbtn_passive_switch_controls =
	SOC_DAPM_SINGLE("ENABLE",
		HI6405_VIRTUAL_UP_REG, HI6405_VIRTUAL_BTN_PASSIVE_BIT, 1, 0);

#define HI6405_S1_CFG_KCONTROLS \
	SOC_SINGLE("S1_IF_I2S_FS", \
		HI6405_SC_CODEC_MUX_CTRL23_REG, HI6405_FS_S1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_S1_LEN), 0), \
	SOC_SINGLE("S1_FUNC_MODE", \
		HI6405_SC_S1_IF_L_REG, HI6405_S1_FUNC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S1_FRAME_MODE", \
		HI6405_SC_S1_IF_H_REG, HI6405_S1_FRAME_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S1_RX_CLK_SEL", \
		HI6405_SC_S1_IF_H_REG, HI6405_S1_RX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S1_TX_CLK_SEL", \
		HI6405_SC_S1_IF_H_REG, HI6405_S1_TX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_TX_CLK_SEL_LEN), 0), \

#define HI6405_S2_CFG_KCONTROLS \
	SOC_SINGLE("S2_IF_I2S_FS", \
		HI6405_SC_CODEC_MUX_CTRL23_REG, HI6405_FS_S2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_S2_LEN), 0), \
	SOC_SINGLE("S2_FUNC_MODE", \
		HI6405_SC_S2_IF_L_REG, HI6405_S2_FUNC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S2_DIRECT_MODE", \
		HI6405_SC_S2_IF_L_REG, HI6405_S2_DIRECT_LOOP_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("S2_FRAME_MODE", \
		HI6405_SC_S2_IF_H_REG, HI6405_S2_FRAME_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S2_RX_CLK_SEL", \
		HI6405_SC_S2_IF_H_REG, HI6405_S2_RX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S2_TX_CLK_SEL", \
		HI6405_SC_S2_IF_H_REG, HI6405_S2_TX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S2_FS_TDM", \
		HI6405_SC_CODEC_MUX_CTRL24_REG, HI6405_S2_FS_TDM_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_FS_TDM_LEN), 0), \
	SOC_SINGLE("S2_TDM_FRAME_MODE", \
		HI6405_S2_TDM_CTRL0_REG, HI6405_S2_TDM_FRAME_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_TDM_FRAME_MODE_LEN), 0), \

#define HI6405_S4_CFG_KCONTROLS \
	SOC_SINGLE("S4_IF_I2S_FS", \
		HI6405_SC_CODEC_MUX_CTRL24_REG, HI6405_FS_S4_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_S4_LEN), 0), \
	SOC_SINGLE("S4_FUNC_MODE", \
		HI6405_SC_S4_IF_L_REG, HI6405_S4_FUNC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_FUNC_MODE_LEN), 0), \
	SOC_SINGLE("S4_FRAME_MODE", \
		HI6405_SC_S4_IF_H_REG, HI6405_S4_FRAME_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_FRAME_MODE_LEN), 0), \
	SOC_SINGLE("S4_DIRECT_MODE", \
		HI6405_SC_S4_IF_L_REG, HI6405_S4_DIRECT_LOOP_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_DIRECT_LOOP_LEN), 0), \
	SOC_SINGLE("S4_RX_CLK_SEL", \
		HI6405_SC_S4_IF_H_REG, HI6405_S4_RX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TX_CLK_SEL", \
		HI6405_SC_S4_IF_H_REG, HI6405_S4_TX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_IF_EN", \
		HI6405_S4_TDM_CTRL0_REG, HI6405_S4_TDM_IF_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_IF_EN_LEN), 0), \
	SOC_SINGLE("S4_FS_TDM", \
		HI6405_SC_CODEC_MUX_CTRL24_REG, HI6405_S4_FS_TDM_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_FS_TDM_LEN), 0), \
	SOC_SINGLE("S4_TDM_FRAME_MODE", \
		HI6405_S4_TDM_CTRL0_REG, HI6405_S4_TDM_FRAME_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_FRAME_MODE_LEN), 0), \

#define HI6405_ANA_PGA_GAIN_KCONTROLS \
	SOC_SINGLE_TLV("HSMIC_PGA_GAIN", \
		HI6405_ANA_HSMIC_MUX_AND_PGA, HI6405_ANA_HSMIC_PGA_BIT, 15, 0, hsmic_pga_tlv), \
	SOC_SINGLE_TLV("AUXMIC_PGA_GAIN", \
		HI6405_ANA_AUXMIC_MUX_AND_PGA, HI6405_ANA_AUXMIC_PGA_BIT, 15, 0, auxmic_pga_tlv), \
	SOC_SINGLE_TLV("MIC3_PGA_GAIN", \
		HI6405_ANA_MIC3_MUX_AND_PGA, HI6405_ANA_MIC3_PGA_BIT, 15, 0, mic3_pga_tlv), \
	SOC_SINGLE_TLV("MIC4_PGA_GAIN", \
		HI6405_ANA_MIC4_MUX_AND_PGA, HI6405_ANA_MIC4_PGA_BIT, 15, 0, mic4_pga_tlv), \
	SOC_SINGLE_TLV("MIC5_PGA_GAIN", \
		HI6405_ANA_MIC5_MUX_AND_PGA, HI6405_ANA_MIC5_PGA_BIT, 15, 0, mic5_pga_tlv), \
	SOC_SINGLE_TLV("MAD_PGA_GAIN", \
		HI6405_ANA_MAD_PGA, HI6405_ANA_MAD_PGA_BIT, 12, 0, mad_pga_tlv), \
	SOC_SINGLE("EP_GAIN", \
		HI6405_CODEC_ANA_RWREG_050, HI6405_CODEC_ANA_EP_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_CODEC_ANA_EP_GAIN_LEN), 0), \

#define HI6405_DAC_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("DACL_MIXER_S1L_GAIN", \
		HI6405_DACL_MIXER4_CTRL2_REG, HI6405_DACL_MIXER4_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S2L_GAIN", \
		HI6405_DACL_MIXER4_CTRL2_REG, HI6405_DACL_MIXER4_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S3L_GAIN", \
		HI6405_DACL_MIXER4_CTRL2_REG, HI6405_DACL_MIXER4_GAIN3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACL_MIXER_S1R_GAIN", \
		HI6405_DACL_MIXER4_CTRL2_REG, HI6405_DACL_MIXER4_GAIN4_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S1R_GAIN", \
		HI6405_DACR_MIXER4_CTRL2_REG, HI6405_DACR_MIXER4_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S2R_GAIN", \
		HI6405_DACR_MIXER4_CTRL2_REG, HI6405_DACR_MIXER4_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_MDM_GAIN", \
		HI6405_DACR_MIXER4_CTRL2_REG, HI6405_DACR_MIXER4_GAIN3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACR_MIXER_S1L_GAIN", \
		HI6405_DACR_MIXER4_CTRL2_REG, HI6405_DACR_MIXER4_GAIN4_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACL_PRE_MIXER_MUX9_GAIN", \
		HI6405_DACL_PRE_MIXER2_CTRL1_REG, HI6405_DACL_PRE_MIXER2_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_PRE_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_PRE_MIXER_SIDETONE_GAIN", \
		HI6405_DACL_PRE_MIXER2_CTRL1_REG, HI6405_DACL_PRE_MIXER2_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_PRE_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_PRE_MIXER_MUX10_GAIN", \
		HI6405_DACR_PRE_MIXER2_CTRL1_REG, HI6405_DACR_PRE_MIXER2_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_PRE_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_PRE_MIXER_SIDETONE_GAIN", \
		HI6405_DACR_PRE_MIXER2_CTRL1_REG, HI6405_DACR_PRE_MIXER2_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_PRE_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACL_POST_MIXER_DAC_GAIN", \
		HI6405_DACL_POST_MIXER2_CTRL1_REG, HI6405_DACL_POST_MIXER2_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_POST_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACL_POST_MIXER_S1L_GAIN", \
		HI6405_DACL_POST_MIXER2_CTRL1_REG, HI6405_DACL_POST_MIXER2_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_POST_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACR_POST_MIXER_DAC_GAIN", \
		HI6405_DACR_POST_MIXER2_CTRL1_REG, HI6405_DACR_POST_MIXER2_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_POST_MIXER2_GAIN1_LEN), 0), \
	SOC_SINGLE("DACR_POST_MIXER_S1R_GAIN", \
		HI6405_DACR_POST_MIXER2_CTRL1_REG, HI6405_DACR_POST_MIXER2_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_POST_MIXER2_GAIN2_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_UTW_GAIN", \
		HI6405_DACSL_MIXER4_CTRL2_REG, HI6405_DACSL_MIXER4_GAIN1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACSL_MIXER4_GAIN1_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_PGA_GAIN", \
		HI6405_DACSL_MIXER4_CTRL2_REG, HI6405_DACSL_MIXER4_GAIN2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACSL_MIXER4_GAIN2_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_MDM_GAIN", \
		HI6405_DACSL_MIXER4_CTRL2_REG, HI6405_DACSL_MIXER4_GAIN3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACSL_MIXER4_GAIN3_LEN), 0), \
	SOC_SINGLE("DACSL_MIXER_SIDETONE_GAIN", \
		HI6405_DACSL_MIXER4_CTRL2_REG, HI6405_DACSL_MIXER4_GAIN4_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACSL_MIXER4_GAIN4_LEN), 0), \
	SOC_SINGLE("DACL_PRE_PGA_GAIN", \
		HI6405_DACL_PRE_PGA_CTRL1_REG, HI6405_DACL_PRE_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_PRE_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACR_PRE_PGA_GAIN", \
		HI6405_DACR_PRE_PGA_CTRL1_REG, HI6405_DACR_PRE_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_PRE_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACL_POST_PGA_GAIN", \
		HI6405_DACL_POST_PGA_CTRL1_REG, HI6405_DACL_POST_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_POST_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("DACR_POST_PGA_GAIN", \
		HI6405_DACR_POST_PGA_CTRL1_REG, HI6405_DACR_POST_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_POST_PGA_GAIN_LEN), 0), \

#define HI6405_S_1_4_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("S1_IL_PGA_GAIN", \
		HI6405_S1_IL_PGA_CTRL1_REG, HI6405_S1_IL_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S1_IR_PGA_GAIN", \
		HI6405_S1_IR_PGA_CTRL1_REG, HI6405_S1_IR_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_IL_PGA_GAIN", \
		HI6405_S2_IL_PGA_CTRL1_REG, HI6405_S2_IL_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_IR_PGA_GAIN", \
		HI6405_S2_IR_PGA_CTRL1_REG, HI6405_S2_IR_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S3_IL_PGA_GAIN", \
		HI6405_S3_IL_PGA_CTRL1_REG, HI6405_S3_IL_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S3_IR_PGA_GAIN", \
		HI6405_S3_IR_PGA_CTRL1_REG, HI6405_S3_IR_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IR_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_OL_PGA_GAIN", \
		HI6405_S2_OL_PGA_CTRL1_REG, HI6405_S2_OL_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OL_PGA_GAIN_LEN), 0), \
	SOC_SINGLE("S2_OR_PGA_GAIN", \
		HI6405_S2_OR_PGA_CTRL1_REG, HI6405_S2_OR_PGA_GAIN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OR_PGA_GAIN_LEN), 0), \

#define HI6405_ADC_BOOST_KCONTROLS \
	SOC_SINGLE("HSMIC_ADC_BOOST", \
		HI6405_CODEC_ANA_RWREG_020, HI6405_CODEC_ANA_HSMIC_ADC_BOOST, 3, 0), \
	SOC_SINGLE("AUXMIC_ADC_BOOST", \
		HI6405_CODEC_ANA_RWREG_024, HI6405_CODEC_ANA_AUXMIC_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC3_ADC_BOOST", \
		HI6405_CODEC_ANA_RWREG_028, HI6405_CODEC_ANA_MIC3_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC4_ADC_BOOST", \
		HI6405_CODEC_ANA_RWREG_032, HI6405_CODEC_ANA_MIC4_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MIC5_ADC_BOOST", \
		HI6405_CODEC_ANA_RWREG_036, HI6405_CODEC_ANA_MIC5_ADC_BOOST, 3, 0), \
	SOC_SINGLE("MAD_ADC_BOOST", \
		HI6405_ANA_MAD_PGA, HI6405_ANA_MAD_BOOST_18_OFFSET, 1, 0), \

#define HI6405_SRC_KCONTROLS \
	SOC_SINGLE("S1_IL_SRC_EN", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_IL_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IL_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_IR_SRC_EN", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_IR_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IR_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_EN", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_MIC1_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_MIC1_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_EN", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_MIC2_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_MIC2_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_MODE", \
		HI6405_MIC1_SRC_CTRL_REG, HI6405_MIC1_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_MIC1_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_MODE", \
		HI6405_MIC2_SRC_CTRL_REG, HI6405_MIC2_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_MIC2_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S1_MIC1_SRC_FS", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_MIC1_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_MIC1_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S1_MIC2_SRC_FS", \
		HI6405_S1_DP_CLK_EN_REG, HI6405_S1_MIC2_SRC_CLK_EN_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_MIC2_SRC_CLK_EN_LEN), 0), \
	SOC_SINGLE("S2_IL_SRC_MODE", \
		HI6405_S2_IL_SRC_CTRL_REG, HI6405_S2_IL_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_IR_SRC_MODE", \
		HI6405_S2_IR_SRC_CTRL_REG, HI6405_S2_IR_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_I_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL0_REG, HI6405_S2_I_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_I_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_IL_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL0_REG, HI6405_S2_IL_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_IR_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL0_REG, HI6405_S2_IR_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IR_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OL_SRC_MODE", \
		HI6405_S2_OL_SRC_CTRL_REG, HI6405_S2_OL_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_OR_SRC_MODE", \
		HI6405_S2_OR_SRC_CTRL_REG, HI6405_S2_OR_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S2_O_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL6_REG, HI6405_S2_O_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_O_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OL_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL6_REG, HI6405_S2_OL_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S2_OR_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL6_REG, HI6405_S2_OR_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IL_SRC_MODE", \
		HI6405_S3_IL_SRC_CTRL_REG, HI6405_S3_IL_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_IR_SRC_MODE", \
		HI6405_S2_IR_SRC_CTRL_REG, HI6405_S3_IR_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_I_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL1_REG, HI6405_S3_I_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_I_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IL_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL1_REG, HI6405_S3_IL_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_IR_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL1_REG, HI6405_S3_IR_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IR_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OL_SRC_MODE", \
		HI6405_S3_OL_SRC_CTRL_REG, HI6405_S3_OL_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_OL_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_OR_SRC_MODE", \
		HI6405_S3_OR_SRC_CTRL_REG, HI6405_S3_OR_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_OR_SRC_MODE_LEN), 0), \
	SOC_SINGLE("S3_O_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL7_REG, HI6405_S3_O_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_O_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OL_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL7_REG, HI6405_S3_OL_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("S3_OR_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL7_REG, HI6405_S3_OR_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OL_SRC_MODE", \
		HI6405_SPA_OL_SRC_CTRL_REG, HI6405_SPA_OL_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OL_SRC_MODE_OFFSET), 0), \
	SOC_SINGLE("SPA_OR_SRC_MODE", \
		HI6405_SPA_OR_SRC_CTRL_REG, HI6405_SPA_OR_SRC_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OR_SRC_MODE_OFFSET), 0), \
	SOC_SINGLE("SPA_OL_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL9_REG, HI6405_SPA_OL_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OL_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OR_SRC_IN_FS", \
		HI6405_SRC_VLD_CTRL10_REG, HI6405_SPA_OR_SRC_DIN_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OR_SRC_DIN_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OL_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL9_REG, HI6405_SPA_OL_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OL_SRC_DOUT_VLD_SEL_LEN), 0), \
	SOC_SINGLE("SPA_OR_SRC_OUT_FS", \
		HI6405_SRC_VLD_CTRL10_REG, HI6405_SPA_OR_SRC_DOUT_VLD_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_SPA_OR_SRC_DOUT_VLD_SEL_LEN), 0), \

#define HI6405_PGA_BYPASS_KCONTROLS \
	SOC_SINGLE("S1_IL_PGA_BYPASS", \
		HI6405_S1_IL_PGA_CTRL0_REG, HI6405_S1_IL_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S1_IR_PGA_BYPASS", \
		HI6405_S1_IR_PGA_CTRL0_REG, HI6405_S1_IR_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S1_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_IL_PGA_BYPASS", \
		HI6405_S2_IL_PGA_CTRL0_REG, HI6405_S2_IL_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_IR_PGA_BYPASS", \
		HI6405_S2_IR_PGA_CTRL0_REG, HI6405_S2_IR_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_OL_PGA_BYPASS", \
		HI6405_S2_OL_PGA_CTRL0_REG, HI6405_S2_OL_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S2_OR_PGA_BYPASS", \
		HI6405_S2_OR_PGA_CTRL0_REG, HI6405_S2_OR_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S2_OR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S3_IL_PGA_BYPASS", \
		HI6405_S3_IL_PGA_CTRL0_REG, HI6405_S3_IL_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IL_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("S3_IR_PGA_BYPASS", \
		HI6405_S3_IR_PGA_CTRL0_REG, HI6405_S3_IR_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S3_IR_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACL_PRE_PGA_BYPASS", \
		HI6405_DACL_PRE_PGA_CTRL0_REG, HI6405_DACL_PRE_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_PRE_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACR_PRE_PGA_BYPASS", \
		HI6405_DACR_PRE_PGA_CTRL0_REG, HI6405_DACR_PRE_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_PRE_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACL_POST_PGA_BYPASS", \
		HI6405_DACL_POST_PGA_CTRL0_REG, HI6405_DACL_POST_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACL_POST_PGA_BYPASS_LEN), 0), \
	SOC_SINGLE("DACR_POST_PGA_BYPASS", \
		HI6405_DACR_POST_PGA_CTRL0_REG, HI6405_DACR_POST_PGA_BYPASS_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_DACR_POST_PGA_BYPASS_LEN), 0), \

#define HI6405_SIDETONE_PGA_GAIN_KCONTROLS \
	SOC_SINGLE("SIDETONE_SRC_CLK_EN", \
		HI6405_DAC_DP_CLK_EN_2_REG, HI6405_SIDE_SRC_CLK_EN_OFFSET, 1, 0), \
	SOC_SINGLE("SIDETONE_PGA_CLK_EN", \
		HI6405_DAC_MIXER_CLK_EN_REG, HI6405_SIDE_PGA_CLK_EN_OFFSET, 1, 0), \
	SOC_SINGLE("SIDETONE_PGA_GAIN", \
		HI6405_SIDE_PGA_CTRL1_REG, HI6405_SIDE_PGA_GAIN_OFFSET, 0xff, 0), \

#ifdef CONFIG_HAC_SUPPORT
#define HI6405_GPIO_HAC_KCONTROLS \
	SOC_ENUM_EXT("HAC", hac_switch_enum[0], hi6405_hac_status_get, hi6405_hac_status_set),\

#endif

#define HI6405_SLIMBUS_DN_PORT_FS_KCONTROLS \
	SOC_SINGLE("SLIMBUS_DN3_FS", \
		HI6405_SC_FS_SLIM_CTRL_1_REG, HI6405_FS_SLIM_DN3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_SLIM_DN3_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN4_FS", \
		HI6405_SC_FS_SLIM_CTRL_1_REG, HI6405_FS_SLIM_DN4_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_SLIM_DN4_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN5_FS", \
		HI6405_SC_FS_SLIM_CTRL_2_REG, HI6405_FS_SLIM_DN5_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_SLIM_DN5_LEN), 0), \
	SOC_SINGLE("SLIMBUS_DN6_FS", \
		HI6405_SC_FS_SLIM_CTRL_2_REG, HI6405_FS_SLIM_DN6_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_FS_SLIM_DN6_LEN), 0), \

#define HI6405_S4_TDM_CFG_KCONTROLS \
	SOC_SINGLE("S4_TDM_TX_CH0_SEL", \
		HI6405_S4_TDM_CTRL1_REG, HI6405_S4_TDM_TX_CH0_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_TX_CH0_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH1_SEL", \
		HI6405_S4_TDM_CTRL1_REG, HI6405_S4_TDM_TX_CH1_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_TX_CH1_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH2_SEL", \
		HI6405_S4_TDM_CTRL1_REG, HI6405_S4_TDM_TX_CH2_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_TX_CH2_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CH3_SEL", \
		HI6405_S4_TDM_CTRL1_REG, HI6405_S4_TDM_TX_CH3_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_TX_CH3_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_CLK_SEL", \
		HI6405_S4_TDM_CTRL2_REG, HI6405_S4_TDM_RX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_TX_CLK_SEL", \
		HI6405_S4_TDM_CTRL2_REG, HI6405_S4_TDM_TX_CLK_SEL_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_TX_CLK_SEL_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I0_SEL", \
		HI6405_S4_TDM_CTRL2_REG, HI6405_S4_TDM_RX_SLOT_SEL_I0_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_I0_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V0_SEL", \
		HI6405_S4_TDM_CTRL2_REG, HI6405_S4_TDM_RX_SLOT_SEL_V0_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_V0_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I1_SEL", \
		HI6405_S4_TDM_CTRL3_REG, HI6405_S4_TDM_RX_SLOT_SEL_I1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_I1_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V1_SEL", \
		HI6405_S4_TDM_CTRL3_REG, HI6405_S4_TDM_RX_SLOT_SEL_V1_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_V1_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I2_SEL", \
		HI6405_S4_TDM_CTRL4_REG, HI6405_S4_TDM_RX_SLOT_SEL_I2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_I2_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V2_SEL", \
		HI6405_S4_TDM_CTRL4_REG, HI6405_S4_TDM_RX_SLOT_SEL_V2_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_V2_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_I3_SEL", \
		HI6405_S4_TDM_CTRL5_REG, HI6405_S4_TDM_RX_SLOT_SEL_I3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_I3_LEN), 0), \
	SOC_SINGLE("S4_TDM_RX_SLOT_V3_SEL", \
		HI6405_S4_TDM_CTRL5_REG, HI6405_S4_TDM_RX_SLOT_SEL_V3_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_RX_SLOT_SEL_V3_LEN), 0), \
	SOC_SINGLE("S4_I2S_TDM_MODE", \
		HI6405_S4_TDM_CTRL6_REG, HI6405_S4_I2S_TDM_MODE_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_I2S_TDM_MODE_LEN), 0), \
	SOC_SINGLE("S4_TDM_MST_SLV_SEL", \
		HI6405_S4_TDM_CTRL6_REG, HI6405_S4_TDM_MST_SLV_OFFSET, HI6405_MAX_VAL_ON_BIT(HI6405_S4_TDM_MST_SLV_LEN), 0), \


static const struct snd_kcontrol_new hi6405_snd_controls[] = {
	HI6405_S1_CFG_KCONTROLS
	HI6405_S2_CFG_KCONTROLS
	HI6405_S4_CFG_KCONTROLS
	HI6405_SRC_KCONTROLS
	HI6405_ANA_PGA_GAIN_KCONTROLS
	HI6405_DAC_PGA_GAIN_KCONTROLS
	HI6405_S_1_4_PGA_GAIN_KCONTROLS
	HI6405_ADC_BOOST_KCONTROLS
	HI6405_PGA_BYPASS_KCONTROLS
	HI6405_SIDETONE_PGA_GAIN_KCONTROLS
#ifdef CONFIG_HAC_SUPPORT
	HI6405_GPIO_HAC_KCONTROLS
#endif
	HI6405_SLIMBUS_DN_PORT_FS_KCONTROLS
	HI6405_S4_TDM_CFG_KCONTROLS
};

#define HI6405_INPUT_WIDGET \
	SND_SOC_DAPM_INPUT("MAINMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("AUXMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC3_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC4_INPUT"), \
	SND_SOC_DAPM_INPUT("MIC5_INPUT"), \
	SND_SOC_DAPM_INPUT("IR_STUDY_INPUT"), \
	SND_SOC_DAPM_INPUT("HSMIC_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIOUP_INPUT"), \
	SND_SOC_DAPM_INPUT("VOICE_INPUT"), \
	SND_SOC_DAPM_INPUT("DSD_L_INPUT"), \
	SND_SOC_DAPM_INPUT("DSD_R_INPUT"), \
	SND_SOC_DAPM_INPUT("D1_D2_INPUT"), \
	SND_SOC_DAPM_INPUT("D3_D4_INPUT"), \
	SND_SOC_DAPM_INPUT("D5_D6_INPUT"), \
	SND_SOC_DAPM_INPUT("S1_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("S2_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("S4_RX_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC1_INPUT"), \
	SND_SOC_DAPM_INPUT("DMIC2_INPUT"), \
	SND_SOC_DAPM_INPUT("IR_TX_INPUT"), \
	SND_SOC_DAPM_INPUT("AUDIO_DOWN_INPUT"), \
	SND_SOC_DAPM_INPUT("U7_EC_INPUT"), \
	SND_SOC_DAPM_INPUT("PLL_TEST_INPUT"), \
	SND_SOC_DAPM_INPUT("HP_HIGHLEVEL_INPUT"), \
	SND_SOC_DAPM_INPUT("I2S2_BLUETOOTH_LOOP_INPUT"),\
	SND_SOC_DAPM_INPUT("TDM_AUDIO_PA_DOWN_INPUT"),\
	SND_SOC_DAPM_INPUT("FM_INPUT"),\
	SND_SOC_DAPM_INPUT("HP_CONCURRENCY_INPUT"),\

#define HI6405_OUTPUT_WIDGET \
	SND_SOC_DAPM_OUTPUT("HP_L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("HP_R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("EP_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DSD_L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("DSD_R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S1_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4_TX_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IR_LED_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIO_DOWN_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U7_EC_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("PLL_TEST_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("HP_HIGHLEVEL_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IV_DSPIF_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U1_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U2_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U3_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U4_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U5_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U6_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U7_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U8_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U9_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("U10_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S2R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4L_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("S4R_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("MAD_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VIRTUAL_BTN_ACTIVE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VIRTUAL_BTN_PASSIVE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("IR_STUDY_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("AUDIOUP_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("VOICE_OUTPUT"), \
	SND_SOC_DAPM_OUTPUT("I2S2_BLUETOOTH_LOOP_OUTPUT"),\
	SND_SOC_DAPM_OUTPUT("TDM_AUDIO_PA_DOWN_OUTPUT"),\
	SND_SOC_DAPM_OUTPUT("FM_OUTPUT"),\
	SND_SOC_DAPM_OUTPUT("HP_CONCURRENCY_OUTPUT"),\

#define HI6405_SUPPLY_WIDGET \
	SND_SOC_DAPM_SUPPLY_S("PLL_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, hi6405_pll_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("44K1_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, hi6405_44k1_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("MAD_CLK_SUPPLY", \
		0, SND_SOC_NOPM, 0, 0, hi6405_mad_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("DP_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, hi6405_dp_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S1_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, hi6405_s1_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S2_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, hi6405_s2_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S4_IF_CLK_SUPPLY", \
		1, SND_SOC_NOPM, 0, 0, hi6405_s4_if_clk_supply_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP1_SUPPLY", \
		3, SND_SOC_NOPM, 0, 0, hi6405_cp1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("CP2_SUPPLY", \
		4, SND_SOC_NOPM, 0, 0, hi6405_cp2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S2_RX_SUPPLY", \
		2, SND_SOC_NOPM, 0, 0, hi6405_s2_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SUPPLY_S("S4_RX_SUPPLY", \
		2, SND_SOC_NOPM, 0, 0, hi6405_s4_rx_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define HI6405_MICBIAS_WIDGET \
	SND_SOC_DAPM_MIC("MICBIAS1_MIC", hi6405_micbias1_power_event), \
	SND_SOC_DAPM_MIC("MICBIAS2_MIC", hi6405_micbias2_power_event), \
	SND_SOC_DAPM_MIC("HSMICBIAS", hi6405_hsmicbias_power_event), \

#define HI6405_PGA_WIDGET \
	SND_SOC_DAPM_PGA_S("MAD_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_mad_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("HSMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_hsmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("AUXMIC_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_auxmic_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC3_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_mic3_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC4_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_mic4_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("MIC5_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		hi6405_mic5_pga_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR1", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcr1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL1", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcl1_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCR0", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcr0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL0", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcl0_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcl2_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("ADCL2_VIRTBTN_IR", \
		1, SND_SOC_NOPM, 0, 0, hi6405_adcl2_virtbtn_ir_power_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_PGA_S("S1_IL_PGA", \
		0, HI6405_S1_DP_CLK_EN_REG, HI6405_S1_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S1_IR_PGA", \
		0, HI6405_S1_DP_CLK_EN_REG, HI6405_S1_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S2_IL_PGA", \
		0, HI6405_S2_DP_CLK_EN_REG, HI6405_S2_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S2_IR_PGA", \
		0, HI6405_S2_DP_CLK_EN_REG, HI6405_S2_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S3_IL_PGA", \
		0, HI6405_S3_DP_CLK_EN_REG, HI6405_S3_IL_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S3_IR_PGA", \
		0, HI6405_S3_DP_CLK_EN_REG, HI6405_S3_IR_PGA_CLK_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IL_PGA", \
		0, HI6405_VIRTUAL_DOWN_REG, HI6405_S4_IL_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("S4_IR_PGA", \
		0, HI6405_VIRTUAL_DOWN_REG, HI6405_S4_IR_BIT, 0, \
		NULL, 0), \
	SND_SOC_DAPM_PGA_S("SIDETONE_PGA", \
		0, SND_SOC_NOPM, 0, 0, \
		NULL, 0), \

#define HI6405_MIXER_WIDGET \
	SND_SOC_DAPM_MIXER("MAD_PGA_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		hi6405_mad_pga_mixer_controls, \
		ARRAY_SIZE(hi6405_mad_pga_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DSDL_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		hi6405_dsdl_mixer_controls, \
		ARRAY_SIZE(hi6405_dsdl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DSDR_MIXER", \
		SND_SOC_NOPM, 0, 0, \
		hi6405_dsdr_mixer_controls, \
		ARRAY_SIZE(hi6405_dsdr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL_MIXER", \
		HI6405_DAC_DP_CLK_EN_1_REG, HI6405_DACL_MIXER4_CLK_EN_OFFSET, 0, \
		hi6405_dacl_mixer_controls, \
		ARRAY_SIZE(hi6405_dacl_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR_MIXER", \
		HI6405_DAC_DP_CLK_EN_1_REG, HI6405_DACR_MIXER4_CLK_EN_OFFSET, 0, \
		hi6405_dacr_mixer_controls, \
		ARRAY_SIZE(hi6405_dacr_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACL_PRE_MIXER", \
		HI6405_DAC_MIXER_CLK_EN_REG, HI6405_DACL_PRE_MIXER2_CLK_EN_OFFSET, 0, \
		hi6405_dacl_pre_mixer_controls, \
		ARRAY_SIZE(hi6405_dacl_pre_mixer_controls)), \
	SND_SOC_DAPM_MIXER("DACR_PRE_MIXER", \
		HI6405_DAC_MIXER_CLK_EN_REG, HI6405_DACR_PRE_MIXER2_CLK_EN_OFFSET, 0, \
		hi6405_dacr_pre_mixer_controls, \
		ARRAY_SIZE(hi6405_dacr_pre_mixer_controls)), \
	SND_SOC_DAPM_MIXER_E("DACL_POST_MIXER", \
		HI6405_DAC_MIXER_CLK_EN_REG, HI6405_DACL_POST_MIXER2_CLK_EN_OFFSET, 0, \
		hi6405_dacl_post_mixer_controls, \
		ARRAY_SIZE(hi6405_dacl_post_mixer_controls), \
		hi6405_dacl_post_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACR_POST_MIXER", \
		HI6405_DAC_MIXER_CLK_EN_REG, HI6405_DACR_POST_MIXER2_CLK_EN_OFFSET, 0, \
		hi6405_dacr_post_mixer_controls, \
		ARRAY_SIZE(hi6405_dacr_post_mixer_controls), \
		hi6405_dacr_post_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACSL_MIXER", \
		HI6405_DAC_DP_CLK_EN_1_REG, HI6405_DACL_S_MIXER4_CLK_EN_OFFSET, 0, \
		hi6405_dacsl_mixer_controls, \
		ARRAY_SIZE(hi6405_dacsl_mixer_controls), \
		hi6405_dacsl_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MIXER_E("DACSR_MIXER", \
		HI6405_DAC_DP_CLK_EN_1_REG, HI6405_DACR_S_MIXER4_CLK_EN_OFFSET, 0, \
		hi6405_dacsr_mixer_controls, \
		ARRAY_SIZE(hi6405_dacsr_mixer_controls), \
		hi6405_dacsr_mixer_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define HI6405_MUX_WIDGET \
	SND_SOC_DAPM_MUX("HSMIC_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_hsmic_pga_mux_controls), \
	SND_SOC_DAPM_MUX("AUXMIC_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_auxmic_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic3_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic4_pga_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic5_pga_mux_controls), \
	SND_SOC_DAPM_MUX("ADCL0_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_adcl0_mux_controls), \
	SND_SOC_DAPM_MUX("ADCR0_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_adcr0_mux_controls), \
	SND_SOC_DAPM_MUX("ADCL1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_adcl1_mux_controls), \
	SND_SOC_DAPM_MUX("ADCR1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_adcr1_mux_controls), \
	SND_SOC_DAPM_MUX("ADCL2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_adcl2_mux_controls), \
	SND_SOC_DAPM_MUX("MIC1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic1_mux_controls), \
	SND_SOC_DAPM_MUX("MIC2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic2_mux_controls), \
	SND_SOC_DAPM_MUX("MIC3_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic3_mux_controls), \
	SND_SOC_DAPM_MUX("MIC4_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic4_mux_controls), \
	SND_SOC_DAPM_MUX("MIC5_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mic5_mux_controls), \
	SND_SOC_DAPM_MUX("SIDETONE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_sidetone_mux_controls), \
	SND_SOC_DAPM_MUX("M1_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_m1_l_mux_controls), \
	SND_SOC_DAPM_MUX("M1_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_m1_r_mux_controls), \
	SND_SOC_DAPM_MUX("BT_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_bt_l_mux_controls), \
	SND_SOC_DAPM_MUX("BT_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_bt_r_mux_controls), \
	SND_SOC_DAPM_MUX("US_R1_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_us_r1_mux_controls), \
	SND_SOC_DAPM_MUX("US_R2_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_us_r2_mux_controls), \
	SND_SOC_DAPM_MUX("S1_IL_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s1_il_mux_controls), \
	SND_SOC_DAPM_MUX("S1_IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s1_ir_mux_controls), \
	SND_SOC_DAPM_MUX("S2_IL_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s2_il_mux_controls), \
	SND_SOC_DAPM_MUX("S2_IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s2_ir_mux_controls), \
	SND_SOC_DAPM_MUX("DSP_IF8_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dsp_if8_mux_controls), \
	SND_SOC_DAPM_MUX("DACL_PRE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dacl_pre_mux_controls), \
	SND_SOC_DAPM_MUX("DACR_PRE_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dacr_pre_mux_controls), \
	SND_SOC_DAPM_MUX_E("DACL_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dacl_pga_mux_controls, \
		hi6405_dacl_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX_E("DACR_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dacr_pga_mux_controls, \
		hi6405_dacr_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX("HP_L_SDM_MUX", \
		HI6405_DAC_DP_CLK_EN_2_REG, HI6405_HP_SDM_L_CLK_EN_OFFSET, 0, \
		&hi6405_dapm_hp_l_sdm_mux_controls), \
	SND_SOC_DAPM_MUX("HP_R_SDM_MUX", \
		HI6405_DAC_DP_CLK_EN_2_REG, HI6405_HP_SDM_R_CLK_EN_OFFSET, 0, \
		&hi6405_dapm_hp_r_sdm_mux_controls), \
	SND_SOC_DAPM_MUX_E("EP_L_SDM_MUX", \
		HI6405_DAC_DP_CLK_EN_2_REG, HI6405_EP_SDM_L_CLK_EN_OFFSET, 0, \
		&hi6405_dapm_ep_l_sdm_mux_controls, \
		HI6405_ep_l_sdm_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX_E("DACSL_PGA_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_dacsl_pag_mux_controls, \
		hi6405_dacsl_pga_mux_event, \
		(SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_MUX("HP_DAC_L_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_hp_dac_l_mux_controls), \
	SND_SOC_DAPM_MUX("HP_DAC_R_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_hp_dac_r_mux_controls), \
	SND_SOC_DAPM_MUX("EP_DAC_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_ep_dac_mux_controls), \
	SND_SOC_DAPM_MUX("IR_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_ir_mux_controls), \
	SND_SOC_DAPM_MUX("MDM_MUX", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_mdm_mux_controls), \

#define HI6405_SWITCH_WIDGET \
	SND_SOC_DAPM_SWITCH_E("PLAY44K1_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_play44k1_switch_controls, \
		hi6405_play44k1_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY48K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_play48k_switch_controls, \
		hi6405_play48k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY96K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_play96k_switch_controls, \
		hi6405_play96k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY192K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_play192k_switch_controls, \
		hi6405_play192k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLAY384K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_play384k_switch_controls, \
		hi6405_play384k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("HP_HIGHLEVEL_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_hp_high_switch_controls, \
		hi6405_hp_high_level_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("HP_CONCURRENCY_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_hp_concurrency_switch_controls, \
		hi6405_hp_concurrency_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLL48K_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_pll48k_switch_controls, \
		hi6405_pll48k_test_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLL44K1_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_pll44k1_switch_controls, \
		hi6405_pll44k1_test_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("PLLMAD_TEST_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_pll32k_switch_controls, \
		hi6405_pll32k_test_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("S2_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_s2_ol_switch_controls, \
		hi6405_s2up_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S2_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s2_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s4_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S4_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("PLAY384K_VIR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_play384k_vir_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("IV_DSPIF_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_iv_dspif_switch_controls, \
		hi6405_iv_dspif_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("IV_2PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_iv_2pa_switch_controls, \
	    hi6405_iv_2pa_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("IV_4PA_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_iv_4pa_switch_controls), \
	SND_SOC_DAPM_SWITCH("IR_EMISSION_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_ir_emission_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_2MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_audioup_2mic_switch_controls, \
		hi6405_audioup_2mic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_4MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_audioup_4mic_switch_controls, \
		hi6405_audioup_4mic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("AUDIOUP_5MIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_audioup_5mic_switch_controls, \
		hi6405_audioup_5mic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE8K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_voice8k_switch_controls, \
		hi6405_voice8k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE16K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_voice16k_switch_controls, \
		hi6405_voice16k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VOICE32K_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_voice32k_switch_controls, \
		hi6405_voice32k_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MOVEUP_2PA_TDM_IV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_moveup_2pa_tdm_iv_switch_controls, \
		hi6405_moveup_2pa_tdm_iv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MOVEUP_4PA_TDM_IV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_moveup_4pa_tdm_iv_switch_controls, \
		hi6405_moveup_4pa_tdm_iv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("MAD_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_mad_switch_controls, \
		hi6405_madswitch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("S1_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s1_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S1_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s1_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("S3_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_s3_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("U3_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_u3_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("U4_OR_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_u4_or_switch_controls), \
	SND_SOC_DAPM_SWITCH("U5_OL_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_u5_ol_switch_controls), \
	SND_SOC_DAPM_SWITCH("US_R1_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_us_r1_switch_controls), \
	SND_SOC_DAPM_SWITCH("US_R2_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_us_r2_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("IR_STUDY_ENV_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_ir_env_study_switch_controls, \
		hi6405_ir_study_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER_ONEMIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_soundtrigger_onemic_switch_controls, \
		hi6405_soundtrigger_onemic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER_DUALMIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_soundtrigger_dualmic_switch_controls, \
		hi6405_soundtrigger_dualmic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("SOUNDTRIGGER_MULTIMIC_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_soundtrigger_multimic_switch_controls, \
		hi6405_soundtrigger_4mic_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("U7_EC_SWITCH", SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_ec_switch_controls, \
		hi6405_ec_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH("AUXMIC_HSMICBIAS_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_auxmic_hsmicbias_switch_controls), \
	SND_SOC_DAPM_SWITCH("AUXMIC_MICBIAS2_SWITCH", \
		SND_SOC_NOPM, 0, 0, \
		&hi6405_dapm_auxmic_micbias2_switch_controls), \
	SND_SOC_DAPM_SWITCH_E("I2S2_BLUETOOTH_LOOP_SWITCH",\
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_i2s2_bluetooth_loop_switch_controls,\
		hi6405_i2s2_bluetooth_loop_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
	SND_SOC_DAPM_SWITCH_E("TDM_AUDIO_PA_DOWN_SWITCH",\
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_tdm_audio_pa_down_switch_controls,\
		hi6405_tdm_audio_pa_down_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
	SND_SOC_DAPM_SWITCH_E("TDM_AUDIO_PA_DN_LOOP_SWITCH",\
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_tdm_audio_pa_down_loop_switch_controls,\
		hi6405_tdm_audio_pa_down_loop_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)),\
	SND_SOC_DAPM_SWITCH_E("FM_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_fm_switch_controls, \
		hi6405_fm_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VIRTUAL_BTN_ACTIVE_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_virtbtn_active_switch_controls, \
		hi6405_virtbtn_active_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_SWITCH_E("VIRTUAL_BTN_PASSIVE_SWITCH", \
		SND_SOC_NOPM, 0, 0, &hi6405_dapm_virtbtn_passive_switch_controls, \
		hi6405_virtbtn_passive_switch_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

#define HI6405_OUT_DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV_E("HP_L_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		hi6405_headphone_l_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("HP_R_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		hi6405_headphone_r_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("EP_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		hi6405_earpiece_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("IR_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		hi6405_ir_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("S2_TX_DRV", \
		HI6405_SC_S2_IF_L_REG, HI6405_S2_IF_TX_ENA_OFFSET, 0, NULL, 0, \
		hi6405_s2_tx_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV("S4_TX_DRV", \
		HI6405_SC_S4_IF_L_REG, HI6405_S4_IF_TX_ENA_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U1_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP1_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U2_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP2_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U3_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP3_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U4_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP4_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U5_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP5_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U6_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP6_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U7_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP7_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U8_DRV", \
		HI6405_SLIM_UP_EN1_REG, HI6405_SLIM_UP8_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U9_DRV", \
		HI6405_SLIM_UP_EN2_REG, HI6405_SLIM_UP9_EN_OFFSET, 0, \
		NULL, 0), \
	SND_SOC_DAPM_OUT_DRV("U10_DRV", \
		HI6405_SLIM_UP_EN2_REG, HI6405_SLIM_UP10_EN_OFFSET, 0, \
		NULL, 0), \

static const struct snd_soc_dapm_widget hi6405_dapm_widgets[] = {
	HI6405_INPUT_WIDGET
	HI6405_OUTPUT_WIDGET
	HI6405_SUPPLY_WIDGET
	HI6405_MICBIAS_WIDGET
	HI6405_PGA_WIDGET
	HI6405_MIXER_WIDGET
	HI6405_MUX_WIDGET
	HI6405_SWITCH_WIDGET
	HI6405_OUT_DRV_WIDGET
};

#define HI6405_DP_SUPPLY_ROUTE \
	{"U1_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U2_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U3_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U4_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U5_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U6_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U8_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U9_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"U10_OUTPUT",                   NULL,              "DP_CLK_SUPPLY"}, \
	{"MAD_OUTPUT",                   NULL,              "DP_CLK_SUPPLY"}, \
	{"VIRTUAL_BTN_ACTIVE_OUTPUT",    NULL,              "DP_CLK_SUPPLY"}, \
	{"VIRTUAL_BTN_PASSIVE_OUTPUT",   NULL,              "DP_CLK_SUPPLY"}, \
	{"IR_STUDY_OUTPUT",              NULL,              "DP_CLK_SUPPLY"}, \
	{"D1_D2_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"D3_D4_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"D5_D6_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"S1_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"S2_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"S4_RX_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"IR_TX_INPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"S1_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY"}, \
	{"S2_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY"}, \
	{"S4_TX_OUTPUT",                 NULL,              "DP_CLK_SUPPLY"}, \
	{"I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "DP_CLK_SUPPLY"}, \
	{"TDM_AUDIO_PA_DOWN_SWITCH",     NULL,              "DP_CLK_SUPPLY"}, \
	{"TDM_AUDIO_PA_DN_LOOP_SWITCH",  NULL,              "DP_CLK_SUPPLY"}, \
	{"FM_SWITCH",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"EP_OUTPUT",                    NULL,              "DP_CLK_SUPPLY"}, \
	{"HP_L_OUTPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"HP_R_OUTPUT",                  NULL,              "DP_CLK_SUPPLY"}, \
	{"DSD_L_OUTPUT",                 NULL,              "DP_CLK_SUPPLY"}, \
	{"DSD_R_OUTPUT",                 NULL,              "DP_CLK_SUPPLY"}, \

#define HI6405_PLL_SUPPLY_ROUTE \
	{"PLAY44K1_SWITCH",              NULL,              "PLL_CLK_SUPPLY"}, \
	{"PLAY48K_SWITCH",               NULL,              "PLL_CLK_SUPPLY"}, \
	{"PLAY96K_SWITCH",               NULL,              "PLL_CLK_SUPPLY"}, \
	{"PLAY192K_SWITCH",              NULL,              "PLL_CLK_SUPPLY"}, \
	{"PLAY384K_SWITCH",              NULL,              "PLL_CLK_SUPPLY"}, \
	{"D1_D2_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"D3_D4_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"D5_D6_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"S1_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"S2_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"S4_RX_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"S1_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY"}, \
	{"IR_TX_INPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"S2_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY"}, \
	{"S4_TX_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY"}, \
	{"SOUNDTRIGGER_ONEMIC_SWITCH",   NULL,              "PLL_CLK_SUPPLY"}, \
	{"SOUNDTRIGGER_DUALMIC_SWITCH",  NULL,              "PLL_CLK_SUPPLY"}, \
	{"SOUNDTRIGGER_MULTIMIC_SWITCH", NULL,              "PLL_CLK_SUPPLY"}, \
	{"VOICE8K_SWITCH",               NULL,              "PLL_CLK_SUPPLY"}, \
	{"VOICE16K_SWITCH",              NULL,              "PLL_CLK_SUPPLY"}, \
	{"VOICE32K_SWITCH",              NULL,              "PLL_CLK_SUPPLY"}, \
	{"AUDIOUP_2MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY"}, \
	{"AUDIOUP_4MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY"}, \
	{"AUDIOUP_5MIC_SWITCH",          NULL,              "PLL_CLK_SUPPLY"}, \
	{"HSMIC_PGA",                    NULL,              "PLL_CLK_SUPPLY"}, \
	{"AUXMIC_PGA",                   NULL,              "PLL_CLK_SUPPLY"}, \
	{"MIC3_PGA",                     NULL,              "PLL_CLK_SUPPLY"}, \
	{"MIC4_PGA",                     NULL,              "PLL_CLK_SUPPLY"}, \
	{"MIC4_PGA",                     NULL,              "PLL_CLK_SUPPLY"}, \
	{"I2S2_BLUETOOTH_LOOP_SWITCH",   NULL,              "PLL_CLK_SUPPLY"}, \
	{"TDM_AUDIO_PA_DOWN_SWITCH",     NULL,              "PLL_CLK_SUPPLY"}, \
	{"TDM_AUDIO_PA_DN_LOOP_SWITCH",  NULL,              "PLL_CLK_SUPPLY"}, \
	{"FM_SWITCH",                    NULL,              "PLL_CLK_SUPPLY"}, \
	{"EP_OUTPUT",                    NULL,              "PLL_CLK_SUPPLY"}, \
	{"HP_L_OUTPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"HP_R_OUTPUT",                  NULL,              "PLL_CLK_SUPPLY"}, \
	{"DSD_L_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY"}, \
	{"DSD_R_OUTPUT",                 NULL,              "PLL_CLK_SUPPLY"}, \

#define HI6405_MAD_SUPPLY_ROUTE \
	{"MAD_PGA",                      NULL,              "MAD_CLK_SUPPLY"}, \

#define HI6405_44K1_SUPPLY_ROUTE \
	{"PLAY44K1_SWITCH",              NULL,              "44K1_CLK_SUPPLY"}, \

#define HI6405_I2S_SUPPLY_ROUTE \
	{"S1_RX_INPUT",                  NULL,              "S1_IF_CLK_SUPPLY"}, \
	{"S2_RX_INPUT",                  NULL,              "S2_IF_CLK_SUPPLY"}, \
	{"S4_RX_INPUT",                  NULL,              "S4_IF_CLK_SUPPLY"}, \
	{"S1_TX_OUTPUT",                 NULL,              "S1_IF_CLK_SUPPLY"}, \
	{"S2_TX_OUTPUT",                 NULL,              "S2_IF_CLK_SUPPLY"}, \
	{"S4_TX_OUTPUT",                 NULL,              "S4_IF_CLK_SUPPLY"}, \
	{"S2_RX_INPUT",                  NULL,              "S2_RX_SUPPLY"}, \
	{"S4_RX_INPUT",                  NULL,              "S4_RX_SUPPLY"}, \

#define HI6405_CP1_SUPPLY_ROUTE \
	{"CP2_SUPPLY",                   NULL,              "CP1_SUPPLY"}, \
	{"EP_OUTPUT",                    NULL,              "CP1_SUPPLY"}, \

#define HI6405_CP2_SUPPLY_ROUTE \
	{"HP_L_OUTPUT",                  NULL,              "CP2_SUPPLY"}, \
	{"HP_R_OUTPUT",                  NULL,              "CP2_SUPPLY"}, \

#define HI6405_PLL_TEST_ROUTE \
	{"PLL48K_TEST_SWITCH",           "ENABLE",          "PLL_TEST_INPUT"}, \
	{"PLL44K1_TEST_SWITCH",          "ENABLE",          "PLL_TEST_INPUT"}, \
	{"PLLMAD_TEST_SWITCH",           "ENABLE",          "PLL_TEST_INPUT"}, \
	{"PLL_TEST_OUTPUT",              NULL,              "PLL48K_TEST_SWITCH"}, \
	{"PLL_TEST_OUTPUT",              NULL,              "PLL44K1_TEST_SWITCH"}, \
	{"PLL_TEST_OUTPUT",              NULL,              "PLLMAD_TEST_SWITCH"}, \

#define HI6405_VOICE_SWITCH_ROUTE \
	{"VOICE_OUTPUT",                 NULL,              "VOICE32K_SWITCH"}, \
	{"VOICE32K_SWITCH",              "ENABLE",          "VOICE_INPUT"}, \
	{"VOICE_OUTPUT",                 NULL,              "VOICE16K_SWITCH"}, \
	{"VOICE16K_SWITCH",              "ENABLE",          "VOICE_INPUT"}, \
	{"VOICE_OUTPUT",                 NULL,              "VOICE8K_SWITCH"}, \
	{"VOICE8K_SWITCH",               "ENABLE",          "VOICE_INPUT"}, \

#define HI6405_MOVEUP_PA_TDM_SWITCH_ROUTE \
	{"MOVEUP_2PA_TDM_IV_SWITCH",     "ENABLE",          "S4_RX_INPUT"}, \
	{"U7_DRV",                       NULL,              "MOVEUP_2PA_TDM_IV_SWITCH"}, \
	{"U8_DRV",                       NULL,              "MOVEUP_2PA_TDM_IV_SWITCH"}, \
	{"MOVEUP_4PA_TDM_IV_SWITCH",     "ENABLE",          "S4_RX_INPUT"}, \
	{"U5_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH"}, \
	{"U6_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH"}, \
	{"U7_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH"}, \
	{"U8_DRV",                       NULL,              "MOVEUP_4PA_TDM_IV_SWITCH"}, \
	{"TDM_AUDIO_PA_DOWN_SWITCH",     "ENABLE",          "TDM_AUDIO_PA_DOWN_INPUT"}, \
	{"TDM_AUDIO_PA_DOWN_OUTPUT",     NULL,            "TDM_AUDIO_PA_DOWN_SWITCH"}, \

#define HI6405_SOUNDTRIGGER_SWITCH_ROUTE \
	{"VOICE_OUTPUT",                 NULL,              "SOUNDTRIGGER_ONEMIC_SWITCH"}, \
	{"VOICE_OUTPUT",                 NULL,              "SOUNDTRIGGER_DUALMIC_SWITCH"}, \
	{"VOICE_OUTPUT",                 NULL,              "SOUNDTRIGGER_MULTIMIC_SWITCH"}, \
	{"SOUNDTRIGGER_ONEMIC_SWITCH",   "ENABLE",          "VOICE_INPUT"}, \
	{"SOUNDTRIGGER_DUALMIC_SWITCH",  "ENABLE",          "VOICE_INPUT"}, \
	{"SOUNDTRIGGER_MULTIMIC_SWITCH", "ENABLE",          "VOICE_INPUT"}, \

#define HI6405_AUDIOUP_SWITCH_ROUTE \
	{"AUDIOUP_2MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT"}, \
	{"AUDIOUP_4MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT"}, \
	{"AUDIOUP_5MIC_SWITCH",          "ENABLE",          "AUDIOUP_INPUT"}, \
	{"AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_2MIC_SWITCH"}, \
	{"AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_4MIC_SWITCH"}, \
	{"AUDIOUP_OUTPUT",               NULL,              "AUDIOUP_5MIC_SWITCH"}, \

#define HI6405_HP_HIGHLEVEL_SWITCH_ROUTE \
	{"HP_HIGHLEVEL_SWITCH",          "ENABLE",          "HP_HIGHLEVEL_INPUT"}, \
	{"HP_HIGHLEVEL_OUTPUT",          NULL,              "HP_HIGHLEVEL_SWITCH"}, \

#define HI6405_HP_CONCURRENCY_SWITCH_ROUTE \
	{"HP_CONCURRENCY_SWITCH",        "ENABLE",          "HP_CONCURRENCY_INPUT"}, \
	{"HP_CONCURRENCY_OUTPUT",        NULL,              "HP_CONCURRENCY_SWITCH"}, \

#define HI6405_TX_OUTPUT_ROUTE \
	{"U1_DRV",                       NULL,              "S1_OL_SWITCH"}, \
	{"U2_DRV",                       NULL,              "S1_OR_SWITCH"}, \
	{"U3_DRV",                       NULL,              "U3_OL_SWITCH"}, \
	{"U4_DRV",                       NULL,              "U4_OR_SWITCH"}, \
	{"U5_DRV",                       NULL,              "S3_OL_SWITCH"}, \
	{"U6_DRV",                       NULL,              "S3_OR_SWITCH"}, \
	{"U8_DRV",                       NULL,              "US_R1_SWITCH"}, \
	{"U9_DRV",                       NULL,              "US_R2_SWITCH"}, \
	{"U10_DRV",                      NULL,              "U5_OL_SWITCH"}, \
	{"IR_STUDY_OUTPUT",              NULL,              "IR_STUDY_ENV_SWITCH"}, \
	{"U1_OUTPUT",                    NULL,              "U1_DRV"}, \
	{"U2_OUTPUT",                    NULL,              "U2_DRV"}, \
	{"U3_OUTPUT",                    NULL,              "U3_DRV"}, \
	{"U4_OUTPUT",                    NULL,              "U4_DRV"}, \
	{"U5_OUTPUT",                    NULL,              "U5_DRV"}, \
	{"U6_OUTPUT",                    NULL,              "U6_DRV"}, \
	{"U7_OUTPUT",                    NULL,              "U7_DRV"}, \
	{"U8_OUTPUT",                    NULL,              "U8_DRV"}, \
	{"U9_OUTPUT",                    NULL,              "U9_DRV"}, \
	{"U10_OUTPUT",                   NULL,              "U10_DRV"}, \
	{"MAD_OUTPUT",                   NULL,              "MAD_SWITCH"}, \
	{"VIRTUAL_BTN_ACTIVE_OUTPUT",    NULL,              "VIRTUAL_BTN_ACTIVE_SWITCH"}, \
	{"VIRTUAL_BTN_PASSIVE_OUTPUT",   NULL,              "VIRTUAL_BTN_PASSIVE_SWITCH"}, \

#define HI6405_RX_OUTPUT_ROUTE \
	{"HP_L_DRV",                     NULL,              "HP_DAC_L_MUX"}, \
	{"HP_R_DRV",                     NULL,              "HP_DAC_R_MUX"}, \
	{"EP_DRV",                       NULL,              "EP_DAC_MUX"}, \
	{"IR_DRV",                       NULL,              "IR_EMISSION_SWITCH"}, \
	{"S2_TX_DRV",                    NULL,              "S2_OL_SWITCH"}, \
	{"S2_TX_DRV",                    NULL,              "S2_OR_SWITCH"}, \
	{"S4_TX_DRV",                    NULL,              "S4_OL_SWITCH"}, \
	{"S4_TX_DRV",                    NULL,              "S4_OR_SWITCH"}, \
	{"HP_L_OUTPUT",                  NULL,              "HP_L_DRV"}, \
	{"HP_R_OUTPUT",                  NULL,              "HP_R_DRV"}, \
	{"EP_OUTPUT",                    NULL,              "EP_DRV"}, \
	{"IR_LED_OUTPUT",                NULL,              "IR_DRV"}, \
	{"S2_TX_OUTPUT",                 NULL,              "S2_TX_DRV"}, \
	{"S4_TX_OUTPUT",                 NULL,              "S4_TX_DRV"}, \

#define HI6405_AUDIODOWN_SWITCH_ROUTE \
	{"PLAY44K1_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT"}, \
	{"PLAY48K_SWITCH",               "ENABLE",          "AUDIO_DOWN_INPUT"}, \
	{"PLAY96K_SWITCH",               "ENABLE",          "AUDIO_DOWN_INPUT"}, \
	{"PLAY192K_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT"}, \
	{"PLAY384K_SWITCH",              "ENABLE",          "AUDIO_DOWN_INPUT"}, \
	{"AUDIO_DOWN_OUTPUT",            NULL,              "PLAY44K1_SWITCH"}, \
	{"AUDIO_DOWN_OUTPUT",            NULL,              "PLAY48K_SWITCH"}, \
	{"AUDIO_DOWN_OUTPUT",            NULL,              "PLAY96K_SWITCH"}, \
	{"AUDIO_DOWN_OUTPUT",            NULL,              "PLAY192K_SWITCH"}, \
	{"AUDIO_DOWN_OUTPUT",            NULL,              "PLAY384K_SWITCH"}, \
	{"PLAY384K_VIR_SWITCH",          "ENABLE",          "S1_IL_PGA"}, \
	{"PLAY384K_VIR_SWITCH",          "ENABLE",          "S1_IR_PGA"}, \

#define HI6405_FM_SWITCH_ROUTE \
	{"FM_SWITCH",                    "ENABLE",          "FM_INPUT"}, \
	{"FM_OUTPUT",                    NULL,              "FM_SWITCH"}, \

#define HI6405_MICBIAS_ROUTE \
	{"MAINMIC_INPUT",                NULL,                        "MICBIAS1_MIC"}, \
	{"AUXMIC_HSMICBIAS_SWITCH",      "ENABLE",                    "HSMICBIAS"}, \
	{"AUXMIC_MICBIAS2_SWITCH",       "ENABLE",                    "MICBIAS2_MIC"}, \
	{"AUXMIC_INPUT",                 NULL,                        "AUXMIC_HSMICBIAS_SWITCH"}, \
	{"AUXMIC_INPUT",                 NULL,                        "AUXMIC_MICBIAS2_SWITCH"}, \
	{"MIC3_INPUT",                   NULL,                        "MICBIAS2_MIC"}, \
	{"MIC4_INPUT",                   NULL,                        "MICBIAS2_MIC"}, \
	{"MIC5_INPUT",                   NULL,                        "MICBIAS2_MIC"}, \
	{"HSMIC_INPUT",                  NULL,                        "HSMICBIAS"}, \

#define HI6405_MICPGA_ROUTE \
	{"HSMIC_PGA_MUX",                "MAINMIC",         "MAINMIC_INPUT"}, \
	{"HSMIC_PGA_MUX",                "HSMIC",           "HSMIC_INPUT"}, \
	{"AUXMIC_PGA_MUX",               "MAINMIC",         "MAINMIC_INPUT"}, \
	{"AUXMIC_PGA_MUX",               "AUXMIC",          "AUXMIC_INPUT"}, \
	{"MIC3_PGA_MUX",                 "MIC3",            "MIC3_INPUT"}, \
	{"MIC3_PGA_MUX",                 "MIC4",            "MIC4_INPUT"}, \
	{"MIC4_PGA_MUX",                 "MIC3",            "MIC3_INPUT"}, \
	{"MIC4_PGA_MUX",                 "MIC4",            "MIC4_INPUT"}, \
	{"MIC5_PGA_MUX",                 "MIC5",            "MIC5_INPUT"}, \
	{"MIC5_PGA_MUX",                 "IR",              "IR_STUDY_INPUT"}, \
	{"MAD_PGA_MIXER",                "MIC5_ENABLE",     "MIC5_INPUT"}, \
	{"MAD_PGA_MIXER",                "MIC4_ENABLE",     "MIC4_INPUT"}, \
	{"MAD_PGA_MIXER",                "MIC3_ENABLE",     "MIC3_INPUT"}, \
	{"MAD_PGA_MIXER",                "AUXMIC_ENABLE",   "AUXMIC_INPUT"}, \
	{"MAD_PGA_MIXER",                "MAINMIC_ENABLE",  "MAINMIC_INPUT"}, \
	{"MAD_PGA_MIXER",                "HSMIC_ENABLE",    "HSMIC_INPUT"}, \
	{"MAD_PGA",                      NULL,              "MAD_PGA_MIXER"}, \
	{"HSMIC_PGA",                    NULL,              "HSMIC_PGA_MUX"}, \
	{"AUXMIC_PGA",                   NULL,              "AUXMIC_PGA_MUX"}, \
	{"MIC3_PGA",                     NULL,              "MIC3_PGA_MUX"}, \
	{"MIC4_PGA",                     NULL,              "MIC4_PGA_MUX"}, \
	{"MIC5_PGA",                     NULL,              "MIC5_PGA_MUX"}, \

#define HI6405_ADC_ROUTE \
	{"ADCL0",                        NULL,              "ADCL0_MUX"}, \
	{"ADCR0",                        NULL,              "ADCR0_MUX"}, \
	{"ADCL1",                        NULL,              "ADCL1_MUX"}, \
	{"ADCR1",                        NULL,              "ADCR1_MUX"}, \
	{"ADCL2",                        NULL,              "ADCL2_MUX"}, \

#define HI6405_ADC_MUX_ROUTE \
	{"ADCL0_MUX",                    "ADC_MAD",         "MAD_PGA"}, \
	{"ADCL0_MUX",                    "ADC0",            "HSMIC_PGA"}, \
	{"ADCL0_MUX",                    "ADC1",            "AUXMIC_PGA"}, \
	{"ADCL0_MUX",                    "ADC2",            "MIC3_PGA"}, \
	{"ADCL0_MUX",                    "ADC3",            "MIC4_PGA"}, \
	{"ADCL0_MUX",                    "ADC4",            "MIC5_PGA"}, \
	{"ADCL1_MUX",                    "ADC_MAD",         "MAD_PGA"}, \
	{"ADCL1_MUX",                    "ADC0",            "HSMIC_PGA"}, \
	{"ADCL1_MUX",                    "ADC1",            "AUXMIC_PGA"}, \
	{"ADCL1_MUX",                    "ADC2",            "MIC3_PGA"}, \
	{"ADCL1_MUX",                    "ADC3",            "MIC4_PGA"}, \
	{"ADCL1_MUX",                    "ADC4",            "MIC5_PGA"}, \
	{"ADCR0_MUX",                    "ADC_MAD",         "MAD_PGA"}, \
	{"ADCR0_MUX",                    "ADC0",            "HSMIC_PGA"}, \
	{"ADCR0_MUX",                    "ADC1",            "AUXMIC_PGA"}, \
	{"ADCR0_MUX",                    "ADC2",            "MIC3_PGA"}, \
	{"ADCR0_MUX",                    "ADC3",            "MIC4_PGA"}, \
	{"ADCR0_MUX",                    "ADC4",            "MIC5_PGA"}, \
	{"ADCR1_MUX",                    "ADC_MAD",         "MAD_PGA"}, \
	{"ADCR1_MUX",                    "ADC0",            "HSMIC_PGA"}, \
	{"ADCR1_MUX",                    "ADC1",            "AUXMIC_PGA"}, \
	{"ADCR1_MUX",                    "ADC2",            "MIC3_PGA"}, \
	{"ADCR1_MUX",                    "ADC3",            "MIC4_PGA"}, \
	{"ADCR1_MUX",                    "ADC4",            "MIC5_PGA"}, \
	{"ADCL2_MUX",                    "ADC_MAD",         "MAD_PGA"}, \
	{"ADCL2_MUX",                    "ADC0",            "HSMIC_PGA"}, \
	{"ADCL2_MUX",                    "ADC1",            "AUXMIC_PGA"}, \
	{"ADCL2_MUX",                    "ADC2",            "MIC3_PGA"}, \
	{"ADCL2_MUX",                    "ADC3",            "MIC4_PGA"}, \
	{"ADCL2_MUX",                    "ADC4",            "MIC5_PGA"}, \

#define HI6405_TX_COMMON_MUX_ROUTE \
	{"MIC1_MUX",                     "ADCL0",           "ADCL0"}, \
	{"MIC1_MUX",                     "ADCL1",           "ADCL1"}, \
	{"MIC1_MUX",                     "ADCR0",           "ADCR0"}, \
	{"MIC1_MUX",                     "ADCR1",           "ADCR1"}, \
	{"MIC1_MUX",                     "S2_L",            "S2_IL_PGA"}, \
	{"MIC1_MUX",                     "S3_L",            "S3_IL_PGA"}, \
	{"MIC1_MUX",                     "S4_L",            "S4_IL_PGA"}, \
	{"MIC1_MUX",                     "DACL_48",         "DACL_PRE_MIXER"}, \
	{"MIC1_MUX",                     "ADCL2",           "ADCL2"}, \
	{"MIC2_MUX",                     "ADCL0",           "ADCL0"}, \
	{"MIC2_MUX",                     "ADCL1",           "ADCL1"}, \
	{"MIC2_MUX",                     "ADCR0",           "ADCR0"}, \
	{"MIC2_MUX",                     "ADCR1",           "ADCR1"}, \
	{"MIC2_MUX",                     "S2_R",            "S2_IR_PGA"}, \
	{"MIC2_MUX",                     "S3_R",            "S3_IR_PGA"}, \
	{"MIC2_MUX",                     "S4_R",            "S4_IR_PGA"}, \
	{"MIC2_MUX",                     "DACR_48",         "DACR_PRE_MIXER"}, \
	{"MIC2_MUX",                     "ADCL2",           "ADCL2"}, \
	{"MIC3_MUX",                     "ADCL0",           "ADCL0"}, \
	{"MIC3_MUX",                     "ADCL1",           "ADCL1"}, \
	{"MIC3_MUX",                     "ADCR0",           "ADCR0"}, \
	{"MIC3_MUX",                     "ADCR1",           "ADCR1"}, \
	{"MIC3_MUX",                     "S1_L",            "S1_IL_PGA"}, \
	{"MIC3_MUX",                     "S2_L",            "S2_IL_PGA"}, \
	{"MIC3_MUX",                     "S4_L",            "S4_IL_PGA"}, \
	{"MIC3_MUX",                     "DACL_48",         "DACL_PRE_MIXER"}, \
	{"MIC3_MUX",                     "ADCL2",           "ADCL2"}, \
	{"MIC4_MUX",                     "ADCL0",           "ADCL0"}, \
	{"MIC4_MUX",                     "ADCL1",           "ADCL1"}, \
	{"MIC4_MUX",                     "ADCR0",           "ADCR0"}, \
	{"MIC4_MUX",                     "ADCR1",           "ADCR1"}, \
	{"MIC4_MUX",                     "S1_R",            "S1_IR_PGA"}, \
	{"MIC4_MUX",                     "S2_R",            "S2_IR_PGA"}, \
	{"MIC4_MUX",                     "S4_R",            "S4_IR_PGA"}, \
	{"MIC4_MUX",                     "DACR_48",         "DACR_PRE_MIXER"}, \
	{"MIC4_MUX",                     "ADCL2",           "ADCL2"}, \
	{"MIC5_MUX",                     "ADCL0",           "ADCL0"}, \
	{"MIC5_MUX",                     "ADCL1",           "ADCL1"}, \
	{"MIC5_MUX",                     "ADCR0",           "ADCR0"}, \
	{"MIC5_MUX",                     "ADCR1",           "ADCR1"}, \
	{"MIC5_MUX",                     "DACL_S",          "DACSL_MIXER"}, \
	{"MIC5_MUX",                     "DACR_S",          "DACSR_MIXER"}, \
	{"MIC5_MUX",                     "ADCL2",           "ADCL2"}, \
	{"SIDETONE_MUX",                 "S1_L",            "S1_IL_PGA"}, \
	{"SIDETONE_MUX",                 "S2_L",            "S2_IL_PGA"}, \
	{"SIDETONE_MUX",                 "ADCL0",           "ADCL0"}, \
	{"SIDETONE_MUX",                 "ADCR0",           "ADCR0"}, \
	{"SIDETONE_MUX",                 "ADCL1",           "ADCL1"}, \
	{"SIDETONE_MUX",                 "ADCR1",           "ADCR1"}, \
	{"SIDETONE_MUX",                 "ADCL2",           "ADCL2"}, \
	{"SIDETONE_PGA",                 NULL,              "SIDETONE_MUX"}, \
	{"M1_L_MUX",                     "ADCL0",           "ADCL0"}, \
	{"M1_L_MUX",                     "ADCL1",           "ADCL1"}, \
	{"M1_L_MUX",                     "DACL_S",          "DACSL_MIXER"}, \
	{"M1_L_MUX",                     "DACL_48",         "DACL_PRE_MIXER"}, \
	{"M1_L_MUX",                     "S1_L",            "S1_IL_PGA"}, \
	{"M1_L_MUX",                     "S2_L",            "S2_IL_PGA"}, \
	{"M1_L_MUX",                     "S3_L",            "S3_IL_PGA"}, \
	{"M1_L_MUX",                     "ADCL2",           "ADCL2"}, \
	{"M1_R_MUX",                     "ADCR0",           "ADCR0"}, \
	{"M1_R_MUX",                     "ADCR1",           "ADCR1"}, \
	{"M1_R_MUX",                     "DACR_S",          "DACSR_MIXER"}, \
	{"M1_R_MUX",                     "DACR_48",         "DACR_PRE_MIXER"}, \
	{"M1_R_MUX",                     "S1_R",            "S1_IR_PGA"}, \
	{"M1_R_MUX",                     "S2_R",            "S2_IR_PGA"}, \
	{"M1_R_MUX",                     "S3_R",            "S3_IR_PGA"}, \
	{"M1_R_MUX",                     "ADCL2",           "ADCL2"}, \
	{"BT_L_MUX",                     "ADCL0",           "ADCL0"}, \
	{"BT_L_MUX",                     "ADCL1",           "ADCL1"}, \
	{"BT_L_MUX",                     "DACL_48",         "DACL_PRE_MIXER"}, \
	{"BT_L_MUX",                     "S1_L",            "S1_IL_PGA"}, \
	{"BT_L_MUX",                     "S3_L",            "S3_IL_PGA"}, \
	{"BT_L_MUX",                     "S4_L",            "S4_IL_PGA"}, \
	{"BT_L_MUX",                     "ADCL2",           "ADCL2"}, \
	{"BT_R_MUX",                     "ADCR0",           "ADCR0"}, \
	{"BT_R_MUX",                     "ADCR1",           "ADCR1"}, \
	{"BT_R_MUX",                     "DACR_48",         "DACR_PRE_MIXER"}, \
	{"BT_R_MUX",                     "S1_R",            "S1_IR_PGA"}, \
	{"BT_R_MUX",                     "S3_R",            "S3_IR_PGA"}, \
	{"BT_R_MUX",                     "S4_R",            "S4_IR_PGA"}, \
	{"BT_R_MUX",                     "ADCL2",           "ADCL2"}, \
	{"US_R1_MUX",                    "ADCL0",           "ADCL0"}, \
	{"US_R1_MUX",                    "ADCL1",           "ADCL1"}, \
	{"US_R1_MUX",                    "ADCR0",           "ADCR0"}, \
	{"US_R1_MUX",                    "ADCR1",           "ADCR1"}, \
	{"US_R1_MUX",                    "ADCL2",           "ADCL2"}, \
	{"US_R2_MUX",                    "ADCL0",           "ADCL0"}, \
	{"US_R2_MUX",                    "ADCL1",           "ADCL1"}, \
	{"US_R2_MUX",                    "ADCR0",           "ADCR0"}, \
	{"US_R2_MUX",                    "ADCR1",           "ADCR1"}, \
	{"US_R2_MUX",                    "ADCL2",           "ADCL2"}, \

#define HI6405_TX_SWITCH_ROUTE \
	{"S1_OL_SWITCH",                 "ENABLE",          "MIC1_MUX"}, \
	{"S1_OR_SWITCH",                 "ENABLE",          "MIC2_MUX"}, \
	{"S2_OL_SWITCH",                 "ENABLE",          "BT_L_MUX"}, \
	{"S2_OR_SWITCH",                 "ENABLE",          "BT_R_MUX"}, \
	{"S3_OL_SWITCH",                 "ENABLE",          "MIC1_MUX"}, \
	{"S3_OR_SWITCH",                 "ENABLE",          "MIC2_MUX"}, \
	{"S4_OL_SWITCH",                 "ENABLE",          "M1_L_MUX"}, \
	{"S4_OR_SWITCH",                 "ENABLE",          "M1_R_MUX"}, \
	{"U3_OL_SWITCH",                 "ENABLE",          "MIC3_MUX"}, \
	{"U4_OR_SWITCH",                 "ENABLE",          "MIC4_MUX"}, \
	{"U5_OL_SWITCH",                 "ENABLE",          "MIC5_MUX"}, \
	{"US_R1_SWITCH",                 "ENABLE",          "US_R1_MUX"}, \
	{"US_R2_SWITCH",                 "ENABLE",          "US_R2_MUX"}, \
	{"MAD_SWITCH",                   "ENABLE",          "MIC1_MUX"}, \
	{"VIRTUAL_BTN_ACTIVE_SWITCH",    "ENABLE",          "ADCL2_VIRTBTN_IR"}, \
	{"VIRTUAL_BTN_PASSIVE_SWITCH",   "ENABLE",          "ADCL2_VIRTBTN_IR"}, \
	{"ADCL2_VIRTBTN_IR",             NULL,              "ADCL2_MUX"}, \
	{"ADCL2_VIRTBTN_IR",             NULL,              "ADCL1_MUX"}, \
	{"IR_STUDY_ENV_SWITCH",          "ENABLE",          "ADCL2_VIRTBTN_IR"}, \

#define HI6405_IV_ROUTE \
	{"IV_DSPIF_SWITCH",              "ENABLE",          "S4_RX_INPUT"}, \
	{"IV_2PA_SWITCH",                "ENABLE",          "S4_RX_INPUT"}, \
	{"IV_4PA_SWITCH",                "ENABLE",          "S4_RX_INPUT"}, \
	{"U7_DRV",                       NULL,              "IV_2PA_SWITCH"}, \
	{"U8_DRV",                       NULL,              "IV_2PA_SWITCH"}, \
	{"U5_DRV",                       NULL,              "IV_4PA_SWITCH"}, \
	{"U6_DRV",                       NULL,              "IV_4PA_SWITCH"}, \
	{"U7_DRV",                       NULL,              "IV_4PA_SWITCH"}, \
	{"U8_DRV",                       NULL,              "IV_4PA_SWITCH"}, \
	{"IV_DSPIF_OUTPUT",              NULL,              "IV_DSPIF_SWITCH"}, \

#define HI6405_EC_ROUTE \
	{"U7_EC_SWITCH",                 "ENABLE",          "U7_EC_INPUT"}, \
	{"U7_DRV",                       NULL,              "U7_EC_SWITCH"}, \
	{"U7_EC_OUTPUT",                 NULL,              "U7_DRV"}, \

#define HI6405_DAC_MIXER_ROUTE \
	{"DACL_MIXER",                   "S1_L",            "S1_IL_PGA"}, \
	{"DACL_MIXER",                   "S2_L",            "S2_IL_PGA"}, \
	{"DACL_MIXER",                   "S3_L",            "S3_IL_PGA"}, \
	{"DACL_MIXER",                   "S1_R",            "S1_IR_PGA"}, \
	{"DACR_MIXER",                   "S1_R",            "S1_IR_PGA"}, \
	{"DACR_MIXER",                   "S2_R",            "S2_IR_PGA"}, \
	{"DACR_MIXER",                   "MDM",             "MDM_MUX"}, \
	{"DACR_MIXER",                   "S1_L",            "S1_IL_PGA"}, \
	{"DACL_PRE_MIXER",               "MUX9",            "DACL_PRE_MUX"}, \
	{"DACL_PRE_MIXER",               "SIDETONE",        "SIDETONE_PGA"}, \
	{"DACR_PRE_MIXER",               "MUX10",           "DACR_PRE_MUX"}, \
	{"DACR_PRE_MIXER",               "SIDETONE",        "SIDETONE_PGA"}, \
	{"DACL_POST_MIXER",              "DACLSRC",         "DACL_PRE_MIXER"}, \
	{"DACL_POST_MIXER",              "S1_L",            "S1_IL_PGA"}, \
	{"DACR_POST_MIXER",              "DACRSRC",         "DACR_PRE_MIXER"}, \
	{"DACR_POST_MIXER",              "S1_R",            "S1_IR_PGA"}, \
	{"DACSL_MIXER",                  "UTW",             "D5_D6_INPUT"}, \
	{"DACSL_MIXER",                  "DACSL_PGA",       "DACL_PRE_MUX"}, \
	{"DACSL_MIXER",                  "MDM",             "MDM_MUX"}, \
	{"DACSL_MIXER",                  "SIDETONE",        "SIDETONE_PGA"}, \
	{"DACSR_MIXER",                  "UTW",             "D5_D6_INPUT"}, \
	{"DACSR_MIXER",                  "DACSR_PGA",       "DACR_PRE_MUX"}, \
	{"DACSR_MIXER",                  "MDM",             "MDM_MUX"}, \
	{"DACSR_MIXER",                  "SIDETONE",        "SIDETONE_PGA"}, \

#define HI6405_DSD_MIXER_ROUTE \
	{"DSDL_MIXER",                   "PGA",             "DSD_L_INPUT"}, \
	{"DSDL_MIXER",                   "DACL_384K",       "DACL_POST_MIXER"}, \
	{"DSDR_MIXER",                   "PGA",             "DSD_R_INPUT"}, \
	{"DSDR_MIXER",                   "DACR_384K",       "DACR_POST_MIXER"}, \

#define HI6405_DAC_MUX_ROUTE \
	{"DACL_PRE_MUX",                 "DACL_MIXER",      "DACL_MIXER"}, \
	{"DACR_PRE_MUX",                 "DACR_MIXER",      "DACR_MIXER"}, \
	{"DACSL_PGA_MUX",                "DACSL_384K",      "DACSL_MIXER"}, \
	{"DACL_PGA_MUX",                 "DACL_384K",       "DACL_POST_MIXER"}, \
	{"DACL_PGA_MUX",                 "S1_L",            "PLAY384K_VIR_SWITCH"}, \
	{"DACL_PGA_MUX",                 "ADCL1",           "ADCL1"},\
	{"DACR_PGA_MUX",                 "DACR_384K",       "DACR_POST_MIXER"}, \
	{"DACR_PGA_MUX",                 "S1_R",            "PLAY384K_VIR_SWITCH"}, \
	{"DACR_PGA_MUX",                 "ADCR1",           "ADCR1"},\

#define HI6405_RX_MUX_ROUTE \
	{"EP_L_SDM_MUX",                 "DACSL_UP16",      "DACSL_PGA_MUX"}, \
	{"HP_L_SDM_MUX",                 "DSD_L",           "DSDL_MIXER"}, \
	{"HP_L_SDM_MUX",                 "DACL_UP16",       "DACL_PGA_MUX"}, \
	{"HP_R_SDM_MUX",                 "DSD_R",           "DSDR_MIXER"}, \
	{"HP_R_SDM_MUX",                 "DACR_UP16",       "DACR_PGA_MUX"}, \
	{"HP_DAC_L_MUX",                 "HP_L_SDM",        "HP_L_SDM_MUX"}, \
	{"HP_DAC_L_MUX",                 "HP_R_SDM",        "HP_R_SDM_MUX"}, \
	{"HP_DAC_R_MUX",                 "HP_L_SDM",        "HP_L_SDM_MUX"}, \
	{"HP_DAC_R_MUX",                 "HP_R_SDM",        "HP_R_SDM_MUX"}, \
	{"EP_DAC_MUX",                   "EP_L_SDM",        "EP_L_SDM_MUX"}, \

#define HI6405_SX_INPUT_MUX_ROUTE \
	{"S1_IL_MUX",                    "D1",              "D1_D2_INPUT"}, \
	{"S1_IL_MUX",                    "S1_INL",          "S1_RX_INPUT"}, \
	{"S1_IR_MUX",                    "D2",              "D1_D2_INPUT"}, \
	{"S1_IR_MUX",                    "S1_INR",          "S1_RX_INPUT"}, \
	{"S2_IL_MUX",                    "D3",              "D3_D4_INPUT"}, \
	{"S2_IL_MUX",                    "S2_INL",          "S2_RX_INPUT"}, \
	{"S2_IR_MUX",                    "D4",              "D3_D4_INPUT"}, \
	{"S2_IR_MUX",                    "S2_INR",          "S2_RX_INPUT"}, \

#define HI6405_SX_PGA_ROUTE \
	{"S1_IL_PGA",                    NULL,              "S1_IL_MUX"}, \
	{"S1_IR_PGA",                    NULL,              "S1_IR_MUX"}, \
	{"S2_IL_PGA",                    NULL,              "S2_IL_MUX"}, \
	{"S2_IR_PGA",                    NULL,              "S2_IR_MUX"}, \
	{"S3_IL_PGA",                    NULL,              "D5_D6_INPUT"}, \
	{"S3_IR_PGA",                    NULL,              "D5_D6_INPUT"}, \
	{"S4_IL_PGA",                    NULL,              "S4_RX_INPUT"}, \
	{"S4_IR_PGA",                    NULL,              "S4_RX_INPUT"}, \

#define HI6405_MDM_MUX_ROUTE \
	{"MDM_MUX",                     "S3_L",             "S3_IL_PGA"}, \
	{"MDM_MUX",                     "S3_R",             "S3_IR_PGA"}, \

#define HI6405_IR_TX_MUX_ROUTE \
	{"IR_MUX",                      "DSPIF8",           "IR_TX_INPUT"}, \
	{"IR_MUX",                      "IR_REG_CTRL",      "IR_TX_INPUT"}, \
	{"IR_MUX",                      "IO_TEST_IN",       "IR_TX_INPUT"}, \
	{"IR_EMISSION_SWITCH",          "ENABLE",           "IR_MUX"}, \

/* mmi_route */
#define HI6405_MMI_ROUTE \
	{"I2S2_BLUETOOTH_LOOP_SWITCH",      "ENABLE",     "I2S2_BLUETOOTH_LOOP_INPUT"},\
	{"I2S2_BLUETOOTH_LOOP_OUTPUT",      NULL,         "I2S2_BLUETOOTH_LOOP_SWITCH"},\
	{"TDM_AUDIO_PA_DN_LOOP_SWITCH",  "ENABLE",          "TDM_AUDIO_PA_DOWN_INPUT"}, \
	{"TDM_AUDIO_PA_DOWN_OUTPUT",     NULL,           "TDM_AUDIO_PA_DN_LOOP_SWITCH"}, \

static const struct snd_soc_dapm_route hi6405_route_map[] = {
	HI6405_DP_SUPPLY_ROUTE
	HI6405_PLL_SUPPLY_ROUTE
	HI6405_MAD_SUPPLY_ROUTE
	HI6405_44K1_SUPPLY_ROUTE
	HI6405_I2S_SUPPLY_ROUTE
	HI6405_CP1_SUPPLY_ROUTE
	HI6405_CP2_SUPPLY_ROUTE
	HI6405_PLL_TEST_ROUTE
	HI6405_VOICE_SWITCH_ROUTE
	HI6405_SOUNDTRIGGER_SWITCH_ROUTE
	HI6405_AUDIOUP_SWITCH_ROUTE
	HI6405_HP_HIGHLEVEL_SWITCH_ROUTE
	HI6405_HP_CONCURRENCY_SWITCH_ROUTE
	HI6405_MICBIAS_ROUTE
	HI6405_MICPGA_ROUTE
	HI6405_TX_COMMON_MUX_ROUTE
	HI6405_ADC_ROUTE
	HI6405_ADC_MUX_ROUTE
	HI6405_TX_SWITCH_ROUTE
	HI6405_TX_OUTPUT_ROUTE
	HI6405_AUDIODOWN_SWITCH_ROUTE
	HI6405_FM_SWITCH_ROUTE
	HI6405_IV_ROUTE
	HI6405_EC_ROUTE
	HI6405_DAC_MUX_ROUTE
	HI6405_DAC_MIXER_ROUTE
	HI6405_DSD_MIXER_ROUTE
	HI6405_RX_MUX_ROUTE
	HI6405_RX_OUTPUT_ROUTE
	HI6405_SX_INPUT_MUX_ROUTE
	HI6405_SX_PGA_ROUTE
	HI6405_MDM_MUX_ROUTE
	HI6405_IR_TX_MUX_ROUTE
	HI6405_MMI_ROUTE
	HI6405_MOVEUP_PA_TDM_SWITCH_ROUTE
};

static enum hi64xx_pll_type hi6405_pll_for_reg_access(struct snd_soc_codec *codec, unsigned int reg)
{
	if ((reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_DIG + HI6405_ADDR_DIG_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_DIG + HI6405_ADDR_DIG_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + HI6405_ADDR_CFG_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_CFG + HI6405_ADDR_CFG_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO1 + HI6405_ADDR_GPIO1_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO1 + HI6405_ADDR_GPIO1_OFFSET_END))
		|| (reg >= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO2 + HI6405_ADDR_GPIO2_OFFSET_START)
		&& reg <= (CODEC_BASE_ADDR + BASE_ADDR_PAGE_GPIO2 + HI6405_ADDR_GPIO2_OFFSET_END))) {
		return PLL_HIGH;
	} else {
		return PLL_NONE;
	}
}

static unsigned int hi6405_virtual_reg_read(struct hi6405_platform_data *platform_data,
		unsigned int v_addr)
{
	unsigned int ret = 0;
	unsigned long flag;

	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	if (HI6405_VIRTUAL_DOWN_REG == v_addr)
		ret = platform_data->virtual_reg[HI6405_VIR_UP];
	else if (HI6405_VIRTUAL_UP_REG == v_addr)
		ret = platform_data->virtual_reg[HI6405_VIR_DOWN];

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);

	return ret;
}

static unsigned int hi6405_reg_read(struct snd_soc_codec *codec,
		unsigned int reg)
{
	unsigned int ret = 0;
	unsigned int reg_mask;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	WARN_ON(!platform_data);

	reg_mask = reg & 0xFFFFF000;
	if (BASE_ADDR_PAGE_CFG == reg_mask || BASE_ADDR_PAGE_IO == reg_mask) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (BASE_ADDR_PAGE_VIRTUAL == reg_mask) {
		ret = hi6405_virtual_reg_read(platform_data, reg);
		return ret;
	}

	if(!platform_data->resmgr) {
		HI6405_LOGE("platform_data->resmgr is null");
		return 0;
	}

	hi64xx_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_read(platform_data->cdc_ctrl, reg);
	hi64xx_resmgr_release_reg_access(platform_data->resmgr, reg);
	return ret;
}

static void hi6405_virtual_reg_write(struct hi6405_platform_data *platform_data,
		unsigned int v_addr, unsigned int value)
{
	unsigned long flag;
	spin_lock_irqsave(&platform_data->v_rw_lock, flag);

	if (HI6405_VIRTUAL_DOWN_REG == v_addr)
		platform_data->virtual_reg[HI6405_VIR_UP] = value;
	else if (HI6405_VIRTUAL_UP_REG == v_addr)
		platform_data->virtual_reg[HI6405_VIR_DOWN] = value;

	spin_unlock_irqrestore(&platform_data->v_rw_lock, flag);
}

static int hi6405_reg_write(struct snd_soc_codec *codec,
		unsigned int reg, unsigned int value)
{
	int ret = 0;
	unsigned int reg_mask;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	WARN_ON(NULL == platform_data);

	reg_mask = reg & 0xFFFFF000;
	if (BASE_ADDR_PAGE_CFG == reg_mask || BASE_ADDR_PAGE_IO == reg_mask) {
		reg = reg | CODEC_BASE_ADDR;
	} else if (BASE_ADDR_PAGE_VIRTUAL == reg_mask ) {
		hi6405_virtual_reg_write(platform_data, reg, value);
		return 0;
	}

	if(!platform_data->resmgr) {
		HI6405_LOGE("platform_data->resmgr null");
		return -EFAULT;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_reg_rw_cache(reg, value, HICODEC_DEBUG_FLAG_WRITE);
#endif

	hi64xx_resmgr_request_reg_access(platform_data->resmgr, reg);
	ret = hi_cdcctrl_reg_write(platform_data->cdc_ctrl, reg, value);
	hi64xx_resmgr_release_reg_access(platform_data->resmgr, reg);
	return ret;
}

static bool hi6405_pll48k_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	int i = 0;
	unsigned int regval;
	WARN_ON(NULL == platform_data);

	udelay(200);
	/* check pll48k lock state */
	while (!((snd_soc_read(codec, HI6405_IRQ_ANA_2_REG) & (0x1<<HI6405_MAIN1_PLL_LOCK_F_OFFSET))
		&& (snd_soc_read(codec, HI64xx_VERSION_REG) == HI6405_VERSION_VALUE))) {
		udelay(200);
		if (++i == 50) {
			HI6405_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	HI6405_LOGI("check time is %d, pllstate:0x%x\n",
		i, regval);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	return true;
}

static int hi6405_pll48k_config(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_154, 0x11);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_156, 0xEB);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_157, 0x80);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_150, 0x51);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_152, 0x40);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_159, 0x9A);
	return 0;
}

static int hi6405_pll48k_turn_on(struct snd_soc_codec *codec)
{
	HI6405_LOGI("turn on pll 48k");
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_153, 0x80);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_150, 0x41);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_149, 0x10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_148, 0x28);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_151, 0x78);
	udelay(25);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_149, 0xD0);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_148, 0xA8);
	return 0;
}

static int hi6405_pll48k_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;
	WARN_ON(NULL == platform_data);

	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_148, 0x2F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_151, 0x42);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_150, 0x51);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_153, 0x0);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_149, 0x10);

	regval = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	HI6405_LOGI("pll 48k off, pllstate:0x%x \n", regval);

	return 0;
}

static bool hi6405_pll44k1_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval[2] = {0};
	int i = 0;

	WARN_ON(!platform_data);

	udelay(200);
	/* check pll44k1 lock state */
	while (!(snd_soc_read(codec, HI6405_IRQ_ANA_2_REG) & (0x1<<HI6405_MAIN2_PLL_LOCK_F_OFFSET))) {
		udelay(200);
		if (++i == 50) {
			HI6405_LOGE("check time is %d", i);
			return false;
		}
	}

	regval[0] = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	regval[1] = snd_soc_read(codec, HI64xx_VERSION_REG);
	HI6405_LOGI("check time is %d, pllstate:0x%x, version:0x%x", i, regval[0], regval[1]);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	return true;
}

static int hi6405_pll44k1_config(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_177, 0x0E);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_179, 0xB3);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_180, 0x30);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_175, 0x00);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_182, 0x9A);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_176, 0x80);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_173, 0x40);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_171, 0x0F);
	OUT_FUNCTION
	return 0;
}

static int hi6405_pll44k1_turn_on(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	HI6405_LOGI("turn on pll 44.1k");
	/* 44.1 clk enable */
	hi64xx_update_bits(codec, HI6405_AUD_CLK_EN_REG,
			0x1<<HI6405_AUD_44P1K_SEL2_OFFSET | 0x1<<HI6405_AUD_44P1K_SEL0_OFFSET,
			0x1<<HI6405_AUD_44P1K_SEL2_OFFSET | 0x1<<HI6405_AUD_44P1K_SEL0_OFFSET);
	/* pll enable */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_172, 0x10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_171, 0x8);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_174, 0x78);
	udelay(25);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_172, 0xD0);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_171, 0x88);

	OUT_FUNCTION
	return 0;
}

static int hi6405_pll44k1_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;
	WARN_ON(NULL == platform_data);

	IN_FUNCTION
	HI6405_LOGI("turn off pll 44.1k");

	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	/* pll disable */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_171, 0x2F);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_174, 0x40);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_172, 0xD0);

	/* 44.1 clk disable */
	hi64xx_update_bits(codec, HI6405_AUD_CLK_EN_REG,
			0x1<<HI6405_AUD_44P1K_SEL2_OFFSET | 0x1<<HI6405_AUD_44P1K_SEL0_OFFSET,
			0);

	regval = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	HI6405_LOGI("44k1 off, pllstate:0x%x \n", regval);

	OUT_FUNCTION
	return 0;
}

static bool hi6405_pllmad_check(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;
	int i = 0;
	WARN_ON(NULL == platform_data);

	msleep(5);
	while (!(snd_soc_read(codec, HI6405_IRQ_ANA_2_REG) & (0x1<<HI6405_MAD_PLL_LOCK_F_OFFSET))) {
		msleep(5);
		if (++i == 10) {
			HI6405_LOGE("check time is %d", i);
			return false;
		}
	}
	regval = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	HI6405_LOGI("check time is %d, pllstate:0x%x\n", i, regval);
	hi64xx_irq_enable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);
	return true;
}

static int hi6405_pllmad_config(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_209, 0xBB);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_210, 0x80);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_203, 0x59);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_213, 0x28);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_205, 0x08);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_147, 0xC8);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_204, 0xF9);
	return 0;
}

static int hi6405_pllmad_turn_on(struct snd_soc_codec *codec)
{
	HI6405_LOGI("turn on pll mad");
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_204, 0xF8);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_203, 0x41);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_147, 0xC0);
	udelay(25);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_204, 0xF9);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_203, 0x43);
	hi64xx_update_bits(codec, HI6405_CLK_SOURCE_SW_REG,
		1<<HI6405_CLK_12288_SW_OFFSET, 1<<HI6405_CLK_12288_SW_OFFSET);
	/* clk source select -> mad pll */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140, 0x3f, 0x3f);
	return 0;
}

static int hi6405_pllmad_turn_off(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	unsigned int regval;

	WARN_ON(NULL == platform_data);

	HI6405_LOGI("turn off pll mad");
	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	/* clk source select -> main1 pll */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140, 0x3f, 0);
	hi64xx_update_bits(codec, HI6405_CLK_SOURCE_SW_REG,
		1<<HI6405_CLK_12288_SW_OFFSET, 0x0);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_203, 0x59);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_147, 0xC8);

	regval = snd_soc_read(codec, HI6405_IRQ_ANA_2_REG);
	HI6405_LOGI("mad off, pllstate:0x%x \n", regval);
	return 0;
}

static int hi6405_enable_regulator(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_104, 0x80);/* en GBR */
	msleep(10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_127, 0x40);/* en fast start */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_130, 0x04);/* cfg LDOTRX to 1.8 */
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_104, 0x84);/* en LDOTRX */

	/* iso_a18_d11/iso_a33_d11 enable */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_049,
			0x1<<HI6405_AVREF_ESD_SW11_OFFSET, 0x1<<HI6405_AVREF_ESD_SW11_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_049,
			0x1<<HI6405_ISO_A18_D11_REG_OFFSET, 0x1<<HI6405_ISO_A18_D11_REG_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_049,
			0x1<<HI6405_ISO_A33_D11_REG_OFFSET, 0x1<<HI6405_ISO_A33_D11_REG_OFFSET);
	return 0;
}

static int hi6405_enable_supply(struct snd_soc_codec *codec)
{
	HI6405_LOGI("enable supply");
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_146, 0x19);/* pll ibias en */
	return 0;
}

static int hi6405_disable_supply(struct snd_soc_codec *codec)
{
	HI6405_LOGI("disable supply");
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_146, 0x59);/* pll bias disable */
	return 0;
}

static int hi6405_enable_ibias(struct snd_soc_codec *codec)
{
	HI6405_LOGI("enable ibias");
	/* ibias on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_000, 1<<HI6405_IBIAS_PD_OFFSET, 0);
	return 0;
}

static int hi6405_disable_ibias(struct snd_soc_codec *codec)
{
	HI6405_LOGI("disable ibias");

	/* ibias off */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_000,
			0x1<<HI6405_IBIAS_PD_OFFSET, 0x1<<HI6405_IBIAS_PD_OFFSET);
	return 0;
}

static int hi6405_enable_micbias(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == data);

	HI6405_LOGI("hs micbias enable");
	/* mask btn irqs */
	hi64xx_irq_mask_btn_irqs(data->mbhc);
	/* eco off */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_080, 1<<HI6405_MBHD_ECO_EN_BIT, 0);
	/* mbhc cmp on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012, 1<<HI6405_MBHD_PD_NORMCOMP, 0);
	/* hsmic chg */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_073, 1<<HI6405_HSMICB2_DSCHG, 0);
	/* hsmic on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_013, 1<<HI6405_HS_MICB_PD_BIT, 0);
	msleep(20);
	/* unmask btn irqs */
	hi64xx_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}

static int hi6405_disable_micbias(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *data = snd_soc_codec_get_drvdata(codec);
	WARN_ON(NULL == data);

	/* mask btn irqs */
	hi64xx_irq_mask_btn_irqs(data->mbhc);
	/* hsmic pd */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_013, 1<<HI6405_HS_MICB_PD_BIT, 1<<HI6405_HS_MICB_PD_BIT);
	/* hsmic dischg */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_073, 1<<HI6405_HSMICB2_DSCHG, 1<<HI6405_HSMICB2_DSCHG);
	msleep(15);
	/* hsmic chg */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_073, 1<<HI6405_HSMICB2_DSCHG, 0);
	HI6405_LOGI("hs micbias disable");
	/* eco on & eco auto saradc on */
	/* mbhc cmp off */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012, 1<<HI6405_MBHD_PD_NORMCOMP, 1<<HI6405_MBHD_PD_NORMCOMP);
	/* eco on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_080, 1<<HI6405_MBHD_ECO_EN_BIT, 1<<HI6405_MBHD_ECO_EN_BIT);
	HI6405_LOGI("eco enable");
	msleep(20);

	/* unmask btn irqs */
	hi64xx_irq_unmask_btn_irqs(data->mbhc);

	return 0;
}

static void hi6405_buck_init(struct snd_soc_codec *codec)
{
	IN_FUNCTION
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_235, 0x10);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_224, 0x66);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_217, 0x74);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_218, 0x20);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_219, 0x07);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_220, 0x05);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_221, 0x03);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_222, 0x02);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_223, 0x01);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_225, 0x33);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_226, 0x39);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_227, 0x74);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_228, 0x20);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_229, 0x78);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_230, 0x58);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_231, 0x38);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_232, 0x1A);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_233, 0x0C);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_234, 0x01);

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_128, 0x07);
	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_129, 0x07);

	snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_090, 0xDC); /* fix high-low temperature and ATE */
	OUT_FUNCTION
}

static void hi6405_ioshare_init(struct snd_soc_codec *codec, struct hi6405_platform_data *platform_data)
{
	if (BUSTYPE_SELECT_SLIMBUS == platform_data->cdc_ctrl->bus_sel) {
		snd_soc_write(codec, HI6405_IOS_MF_CTRL1_REG, 1<<HI6405_IOS_MF_CTRL1_OFFSET);
		snd_soc_write(codec, HI6405_IOS_MF_CTRL3_REG, 2<<HI6405_IOS_MF_CTRL3_OFFSET);
		snd_soc_write(codec, HI6405_IOS_MF_CTRL4_REG, 2<<HI6405_IOS_MF_CTRL4_OFFSET);
		snd_soc_write(codec, HI6405_IOS_IOM_CTRL7_REG, 0x114);
		snd_soc_write(codec, HI6405_IOS_IOM_CTRL8_REG, 0x115);
		/* enable hi6405 slim framer */
		hi64xx_update_bits(codec, HI6405_SLIM_CTRL1_REG,
			0x1<<HI6405_SLIM_CLK_EN_OFFSET, 0x1<<HI6405_SLIM_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_SLIM_CTRL1_REG,
			0x1<<HI6405_SLIM_SWIRE_DIV_EN_OFFSET, 0x1<<HI6405_SLIM_SWIRE_DIV_EN_OFFSET);
	}

	/* ssi ioshare config */
	snd_soc_write(codec, HI6405_IOS_IOM_CTRL5_REG, 0x10D);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL1_REG, 1<<HI6405_IOS_MF_CTRL1_OFFSET);

	/* slimbus ioshare config */
	snd_soc_write(codec, HI6405_IOS_MF_CTRL3_REG, 2<<HI6405_IOS_MF_CTRL3_OFFSET);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL4_REG, 2<<HI6405_IOS_MF_CTRL4_OFFSET);

	/* I2S2 ioshare config */
	snd_soc_write(codec, HI6405_IOS_MF_CTRL7_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL8_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL9_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL10_REG, 0x01);
	/* I2S4 ioshare config */
	snd_soc_write(codec, HI6405_IOS_MF_CTRL11_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL12_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL13_REG, 0x01);
	snd_soc_write(codec, HI6405_IOS_MF_CTRL14_REG, 0x01);
}

static void hi6405_slimbus_init(struct snd_soc_codec *codec, struct hi6405_platform_data *platform_data)
{
	/* slim&ssi mclk enable */
	hi64xx_update_bits(codec, HI6405_CFG_CLK_CTRL_REG, 0x1<<HI6405_INTF_SSI_CLK_EN_OFFSET,
			0x1<<HI6405_INTF_SSI_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_CFG_CLK_CTRL_REG, 0x1<<HI6405_INTF_SLIM_CLK_EN_OFFSET,
			0x1<<HI6405_INTF_SLIM_CLK_EN_OFFSET);

	snd_soc_write(codec, HI6405_SLIM_CTRL3_REG, 0xBF);

	/* slimbus clk schmitt config */
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL8_REG,
			0x3<<HI6405_ST_OFFSET, 0x1<<HI6405_ST_OFFSET);
	/* slimbus pin disable pd */
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL7_REG,
			0x1<<HI6405_PD_OFFSET, 0x0<<HI6405_PD_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL8_REG,
			0x1<<HI6405_PD_OFFSET, 0x0<<HI6405_PD_OFFSET);

	/* slimbus drv codec side */
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL7_REG,
			0x7<<HI6405_DS_OFFSET,
			platform_data->cdc_ctrl->slimbusdata_cdc_drv<<HI6405_DS_OFFSET);

	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL8_REG,
			0x7<<HI6405_DS_OFFSET,
			platform_data->cdc_ctrl->slimbusclk_cdc_drv<<HI6405_DS_OFFSET);

	/* slimbus frame config */
	snd_soc_write(codec, HI6405_SLIM_CTRL0_REG, 0x6);

	/* slimbus up1&2 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_3_REG, 0x44);
	/* slimbus up3&4 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_4_REG, 0x44);
	/* slimbus up7&8 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_6_REG, 0x44);
	/* slimbus up9&10 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_7_REG, 0x44);

	/* slimbus dn1&dn2 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_0_REG, 0x44);
	/* slimbus dn5&dn6 port fs config */
	snd_soc_write(codec, HI6405_SC_FS_SLIM_CTRL_2_REG, 0x44);
}

static void hi6405_pll_lock_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_165,
		HI6405_CODEC_ANA_MAIN1_LOCK_DET_MODE_ONLY_F<<HI6405_CODEC_ANA_MAIN1_LOCK_DET_MODE_OFFSET,
		HI6405_CODEC_ANA_MAIN1_LOCK_DET_MODE_ONLY_F<<HI6405_CODEC_ANA_MAIN1_LOCK_DET_MODE_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_166,
		HI6405_CODEC_ANA_MAIN1_LCK_PREC_64<<HI6405_CODEC_ANA_MAIN1_LCK_PREC_OFFSET,
		HI6405_CODEC_ANA_MAIN1_LCK_PREC_64<<HI6405_CODEC_ANA_MAIN1_LCK_PREC_OFFSET);

	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_188,
		HI6405_CODEC_ANA_MAIN2_LOCK_DET_MODE_ONLY_F<<HI6405_CODEC_ANA_MAIN2_LOCK_DET_MODE_OFFSET,
		HI6405_CODEC_ANA_MAIN2_LOCK_DET_MODE_ONLY_F<<HI6405_CODEC_ANA_MAIN2_LOCK_DET_MODE_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_189,
		HI6405_CODEC_ANA_MAIN2_LCK_PREC_64<<HI6405_CODEC_ANA_MAIN2_LCK_PREC_OFFSET,
		HI6405_CODEC_ANA_MAIN2_LCK_PREC_64<<HI6405_CODEC_ANA_MAIN2_LCK_PREC_OFFSET);
}

static void hi6405_clk_cfg_init(struct snd_soc_codec *codec)
{
	/* 12M288&6M144 clk enable */
	hi64xx_update_bits(codec, HI6405_AUD_CLK_EN_REG,
			0x1<<HI6405_AUD_12M_CLK_EN_OFFSET, 0x1<<HI6405_AUD_12M_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_AUD_CLK_EN_REG,
			0x1<<HI6405_AUD_6144K_CLK_EN_OFFSET, 0x1<<HI6405_AUD_6144K_CLK_EN_OFFSET);
	/* 11M289&5M644 clk enable */
	hi64xx_update_bits(codec, HI6405_AUD_CLK_EN_REG,
			0x1<<HI6405_AUD_11M_CLK_EN_OFFSET | 0x1<<HI6405_AUD_56448K_CLK_EN_OFFSET,
			0x1<<HI6405_AUD_11M_CLK_EN_OFFSET | 0x1<<HI6405_AUD_56448K_CLK_EN_OFFSET);
	/* clk source select -> main1 pll */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_140, 0x3f, 0x0);

	/* pll lock mode cfg */
	hi6405_pll_lock_init(codec);
}

static void hi6405_efuse_init(struct snd_soc_codec *codec)
{
	unsigned int inf_ate_ctrl;
	unsigned int inf_trim_ctrl;
	unsigned int bgr_ate;
	unsigned int die_id0;
	unsigned int die_id1;

	/* enable efuse */
	hi64xx_update_bits(codec, HI6405_DIE_ID_CFG1_REG,
			0x1<<HI6405_EFUSE_MODE_SEL_OFFSET, 0x1<<HI6405_EFUSE_MODE_SEL_OFFSET);
	hi64xx_update_bits(codec, HI6405_CFG_CLK_CTRL_REG,
			0x1<<HI6405_EFUSE_CLK_EN_OFFSET, 0x1<<HI6405_EFUSE_CLK_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DIE_ID_CFG1_REG,
			0x1<<HI6405_EFUSE_READ_EN_OFFSET, 0x1<<HI6405_EFUSE_READ_EN_OFFSET);

	msleep(5);

	/* default para set */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_052,
			0x1<<HI6405_CODEC_OTPREG_SEL_FIR_OFFSET, 0x0<<HI6405_CODEC_OTPREG_SEL_FIR_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_053,
			0x1<<HI6405_CODEC_OTPREG_SEL_INF_OFFSET, 0x0<<HI6405_CODEC_OTPREG_SEL_INF_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_133,
			0x1<<HI6405_CODEC_OTPREG_SEL_BGR_OFFSET, 0x0<<HI6405_CODEC_OTPREG_SEL_BGR_OFFSET);

	die_id0 = snd_soc_read(codec, HI6405_DIE_ID_OUT_DATA0_REG);
	die_id1 = snd_soc_read(codec, HI6405_DIE_ID_OUT_DATA1_REG);
	bgr_ate = snd_soc_read(codec, HI6405_DIE_ID_OUT_DATA2_REG);

	inf_ate_ctrl = die_id0 & 0xf;
	inf_trim_ctrl = ((die_id0 & 0xf0)>>4) | ((die_id1 & 0x7)<<4);

	HI6405_LOGI("efuse inf ate: 0x%x, inf trim: 0x%x, bgr ate0x%x\n",
			inf_ate_ctrl, inf_trim_ctrl, bgr_ate);

}

static int hi6405_slim_enumerate(struct hi64xx_resmgr *resmgr)
{
	int ret = 0;

	/* open codec pll and soc asp clk to make sure codec framer be enumerated */
	ret = hi64xx_resmgr_request_pll(resmgr, PLL_HIGH);
	if (ret) {
		HI6405_LOGE("hi64xx_resmgr_request_pll failed");
		return ret;
	}
	ret = slimbus_track_activate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_PLAY, NULL);
	if (ret) {
		HI6405_LOGE("slimbus_track_activate audio play failed");
	}
	msleep(1);
	ret = slimbus_track_deactivate(SLIMBUS_DEVICE_HI6405, SLIMBUS_6405_TRACK_AUDIO_PLAY, NULL);
	if (ret) {
		HI6405_LOGE("slimbus_track_deactivate audio play failed");
	}
	ret = hi64xx_resmgr_release_pll(resmgr, PLL_HIGH);
	if (ret) {
		HI6405_LOGE("hi64xx_resmgr_release_pll failed");
	}
	return ret;
}

static int hi6405_utils_init(struct hi6405_platform_data *platform_data)
{
	int ret = 0;
	struct utils_config cfg;

	cfg.hi64xx_dump_reg = NULL;
	ret = hi64xx_utils_init(platform_data->codec, platform_data->cdc_ctrl,
				&cfg, platform_data->resmgr, HI64XX_CODEC_TYPE_6405);

	return ret;
}

static int hi6405_resmgr_init(struct hi6405_platform_data *platform_data)
{
	int ret = 0;
	struct resmgr_config cfg = {0};

	cfg.pll_num = 3;
	cfg.pll_sw_mode = MODE_MULTIPLE;
	cfg.pfn_pll_ctrls[PLL_LOW].turn_on = hi6405_pllmad_turn_on;
	cfg.pfn_pll_ctrls[PLL_LOW].turn_off = hi6405_pllmad_turn_off;
	cfg.pfn_pll_ctrls[PLL_LOW].is_locked = hi6405_pllmad_check;
	cfg.pfn_pll_ctrls[PLL_HIGH].turn_on = hi6405_pll48k_turn_on;
	cfg.pfn_pll_ctrls[PLL_HIGH].turn_off = hi6405_pll48k_turn_off;
	cfg.pfn_pll_ctrls[PLL_HIGH].is_locked = hi6405_pll48k_check;
	cfg.pfn_pll_ctrls[PLL_44_1].turn_on = hi6405_pll44k1_turn_on;
	cfg.pfn_pll_ctrls[PLL_44_1].turn_off = hi6405_pll44k1_turn_off;
	cfg.pfn_pll_ctrls[PLL_44_1].is_locked = hi6405_pll44k1_check;
	cfg.enable_supply = hi6405_enable_supply;
	cfg.disable_supply = hi6405_disable_supply;
	cfg.enable_ibias = hi6405_enable_ibias;
	cfg.disable_ibias = hi6405_disable_ibias;
	cfg.enable_micbias = hi6405_enable_micbias;
	cfg.disable_micbias = hi6405_disable_micbias;
	cfg.pll_for_reg_access = hi6405_pll_for_reg_access;

	ret = hi64xx_resmgr_init(platform_data->codec, platform_data->cdc_ctrl,
				platform_data->irqmgr, &cfg, &platform_data->resmgr);

	return ret;
}

static void hi6405_set_mad_param(struct snd_soc_codec *codec, struct hi6405_board_cfg *board_cfg)
{
	/* auto active time */
	snd_soc_write(codec, HI6405_MAD_AUTO_ACT_TIME_REG, 0x0);

	/* pll time */
	snd_soc_write(codec, HI6405_MAD_PLL_TIME_L_REG, 0x1);

	/* adc time */
	snd_soc_write(codec, HI6405_MAD_ADC_TIME_H_REG, 0x0);
	snd_soc_write(codec, HI6405_MAD_ADC_TIME_L_REG, 0x3);

	/* mad_ana_time */
	snd_soc_write(codec, HI6405_MAD_ANA_TIME_H_REG, 0x0);
	snd_soc_write(codec, HI6405_MAD_ANA_TIME_L_REG, 0x5);

	/* omt */
	snd_soc_write(codec, HI6405_MAD_OMIT_SAMP_REG, 0x20);

	/* mad_vad_time */
	snd_soc_write(codec, HI6405_MAD_VAD_TIME_H_REG, 0x0);
	snd_soc_write(codec, HI6405_MAD_VAD_TIME_L_REG, 0xa0);

	/* mad_sleep_time */
	snd_soc_write(codec, HI6405_MAD_SLEEP_TIME_L_REG, 0x0);

	/* mad_buffer_fifo_thre */
	if (board_cfg->wakeup_hisi_algo_support)
		snd_soc_write(codec, HI6405_MAD_BUFFER_CTRL0_REG, 0x3f);
	else
		snd_soc_write(codec, HI6405_MAD_BUFFER_CTRL0_REG, 0x7f);
	hi64xx_update_bits(codec, HI6405_MAD_BUFFER_CTRL1_REG, 0x1f, 0x1f);

	/* mad_cnt_thre,vad delay cnt */
	snd_soc_write(codec, HI6405_MAD_CNT_THRE_REG, 0x2);

	/* mad_snr_thre */
	snd_soc_write(codec, HI6405_MAD_SNR_THRE_SUM_REG, 0x32);
	snd_soc_write(codec, HI6405_MAD_SNR_THRE_REG, 0x20);

	/* mad_min_chan_eng */
	snd_soc_write(codec, HI6405_MAD_MIN_CHAN_ENG_REG, 0x14);

	/* mad_ine */
	snd_soc_write(codec, HI6405_MAD_INE_REG, 0x14);
	/* mad_band_thre */
	snd_soc_write(codec, HI6405_MAD_BAND_THRE_REG, 0x8);
	/* mad_scale */
	snd_soc_write(codec, HI6405_MAD_SCALE_REG, 0x3);

	/* mad_vad_num */
	snd_soc_write(codec, HI6405_MAD_VAD_NUM_REG, 0x1);
	/* mad_alpha_en1 */
	snd_soc_write(codec, HI6405_MAD_ALPHA_EN1_REG, 0xc);

	/* mad_vad_ao ->en, mad_irq_en->en, mad_en->en, mad_wind_sel */
	snd_soc_write(codec, HI6405_MAD_CTRL_REG, 0x63);

	/* mad capless config */
	snd_soc_write(codec, HI6405_ANA_MAD_CAPLESS_MIXER, 0x0);
	snd_soc_write(codec, HI6405_ANA_MAD_PGA_CAPLESS_MIXER, 0x0);

	return;
}

static void hi6405_set_dsp_if_bypass(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL22_REG,
		1<<HI6405_S3_O_DSP_BYPASS_OFFSET | 1<<HI6405_S2_O_DSP_BYPASS_OFFSET |
		1<<HI6405_S1_O_DSP_BYPASS_OFFSET,
		1<<HI6405_S3_O_DSP_BYPASS_OFFSET | 1<<HI6405_S2_O_DSP_BYPASS_OFFSET |
		1<<HI6405_S1_O_DSP_BYPASS_OFFSET);
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL8_REG,
		1<<HI6405_S4_I_DSP_BYPASS_OFFSET | 1<<HI6405_S3_I_DSP_BYPASS_OFFSET |
		1<<HI6405_S2_I_DSP_BYPASS_OFFSET | 1<<HI6405_S1_I_DSP_BYPASS_OFFSET,
		1<<HI6405_S4_I_DSP_BYPASS_OFFSET | 1<<HI6405_S3_I_DSP_BYPASS_OFFSET |
		1<<HI6405_S2_I_DSP_BYPASS_OFFSET | 1<<HI6405_S1_I_DSP_BYPASS_OFFSET);
}

static void hi6405_pga_fade_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_DACL_MIXER4_CTRL1_REG,
		0x1<<HI6405_DACL_MIXER4_FADE_EN_OFFSET,
		0x1<<HI6405_DACL_MIXER4_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_MIXER4_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_MIXER4_FADE_IN_LEN, HI6405_DACL_MIXER4_FADE_IN_OFFSET),
		0xf<<HI6405_DACL_MIXER4_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_MIXER4_CTRL4_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_MIXER4_FADE_OUT_LEN, HI6405_DACL_MIXER4_FADE_OUT_OFFSET),
		0xf<<HI6405_DACL_MIXER4_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACR_MIXER4_CTRL1_REG,
		0x1<<HI6405_DACR_MIXER4_FADE_EN_OFFSET,
		0x1<<HI6405_DACR_MIXER4_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_MIXER4_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_MIXER4_FADE_IN_LEN, HI6405_DACR_MIXER4_FADE_IN_OFFSET),
		0xf<<HI6405_DACR_MIXER4_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_MIXER4_CTRL4_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_MIXER4_FADE_OUT_LEN, HI6405_DACR_MIXER4_FADE_OUT_OFFSET),
		0xf<<HI6405_DACR_MIXER4_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACL_PRE_MIXER2_CTRL1_REG,
		0x1<<HI6405_DACL_PRE_MIXER2_FADE_EN_OFFSET,
		0x1<<HI6405_DACL_PRE_MIXER2_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_PRE_MIXER2_CTRL2_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_PRE_MIXER2_FADE_IN_LEN, HI6405_DACL_PRE_MIXER2_FADE_IN_OFFSET),
		0xf<<HI6405_DACL_PRE_MIXER2_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_PRE_MIXER2_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_PRE_MIXER2_FADE_OUT_LEN, HI6405_DACL_PRE_MIXER2_FADE_OUT_OFFSET),
		0xf<<HI6405_DACL_PRE_MIXER2_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACR_PRE_MIXER2_CTRL1_REG,
		0x1<<HI6405_DACR_PRE_MIXER2_FADE_EN_OFFSET,
		0x1<<HI6405_DACR_PRE_MIXER2_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_PRE_MIXER2_CTRL2_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_PRE_MIXER2_FADE_IN_LEN, HI6405_DACR_PRE_MIXER2_FADE_IN_OFFSET),
		0xf<<HI6405_DACR_PRE_MIXER2_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_PRE_MIXER2_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_PRE_MIXER2_FADE_OUT_LEN, HI6405_DACR_PRE_MIXER2_FADE_OUT_OFFSET),
		0xf<<HI6405_DACR_PRE_MIXER2_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACL_POST_MIXER2_CTRL1_REG,
		0x1<<HI6405_DACL_POST_MIXER2_FADE_EN_OFFSET,
		0x1<<HI6405_DACL_POST_MIXER2_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_POST_MIXER2_CTRL2_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_POST_MIXER2_FADE_IN_LEN, HI6405_DACL_POST_MIXER2_FADE_IN_OFFSET),
		0xf<<HI6405_DACL_POST_MIXER2_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACL_POST_MIXER2_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACL_POST_MIXER2_FADE_OUT_LEN, HI6405_DACL_POST_MIXER2_FADE_OUT_OFFSET),
		0xf<<HI6405_DACL_POST_MIXER2_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACR_POST_MIXER2_CTRL1_REG,
		0x1<<HI6405_DACR_POST_MIXER2_FADE_EN_OFFSET,
		0x1<<HI6405_DACR_POST_MIXER2_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_POST_MIXER2_CTRL2_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_POST_MIXER2_FADE_IN_LEN, HI6405_DACR_POST_MIXER2_FADE_IN_OFFSET),
		0xf<<HI6405_DACR_POST_MIXER2_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACR_POST_MIXER2_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACR_POST_MIXER2_FADE_OUT_LEN, HI6405_DACR_POST_MIXER2_FADE_OUT_OFFSET),
		0xf<<HI6405_DACR_POST_MIXER2_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_DACSL_MIXER4_CTRL1_REG,
		0x1<<HI6405_DACSL_MIXER4_FADE_EN_OFFSET,
		0x1<<HI6405_DACSL_MIXER4_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACSL_MIXER4_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_DACSL_MIXER4_FADE_IN_LEN, HI6405_DACSL_MIXER4_FADE_IN_OFFSET),
		0xf<<HI6405_DACSL_MIXER4_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_DACSL_MIXER4_CTRL4_REG,
		HI6405_MASK_ON_BIT(HI6405_DACSL_MIXER4_FADE_OUT_LEN, HI6405_DACSL_MIXER4_FADE_OUT_OFFSET),
		0xf<<HI6405_DACSL_MIXER4_FADE_OUT_OFFSET);

	hi64xx_update_bits(codec, HI6405_S2_IL_PGA_CTRL0_REG,
		0x1<<HI6405_S2_IL_PGA_FADE_EN_OFFSET,
		0x1<<HI6405_S2_IL_PGA_FADE_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_S2_IL_PGA_CTRL2_REG,
		HI6405_MASK_ON_BIT(HI6405_S2_IL_PGA_FADE_IN_LEN, HI6405_S2_IL_PGA_FADE_IN_OFFSET),
		0xc<<HI6405_S2_IL_PGA_FADE_IN_OFFSET);
	hi64xx_update_bits(codec, HI6405_S2_IL_PGA_CTRL3_REG,
		HI6405_MASK_ON_BIT(HI6405_S2_IL_PGA_FADE_OUT_LEN, HI6405_S2_IL_PGA_FADE_OUT_OFFSET),
		0xc<<HI6405_S2_IL_PGA_FADE_OUT_OFFSET);
}

static void hi6405_mic_pga_gain_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_ANA_HSMIC_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, HI6405_ANA_AUXMIC_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, HI6405_ANA_MIC3_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, HI6405_ANA_MIC4_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, HI6405_ANA_MIC5_MUX_AND_PGA, 0xf, 0x4);
	hi64xx_update_bits(codec, HI6405_ANA_MAD_PGA, 0xf, 0x4);
}

static void hi6405_adc_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_ADC0L_PGA_CTRL1_REG, DEFAULT_ADC_PGA_GAIN);
	snd_soc_write(codec, HI6405_ADC0R_PGA_CTRL1_REG, DEFAULT_ADC_PGA_GAIN);
	snd_soc_write(codec, HI6405_ADC1L_PGA_CTRL1_REG, DEFAULT_ADC_PGA_GAIN);
	snd_soc_write(codec, HI6405_ADC1R_PGA_CTRL1_REG, DEFAULT_ADC_PGA_GAIN);
	snd_soc_write(codec, HI6405_ADC2L_PGA_CTRL1_REG, DEFAULT_ADC_PGA_GAIN);
}

static void hi6405_mixer_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_DACL_MIXER4_CTRL2_REG, DEFAULT_DACLR_MIXER_PGA_GAIN);
	snd_soc_write(codec, HI6405_DACR_MIXER4_CTRL2_REG, DEFAULT_DACLR_MIXER_PGA_GAIN);
	hi64xx_update_bits(codec, HI6405_DACL_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, HI6405_DACR_PRE_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, HI6405_DACL_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	hi64xx_update_bits(codec, HI6405_DACR_POST_MIXER2_CTRL1_REG, 0x1E, 0xff);
	snd_soc_write(codec, HI6405_DACSL_MIXER4_CTRL2_REG, DEFAULT_DACLR_MIXER_PGA_GAIN);
	snd_soc_write(codec, HI6405_DACSR_MIXER4_CTRL2_REG, DEFAULT_DACLR_MIXER_PGA_GAIN);
}

static void hi6405_dac_pga_gain_init(struct snd_soc_codec *codec)
{
	snd_soc_write(codec, HI6405_DACL_PRE_PGA_CTRL1_REG, 0x6E);/* -5db */
	snd_soc_write(codec, HI6405_DACR_PRE_PGA_CTRL1_REG, 0x6E);
	snd_soc_write(codec, HI6405_DACL_POST_PGA_CTRL1_REG, 0x6E);
	snd_soc_write(codec, HI6405_DACR_POST_PGA_CTRL1_REG, 0x6E);
}

static void hi6405_adc_init(struct snd_soc_codec *codec)
{
	/* adc source select */
	snd_soc_write(codec, HI6405_SC_ADC_ANA_SEL_REG, 0x3f);
}

static void hi6405_hsd_cfg_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_078,
		0x3<<HI6405_HSD_VL_SEL_BIT, 0x2<<HI6405_HSD_VL_SEL_BIT);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_078,
		0xf<<HI6405_HSD_VH_SEL_BIT, 0x8<<HI6405_HSD_VH_SEL_BIT);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_079,
		0x1<<HI6405_HSD_VTH_SEL_BIT, 0x1<<HI6405_HSD_VTH_SEL_BIT);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_079,
		HI6405_MASK_ON_BIT(HI6405_HSD_VTH_MICL_CFG_LEN, HI6405_HSD_VTH_MICL_CFG_OFFSET),
		HI6405_HSD_VTH_MICL_CFG_800MV<<HI6405_HSD_VTH_MICL_CFG_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_079,
		HI6405_MASK_ON_BIT(HI6405_HSD_VTH_MICH_CFG_LEN, HI6405_HSD_VTH_MICH_CFG_OFFSET),
		HI6405_HSD_VTH_MICH_CFG_95<<HI6405_HSD_VTH_MICH_CFG_OFFSET);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_080,
		HI6405_MASK_ON_BIT(HI6405_MBHD_VTH_ECO_CFG_LEN, HI6405_MBHD_VTH_ECO_CFG_OFFSET),
		HI6405_MBHD_VTH_ECO_CFG_125MV<<HI6405_MBHD_VTH_ECO_CFG_OFFSET);
}

static void hi6405_classH_init(struct snd_soc_codec *codec, struct hi6405_platform_data *platform_data)
{
	/* broadconfig just controle rcv classH state */
	if (platform_data->board_config.classh_rcv_hp_switch)
		platform_data->rcv_hp_classh_state |= rcv_classh_state;
	else
		platform_data->rcv_hp_classh_state &= ~rcv_classh_state;/*lint !e64*/
	/* headphone default:classH */
	platform_data->rcv_hp_classh_state |= hp_classh_state;
	hi6405_set_classh_config(codec, platform_data->rcv_hp_classh_state);
}

static void hi6405_mux_init(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_CODEC_MUX_CTRL37_REG,
		HI6405_MASK_ON_BIT(HI6405_SPA_OL_SRC_DIN_SEL_LEN, HI6405_SPA_OL_SRC_DIN_SEL_OFFSET) |
		HI6405_MASK_ON_BIT(HI6405_SPA_OR_SRC_DIN_SEL_LEN, HI6405_SPA_OR_SRC_DIN_SEL_OFFSET),
		1<<HI6405_SPA_OL_SRC_DIN_SEL_OFFSET |
		1<<HI6405_SPA_OR_SRC_DIN_SEL_OFFSET);
}

static void hi6405_key_gpio_init(struct snd_soc_codec *codec)
{
#ifdef CONFIG_VIRTUAL_BTN_SUPPORT
	hi64xx_update_bits(codec, HI6405_APB_CLK_CFG_REG, 0x1<<HI6405_GPIO_PCLK_EN_OFFSET, 0x1<<HI6405_GPIO_PCLK_EN_OFFSET);

	/*GPIO19---KEY_INT*/
	hi64xx_update_bits(codec, HI6405_IOS_MF_CTRL19_REG, 0x1F<<HI6405_IOS_MF_CTRL19_OFFSET, 0x8<<HI6405_IOS_MF_CTRL19_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL23_REG, 0x3<<HI6405_PU_OFFSET, 0x0<<HI6405_PU_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL23_REG, 0x1<<HI6405_ST_OFFSET, 0x1<<HI6405_ST_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2DIR_REG, 0x1<<GPIO2_19_OFFSET, 0x0<<GPIO2_19_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2IS_REG, 0x1<<GPIO2_19_OFFSET, 0x0<<GPIO2_19_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2IBE_REG, 0x1<<GPIO2_19_OFFSET, 0x0<<GPIO2_19_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2IEV_REG, 0x1<<GPIO2_19_OFFSET, 0x1<<GPIO2_19_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2IE_REG, 0x1<<GPIO2_19_OFFSET, 0x0<<GPIO2_19_OFFSET);

	/* GPIO18---PWM_SMT */
	hi64xx_update_bits(codec, HI6405_IOS_MF_CTRL18_REG, 0x1F<<HI6405_IOS_MF_CTRL18_OFFSET, 0x8<<HI6405_IOS_MF_CTRL18_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL22_REG, 0x3<<HI6405_PU_OFFSET, 0x0<<HI6405_PU_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL22_REG, 0x7<<HI6405_DS_OFFSET, 0x1<<HI6405_DS_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO2DIR_REG, 0x1<<GPIO2_18_OFFSET, 0x1<<GPIO2_18_OFFSET);

	/* GPIO15---AP_AI*/
	hi64xx_update_bits(codec, HI6405_IOS_MF_CTRL15_REG, 0x1F<<HI6405_IOS_MF_CTRL15_OFFSET, 0x8<<HI6405_IOS_MF_CTRL15_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL19_REG, 0x3<<HI6405_PU_OFFSET, 0x0<<HI6405_PU_OFFSET);
	hi64xx_update_bits(codec, HI6405_IOS_IOM_CTRL19_REG, 0x7<<HI6405_DS_OFFSET, 0x1<<HI6405_DS_OFFSET);
	hi64xx_update_bits(codec, CODEC_BASE_ADDR | GPIO1DIR_REG, 0x1<<GPIO2_15_OFFSET, 0x1<<GPIO2_15_OFFSET);
#endif
	hi64xx_update_bits(codec, HI6405_ANA_MICBIAS1, 0x1<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET, 0x0<<HI6405_ANA_MICBIAS1_DSCHG_OFFSET);
	hi64xx_update_bits(codec, HI6405_ANA_MICBIAS2, 0x1<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET, 0x0<<HI6405_ANA_MICBIAS2_DSCHG_OFFSET);
	hi64xx_update_bits(codec, HI6405_ANA_HSMICBIAS, 0x1<<HI6405_ANA_HSMIC_DSCHG_OFFSET, 0x0<<HI6405_ANA_HSMIC_DSCHG_OFFSET);
}

static void hi6405_chip_init(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	WARN_ON(NULL == platform_data);

	hi6405_efuse_init(codec);
	hi6405_ioshare_init(codec, platform_data);
	hi6405_enable_regulator(codec);
	hi6405_buck_init(codec);
	hi6405_pll48k_config(codec);
	hi6405_pllmad_config(codec);
	hi6405_pll44k1_config(codec);
	hi6405_clk_cfg_init(codec);

	hi6405_slimbus_init(codec, platform_data);
	hi6405_set_mad_param(codec, &platform_data->board_config);
	hi6405_set_dsp_if_bypass(codec);
	hi6405_adc_init(codec);
	hi6405_pga_fade_init(codec);
	hi6405_mic_pga_gain_init(codec);
	hi6405_adc_pga_gain_init(codec);
	hi6405_mixer_pga_gain_init(codec);
	hi6405_dac_pga_gain_init(codec);
	hi6405_hsd_cfg_init(codec);
	hi6405_classH_init(codec, platform_data);
	hi6405_key_gpio_init(codec);
	hi6405_mux_init(codec);

	OUT_FUNCTION
}

static unsigned int hi6405_get_voltage_value(struct snd_soc_codec *codec, unsigned int voltage_coefficient)
{
	int retry = 3;
	unsigned int sar_value = 0;
	unsigned int voltage_value = 0;

	/* saradc on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012, 0x1 << HI6405_MBHD_SAR_PD_BIT, 0);
	/* start saradc */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012,
			0x1<<HI6405_SARADC_START_BIT, 0x1<<HI6405_SARADC_START_BIT);

	while(retry--) {
		usleep_range(1000, 1100);
		if (hi64xx_check_saradc_ready_detect(codec)) {
			sar_value = snd_soc_read(codec, HI6405_CODEC_ANA_ROREG_000);
			sar_value = snd_soc_read(codec, HI6405_CODEC_ANA_ROREG_001) + (sar_value<<8);
			HI6405_LOGI("saradc value is %#x\n", sar_value);

			break;
		}
	}
	if (0 > retry)
		HI6405_LOGE("get saradc err \n");

	/* end saradc */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012,
			0x1<<HI6405_SARADC_START_BIT, 0x0<<HI6405_SARADC_START_BIT);
	/* saradc pd */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012,
			0x1 << HI6405_MBHD_SAR_PD_BIT, 0x1 << HI6405_MBHD_SAR_PD_BIT);

	voltage_value = sar_value * voltage_coefficient / 0xFFF;
	return voltage_value;
}

static void hi6405_hs_mbhc_on(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);
	if (!platform_data) {
		HI6405_LOGE("get hi6405 platform data failed\n");
		return;
	}

	hi64xx_irq_mask_btn_irqs(platform_data->mbhc);

	/* sar clk use clk32_sys */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_077,
	        0x3<<HI6405_CLK_SAR_SEL_BIT, 0x1<<HI6405_CLK_SAR_SEL_BIT);
	/* saradc tsmp set 4 * Tclk */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_077,
	        0x3<<HI6405_SAR_TSMP_CFG_BIT, 0x3<<HI6405_SAR_TSMP_CFG_BIT);
	/* sar on */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_078, 0x0<<HI6405_RST_SAR_BIT, 0);
	/* cmp fast mode en */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_077, 0x3<<HI6405_SAR_INPUT_SEL_BIT, 0);
	msleep(30);

	hi64xx_irq_unmask_btn_irqs(platform_data->mbhc);

	msleep(120);
}

static void hi6405_hs_mbhc_off(struct snd_soc_codec *codec)
{
	/* eco off */
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_080, 0x1<<HI6405_MBHD_ECO_EN_BIT, 0);
	HI6405_LOGI("eco disable\n");
}

static void hi6405_hs_enable_hsdet(struct snd_soc_codec *codec, struct hi64xx_mbhc_config mbhc_config)
{
	unsigned int voltage_coefficent;

	if (mbhc_config.coefficient < MBHC_VOLTAGE_COEFFICIENT_MIN
		||mbhc_config.coefficient > MBHC_VOLTAGE_COEFFICIENT_MAX) {
		/*default set coefficent 2700mv*/
		voltage_coefficent = (MBHC_VOLTAGE_COEFFICIENT_DEFAULT - MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	} else {
		voltage_coefficent = (mbhc_config.coefficient - MBHC_VOLTAGE_COEFFICIENT_MIN) / 100;
	}

	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_073, 0xF<<HI6405_HSMICB_ADJ,
			voltage_coefficent<<HI6405_HSMICB_ADJ);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_012, 0x1<<HI6405_MBHD_PD_MBHD_VTN, 0);
	hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_080, 0x1<<HI6405_MBHD_HSD_EN_BIT,
			0x1<<HI6405_MBHD_HSD_EN_BIT);
}

static struct hs_mbhc_reg hi6405_hs_mbhc_reg = {
	.irq_source_reg = CODEC_BASE_ADDR + HI6405_CODEC_ANA_IRQ_SRC_STAT_REG,
	.irq_mbhc_2_reg = HI6405_IRQ_REG2_REG,
};

static struct hs_mbhc_func hi6405_hs_mbhc_func = {
	.hs_mbhc_on =  hi6405_hs_mbhc_on,
	.hs_get_voltage = hi6405_get_voltage_value,
	.hs_enable_hsdet = hi6405_hs_enable_hsdet,
	.hs_mbhc_off =  hi6405_hs_mbhc_off,
};

static struct hs_res_detect_func hi6405_hs_res_detect_func_null = {
	.hs_res_detect = NULL,
	.hs_path_enable = NULL,
	.hs_path_disable = NULL,
};

static struct hi64xx_hs_cfg hi6405_hs_cfg = {
	.mbhc_reg = &hi6405_hs_mbhc_reg,
	.mbhc_func = &hi6405_hs_mbhc_func,
	.res_detect_func = &hi6405_hs_res_detect_func_null,
};

static int hi6405_codec_probe(struct snd_soc_codec *codec)
{
	int ret = 0;
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	IN_FUNCTION
	if (!platform_data) {
		HI6405_LOGE("get hi6405 platform data failed\n");
		return -ENOENT;
	}

	snd_soc_codec_set_drvdata(codec, platform_data);
	platform_data->codec = codec;
	hi6405_codec = codec;

	ret = hi6405_resmgr_init(platform_data);
	if (0 != ret) {
		HI6405_LOGE("hi6405 resmgr init failed:0x%x\n", ret);
		return -ENOMEM;
	}

	HI6405_LOGI("hi6405 version:0x%x\n", snd_soc_read(codec, HI6405_VERSION_REG));

	hi6405_chip_init(codec);

	ret = hi64xx_mbhc_init(codec, platform_data->node, &hi6405_hs_cfg,
			platform_data->resmgr, platform_data->irqmgr, &platform_data->mbhc);
	if (0 != ret) {
		HI6405_LOGE("hifi config fail. err code is %x\n", ret);
		goto mbhc_init_err;
	}

	ret = hi6405_hifi_config_init(codec, platform_data->resmgr,
				platform_data->irqmgr, platform_data->cdc_ctrl->bus_sel);
	if (ret) {
		HI6405_LOGE("hi6405 dsp init failed:0x%x\n", ret);
		goto misc_init_err_exit;
	}

	ret = hi6405_utils_init(platform_data);
	if (ret) {
		HI6405_LOGE("hi6405 utils init failed:0x%x\n", ret);
		goto utils_init_failed;
	}

	ret = hi64xx_vad_init(codec, platform_data->irqmgr);
	if (ret) {
		HI6405_LOGE("hi6405 vad init failed:0x%x\n", ret);
		goto vad_init_failed;
	}

	if (platform_data->cdc_ctrl->pm_runtime_support) {
		ret = hi6405_slim_enumerate(platform_data->resmgr);
		if (ret) {
			HI6405_LOGE("hi6405 slim enum failed:0x%x\n", ret);
			goto slim_enumerate_failed;
		}
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	ret = hicodec_debug_init(codec, &hi6405_dump_info);
	if (ret) {
		dev_info(codec->dev, "hicodec_debug_init error, errornum = %d", ret);
		ret = 0;
	}
#endif

	HI6405_LOGI("hi6405 codec probe ok\n");
	return ret;

slim_enumerate_failed:
	hi64xx_vad_deinit();
vad_init_failed:
	hi64xx_utils_deinit();
utils_init_failed:
	hi6405_hifi_config_deinit();
misc_init_err_exit:
	hi64xx_mbhc_deinit(platform_data->mbhc);
mbhc_init_err:
	hi64xx_resmgr_deinit(platform_data->resmgr);

	HI6405_LOGE("hi6405 codec probe failed\n");
	return ret;
}

static int hi6405_codec_remove(struct snd_soc_codec *codec)
{
	struct hi6405_platform_data *platform_data = snd_soc_codec_get_drvdata(codec);

	if (!platform_data) {
		HI6405_LOGE("get hi6405 platform data failed\n");
		return -ENOENT;
	}

#ifdef CONFIG_SND_SOC_HICODEC_DEBUG
	hicodec_debug_uninit(codec);
#endif

	hi64xx_vad_deinit();
	hi64xx_utils_deinit();
	hi6405_hifi_config_deinit();
	hi64xx_mbhc_deinit(platform_data->mbhc);
	hi64xx_resmgr_deinit(platform_data->resmgr);

	return 0;
}

static int hi6405_audio_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	int rate;
	int ret = 0;

	WARN_ON(NULL == codec);

	rate = params_rate(params);
	switch (rate) {
	case 8000:
	case 11025:
	case 16000:
	case 22050:
	case 32000:
	case 44100:
	case 48000:
	case 88200:
	case 96000:
	case 176400:
	case 192000:
		break;
	case 384000:
		pr_err("rate : %d\n", rate);
		break;
	default:
		pr_err("unknown rate : %d!\n", rate);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int hi6405_audio_hw_free(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6405_audio_dai_ops = {
	.hw_params = hi6405_audio_hw_params,
	.hw_free = hi6405_audio_hw_free,
};

static int hi6405_voice_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params,
				struct snd_soc_dai *dai)
{
	struct snd_soc_codec *codec = dai->codec;
	int ret = 0;
	int rate;

	WARN_ON(NULL == codec);

	rate = params_rate(params);
	switch (rate) {
	case 8000:
	case 16000:
	case 32000:
		break;
	default:
		pr_err("unknown rate : %d!\n", rate);
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int hi6405_voice_hw_free(struct snd_pcm_substream *substream,
				struct snd_soc_dai *dai)
{
	return 0;
}

struct snd_soc_dai_ops hi6405_voice_dai_ops = {
	.hw_params = hi6405_voice_hw_params,
	.hw_free = hi6405_voice_hw_free,
};

struct snd_soc_dai_driver hi6405_dai[] = {
	{
		.name = "hi6405-audio-dai",
		.playback = {
			.stream_name = "Playback",
			.channels_min = 2,
			.channels_max = 2,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS},
		.capture = {
			.stream_name = "Capture",
			.channels_min = 1,
			.channels_max = 6,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS},
		.ops = &hi6405_audio_dai_ops,
	},
	{
		.name = "hi6405-voice-dai",
		.playback = {
			.stream_name = "Down",
			.channels_min = 1,
			.channels_max = 2,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS},
		.capture = {
			.stream_name = "Up",
			.channels_min = 1,
			.channels_max = 6,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS},
		.ops = &hi6405_voice_dai_ops,
	},
	{
		.name = "hi6405-fm-dai",
		.playback = {
			.stream_name = "FM",
			.channels_min = 1,
			.channels_max = 2,
			.rates = HI6405_RATES,
			.formats = HI6405_FORMATS},
	},
};

static struct snd_soc_codec_driver hi6405_codec_driver = {
	.probe = hi6405_codec_probe,
	.remove = hi6405_codec_remove,
	.read = hi6405_reg_read,
	.write = hi6405_reg_write,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(4,9,0)
	.component_driver = {
		.controls = hi6405_snd_controls,
		.num_controls = ARRAY_SIZE(hi6405_snd_controls),
		.dapm_widgets = hi6405_dapm_widgets,
		.num_dapm_widgets = ARRAY_SIZE(hi6405_dapm_widgets),
		.dapm_routes = hi6405_route_map,
		.num_dapm_routes = ARRAY_SIZE(hi6405_route_map),
	},
#else
	.controls = hi6405_snd_controls,
	.num_controls = ARRAY_SIZE(hi6405_snd_controls),
	.dapm_widgets = hi6405_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(hi6405_dapm_widgets),
	.dapm_routes = hi6405_route_map,
	.num_dapm_routes = ARRAY_SIZE(hi6405_route_map),
#endif
};

static bool hi6405_check_card_valid(struct hi6405_platform_data *platform_data)
{
	unsigned int val = 0;

	val = hi_cdcctrl_reg_read(platform_data->cdc_ctrl, HI64xx_VERSION_REG);
	if (HI6405_VERSION_VALUE != val) {
		HI6405_LOGE("read hi6405 version failed:0x%x\n", val);
		return false;
	}

	return true;
}

static void hi6405_get_board_micnum(struct device_node *node, struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	if (!of_property_read_u32(node, "hisilicon,mic_num", &val)) {
		board_cfg->mic_num = (int)val;
	} else {
		board_cfg->mic_num = 2;
	}
}

static void hi6405_get_board_hpswitch(struct device_node *node, struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	if (!of_property_read_u32(node, "hisilicon,classh_rcv_hp_switch", &val)){
		if(val){
			board_cfg->classh_rcv_hp_switch = true;
		} else {
			board_cfg->classh_rcv_hp_switch = false;
		}
	} else {
		board_cfg->classh_rcv_hp_switch = false;
	}
}

#ifdef CONFIG_HAC_SUPPORT
static void hi6405_get_board_hac(struct device_node *node, struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;
	int ret = 0;

	if (!of_property_read_u32(node, "hisilicon,hac_gpio", &val)) {
		board_cfg->hac_gpio = val;
		ret = hi6405_hac_gpio_init(board_cfg->hac_gpio);
		if (0 != ret) {
			HI6405_LOGE("gpio resource init fail, ret = %d\n", ret);
		}
	} else {
		board_cfg->hac_gpio = -1;
	}
}
#endif

static void hi6405_get_board_wakeup_hisi_algo_support(struct device_node *node, struct hi6405_board_cfg *board_cfg)
{
	unsigned int val = 0;

	if (NULL == node || NULL == board_cfg) {
		pr_err("%s: input null pointer! \n", __FUNCTION__);
		return;
	}

	board_cfg->wakeup_hisi_algo_support = false;
	if (!of_property_read_u32(node, "hisilicon,wakeup_hisi_algo_support", &val)) {
		if (val) {
			board_cfg->wakeup_hisi_algo_support = true;
		}
	}
}

static void hi6405_get_board_cfg(struct device_node *node, struct hi6405_board_cfg *board_cfg)
{
	hi6405_get_board_micnum(node, board_cfg);
	hi6405_get_board_hpswitch(node, board_cfg);
#ifdef CONFIG_HAC_SUPPORT
	hi6405_get_board_hac(node, board_cfg);
	HI6405_LOGI("hac_gpio %d \n", board_cfg->hac_gpio);
#endif
	HI6405_LOGI("mic_num %d \n", board_cfg->mic_num);
	hi6405_get_board_wakeup_hisi_algo_support(node, board_cfg);
	HI6405_LOGI("wakeup_hisi_algo_support %d \n", board_cfg->wakeup_hisi_algo_support);
}

static int hi6405_init_platform_data(struct platform_device *pdev,
				struct hi6405_platform_data *platform_data)
{
	const struct of_device_id *match;

	platform_data->irqmgr = (struct hi64xx_irq *)dev_get_drvdata(pdev->dev.parent);
	platform_data->cdc_ctrl = (struct hi_cdc_ctrl *)dev_get_drvdata(pdev->dev.parent->parent);

	if (!hi6405_check_card_valid(platform_data)) {
		return -ENODEV;
	}

	match = of_match_device(hi6405_platform_match, &pdev->dev);
	if (!match) {
		HI6405_LOGE("get device info err\n");
		return -ENOENT;
	}

	platform_data->node = pdev->dev.of_node;
	hi6405_get_board_cfg(platform_data->node, &platform_data->board_config);
	platform_data->voice_up_params.channels = 2;
	if (2 < platform_data->board_config.mic_num)
		platform_data->voice_up_params.channels = 4;
	platform_data->voice_down_params.channels = 2;
	platform_data->capture_params.channels = 2;
	platform_data->capture_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->soundtrigger_params.rate = SLIMBUS_SAMPLE_RATE_16K;
	platform_data->soundtrigger_params.channels = 1;
	platform_data->voiceup_state = TRACK_FREE;
	platform_data->audioup_4mic_state = TRACK_FREE;
	platform_data->play_params.channels = 2;
	platform_data->play_params.rate = SLIMBUS_SAMPLE_RATE_48K ;
	platform_data->pa_iv_params.channels = 4;
	platform_data->pa_iv_params.rate = SLIMBUS_SAMPLE_RATE_48K ;

	spin_lock_init(&platform_data->v_rw_lock);
	mutex_init(&platform_data->impdet_dapm_mutex);

	platform_set_drvdata(pdev, platform_data);
	dev_set_name(&pdev->dev, "hi6405-codec");

	return 0;
}

static inline void hi6405_deinit_platform_data(struct hi6405_platform_data *platform_data)
{
	mutex_destroy(&platform_data->impdet_dapm_mutex);
}

static int hi6405_irq_init(struct hi64xx_irq *irq_data)
{
	struct hi64xx_irq_map irqmap;
	int ret = 0;

	irqmap.irq_regs[0] = HI6405_IRQ_REG0_REG;
	irqmap.irq_regs[1] = HI6405_IRQ_REG1_REG;
	irqmap.irq_regs[2] = HI6405_IRQ_REG2_REG;
	irqmap.irq_regs[3] = HI6405_IRQ_REG3_REG;
	irqmap.irq_regs[4] = HI6405_IRQ_REG4_REG;
	irqmap.irq_regs[5] = HI6405_IRQ_REG5_REG;
	irqmap.irq_regs[6] = HI6405_IRQ_REG6_REG;
	irqmap.irq_mask_regs[0] = HI6405_IRQM_REG0_REG;
	irqmap.irq_mask_regs[1] = HI6405_IRQM_REG1_REG;
	irqmap.irq_mask_regs[2] = HI6405_IRQM_REG2_REG;
	irqmap.irq_mask_regs[3] = HI6405_IRQM_REG3_REG;
	irqmap.irq_mask_regs[4] = HI6405_IRQM_REG4_REG;
	irqmap.irq_mask_regs[5] = HI6405_IRQM_REG5_REG;
	irqmap.irq_mask_regs[6] = HI6405_IRQM_REG6_REG;
	irqmap.irq_num = HI6405_IRQ_NUM;

	ret = hi64xx_irq_init_irq(irq_data, &irqmap);

	return ret;
}

static inline void hi6405_dsm_reqport(int dsm_type, char *str_error)
{
#ifdef CONFIG_HUAWEI_DSM
	audio_dsm_report_info(AUDIO_CODEC, dsm_type, str_error);
#endif
}

static void hi6405_dsp_power_down(struct snd_soc_codec *codec)
{
	hi64xx_update_bits(codec, HI6405_SC_DSP_CTRL0_REG,
			0x1<<HI6405_SC_DSP_SFT_RUNSTALL_OFFSET | 0x1<<HI6405_SC_DSP_EN_OFFSET |
			0x1<<HI6405_SC_HIFI_CLK_EN_OFFSET | 0x1<<HI6405_SC_HIFI_ACLK_EN_OFFSET,
			0x1<<HI6405_SC_DSP_SFT_RUNSTALL_OFFSET | 0x0<<HI6405_SC_DSP_EN_OFFSET |
			0x0<<HI6405_SC_HIFI_CLK_EN_OFFSET | 0x0<<HI6405_SC_HIFI_ACLK_EN_OFFSET);
	hi64xx_update_bits(codec, HI6405_APB_CLK_CFG_REG,
			0x1<<HI6405_APB_PD_PCLK_EN_OFFSET, 0x0<<HI6405_APB_PD_PCLK_EN_OFFSET);
	snd_soc_write(codec, HI6405_DSP_IF_CLK_EN, 0x0);
	hi64xx_update_bits(codec, HI6405_SW_RST_REQ_REG,
			0x1<<HI6405_DSP_PD_SRST_REQ_OFFSET, 0x1<<HI6405_DSP_PD_SRST_REQ_OFFSET);
	hi64xx_update_bits(codec, HI6405_DSP_LP_CTRL0_REG,
			0x1<<HI6405_DSP_TOP_ISO_CTRL_OFFSET | 0x1<<HI6405_DSP_TOP_MTCMOS_CTRL_OFFSET,
			0x1<<HI6405_DSP_TOP_ISO_CTRL_OFFSET | 0x1<<HI6405_DSP_TOP_MTCMOS_CTRL_OFFSET);
}

#ifdef CONFIG_HUAWEI_DSM
#define HI6405_IRQ_HANDLER(irqname, irq_reg, irq_offset, dsmtype) \
	({struct hi6405_platform_data *platform_data;\
	struct snd_soc_codec *codec;\
	platform_data = (struct hi6405_platform_data *)(data);\
	WARN_ON(NULL == platform_data);\
	codec = platform_data->codec;\
	if (codec) {\
		HI6405_LOGW("hi6405: "irqname" irq received!!!\n");\
		snd_soc_write(codec, irq_reg, 0x1<<irq_offset);\
		hi6405_dsm_reqport(dsmtype, "64xx codec "irqname"\n");\
	}})
#else
#define HI6405_IRQ_HANDLER(irqname, irq_reg, irq_offset, dsmtype)
#endif
static irqreturn_t hi6405_bunk1_ocp_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("bunk1_ocp", HI6405_IRQ_REG5_REG, HI6405_PMU_BUNK1_OCP_IRQ_OFFSET, DSM_CODEC_BUNK1_OCP);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_bunk1_scp_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("bunk1_scp", HI6405_IRQ_REG5_REG, HI6405_PMU_BUNK1_SCP_IRQ_OFFSET, DSM_CODEC_BUNK1_SCP);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_ldo_avdd18_ocp_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("ldo_avdd18_ocp", HI6405_IRQ_REG5_REG, HI6405_PMU_LDO_AVDD18_OCP_IRQ_OFFSET, DSM_CODEC_LDO_AVDD18_OCP);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_ldop_ocp_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("ldop_ocp", HI6405_IRQ_REG5_REG, HI6405_PMU_LDOP_OCP_IRQ_OFFSET, DSM_CODEC_LDOP_OCP);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_ldon_ocp_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("ldon_ocp", HI6405_IRQ_REG5_REG, HI6405_PMU_LDON_OCP_IRQ_OFFSET, DSM_CODEC_LDON_OCP);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_cp1_short_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("cp1_short", HI6405_IRQ_REG5_REG, HI6405_PMU_CP1_SHORT_IRQ_OFFSET, DSM_CODEC_CP1_SHORT);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_cp2_short_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("cp2_short", HI6405_IRQ_REG5_REG, HI6405_PMU_CP2_SHORT_IRQ_OFFSET, DSM_CODEC_CP2_SHORT);
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_pll_unlock_handler(int irq, void *data)
{
	struct hi6405_platform_data *platform_data;
	struct snd_soc_codec *codec;
	unsigned int i = 0, j = 0;
	unsigned char output_str[PLL_DATA_ALL_NUM] = {0};
	unsigned char buf[PLL_DATA_BUF_SIZE] = {0};

	platform_data = (struct hi6405_platform_data *)(data);
	WARN_ON(NULL == platform_data);
	codec = platform_data->codec;
	if (codec) {
		HI6405_LOGW("hi6405_pll_unlock irq received\n");
		snd_soc_write(codec, HI6405_IRQ_REG2_REG, 0x1<<HI6405_PLL_48K_UNLOCK_F_IRQ_OFFSET);
#ifdef CONFIG_HUAWEI_DSM
		hi6405_dsm_reqport(DSM_HI6402_PLL_UNLOCK, "64xx codec pll_unlock\n");
#endif
		hi64xx_update_bits(codec, HI6405_PLL_TEST_CTRL1_REG,
			0x1<<HI6405_PLL_FIFO_CLK_EN_OFFSET, 0x1<<HI6405_PLL_FIFO_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_200,
				0x1<<HI6405_CODEC_ANA_TEST_SOFT_RST_N_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_SOFT_RST_N_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_PLL_SEL_OFFSET      | 0x1<<HI6405_CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
				0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET | 0x7<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET,
				0x0<<HI6405_CODEC_ANA_TEST_PLL_SEL_OFFSET      | 0x1<<HI6405_CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
				0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET | 0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_194, 0x7);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_195, 0x7);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_196,
				0x1<<HI6405_CODEC_ANA_PVT_SEL_OFFSET | 0x7<<HI6405_CODEC_ANA_TEST_LOOP_I_OFFSET |
				0xf<<HI6405_CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET,
				0x1<<HI6405_CODEC_ANA_PVT_SEL_OFFSET | 0x1<<HI6405_CODEC_ANA_TEST_LOOP_I_OFFSET |
				0xf<<HI6405_CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_197, 0xff);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_198, 0xf);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_199, 0xa0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_200,
				0x3<<HI6405_CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET, 0x2<<HI6405_CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_201, 0x0);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_202, 0xff);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_167,
				0x7f<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET, 0x40<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_168,
				0x3f<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET, 0x20<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_190,
				0x7f<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET, 0x40<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_191,
				0x3f<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET, 0x20<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET, 0x0<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET);
		msleep(100);
		hi6405_dsp_power_down(codec);
		for(i = 0; i < PLL_DATA_ALL_NUM; i = i + PLL_DATA_GROUP_NUM){
			memset(output_str, 0, sizeof(output_str));
			memset(buf, 0, sizeof(buf));
			for(j = 0; j < PLL_DATA_GROUP_NUM; ++j){
				snprintf(buf, sizeof(buf), "%9x", snd_soc_read(codec, HI6405_PLL_FIFO));
				strncat(output_str, buf, strlen(buf));
			}
			HI6405_LOGI("hi6405_pll_main1 PLL data:%s\n", output_str);
		}
		hi64xx_watchdog_send_event();
	}
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_pll44k1_unlock_handler(int irq, void *data)
{
	struct hi6405_platform_data *platform_data;
	struct snd_soc_codec *codec;
	unsigned int i = 0, j = 0;
	unsigned char output_str[PLL_DATA_ALL_NUM] = {0};
	unsigned char buf[PLL_DATA_BUF_SIZE] = {0};

	platform_data = (struct hi6405_platform_data *)(data);
	WARN_ON(NULL == platform_data);
	codec = platform_data->codec;
	if (codec) {
		HI6405_LOGW("hi6405_pll44k1_unlock irq received\n");
		snd_soc_write(codec, HI6405_IRQ_REG4_REG, 0x1<<HI6405_PLL_44P1K_UNLOCK_F_IRQ_OFFSET);
#ifdef CONFIG_HUAWEI_DSM
		hi6405_dsm_reqport(DSM_HI6402_PLL_UNLOCK, "64xx codec pll44k1_unlock\n");
#endif
		hi64xx_update_bits(codec, HI6405_PLL_TEST_CTRL1_REG,
			0x1<<HI6405_PLL_FIFO_CLK_EN_OFFSET, 0x1<<HI6405_PLL_FIFO_CLK_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_REF_CLK_CG_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_200,
				0x1<<HI6405_CODEC_ANA_TEST_SOFT_RST_N_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_SOFT_RST_N_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_PLL_SEL_OFFSET      | 0x1<<HI6405_CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
				0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET | 0x7<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET,
				0x1<<HI6405_CODEC_ANA_TEST_PLL_SEL_OFFSET      | 0x1<<HI6405_CODEC_ANA_TEST_DCO_OPEN_EN_OFFSET |
				0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_EN_OFFSET | 0x1<<HI6405_CODEC_ANA_TEST_PLL_CLOSE_CAPTUTE_MODE_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_194, 0x7);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_195, 0x7);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_196,
				0x1<<HI6405_CODEC_ANA_PVT_SEL_OFFSET | 0x7<<HI6405_CODEC_ANA_TEST_LOOP_I_OFFSET |
				0xf<<HI6405_CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET,
				0x1<<HI6405_CODEC_ANA_PVT_SEL_OFFSET | 0x1<<HI6405_CODEC_ANA_TEST_LOOP_I_OFFSET |
				0xf<<HI6405_CODEC_ANA_TEST_FREQ_CALC_CNT_TIME_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_197, 0xff);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_198, 0xf);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_199, 0xa0);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_200,
				0x3<<HI6405_CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET, 0x2<<HI6405_CODEC_ANA_TEST_SCAN_CNT_TIME_OFFSET);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_201, 0x0);
		snd_soc_write(codec, HI6405_CODEC_ANA_RWREG_202, 0xff);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_167,
				0x7f<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET, 0x40<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_FINE_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_168,
				0x3f<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET, 0x20<<HI6405_CODEC_ANA_MAIN1_TEST_TUNE_PVT_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_190,
				0x7f<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET, 0x40<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_FINE_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_191,
				0x3f<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET, 0x20<<HI6405_CODEC_ANA_MAIN2_TEST_TUNE_PVT_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET, 0x0<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET);
		hi64xx_update_bits(codec, HI6405_CODEC_ANA_RWREG_193,
				0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET, 0x1<<HI6405_CODEC_ANA_TEST_MODE_EN_OFFSET);
		msleep(100);
		hi6405_dsp_power_down(codec);
		for(i = 0; i < PLL_DATA_ALL_NUM; i = i + PLL_DATA_GROUP_NUM){
			memset(output_str, 0, sizeof(output_str));
			memset(buf, 0, sizeof(buf));
			for(j = 0; j < PLL_DATA_GROUP_NUM; ++j){
				snprintf(buf, sizeof(buf), "%9x", snd_soc_read(codec, HI6405_PLL_FIFO));
				strncat(output_str, buf, strlen(buf));
			}
			HI6405_LOGI("hi6405_pll_main2 PLL data:%s\n", output_str);
		}
		hi64xx_watchdog_send_event();
	}
	return IRQ_HANDLED;
}

static irqreturn_t hi6405_pllmad_unlock_handler(int irq, void *data)
{
	HI6405_IRQ_HANDLER("pllmad_unlock", HI6405_IRQ_REG4_REG, HI6405_PLL_MAD_UNLOCK_F_IRQ_OFFSET, DSM_HI6402_PLL_UNLOCK);
	return IRQ_HANDLED;
}

static void hi6405_free_irq(struct hi6405_platform_data *platform_data)
{
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_OCP, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_SCP, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_CP1_SHORT, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_CP2_SHORT, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDO_AVDD18_OCP, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDOP_OCP, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDON_OCP, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK, platform_data);
	hi64xx_irq_free_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK, platform_data);
}

static int hi6405_request_irq(struct hi6405_platform_data *platform_data)
{
	int ret;

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_OCP, hi6405_bunk1_ocp_handler, "bunk1_ocp", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_bunk1_ocp_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_SCP, hi6405_bunk1_scp_handler, "bunk1_scp", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_bunk1_scp_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_CP1_SHORT, hi6405_cp1_short_handler, "cp1_short", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_cp1_short_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_CP2_SHORT, hi6405_cp2_short_handler, "cp2_short", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_cp2_short_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_LDO_AVDD18_OCP, hi6405_ldo_avdd18_ocp_handler, "ldo_avdd18_ocp", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_ldo_avdd18_ocp_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_LDOP_OCP, hi6405_ldop_ocp_handler, "ldop_ocp", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_ldop_ocp_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, HI6405_IRQ_LDON_OCP, hi6405_ldon_ocp_handler, "ldon_ocp", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_ldon_ocp_exit;
	}

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK, hi6405_pll_unlock_handler, "pll_unlock", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_pll_unlock_exit;
	}
	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK);

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK, hi6405_pll44k1_unlock_handler, "pll44k1_unlock", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_pll44k1_unlock_exit;
	}
	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK);

	ret = hi64xx_irq_request_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK, hi6405_pllmad_unlock_handler, "pllmad_unlock", platform_data);
	if (0 != ret) {
		HI6405_IRQ_REQUEST_ERR_PRINT
		goto irq_pllmad_unlock_exit;
	}
	hi64xx_irq_disable_irq(platform_data->irqmgr, IRQ_PLLMAD_UNLOCK);

	return ret;

irq_pllmad_unlock_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, IRQ_PLL44K1_UNLOCK, platform_data);
irq_pll44k1_unlock_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, IRQ_PLL_UNLOCK, platform_data);
irq_pll_unlock_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDON_OCP, platform_data);
irq_ldon_ocp_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDOP_OCP, platform_data);
irq_ldop_ocp_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_LDO_AVDD18_OCP, platform_data);
irq_ldo_avdd18_ocp_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_CP2_SHORT, platform_data);
irq_cp2_short_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_CP1_SHORT, platform_data);
irq_cp1_short_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_SCP, platform_data);
irq_bunk1_scp_exit:
	hi64xx_irq_free_irq(platform_data->irqmgr, HI6405_IRQ_BUNK1_OCP, platform_data);
irq_bunk1_ocp_exit:

	HI6405_LOGE("request irq failed!!!\n");
	return ret;
}

#ifdef CONFIG_HISI_DIEID
int hi6405_codec_get_dieid(char *dieid, unsigned int len)
{
	unsigned int dieid_value;
	unsigned int reg_value;
	unsigned int length;
	char dieid_buf[CODEC_DIEID_BUF] = {0};
	char buf[CODEC_DIEID_TEM_BUF] = {0};
	int ret = 0;

	if (NULL == dieid) {
		HI6405_LOGE("dieid is NULL\n");
		return -EINVAL;
	}

	if (NULL == hi6405_codec) {
		HI6405_LOGW("codec is NULL\n");
		return -EINVAL;
	}

	ret = snprintf(dieid_buf, sizeof(dieid_buf), "%s%s%s", "\r\n", "CODEC", ":0x");/* [false alarm]:snprintf is safe */
	if (ret < 0) {
		HI6405_LOGE("snprintf main codec dieid head fail.\n");
		return ret;
	}

	/* enable efuse */
	hi64xx_update_bits(hi6405_codec, HI6405_DIE_ID_CFG1_REG,
			0x1<<HI6405_EFUSE_MODE_SEL_OFFSET, 0x1<<HI6405_EFUSE_MODE_SEL_OFFSET);
	hi64xx_update_bits(hi6405_codec, HI6405_CFG_CLK_CTRL_REG,
			0x1<<HI6405_EFUSE_CLK_EN_OFFSET, 0x1<<HI6405_EFUSE_CLK_EN_OFFSET);
	hi64xx_update_bits(hi6405_codec, HI6405_DIE_ID_CFG1_REG,
			0x1<<HI6405_EFUSE_READ_EN_OFFSET, 0x1<<HI6405_EFUSE_READ_EN_OFFSET);
	msleep(5);

	for (reg_value = HI6405_DIE_ID_OUT_DATA0_REG;reg_value <= HI6405_DIE_ID_OUT_DATA15_REG;reg_value++) {
		dieid_value = snd_soc_read(hi6405_codec, reg_value);
		ret = snprintf(buf, sizeof(buf), "%02x", dieid_value);/* [false alarm]:snprintf is safe */
		if( ret < 0){
			HI6405_LOGE("snprintf codec dieid fail.\n");
			return ret;
		}
		strncat(dieid_buf, buf, strlen(buf));
	}
	strncat(dieid_buf, "\r\n", strlen("\r\n"));

	length = strlen(dieid_buf);
	if (len > length) {
		strncpy(dieid, dieid_buf, length);
		dieid[length] = '\0';
	} else {
		HI6405_LOGE("dieid buf length = %d is not enough!\n", length);
		return -ENOMEM;
	}

	return 0;
}
#endif

static int hi6405_platform_probe(struct platform_device *pdev)
{
	int ret = 0;
	struct device *dev = &pdev->dev;
	struct hi6405_platform_data *platform_data = NULL;

	platform_data = devm_kzalloc(dev, sizeof(*platform_data), GFP_KERNEL);
	if (NULL == platform_data) {
		HI6405_LOGE("malloc hi6405 platform data failed\n");
		return -ENOMEM;
	}

	ret = hi6405_init_platform_data(pdev, platform_data);
	if (ret) {
		HI6405_LOGE("hi6405_init_platform_data failed:0x%x.\n", ret);
		goto free_platform_data;
	}

	ret = hi6405_irq_init(platform_data->irqmgr);
	if (ret) {
		HI6405_LOGE("hi6405 irq init failed:0x%x.\n", ret);
		goto irq_init_err_exit;
	}

	ret = hi6405_request_irq(platform_data);
	if (ret) {
		HI6405_LOGE("hi6405_request_irq failed:0x%x.\n", ret);
		goto irq_request_err_exit;
	}

	ret = hi64xx_compat_init(platform_data->cdc_ctrl, platform_data->irqmgr);
	if (ret) {
		HI6405_LOGE("hi64xx compat init failed:0x%x\n", ret);
		goto compat_init_err_exit;
	}

	ret = snd_soc_register_codec(dev, &hi6405_codec_driver,
			hi6405_dai, ARRAY_SIZE(hi6405_dai));
	if (ret) {
		HI6405_LOGE("registe codec driver failed:0x%x.\n", ret);
		goto codec_register_err_exit;
	}

	return ret;

codec_register_err_exit:
	hi64xx_compat_deinit();
compat_init_err_exit:
	hi6405_free_irq(platform_data);
irq_request_err_exit:
	hi64xx_irq_deinit_irq(platform_data->irqmgr);
irq_init_err_exit:
	hi6405_deinit_platform_data(platform_data);
free_platform_data:
	if (platform_data) {
		devm_kfree(dev, platform_data);
	}
	dev_err(dev, "%s: init failed\n", __FUNCTION__);
	return ret;
}

static int hi6405_platform_remove(struct platform_device *pdev)
{
	struct hi6405_platform_data *platform_data = platform_get_drvdata(pdev);

	WARN_ON(NULL == platform_data);

#ifdef CONFIG_HAC_SUPPORT
	if (gpio_is_valid(platform_data->board_config.hac_gpio)) {
		gpio_free(platform_data->board_config.hac_gpio);
	}
#endif

	snd_soc_unregister_codec(&pdev->dev);

	hi64xx_compat_deinit();

	hi6405_free_irq(platform_data);

	hi64xx_irq_deinit_irq(platform_data->irqmgr);

	hi6405_deinit_platform_data(platform_data);

	return 0;
}

static void hi6405_platform_shutdown(struct platform_device *pdev)
{
	struct hi6405_platform_data *platform_data;
	struct snd_soc_codec *codec;

	platform_data = platform_get_drvdata(pdev);
	WARN_ON(NULL == platform_data);
	codec = platform_data->codec;

	if (NULL != codec) {
		hi6405_headphone_pop_off(codec);
	}
}

static struct platform_driver hi6405_platform_driver = {
	.probe = hi6405_platform_probe,
	.remove = hi6405_platform_remove,
	.shutdown = hi6405_platform_shutdown,
	.driver = {
		.owner = THIS_MODULE,
		.name = "hi6405-codec",
		.of_match_table = of_match_ptr(hi6405_platform_match),
	},
};

static int __init hi6405_platform_init(void)
{
	return platform_driver_register(&hi6405_platform_driver);
}
module_init(hi6405_platform_init);

static void __exit hi6405_platform_exit(void)
{
	platform_driver_unregister(&hi6405_platform_driver);
}
module_exit(hi6405_platform_exit);

MODULE_DESCRIPTION("ASoC hi6405 codec driver");
MODULE_LICENSE("GPL");
