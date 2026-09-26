/*
 * hifi-usb.h
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

#include <linux/completion.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/hisi/usb/hisi_usb_reg_cfg.h>
#include <linux/mutex.h>
#include <linux/notifier.h>
#include <linux/pm_wakeup.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/usb/hifi-usb-mailbox.h>
#include <linux/wait.h>
#include "hifi-usb-stat.h"
#include "hifi-usb-urb-buf.h"
#include "proxy-hcd.h"

#define HIFI_USB_CONFIRM_UDEV_CONNECT_TIME (4 * HZ)
#define HIFI_USB_CONFIRM_UDEV_RECONNECT_TIME (5 * HZ)

#define HIFI_USB_HIBERNATE_DELAY 500
#define HIFI_USB_WAKEUP_TIMEOUT (5 * HZ)

#define MAX_QUIRK_DEVICES_ONE_GROUP 255

#define HIFI_USB_WAKE_LOCK_TIME 1000
#define URB_HIGH_ADDR_SHIFT 32
#define HIFI_USB_WAKE_LOCK_SHORT_TIME 200
#define HIFI_USB_WAKE_LOCK_LONG_TIME 2000

#define HUB_CTRL_BUF_LEN	4

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
	struct proxy_hcd_client *client;
	struct dentry *debugfs_root;

	/* for message process */
	struct mutex			msg_lock;
	struct completion		msg_completion;
	struct work_struct		msg_work;
	struct list_head		msg_queue;
	struct hifi_usb_op_msg		op_msg;
	struct hifi_usb_runstop_msg	runstop_msg;
	unsigned int			runstop;

	struct urb_buffers		urb_bufs;
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	struct list_head		complete_urb_list;
#endif
	struct timer_list		confirm_udev_timer;
	struct wakeup_source		hifi_usb_wake_lock;

	spinlock_t			lock; /* for complete_urb_list */

	struct hifi_usb_stats		stat;

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	/* for hibernation */
	unsigned int			hibernation_policy;
	unsigned int			hibernation_state : 1;
	unsigned int			hibernation_support : 1;
	unsigned int			ignore_port_status_change_once : 1;

	struct notifier_block		fb_notify;
	/* hibernation allowed when all bits cleared */
	unsigned int			hibernation_ctrl;

	__u32				port_status;
	unsigned int			hibernation_count;
	unsigned int			revive_time;
	unsigned int			max_revive_time;
	bool				hifiusb_hibernating;
#endif
	bool				hifiusb_suspended;
	bool				hid_key_pressed;
	atomic_t			hifi_reset_flag;

	struct hifi_usb_phy_ldo_cfg	hifi_usb_phy_ldo_33v;
	struct hifi_usb_phy_ldo_cfg	hifi_usb_phy_ldo_18v;

	/* controller qos */
	struct chip_usb_reg_cfg *qos_set;
	struct chip_usb_reg_cfg *qos_reset;

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	/* for check hifi usb status */
	struct mutex			status_check_lock;
	struct delayed_work		delayed_work;
	struct completion		wakeup_completion;
#endif
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

struct complete_urb_wrap {
	struct list_head list_node;
	struct hifi_usb_op_msg op_msg;
};

static inline struct usb_device *hifi_usb_to_udev(struct hifi_usb_proxy *proxy)
{
	struct proxy_hcd *phcd = proxy->client->phcd;
	struct usb_device *udev = phcd->phcd_udev.udev;
	return udev;
}

void hifi_usb_msg_receiver(struct hifi_usb_op_msg *__msg);
void hifi_usb_announce_udev(struct usb_device *udev);
struct hifi_usb_proxy *get_hifi_usb_proxy_handle(void);
int hifi_usb_runstop_and_wait(struct hifi_usb_proxy *proxy, bool run);
int hifi_usb_proxy_alloc_dev_unlocked(struct hifi_usb_proxy *proxy,
				      int *slot_id);
int hifi_usb_proxy_hub_control_unlocked(struct proxy_hcd_client *client,
					struct usb_ctrlrequest *cmd,
					char *buf);

#endif /* _HIFI_USB_H_ */
