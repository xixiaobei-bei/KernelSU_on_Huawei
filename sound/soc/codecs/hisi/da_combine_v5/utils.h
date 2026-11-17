/*
 * utils.h -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __DA_COMBINE_V5_UTILS_H__
#define __DA_COMBINE_V5_UTILS_H__

#include <sound/soc.h>
#include <linux/version.h>

unsigned int da_combine_v5_reg_read(struct snd_soc_component *codec,
	unsigned int reg);
int da_combine_v5_reg_write(struct snd_soc_component *codec,
	unsigned int reg, unsigned int value);
#if (LINUX_VERSION_CODE < KERNEL_VERSION(4,19,0))
unsigned int da_combine_v5_reg_read_by_codec(struct snd_soc_codec *codec,
	unsigned int reg);
int da_combine_v5_reg_write_by_codec(struct snd_soc_codec *codec,
	unsigned int reg, unsigned int value);
#endif

#endif

