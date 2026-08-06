/*
 * da_combine_vad.h
 *
 * da_combine_vad codec driver
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

#ifndef __DA_COMBINE_VAD_H__
#define __DA_COMBINE_VAD_H__

#include <sound/soc.h>
#include <linux/hisi/da_combine/da_combine_irq.h>

int da_combine_fast_mode_set(bool enable);
int da_combine_vad_init(struct snd_soc_component *codec, struct da_combine_irq *irq);
int da_combine_vad_deinit(struct device_node *node);

#endif /* __DA_COMBINE_VAD_H__ */

