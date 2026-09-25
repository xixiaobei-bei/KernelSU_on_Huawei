/*
 * bias_ic_common.h
 *
 * bias_ic_common driver for i2c device lcd bias ic
 *
 * Copyright (c) 2019 Huawei Technologies Co., Ltd.
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
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/delay.h>
#include <linux/uaccess.h>

#define BIAS_IC_READ_INHIBITION 0x01
#define BIAS_IC_HAVE_E2PROM 0x02
#define BIAS_IC_CONFIG_NEED_ENABLE 0x04
#define BIAS_IC_RESUME_NEED_CONFIG 0x08

/* bias ic: tps65132 */
enum tps65132_val_type {
	TPS65132_VOL_40 = 0x00, /* 4.0V */
	TPS65132_VOL_41 = 0x01, /* 4.1V */
	TPS65132_VOL_42 = 0x02, /* 4.2V */
	TPS65132_VOL_43 = 0x03, /* 4.3V */
	TPS65132_VOL_44 = 0x04, /* 4.4V */
	TPS65132_VOL_45 = 0x05, /* 4.5V */
	TPS65132_VOL_46 = 0x06, /* 4.6V */
	TPS65132_VOL_47 = 0x07, /* 4.7V */
	TPS65132_VOL_48 = 0x08, /* 4.8V */
	TPS65132_VOL_49 = 0x09, /* 4.9V */
	TPS65132_VOL_50 = 0x0A, /* 5.0V */
	TPS65132_VOL_51 = 0x0B, /* 5.1V */
	TPS65132_VOL_52 = 0x0C, /* 5.2V */
	TPS65132_VOL_53 = 0x0D, /* 5.3V */
	TPS65132_VOL_54 = 0x0E, /* 5.4V */
	TPS65132_VOL_55 = 0x0F, /* 5.5V */
	TPS65132_VOL_56 = 0x10, /* 5.6V */
	TPS65132_VOL_57 = 0x11, /* 5.7V */
	TPS65132_VOL_58 = 0x12, /* 5.8V */
	TPS65132_VOL_59 = 0x13, /* 5.9V */
	TPS65132_VOL_60 = 0x14, /* 6.0V */
	TPS65132_VOL_MAX
};

/* bias ic: rt4801h */
enum rt4801h_val_type {
	RT4801H_VOL_40 = 0x00, /* 4.0V */
	RT4801H_VOL_41 = 0x01, /* 4.1V */
	RT4801H_VOL_42 = 0x02, /* 4.2V */
	RT4801H_VOL_43 = 0x03, /* 4.3V */
	RT4801H_VOL_44 = 0x04, /* 4.4V */
	RT4801H_VOL_45 = 0x05, /* 4.5V */
	RT4801H_VOL_46 = 0x06, /* 4.6V */
	RT4801H_VOL_47 = 0x07, /* 4.7V */
	RT4801H_VOL_48 = 0x08, /* 4.8V */
	RT4801H_VOL_49 = 0x09, /* 4.9V */
	RT4801H_VOL_50 = 0x0A, /* 5.0V */
	RT4801H_VOL_51 = 0x0B, /* 5.1V */
	RT4801H_VOL_52 = 0x0C, /* 5.2V */
	RT4801H_VOL_53 = 0x0D, /* 5.3V */
	RT4801H_VOL_54 = 0x0E, /* 5.4V */
	RT4801H_VOL_55 = 0x0F, /* 5.5V */
	RT4801H_VOL_56 = 0x10, /* 5.6V */
	RT4801H_VOL_57 = 0x11, /* 5.7V */
	RT4801H_VOL_58 = 0x12, /* 5.8V */
	RT4801H_VOL_59 = 0x13, /* 5.9V */
	RT4801H_VOL_60 = 0x14, /* 6.0V */
	RT4801H_VOL_MAX
};

/* bias ic: lv52134a */
enum lv52134a_val_type {
	LV52134A_VOL_41 = 0x01, /* 4.1V */
	LV52134A_VOL_42 = 0x02, /* 4.2V */
	LV52134A_VOL_43 = 0x03, /* 4.3V */
	LV52134A_VOL_44 = 0x04, /* 4.4V */
	LV52134A_VOL_45 = 0x05, /* 4.5V */
	LV52134A_VOL_46 = 0x06, /* 4.6V */
	LV52134A_VOL_47 = 0x07, /* 4.7V */
	LV52134A_VOL_48 = 0x08, /* 4.8V */
	LV52134A_VOL_49 = 0x09, /* 4.9V */
	LV52134A_VOL_50 = 0x0A, /* 5.0V */
	LV52134A_VOL_51 = 0x0B, /* 5.1V */
	LV52134A_VOL_52 = 0x0C, /* 5.2V */
	LV52134A_VOL_53 = 0x0D, /* 5.3V */
	LV52134A_VOL_54 = 0x0E, /* 5.4V */
	LV52134A_VOL_55 = 0x0F, /* 5.5V */
	LV52134A_VOL_56 = 0x10, /* 5.6V */
	LV52134A_VOL_57 = 0x11, /* 5.7V */
	LV52134A_VOL_MAX
};

struct bias_ic_info {
	unsigned char ic_type;
	unsigned char vpos_reg;
	unsigned char vneg_reg;
	unsigned char vpos_val;
	unsigned char vneg_val;
	unsigned char vpos_mask;
	unsigned char vneg_mask;
	unsigned char state_reg;
	unsigned char state_val;
	unsigned char state_mask;
};

struct bias_ic_device {
	struct device *dev;
	struct i2c_client *client;
	struct bias_ic_info bias_config;
};
