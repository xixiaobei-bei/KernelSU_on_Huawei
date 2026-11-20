/*
 * hwcxext_i2c_ops.h
 *
 * hwcxext_i2c_ops header file
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

#ifndef __HWCXEXT_I2C_OPS__
#define __HWCXEXT_I2C_OPS__

#include "hwcxext_codec_def.h"

#define HWCXEXT_I2C_READ      0
#define HWCXEXT_I2C_WRITE     1

#ifdef CONFIG_HUAWEI_DSM_AUDIO
#ifndef DSM_BUF_SIZE
#define DSM_BUF_SIZE 1024
#endif
#endif

struct hwcxext_reg_ctl {
	/* one reg address or reg address_begin of read registers */
	unsigned int addr;
	unsigned int mask;
	union {
		unsigned int value;
		unsigned int chip_version;
	};
	/* delay us */
	unsigned int delay;
	/* ctl type, default 0(read) */
	unsigned int ctl_type;
};

struct hwcxext_reg_ctl_sequence {
	unsigned int num;
	struct hwcxext_reg_ctl *regs;
};


struct hwcxext_regs_size_ctl {
	unsigned int addr;
	unsigned int size;
};

struct hwcxext_regs_size_sequence {
	unsigned int num;
	struct hwcxext_regs_size_ctl *regs;
};

struct hwcxext_codec_regmap_cfg {
	/* write reg or update_bits */
	unsigned int value_mask;

	/* regmap config */
	int num_writeable;
	int num_unwriteable;
	int num_readable;
	int num_unreadable;
	int num_volatile;
	int num_unvolatile;
	int num_defaults;

	unsigned int *reg_writeable;
	unsigned int *reg_unwriteable;
	unsigned int *reg_readable;
	unsigned int *reg_unreadable;
	unsigned int *reg_volatile;
	unsigned int *reg_unvolatile;
	struct reg_default *reg_defaults;
	struct regmap_config cfg;
	struct regmap *regmap;
	struct hwcxext_regs_size_sequence *regs_size_seq;
};

#ifdef CONFIG_HWCXEXT_I2C_OPS
void hwcxext_i2c_dsm_report(int flag, int errno);
int hwcxext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwcxext_reg_ctl_sequence **reg_ctl, const char *seq_str);
int hwcxext_i2c_ops_regs_seq(struct hwcxext_codec_regmap_cfg *cfg,
	struct hwcxext_reg_ctl_sequence *regs_seq);
void hwcxext_i2c_free_reg_ctl(struct hwcxext_reg_ctl_sequence **reg_ctl);
int hwcxext_i2c_regmap_init(struct i2c_client *i2c,
	struct hwcxext_codec_v1_priv *hwcxext_codec);
void hwcxext_i2c_regmap_deinit(struct hwcxext_codec_regmap_cfg *cfg);
#else
static inline void hwcxext_i2c_dsm_report(int flag, int errno)
{
}

static inline int hwcxext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwcxext_reg_ctl_sequence **reg_ctl, const char *seq_str)
{
	return 0;
}

static inline int hwcxext_i2c_ops_regs_seq(struct hwcxext_reg_ctl_sequence *regs_seq)
{
	return 0;
}

static inline void hwcxext_i2c_free_reg_ctl(struct hwcxext_reg_ctl_sequence **reg_ctl)
{
}

static inline int hwcxext_i2c_regmap_init(struct i2c_client *i2c,
	struct hwcxext_codec_v1_priv *hwcxext_codec)
{
	return 0;
}

static inline void hwcxext_i2c_regmap_deinit(struct hwcxext_codec_regmap_cfg *cfg)
{
}

#endif
#endif // HWCXEXT_I2C_OPS
