/*
 * proxy-hcd.c
 *
 * utilityies for proxy-hcd
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

#include "proxy-hcd.h"

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/slab.h>
#include <asm/unaligned.h>
#include <asm/byteorder.h>
#include <linux/usb.h>
#include <linux/usb/hcd.h>
#include <linux/usb/ch9.h>
#include <linux/usb/ch11.h>

#include "proxy-hcd-stat.h"
#include "client-ref.h"
#include "proxy-hcd-debugfs.h"

#define DBG(format, arg...) pr_debug("[phcd][DBG][%s]" format, __func__, ##arg)
#define INFO(format, arg...) pr_info("[phcd][INFO][%s]" format, __func__, ##arg)
#define ERR(format, arg...) pr_err("[phcd][ERR][%s]" format, __func__, ##arg)

static int phcd_setup(struct usb_hcd *hcd)
{
	DBG("+\n");

	hcd->self.sg_tablesize = 0; /* Not accept scatter-gather lists */
	hcd->self.no_sg_constraint = 0; /* Not support to build packet from discontinuous buffers */
	hcd->self.no_stop_on_short = 1; /* Controllers don't stop the ep queue on short packets */

	hcd->speed = HCD_USB2;
	hcd->self.root_hub->speed = USB_SPEED_HIGH;
	hcd->has_tt = 1; /* Integrated TT in root hub */

	DBG("-\n");
	return 0;
}

static void reset_proxy_hcd_stat(struct proxy_hcd_stat *stat)
{
	stat->stat_alloc_dev = 0;
	stat->stat_free_dev = 0;
	stat->stat_hub_control = 0;
	stat->stat_hub_status_data = 0;
	stat->last_hub_control_time = 0;
	stat->last_hub_status_data_time = 0;
	stat->stat_bus_suspend = 0;
	stat->stat_bus_resume = 0;
}

/*
 * Start the HC after it was halted.
 *
 * This function is called by the USB core when the HC driver is added.
 * Its opposite is phcd_stop().
 */
static int phcd_start(struct usb_hcd *hcd)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;

	DBG("+\n");

	spin_lock_irqsave(&phcd->lock, flags);
	hcd->uses_new_polling = 1; /* support the new root-hub polling mechanism. */
	phcd->hcd = hcd;

	reset_proxy_hcd_stat(&phcd->stat);
	spin_unlock_irqrestore(&phcd->lock, flags);

	DBG("-\n");

	return 0;
}

/*
 * This function is called by the USB core when the HC driver is removed.
 * Its opposite is phcd_start().
 */
static void phcd_stop(struct usb_hcd *hcd)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;

	DBG("+\n");

	spin_lock_irqsave(&phcd->lock, flags);
	phcd->hcd = NULL;
	spin_unlock_irqrestore(&phcd->lock, flags);

	DBG("-\n");
}

static int phcd_get_frame_number(struct usb_hcd *hcd)
{
	INFO("\n");
	return PHCD_FRAME_NUMBER;
}

static unsigned int phcd_get_ep_index(u32 pipe)
{
	unsigned int index;

	if (usb_pipecontrol(pipe))
		index = (unsigned int) (usb_pipeendpoint(pipe) * 2); /* 2 indicates ep directions IN and OUT */
	else
		index = (unsigned int) (usb_pipeendpoint(pipe) * 2) + /* 2 indicates ep directions IN and OUT */
			(usb_pipein(pipe) ? 1 : 0) - 1;
	return index;
}

static int phcd_get_ep_index_from_desc(struct usb_endpoint_descriptor *desc)
{
	int index;

	if (usb_endpoint_xfer_control(desc)) {
		index = (unsigned int) ((desc->bEndpointAddress & 0xf) * 2); /* 2 indicates ep directions IN and OUT */
	} else {
		index = (unsigned int) ((desc->bEndpointAddress & 0xf) * 2) + /* 2 indicates ep directions IN and OUT */
			(usb_endpoint_dir_in(desc) ? 1 : 0) - 1;
	}

	return index;
}

static struct proxy_hcd_urb *find_urb_by_buf(struct list_head *head, const void *buf)
{
	struct proxy_hcd_urb *pos = NULL;

	list_for_each_entry(pos, head, urb_list) {
		if (pos->urb_msg->share_buf == buf)
			return pos;
	}

	return NULL;
}

/* this function should called int lock context */
static struct proxy_hcd_urb *find_phcd_urb(struct proxy_hcd *phcd,
			struct proxy_hcd_ep *phcd_ep, struct urb_msg *urb_msg)
{
	struct proxy_hcd_urb *phcd_urb = NULL;

	if (urb_msg->slot_id != phcd->phcd_udev.udev->slot_id) {
		ERR("slot_id not match!\n");
		return NULL;
	}

	phcd_urb = find_urb_by_buf(&phcd_ep->urb_list, urb_msg->share_buf);
	if (phcd_urb == NULL) {
		ERR("Can't find the urb, may be completed or dequeued already\n");
		return NULL;
	}

	/* double check */
	if (phcd_urb->urb_msg->share_buf_dma != urb_msg->share_buf_dma) {
		ERR("share_buf_dma NOT MATCH\n");
		WARN_ON(1);
		return NULL;
	}

	return phcd_urb;
}

/* Return true means the device is not a usbaudio device. */
static bool non_usbaudio_monitor(struct usb_hcd *hcd, struct urb *urb)
{
	struct usb_device *udev = urb->dev;
	struct usb_ctrlrequest *ctrl = NULL;
	int configuration;

	if (udev->parent == NULL)
		return false;

	if (udev->parent->parent != NULL)
		return false;

	if (!usb_endpoint_xfer_control(&urb->ep->desc))
		return false;

	ctrl = (struct usb_ctrlrequest *)urb->setup_packet;
	if (ctrl == NULL) {
		ERR("req NULL\n");
		return false;
	}

	configuration = le16_to_cpu(ctrl->wValue);
	if (configuration <= 0)
		return false;

	if ((ctrl->bRequest == USB_REQ_SET_CONFIGURATION)
				&& (ctrl->bRequestType == 0)) {
		INFO("to check_non_usbaudio_device, configuration %d\n", configuration);
		if (stop_hifi_usb_when_non_usbaudio(udev, configuration)) {
			DBG("non-usbaudio device using hifiusb\n");
			return true;
		}
	}

	return false;
}

static void phcd_check_urb_result(struct urb *urb, struct urb_msg *urb_msg,
				int status)
{
	/* check urb result */
	if ((urb->actual_length != urb->transfer_buffer_length
				&& (urb->transfer_flags & URB_SHORT_NOT_OK)) ||
			((status != 0) && !usb_endpoint_xfer_isoc(&urb->ep->desc)))
		DBG("Giveback URB: len = %u, expected = %u, status = %d\n",
				urb->actual_length, urb->transfer_buffer_length, status);

	if (urb->actual_length > PHCD_MAX_XFER_LEN) {
		ERR("Too large actual_length!\n");
		urb->actual_length = PHCD_MAX_XFER_LEN;
	}
}

static void phcd_urb_complete_fail_proc(struct proxy_hcd *phcd, struct proxy_hcd_ep *phcd_ep)
{
	if (phcd_ep != NULL)
		urb_complete_fail_add_stat(&phcd_ep->urb_stat);
	else
		urb_complete_pipe_err_add_stat(&phcd->phcd_udev.stat);
}

