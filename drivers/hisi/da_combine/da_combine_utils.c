/*
 * da_combine_utils.c
 *
 * da_combine_utils codec driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/hisi/da_combine/da_combine_utils.h>

#include <linux/version.h>
#include <linux/module.h>
#include <linux/mutex.h>
#include <linux/proc_fs.h>
#include <linux/vmalloc.h>
#include <linux/hisi/da_combine/da_combine_resmgr.h>
#include <linux/hisi/audio_log.h>

#include "asoc_adapter.h"
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
#include <linux/hisi/da_combine/da_combine_v5.h>
#else
#define LOG_TAG "DA_combine_utils"
#endif


static struct utils_config *g_utils_config;
static unsigned int g_cdc_type = DA_COMBINE_CODEC_TYPE_BUTT;
static struct snd_soc_component *g_snd_codec;

int da_combine_update_bits(struct snd_soc_component *codec, unsigned int reg,
	unsigned int mask, unsigned int value)
{
	int change;
	unsigned int old;
	unsigned int new;

	if (codec == NULL) {
		AUDIO_LOGE("parameter is null");
		return -EINVAL;
	}
	old = snd_soc_component_read32(codec, reg);
	new = (old & ~mask) | (value & mask);
	change = (old != new);
	if (change != 0)
		snd_soc_component_write(codec, reg, new);

	return change;
}


int da_combine_utils_init(struct snd_soc_component *codec, struct hi_cdc_ctrl *cdc_ctrl,
	const struct utils_config *config, struct da_combine_resmgr *resmgr,
	unsigned int codec_type)
{
	g_utils_config = kzalloc(sizeof(*g_utils_config), GFP_KERNEL);
	if (g_utils_config == NULL) {
		AUDIO_LOGE("Failed to kzalloc utils config");
		g_snd_codec = NULL;
		return -EFAULT;
	}
	memcpy(g_utils_config, config, sizeof(*g_utils_config));

	g_cdc_type = codec_type;
	g_snd_codec = codec;

	return 0;
}

void da_combine_utils_deinit(void)
{
	if (g_utils_config != NULL) {
		kfree(g_utils_config);
		g_utils_config = NULL;
	}

	g_snd_codec = NULL;
}

#ifdef CONFIG_HISI_DIEID
int codec_get_dieid(char *dieid, unsigned int len)
{
#ifdef CONFIG_SND_SOC_DA_COMBINE_V5
	if (g_cdc_type == DA_COMBINE_CODEC_TYPE_V5)
		return da_combine_v5_codec_get_dieid(dieid, len);
#endif
	return -1;
}
#endif

unsigned int da_combine_utils_reg_read(unsigned int reg)
{
	if (g_snd_codec == NULL) {
		AUDIO_LOGE("parameter is NULL");
		return 0;
	}

	return snd_soc_component_read32(g_snd_codec, reg);
}

