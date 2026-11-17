/*
 * headset_config.h
 *
 * headset init config
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

#ifndef __DA_COMBINE_V3_HEADSET_CONFIG_H__
#define __DA_COMBINE_V3_HEADSET_CONFIG_H__

#include "type.h"

int hs_cfg_init(struct snd_soc_component *codec, struct da_combine_v3_platform_data *platform_data);
void hs_cfg_deinit(struct da_combine_mbhc *mbhc);
void hp_classh_init(struct snd_soc_component *codec, struct da_combine_v3_platform_data *platform_data);

#endif /* __DA_COMBINE_V3_HEADSET_CONFIG_H_ */

