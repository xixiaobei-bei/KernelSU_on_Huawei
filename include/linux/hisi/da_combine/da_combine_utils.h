/*
 * da_combine_utils.h
 *
 * da_combine_utils codec driver
 *
 * Copyright (c) 2014-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __DA_COMBINE_UTILS_H__
#define __DA_COMBINE_UTILS_H__

#include <sound/soc.h>
#include <linux/hisi/da_combine/hi_cdc_ctrl.h>

struct utils_config {
	/* functions to dump codec registers */
	void (*da_combine_dump_reg)(char *, unsigned int);
};

struct da_combine_resmgr;

int da_combine_update_bits(struct snd_soc_component *codec, unsigned int reg,
	unsigned int mask, unsigned int value);
int da_combine_utils_init(struct snd_soc_component *codec, struct hi_cdc_ctrl *cdc_ctrl,
	const struct utils_config *config, struct da_combine_resmgr *resmgr,
	unsigned int codec_type);

void da_combine_dump_debug_info(void);

void da_combine_utils_deinit(void);

#ifdef CONFIG_HISI_DIEID
int codec_get_dieid(char *dieid, unsigned int len);
#endif

unsigned int da_combine_utils_reg_read(unsigned int reg);

#endif /* __DA_COMBINE_UTILS_H__ */

