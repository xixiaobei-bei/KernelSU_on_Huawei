/*
 * vbus_channel_boost_gpio.h
 *
 * boost with gpio for vbus channel driver
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

#ifndef _VBUS_CHANNEL_BOOST_GPIO_H_
#define _VBUS_CHANNEL_BOOST_GPIO_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <chipset_common/hwpower/hardware_channel/vbus_channel_error_handle.h>

#define BOOST_GPIO_SWITCH_DISABLE              0
#define BOOST_GPIO_SWITCH_ENABLE               1
#define BOOST_GPIO_OTG_SC_VOL_MV               3000 /* 3000 mv */
#define BOOST_GPIO_OTG_SC_CHECK_TIMES          3    /* 3 times */
#define BOOST_GPIO_OTG_SC_CHECK_DELAYED_TIME   1000 /* 1000 ms */

/* fix a hardware issue, has leakage when open boost gpio */
#define CHARGE_OTG_CLOSE_WORK_TIMEOUT          3000

struct boost_gpio_dev {
	struct platform_device *pdev;
	struct device *dev;
	int gpio_en;
	unsigned int user;
	int mode;
	/* fix a hardware issue, has leakage when open boost gpio */
	u32 charge_otg_ctl_flag;
	struct delayed_work charge_otg_close_work;
	/* fix a hardware issue, has short circuit when open boost gpio */
	u32 otg_sc_check_enable;
	struct otg_sc_para otg_sc_check_para;
	/* fix a hardware issue, otg boost over current protect */
	u32 otg_ocp_check_enable;
	int otg_ocp_int;
	int otg_ocp_irq;
	struct work_struct otg_ocp_work;
};

#endif /* _VBUS_CHANNEL_BOOST_GPIO_H_ */
