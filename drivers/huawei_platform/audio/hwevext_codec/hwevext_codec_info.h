/*
 * hwevext_codec_info.h
 *
 * hwevext_codec_info header file
 *
 * Copyright (c) 2021~2022 Huawei Technologies Co., Ltd.
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

#ifndef __HWEVEXT_CODEC_INFO__
#define __HWEVEXT_CODEC_INFO__

#include <linux/regmap.h>

struct hwevext_codec_info_ctl_ops {
	int (*dump_regs)(struct regmap *regmap);
};

#ifdef HWEVEXT_CODEC_INFO_PERMISSION_ENABLE
void hwevext_codec_register_info_ctl_ops(struct hwevext_codec_info_ctl_ops *ops);
void hwevext_codec_info_store_regmap(struct regmap *regmap);
void hwevext_codec_info_set_ctl_support(bool status);
void hwevext_codec_info_set_adc_channel(unsigned int channel);
#else
static inline void hwevext_codec_register_info_ctl_ops(
	struct hwevext_codec_info_ctl_ops *ops)
{
}

static inline void hwevext_codec_info_store_regmap(struct regmap *regmap)
{
}

static inline void hwevext_codec_info_set_ctl_support(bool status)
{
}

static inline void hwevext_codec_info_set_adc_channel(unsigned int channel)
{
}
#endif
#endif // HWEVEXT_CODEC_INFO