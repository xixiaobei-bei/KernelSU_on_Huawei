/*
 * wireless_accessory.h
 *
 * wireless accessory driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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

#ifndef _WIRELESS_ACCESSORY_H_
#define _WIRELESS_ACCESSORY_H_

#include <linux/device.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>

#define ACC_DEVICE_OFFLINE       0
#define ACC_DEVICE_ONLINE        1
#define ACC_CONNECTED_STR        "CONNECTED"
#define ACC_DISCONNECTED_STR     "DISCONNECTED"
#define ACC_UNKNOWN_STR          "UNKNOWN"
#define ACC_PING_SUCC_STR        "PING_SUCC"
#define ACC_PING_TIMEOUT_STR     "PING_TIMEOUT"
#define ACC_CHARGE_DONE_STR      "CHARGE_DONE"
#define ACC_CHARGE_ERROR_STR     "CHARGE_ERR"
#define ACC_DEV_INFO_MAX         512
/* name_len + len('=') + vaule_len + len('\0') */
#define ACC_DEV_INFO_LEN         (ACC_NAME_MAX_LEN + ACC_VALUE_MAX_LEN + 2)
#define ACC_DEV_INFO_NUM_MAX     20
#define ACC_NAME_MAX_LEN         23
#define ACC_VALUE_MAX_LEN        31

/* dev no should begin from 1 */
enum wireless_acc_dev_no {
	ACC_DEV_NO_BEGIN = 1,
	ACC_DEV_NO_PEN = ACC_DEV_NO_BEGIN,
	ACC_DEV_NO_KB,
	ACC_DEV_NO_END,
};

struct wireless_acc_key_info {
	char name[ACC_NAME_MAX_LEN + 1];
	char value[ACC_VALUE_MAX_LEN + 1];
};

struct wireless_acc_dev_info {
	struct wireless_acc_key_info *key_info;
	u8 info_no; /* the informartion number of each device */
};

struct wireless_acc_dev {
	struct device *dev;
	struct wireless_acc_dev_info dev_info[ACC_DEV_NO_END];
};

#ifdef CONFIG_WIRELESS_ACCESSORY
void wireless_acc_report_uevent(struct wireless_acc_key_info *key_info,
	u8 info_no, u8 dev_no);
#else
static inline void wireless_acc_report_uevent(
	struct wireless_acc_key_info *key_info, u8 info_no, u8 dev_no)
{
}
#endif /* CONFIG_WIRELESS_ACCESSORY */

#endif /* _WIRELESS_ACCESSORY_H_ */
