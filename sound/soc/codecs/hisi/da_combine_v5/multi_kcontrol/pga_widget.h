/*
 * pga_widget.h -- da combine v5 codec driver
 *
 * Copyright (c) 2018 Huawei Technologies Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __DA_COMBINE_V5_PGA_WIDGET_H__
#define __DA_COMBINE_V5_PGA_WIDGET_H__

#include "linux/hisi/da_combine/da_combine_v5_type.h"

int da_combine_v5_add_pga_widgets(struct snd_soc_component *codec, bool single_kcontrol);

#endif
