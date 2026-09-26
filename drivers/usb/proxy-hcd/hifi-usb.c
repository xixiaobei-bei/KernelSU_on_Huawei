/*
 * hifi-usb.c
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

#include "hifi-usb.h"

#include <linux/bitops.h>
#include <linux/device.h>
#include <linux/hisi/h2x_interface.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/platform_device.h>
#include <linux/pm_wakeup.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/types.h>
#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <linux/usb/ch9.h>
#include <linux/usb/hcd.h>
#include <linux/workqueue.h>

#include "hifi-usb-debugfs.h"
#include "hifi-usb-internal.h"
#include "hifi-usb-ldo.h"
#include "hifi-usb-stat.h"
#include "hifi-usb-urb-buf.h"
#include "usbaudio-monitor.h"
#include "hifi-usb-hibernate.h"

#ifdef CONFIG_HUAWEI_DSM
#include <dsm_audio/dsm_audio.h>
#endif

#define dbg(format, arg...) pr_debug("[phcd][DBG][%s]" format, __func__, ##arg)
#define info(format, arg...) pr_info("[phcd][INFO][%s]" format, __func__, ##arg)
#define err(format, arg...) pr_err("[phcd][ERR][%s]" format, __func__, ##arg)

#define INVALID_SLOT_ID (-1)

static atomic_t msg_recv_enable = ATOMIC_INIT(0);
static atomic_t msg_recv_in_progress = ATOMIC_INIT(0);
static struct hifi_usb_proxy *hifi_usb;

static int hifi_usb_send_op_msg(const struct hifi_usb_op_msg *op_msg);

struct hifi_usb_proxy *get_hifi_usb_proxy_handle(void)
{
	return hifi_usb;
}

static void enable_isr(void)
{
	atomic_set(&msg_recv_enable, 1);
}

static void disable_isr(void)
{
	atomic_set(&msg_recv_enable, 0);
}

static int isr_enabled(void)
{
	return atomic_read(&msg_recv_enable);
}

static int wait_isr_finish(void)
{
	/* wait for out of irq */
	int count = 50;

	while (--count > 0) {
		if (atomic_read(&msg_recv_in_progress) == 0)
			break;
		msleep(20); /* 20ms sleep */
	}
	if (count <= 0) {
		err("wait for out of irq timeout!!!\n");
		return -ETIME;
	}

	return 0;
}

static void flush_isr_sync(void)
{
	disable_isr();
	if (wait_isr_finish())
		WARN_ON(1);
}

static atomic_t start_hifi_usb_retry_count = ATOMIC_INIT(0);
static unsigned long start_hifi_usb_jiffies;

void hifi_usb_announce_udev(struct usb_device *udev)
{
	struct usb_hcd *hcd = NULL;

	if ((udev == NULL) || (udev->parent == NULL) || (udev->parent->parent != NULL))
		return;

	hcd = bus_to_hcd(udev->bus);

	if (usb_using_hifi_usb(udev))
		pr_info("phcd enum audio device time: %u ms\n",
			jiffies_to_msecs(jiffies - start_hifi_usb_jiffies));
}

static int __start_hifi_usb(bool reset_power)
{
	if (get_never_hifi_usb_value()) {
		dbg("never use hifi usb\n");
		return -EPERM;
	}

	if (atomic_read(&start_hifi_usb_retry_count) > 1) {
		dbg("start hifi usb retry more than 1 time, don't start again\n");
		return -EPERM;
	}

	start_hifi_usb_jiffies = jiffies;

	if (reset_power)
		return chip_usb_otg_event(START_HIFI_USB_RESET_VBUS);
	else
		return chip_usb_otg_event(START_HIFI_USB);
}

int usb_start_hifi_usb(void)
{
	return __start_hifi_usb(false);
}
EXPORT_SYMBOL(usb_start_hifi_usb);

int usb_start_hifi_usb_reset_power(void)
{
	return __start_hifi_usb(true);
}
EXPORT_SYMBOL(usb_start_hifi_usb_reset_power);

void usb_stop_hifi_usb(void)
{
	info("\n");
	if (get_always_hifi_usb_value()) {
		info("always use hifi usb\n");
		return;
	}
	if (chip_usb_otg_event(STOP_HIFI_USB))
		err("STOP_HIFI_USB failed\n");
}
EXPORT_SYMBOL(usb_stop_hifi_usb);

void usb_stop_hifi_usb_reset_power(void)
{
	info("\n");
	if (get_always_hifi_usb_value()) {
		info("always use hifi usb\n");
		return;
	}
	if (chip_usb_otg_event(STOP_HIFI_USB_RESET_VBUS))
		err("STOP_HIFI_USB failed\n");
}
EXPORT_SYMBOL(usb_stop_hifi_usb_reset_power);

bool usb_using_hifi_usb(struct usb_device *udev)
{
	struct usb_hcd *hcd = NULL;

	if (udev == NULL || udev->bus == NULL)
		return false;

	hcd = bus_to_hcd(udev->bus);
	if (!strncmp("proxy-hcd", hcd->driver->description,
		sizeof("proxy-hcd") - 1))
		return true;
	else
		return false;
}
EXPORT_SYMBOL(usb_using_hifi_usb);

void export_usbhid_key_pressed(struct usb_device *udev, bool key_pressed)
{
	dbg("+\n");
	if (udev && usb_using_hifi_usb(udev)) {
		struct hifi_usb_proxy *proxy = hifi_usb;
		struct hifi_usb_op_msg op_msg;
		int ret;

		if (proxy == NULL)
			return;

		if (proxy->hid_key_pressed == key_pressed) {
			info("repeat key %s\n", key_pressed ? "press" : "release");
			return;
		}

		proxy->hid_key_pressed = key_pressed;
		info("key_pressed %d\n", key_pressed);

		/* send key_pressed message to hifi */
		op_msg.msg_id = ID_AP_HIFI_USB_HCD_MESG;
		op_msg.reserved = 0;
		op_msg.msg_type = AP_HIFI_USB_HID_KEY_PRESSED;
		op_msg.data_len = 1;
		op_msg.data[0] = key_pressed ? 1U : 0U;

		ret = hifi_usb_send_op_msg(&op_msg);
		if (ret)
			err("hifi_usb_send_op_msg failed ret %d\n", ret);
	}
	dbg("-\n");
}
EXPORT_SYMBOL(export_usbhid_key_pressed);

static int hifi_usb_send_op_msg(const struct hifi_usb_op_msg *op_msg)
{
	int retval;
	struct hifi_usb_proxy *proxy = hifi_usb;

	if (proxy == NULL) {
		WARN_ON(1);
		return -1;
	}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (hifi_usb_in_hibernation(proxy)) {
		err("hibernated! dropp the msg!\n");
		return 0;
	}
#endif

	retval = hifi_usb_send_mailbox(op_msg, sizeof(struct hifi_usb_op_msg));
	if (retval != 0)
		err("send mailbox to hifi failed\n");
	return retval;
}

static int hifi_usb_send_op_msg_and_wait(struct hifi_usb_proxy *proxy)
{
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	unsigned long retval;
	int ret;

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (hifi_usb_in_hibernation(proxy)) {
		ret = check_hifi_usb_status(HIFI_USB_PROXY);
		if (ret)
			info("check_hifi_usb_status ret %d\n", ret);
		return -ESHUTDOWN;
	}
#endif

	if (atomic_read(&proxy->hifi_reset_flag)) {
		err("hifi reset, cannot send msg\n");
		return -ESHUTDOWN;
	}

	init_completion(&proxy->msg_completion);

	if (op_msg->data_len > HIFI_USB_MSG_MAX_DATA_LEN)
		op_msg->data_len = HIFI_USB_MSG_MAX_DATA_LEN;

	ret = hifi_usb_send_op_msg(op_msg);
	if (ret)
		return ret;

	HIFI_USB_STAT(stat_send_msg, &proxy->stat);

	retval = wait_for_completion_timeout(&proxy->msg_completion, HIFI_USB_MSG_TIMEOUT);
	if (retval == 0) {
		err("timeout, msg_id %u, msg_type %u, data_len %u!\n",
			op_msg->msg_id, op_msg->msg_type, op_msg->data_len);
		HIFI_USB_STAT(stat_wait_msg_timeout, &proxy->stat);
		return -ETIME;
	}

	return op_msg->result;
}

