/*
 * hisi_usb_hibernate.c
 *
 * hibernate function for hifi-usb
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

#include "hifi-usb.h"
#include <linux/hisi/h2x_interface.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/pm_wakeup.h>
#include <linux/usb/ch11.h>
#include <linux/usb/ch9.h>
#include <linux/platform_device.h>
#include <linux/usb/hifi-usb-mailbox.h>
#include <linux/workqueue.h>
#include "hifi-usb-internal.h"
#include "hifi-usb-ldo.h"
#include "hifi-usb-hibernate.h"
#include "usbaudio-monitor.h"
#ifdef CONFIG_FB
#include <linux/fb.h>
#endif

#define HIFI_USB_INVALID_REVIVE_VALUE	9999
#define USB_REQ_VENDOR_FEATURE		0x1
#define MAX_EARPHONE_FEATURE_LEN	4

#define dbg(format, arg...) pr_debug("[phcd][DBG][%s]" format, __func__, ##arg)
#define info(format, arg...) pr_info("[phcd][INFO][%s]" format, __func__, ##arg)
#define err(format, arg...) pr_err("[phcd][ERR][%s]" format, __func__, ##arg)

/*
 * actually, not all huawei earphone support remote wakeup with CC
 * so, this function's name should be like that "cc_remote_wakeup_support"
 */
static void mark_huawei_earphone_device(bool huawei_earphone)
{
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();
	unsigned long flags;

	dbg("+\n");
	if (proxy == NULL) {
		WARN_ON(1);
		return;
	}

	mutex_lock(&proxy->msg_lock);
	spin_lock_irqsave(&proxy->lock, flags);

	switch (proxy->hibernation_policy) {
	case HIFI_USB_HIBERNATION_ALLOW:
		proxy->hibernation_support = huawei_earphone ? 1U : 0U;
		break;
	case HIFI_USB_HIBERNATION_FORBID:
		proxy->hibernation_support = 0;
		break;
	default:
		break;
	}

	info("huawei_earphone %d, policy %u, hibernation_support %u\n",
			huawei_earphone, proxy->hibernation_policy,
			proxy->hibernation_support);

	spin_unlock_irqrestore(&proxy->lock, flags);
	mutex_unlock(&proxy->msg_lock);

	dbg("-\n");
}

/*
 * The device connected can do remote wakeup with the CC of Type-C. Notify HCD
 * enable the corresponding policy.
 */
void check_customized_earphone_device(struct usb_device *dev)
{
	unsigned char feature[MAX_EARPHONE_FEATURE_LEN] = {0};
	__u16 len = 1;
	int timeout = 3000; /* 3000ms wait for message to complete */
	int ret;

	if ((dev == NULL) || !usb_using_hifi_usb(dev))
		return;

	if ((dev->descriptor.idVendor == HUAWEI_EARPHONE_VENDOR_ID)
			&& (dev->descriptor.idProduct == HUAWEI_EARPHONE_PRODUCT_ID)) {
		ret = usb_control_msg(dev, usb_rcvctrlpipe(dev, 0),
				USB_REQ_VENDOR_FEATURE,
				USB_DIR_IN | USB_TYPE_VENDOR | USB_RECIP_DEVICE,
				0, 0, feature, len, timeout);
		if (ret > 0) {
			info("vendor feature 0x%02x\n", feature[0]);
			if (feature[0] & 0x1)
				mark_huawei_earphone_device(true);
			else
				mark_huawei_earphone_device(false);
		} else {
			info("no vendor feature\n");
			mark_huawei_earphone_device(false);
		}
		/* checkout 384k audio adapter, and then disable hibernation */
		if (is_customized_384k_audio_adapter(dev)) {
			/* check 3.5mm headset disconnected */
			if (((dev->actconfig != NULL) && (dev->actconfig->desc.bNumInterfaces == 1)) ||
				((dev->config != NULL) && (dev->config[0].desc.bNumInterfaces == 1))) {
				info("384k audio adapter without 3.5mm headset\n");
				mark_huawei_earphone_device(false);
			}
		}
	}
}
EXPORT_SYMBOL(check_customized_earphone_device);

