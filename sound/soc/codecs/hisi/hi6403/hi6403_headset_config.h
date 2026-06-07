/*
 * hi6403_headset_config.h
 *
 * headset init config
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

#ifndef __HI6403_HIGH_RES_CFG_H__
#define __HI6403_HIGH_RES_CFG_H__

#include "hi6403_type.h"

int hi6403_hs_cfg_init(struct snd_soc_codec *codec, struct hi6403_platform_data *platform_data);
void hi6403_hs_cfg_deinit(struct hi64xx_mbhc *mbhc);
void hi6403_hp_classh_init(struct snd_soc_codec *codec, struct hi6403_platform_data *platform_data);

#endif /* __HI6403_HIGH_RES_CFG_H__ */

