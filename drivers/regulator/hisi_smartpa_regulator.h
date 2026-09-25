/*
 * FileName: hisi_sub_regulator_spmi.c
 * Copyright (C) Hisilicon technologies Co., Ltd.2015-2019. All rights reserved.
 * Description: provide regulator functions
 * Author: limengying
 * Create: 2019-03-17
 */
#ifndef HISI_SMARTPA_REGULATOR_H
#define HISI_SMARTPA_REGULATOR_H

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/err.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/regmap.h>
#include <linux/regulator/driver.h>
#include <linux/regulator/machine.h>
#include <linux/regulator/of_regulator.h>
#include <linux/slab.h>
#include <linux/time.h>

#define I2C_ADDR 0x75

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#define PMU_LOW(x, y, z, v) { .offset = (x), .mask = (y), .shift = (z), .value = (v)}

struct smartpa_regs {
	unsigned int offset;
	unsigned char mask;
	unsigned char shift;
	unsigned char value;
};

struct hisi_regulator_reg_ops {
	int (*hisi_regulator_reg_read)(int i2c_addr, int reg_addr, int *value);
	int (*hisi_regulator_reg_write)(
		int i2c_addr, int reg_addr, int mask, int value);
};

struct hisi_smartpa_regulator_time_out_regs {
	unsigned int time_out_reg;
	unsigned int mask;
	unsigned int enable;
	unsigned int disable;
};

struct hisi_smartpa_regulator_vbat_protect_regs {
	unsigned int vbat_protect_reg;
	unsigned int mask;
	unsigned int shift;
	unsigned int value;
};
struct hisi_smartpa_regulator_ctrl_regs {
	unsigned int enable_reg1;
	unsigned int enable_reg2;
};

struct hisi_smartpa_regulator_vset_reg1 {
	unsigned int vset_reg;
};
struct hisi_smartpa_regulator_vset_reg2 {
	unsigned int vset_reg;
};

struct hisi_smartpa_regulator_ctrl_data {
	unsigned int shift1;
	unsigned int mask1;
	unsigned int shift2;
	unsigned int mask2;
};

struct hisi_smartpa_regulator_vset_data1 {
	unsigned int shift;
	unsigned int mask;
	unsigned int bit;
	unsigned int byte;
};
struct hisi_smartpa_regulator_vset_data2 {
	unsigned int shift;
	unsigned int mask;
	unsigned int bit;
	unsigned int byte;
};

struct hisi_smartpa_regulator {
	int status;		   /* this property in child  node */
	unsigned int off_on_delay; /* this property in parent node */
	unsigned int time_out_flag;
	unsigned int vbat_protect_flag;
	/* this property must be unify which is in child node */
	struct hisi_smartpa_regulator_ctrl_regs ctrl_regs;
	struct hisi_smartpa_regulator_ctrl_data ctrl_data;
	struct hisi_smartpa_regulator_vset_reg1 vset_reg1;
	struct hisi_smartpa_regulator_vset_reg2 vset_reg2;
	struct hisi_smartpa_regulator_vset_data1 vset_data1;
	struct hisi_smartpa_regulator_vset_data2 vset_data2;
	struct hisi_smartpa_regulator_time_out_regs time_out_regs;
	struct hisi_smartpa_regulator_vbat_protect_regs vbat_protect_regs;
	unsigned int vol_numb;
	unsigned int *vset_table;
	struct regulator_desc rdesc;
	int (*dt_parse)(
		struct hisi_smartpa_regulator *, struct platform_device *);
};
int hisi_smartpa_flash_init(void);
int hisi_smartpa_flash_irq_enable(int enable);
#endif
