/*
 * hi6405_dsp_config.h
 *
 * dsp init
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

#ifndef __HI6405_DSP_CONFIG_H__
#define __HI6405_DSP_CONFIG_H__

#include <linux/hisi/hi64xx/hi64xx_irq.h>
#include <linux/hisi/hi64xx/hi64xx_resmgr.h>
#include <linux/hisi/hi64xx/hi64xx_mbhc.h>

int hi6405_dsp_config_init(struct snd_soc_codec *codec,
	struct hi64xx_resmgr *resmgr, struct hi64xx_irq *irqmgr,
	enum bustype_select bus_sel);
void hi6405_dsp_config_deinit(void);

#endif

