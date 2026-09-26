/*
 * vbus_channel_charger.h
 *
 * charger otg for vbus channel driver
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

#ifndef _VBUS_CHANNEL_CHARGER_H_
#define _VBUS_CHANNEL_CHARGER_H_

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/usb/otg.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_address.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

#define CHARGER_OTG_SYS_WDT_TIMEOUT    180
#define CHARGER_OTG_WORK_TIME          30000

enum charger_otg_device_id {
	OTG_DEVICE_ID_BEGIN = 0,
	OTG_DEVICE_ID_BQ2419X = OTG_DEVICE_ID_BEGIN, /* for bq2419x */
	OTG_DEVICE_ID_BQ2429X, /* for bq2429x */
	OTG_DEVICE_ID_BQ2560X, /* for bq2560x */
	OTG_DEVICE_ID_BQ25882, /* for bq25882 */
	OTG_DEVICE_ID_BQ25892, /* for bq25892 */
	OTG_DEVICE_ID_HL7019, /* for hl7019 */
	OTG_DEVICE_ID_RT9466, /* for rt9466 */
	OTG_DEVICE_ID_RT9471, /* for rt9471 */
	OTG_DEVICE_ID_HI6522, /* for hi6522 */
	OTG_DEVICE_ID_HI6523, /* for hi6523 */
	OTG_DEVICE_ID_HI6526, /* for hi6526 */
	OTG_DEVICE_ID_DUAL_CHARGER, /* for dual_charger */
	OTG_DEVICE_ID_BD99954, /* for bd99954 */
	OTG_DEVICE_ID_BQ25713, /* for bq25713 */
	OTG_DEVICE_ID_END,
};

struct charger_otg_device_ops {
	const char *dev_name;
	int (*otg_set_charger_enable)(int enable);
	int (*otg_set_enable)(int enable);
	int (*otg_set_current)(int value);
	int (*otg_set_watchdog_timer)(int value);
	int (*otg_reset_watchdog_timer)(void);
	int (*otg_set_switch_mode)(int mode);
};

struct charger_otg_dev {
	struct platform_device *pdev;
	struct device *dev;
	struct wakeup_source otg_lock;
	struct delayed_work otg_work;
	int dev_id;
	u32 otg_curr;
	unsigned int user;
	int mode;
	struct charger_otg_device_ops *ops;
};

#ifdef CONFIG_VBUS_CHANNEL_CHARGER
int charger_otg_ops_register(struct charger_otg_device_ops *ops);
#else
static inline int charger_otg_ops_register(struct charger_otg_device_ops *ops)
{
	return -1;
}
#endif /* CONFIG_VBUS_CHANNEL_CHARGER */

#endif /* _VBUS_CHANNEL_CHARGER_H_ */
