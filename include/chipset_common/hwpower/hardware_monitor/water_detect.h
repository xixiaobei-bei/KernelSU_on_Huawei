/*
 * water_detect.h
 *
 * water intruded detect driver
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

#ifndef _WATER_DETECT_H_
#define _WATER_DETECT_H_

#include <linux/notifier.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/kobject.h>
#include <linux/slab.h>

enum water_detect_type {
	WD_TYPE_BEGIN = 0,
	WD_TYPE_USB_DP_DN = WD_TYPE_BEGIN, /* usb d+/d- pin */
	WD_TYPE_USB_ID, /* usb id pin */
	WD_TYPE_USB_GPIO, /* usb gpio */
	WD_TYPE_AUDIO_DP_DN, /* audio d+/d- pin */
	WD_TYPE_END,
};

enum water_detect_intruded_status {
	WD_NON_STBY_DRY, /* dry on non-standby mode */
	WD_NON_STBY_MOIST, /* moist on non-standby mode */
	WD_STBY_DRY, /* dry on standby mode */
	WD_STBY_MOIST, /* moist on standby mode */
};

enum water_detect_operate_mode {
	WD_OP_SET,
	WD_OP_CLR,
};

struct water_detect_enable {
	unsigned int usb_dp_dn;
	unsigned int usb_id;
	unsigned int usb_gpio;
	unsigned int audio_dp_dn;
};

struct water_detect_ops {
	const char *type_name;
	void *dev_data;
	int (*is_water_intruded)(void *);
};

struct water_detect_dev {
	struct notifier_block nb;
	struct water_detect_enable enable;
	struct water_detect_ops *ops[WD_TYPE_END];
	unsigned int total_ops;
	unsigned int status;
};

#ifdef CONFIG_HUAWEI_WATER_DETECT
int water_detect_ops_register(struct water_detect_ops *ops);
bool water_detect_get_intruded_status(void);
#else
static inline int water_detect_ops_register(struct water_detect_ops *ops)
{
	return -1;
}

static inline bool water_detect_get_intruded_status(void)
{
	return false;
}
#endif /* CONFIG_HUAWEI_WATER_DETECT */

#endif /* _WATER_DETECT_H_ */