static void fill_dev_ctrl_msg(struct hifi_usb_op_msg *op_msg, __u8 msg_type, int slot_id)
{
	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = msg_type;
	op_msg->dev_ctrl.slot_id = (u16)slot_id;
	op_msg->data_len = sizeof(op_msg->dev_ctrl);
}

static void fill_ep_ctrl_msg(struct hifi_usb_op_msg *op_msg, __u8 msg_type,
		int slot_id, struct usb_endpoint_descriptor *desc)
{
	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = msg_type;

	op_msg->ep_ctrl.slot_id = (u16)slot_id;
	memcpy(&op_msg->ep_ctrl.ep_desc, desc, sizeof(op_msg->ep_ctrl.ep_desc));

	op_msg->data_len = sizeof(op_msg->ep_ctrl);
}

int hifi_usb_proxy_alloc_dev_unlocked(struct hifi_usb_proxy *proxy, int *slot_id)
{
	struct hifi_usb_op_msg *op_msg = NULL;
	int ret;

	if (!proxy || !slot_id)
		return -EINVAL;

	op_msg = &proxy->op_msg;
	fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_ALLOC_DEV, 0);
	ret = hifi_usb_send_op_msg_and_wait(proxy);
	if (ret) {
		/* failed */
		ret = 0;
		*slot_id = 0;
	} else {
		ret = 1;
		*slot_id = op_msg->dev_ctrl.slot_id;
	}

	return ret;
}

static int hifi_usb_proxy_alloc_dev(struct proxy_hcd_client *client, int *slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	int ret;

	info("\n");
	mutex_lock(&proxy->msg_lock);
	ret = hifi_usb_proxy_alloc_dev_unlocked(proxy, slot_id);
	proxy->slot_id = *slot_id;
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

void hifi_usb_proxy_free_dev(struct proxy_hcd_client *client, int slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	struct usb_device *udev = NULL;

	info("\n");
	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		info("hibernated, fake free dev\n");
	} else
#endif
	{
		fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_FREE_DEV, slot_id);
		if (hifi_usb_send_op_msg_and_wait(proxy))
			err("free_dev msg timeout!\n");
	}

	proxy->slot_id = INVALID_SLOT_ID;
	wake_up(&proxy->free_dev_wait_queue);

#ifdef CONFIG_USB_384K_AUDIO_ADAPTER_SUPPORT
	/* huawei 384k usbaudio adapter is quirky device, it may disconnect but was not pulled out.
	 * so, do not confirm usb device connection!
	 */
	udev = client_to_phcd(client)->phcd_udev.udev;
	if (is_customized_384k_audio_adapter(udev)) {
		info("\"%s\" is a quirky device, do not confirm usb device connection\n", udev->product);
		proxy->hibernation_support = 0;
		mutex_unlock(&proxy->msg_lock);
		return;
	}
#endif

	if (!timer_pending(&proxy->confirm_udev_timer))
		mod_timer(&proxy->confirm_udev_timer, jiffies + HIFI_USB_CONFIRM_UDEV_RECONNECT_TIME);

	mutex_unlock(&proxy->msg_lock);
}

int hifi_usb_proxy_enable_device(struct proxy_hcd_client *client, int slot_id, int speed, int ep0_maxpks)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);

	fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_ENABLE_DEV, slot_id);
	op_msg->dev_ctrl.speed = (u16)speed;
	op_msg->dev_ctrl.ep0_mps = (u16)ep0_maxpks;

	ret = hifi_usb_send_op_msg_and_wait(proxy);

	mutex_unlock(&proxy->msg_lock);

	return ret;
}

int hifi_usb_proxy_reset_device(struct proxy_hcd_client *client, int slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		info("hibernated, fake reset device\n");
		ret = -ESHUTDOWN;
	} else
#endif
	{
		fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_RESET_DEV, slot_id);
		ret = hifi_usb_send_op_msg_and_wait(proxy);
	}
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

int hifi_usb_proxy_address_device(struct proxy_hcd_client *client, int slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
	fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_ADDRESS_DEV, slot_id);
	ret = hifi_usb_send_op_msg_and_wait(proxy);
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

int hifi_usb_proxy_update_device(struct proxy_hcd_client *client, int slot_id, struct usb_ext_cap_descriptor *ext_cap)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	info("\n");
	mutex_lock(&proxy->msg_lock);
	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = AP_HIFI_USB_UPDATE_DEV;
	op_msg->bos_data.slot_id = (u16)slot_id;
	memcpy(&op_msg->bos_data.ext_cap, (char *)ext_cap, sizeof(struct usb_ext_cap_descriptor));
	op_msg->data_len = sizeof(op_msg->bos_data);
	ret = hifi_usb_send_op_msg_and_wait(proxy);
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

int hifi_usb_proxy_hub_control_unlocked(struct proxy_hcd_client *client,
					struct usb_ctrlrequest *cmd,
					char *buf)
{
	struct hifi_usb_proxy *proxy = NULL;
	struct hifi_usb_op_msg *op_msg = NULL;
	int ret;

	if (!client || !cmd || !buf)
		return -EINVAL;

	proxy = client->client_priv;
	op_msg = &proxy->op_msg;
	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = AP_HIFI_USB_HUB_CONTROL;

	op_msg->hub_ctrl.type_req = (cmd->bRequestType << BITS_PER_BYTE) | cmd->bRequest;
	op_msg->hub_ctrl.w_value = cmd->wValue;
	op_msg->hub_ctrl.w_index = cmd->wIndex;
	op_msg->hub_ctrl.w_length = cmd->wLength;

	op_msg->data_len = sizeof(op_msg->hub_ctrl);

	ret = hifi_usb_send_op_msg_and_wait(proxy);
	if (ret)
		ret = -ENODEV;
	else if (op_msg->hub_ctrl.w_length != 0)
		memcpy(buf, op_msg->hub_ctrl.buf, op_msg->hub_ctrl.w_length);

	return ret;
}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
static int hibernation_hub_control(struct hifi_usb_proxy *proxy, struct usb_ctrlrequest *cmd, char *buf)
{
	__u16 type_req = (cmd->bRequestType << BITS_PER_BYTE) | cmd->bRequest;

	if (type_req == GetPortStatus) {
		info("fake port status 0x%x\n", proxy->port_status);
		memcpy(buf, &proxy->port_status, cmd->wLength);
	} else if (type_req == ClearPortFeature) {
		info("fake clear port feature 0x%x\n", cmd->wValue);
	} else if (type_req == SetPortFeature) {
		info("fake set port feature 0x%x\n", cmd->wValue);
		if (cmd->wValue == USB_PORT_FEAT_RESET)
			proxy->port_status = 0;
	} else {
		err("ilegal type_req 0x%x\n", type_req);
	}

	return 0;
}
#endif

int hifi_usb_proxy_hub_control(struct proxy_hcd_client *client, struct usb_ctrlrequest *cmd, char *buf)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	int ret;

	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy)))
		ret = hibernation_hub_control(proxy, cmd, buf);
	else
#endif
		ret = hifi_usb_proxy_hub_control_unlocked(client, cmd, buf);
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