enum hibernation_policy phcd_get_hibernation_policy(struct proxy_hcd_client *client)
{
	struct proxy_hcd *phcd = client_to_phcd(client);
	struct platform_device *pdev = phcd->pdev;
	const char *hibernation_policy = NULL;
	int err;

	err = device_property_read_string(&pdev->dev, "hibernation-policy", &hibernation_policy);
	if (err < 0)
		return HIFI_USB_HIBERNATION_ALLOW;

	if (strncmp(hibernation_policy, "forbid", sizeof("forbid") - 1) == 0) {
		info("forbid hibernation\n");
		return HIFI_USB_HIBERNATION_FORBID;
	} else {
		return HIFI_USB_HIBERNATION_ALLOW;
	}
}

void hifi_usb_hibernation_ctrl(enum hibernation_ctrl_type type, bool set)
{
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();
	unsigned int old_hibernation_ctrl;
	unsigned int hibernation_ctrl_mask;
	int ret;

	dbg("+\n");
	if (!proxy)
		return;

	old_hibernation_ctrl = proxy->hibernation_ctrl;
	hibernation_ctrl_mask = (1 << USB_CTRL) | (1 << FB_CTRL | 1 << AUDIO_CTRL);

	if (set)
		proxy->hibernation_ctrl |= (1U << (unsigned int)type);
	else
		proxy->hibernation_ctrl &= ~(1U << (unsigned int)type);

	info("hibernation control: USB %s, FB %s, AUDIO %s\n",
	    (proxy->hibernation_ctrl & (1 << USB_CTRL)) ? "allow" : "forbid",
	    (proxy->hibernation_ctrl & (1 << FB_CTRL)) ? "allow" : "forbid",
	    (proxy->hibernation_ctrl & (1 << AUDIO_CTRL)) ? "allow" : "forbid");

	if ((proxy->hibernation_support != 0)
			&& (proxy->hibernation_ctrl != old_hibernation_ctrl)) {
		if (proxy->hibernation_ctrl == hibernation_ctrl_mask) {
			dbg("Do hibernate\n");
			ret = chip_usb_otg_event(HIFI_USB_HIBERNATE);
			if (ret)
				err("Enqueue hibernate event failed ret %d\n", ret);
		} else if (old_hibernation_ctrl == hibernation_ctrl_mask) {
			dbg("Do revive\n");

			if (type == FB_CTRL) {
				mutex_unlock(&proxy->msg_lock);
				(void)check_hifi_usb_status(HIFI_USB_FB);
				mutex_lock(&proxy->msg_lock);
			} else {
				(void)check_hifi_usb_status(HIFI_USB_PROXY);
			}
		}
	}
	dbg("-\n");
}

#ifdef CONFIG_FB
static int hifi_usb_fb_notifier_callback(struct notifier_block *self,
			unsigned long event, void *data)
{
	int *blank = NULL;
	struct fb_event *evdata = data;
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();

	dbg("+\n");
	if ((evdata == NULL) || (evdata->data == NULL))
		return 0;

	if (proxy == NULL) {
		WARN_ON(1);
		return 0;
	}
	mutex_lock(&proxy->msg_lock);

	blank = evdata->data;

	dbg("event 0x%lx, blank 0x%x\n", event, *blank);

	if (event == FB_EARLY_EVENT_BLANK && *blank == FB_BLANK_UNBLANK) {
		info("display on, fb forbid hifiusb hibernate\n");
		hifi_usb_hibernation_ctrl(FB_CTRL, false);
	} else if (event == FB_EVENT_BLANK && *blank == FB_BLANK_POWERDOWN) {
		info("display off, fb allow hifiusb hibernate\n");
		hifi_usb_hibernation_ctrl(FB_CTRL, true);
	}

	mutex_unlock(&proxy->msg_lock);

	dbg("-\n");
	return 0;
}
#endif

