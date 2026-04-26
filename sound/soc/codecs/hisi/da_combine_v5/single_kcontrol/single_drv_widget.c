/*
 * single_drv_widget.c -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include "single_drv_widget.h"

#include <sound/core.h>

#include "linux/hisi/audio_log.h"
#include "linux/hisi/da_combine/da_combine_v5.h"
#include "linux/hisi/da_combine/da_combine_v5_regs.h"
#include "linux/hisi/da_combine/da_combine_utils.h"
#include "linux/hisi/da_combine/da_combine_v5_type.h"
#include "switch_widget.h"
#include "switch_widget_utils.h"
#include "slimbus_da_combine_v5.h"

static void audio_play_drv_enable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, VIRTUAL_DOWN_REG,
		BIT(PLAY48K_BIT), BIT(PLAY48K_BIT));
	play_config_power_event(SAMPLE_RATE_INDEX_48K, codec, SND_SOC_DAPM_PRE_PMU);
	AUDIO_LOGI("AUDIO PLAY DRV Open!");

	AUDIO_LOGD("end");
}

void audio_play_drv_disable(struct snd_soc_component *codec)
{
	AUDIO_LOGD("begin");

	da_combine_update_bits(codec, VIRTUAL_DOWN_REG, BIT(PLAY48K_BIT), 0x0);
	play_config_power_event(SAMPLE_RATE_INDEX_48K, codec, SND_SOC_DAPM_POST_PMD);
	AUDIO_LOGI("AUDIO PLAY DRV Close!");

	AUDIO_LOGD("end");
}

static int audio_play_drv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		audio_play_drv_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		audio_play_drv_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");
	return 0;
}

static int audio_capture_drv_power_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *kcontrol, int event)
{
	AUDIO_LOGI("power event: %d", event);
	return audioup_4mic_power_event(w, kcontrol, event);
}

static int ec_drv_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		/* "U7_EC_SWITC" */
		ec_switch_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		/* "U7_EC_SWITC" */
		ec_switch_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static void bt_capture_drv_enable(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGI("begin");

	platform_data->bt_uplink_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->bt_uplink_params.channels = 2;

	up_src_pre_pmu(codec, platform_data->bt_uplink_params.channels,
		SAMPLE_RATE_REG_CFG_48K, SRC_MODE_2);
	u56_select_pga(codec);
	ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP,
		&platform_data->bt_uplink_params);
	if (ret != 0)
		AUDIO_LOGE("slimbus activate track bt up err: %d", ret);

	AUDIO_LOGI("end");
}

static void bt_capture_drv_disable(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGI("begin");

	u56_select_pga(codec);
	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_BT_UP, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus track deactivate bt up err: %d", ret);
	up_src_post_pmu(codec,
		platform_data->bt_uplink_params.channels);

	AUDIO_LOGI("end");
}

static int bt_capture_drv_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_capture_drv_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_capture_drv_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

static void bt_play_drv_enable(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGI("begin");

	platform_data->bt_downlink_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->bt_downlink_params.channels = 2;

	ret = slimbus_activate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN, &platform_data->bt_downlink_params);
	if (ret != 0)
		AUDIO_LOGE("slimbus track activate bt down err: %d", ret);

	AUDIO_LOGI("end");
}

static void bt_play_drv_disable(struct snd_soc_component *codec)
{
	int ret;
	struct da_combine_v5_platform_data *platform_data = snd_soc_component_get_drvdata(codec);

	AUDIO_LOGI("begin");

	platform_data->bt_downlink_params.rate = SLIMBUS_SAMPLE_RATE_48K;
	platform_data->bt_downlink_params.channels = 2;

	ret = slimbus_deactivate_track(SLIMBUS_DEVICE_DA_COMBINE_V5,
		SLIMBUS_DA_COMBINE_V5_TRACK_BT_DOWN, &platform_data->bt_downlink_params);
	if (ret != 0)
		AUDIO_LOGE("slimbus track deactivate bt down err: %d", ret);

	AUDIO_LOGI("end");
}

static int bt_play_drv_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event)
{
	struct snd_soc_component *codec = snd_soc_dapm_to_component(w->dapm);

	AUDIO_LOGD("begin");

	switch (event) {
	case SND_SOC_DAPM_PRE_PMU:
		bt_play_drv_enable(codec);
		break;
	case SND_SOC_DAPM_POST_PMD:
		bt_play_drv_disable(codec);
		break;
	default:
		AUDIO_LOGW("power event err: %d", event);
		break;
	}

	AUDIO_LOGD("end");

	return 0;
}

#define DRV_WIDGET \
	SND_SOC_DAPM_OUT_DRV_E("AUDIO_PLAY_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		audio_play_drv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("AUDIO_CAPTURE_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		audio_capture_drv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("EC_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		ec_drv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("BT_CAPTURE_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		bt_capture_drv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \
	SND_SOC_DAPM_OUT_DRV_E("BT_PLAY_DRV", \
		SND_SOC_NOPM, 0, 0, NULL, 0, \
		bt_play_drv_power_event, (SND_SOC_DAPM_PRE_PMU | SND_SOC_DAPM_POST_PMD)), \

static const struct snd_soc_dapm_widget drv_widgets[] = {
	DRV_WIDGET
};

int da_combine_v5_add_single_drv_widgets(struct snd_soc_component *codec)
{
	struct snd_soc_dapm_context *dapm = NULL;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}

	dapm = snd_soc_component_get_dapm(codec);
	return snd_soc_dapm_new_controls(dapm, drv_widgets,
		ARRAY_SIZE(drv_widgets));
}

