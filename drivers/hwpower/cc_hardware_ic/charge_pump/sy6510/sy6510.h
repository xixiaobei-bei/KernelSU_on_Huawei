/*
 * sy6510.h
 *
 * charge-pump sy6510 macro, addr etc. (bp: bypass mode; cp: charge-pump mode)
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#ifndef _SY6510_H_
#define _SY6510_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/delay.h>

#define SY6510_ADDR_LEN              1

/* reg0x00: enable */
#define SY6510_REG_00                0x00
#define SY6510_00_INIT_VAL           0xAE

/* reg0x01: iin limit / force bypass / force cp */
#define SY6510_REG_01                0x01
#define SY6510_01_INIT_VAL           0x40
#define SY6510_01_FORCE_BPCP_SHIFT   2
#define SY6510_01_FORCE_BPCP_MASK    BIT(2)
#define SY6510_01_FORCE_BP_EN        1
#define SY6510_01_FORCE_CP_EN        0

/* reg0x02: status */
#define SY6510_REG_02                0x02
#define SY6510_02_POWER_READY_MASK   BIT(0)
#define SY6510_02_POWER_READY_SHIFT  0
#define SY6510_02_INIT_VAL           0x47

/* reg0x03: reverse bypass mode */
#define SY6510_REG_03                0x03
#define SY6510_03_INIT_VAL           0x00
#define SY6510_03_REVERSE_BP_MASK    BIT(7)
#define SY6510_03_REVERSE_BP_SHIFT   7
#define SY6510_03_REVERSE_BP_EN      1

struct sy6510_dev_info {
	struct i2c_client *client;
	struct device *dev;
};

#endif /* _SY6510_H_ */
