/*
 * power_test.h
 *
 * common interface, variables, definition etc for power test
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

#ifndef _POWER_TEST_H_
#define _POWER_TEST_H_

#include <linux/list.h>
#include <linux/device.h>

#define POWER_TEST_MAX_OPS            32

struct power_test_ops {
	struct list_head ptst_node;
	const char *name; /* same as dts variable name */
	unsigned int enable;
	void (*init)(struct device *dev);
	void (*exit)(struct device *dev);
};

struct power_test_dev {
	struct device *dev;
	struct device_node *np;
};

int power_test_ops_register(struct power_test_ops *ops);

#endif /* _POWER_TEST_H_ */
