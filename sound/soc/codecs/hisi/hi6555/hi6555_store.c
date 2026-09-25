/*
 * hi6555_store.c codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "hi6555_store.h"
#include "hi6555_type.h"
#include "audio_log.h"
#include "hi6555_core_path.h"
#include "hi6555_audio_path.h"
#include "hi6555_ultr_path.h"
#include "hi6555_voice_path.h"
#include "hi6555_smpa_path.h"
#include "hi6555_third_codec_path.h"
#include "hi6555_btfm_path.h"

#define LOG_TAG "hi6555_store"

struct snd_path {
	char *name;
	int (*add_snd_resource)(struct snd_soc_codec *codec);
};

__attribute__((weak)) int add_audio(struct snd_soc_codec *codec);
__attribute__((weak)) int add_ultr(struct snd_soc_codec *codec);
__attribute__((weak)) int add_voice(struct snd_soc_codec *codec);
__attribute__((weak)) int add_smpa(struct snd_soc_codec *codec);
__attribute__((weak)) int add_third_codec(struct snd_soc_codec *codec);
__attribute__((weak)) int add_btfm(struct snd_soc_codec *codec);
__attribute__((weak)) int add_mad(struct snd_soc_codec *codec);


int add_audio(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_ultr(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_voice(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_smpa(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_third_codec(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_btfm(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

int add_mad(struct snd_soc_codec *codec)
{
	AUDIO_LOGE("this is a stub realization for snd codec, please check");

	return 0;
}

static struct snd_path snd_path_tab[] = {
	{ "audio", add_audio },
	{ "ultr", add_ultr },
	{ "voice", add_voice },
	{ "smpa", add_smpa },
	{ "third_codec", add_third_codec },
	{ "btfm", add_btfm },
	{ "mad", add_mad },
};

static int add_path_by_name(struct snd_soc_codec *codec, const char *path_name)
{
	unsigned int size = ARRAY_SIZE(snd_path_tab);
	unsigned int i;
	int ret = -EINVAL;

	for (i = 0; i < size; i++) {
		if (strcmp(path_name, snd_path_tab[i].name) == 0) {
			ret = snd_path_tab[i].add_snd_resource(codec);
			if (ret != 0) {
				AUDIO_LOGE("path %s resource add failed, ret: %d", path_name, ret);
				goto exit;
			}
			AUDIO_LOGI("add %s path for codec driver", path_name);
			break;

		}

	}

	if (i == size)
		AUDIO_LOGE("can not find path resource for %s", path_name);
exit:
	return ret;
}

int hi6555_store_subscribe_path(struct snd_soc_codec *codec, const char **name_list, unsigned int num)
{
	unsigned int size = ARRAY_SIZE(snd_path_tab);
	unsigned int i;
	int ret;

	if (codec == NULL) {
		AUDIO_LOGE("codec parameter is null");
		return -EINVAL;
	}

	if (name_list == NULL) {
		AUDIO_LOGE("name list is null");
		return -EINVAL;
	}

	if (num > size) {
		AUDIO_LOGE("invalid name size, name size: %d, snd path table size: %d", num, size);
		return -EINVAL;
	}

	ret = add_core(codec);
	if (ret != 0) {
		AUDIO_LOGE("add codec core path failed: %d", ret);
		return ret;
	}

	for (i = 0; i < num; i++) {
		ret = add_path_by_name(codec, name_list[i]);
		if (ret != 0)
			break;
	}

	return ret;
}

