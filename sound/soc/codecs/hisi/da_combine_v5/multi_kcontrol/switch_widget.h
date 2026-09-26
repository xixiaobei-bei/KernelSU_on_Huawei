/*
 * switch_widget.h -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __DA_COMBINE_V5_SWITCH_WIDGET_H__
#define __DA_COMBINE_V5_SWITCH_WIDGET_H__

#include <sound/soc.h>

int da_combine_v5_add_switch_widgets(struct snd_soc_component *codec, bool single_kcontrol);
int audioup_2mic_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event);
int audioup_4mic_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event);
int s2up_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event);
int iv_2pa_switch_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event);
int ec_switch_power_event(struct snd_soc_dapm_widget *w, struct snd_kcontrol *kcontrol, int event);

#endif

