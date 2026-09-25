/*
 * hi6405_path_widget.h -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef _HI6405_PATH_WIDGET_H_
#define _HI6405_PATH_WIDGET_H_

#include <sound/soc.h>

#include "linux/hisi/hi64xx/hi6405_type.h"

int hi6405_add_path_widgets(struct snd_soc_codec *codec);
void hi6405_set_classH_config(struct snd_soc_codec *codec,
	enum classH_state classH_state_cfg);
void hi6405_headphone_pop_on(struct snd_soc_codec *codec);
void hi6405_headphone_pop_off(struct snd_soc_codec *codec);
void headphone_pop_on_delay_work(struct work_struct *work);

#endif

