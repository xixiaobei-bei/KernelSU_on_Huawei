/*
 * hw_buck.h
 *
 * buck common header file
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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


#ifndef _HUAWEI_BUCK_H_
#define _HUAWEI_BUCK_H_

#include <linux/debugfs.h>
#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <media/v4l2-subdev.h>

struct hw_buck_info {
	const char *name;
	unsigned int slave_address;
	unsigned int en_pin;
	unsigned int pin_vsel;
};

struct hw_buck_ctrl_t;

struct hw_buck_fn_t {
	/* buck function table */
	int (*buck_init)(struct hw_buck_ctrl_t *);
	int (*buck_exit)(struct hw_buck_ctrl_t *);
	int (*buck_match)(struct hw_buck_ctrl_t *);
	int (*buck_get_dt_data)(struct hw_buck_ctrl_t *);
	int (*buck_power_config)(struct hw_buck_ctrl_t *, u32, int);
};

struct hw_buck_ctrl_t {
	struct platform_device *pdev;
	struct mutex *buck_mutex;
	struct device *dev;
	struct hw_buck_fn_t *func_tbl;
	struct hw_buck_i2c_client *buck_i2c_client;
	struct hw_buck_info buck_info;
	void *pdata;
};

struct hw_buck_i2c_client {
	struct hw_buck_i2c_fn_t *i2c_func_tbl;
	struct i2c_client *client;
};

struct hw_buck_i2c_fn_t {
	int (*i2c_read)(struct hw_buck_i2c_client *, u8, u8 *);
	int (*i2c_write)(struct hw_buck_i2c_client *, u8, u8);
};

struct buck_i2c_reg {
	unsigned int address;
	unsigned int value;
};

struct hw_buck_ctrl_t *hw_get_buck_ctrl(void);
int hw_buck_remove(struct i2c_client *client);
int32_t hw_buck_i2c_probe(struct i2c_client *client,
			const struct i2c_device_id *id);

int hw_buck_get_dt_data(struct hw_buck_ctrl_t *buck_ctrl);

#endif