int phcd_urb_complete(struct proxy_hcd *phcd, struct urb_msg *urb_msg)
{
	struct usb_hcd *hcd = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	struct proxy_hcd_urb *phcd_urb = NULL;
	struct urb *urb = NULL;
	int status;
	int ep_index;
	unsigned long flags;

	DBG("+\n");

	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb_msg->pipe);
	if ((ep_index < 0) || (ep_index >= PROXY_HCD_DEV_MAX_EPS)) {
		ERR("invalid ep_index %d\n", ep_index);
		goto ptr_null_err;
	}

	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	hcd = phcd->hcd;
	if (hcd == NULL) {
		ERR("hcd removed!\n");
		goto ptr_null_err;
	}

	if (phcd->phcd_udev.udev == NULL) {
		ERR("udev disconnected!\n");
		goto ptr_null_err;
	}

	phcd_urb = find_phcd_urb(phcd, phcd_ep, urb_msg);
	if (phcd_urb == NULL)
		goto ptr_null_err;

	urb = phcd_urb->urb;
	urb->actual_length = le32_to_cpu(urb_msg->actual_length);
	status = le32_to_cpu(urb_msg->status);
	phcd_check_urb_result(urb, urb_msg, status);

	/* copy the data */
	if (usb_urb_dir_in(urb) && (urb->actual_length != 0) && urb_msg->buf)
		memcpy(urb->transfer_buffer, urb_msg->buf, urb->actual_length);

	list_del_init(&phcd_urb->urb_list);
	urb->hcpriv = NULL;
	usb_hcd_unlink_urb_from_ep(bus_to_hcd(urb->dev->bus), urb);
	urb_complete_add_stat(&phcd_ep->urb_stat);

	spin_unlock_irqrestore(&phcd->lock, flags);

	usb_hcd_giveback_urb(bus_to_hcd(urb->dev->bus), urb, status);

	kfree(phcd_urb->urb_msg);
	kfree(phcd_urb);

	DBG("-\n");
	return 0;

ptr_null_err:

	phcd_urb_complete_fail_proc(phcd, phcd_ep);

	spin_unlock_irqrestore(&phcd->lock, flags);
	return -1;
}

/* except buf related */
static void fill_phcd_urb_comm(struct urb_msg *urb_msg, struct urb *urb)
{
	urb_msg->urb_magic			= __cpu_to_le64((__u64)(uintptr_t)urb);
	urb_msg->slot_id			= __cpu_to_le32(urb->dev->slot_id);
	urb_msg->pipe				= __cpu_to_le32(urb->pipe);

	urb_msg->status				= __cpu_to_le32(urb->status);
	urb_msg->transfer_flags		= __cpu_to_le32(urb->transfer_flags);
	urb_msg->transfer_buffer_length	= __cpu_to_le32(urb->transfer_buffer_length);
	urb_msg->actual_length			= __cpu_to_le32(urb->actual_length);
	urb_msg->interval				= __cpu_to_le32(urb->interval);
}

/* This function should be protected by lock. */
static int prepare_phcd_urb(struct proxy_hcd_urb *phcd_urb)
{
	struct urb *urb = phcd_urb->urb;
	__u8 *buf = NULL;

	if (usb_endpoint_xfer_control(&urb->ep->desc)) {
		buf = phcd_urb->urb_msg->buf;

		if (urb->setup_packet == NULL) {
			WARN_ON(1);
			return -EINVAL;
		}

		if ((sizeof(struct usb_ctrlrequest) + urb->transfer_buffer_length) > PHCD_MAX_XFER_LEN) {
			WARN_ON(1);
			return -EINVAL;
		}

		fill_phcd_urb_comm(phcd_urb->urb_msg, urb);

		/* copy setup packet */
		memcpy(buf, urb->setup_packet, sizeof(struct usb_ctrlrequest));
		buf += sizeof(struct usb_ctrlrequest);

		/* copy the setup data */
		if (usb_urb_dir_out(urb))
			memcpy(buf, urb->transfer_buffer, urb->transfer_buffer_length);
	} else if (usb_endpoint_xfer_bulk(&urb->ep->desc) || usb_endpoint_xfer_int(&urb->ep->desc)) {
		if (urb->transfer_buffer_length > PHCD_MAX_XFER_LEN) {
			WARN_ON(1);
			return -EINVAL;
		}

		fill_phcd_urb_comm(phcd_urb->urb_msg, urb);

		if (usb_urb_dir_out(urb))
			memcpy(phcd_urb->urb_msg->buf, urb->transfer_buffer,
				urb->transfer_buffer_length);
	} else {
		DBG("isoc xfer\n");
		DBG("len %u, pkts %d\n", urb->transfer_buffer_length,
			urb->number_of_packets);
		/* support only one packet */
		if (urb->number_of_packets != 1) {
			DBG("number_of_packets too large\n");
			return -EPIPE;
		}

		fill_phcd_urb_comm(phcd_urb->urb_msg, urb);

		if (usb_urb_dir_out(urb)) {
			DBG("isoc out xfer\n");
			memcpy(phcd_urb->urb_msg->buf, urb->transfer_buffer,
				urb->transfer_buffer_length);
		} else {
			DBG("isoc in xfer\n");
			memcpy(phcd_urb->urb_msg->buf, urb->transfer_buffer,
				urb->transfer_buffer_length);
		}
	}

	return 0;
}

static int intercept_urb(struct usb_hcd *hcd, struct urb *urb)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;
	int ret;

	spin_lock_irqsave(&phcd->lock, flags);
	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (ret) {
		ERR("usb_hcd_link_urb_to_ep failed, ret %d\n", ret);
		spin_unlock_irqrestore(&phcd->lock, flags);
		return ret;
	}
	usb_hcd_unlink_urb_from_ep(bus_to_hcd(urb->dev->bus), urb);
	spin_unlock_irqrestore(&phcd->lock, flags);

	usb_hcd_giveback_urb(bus_to_hcd(urb->dev->bus), urb, 0);

	return 0;
}

/*
 * return 1 means urb was intercepted. return 0 means not intercepted.
 * return a negative value means error.
 */
static int intercept_some_control_msg(struct usb_hcd *hcd, struct urb *urb)
{
	struct usb_ctrlrequest *ctrl = NULL;
	char *p = NULL;
	int i;
	int ret;

	if (!usb_endpoint_xfer_control(&urb->ep->desc))
		return 0;

	ctrl = (struct usb_ctrlrequest *)urb->setup_packet;
	if (ctrl == NULL) {
		WARN_ON(1);
		return -1;
	}

	if ((ctrl->bRequestType == USB_RECIP_DEVICE) &&
		((ctrl->bRequest == USB_REQ_CLEAR_FEATURE) ||
		(ctrl->bRequest == USB_REQ_SET_FEATURE)) &&
		(le16_to_cpu(ctrl->wValue) == USB_DEVICE_REMOTE_WAKEUP)) {
		DBG("intercept %s remote wakeup\n", ctrl->bRequest
				== USB_REQ_CLEAR_FEATURE ? "clear" : "set");

		urb->actual_length = 0;
		urb->status = 0;

		ret = intercept_urb(hcd, urb);
		if (ret)
			return ret;

		return 1;
	}

	if ((ctrl->bRequestType == (USB_RECIP_DEVICE | USB_DIR_IN)) &&
		(ctrl->bRequest == USB_REQ_GET_STATUS)) {
		p = (char *)urb->transfer_buffer;

		DBG("intercept get device status\n");

		for (i = 0; i < ctrl->wLength; i++)
			p[i] = 0;
		urb->actual_length = ctrl->wLength;
		urb->status = 0;

		ret = intercept_urb(hcd, urb);
		if (ret)
			return ret;

		return 1;
	}

	return 0;
}

