/*
 * hwcxext_codec_info.h
 *
 * hwcxext_codec_info header file
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

#ifndef __HWCXEXT_CODEC_INFO__
#define __HWCXEXT_CODEC_INFO__

#include "hwcxext_i2c_ops.h"

struct hwcxext_codec_info_ctl_ops {
	int (*dump_regs)(struct hwcxext_codec_regmap_cfg *cfg);
};

#ifdef HWCXEXT_CODEC_INFO_PERMISSION_ENABLE
void hwcxext_codec_register_info_ctl_ops(struct hwcxext_codec_info_ctl_ops *ops);
void hwcxext_codec_info_store_regmap(struct hwcxext_codec_regmap_cfg *regmap);
void hwcxext_codec_info_set_ctl_support(bool status);
#else
static inline void hwcxext_codec_register_info_ctl_ops(
	struct hwevext_codec_info_ctl_ops *ops)
{
}

static inline void hwcxext_codec_info_store_regmap(
	struct hwcxext_codec_regmap_cfg *cfg)
{
}

static inline void hwcxext_codec_info_set_ctl_support(bool status)
{
}
#endif
#endif // HWCXEXT_CODEC_INFO