/*
 * soundtrigger_dma_drv_comm.c
 *
 * soundtrigger_dma_drv_comm is a kernel driver common operation which is used to manager dma
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "soundtrigger_dma_drv_comm.h"

#include <linux/hisi/audio_log.h>

#include "slimbus.h"
#include "slimbus_da_combine_v5.h"
#include "dsp_misc.h"
#include "dsp_utils.h"
#include "asp_dma.h"

#define LOG_TAG "soundtrigger_comm"

void stop_dma(int32_t pcm_channel, const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t i;
	uint32_t dma_port_num;
	uint32_t dma_channel;
	const struct soundtrigger_pcm_info *pcm_info = NULL;

	if (dma_drv_info == NULL)
		return;

	pcm_info = &(dma_drv_info->pcm_info[pcm_channel]);

	dma_port_num = ARRAY_SIZE(pcm_info->channel);
	for (i = 0; i < dma_port_num; i++) {
		dma_channel = pcm_info->channel[i];
		asp_dma_stop((unsigned short)dma_channel);
	}
}

void dump_dma_addr_info(const struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t pcm_num;
	uint32_t dma_port_num;
	uint32_t buff_num;
	uint32_t i, j, k;
	const struct soundtrigger_pcm_info *pcm_info = NULL;

	if (dma_drv_info == NULL)
		return;

	AUDIO_LOGI("dma config soundtrigger addr: %pK", (void *)CODEC_DSP_SOUNDTRIGGER_BASE_ADDR);

	pcm_num = ARRAY_SIZE(dma_drv_info->pcm_info);
	for (i = 0; i < pcm_num; i++) {
		pcm_info = &(dma_drv_info->pcm_info[i]);
		dma_port_num = ARRAY_SIZE(pcm_info->channel);
		buff_num = ARRAY_SIZE(pcm_info->dma_cfg[0]);
		for (j = 0; j < dma_port_num; j++) {
			for (k = 0; k < buff_num; k++) {
				AUDIO_LOGI("dma soundtrigger info: dma num: %u, buffer num: %u", j, k);

				AUDIO_LOGI("buffer physical addr: %pK, buffer: %pK, lli dma physical addr: %pK, dma cfg: %pK",
					pcm_info->buffer_phy_addr[j][k],
					pcm_info->buffer[j][k],
					pcm_info->lli_dma_phy_addr[j][k],
					pcm_info->dma_cfg[j][k]);

				AUDIO_LOGI("a count: %u, src addr: 0x%pK, dest addr: 0x%pK, config: %u",
					pcm_info->dma_cfg[j][k]->a_count,
					(const void *)(uintptr_t)(pcm_info->dma_cfg[j][k]->src_addr),
					(const void *)(uintptr_t)(pcm_info->dma_cfg[j][k]->des_addr),
					pcm_info->dma_cfg[j][k]->config);
			}
		}
	}
}

int32_t slimbus_activate(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	uint32_t track_type;
	int32_t ret;
	struct slimbus_track_param slimbus_params;
	enum slimbus_device device_type = SLIMBUS_DEVICE_NUM;

	if (dma_drv_info == NULL)
		return -EINVAL;

	memset(&slimbus_params, 0, sizeof(slimbus_params));

	slimbus_params.rate = FAST_TRAN_RATE;
	AUDIO_LOGI("fast slimbus rate: %u", slimbus_params.rate);

	if (dma_drv_info->type == CODEC_DA_COMBINE_V3) {
		slimbus_params.channels = 1;
		device_type = SLIMBUS_DEVICE_DA_COMBINE_V3;
		track_type = (uint32_t)SLIMBUS_TRACK_SOUND_TRIGGER;
	}
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (dma_drv_info->type == CODEC_DA_COMBINE_V5) {
		slimbus_params.channels = 1;
		device_type = SLIMBUS_DEVICE_DA_COMBINE_V5;
		track_type = (uint32_t)SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER;
	}
#endif
	else {
		AUDIO_LOGE("device type is err: %d", device_type);
		return -EINVAL;
	}

	(void)da_combine_request_pll_resource(HIGH_FREQ_SCENE_FASTTRANS);
	msleep(2);
	ret = slimbus_activate_track(device_type, track_type, &slimbus_params);
	AUDIO_LOGI("dma start request pll resource and switch");
	if (ret != 0) {
		AUDIO_LOGI("slimbus track activate fail");
		da_combine_release_pll_resource(HIGH_FREQ_SCENE_FASTTRANS);
		return ret;
	}

	return 0;
}

int32_t slimbus_deactivate(struct soundtrigger_dma_drv_info *dma_drv_info)
{
	int32_t ret;
	uint32_t track_type;
	enum slimbus_device device_type = SLIMBUS_DEVICE_NUM;

	if (dma_drv_info == NULL)
		return -EINVAL;

	if (dma_drv_info->type == CODEC_DA_COMBINE_V3) {
		device_type = SLIMBUS_DEVICE_DA_COMBINE_V3;
		track_type = (uint32_t)SLIMBUS_TRACK_SOUND_TRIGGER;
	}
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	else if (dma_drv_info->type == CODEC_DA_COMBINE_V5) {
		device_type = SLIMBUS_DEVICE_DA_COMBINE_V5;
		track_type = (uint32_t)SLIMBUS_DA_COMBINE_V5_TRACK_SOUND_TRIGGER;
	}
#endif
	else {
		AUDIO_LOGE("device type is err: %d", dma_drv_info->type);

		if (dma_drv_info->wake_lock.active)
			__pm_relax(&dma_drv_info->wake_lock);

		return -EINVAL;
	}

	ret = slimbus_deactivate_track(device_type, track_type, NULL);
	if (ret != 0)
		AUDIO_LOGE("slimbus track deactivate err: %d", ret);

	msleep(2);
	da_combine_release_pll_resource(HIGH_FREQ_SCENE_FASTTRANS);

	AUDIO_LOGI("release pll resource and switch to soc");

	return 0;
}