static void hifi_usb_hibernate_work(struct work_struct *work)
{
	struct hifi_usb_proxy *proxy = container_of(work,
			struct hifi_usb_proxy, delayed_work.work);

	mutex_lock(&proxy->msg_lock);
	hifi_usb_hibernation_ctrl(AUDIO_CTRL, true);
	mutex_unlock(&proxy->msg_lock);
}

static void hifi_usb_complete_wakeup(struct chip_usb_event *unused)
{
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();

	if (proxy == NULL) {
		WARN_ON(1);
		return;
	}

	info("wakeup complete\n");
	complete(&proxy->wakeup_completion);
}

static int hifi_usb_queue_wakeup(struct hifi_usb_proxy *proxy)
{
	struct chip_usb_event event = {0};
	unsigned long left;
	int ret;

	reinit_completion(&proxy->wakeup_completion);

	event.type = HIFI_USB_WAKEUP;
	event.flags = EVENT_CB_AT_COMPLETE;
	event.callback = hifi_usb_complete_wakeup;
	ret = chip_usb_queue_event(&event);
	if (ret)
		return ret;

	left = wait_for_completion_timeout(&proxy->wakeup_completion,
					   HIFI_USB_WAKEUP_TIMEOUT);
	if (!left) {
		err("wait for wakeup timeout\n");
		ret = -ETIMEDOUT;
	}

	return ret;
}

int check_hifi_usb_status(enum hifi_usb_status_trigger trigger)
{
	int ret = 0;
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();
	static const char * const trigger_name[] = {
		"0",
		"AUDIO",
		"TCPC",
		"FB",
		"PROXY",
		"URB_ENQUEUE",
		NULL
	};

	dbg("+\n");

	if (proxy == NULL) {
		WARN_ON(1);
		return -ENODEV;
	}

	__pm_wakeup_event(&proxy->hifi_usb_wake_lock, HIFI_USB_WAKE_LOCK_TIME);

	if (proxy->hibernation_support == 0)
		return ret;

	info("trigger %d(%s)\n", trigger, trigger_name[trigger]);

	if ((trigger == HIFI_USB_AUDIO) || (trigger == HIFI_USB_FB)) {
		mutex_lock(&proxy->status_check_lock);
		ret = hifi_usb_queue_wakeup(proxy);
		mutex_unlock(&proxy->status_check_lock);

		if (trigger == HIFI_USB_AUDIO) {
			mutex_lock(&proxy->msg_lock);
			proxy->hibernation_ctrl &= ~(1 << AUDIO_CTRL);
			mutex_unlock(&proxy->msg_lock);
			queue_delayed_work(system_power_efficient_wq, &proxy->delayed_work,
				msecs_to_jiffies(HIFI_USB_HIBERNATE_DELAY));
		}
	} else if ((trigger == HIFI_USB_TCPC) || (trigger == HIFI_USB_PROXY)) {
		ret = chip_usb_otg_event(HIFI_USB_WAKEUP);
	} else if (trigger == HIFI_USB_URB_ENQUEUE) {
		ret = chip_usb_otg_event(HIFI_USB_WAKEUP);
	}
	dbg("-\n");
	return ret;
}
EXPORT_SYMBOL(check_hifi_usb_status);

/* Core functions for hibernation. */
static int hifi_usb_hw_revive(struct hifi_usb_proxy *hifi_usb)
{
#ifdef CONFIG_HIFI_USB_HAS_H2X
	int ret;
#endif

	dbg("+\n");
	hifi_usb_phy_ldo_auto(hifi_usb);

#ifdef CONFIG_HIFI_USB_HAS_H2X
	if (!hifi_usb->usb_not_using_h2x) {
		ret = usb_h2x_on();
		if (ret) {
			err("usb_h2x_on failed\n");
			return ret;
		}
	}
#endif

	dbg("-\n");
	return 0;
}