static int phcd_urb_enqueue(struct usb_hcd *hcd, struct urb *urb, gfp_t mem_flags)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	struct proxy_hcd_urb *phcd_urb = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	int ep_index, slot_id, ret;
	unsigned long flags;

	DBG("+\n");

	if (!HCD_HW_ACCESSIBLE(hcd)) {
		ERR("hcd not hw accessible!\n");
		return -ESHUTDOWN;
	}

	ret = intercept_some_control_msg(hcd, urb);
	if (ret < 0)
		return ret;
	else if (ret > 0)
		return 0;

	/*
	 * monitor device by configuration descriptor.
	 * if device is not usbaudio, switch USB Host Controller.
	 */
	if (non_usbaudio_monitor(hcd, urb))
		return -ENODEV; /* usbaudio monitor issured, stop enumeration */

	phcd_urb = kzalloc(sizeof(*phcd_urb), mem_flags);
	if (phcd_urb == NULL) {
		ERR("alloc urb_priv failed\n");
		return -ENOMEM;
	}

	phcd_urb->urb_msg = kzalloc(sizeof(*phcd_urb->urb_msg) + PHCD_MAX_XFER_LEN, mem_flags);
	if (phcd_urb->urb_msg == NULL) {
		ERR("alloc urb_msg failed\n");
		kfree(phcd_urb);
		return -ENOMEM;
	}

	usb_get_urb(urb);
	phcd_urb->urb_msg->buf = (__u8 *)(phcd_urb->urb_msg + 1);

	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	DBG("ep %d, urb %pK enqueue\n", ep_index, urb);

	slot_id = urb->dev->slot_id;
	if ((phcd->phcd_udev.udev == NULL) || (slot_id != phcd->phcd_udev.udev->slot_id)) {
		ERR("slot_id not match\n");
		ret = -ENODEV;
		goto free_phcd_urb;
	}

	phcd_urb->urb = urb;
	urb->hcpriv = phcd_urb;

	INIT_LIST_HEAD(&phcd_urb->urb_list);
	list_add_tail(&phcd_urb->urb_list, &phcd_ep->urb_list);

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (unlikely(ret)) {
		ERR("usb_hcd_link_urb_to_ep failed %d\n", ret);
		goto del_phcd_urb;
	}

	/* prepare the phcd_urb for transaction */
	ret = prepare_phcd_urb(phcd_urb);
	if (unlikely(ret))
		goto unlink_urb;

	phcd_urb->flags |= PHCD_URB_ENQUEUEING;
	spin_unlock_irqrestore(&phcd->lock, flags);

	ret = proxy_urb_enqueue(phcd, phcd_urb->urb_msg);
	if (unlikely(ret)) {
		ERR("proxy_urb_enqueue failed %d\n", ret);
		spin_lock_irqsave(&phcd->lock, flags);
		goto unlink_urb;
	}

	spin_lock_irqsave(&phcd->lock, flags);
	if (urb->hcpriv == phcd_urb)
		phcd_urb->flags &= ~PHCD_URB_ENQUEUEING;
	urb_enqueue_add_stat(&phcd_ep->urb_stat);
	spin_unlock_irqrestore(&phcd->lock, flags);

	usb_put_urb(urb);
	DBG("-\n");

	return 0;

unlink_urb:

	/* The urb maybe already dequeued! */
	if ((!urb->hcpriv) || (urb->hcpriv != phcd_urb)) {
		ERR("phcd_urb was freed\n");
		spin_unlock_irqrestore(&phcd->lock, flags);
		return 0;
	}

	phcd_urb->flags &= ~PHCD_URB_ENQUEUEING;
	usb_hcd_unlink_urb_from_ep(hcd, urb);
del_phcd_urb:
	list_del(&phcd_urb->urb_list);
	urb->hcpriv = NULL;
free_phcd_urb:
	urb_enqueue_fail_add_stat(&phcd_ep->urb_stat);
	spin_unlock_irqrestore(&phcd->lock, flags);
	usb_put_urb(urb);
	kfree(phcd_urb->urb_msg);
	kfree(phcd_urb);
	return ret;
}

/* This function under phcd->lock */
static int proxy_urb_dequeue_fail_proc(struct usb_hcd *hcd, struct urb *urb,
					int status)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	struct proxy_hcd_urb *phcd_urb = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	int ep_index;
	int ret = 0;

	/* If dequeue command failed, dequeue the urb right now. */
	ERR("proxy_urb_dequeue failed %d\n", ret);

	phcd_urb = urb->hcpriv;
	if (phcd_urb == NULL) {
		WARN_ON(1);
		ret = -ENOENT;
		return ret;
	}

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	urb_dequeue_giveback_add_stat(&phcd_ep->urb_stat);
	INFO("client not response, giveback urb right now\n");
	usb_hcd_unlink_urb_from_ep(hcd, urb);
	urb->hcpriv = NULL;
	list_del(&phcd_urb->urb_list);

	usb_hcd_giveback_urb(hcd, urb, status);

	kfree(phcd_urb->urb_msg);
	kfree(phcd_urb);

	client_ref_put(&phcd->client->client_ref); /* Banlance with urb enqueue. */

	return ret;
}

static int phcd_urb_dequeue(struct usb_hcd *hcd, struct urb *urb, int status)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	struct proxy_hcd_urb *phcd_urb = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	unsigned long wait_enqueue_complete_count = MAX_ENQUEUE_COUNT;
	unsigned long flags;
	int ep_index;
	int slot_id;
	int ret = 0;

	DBG("+\n");

	if (!HCD_HW_ACCESSIBLE(hcd)) {
		ERR("hcd not hw accessible!\n");
		return -ESHUTDOWN;
	}

