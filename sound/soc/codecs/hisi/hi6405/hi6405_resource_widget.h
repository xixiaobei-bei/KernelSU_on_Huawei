/*
 * hi6405_resource_widget.h -- hi6405 codec driver
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef _HI6405_RESOURCE_WIDGET_H_
#define _HI6405_RESOURCE_WIDGET_H_

#include "linux/hisi/hi64xx/hi6405_type.h"

int hi6405_add_resource_widgets(struct snd_soc_codec *codec);
int hi6405_resmgr_init(struct hi6405_platform_data *platform_data);
void hi6405_supply_pll_init(struct snd_soc_codec *codec);
#ifdef CONFIG_HIGH_RESISTANCE_HS_DET
void request_cp_dp_clk(struct snd_soc_codec *codec);
void release_cp_dp_clk(struct snd_soc_codec *codec);
#endif
#endif

