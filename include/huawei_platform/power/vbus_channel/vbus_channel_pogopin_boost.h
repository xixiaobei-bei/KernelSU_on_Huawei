/*
 * vbus_channel_pogopin_boost.h
 *
 * pogopin boost for vbus channel driver
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

#ifndef _VBUS_CHANNEL_POGOPIN_BOOST_H_
#define _VBUS_CHANNEL_POGOPIN_BOOST_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/delay.h>

#define BUCKBOOST_GPIO_SWITCH_DISABLE       0
#define BUCKBOOST_GPIO_SWITCH_ENABLE        1

#define MOS_GPIO_SWITCH_DISABLE             0
#define MOS_GPIO_SWITCH_ENABLE              1

#define OTG_GPIO_SWITCH_DISABLE             0
#define OTG_GPIO_SWITCH_ENABLE              1

struct pogopin_boost_dev {
	struct platform_device *pdev;
	struct device *dev;
	int buckboost_en;
	int mos_en;
	unsigned int otg_en;
	unsigned int otg_en_flag;
	unsigned int user;
	/* fix a hardware issue, has leakage when open boost gpio */
	u32 charge_otg_ctl;
};

#endif /* _VBUS_CHANNEL_POGOPIN_BOOST_H_ */
