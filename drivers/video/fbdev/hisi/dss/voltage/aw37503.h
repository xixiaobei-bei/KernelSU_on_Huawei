/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#ifndef __AW37503_H
#define __AW37503_H

#define AW37503_VOL_40     0x00     // 4.0V
#define AW37503_VOL_41     0x01     // 4.1V
#define AW37503_VOL_42     0x02     // 4.2V
#define AW37503_VOL_43     0x03     // 4.3V
#define AW37503_VOL_44     0x04     // 4.4V
#define AW37503_VOL_45     0x05     // 4.5V
#define AW37503_VOL_46     0x06     // 4.6V
#define AW37503_VOL_47     0x07     // 4.7V
#define AW37503_VOL_48     0x08     // 4.8V
#define AW37503_VOL_49     0x09     // 4.9V
#define AW37503_VOL_50     0x0A     // 5.0V
#define AW37503_VOL_51     0x0B     // 5.1V
#define AW37503_VOL_52     0x0C     // 5.2V
#define AW37503_VOL_53     0x0D     // 5.3V
#define AW37503_VOL_54     0x0E     // 5.4V
#define AW37503_VOL_55     0x0F     // 5.5V
#define AW37503_VOL_56     0x10     // 5.6V
#define AW37503_VOL_57     0x11     // 5.7V
#define AW37503_VOL_58     0x12     // 5.8V
#define AW37503_VOL_59     0x13     // 5.9V
#define AW37503_VOL_60     0x14     // 6.0V
#define AW37503_VOL_MAX    0x15     // 6.1V

#define AW37503_REG_VPOS   0x00
#define AW37503_REG_VNEG   0x01
#define AW37503_REG_APP_DIS   0x03
#define AW37503_REG_VENDORID  0x04
#define AW37503_SLAV_ADDR  0x3E
#define AW37503_ENABLE_FLAG  0x01

#define AW37503_REG_VOL_MASK  0x1F
#define AW37503_APPS_BIT   (1 << 6)
#define AW37503_DISP_BIT   (1 << 1)
#define AW37503_DISN_BIT   (1 << 0)

struct aw37503_voltage {
	u32 voltage;
	int value;
};

static struct aw37503_voltage vol_table[] = {
	{ 4000000, AW37503_VOL_40 },
	{ 4100000, AW37503_VOL_41 },
	{ 4200000, AW37503_VOL_42 },
	{ 4300000, AW37503_VOL_43 },
	{ 4400000, AW37503_VOL_44 },
	{ 4500000, AW37503_VOL_45 },
	{ 4600000, AW37503_VOL_46 },
	{ 4700000, AW37503_VOL_47 },
	{ 4800000, AW37503_VOL_48 },
	{ 4900000, AW37503_VOL_49 },
	{ 5000000, AW37503_VOL_50 },
	{ 5100000, AW37503_VOL_51 },
	{ 5200000, AW37503_VOL_52 },
	{ 5300000, AW37503_VOL_53 },
	{ 5400000, AW37503_VOL_54 },
	{ 5500000, AW37503_VOL_55 },
	{ 5600000, AW37503_VOL_56 },
	{ 5700000, AW37503_VOL_57 },
	{ 5800000, AW37503_VOL_58 },
	{ 5900000, AW37503_VOL_59 },
	{ 6000000, AW37503_VOL_60 }
};


struct aw37503_device_info {
	struct device *dev;
	struct i2c_client *client;
};

struct work_data {
	struct i2c_client *client;
	struct delayed_work setvol_work;
	int vpos;
	int vneg;
};

struct aw37503_configure_info {
	char *lcd_name;
	int vpos_cmd;
	int vneg_cmd;
};

#endif
