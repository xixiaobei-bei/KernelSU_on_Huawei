/*
 * da_combine_v3_dsp_config.h
 *
 * dsp init
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

#ifndef __DA_COMBINE_V3_DSP_CONFIG_H__
#define __DA_COMBINE_V3_DSP_CONFIG_H__

#include <linux/hisi/da_combine/da_combine_irq.h>
#include <linux/hisi/da_combine/da_combine_resmgr.h>
#include <linux/hisi/da_combine/da_combine_mbhc.h>

int dsp_config_init(struct snd_soc_component *codec,
	struct da_combine_resmgr *resmgr, struct da_combine_irq *irqmgr,
	enum bustype_select bus_sel);
void hifi_config_deinit(void);

#endif