static void *op_msg_to_data(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg, dma_addr_t *dma)
{
	void *data = NULL;
	dma_addr_t data_dma;
	void *p_min = NULL;
	void *p_max = NULL;

	data_dma = ((dma_addr_t)op_msg->urb.urb_addr_hi << URB_HIGH_ADDR_SHIFT) | op_msg->urb.urb_addr_lo;
	data = proxy->urb_bufs.urb_buf[0] + (data_dma - proxy->urb_bufs.urb_buf_dma[0]);

	p_max = proxy->urb_bufs.urb_buf[proxy->urb_bufs.urb_buf_num - 1];
	p_min = proxy->urb_bufs.urb_buf[0];
	if ((data > p_max) || (data < p_min)) {
		WARN_ON(1);
		return NULL;
	}

	*dma = data_dma;
	return data;
}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
static int add_comp_urb_to_list(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg)
{
	struct complete_urb_wrap *comp_urb = NULL;
	dma_addr_t urb_buf_dma;
	void *urb_buf = op_msg_to_data(proxy, op_msg, &urb_buf_dma);

	if (urb_buf == NULL) {
		err("urb_buf error\n");
		return -EINVAL;
	}

	comp_urb = kzalloc(sizeof(*comp_urb), GFP_ATOMIC);
	if (comp_urb == NULL) {
		err("no memory for atomic allocation\n");
		return -ENOMEM;
	}

	memcpy(&comp_urb->op_msg, op_msg, sizeof(*op_msg));
	list_add_tail(&comp_urb->list_node, &proxy->complete_urb_list);

	info("hibernated, add the completed urb to complete_urb_list, urb_buf %pK\n", urb_buf);
	return 0;
}
#endif

static void hifi_usb_set_urb_msg(struct urb_msg *urb_msg, struct hifi_urb_msg *msg, struct hifi_usb_op_msg *op_msg)
{
	urb_msg->urb_magic = 0;
	urb_msg->slot_id = 1;
	urb_msg->pipe = op_msg->urb.pipe;
	urb_msg->status = msg->status;
	urb_msg->transfer_flags = msg->transfer_flags;
	urb_msg->actual_length = msg->actual_length;
	urb_msg->transfer_buffer_length = msg->transfer_buffer_length;
	urb_msg->interval = msg->interval;
}

static int handle_urb_complete(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg)
{
	struct urb_msg urb_msg;
	struct hifi_urb_msg *msg = NULL;
	void *data = NULL;
	dma_addr_t data_dma;
	unsigned long flags;
	int ret;

	dbg("+\n");

	HIFI_USB_STAT(stat_urb_complete_msg, &proxy->stat);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	spin_lock_irqsave(&proxy->lock, flags);
	if (hifi_usb_in_hibernation(proxy)) {
		ret = add_comp_urb_to_list(proxy, op_msg);
		if (ret)
			err("add_comp_urb_to_list failed\n");
		spin_unlock_irqrestore(&proxy->lock, flags);
		return ret;
	}
	spin_unlock_irqrestore(&proxy->lock, flags);
#endif

	data_dma = ((dma_addr_t)op_msg->urb.urb_addr_hi << URB_HIGH_ADDR_SHIFT) | op_msg->urb.urb_addr_lo;
	data = proxy->urb_bufs.urb_buf[0] + (data_dma - proxy->urb_bufs.urb_buf_dma[0]);

	if ((data > proxy->urb_bufs.urb_buf[proxy->urb_bufs.urb_buf_num - 1]) ||
	    (data < proxy->urb_bufs.urb_buf[0])) {
		err("buffer addr error\n");
		return -EINVAL;
	}

	msg = data;

	hifi_usb_set_urb_msg(&urb_msg, msg, op_msg);

	urb_msg.share_buf = data;
	urb_msg.share_buf_dma = data_dma;

	if ((urb_msg.transfer_flags & URB_DIR_MASK) == URB_DIR_IN) {
		if (usb_pipecontrol(urb_msg.pipe))
			urb_msg.buf = msg->ctrldata;
		else if (usb_pipeint(urb_msg.pipe) || usb_pipebulk(urb_msg.pipe))
			urb_msg.buf = msg->buf;
		else
			urb_msg.buf = NULL;
	} else {
		urb_msg.buf = NULL;
	}

	dbg("to proxy_urb_complete\n");
	mutex_unlock(&proxy->msg_lock);
	proxy_urb_complete(proxy->client, &urb_msg);
	mutex_lock(&proxy->msg_lock);
	dbg("proxy_urb_complete done\n");

	free_urb_buf(&proxy->urb_bufs, data);

	dbg("-\n");
	return 0;
}

static int do_isoc_xfer_test(struct proxy_hcd_client *client, struct urb_msg *urb_msg)
{
	struct hifi_usb_test_msg usb_op_msg = {0};
	struct hifi_usb_test_msg *op_msg = &usb_op_msg;

	info("+\n");
	op_msg->msg_id = ID_AP_HIFI_USB_TEST;
	op_msg->reserved = 0;
	op_msg->msg_type = AP_HIFI_USB_TEST_ISOC;

	op_msg->data_len = urb_msg->transfer_buffer_length;
	if (op_msg->data_len > (__u16)sizeof(op_msg->data))
		op_msg->data_len = sizeof(op_msg->data);

	memcpy(op_msg->data, urb_msg->buf, op_msg->data_len);

	if (hifi_usb_send_mailbox((struct hifi_usb_op_msg *)op_msg, sizeof(struct hifi_usb_test_msg)))
		err("send isoc test case failed\n");

	info("-\n");
	return -1;
}

static int prepare_urb_op_msg(struct hifi_usb_proxy *proxy, struct urb_msg *urb_msg, struct hifi_usb_op_msg *op_msg)
{
	struct hifi_urb_msg *msg = NULL;
	void *data = NULL;
	dma_addr_t data_dma;

	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = AP_HIFI_USB_URB_ENQUEUE;
	op_msg->data_len = sizeof(op_msg->urb);

	data = alloc_urb_buf(&proxy->urb_bufs, &data_dma);
	if (data == NULL) {
		dbg("alloc buf failed!\n");
		return -ENOMEM;
	}
	urb_msg->share_buf = data;
	urb_msg->share_buf_dma = data_dma;

	op_msg->urb.urb_addr_lo = lower_32_bits(data_dma);
	op_msg->urb.urb_addr_hi = upper_32_bits(data_dma);
	op_msg->urb.pipe = urb_msg->pipe;
	op_msg->urb.slot_id = (__u16)urb_msg->slot_id;

	msg = data;
	msg->status = urb_msg->status;
	msg->transfer_flags = urb_msg->transfer_flags;
	msg->transfer_buffer_length = urb_msg->transfer_buffer_length;
	msg->actual_length = urb_msg->actual_length;
	msg->interval = urb_msg->interval;

	if (usb_pipecontrol(urb_msg->pipe)) {
		struct usb_ctrlrequest *ctrl = (struct usb_ctrlrequest *)urb_msg->buf;

		if ((ctrl->bRequest == USB_REQ_SET_CONFIGURATION) &&
		    (ctrl->bRequestType == 0)) {
			dbg("del confirm_udev_timer\n");
			del_timer_sync(&proxy->confirm_udev_timer);
		}
		dbg("ctrl: %02x %02x %04x %04x %04x\n", ctrl->bRequestType,
				ctrl->bRequest, ctrl->wValue, ctrl->wIndex, ctrl->wLength);
		memcpy(msg->buf, urb_msg->buf, sizeof(msg->ctrlrequest));

		if ((urb_msg->transfer_flags & URB_DIR_MASK) == URB_DIR_OUT)
			memcpy(msg->ctrldata, urb_msg->buf + sizeof(msg->ctrlrequest),
				urb_msg->transfer_buffer_length);
	} else if (usb_pipeint(urb_msg->pipe) || usb_pipebulk(urb_msg->pipe)) {
		if ((urb_msg->transfer_flags & URB_DIR_MASK) == URB_DIR_OUT)
			memcpy(msg->buf, urb_msg->buf, urb_msg->transfer_buffer_length);
	} else {
		free_urb_buf(&proxy->urb_bufs, data);
		return -EINVAL;
	}

	return 0;
}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
static int hibernation_urb_enqueue(struct hifi_usb_proxy *proxy, struct urb_msg *urb_msg)
{
	struct hifi_usb_op_msg usb_op_msg = {0};
	struct hifi_usb_op_msg *op_msg = &usb_op_msg;
	int ret;

	if (usb_pipecontrol(urb_msg->pipe)) {
		/* for ep0 in or out xfers */

#ifdef DEBUG
		struct usb_ctrlrequest *ctrl = (struct usb_ctrlrequest *)urb_msg->buf;
		dbg("ctrl urb in hibernation: %02x %02x %04x %04x %04x\n", ctrl->bRequestType, ctrl->bRequest,
			ctrl->wValue, ctrl->wIndex, ctrl->wLength);
#endif

		ret = check_hifi_usb_status(HIFI_USB_URB_ENQUEUE);
		if (!ret)
			ret = -EAGAIN;
		else
			ret = -ESHUTDOWN;
	} else {
		/* for non ep0 xfers */
		struct hifi_urb_msg *msg = NULL;
		dma_addr_t msg_dma;

		ret = prepare_urb_op_msg(proxy, urb_msg, op_msg);
		if (ret) {
			err("prepare_urb_op_msg error ret %d\n", ret);
			return ret;
		}

		msg = (struct hifi_urb_msg *)op_msg_to_data(proxy, op_msg, &msg_dma);
		if (msg != NULL) {
			msg->status = -ESHUTDOWN;
			msg->actual_length = 0;
		}

		ret = add_comp_urb_to_list(proxy, op_msg);
	}

	return ret;
}
#endif