wait_enqueue_complete:
	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	slot_id = urb->dev->slot_id;
	if (!phcd->phcd_udev.udev || (slot_id != phcd->phcd_udev.udev->slot_id)) {
		ERR("slot_Id not match\n");
		ret = -ESHUTDOWN;
		goto urb_dequeue_err;
	}

	phcd_urb = urb->hcpriv;
	if (phcd_urb == NULL) {
		WARN_ON(1);
		ret = -ENOENT;
		goto urb_dequeue_err;
	}

	if (phcd_urb->flags & PHCD_URB_ENQUEUEING) {
		if (wait_enqueue_complete_count-- == 0) {
			WARN_ON_ONCE(1);
			ret = -EBUSY;
			goto urb_dequeue_err;
		}

		spin_unlock_irqrestore(&phcd->lock, flags);
		if (in_atomic() || in_interrupt()) {
			cpu_relax();
			mdelay(1);
		} else {
			might_sleep();
			msleep(10);
		}
		goto wait_enqueue_complete;
	}

	if (wait_enqueue_complete_count != MAX_ENQUEUE_COUNT)
		INFO("wait_enqueue_complete_count:%lu\n",
				wait_enqueue_complete_count);

	DBG("ep %d, urb %pK dequeue\n", ep_index, urb);

	/* urb maybe unlinked just before here */
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret) {
		ERR("usb_hcd_check_unlink_urb urb_dequeue_err %d\n", ret);
		goto urb_dequeue_err;
	}

	spin_unlock_irqrestore(&phcd->lock, flags);

	/* issue dequeue command */
	ret = proxy_urb_dequeue(phcd, phcd_urb->urb_msg);
	if (unlikely(ret)) {
		spin_lock_irqsave(&phcd->lock, flags);
		ret = proxy_urb_dequeue_fail_proc(hcd, urb, status);
		if (ret != 0)
			goto urb_dequeue_err;

		spin_unlock_irqrestore(&phcd->lock, flags);
		return 0;
	}

	urb_dequeue_add_stat(&phcd_ep->urb_stat);

	DBG("-\n");
	return 0;
urb_dequeue_err:
	urb_dequeue_fail_add_stat(&phcd_ep->urb_stat);
	spin_unlock_irqrestore(&phcd->lock, flags);
	DBG("- ret %d\n", ret);
	return ret;
}

static int phcd_map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb,
			   gfp_t mem_flags)
{
	DBG("+-\n");
	return 0;
}

static void phcd_unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
	DBG("+-\n");
}

/*
 * Called when clearing halted device. The core should have sent the control
 * message to clear the device halt condition. The host side of the halt should
 * already be cleared with a reset endpoint command issued when the STALL tx
 * event was received.
 */
static void phcd_endpoint_reset(struct usb_hcd *hcd,
		struct usb_host_endpoint *ep)
{
	DBG("ep addr 0x%02x\n", ep->desc.bEndpointAddress);
}

static void phcd_endpoint_disable(struct usb_hcd *hcd,
		struct usb_host_endpoint *ep)
{
	DBG("ep addr 0x%02x\n", ep->desc.bEndpointAddress);
}

static void reset_urb_stat(struct proxy_hcd_urb_stat *stat)
{
	stat->stat_urb_enqueue = 0;
	stat->stat_urb_dequeue = 0;
	stat->stat_urb_enqueue_fail = 0;
	stat->stat_urb_dequeue_fail = 0;
	stat->stat_urb_dequeue_giveback = 0;
	stat->stat_urb_giveback = 0;
	stat->stat_urb_complete = 0;
	stat->stat_urb_complete_fail = 0;
}

static int phcd_init_virt_device(struct proxy_hcd *phcd, struct usb_device *udev)
{
	int i;

	if (phcd->phcd_udev.udev != NULL)
		return -EBUSY;

	phcd->phcd_udev.udev = udev;

	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++) {
		INIT_LIST_HEAD(&phcd->phcd_udev.phcd_eps[i].urb_list);
		reset_urb_stat(&phcd->phcd_udev.phcd_eps[i].urb_stat);
	}

	return 0;
}

/*
 * Returns 0 if the HC ran out of device slots, the Enable Slot command
 * timed out, or allocating memory failed.  Returns 1 on success.
 */
static int phcd_alloc_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;
	int ret;
	int slot_id = 0;

	DBG("+\n");

	might_sleep();
	mutex_lock(&phcd->mutex);

	ret = proxy_alloc_dev(phcd, &slot_id);
	if (ret != 1) {
		mutex_unlock(&phcd->mutex);
		pr_err("%s: proxy_alloc_dev failed, ret %d\n", __func__, ret);
		return 0;
	}

	spin_lock_irqsave(&phcd->lock, flags);
	udev->slot_id = slot_id;
	ret = phcd_init_virt_device(phcd, udev);
	spin_unlock_irqrestore(&phcd->lock, flags);
	if (ret) {
		mutex_unlock(&phcd->mutex);
		pr_err("%s: phcd_init_virt_device failed ret %d\n", __func__, ret);
		return 0;
	}

	alloc_dev_add_stat(&phcd->stat);
	mutex_unlock(&phcd->mutex);

	DBG("-\n");

	return 1;
}

void phcd_giveback_all_urbs(struct proxy_hcd_client *client)
{
	struct proxy_hcd *phcd = client_to_phcd(client);
	struct usb_hcd *hcd = NULL;
	unsigned long flags;
	int i;

	INFO("+\n");

	if (phcd == NULL) {
		ERR("phcd NULL\n");
		return;
	}

	spin_lock_irqsave(&phcd->lock, flags);

	hcd = phcd->hcd;
	if (hcd == NULL)
		goto done;

	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++) {
		struct list_head *list = &phcd->phcd_udev.phcd_eps[i].urb_list;

		while (!list_empty(list)) {
			struct proxy_hcd_urb *phcd_urb = NULL;
			struct urb *urb;

			phcd_urb = list_first_entry(list, struct proxy_hcd_urb, urb_list);
			urb = phcd_urb->urb;

			if (urb->unlinked)
				DBG("ep %d urb %pK unlinked, giveback\n", i, urb);
			else
				DBG("ep %d urb %pK not unlinked, giveback\n", i, urb);

			list_del_init(&phcd_urb->urb_list);
			usb_hcd_unlink_urb_from_ep(hcd, urb);
			urb->hcpriv = NULL;
			urb_giveback_add_stat(&phcd->phcd_udev.phcd_eps[i].urb_stat);

			spin_unlock_irqrestore(&phcd->lock, flags);

			usb_hcd_giveback_urb(hcd, urb, -ESHUTDOWN);
			kfree(phcd_urb->urb_msg);
			kfree(phcd_urb);
			client_ref_put(&phcd->client->client_ref); /* Banlance with urb enqueue. */

			spin_lock_irqsave(&phcd->lock, flags);
		}
	}
done:
	spin_unlock_irqrestore(&phcd->lock, flags);

	INFO("-\n");
}

/*
 * At this point, the struct usb_device is about to go away, the device has
 * disconnected, and all traffic has been stopped and the endpoints have been
 * disabled.  Free any HC data structures associated with that device.
 */
static void phcd_free_dev(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;
	int i;

	DBG("+\n");

	might_sleep();
	mutex_lock(&phcd->mutex);
	free_dev_add_stat(&phcd->stat);
	proxy_free_dev(phcd, udev->slot_id);

	spin_lock_irqsave(&phcd->lock, flags);
	phcd->phcd_udev.udev = NULL;

	/* All urb must have been completed. */
	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++) {
		if (!list_empty(&phcd->phcd_udev.phcd_eps[i].urb_list)) {
			WARN_ON(1);
			DBG("ep%d has remaining urb\n", i);
		}
	}

	spin_unlock_irqrestore(&phcd->lock, flags);
	mutex_unlock(&phcd->mutex);

	DBG("-\n");
}

