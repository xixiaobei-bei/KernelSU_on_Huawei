/*
 * hw_accessory.h
 *
 * huawei accessory driver
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

#ifndef _HW_ACCESSORY_H_
#define _HW_ACCESSORY_H_

#define ACC_DEVICE_OFFLINE       0
#define ACC_DEVICE_ONLINE        1
#define ACC_CONNECTED_STR        "CONNECTED"
#define ACC_DISCONNECTED_STR     "DISCONNECTED"
#define ACC_UNKNOWN_STR          "UNKNOWN"
#define ACC_PING_SUCC_STR        "PING_SUCC"
#define ACC_DEV_INFO_MAX         512
/* name_len + len('=') + vaule_len + len('\0') */
#define ACC_DEV_INFO_LEN         (NAME_MAX_LEN + VALUE_MAX_LEN + 2)
#define ACC_DEV_INFO_NUM_MAX     20
#define NAME_MAX_LEN             23
#define VALUE_MAX_LEN            31

/* dev no begin 1 */
enum acc_dev_no {
	ACC_DEV_NO_BEGIN = 1,
	ACC_DEV_NO_PEN = ACC_DEV_NO_BEGIN,
	ACC_DEV_NO_KB,
	ACC_DEV_NO_MAX,
};

struct key_info {
	char name[NAME_MAX_LEN + 1];
	char value[VALUE_MAX_LEN + 1];
};

struct dev_info {
	struct key_info *p_info; /* pointer to device information */
	u8 info_no; /* the informartion number of each device */
};

struct acc_info {
	struct platform_device *pdev;
	struct device *dev;
	struct dev_info dev_info_tab[ACC_DEV_NO_MAX];
};

#ifdef CONFIG_WIRELESS_ACCESSORY
extern void accessory_notify_android_uevent(struct key_info *p_dev_info,
	u8 dev_info_no, u8 dev_no);
#else
static inline void accessory_notify_android_uevent(
	struct key_info *p_dev_info, u8 dev_info_no, u8 dev_no)
{
}
#endif /* CONFIG_WIRELESS_ACCESSORY */

#endif /* _HW_ACCESSORY_H_ */
