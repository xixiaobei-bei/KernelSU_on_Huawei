/*
 * hisi-usb-debugfs.h
 *
 * utilityies for hifi-usb debugging
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HIFI_USB_DEBUGFS_H
#define _HIFI_USB_DEBUGFS_H

#include "hifi-usb.h"

#define MAX_HIFI_TEST_CMD_LEN   32
#define MAX_HIFI_CMD_LEN        32

/* hifi usb fault inject */

enum inject_point {
	INJECT_AT_NULL,
	INJECT_AT_STARTSTOP,
	INJECT_AT_HCD_MESG,
	INJECT_AT_COMPLETE,
	INJECT_END,
};

enum fault_type {
	NONE_FAULT,
	RESET_HIFI,
	MISS_IPC,
	FAULT_END,
};

struct fault_inject_point {
	char *name;
	size_t len;
	enum inject_point point;
	u32 data;
};

#ifdef CONFIG_HISI_DEBUG_FS
int hifi_usb_debugfs_init(struct hifi_usb_proxy *hifi_usb);
void hifi_usb_debugfs_exit(struct hifi_usb_proxy *hifi_usb);
void hifi_usb_handle_test_mesg(struct hifi_usb_proxy *hifi_usb,
		struct hifi_usb_test_msg *mesg);
#else
static inline int hifi_usb_debugfs_init(struct hifi_usb_proxy *hifi_usb)
{
	return 0;
}
static inline void hifi_usb_debugfs_exit(struct hifi_usb_proxy *hifi_usb)
{
}
static inline void hifi_usb_handle_test_mesg(struct hifi_usb_proxy *hifi_usb,
		struct hifi_usb_test_msg *mesg){}
#endif /* CONFIG_HISI_DEBUG_FS */

#endif /* _HIFI_USB_DEBUGFS_H */