static int phcd_add_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	struct proxy_hcd *phcd = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	int ep_index;
	int ret;

	DBG("+\n");

	if ((hcd == NULL) || (ep == NULL) || (udev == NULL)) {
		DBG("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		DBG("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_add_endpoint(phcd, udev, ep);
	if (ret < 0) {
		ERR("proxy_add_endpoint failed %d\n", ret);
		return ret;
	}

	ep_index = phcd_get_ep_index_from_desc(&ep->desc);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	phcd_ep->added = 1;

	/* Store the usb_device pointer for later use */
	ep->hcpriv = udev;

	DBG("add ep 0x%x, slot id %d\n", (unsigned int) ep->desc.bEndpointAddress, udev->slot_id);
	DBG("-\n");

	return 0;
}

void phcd_mark_all_endpoint_dropped(struct proxy_hcd *phcd)
{
	int i;

	DBG("+\n");
	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++)
		phcd->phcd_udev.phcd_eps[i].added = 0;
	DBG("-\n");
}

__u32 phcd_current_port_status(struct proxy_hcd *phcd)
{
	unsigned long flags;
	__u32 status;

	spin_lock_irqsave(&phcd->lock, flags);
	status = phcd->port_status[0] & PHCD_PORT_STATUS_MASK;
	spin_unlock_irqrestore(&phcd->lock, flags);

	return status;
}

static int phcd_drop_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	struct proxy_hcd *phcd = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	int ep_index;
	int ret;

	DBG("+\n");
	if ((hcd == NULL) || (ep == NULL) || (udev == NULL)) {
		DBG("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		DBG("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	ep_index = phcd_get_ep_index_from_desc(&ep->desc);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	if (!phcd_ep->added) {
		DBG("ep %d not added\n", ep_index);
		return 0;
	}

	might_sleep();

	ret = proxy_drop_endpoint(phcd, udev, ep);
	if (ret < 0) {
		ERR("proxy_drop_endpoint failed %d\n", ret);
		return ret;
	}
	phcd_ep->added = 0;

	DBG("drop ep 0x%x, slot id %d\n", (unsigned int) ep->desc.bEndpointAddress, udev->slot_id);
	DBG("-\n");

	return 0;
}

static int phcd_check_bandwidth(struct usb_hcd *hcd, struct usb_device *udev)
{
	int ret;
	struct proxy_hcd *phcd = NULL;

	DBG("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		DBG("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		DBG("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_check_bandwidth(phcd, udev->slot_id);
	if (ret < 0) {
		ERR("proxy_check_bandwidth failed %d\n", ret);
		return ret;
	}
	DBG("-\n");

	return ret;
}

static void phcd_reset_bandwidth(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = NULL;
	int ret;

	DBG("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		DBG("invalid args\n");
		return;
	}
	if (udev->parent == NULL) {
		DBG("for root hub\n");
		return;
	}
	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_reset_bandwidth(phcd, udev->slot_id);
	if (ret < 0)
		ERR("proxy_add_endpoint failed %d\n", ret);
	DBG("-\n");
}

static int phcd_address_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	DBG("+\n");
	if (!udev->slot_id)
		return -EINVAL;

	might_sleep();

	ret = proxy_address_device(phcd, udev->slot_id);
	if (ret < 0)
		ERR("proxy_address_device failed %d\n", ret);
	DBG("-\n");
	return ret;
}

static int phcd_enable_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	DBG("+\n");
	if (!udev->slot_id)
		return -EINVAL;

	might_sleep();

	ret = proxy_enable_device(phcd, udev->slot_id, udev->speed,
		udev->ep0.desc.wMaxPacketSize);
	if (ret < 0)
		ERR("proxy_enable_device failed %d\n", ret);
	DBG("-\n");

	return ret;
}

static int phcd_update_hub_device(struct usb_hcd *hcd, struct usb_device *hdev,
			struct usb_tt *tt, gfp_t mem_flags)
{
	DBG("+\n");
	if (hdev->parent == NULL)
		return 0;
	DBG("-\n");
	return -ENODEV;
}

static int phcd_reset_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	int ret;
	struct proxy_hcd *phcd = NULL;

	DBG("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		DBG("invalid args\n");
		return -EINVAL;
	}

	if (udev->parent == NULL) {
		DBG("for root hub\n");
		return 0;
	}

	DBG("udev udev->route 0x%x\n", udev->route);
	DBG("udev udev->portnum 0x%x\n", udev->portnum);

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_reset_device(phcd, udev->slot_id);
	if (ret < 0)
		ERR("proxy_reset_device failed %d\n", ret);
	DBG("-\n");

	return ret;
}

#ifdef CONFIG_PM
static int phcd_update_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = NULL;
	int ret = 0;

	DBG("+\n");
	DBG("lpm_capable %u\n", udev->lpm_capable);

	if ((hcd->speed == HCD_USB3) || !udev->lpm_capable)
		return 0;

	/* we only support lpm for non-hub device connected to root hub yet */
	if ((udev->parent == NULL) || (udev->parent->parent != NULL) ||
			(udev->descriptor.bDeviceClass == USB_CLASS_HUB))
		return 0;

	udev->usb2_hw_lpm_capable = 1; /* enable LPM mode */
	udev->l1_params.timeout = 512; /* LPM default timeout value */
	udev->l1_params.besl = 4; /* indicate the latency resume from L1. */
	udev->usb2_hw_lpm_besl_capable = 1; /* enable latency resume from L1 mode */

	phcd = hcd_to_phcd(hcd);
	if (udev->bos && udev->bos->ext_cap) {
		ret = proxy_update_device(phcd, udev->slot_id, udev->bos->ext_cap);
		if (ret)
			pr_err("proxy_update_device return %d\n", ret);
	}

	DBG("-\n");

	return ret;
}

static int phcd_set_usb2_hardware_lpm(struct usb_hcd *hcd,
			struct usb_device *udev, int enable)
{
	DBG("+\n");
	if (hcd->speed == HCD_USB3 || !udev->lpm_capable)
		return -EPERM;

	if (!udev->parent || udev->parent->parent ||
			udev->descriptor.bDeviceClass == USB_CLASS_HUB)
		return -EPERM;

	if (udev->usb2_hw_lpm_capable != 1)
		return -EPERM;
	DBG("-\n");

	return 0;
}
#else
static int phcd_update_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	return 0;
}

static int phcd_set_usb2_hardware_lpm(struct usb_hcd *hcd,
				struct usb_device *udev, int enable)
{
	return 0;
}
#endif

/* Fill in the USB 2.0 roothub descriptor */
static void phcd_usb2_hub_descriptor(struct proxy_hcd *phcd,
			struct usb_hub_descriptor *desc)
{
	u16 temp;

	desc->bDescLength = USB_DT_HUB_NONVAR_SIZE + 2;	/* 2 is the header length */
	desc->bDescriptorType = USB_DT_HUB;

	desc->bPwrOn2PwrGood = 0; /* xhci section 5.4.9 says 20ms max */
	desc->bHubContrCurrent = 0;

	desc->bNbrPorts = 1;

	temp = 0;
	/* Bits 1:0 - support per-port power switching, or power always on */
	temp |= HUB_CHAR_COMMON_LPSM;
	/* Bit  2 - root hubs are not part of a compound device */
	/* Bits 4:3 - individual port over current protection */
	temp |= HUB_CHAR_INDV_PORT_OCPM;
	/* Bits 6:5 - no TTs in root ports */
	/* Bit  7 - no port indicators */
	desc->wHubCharacteristics = cpu_to_le16(temp);