static int hifi_usb_hw_hibernate(struct hifi_usb_proxy *hifi_usb)
{
#ifdef CONFIG_HIFI_USB_HAS_H2X
	int ret;
#endif

	dbg("+\n");
#ifdef CONFIG_HIFI_USB_HAS_H2X
	if (!hifi_usb->usb_not_using_h2x) {
		ret = usb_h2x_off();
		if (ret) {
			err("usb_h2x_off failed\n");
			return ret;
		}
	}
#endif

	hifi_usb_phy_ldo_on(hifi_usb);

	dbg("-\n");
	return 0;
}

static void hifi_usb_clear_port_reset_change(struct hifi_usb_proxy *proxy)
{
	struct usb_ctrlrequest cmd = {0};
	u32 status = 0;

	dbg("+\n");
	cmd.bRequestType = (__u8)((ClearPortFeature >> BITS_PER_BYTE) & 0xff);
	cmd.bRequest = (__u8)(ClearPortFeature & 0xff);
	cmd.wValue = USB_PORT_FEAT_C_CONNECTION;
	cmd.wIndex = 1;
	cmd.wLength = 0;

	(void)hifi_usb_proxy_hub_control_unlocked(proxy->client, &cmd, (char *)&status);
	dbg("-\n");
}

static void hifi_usb_get_port_status(struct hifi_usb_proxy *proxy, __u32 *status)
{
	struct usb_ctrlrequest cmd = {0};

	dbg("+\n");
	cmd.bRequestType = (__u8)((GetPortStatus >> BITS_PER_BYTE) & 0xff);
	cmd.bRequest = (__u8)(GetPortStatus & 0xff);
	cmd.wValue = 0;
	cmd.wIndex = 1;
	cmd.wLength = 4; /* set cmd length: 4 */

	(void)hifi_usb_proxy_hub_control_unlocked(proxy->client, &cmd, (char *)status);
	info("port status: 0x%x\n", *status);
	dbg("-\n");
}

static bool ignore_and_clear_port_status_change_once(
					struct hifi_usb_proxy *proxy)
{
	int count = 100; /* maximum timeout times */
	__u32 status = 0;

	dbg("+\n");

	while ((proxy->ignore_port_status_change_once == 1)  && (count > 0)) {
		msleep(10); /* 10ms delay */
		count--;
	}

	if (count == 0) {
		err("wait for fist port status change timeout, maybe the device has been disconnected\n");
		proxy->ignore_port_status_change_once = 0;
		hifi_usb_get_port_status(proxy, &status);
		return false;
	}

	udelay(100); /* 100us delay */
	hifi_usb_get_port_status(proxy, &status);
	udelay(100); /* 100us delay */
	hifi_usb_clear_port_reset_change(proxy);
	udelay(100); /* 100us delay */
	hifi_usb_get_port_status(proxy, &status);
	msleep(20); /* 20ms delay */

	dbg("-\n");
	return true;
}

static int reinitialize_hifi_usb_context(struct hifi_usb_proxy *proxy)
{
	struct usb_device *udev = hifi_usb_to_udev(proxy);
	int slot_id = 0;
	int ret;

	dbg("+\n");

	ret = hifi_usb_proxy_alloc_dev_unlocked(proxy, &slot_id);
	if (ret == 0) {
		err("hifi_usb_proxy_alloc_dev_unlocked failed\n");
		return -EPIPE;
	}

	if (udev && (udev->slot_id != slot_id)) {
		err("NEW slot_id %d\n", slot_id);
		udev->slot_id = slot_id;
	}

	dbg("-\n");
	return 0;
}

