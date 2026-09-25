/*
 * hisi-usb.h
 *
 * utilityies for hifi-usb
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

#ifndef _HIFI_USB_H_
#define _HIFI_USB_H_

#include <linux/mutex.h>
#include <linux/completion.h>
#include <linux/timer.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/pm_wakeup.h>
#include <linux/notifier.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/usb/hifi-usb-mailbox.h>

#include "proxy-hcd.h"
#include "hifi-usb-urb-buf.h"
#include "hifi-usb-stat.h"

#define HIFI_USB_CONFIRM_UDEV_CONNECT_TIME (2 * HZ)
#define HIFI_USB_CONFIRM_UDEV_RECONNECT_TIME (5 * HZ)

#define HIFI_USB_HIBERNATE_DELAY 500
#define HIFI_USB_WAKEUP_TIMEOUT (5 * HZ)

#define MAX_QUIRK_DEVICES_ONE_GROUP 255

#define HIFI_USB_WAKE_LOCK_TIME 1000
#define URB_HIGH_ADDR_SHIFT 32
#define HIFI_USB_WAKE_LOCK_SHORT_TIME 200
#define HIFI_USB_WAKE_LOCK_LONG_TIME 2000

struct hifi_usb_msg_wrap {
	struct list_head node;
	struct hifi_usb_op_msg msg;
};

struct hifi_usb_phy_ldo_cfg {
	u32 addr;
	u32 bit;
	u32 always_on;
	u32 accessable; /* 0: not accessable, non-zero: accessable */
};

struct hifi_usb_proxy {
	struct proxy_hcd_client		*client;
	struct dentry			*debugfs_root;

	/* for message process */
	struct mutex			msg_lock;
	struct completion		msg_completion;
	struct work_struct		msg_work;
	struct list_head		msg_queue;
	struct hifi_usb_op_msg		op_msg;
	struct hifi_usb_runstop_msg	runstop_msg;
	unsigned int			runstop;

	struct urb_buffers		urb_bufs;
	struct list_head		complete_urb_list;
	struct timer_list		confirm_udev_timer;
	struct wakeup_source		hifi_usb_wake_lock;

	spinlock_t			lock; /* for complete_urb_list */

	struct hifi_usb_stats		stat;

	/* for hibernation */
	unsigned int			hibernation_policy;
	unsigned int			hibernation_state:1;
	unsigned int			hibernation_support:1;
	unsigned int			ignore_port_status_change_once:1;

	struct notifier_block		fb_notify;
	/* hibernation allowed when all bits cleared */
	unsigned int			hibernation_ctrl;

	__u32				port_status;
	unsigned int			hibernation_count;
	unsigned int			revive_time;
	unsigned int			max_revive_time;

	bool				hifiusb_suspended;
	bool				hifiusb_hibernating;
	bool				hid_key_pressed;
	atomic_t			hifi_reset_flag;

	struct hifi_usb_phy_ldo_cfg	hifi_usb_phy_ldo_33v;
	struct hifi_usb_phy_ldo_cfg	hifi_usb_phy_ldo_18v;

	/* for check hifi usb status */
	struct mutex			status_check_lock;
	struct delayed_work		delayed_work;
	struct completion		wakeup_completion;

	/* When stop hifi_usb, should wait for free_dev completed. */
	int				slot_id;
	wait_queue_head_t		free_dev_wait_queue;

#ifdef CONFIG_HIFI_USB_HAS_H2X
	/*
	 * for onetrack of apr es and cs
	 * es: using h2x
	 * cs: using x2x
	 */
	bool				usb_not_using_h2x;
#endif
};

enum hibernation_ctrl_type {
	USB_CTRL = 0,
	FB_CTRL = 1,
	AUDIO_CTRL = 2 /* corresponding bit set to 1 means allowing hibernate */
};

struct complete_urb_wrap {
	struct list_head list_node;
	struct hifi_usb_op_msg op_msg;
};

extern u32 hifi_usb_base_quirk_devices[MAX_QUIRK_DEVICES_ONE_GROUP + 1];
extern u32 hifi_usb_ext_quirk_devices[MAX_QUIRK_DEVICES_ONE_GROUP + 1];

void hifi_usb_msg_receiver(struct hifi_usb_op_msg *__msg);
void hifi_usb_announce_udev(struct usb_device *udev);
int always_use_hifi_usb(int val);
int never_use_hifi_usb(int val);

#define HUAWEI_USB_C_AUDIO_ADAPTER "HUAWEI USB-C TO 3.5MM AUDIO ADAPTER"

#define HUAWEI_EARPHONE_VENDOR_ID	0x12d1
#define HUAWEI_EARPHONE_PRODUCT_ID	0x3a07
#define USB_REQ_VENDOR_FEATURE		0x1
#define MAX_EARPHONE_FEATURE_LEN	4

#define HIFI_USB_INVALID_REVIVE_VALUE	9999
#define HUB_CTRL_BUF_LEN	4

#endif /* _HIFI_USB_H_ */
