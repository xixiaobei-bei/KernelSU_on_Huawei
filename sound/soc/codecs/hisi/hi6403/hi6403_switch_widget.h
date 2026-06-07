/*
 * hi6403_switch_widget.h
 *
 * hi6403 switch widget for codec driver
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HI6403_SWITCH_WIDGET_H_
#define _HI6403_SWITCH_WIDGET_H_

#include <sound/soc.h>

int hi6403_add_switch_widgets(struct snd_soc_codec *codec);
void hi6403_headphone_high_mode_shadow_config(struct snd_soc_codec *codec);
void hi6403_headphone_low_mode_shadow_config(struct snd_soc_codec *codec);

#endif