int hifi_usb_proxy_urb_enqueue(struct proxy_hcd_client *client, struct urb_msg *urb_msg)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg usb_op_msg = {0};
	struct hifi_usb_op_msg *op_msg = &usb_op_msg;
	unsigned long flags;
	dma_addr_t tmp_dma;
	int ret;

	dbg("+\n");

	spin_lock_irqsave(&proxy->lock, flags);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	/* special route for urb when hibernated */
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		ret = hibernation_urb_enqueue(proxy, urb_msg);
		goto err;
	}
#endif

	if (usb_pipeisoc(urb_msg->pipe)) {
		dbg("to do_isoc_xfer_test\n");
		ret = do_isoc_xfer_test(client, urb_msg);
		goto err;
	}

	ret = prepare_urb_op_msg(proxy, urb_msg, op_msg);
	if (ret) {
		err("prepare_urb_op_msg failed!\n");
		goto err;
	}

	ret = hifi_usb_send_op_msg(op_msg);
	if (ret) {
		free_urb_buf(&proxy->urb_bufs, op_msg_to_data(proxy, op_msg, &tmp_dma));
		goto err;
	}

	HIFI_USB_STAT(stat_urb_enqueue_msg, &proxy->stat);

	spin_unlock_irqrestore(&proxy->lock, flags);
	dbg("-\n");
	return 0;

err:
	spin_unlock_irqrestore(&proxy->lock, flags);
	return ret;
}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
static int find_comp_urb_and_free(struct proxy_hcd_client *client, struct urb_msg *urb_msg)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct complete_urb_wrap *wrap = NULL;
	struct complete_urb_wrap *tmp = NULL;
	unsigned int found = 0;
	void *data = NULL;
	dma_addr_t data_dma;

	list_for_each_entry_safe(wrap, tmp, &proxy->complete_urb_list, list_node) {
		data = op_msg_to_data(proxy, &wrap->op_msg, &data_dma);
		if ((data != NULL) && (urb_msg->share_buf == data)) {
			info("found urb in complete_urb_list, free it, urb_buf %pK\n", data);
			found = 1;
			list_del_init(&wrap->list_node);
			kfree(wrap);
			free_urb_buf(&proxy->urb_bufs, data);
			break;
		}
	}

	if (found)
		return 0;

	return -ENOENT;
}
#endif

static int hifi_usb_proxy_urb_dequeue(struct proxy_hcd_client *client, struct urb_msg *urb_msg)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg __op_msg = {0};
	struct hifi_usb_op_msg *op_msg = &__op_msg;
	void *data = NULL;
	dma_addr_t data_dma;
	unsigned long flags;
	int ret = 0;

	dbg("+\n");

	if (urb_msg == NULL) {
		dbg("URB not enqueued\n");
		WARN_ON(1);
		return -EINVAL;
	}

	mutex_lock(&proxy->msg_lock);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (proxy->hibernation_support) {
		spin_lock_irqsave(&proxy->lock, flags);
		if (find_comp_urb_and_free(client, urb_msg) == 0) {
			ret = -ENOENT;
		} else if (proxy->hibernation_state) {
			err("hibernated, can't dequeue\n");
			ret = -ESHUTDOWN;
		}
		spin_unlock_irqrestore(&proxy->lock, flags);
		if (ret)
			goto urb_proc_err;
	}
#endif
	op_msg->msg_id = ID_AP_HIFI_USB_HCD_MESG;
	op_msg->reserved = 0;
	op_msg->msg_type = AP_HIFI_USB_URB_DEQUEUE;
	op_msg->data_len = sizeof(op_msg->urb);

	dbg("data_len %u\n", op_msg->data_len);

	data = urb_msg->share_buf;
	data_dma = urb_msg->share_buf_dma;

	op_msg->urb.urb_addr_lo = lower_32_bits(data_dma);
	op_msg->urb.urb_addr_hi = upper_32_bits(data_dma);
	op_msg->urb.pipe = urb_msg->pipe;
	op_msg->urb.slot_id = (__u16)urb_msg->slot_id;

	ret = hifi_usb_send_op_msg(op_msg);
	if (ret)
		goto urb_proc_err;

	HIFI_USB_STAT(stat_urb_dequeue_msg, &proxy->stat);
	mutex_unlock(&proxy->msg_lock);
	dbg("-\n");
	return 0;
urb_proc_err:
	mutex_unlock(&proxy->msg_lock);
	return ret;
}

static int hifi_usb_proxy_add_endpoint(struct proxy_hcd_client *client, int slot_id,
		struct usb_endpoint_descriptor *desc)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
	fill_ep_ctrl_msg(op_msg, AP_HIFI_USB_ADD_ENDPOINT, slot_id, desc);
	ret = hifi_usb_send_op_msg_and_wait(proxy);
	if (ret)
		ret = -ENODEV;
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

static int hifi_usb_proxy_drop_endpoint(struct proxy_hcd_client *client, int slot_id,
		struct usb_endpoint_descriptor *desc)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		info("hibernated, fake drop_endpoint\n");
		ret = -ESHUTDOWN;
	} else
#endif
	{
		fill_ep_ctrl_msg(op_msg, AP_HIFI_USB_DROP_ENDPOINT, slot_id, desc);
		ret = hifi_usb_send_op_msg_and_wait(proxy);
		if (ret)
			ret = -ENODEV;
	}
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

static int hifi_usb_proxy_check_bandwidth(struct proxy_hcd_client *client, int slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		info("hibernated, fake check_bandwidth\n");
		ret = -ESHUTDOWN;
	} else 
#endif
	{
		fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_CHECK_BANDWIDTH, slot_id);
		ret = hifi_usb_send_op_msg_and_wait(proxy);
	}
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

static int hifi_usb_proxy_reset_bandwidth(struct proxy_hcd_client *client, int slot_id)
{
	struct hifi_usb_proxy *proxy = client->client_priv;
	struct hifi_usb_op_msg *op_msg = &proxy->op_msg;
	int ret;

	mutex_lock(&proxy->msg_lock);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (unlikely(hifi_usb_in_hibernation(proxy))) {
		info("hibernated, fake reset_bandwidth\n");
		ret = -ESHUTDOWN;
	} else
#endif
	{
		fill_dev_ctrl_msg(op_msg, AP_HIFI_USB_RESET_BANDWIDTH, slot_id);
		ret = hifi_usb_send_op_msg_and_wait(proxy);
	}
	mutex_unlock(&proxy->msg_lock);

	return ret;
}

static struct proxy_hcd_client_ops hifi_usb_client_ops = {
	.alloc_dev		= hifi_usb_proxy_alloc_dev,
	.free_dev		= hifi_usb_proxy_free_dev,

