/*
 * hwevext_i2c_ops.h
 *
 * hwevext_i2c_ops header file
 *
 * Copyright (c) 2021 Huawei Technologies Co., Ltd.
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

#ifndef __HWEVEXT_I2C_OPS__
#define __HWEVEXT_I2C_OPS__

#include <linux/i2c.h>

#define HWEVEXT_I2C_READ      0
#define HWEVEXT_I2C_WRITE     1

#ifdef CONFIG_HUAWEI_DSM_AUDIO
#ifndef DSM_BUF_SIZE
#define DSM_BUF_SIZE 1024
#endif
#endif

struct hwevext_reg_ctl {
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

struct hwevext_reg_ctl_sequence {
	unsigned int num;
	struct hwevext_reg_ctl *regs;
};

#ifdef CONFIG_HWEVEXT_I2C_OPS
void hwevext_i2c_dsm_report(int flag, int errno);
void hwevext_i2c_ops_store_regmap(struct regmap *regmap);
void hwevext_i2c_set_reg_value_mask(int val_bits);
int hwevext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwevext_reg_ctl_sequence **reg_ctl, const char *seq_str);
int hwevext_i2c_ops_regs_seq(struct hwevext_reg_ctl_sequence *regs_seq);
void hwevext_i2c_free_reg_ctl(struct hwevext_reg_ctl_sequence *reg_ctl);
#else
static inline void hwevext_i2c_dsm_report(int flag, int errno)
{
}

static inline void hwevext_i2c_ops_store_regmap(struct regmap *regmap)
{
}

static inline void hwevext_i2c_set_reg_value_mask(int val_bits)
{

}

static inline int hwevext_i2c_parse_dt_reg_ctl(struct i2c_client *i2c,
	struct hwevext_reg_ctl_sequence **reg_ctl, const char *seq_str)
{
	return 0;
}

static inline int hwevext_i2c_ops_regs_seq(struct hwevext_reg_ctl_sequence *regs_seq)
{
	return 0;
}

static inline void hwevext_i2c_free_reg_ctl(struct hwevext_reg_ctl_sequence *reg_ctl)
{
}
#endif
#endif // HWEVEXT_I2C_OPS