static int hifi_usb_reset_device(struct hifi_usb_proxy *proxy)
{
	struct usb_device *udev = hifi_usb_to_udev(proxy);
	int ret;

	dbg("+\n");
	if (udev == NULL) {
		WARN_ON(1);
		return -ENODEV;
	}

	ret = usb_lock_device_for_reset(udev, NULL);
	if (!ret) {
		info("to usb_reset_device\n");
		ret = usb_reset_device(udev);
		if (ret)
			err("usb_reset_device ret %d\n", ret);
		usb_unlock_device(udev);
	}

	dbg("-\n");
	return ret;
}

static void hifi_usb_revive_set_proxy(struct hifi_usb_proxy *proxy, unsigned long *flags)
{
	if (proxy == NULL)
		return;
	proxy->ignore_port_status_change_once = 1;
	proxy->hibernation_ctrl &= ~(1 << USB_CTRL);

	spin_lock_irqsave(&proxy->lock, *flags);
	proxy->hibernation_state = 0;
	proxy->hifiusb_suspended = false;
	proxy->hifiusb_hibernating = false;
	spin_unlock_irqrestore(&proxy->lock, *flags);
}

static void hifi_usb_set_revive_time(struct hifi_usb_proxy *proxy, unsigned long jiffies_stamp)
{
	int ret;

	ret = hifi_usb_reset_device(proxy);
	if (ret) {
		proxy->revive_time = HIFI_USB_INVALID_REVIVE_VALUE;
	} else {
		proxy->revive_time = jiffies_to_msecs(jiffies - jiffies_stamp);
		if (proxy->max_revive_time < proxy->revive_time)
			proxy->max_revive_time = proxy->revive_time;
	}
	info("revive_time %ums\n", proxy->revive_time);
}

int hifi_usb_revive(void)
{
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();
	unsigned long flags;
	unsigned long jiffies_stamp;
	int ret;

	info("+\n");

	if (proxy == NULL)
		return -ENODEV;

	mutex_lock(&proxy->msg_lock);
	jiffies_stamp = jiffies;

	if (proxy->hibernation_state == 0) {
		err("not hibernated\n");
		mutex_unlock(&proxy->msg_lock);
		return 0;
	}

	ret = hifi_usb_hw_revive(proxy);
	if (ret)
		goto err_hw_revive;

	msleep(50); /* 50ms delay */

	/*
	 * The first "port status change" message should be ignored!
	 * Make a mark before hifi usb run.
	 */
	hifi_usb_revive_set_proxy(proxy, &flags);

	ret = hifi_usb_runstop_and_wait(proxy, true);
	if (ret) {
		err("hifi_usb_runstop_and_wait failed, ret %d\n", ret);
		goto err_runstop;
	}

	if (!ignore_and_clear_port_status_change_once(proxy)) {
		mutex_unlock(&proxy->msg_lock);

		/*
		 * device maybe disconnected, notify usb core to monitor
		 * port status
		 */
		proxy_port_status_change(proxy->client, 1 << 1);
		return 0;
	}

	if (reinitialize_hifi_usb_context(proxy))
		goto err_context;

	hifi_usb_set_qos(proxy, true);
	mutex_unlock(&proxy->msg_lock);

	hifi_usb_set_revive_time(proxy, jiffies_stamp);

#ifdef DEBUG
	if (!list_empty(&proxy->complete_urb_list))
		WARN_ON(1);
#endif

	info("-\n");
	return 0;

err_context:
	(void)hifi_usb_runstop_and_wait(proxy, false);
err_runstop:
	proxy->ignore_port_status_change_once = 0;
	(void)hifi_usb_hw_hibernate(proxy);
err_hw_revive:
	spin_lock_irqsave(&proxy->lock, flags);
	proxy->hibernation_state = 1;
	spin_unlock_irqrestore(&proxy->lock, flags);
	mutex_unlock(&proxy->msg_lock);

	/* try to recovery hifi usb */
	usb_stop_hifi_usb();

	return ret;
}
EXPORT_SYMBOL(hifi_usb_revive);