	.address_device		= hifi_usb_proxy_address_device,
	.enable_device		= hifi_usb_proxy_enable_device,
	.reset_device		= hifi_usb_proxy_reset_device,
	.update_device		= hifi_usb_proxy_update_device,

	.add_endpoint		= hifi_usb_proxy_add_endpoint,
	.drop_endpoint		= hifi_usb_proxy_drop_endpoint,
	.check_bandwidth	= hifi_usb_proxy_check_bandwidth,
	.reset_bandwidth	= hifi_usb_proxy_reset_bandwidth,

	.hub_control		= hifi_usb_proxy_hub_control,

	.urb_enqueue		= hifi_usb_proxy_urb_enqueue,
	.urb_dequeue		= hifi_usb_proxy_urb_dequeue,
};

static void hifi_usb_handle_runstop_mesg(struct hifi_usb_proxy *proxy,
		struct hifi_usb_runstop_msg *runstop_msg)
{
	dbg("+\n");
	proxy->runstop_msg.result = runstop_msg->result;

	if (proxy->runstop_msg.result == 0) {
		if (proxy->runstop_msg.runstop != 0)
			proxy->runstop = 1;
		else
			proxy->runstop = 0;
	}

	complete(&proxy->msg_completion);
	dbg("-\n");
}

static void hifi_usb_handle_init_mesg(struct hifi_usb_proxy *proxy,
		struct hifi_usb_init_msg *init_msg)
{
	if (proxy->runstop == 0) {
		info("hifi reset, but hifi usb not running\n");
		return;
	}
	info("hifi reset route\n");

	atomic_set(&proxy->hifi_reset_flag, 1);
	mutex_unlock(&proxy->msg_lock);
	phcd_giveback_all_urbs(proxy->client);
	usb_stop_hifi_usb_reset_power();
	mutex_lock(&proxy->msg_lock);
}

static int put_msg(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *msg)
{
	struct hifi_usb_msg_wrap *msg_wrap;

	msg_wrap = kzalloc(sizeof(*msg_wrap), GFP_ATOMIC);
	if (msg_wrap == NULL) {
		err("No memory!\n");
		return -ENOMEM;
	}

	memcpy(&msg_wrap->msg, msg, sizeof(msg_wrap->msg));

	list_add_tail(&msg_wrap->node, &proxy->msg_queue);

	return 0;
}

struct hifi_usb_msg_wrap *get_msg(struct hifi_usb_proxy *proxy)
{
	struct hifi_usb_msg_wrap *wrap = NULL;
	unsigned long flags;

	spin_lock_irqsave(&proxy->lock, flags);
	wrap = list_first_entry_or_null(&proxy->msg_queue, struct hifi_usb_msg_wrap, node);

	if (wrap != NULL)
		list_del_init(&wrap->node);
	spin_unlock_irqrestore(&proxy->lock, flags);

	return wrap;
}

static char *get_op_msg_type_name(__u16 msg_type)
{
	if (msg_type == AP_HIFI_USB_HUB_CONTROL)
		return "AP_HIFI_USB_HUB_CONTROL";
	if (msg_type == AP_HIFI_USB_ALLOC_DEV)
		return "AP_HIFI_USB_ALLOC_DEV";
	if (msg_type == AP_HIFI_USB_FREE_DEV)
		return "AP_HIFI_USB_FREE_DEV";
	if (msg_type == AP_HIFI_USB_ENABLE_DEV)
		return "AP_HIFI_USB_ENABLE_DEV";
	if (msg_type == AP_HIFI_USB_RESET_DEV)
		return "AP_HIFI_USB_RESET_DEV";
	if (msg_type == AP_HIFI_USB_ADDRESS_DEV)
		return "AP_HIFI_USB_ADDRESS_DEV";
	if (msg_type == AP_HIFI_USB_UPDATE_DEV)
		return "AP_HIFI_USB_UPDATE_DEV";
	if (msg_type == AP_HIFI_USB_URB_ENQUEUE)
		return "AP_HIFI_USB_URB_ENQUEUE";
	if (msg_type == HIFI_AP_USB_URB_COMPL)
		return "HIFI_AP_USB_URB_COMPL";
	if (msg_type == AP_HIFI_USB_URB_DEQUEUE)
		return "AP_HIFI_USB_URB_DEQUEUE";
	if (msg_type == AP_HIFI_USB_ADD_ENDPOINT)
		return "AP_HIFI_USB_ADD_ENDPOINT";
	if (msg_type == AP_HIFI_USB_DROP_ENDPOINT)
		return "AP_HIFI_USB_DROP_ENDPOINT";
	if (msg_type == AP_HIFI_USB_CHECK_BANDWIDTH)
		return "AP_HIFI_USB_CHECK_BANDWIDTH";
	if (msg_type == AP_HIFI_USB_RESET_BANDWIDTH)
		return "AP_HIFI_USB_RESET_BANDWIDTH";
	if (msg_type == HIFI_USB_HUB_STATUS_CHANGE)
		return "HIFI_USB_HUB_STATUS_CHANGE";
	if (msg_type == HIFI_AP_USB_HCD_DIED)
		return "HIFI_AP_USB_HCD_DIED";

	return "ilegle msg type";
}

static void handle_hub_status_change(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg)
{
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (proxy->hibernation_support) {
		if (proxy->hibernation_state) {
			err("port status change on hibernation\n");
			return;
		}
		if (proxy->ignore_port_status_change_once == 1) {
			proxy->ignore_port_status_change_once = 0;
			info("ignore_port_status_change_once\n");
			return;
		}
	}
#endif

	HIFI_USB_STAT(stat_hub_status_change_msg, &proxy->stat);
	if (!proxy->hifi_usb_wake_lock.active) {
		info("hifi_usb_wake_lock on HUB_STATUS_CHANGE\n");
		__pm_wakeup_event(&proxy->hifi_usb_wake_lock, HIFI_USB_WAKE_LOCK_SHORT_TIME);
	}
	spin_unlock(&proxy->lock);
	proxy_port_status_change(proxy->client, op_msg->port_data.port_bitmap);
	spin_lock(&proxy->lock);
}

static void hifi_usb_handle_drop_endpoint(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg)
{
	if (op_msg->msg_type != proxy->op_msg.msg_type) {
		proxy->op_msg.result = -EFAULT;
		err("msg_type not match, %u != %u\n", op_msg->msg_type, proxy->op_msg.msg_type);
	} else {
		proxy->op_msg.result = op_msg->result;
	}
}

static void hifi_usb_handle_hcd_died(struct hifi_usb_proxy *proxy)
{
	if (proxy->runstop) {
		info("hc died, to usb_stop_hifi_usb\n");
		usb_stop_hifi_usb_reset_power();
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_HIFIUSB_HC_DIED, "USB controller dead\n");
#endif
	}
}

static void hifi_usb_handle_op_mesg(struct hifi_usb_proxy *proxy, struct hifi_usb_op_msg *op_msg)
{
	int compl = 1;

	dbg("hifi IPC mesg type %s %u, result %d\n", get_op_msg_type_name(op_msg->msg_type),
			op_msg->msg_type, op_msg->result);

	HIFI_USB_STAT(stat_receive_msg, &proxy->stat);

	switch (op_msg->msg_type) {
	case AP_HIFI_USB_NOP:
		break;
	case AP_HIFI_USB_HUB_CONTROL:
		proxy->op_msg.result = op_msg->result;
		memcpy(proxy->op_msg.hub_ctrl.buf, op_msg->hub_ctrl.buf, HUB_CTRL_BUF_LEN);
		break;
	case AP_HIFI_USB_ALLOC_DEV:
		proxy->op_msg.dev_ctrl.slot_id = op_msg->dev_ctrl.slot_id;
		/* fallthrough */
	case AP_HIFI_USB_FREE_DEV:
	case AP_HIFI_USB_ENABLE_DEV:
	case AP_HIFI_USB_RESET_DEV:
	case AP_HIFI_USB_ADDRESS_DEV:
	case AP_HIFI_USB_CHECK_BANDWIDTH:
	case AP_HIFI_USB_RESET_BANDWIDTH:
	case AP_HIFI_USB_ADD_ENDPOINT:
	case AP_HIFI_USB_DROP_ENDPOINT:
		hifi_usb_handle_drop_endpoint(proxy, op_msg);
		break;
	case AP_HIFI_USB_URB_ENQUEUE:
	case AP_HIFI_USB_URB_DEQUEUE:
		compl = 0;
		break;
	case HIFI_AP_USB_URB_COMPL:
		put_msg(proxy, op_msg);
		schedule_work(&proxy->msg_work);
		compl = 0;
		break;
	case HIFI_USB_HUB_STATUS_CHANGE:
		compl = 0;
		handle_hub_status_change(proxy, op_msg);
		break;
	case HIFI_AP_USB_HCD_DIED:
		compl = 0;
		put_msg(proxy, op_msg);
		schedule_work(&proxy->msg_work);
		break;
	default:
		break;
	}

	if (compl)
		complete(&proxy->msg_completion);
}

