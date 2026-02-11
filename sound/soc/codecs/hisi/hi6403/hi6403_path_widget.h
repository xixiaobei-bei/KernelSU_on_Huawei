/*
 * hi6403_path_widget.h
 *
 * path widget
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

#ifndef _HI6403_PATH_WIDGET_H_
#define _HI6403_PATH_WIDGET_H_

#include <sound/soc.h>

#include "hi6403_type.h"

int hi6403_add_path_widgets(struct snd_soc_codec *codec);
void hi6403_headphone_pop_on(struct snd_soc_codec *codec);
void hi6403_headphone_pop_off(struct snd_soc_codec *codec);
void hi6403_set_classh_config(struct snd_soc_codec *codec,
	enum classh_state classH_state_cfg);

#endif

