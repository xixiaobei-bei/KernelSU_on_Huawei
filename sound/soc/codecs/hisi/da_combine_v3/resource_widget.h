/*
 * resource_widget.h
 *
 * pll resource widget
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

#ifndef __DA_COMBINE_V3_RESOURCE_WIDGET_H__
#define __DA_COMBINE_V3_RESOURCE_WIDGET_H__

#include "type.h"

int add_resource_widgets(struct snd_soc_component *codec);
int resmgr_init(struct da_combine_v3_platform_data *platform_data);
void supply_pll_init(struct snd_soc_component *codec);
void da_combine_v3_request_cp_dp_clk(struct snd_soc_component *codec);
void da_combine_v3_release_cp_dp_clk(struct snd_soc_component *codec);

#endif

