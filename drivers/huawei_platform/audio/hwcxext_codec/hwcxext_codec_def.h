/*
 * hwcxext_codec_def.h
 *
 * hwcxext_codec header file
 *
 * Copyright (c) 2022 Huawei Technologies Co., Ltd.
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

#ifndef __HWCXEXT_CODEC_DEF__
#define __HWCXEXT_CODEC_DEF__

#include <linux/clk.h>
#include <linux/i2c.h>
#include <linux/regmap.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "hwcxext_i2c_ops.h"

struct hwcxext_codec_v1_priv {
	struct clk *mclk;
	struct hwcxext_codec_regmap_cfg *regmap_cfg;
	struct snd_soc_component *component;
	struct hwcxext_mbhc_priv *mbhc_data;
	struct i2c_client *i2c;
	/* init regs */
	struct hwcxext_reg_ctl_sequence *init_regs_seq;
	int power_boost_en_gpio;
	unsigned int max_times_hs_recognize;
};

#endif // HWCXEXT_CODEC_DEF