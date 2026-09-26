/*
 * bias_bl_common.c
 *
 * bias_bl_common driver for lcd bias ic and backlight ic
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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
#include "bias_bl_common.h"

static struct bias_bl_common_ops *bias_bl_ops;

void bias_bl_adapt_register(struct bias_bl_common_ops *ops)
{
	if (bias_bl_ops) {
		printk(KERN_INFO "bias_bl_ops has already been registered!\n");
		return;
	}
	if (ops == NULL) {
		printk(KERN_INFO "bias_bl_ops register NULL!\n");
		return;
	}

	bias_bl_ops = ops;
}

void bias_bl_adapt_unregister(struct bias_bl_common_ops *ops)
{
	if (ops == NULL) {
		printk(KERN_INFO "bias_bl_ops unregister NULL!\n");
		return;
	}
	if (bias_bl_ops == ops) {
		bias_bl_ops = NULL;
		printk(KERN_INFO "bias_bl_ops unregister success!\n");
		return;
	}
	printk(KERN_INFO "bias_bl_ops unregister fail!\n");
}

struct bias_bl_common_ops *bias_bl_get_ops(void)
{
	return bias_bl_ops;
}