static void hifi_usb_proxy_hibernate_proc(struct hifi_usb_proxy *proxy)
{
	proxy->hifiusb_hibernating = true;
	proxy->hibernation_count++;
	proxy->hibernation_state = 1;
	proxy->revive_time = 0;
}

static void hifi_usb_hibernate_err_proc(struct hifi_usb_proxy *proxy)
{
	unsigned long flags;

	spin_lock_irqsave(&proxy->lock, flags);
	proxy->hifiusb_hibernating = false;
	proxy->hibernation_state = 0;
	spin_unlock_irqrestore(&proxy->lock, flags);
}

int hifi_usb_hibernate(void)
{
	struct hifi_usb_proxy *proxy = get_hifi_usb_proxy_handle();
	unsigned long flags;
	int ret;

	info("+\n");

	if (proxy == NULL) {
		WARN_ON(1);
		return -ENODEV;
	}

	mutex_lock(&proxy->msg_lock);

	if (proxy->hibernation_state == 1) {
		err("already hibernated\n");
		mutex_unlock(&proxy->msg_lock);
		return 0;
	}

	/* set hibernateion_state in spinlock, synchronous with enqueue urb */
	spin_lock_irqsave(&proxy->lock, flags);
	if (!proxy->hifiusb_suspended) {
		spin_unlock_irqrestore(&proxy->lock, flags);
		mutex_unlock(&proxy->msg_lock);
		return -EBUSY;
	}

	hifi_usb_proxy_hibernate_proc(proxy);

	spin_unlock_irqrestore(&proxy->lock, flags);

	proxy->port_status = phcd_current_port_status(proxy->client->phcd);
	if (!(proxy->port_status & USB_PORT_STAT_CONNECTION) || !(proxy->port_status & USB_PORT_STAT_ENABLE)) {
		err("port status changed\n");
		goto hibernate_proc_err;
	}

	ret = hifi_usb_runstop_and_wait(proxy, false);
	if (ret) {
		err("hifi_usb_runstop_and_wait failed\n");
		goto hibernate_proc_err;
	}

	msleep(50); /* 50ms delay */

	if (hifi_usb_hw_hibernate(proxy))
		goto hibernate_proc_err;

	phcd_mark_all_endpoint_dropped(proxy->client->phcd);

	hifi_usb_set_qos(proxy, false);

	spin_lock_irqsave(&proxy->lock, flags);
	proxy->hifiusb_hibernating = false;
	spin_unlock_irqrestore(&proxy->lock, flags);

	mutex_unlock(&proxy->msg_lock);
	info("-\n");

	return 0;

hibernate_proc_err:
	hifi_usb_hibernate_err_proc(proxy);

	mutex_unlock(&proxy->msg_lock);
	/* try to recovery hifi usb */
	usb_stop_hifi_usb();

	return -EFAULT;
}
EXPORT_SYMBOL(hifi_usb_hibernate);

void hifi_usb_hibernation_init(struct hifi_usb_proxy *hifiusb)
{
#ifdef CONFIG_FB
	int ret;

	hifiusb->fb_notify.notifier_call = hifi_usb_fb_notifier_callback;
	ret = fb_register_client(&hifiusb->fb_notify);
	if (ret)
		err("register hifi_usb_fb_notifier_callback fail\n");
#endif

	hifiusb->hibernation_policy = phcd_get_hibernation_policy(hifiusb->client);
	hifiusb->hibernation_ctrl = 0;
	INIT_DELAYED_WORK(&hifiusb->delayed_work, hifi_usb_hibernate_work);

	hifi_usb_phy_ldo_init(hifiusb);

	mutex_init(&hifiusb->status_check_lock);
	init_completion(&hifiusb->wakeup_completion);
}

void hifi_usb_hibernation_exit(struct hifi_usb_proxy *hifiusb)
{
#ifdef CONFIG_FB
	int ret;

	ret = fb_unregister_client(&hifiusb->fb_notify);
	if (ret)
		err("unregister fb_notify fail\n");
#endif
}