static void hifi_usb_handle_suspend_mesg(struct hifi_usb_proxy *proxy)
{
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (proxy->hibernation_support) {
		hifi_usb_hibernation_ctrl(USB_CTRL, true);
		msleep(50); /* debounce of suspend state 50ms */
	}
#endif
}

static void hifi_usb_handle_running_mesg(struct hifi_usb_proxy *proxy)
{
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (proxy->hibernation_support)
		hifi_usb_hibernation_ctrl(USB_CTRL, false);
#endif
}

/* This function under msg_lock */
static void hifi_usb_msg_handler(struct hifi_usb_proxy *hifiusb, struct hifi_usb_op_msg *__msg)
{
	struct hifi_usb_mesg_header *msg_header = (struct hifi_usb_mesg_header *)__msg;

	switch (msg_header->msg_id) {
	case ID_HIFI_AP_USB_HCD_MESG:
		switch (__msg->msg_type) {
		case HIFI_AP_USB_URB_COMPL:
			handle_urb_complete(hifiusb, __msg);
			break;
		case HIFI_AP_USB_HCD_DIED:
			hifi_usb_handle_hcd_died(hifiusb);
			break;
		default:
			break;
		}
		break;
	case ID_HIFI_AP_USB_SUSPENDED:
		info("HiFi USB SUSPENDED\n");
		hifi_usb_handle_suspend_mesg(hifiusb);
		break;
	case ID_HIFI_AP_USB_RUNNING:
		info("HiFi USB RUNNING\n");
		hifi_usb_handle_running_mesg(hifiusb);
		break;
	case ID_HIFI_AP_USB_INIT:
		hifi_usb_handle_init_mesg(hifiusb, (struct hifi_usb_init_msg *)__msg);
		break;
	default:
		break;
	}
}

static void hifi_usb_msg_work(struct work_struct *work)
{
	struct hifi_usb_proxy *hifiusb = NULL;
	struct hifi_usb_msg_wrap *wrap = NULL;

	dbg("+\n");
	/* hifiusb is safe, because stop_hifi_usb will disable irq and flush this work */
	hifiusb = container_of(work, struct hifi_usb_proxy, msg_work);

	mutex_lock(&hifiusb->msg_lock);
	while (1) {
		wrap = get_msg(hifiusb);
		if (wrap == NULL)
			break;

		hifi_usb_msg_handler(hifiusb, &wrap->msg);
		kfree(wrap);
	}
	mutex_unlock(&hifiusb->msg_lock);
	dbg("-\n");
}

/* This function under proxy->lock */
static void hifi_usb_handle_msg_header(struct hifi_usb_op_msg *__msg,
				struct hifi_usb_proxy *proxy)
{
	struct hifi_usb_mesg_header *msg_header =
			(struct hifi_usb_mesg_header *)__msg;

	switch (msg_header->msg_id) {
	case ID_AP_HIFI_USB_RUNSTOP:
		hifi_usb_handle_runstop_mesg(proxy, (struct hifi_usb_runstop_msg *)__msg);
		break;
	case ID_HIFI_AP_USB_INIT:
		info("HiFi USB just init\n");
		atomic_set(&proxy->hifi_reset_flag, 0);
		break;
	case ID_HIFI_AP_USB_HCD_MESG:
		hifi_usb_handle_op_mesg(proxy, (struct hifi_usb_op_msg *)__msg);
		break;
	case ID_HIFI_AP_USB_WAKEUP:
		info("HiFi USB Wakeup AP\n");
		if (!proxy->hifi_usb_wake_lock.active) {
			info("hifi_usb_wake_lock on ID_HIFI_AP_USB_WAKEUP\n");
			__pm_wakeup_event(&proxy->hifi_usb_wake_lock, HIFI_USB_WAKE_LOCK_SHORT_TIME);
		}

		spin_unlock(&proxy->lock);
		proxy_port_status_change(proxy->client, 0);
		spin_lock(&proxy->lock);
		break;
	case ID_HIFI_AP_USB_RUNNING:
		info("HiFi USB RUNNING\n");
		proxy->hifiusb_suspended = false;
		put_msg(proxy, __msg);
		schedule_work(&proxy->msg_work);
		break;
	case ID_HIFI_AP_USB_SUSPENDED:
		info("HiFi USB SUSPENDED\n");
		proxy->hifiusb_suspended = true;
		put_msg(proxy, __msg);
		schedule_work(&proxy->msg_work);
		break;
	case ID_AP_HIFI_USB_TEST:
		hifi_usb_handle_test_mesg(proxy,
			(struct hifi_usb_test_msg *)__msg);
		break;
	default:
		err("Unsupported mesg_id 0x%x\n", msg_header->msg_id);
		break;
	}
}

/* this is in interrupt, but not in irq */
void hifi_usb_msg_receiver(struct hifi_usb_op_msg *__msg)
{
	struct hifi_usb_proxy *proxy = NULL;
	struct hifi_usb_mesg_header *msg_header = (struct hifi_usb_mesg_header *)__msg;

	if (msg_header == NULL)
		return;

	dbg("+\n");

	if (msg_header->msg_id == ID_AP_USE_HIFI_USB)
		return;

	if (in_irq()) {
		WARN_ON(1);
		err("In irq but not in bh, there is risk of lockup!\n");
		return;
	}

	atomic_inc(&msg_recv_in_progress);

	if (!isr_enabled()) {
		err("isr disabled\n");
		goto out;
	}

	proxy = hifi_usb;
	if (proxy == NULL) {
		err("hifi_usb NULL\n");
		goto out;
	}

	spin_lock(&proxy->lock);
	hifi_usb_handle_msg_header(__msg, proxy);
	spin_unlock(&proxy->lock);
out:
	atomic_dec(&msg_recv_in_progress);
	dbg("-\n");
}

void hifi_usb_hifi_reset_inform(void)
{
	struct hifi_usb_proxy *proxy = hifi_usb;
	unsigned long flags;
	struct hifi_usb_op_msg mesg = {0};

	info("+\n");
	if (proxy == NULL) {
		info("hifi reset, but no hifi usb\n");
		return;
	}

	mesg.msg_id = ID_HIFI_AP_USB_INIT;
	mesg.reserved = 0;
	mesg.msg_type = 0;
	mesg.data_len = 0;

	spin_lock_irqsave(&proxy->lock, flags);

	(void)put_msg(proxy, &mesg);
	schedule_work(&proxy->msg_work);

	spin_unlock_irqrestore(&proxy->lock, flags);
	info("-\n");
}
EXPORT_SYMBOL(hifi_usb_hifi_reset_inform);

