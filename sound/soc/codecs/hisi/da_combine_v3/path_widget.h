/*
 * path_widget.h
 *
 * path widget
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

#ifndef __DA_COMBINE_V3_PATH_WIDGET_H__
#define __DA_COMBINE_V3_PATH_WIDGET_H__

#include <sound/soc.h>

#include "type.h"

int add_path_widgets(struct snd_soc_component *codec);
void headphone_pop_on(struct snd_soc_component *codec);
void headphone_pop_off(struct snd_soc_component *codec);
void set_classh_config(struct snd_soc_component *codec,
	enum classh_state classH_state_cfg);

#endif

