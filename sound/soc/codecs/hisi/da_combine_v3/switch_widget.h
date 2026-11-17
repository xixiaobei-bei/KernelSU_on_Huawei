/*
 * switch_widget.h
 *
 * da combine v3 switch widget for codec driver
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
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

#ifndef __DA_COMBINE_V3_SWITCH_WIDGET_H__
#define __DA_COMBINE_V3_SWITCH_WIDGET_H__

#include <sound/soc.h>

int add_switch_widgets(struct snd_soc_component *codec);
void headphone_high_mode_shadow_config(struct snd_soc_component *codec);
void headphone_low_mode_shadow_config(struct snd_soc_component *codec);

#endif