/* The fllowing functions were used to support start/stop hifi usb. */
int hifi_usb_runstop_and_wait(struct hifi_usb_proxy *proxy, bool run)
{
	int retval;
	__s32 result;

	dbg("+\n");
	if (!proxy)
		return -ENODEV;

	info("%s\n", run ? "run" : "stop");
	init_completion(&proxy->msg_completion);

	proxy->runstop_msg.mesg_id = ID_AP_HIFI_USB_RUNSTOP;
	proxy->runstop_msg.reserved = 0;
	proxy->runstop_msg.runstop = run;
	proxy->runstop_msg.result = 0;

	result = hifi_usb_send_mailbox((struct hifi_usb_op_msg *)(&proxy->runstop_msg), sizeof(proxy->runstop_msg));
	if (result) {
		err("send mailbox to hifi failed\n");
		return result;
	}

	retval = wait_for_completion_timeout(&proxy->msg_completion, HIFI_USB_MSG_TIMEOUT);
	if (retval == 0) {
		err("wait for response timeout!\n");
		WARN_ON(1);
		return -EBUSY;
	}

	result = proxy->runstop_msg.result;
	if (result)
		err("result %d\n", result);
	dbg("-\n");
	return result;
}

#ifdef CONFIG_HIFI_USB_HAS_H2X
static int hifi_usb_h2x_on(void)
{
	if (hifi_usb == NULL)
		return -EFAULT;

	if (!hifi_usb->usb_not_using_h2x)
		return usb_h2x_on();

	return 0;
}

static int hifi_usb_h2x_off(void)
{
	if (hifi_usb == NULL)
		return -EFAULT;

	if (!hifi_usb->usb_not_using_h2x)
		return usb_h2x_off();

	return 0;
}
#endif
static void hifi_usb_start_default_proxy(struct hifi_usb_proxy *proxy)
{
	urb_buf_reset(&proxy->urb_bufs);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	proxy->hibernation_state = 0;
	proxy->hibernation_support = 0;
	proxy->ignore_port_status_change_once = 0;
	proxy->hibernation_ctrl &= ~(1 << USB_CTRL);
	proxy->hibernation_ctrl |= (1 << AUDIO_CTRL);
	proxy->hibernation_count = 0;
	proxy->revive_time = 0;
	proxy->max_revive_time = 0;
	proxy->hifiusb_hibernating = false;

	if (proxy->hibernation_policy == HIFI_USB_HIBERNATION_FORCE)
		proxy->hibernation_support = 1;
#endif

	proxy->hifiusb_suspended = false;
	proxy->hid_key_pressed = false;
	proxy->slot_id = INVALID_SLOT_ID;
	atomic_set(&proxy->hifi_reset_flag, 0);

}

static int hifi_usb_notify_hifi_run(struct hifi_usb_proxy *proxy)
{
	int ret;

	if (proxy == NULL)
		return -ENODEV;

	ret = hifi_usb_runstop_and_wait(proxy, true);
	if (ret) {
		err("hifi_usb_run failed or timeout\n");
		/* IPC may missed, do stop */
		if (hifi_usb_runstop_and_wait(proxy, false))
			err("hifi_usb_run failed or timeout\n");
		atomic_inc(&start_hifi_usb_retry_count);
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_HIFIUSB_START,
			"start hifi usb failed\n");
#endif
	}

	return ret;
}

int start_hifi_usb(void)
{
	struct hifi_usb_proxy *proxy = NULL;
	struct proxy_hcd_client *client = NULL;
	int ret;

	info("+\n");

	proxy = hifi_usb;
	if (proxy == NULL)
		return -ENODEV;

	/* Here is dangerous, hifi_usb may be set NULL when remove module! */
	mutex_lock(&proxy->msg_lock);

	hifi_usb_start_default_proxy(proxy);

#ifdef DEBUG
	if (!list_empty(&proxy->msg_queue))
		WARN_ON(1);
#endif

	enable_isr();
	client = proxy->client;
	client_ref_start(&client->client_ref, client_ref_release);

#ifdef CONFIG_HIFI_USB_HAS_H2X
	ret = hifi_usb_h2x_on();
	if (ret) {
		err("usb_h2x_on failed\n");
		goto h2x_on_err;
	}
#endif

	hifi_usb_phy_ldo_always_on(hifi_usb);

	if (usbaudio_nv_is_ready() != 0) {
		err("usbaudio nv is not ready\n");
		ret = -EBUSY;
		goto hifi_run_err;
	}

	ret = hifi_usb_notify_hifi_run(proxy);
	if (ret)
		goto hifi_run_err;

	atomic_set(&start_hifi_usb_retry_count, 0);

	mod_timer(&proxy->confirm_udev_timer, jiffies + HIFI_USB_CONFIRM_UDEV_CONNECT_TIME);

	__pm_wakeup_event(&proxy->hifi_usb_wake_lock, HIFI_USB_WAKE_LOCK_LONG_TIME);

	hifi_usb_set_qos(proxy, true);

	info("START_HIFI_USB time %u ms\n", jiffies_to_msecs(jiffies - start_hifi_usb_jiffies));
	start_hifi_usb_jiffies = jiffies;

	mutex_unlock(&proxy->msg_lock);

	info("-\n");

	return 0;
hifi_run_err:
	hifi_usb_phy_ldo_force_auto(hifi_usb);
#ifdef CONFIG_HIFI_USB_HAS_H2X
	if (hifi_usb_h2x_off())
		err("error usb_h2x_on failed\n");
h2x_on_err:
#endif
	client_ref_kill_sync(&client->client_ref);
	disable_isr();

	mutex_unlock(&proxy->msg_lock);

	return ret;
}
EXPORT_SYMBOL(start_hifi_usb);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
static void flush_completed_urb(struct hifi_usb_proxy *proxy)
{
	struct complete_urb_wrap *wrap = NULL;
	unsigned long flags;
	void *data = NULL;
	dma_addr_t data_dma;

	dbg("+\n");

	spin_lock_irqsave(&proxy->lock, flags);
	while (!list_empty(&proxy->complete_urb_list)) {
		wrap = list_first_entry(&proxy->complete_urb_list, struct complete_urb_wrap, list_node);
		list_del_init(&wrap->list_node);

		data = op_msg_to_data(proxy, &wrap->op_msg, &data_dma);
		free_urb_buf(&proxy->urb_bufs, data);
		info("flush urb in complete_urb_list, free urb_buf %pK\n", data);
		kfree(wrap);
	}
	spin_unlock_irqrestore(&proxy->lock, flags);

	dbg("-\n");
}
#endif

void stop_hifi_usb(void)
{
	struct hifi_usb_proxy *proxy = hifi_usb;
	int ret;

	info("+\n");

	if (proxy == NULL) {
		err("No proxy, hifi usb exited\n");
		WARN_ON(1);
		return;
	}

	hifi_usb_wait_for_free_dev(proxy);

	hifi_usb_port_disconnect(proxy); /* must not under msg_lock */

	mutex_lock(&proxy->msg_lock);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	flush_completed_urb(proxy);
#endif
	del_timer_sync(&proxy->confirm_udev_timer);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	if (proxy->hibernation_state) {
		err("hifi usb has been stopped,it don't need to be stopped again\n");
		hifi_usb_phy_ldo_force_auto(hifi_usb);
		goto done;
	}
#endif

	hifi_usb_set_qos(proxy, false);

	/* Notify hifi stop. After this operation complete, should no msg send to AP from hifi. */
	ret = hifi_usb_runstop_and_wait(proxy, false);
	if (ret) {
		err("stop hifi usb failed!!!\n");
#ifdef CONFIG_HUAWEI_DSM
		audio_dsm_report_info(AUDIO_CODEC, DSM_HIFIUSB_STOP,
			"stop hifi usb failed\n");
#endif
	}

#ifdef CONFIG_HIFI_USB_HAS_H2X
	ret = hifi_usb_h2x_off();
	if (ret)
		err("usb_h2x_off failed\n");
#endif
	hifi_usb_phy_ldo_force_auto(hifi_usb);

done:
	flush_isr_sync();

	mutex_unlock(&proxy->msg_lock);
	flush_work(&proxy->msg_work);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	mutex_lock(&proxy->msg_lock);
	proxy->hibernation_ctrl &= ~(1 << USB_CTRL);
	proxy->hibernation_ctrl |= (1 << AUDIO_CTRL);
	proxy->hibernation_support = 0;
	proxy->hibernation_state = 0;
	mutex_unlock(&proxy->msg_lock);
#endif

	info("-\n");
}
EXPORT_SYMBOL(stop_hifi_usb);

