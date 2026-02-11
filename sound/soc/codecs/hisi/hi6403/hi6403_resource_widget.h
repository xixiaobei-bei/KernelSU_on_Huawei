/*
 * hi6403_resource_widget.h
 *
 * pll resource widget
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

#ifndef _HI6403_RESOURCE_WIDGET_H_
#define _HI6403_RESOURCE_WIDGET_H_

#include "hi6403_type.h"

int hi6403_add_resource_widgets(struct snd_soc_codec *codec);
int hi6403_resmgr_init(struct hi6403_platform_data *platform_data);
void hi6403_supply_pll_init(struct snd_soc_codec *codec);
void hi6403_request_cp_dp_clk(struct snd_soc_codec *codec);
void hi6403_release_cp_dp_clk(struct snd_soc_codec *codec);

#endif

