/*
 * da_separate_audio_path.h codec driver.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#ifndef __DA_SEPARATE_AUDIO_PATH_H__
#define __DA_SEPARATE_AUDIO_PATH_H__

#include <sound/soc.h>
#include "da_separate_reg.h"

int add_audio(struct snd_soc_component *codec);
#endif