int get_hifi_usb_retry_count(void)
{
	return atomic_read(&start_hifi_usb_retry_count);
}
EXPORT_SYMBOL(get_hifi_usb_retry_count);

int get_usbaudio_nv_is_ready(void)
{
	return usbaudio_nv_is_ready();
}
EXPORT_SYMBOL(get_usbaudio_nv_is_ready);

void reset_hifi_usb(void)
{
	atomic_set(&start_hifi_usb_retry_count, 0);
}
EXPORT_SYMBOL(reset_hifi_usb);

static void confirm_udev_timer_fn(unsigned long data)
{
	if (!isr_enabled())
		return;

	info("No usb device connected, do STOP_HIFI_USB\n");
	usb_stop_hifi_usb();
}

static void hifi_usb_dts_config(struct hifi_usb_proxy *hifiusb)
{
	struct proxy_hcd_client *client = hifiusb->client;
	struct proxy_hcd *phcd = client_to_phcd(client);
	struct platform_device *pdev = phcd->pdev;
	struct device *dev = &pdev->dev;
	int	size;
	int ret;
	u32 *p_usb_base_quirk = get_ptr_usb_base_quirk();
	u32 *p_usb_ext_quirk = get_ptr_usb_ext_quirk();

#ifdef CONFIG_HIFI_USB_HAS_H2X
	hifiusb->usb_not_using_h2x = device_property_read_bool(dev, "hifiusb,usb_not_using_h2x");
#endif

	init_data_usb_base_quirk();
	init_data_usb_ext_quirk();

	size = of_property_count_u32_elems(dev->of_node, "base-quirk-devices");
	if (size > 0 && size <= MAX_QUIRK_DEVICES_ONE_GROUP) {
		ret = of_property_read_u32_array(dev->of_node, "base-quirk-devices",
				p_usb_base_quirk, size);
		if (ret < 0) {
			err("read elements form base_quirk_devices failed due to %d\n", ret);
			return;
		}
	}

	size = of_property_count_u32_elems(dev->of_node, "ext-quirk-devices");
	if (size > 0 && size <= MAX_QUIRK_DEVICES_ONE_GROUP) {
		ret = of_property_read_u32_array(dev->of_node, "ext-quirk-devices",
				p_usb_ext_quirk, size);
		if (ret < 0)
			err("read elements form ext_quirk_devices failed due to %d\n", ret);
	}

	hifiusb->qos_set = of_get_chip_usb_reg_cfg(dev->of_node, "qos-set");
	hifiusb->qos_reset = of_get_chip_usb_reg_cfg(dev->of_node, "qos-reset");
}

static int hifi_usb_init(struct proxy_hcd_client *client)
{
	struct hifi_usb_proxy *proxy = NULL;
	int ret;

	dbg("+\n");

	if (hifi_usb != NULL) {
		WARN_ON(1);
		return -EBUSY;
	}

	proxy = kzalloc(sizeof(*proxy), GFP_KERNEL);
	if (proxy == NULL) {
		err("alloc proxy failed, no memory!\n");
		return -ENOMEM;
	}

	ret = urb_buf_init(&proxy->urb_bufs);
	if (ret) {
		err("urb_buf_init failed!\n");
		kfree(proxy);
		return ret;
	}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	INIT_LIST_HEAD(&proxy->complete_urb_list);
#endif
	mutex_init(&proxy->msg_lock);
	spin_lock_init(&proxy->lock);
	wakeup_source_init(&proxy->hifi_usb_wake_lock, "hifi_usb_wake_lock");
	setup_timer(&proxy->confirm_udev_timer, confirm_udev_timer_fn,
		(unsigned long)(uintptr_t)proxy);
	INIT_WORK(&proxy->msg_work, hifi_usb_msg_work);
	INIT_LIST_HEAD(&proxy->msg_queue);

	proxy->client = client;
	client->client_priv = proxy;

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	hifi_usb_hibernation_init(proxy);
#endif
	init_waitqueue_head(&proxy->free_dev_wait_queue);
	hifi_usb_dts_config(proxy);

	if (hifi_usb_debugfs_init(proxy))
		err("hifi_usb_debugfs_init failed\n");

	ret = hifi_usb_mailbox_init();
	if (ret) {
		err("hifi_usb_mailbox_init failed\n");
		hifi_usb_debugfs_exit(proxy);
#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
		hifi_usb_hibernation_exit(proxy);
#endif
		urb_buf_destroy(&proxy->urb_bufs);
		wakeup_source_trash(&proxy->hifi_usb_wake_lock);
		kfree(proxy);
		return ret;
	}

	hifi_usb = proxy;

	dbg("-\n");
	return 0;
}

static void hifi_usb_exit(struct proxy_hcd_client *client)
{
	struct hifi_usb_proxy *proxy = client->client_priv;

	dbg("+\n");
	if (proxy == NULL) {
		WARN_ON(1);
		return;
	}

	if (hifi_usb == NULL) {
		WARN_ON(1);
		return;
	}

	if (proxy != hifi_usb) {
		WARN_ON(1);
		return;
	}

	/* barrier whit start/stop hifi usb */
	flush_isr_sync();
	flush_work(&proxy->msg_work);
	hifi_usb = NULL;
	hifi_usb_mailbox_exit();

	/* remove debugfs node */
	hifi_usb_debugfs_exit(proxy);

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	hifi_usb_hibernation_exit(proxy);
#endif

	urb_buf_destroy(&proxy->urb_bufs);
	wakeup_source_trash(&proxy->hifi_usb_wake_lock);
	kfree(proxy);

	dbg("-\n");
}

int hifi_usb_suspend(struct proxy_hcd_client *client)
{
	struct hifi_usb_proxy *proxy = client->client_priv;

	dbg("+\n");

	if (proxy == NULL) {
		err("proxy NULL!\n");
		return 0;
	}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	mutex_lock(&proxy->msg_lock);
	if (hifi_usb_in_hibernation(proxy))
		info("hibernation_state: hibernated\n");
	mutex_unlock(&proxy->msg_lock);
#endif

	dbg("-\n");

	return 0;
}

void hifi_usb_resume(struct proxy_hcd_client *client)
{
	struct hifi_usb_proxy *proxy = client->client_priv;

	dbg("+\n");
	if (proxy == NULL) {
		err("proxy NULL!\n");
		return;
	}

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
	mutex_lock(&proxy->msg_lock);

	if (hifi_usb_in_hibernation(proxy)) {
		struct usb_device *udev = hifi_usb_to_udev(proxy);

		info("hibernation_state: hibernated\n");
#ifdef DEBUG
		if ((udev != NULL) && udev->reset_resume)
			err("usb %s reset_resume was set\n", dev_name(&udev->dev));
		if ((proxy->port_status & USB_PORT_STAT_ENABLE) == 0)
			err("port was not enabled!\n");
#endif
		if (((proxy->port_status & USB_PORT_STAT_ENABLE) == 0) || (udev && udev->reset_resume)) {
			mutex_unlock(&proxy->msg_lock);
			chip_usb_wakeup_hifi_usb();
			mutex_lock(&proxy->msg_lock);
		}
	}

	mutex_unlock(&proxy->msg_lock);
#endif
	dbg("-\n");
}

static struct proxy_hcd_client hifi_usb_phcd_client = {
	.ops = &hifi_usb_client_ops,
	.client_init = hifi_usb_init,
	.client_exit = hifi_usb_exit,
	.client_suspend = hifi_usb_suspend,
	.client_resume = hifi_usb_resume,

	.name = "hisilicon,proxy-hcd-hifi",
};

static int __init hifi_usb_module_init(void)
{
	return phcd_register_client(&hifi_usb_phcd_client);
}

static void __exit hifi_usb_module_exit(void)
{
	phcd_unregister_client(&hifi_usb_phcd_client);
}

module_init(hifi_usb_module_init);
module_exit(hifi_usb_module_exit);
