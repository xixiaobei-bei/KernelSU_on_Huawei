/*
 * multi_switchcap.h
 *
 * multi switchcap driver
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

#ifndef _MULTI_SWITCHCAP_H_
#define _MULTI_SWITCHCAP_H_

#include <huawei_platform/power/direct_charger/direct_charger.h>

#define MULTI_SWITCHCAP_NUM   5

struct ovp_ops {
	int (*ovp_enable)(int);
};

struct multi_sc_ops {
	int (*sc_init)(void *data);
	int (*sc_enable)(void *data, int enable);
	int (*is_sc_close)(void *data);
	int (*get_sc_id)(void *data);
	int (*sc_exit)(void *data);
	int (*sc_chip_power)(void *data, bool enable);
};

struct multi_sc_chip_info {
	int dev_id;
	struct multi_sc_ops *ops;
	void *data;
};

struct multi_sc_info {
	struct platform_device *pdev;
	struct device *dev;
	int gpio_btst_delay;
	u32 sc_num;
	u32 gpio_high_effect;
};

#ifdef CONFIG_MULTI_SWITCHCAP
int multi_switchcap_chip_register(struct multi_sc_chip_info *info);
int ovp_for_sc_ops_register(struct ovp_ops *ops);
#else
static inline int multi_switchcap_chip_register(
	struct multi_sc_chip_info *info)
{
	return 0;
}
static inline int ovp_for_sc_ops_register(struct ovp_ops *ops)
{
	return 0;
}
#endif /* CONFIG_MULTI_SWITCHCAP */

#endif /* _MULTI_SWITCHCAP_H_ */
