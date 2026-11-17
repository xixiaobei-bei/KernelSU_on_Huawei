/*
 * codec_pm.h
 *
 * expose route pm api for da combine v5 driver
 *
 * Copyright (c) 2020 Huawei Technologies Co., Ltd.
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

#ifndef __DA_COMBINE_V5_CODEC_PM_H__
#define __DA_COMBINE_V5_CODEC_PM_H__

#include <sound/soc.h>
#include <linux/device.h>

void da_combine_v5_codec_pm_init(struct snd_soc_component *codec);
int da_combine_v5_codec_suspend(struct device *dev);
int da_combine_v5_codec_resume(struct device *dev);
int da_combine_v5_codec_freeze(struct device *dev);
int da_combine_v5_codec_restore(struct device *dev);
int da_combine_v5_codec_thaw(struct device *dev);

#endif