	desc->u.hs.DeviceRemovable[0] = 0x0;
	desc->u.hs.DeviceRemovable[1] = 0xff;
}

/* Merge local status and new status for a port. */
void phcd_update_port_status(struct proxy_hcd *phcd, __u16 port_index,
			__u32 status)
{
	__u32 old_status, new_status;
	__u32 mask = PHCD_PORT_STATUS_MASK | PHCD_PORT_STATUS_CHANGE_MASK;
	__u16 offset = 16;

	old_status = phcd->port_status[port_index - 1];
	new_status = (old_status & (~mask)) | (status & mask);

	if ((new_status & USB_PORT_STAT_CONNECTION)
			!= (old_status & USB_PORT_STAT_CONNECTION))
		new_status |= (USB_PORT_STAT_C_CONNECTION << offset);

	if ((new_status & USB_PORT_STAT_ENABLE)
			!= (old_status & USB_PORT_STAT_ENABLE))
		new_status |= (USB_PORT_STAT_C_ENABLE << offset);

	if ((new_status & USB_PORT_STAT_RESET)
			!= (old_status & USB_PORT_STAT_RESET))
		new_status |= (USB_PORT_STAT_C_RESET << offset);

	phcd->port_status[port_index - 1] = new_status;
	DBG("old status 0x%x, new status 0x%x, mask 0x%x\n",
		old_status, new_status, mask);

	if ((phcd->port_status[port_index - 1] & PHCD_PORT_STATUS_CHANGE_MASK) == 0) {
		DBG("clear port_bitmap\n");
		phcd->port_bitmap &= ~(1U << port_index);
	}

	DBG("port_status 0x%x\n", phcd->port_status[port_index - 1]);
}

/* Used by hub_control, report the port status. */
static __u32 phcd_get_port_status(struct proxy_hcd *phcd, __u16 wIndex)
{
	u32 status = 0;
	int retval;

	DBG("+\n");
	retval = proxy_hub_control(phcd, GetPortStatus, 0, wIndex,
		(char *)&status, (__u16)sizeof(status));
	if (retval < 0) {
		status = USB_PORT_STAT_POWER;
		return status;
	}
	DBG("-\n");

	return status;
}

/* Used by hub_control, clear change bits in port status. */
static void phcd_clear_port_change_bit(struct proxy_hcd *phcd, __u16 wValue,
		__u16 wIndex)
{
	__u32 port_change_bits = 0;

	switch (wValue) {
	case USB_PORT_FEAT_C_CONNECTION:
		port_change_bits |= USB_PORT_STAT_C_CONNECTION;
		break;
	case USB_PORT_FEAT_C_ENABLE:
		port_change_bits |= USB_PORT_STAT_C_ENABLE;
		break;
	case USB_PORT_FEAT_C_SUSPEND:
		port_change_bits |= USB_PORT_STAT_C_SUSPEND;
		break;
	case USB_PORT_FEAT_C_OVER_CURRENT:
		port_change_bits |= USB_PORT_STAT_C_OVERCURRENT;
		break;
	case USB_PORT_FEAT_C_RESET:
		port_change_bits |= USB_PORT_STAT_C_RESET;
		break;
	case USB_PORT_FEAT_C_PORT_L1:
		port_change_bits |= USB_PORT_STAT_C_L1;
		break;
	default:
		/* Should never happen */
		ERR("clear unknown change 0x%x\n", wValue);
		return;
	}

	port_change_bits <<= 16;	/* move change bits to high word */
	phcd->port_status[wIndex - 1] &= ~port_change_bits;

	DBG("phcd->port_status 0x%x\n", phcd->port_status[wIndex - 1]);

	if ((phcd->port_status[wIndex - 1] & PHCD_PORT_STATUS_CHANGE_MASK) == 0) {
		DBG("set port_bitmap 0\n");
		phcd->port_bitmap = 0;
	}
}

static int phcd_hub_set_port_feature(u16 wValue, __u16 wIndex, u16 typeReq,
			u16 wLength, struct proxy_hcd *phcd, unsigned long *flags)
{
	u32 status;
	__u16 offset = 16;
	int ret = 0;

	switch (wValue) {
	case USB_PORT_FEAT_SUSPEND:
		status = phcd->port_status[wIndex - 1];
		if ((status & USB_PORT_STAT_SUSPEND) == 0) {
			status |= USB_PORT_STAT_SUSPEND;
			status |= USB_PORT_STAT_C_SUSPEND << offset;
		}
		phcd->port_status[wIndex - 1] = status;
		break;

	case USB_PORT_FEAT_POWER:
		status = phcd->port_status[wIndex - 1];
		if ((status & USB_PORT_STAT_POWER) == 0) {
			status |= USB_PORT_STAT_POWER;
			DBG("enable port power\n");
		}
		phcd->port_status[wIndex - 1] = status;
		break;

	case USB_PORT_FEAT_RESET:
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, typeReq, wValue,
			wIndex, (char *)&status, wLength);
		spin_lock_irqsave(&phcd->lock, *flags);
		break;

	default:
		ret = -1;
	}

	return ret;
}

static int phcd_hud_clear_port_feature(u16 wValue, u16 wIndex, u16 typeReq,
			u16 wLength, struct proxy_hcd *phcd, unsigned long *flags)
{
	u32 status;
	__u16 offset = 16;
	int ret = 0;

	switch (wValue) {
	case USB_PORT_FEAT_SUSPEND:
		status = phcd->port_status[wIndex - 1];
		if ((status & USB_PORT_STAT_SUSPEND) != 0) {
			status &= ~USB_PORT_STAT_SUSPEND;
			status |= USB_PORT_STAT_C_SUSPEND << offset;
		}
		phcd->port_status[wIndex - 1] = status;
		break;

	case USB_PORT_FEAT_C_CONNECTION:
	case USB_PORT_FEAT_C_ENABLE:
	case USB_PORT_FEAT_C_RESET:
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, typeReq,
			wValue, wIndex, (char *)&status, wLength);
		spin_lock_irqsave(&phcd->lock, *flags);
		phcd_clear_port_change_bit(phcd, wValue, wIndex);
		break;
	case USB_PORT_FEAT_C_SUSPEND:
	case USB_PORT_FEAT_C_OVER_CURRENT:
	case USB_PORT_FEAT_C_PORT_L1:
		phcd_clear_port_change_bit(phcd, wValue, wIndex);
		break;

	case USB_PORT_FEAT_ENABLE:
		status = phcd->port_status[wIndex - 1];
		if ((status & USB_PORT_STAT_ENABLE) != 0) {
			status &= ~USB_PORT_STAT_ENABLE;
			status |= USB_PORT_STAT_C_ENABLE << offset;
		}
		phcd->port_status[wIndex - 1] = status;
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, typeReq,
			wValue, wIndex, (char *)&status, wLength);
		spin_lock_irqsave(&phcd->lock, *flags);
		break;

	case USB_PORT_FEAT_POWER:
		if ((phcd->port_status[wIndex - 1] & USB_PORT_STAT_POWER) != 0)
			phcd->port_status[wIndex - 1] &= ~USB_PORT_STAT_POWER;
		break;

	default:
		ret = -1;
	}
	return ret;
}

static int phcd_hub_get_port_status(struct proxy_hcd *phcd, u16 wValue, u16 wIndex,
			u16 wLength, unsigned long *flags, char *buf)
{
	u32 status;
	int max_ports = phcd->port_count;

	DBG("GetPortStatus\n");
	if (!wIndex || wIndex > max_ports || wValue)
		return -1;

	if ((size_t)wLength < sizeof(status))
		return -1;

	spin_unlock_irqrestore(&phcd->lock, *flags);
	status = phcd_get_port_status(phcd, wIndex);
	spin_lock_irqsave(&phcd->lock, *flags);

	/* update phcd->port_status[i] by status */
	phcd_update_port_status(phcd, wIndex, status);
	put_unaligned(cpu_to_le32(phcd->port_status[wIndex - 1]), (__le32 *) buf);
	DBG("Get port status: 0x%x\n", phcd->port_status[wIndex - 1]);

	return 0;
}

static void hub_control_add_stat(struct proxy_hcd_stat *stat)
{
	struct timespec uptime;

	stat->stat_hub_control++;
	get_monotonic_boottime(&uptime);
	stat->last_hub_control_time = (unsigned long)uptime.tv_sec;
}

/* This is hub_control callback. */
static int phcd_hub_control(struct usb_hcd *hcd, u16 typeReq, u16 wValue, u16 wIndex, char *buf, u16 wLength)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int max_ports = phcd->port_count;
	unsigned long flags;
	int ret = 0;

	DBG("+\n");
	mutex_lock(&phcd->mutex);
	spin_lock_irqsave(&phcd->lock, flags);

	hub_control_add_stat(&phcd->stat);

	switch (typeReq) {
	case GetHubStatus:
		DBG("GetHubStatus\n");
		/* No power source, over-current reported per port */
		memset(buf, 0, 4);
		break;

	case GetHubDescriptor:
		DBG("GetHubDescriptor\n");
		phcd_usb2_hub_descriptor(phcd, (struct usb_hub_descriptor *) buf);
		break;

	case GetPortStatus:
		ret = phcd_hub_get_port_status(phcd, wValue, wIndex, wLength, &flags, buf);
		if (ret != 0)
			goto port_proc_err;
		break;

	case SetPortFeature:
		DBG("SetPortFeature, wValue %u\n", wValue);
		wIndex &= 0xff;
		if (!wIndex || wIndex > max_ports)
			goto port_proc_err;

		ret = phcd_hub_set_port_feature(wValue, wIndex, typeReq, wLength, phcd, &flags);
		if (ret == -1)
			goto port_proc_err;
		break;

	case ClearPortFeature:
		DBG("ClearPortFeature, wValue %u\n", wValue);
		if (!wIndex || wIndex > max_ports || wLength)
			goto port_proc_err;

		ret = phcd_hud_clear_port_feature(wValue, wIndex, typeReq, wLength, phcd, &flags);
		if (ret == -1)
			goto port_proc_err;
		break;

	default:
port_proc_err:
		ERR("Error! typeReq 0x%04x, wValue 0x%04x, wIndex 0x%04x, wLength 0x%04x\n",
				typeReq, wValue, wIndex, wLength);
		/* "stall" on error */
		ret = -EPIPE;
	}
	spin_unlock_irqrestore(&phcd->lock, flags);
	mutex_unlock(&phcd->mutex);
	DBG("ret %d\n", ret);
	DBG("-\n");
	return ret;
}

static void hub_status_data_add_stat(struct proxy_hcd_stat *stat)
{
	struct timespec uptime;

	stat->stat_hub_status_data++;
	get_monotonic_boottime(&uptime);
	stat->last_hub_status_data_time = (unsigned long)uptime.tv_sec;
}

/*
 * Returns 0 if the status hasn't changed, or the number of bytes in buf.
 * Ports are 0-indexed from the HCD point of view,
 * and 1-indexed from the USB core pointer of view.
 *
 * Note that the status change bits will be cleared as soon as a port status
 * change event is generated, so we use the saved status from that event.
 *
 * context: may in interrupt.
 */
static int phcd_hub_status_data(struct usb_hcd *hcd, char *buf)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	unsigned long flags;
	int retval;
	unsigned int i;
	u32 status;

	DBG("+\n");

	spin_lock_irqsave(&phcd->lock, flags);
	for (i = 0; ((i < phcd->port_count) && (i < PROXY_HCD_MAX_PORTS)); i++) {
		status = phcd->port_status[i];
		if ((status & PHCD_PORT_STATUS_CHANGE_MASK) != 0)
			phcd->port_bitmap |= (1 << (i + 1));
	}

	*buf = (unsigned char)phcd->port_bitmap;
	retval = *buf ? 1 : 0;

	if (!retval)
		clear_bit(HCD_FLAG_POLL_RH, &hcd->flags); /* this is atomic */

	hub_status_data_add_stat(&phcd->stat);
	spin_unlock_irqrestore(&phcd->lock, flags);

	DBG("retval %d, buf 0x%x\n", retval, *buf);
	DBG("-\n");

	return retval;
}

#ifdef CONFIG_PM
static int phcd_bus_suspend(struct usb_hcd *hcd)
{
	DBG("+\n");
	bus_suspend_add_stat(&hcd_to_phcd(hcd)->stat);
	DBG("-\n");
	return 0;
}

static int phcd_bus_resume(struct usb_hcd *hcd)
{
	DBG("+\n");
	bus_resume_add_stat(&hcd_to_phcd(hcd)->stat);
	DBG("-\n");
	return 0;
}
#else
#define phcd_bus_suspend NULL
#define phcd_bus_resume NULL
#endif	/* CONFIG_PM */

struct hc_driver phcd_hc_driver = {
	.description =		"proxy-hcd",
	.product_desc =		"Proxy Host Controller",
	.hcd_priv_size =	sizeof(struct proxy_hcd *),

	/* generic hardware linkage */
	.irq =			NULL,
	.flags =		HCD_MEMORY | HCD_USB2,

	/* basic lifecycle operations */
	.reset =		phcd_setup,
	.start =		phcd_start,
	.stop =			phcd_stop,

	/* managing i/o requests and associated device resources */
	.get_frame_number =	phcd_get_frame_number,
	.urb_enqueue =		phcd_urb_enqueue,
	.urb_dequeue =		phcd_urb_dequeue,
	.map_urb_for_dma =	phcd_map_urb_for_dma,
	.unmap_urb_for_dma =	phcd_unmap_urb_for_dma,

	.endpoint_disable =	phcd_endpoint_disable,
	.endpoint_reset =	phcd_endpoint_reset,

	.alloc_dev =		phcd_alloc_dev,
	.free_dev =		phcd_free_dev,

	.add_endpoint =		phcd_add_endpoint,
	.drop_endpoint =	phcd_drop_endpoint,

	.check_bandwidth =	phcd_check_bandwidth,
	.reset_bandwidth =	phcd_reset_bandwidth,

	.address_device =	phcd_address_device,
	.enable_device =	phcd_enable_device,
	.update_hub_device =	phcd_update_hub_device,
	.reset_device =		phcd_reset_device,

	/* root hub support */
	.hub_control =		phcd_hub_control,
	.hub_status_data =	phcd_hub_status_data,
	.bus_suspend =		phcd_bus_suspend,
	.bus_resume =		phcd_bus_resume,

	/* call back when device connected and addressed */
	.update_device =        phcd_update_device,
	.set_usb2_hw_lpm =	phcd_set_usb2_hardware_lpm,
};

static int phcd_client_init(struct proxy_hcd_client *client,
				struct proxy_hcd *phcd)
{
	client_ref_init(&client->client_ref);
	client->phcd = phcd;

	if (client->client_init != NULL)
		return client->client_init(client);

	return 0;
}

static void phcd_client_exit(struct proxy_hcd_client *client)
{
	if (client->client_exit != NULL)
		client->client_exit(client);

	client->phcd = NULL;
}

static LIST_HEAD(phcd_client_list);
static DEFINE_SPINLOCK(phcd_client_list_lock);

int phcd_register_client(struct proxy_hcd_client *client)
{
	spin_lock(&phcd_client_list_lock);
	list_add_tail(&client->node, &phcd_client_list);
	spin_unlock(&phcd_client_list_lock);
	return 0;
}

void phcd_unregister_client(struct proxy_hcd_client *client)
{
	struct proxy_hcd *phcd = client_to_phcd(client);

	spin_lock(&phcd_client_list_lock);
	list_del_init(&phcd->client->node);
	spin_unlock(&phcd_client_list_lock);
	phcd->client = NULL;
}

struct proxy_hcd_client *find_client_by_name(const char *name)
{
	struct proxy_hcd_client *client = NULL;

	if (name == NULL)
		return NULL;

	spin_lock(&phcd_client_list_lock);

	list_for_each_entry(client, &phcd_client_list, node) {
		if (strncmp(name, client->name, PROXY_HCD_CLIENT_NAME_LEN) == 0) {
			spin_unlock(&phcd_client_list_lock);
			return client;
		}
	}

	spin_unlock(&phcd_client_list_lock);

	return NULL;
}

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
		INFO("forbid hibernation\n");
		return HIFI_USB_HIBERNATION_FORBID;
	} else {
		return HIFI_USB_HIBERNATION_ALLOW;
	}
}

#ifdef CONFIG_OF
static const struct of_device_id phcd_of_match[] = {
	{
		.compatible = "hisilicon,proxy-hcd-hifi",
	},
	{
		.compatible = "hisilicon,proxy-hcd-xhci",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, phcd_of_match);
#endif

static int phcd_plat_hcd_create_register(struct platform_device *pdev,
					struct proxy_hcd *phcd)
{
	struct usb_hcd *hcd = NULL;
	int ret;

	hcd = usb_create_hcd(&phcd_hc_driver, &pdev->dev, dev_name(&pdev->dev));
	if (hcd == NULL) {
		phcd_client_exit(phcd->client);
		kfree(phcd);
		return -ENOMEM;
	}

	*((struct proxy_hcd **) hcd->hcd_priv) = phcd;

	/* register usb_hcd */
	ret = usb_add_hcd(hcd, 0, 0);
	if (ret) {
		usb_put_hcd(hcd);
		phcd_client_exit(phcd->client);
		kfree(phcd);
	}

	return ret;
}

static int phcd_plat_probe(struct platform_device *pdev)
{
	struct proxy_hcd *phcd = NULL;
	const struct of_device_id *match = NULL;
	int ret;

	INFO("+\n");
	if (usb_disabled())
		return -ENODEV;

	phcd = kzalloc(sizeof(struct proxy_hcd), GFP_KERNEL);
	if (phcd == NULL)
		return -ENOMEM;

	spin_lock_init(&phcd->lock);
	mutex_init(&phcd->mutex);
	phcd->port_count = PROXY_HCD_MAX_PORTS;
	phcd->pdev = pdev;

	match = of_match_device(phcd_of_match, &pdev->dev);
	if (match != NULL)
		phcd->client = find_client_by_name(match->compatible);
	else
		phcd->client = NULL;
	if (phcd->client == NULL) {
		ERR("no client!!!\n");
		kfree(phcd);
		return -ENODEV;
	}

	if (!pdev->dev.dma_mask)
		ret = dma_coerce_mask_and_coherent(&pdev->dev,
			DMA_BIT_MASK(64));
	else
		ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64));
	if (ret) {
		ERR("set dmamask to MASK 64 failed\n");
		kfree(phcd);
		return ret;
	}

	ret = phcd_client_init(phcd->client, phcd);
	if (ret) {
		ERR("phcd_client_init failed\n");
		kfree(phcd);
		return ret;
	}

	/* create & register usb_hcd */
	ret = phcd_plat_hcd_create_register(pdev, phcd);
	if (ret != 0)
		return ret;

	if (phcd_debugfs_init(phcd))
		DBG("phcd_debugfs_init failed\n");

	INFO("-\n");

	return 0;
}

static int phcd_plat_remove(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);

	INFO("+\n");

	phcd_debugfs_exit(phcd);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	phcd_client_exit(phcd->client);
	kfree(phcd);

	INFO("-\n");

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phcd_plat_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	INFO("+\n");

	/* barrier from alloc_dev and free_dev */
	mutex_lock(&phcd->mutex);
	if (phcd->client->client_suspend)
		ret = phcd->client->client_suspend(phcd->client);
	else
		ret = 0;
	mutex_unlock(&phcd->mutex);

	mailbox_usb_suspend(false);

	INFO("-\n");
	return ret;
}

static int phcd_plat_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);

	INFO("+\n");

	/* barrier from alloc_dev and free_dev */
	mutex_lock(&phcd->mutex);
	if (phcd->client->client_resume)
		phcd->client->client_resume(phcd->client);
	mutex_unlock(&phcd->mutex);

	INFO("-\n");
	return 0;
}

int phcd_plat_prepare(struct device *dev)
{
	INFO("+\n");
	mailbox_usb_suspend(true);
	INFO("-\n");
	return 0;
}
void phcd_plat_complete(struct device *dev)
{
	INFO("+\n");
	INFO("-\n");
}

int phcd_plat_suspend_late(struct device *dev)
{
	DBG("+\n");
	DBG("-\n");
	return 0;
}
int phcd_plat_resume_early(struct device *dev)
{
	DBG("+\n");
	DBG("-\n");
	return 0;
}

static const struct dev_pm_ops phcd_plat_pm_ops = {
	SET_SYSTEM_SLEEP_PM_OPS(phcd_plat_suspend, phcd_plat_resume)
	.prepare = phcd_plat_prepare,
	.complete = phcd_plat_complete,
	.suspend_late = phcd_plat_suspend_late,
	.resume_early = phcd_plat_resume_early,
};
#define DEV_PM_OPS	(&phcd_plat_pm_ops)
#else
#define DEV_PM_OPS	NULL
#endif /* CONFIG_PM_SLEEP */

static struct platform_driver phcd_plat_driver = {
	.probe	= phcd_plat_probe,
	.remove	= phcd_plat_remove,
	.driver	= {
		.name = "proxy-hcd",
		.pm = DEV_PM_OPS,
		.of_match_table = of_match_ptr(phcd_of_match),
	},
};

static int __init phcd_plat_init(void)
{
	return platform_driver_register(&phcd_plat_driver);
}

static void __exit phcd_plat_exit(void)
{
	platform_driver_unregister(&phcd_plat_driver);
}

module_init(phcd_plat_init);
module_exit(phcd_plat_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("proxy HCD");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
