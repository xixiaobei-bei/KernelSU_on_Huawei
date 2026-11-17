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

#include <asm/byteorder.h>
#include <asm/unaligned.h>
#include <linux/dma-mapping.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/usb.h>
#include <linux/usb/ch11.h>
#include <linux/usb/ch9.h>
#include <linux/usb/hcd.h>

#include "client-ref.h"
#include "proxy-hcd-debugfs.h"
#include "proxy-hcd-stat.h"
#include "usbaudio-monitor.h"

#define dbg(format, arg...) pr_debug("[phcd][DBG][%s]" format, __func__, ##arg)
#define info(format, arg...) pr_info("[phcd][INFO][%s]" format, __func__, ##arg)
#define err(format, arg...) pr_err("[phcd][ERR][%s]" format, __func__, ##arg)

static int phcd_setup(struct usb_hcd *hcd)
{
	dbg("+\n");

	hcd->self.sg_tablesize = 0; /* Not accept scatter-gather lists */
	hcd->self.no_sg_constraint = 0; /* Not support to build packet from discontinuous buffers */
	hcd->self.no_stop_on_short = 1; /* Controllers don't stop the ep queue on short packets */

	hcd->speed = HCD_USB2;
	hcd->self.root_hub->speed = USB_SPEED_HIGH;
	hcd->has_tt = 1; /* Integrated TT in root hub */

	dbg("-\n");
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

	dbg("+\n");

	spin_lock_irqsave(&phcd->lock, flags);
	hcd->uses_new_polling = 1; /* support the new root-hub polling mechanism. */
	phcd->hcd = hcd;

	reset_proxy_hcd_stat(&phcd->stat);
	spin_unlock_irqrestore(&phcd->lock, flags);

	dbg("-\n");

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

	dbg("+\n");

	spin_lock_irqsave(&phcd->lock, flags);
	phcd->hcd = NULL;
	spin_unlock_irqrestore(&phcd->lock, flags);

	dbg("-\n");
}

static int phcd_get_frame_number(struct usb_hcd *hcd)
{
	info("\n");
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
		err("slot_id not match!\n");
		return NULL;
	}

	phcd_urb = find_urb_by_buf(&phcd_ep->urb_list, urb_msg->share_buf);
	if (phcd_urb == NULL) {
		err("Can't find the urb, may be completed or dequeued already\n");
		return NULL;
	}

	/* double check */
	if (phcd_urb->urb_msg->share_buf_dma != urb_msg->share_buf_dma) {
		err("share_buf_dma NOT MATCH\n");
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
		err("req NULL\n");
		return false;
	}

	configuration = le16_to_cpu(ctrl->wValue);
	if (configuration <= 0)
		return false;

	if ((ctrl->bRequest == USB_REQ_SET_CONFIGURATION)
				&& (ctrl->bRequestType == 0)) {
		info("to check_non_usbaudio_device, configuration %d\n", configuration);
		if (stop_hifi_usb_when_non_usbaudio(udev, configuration)) {
			dbg("non-usbaudio device using hifiusb\n");
			return true;
		}
	}

	return false;
}

static void phcd_check_urb_result(struct urb *urb, struct urb_msg *urb_msg,
				int status)
{
	/* check urb result */
	if ((urb->actual_length != urb->transfer_buffer_length &&
			(urb->transfer_flags & URB_SHORT_NOT_OK)) ||
			((status != 0) && !usb_endpoint_xfer_isoc(&urb->ep->desc)))
		dbg("Giveback URB: len = %u, expected = %u, status = %d\n",
				urb->actual_length, urb->transfer_buffer_length, status);

	if (urb->actual_length > PHCD_MAX_XFER_LEN) {
		err("Too large actual_length!\n");
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

	dbg("+\n");

	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb_msg->pipe);
	if ((ep_index < 0) || (ep_index >= PROXY_HCD_DEV_MAX_EPS)) {
		err("invalid ep_index %d\n", ep_index);
		goto ptr_null_err;
	}

	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	hcd = phcd->hcd;
	if (hcd == NULL) {
		err("hcd removed!\n");
		goto ptr_null_err;
	}

	if (phcd->phcd_udev.udev == NULL) {
		err("udev disconnected!\n");
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

	dbg("-\n");
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
		dbg("isoc xfer\n");
		dbg("len %u, pkts %d\n", urb->transfer_buffer_length,
			urb->number_of_packets);
		/* support only one packet */
		if (urb->number_of_packets != 1) {
			dbg("number_of_packets too large\n");
			return -EPIPE;
		}

		fill_phcd_urb_comm(phcd_urb->urb_msg, urb);

		if (usb_urb_dir_out(urb)) {
			dbg("isoc out xfer\n");
			memcpy(phcd_urb->urb_msg->buf, urb->transfer_buffer,
				urb->transfer_buffer_length);
		} else {
			dbg("isoc in xfer\n");
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
		err("usb_hcd_link_urb_to_ep failed, ret %d\n", ret);
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
		dbg("intercept %s remote wakeup\n", ctrl->bRequest
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

		dbg("intercept get device status\n");

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

	dbg("+\n");

	if (!HCD_HW_ACCESSIBLE(hcd)) {
		err("hcd not hw accessible!\n");
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
		err("alloc urb_priv failed\n");
		return -ENOMEM;
	}

	phcd_urb->urb_msg = kzalloc(sizeof(*phcd_urb->urb_msg) + PHCD_MAX_XFER_LEN, mem_flags);
	if (phcd_urb->urb_msg == NULL) {
		err("alloc urb_msg failed\n");
		kfree(phcd_urb);
		return -ENOMEM;
	}

	usb_get_urb(urb);
	phcd_urb->urb_msg->buf = (__u8 *)(phcd_urb->urb_msg + 1);

	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	dbg("ep %d, urb %pK enqueue\n", ep_index, urb);

	slot_id = urb->dev->slot_id;
	if ((phcd->phcd_udev.udev == NULL) || (slot_id != phcd->phcd_udev.udev->slot_id)) {
		err("slot_id not match\n");
		ret = -ENODEV;
		goto free_phcd_urb;
	}

	phcd_urb->urb = urb;
	urb->hcpriv = phcd_urb;

	INIT_LIST_HEAD(&phcd_urb->urb_list);
	list_add_tail(&phcd_urb->urb_list, &phcd_ep->urb_list);

	ret = usb_hcd_link_urb_to_ep(hcd, urb);
	if (unlikely(ret)) {
		err("usb_hcd_link_urb_to_ep failed %d\n", ret);
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
		err("proxy_urb_enqueue failed %d\n", ret);
		spin_lock_irqsave(&phcd->lock, flags);
		goto unlink_urb;
	}

	spin_lock_irqsave(&phcd->lock, flags);
	if (urb->hcpriv == phcd_urb)
		phcd_urb->flags &= ~PHCD_URB_ENQUEUEING;
	urb_enqueue_add_stat(&phcd_ep->urb_stat);
	spin_unlock_irqrestore(&phcd->lock, flags);

	usb_put_urb(urb);
	dbg("-\n");

	return 0;

unlink_urb:

	/* The urb maybe already dequeued! */
	if ((!urb->hcpriv) || (urb->hcpriv != phcd_urb)) {
		err("phcd_urb was freed\n");
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
	err("proxy_urb_dequeue failed %d\n", ret);

	phcd_urb = urb->hcpriv;
	if (phcd_urb == NULL) {
		WARN_ON(1);
		ret = -ENOENT;
		return ret;
	}

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	urb_dequeue_giveback_add_stat(&phcd_ep->urb_stat);
	info("client not response, giveback urb right now\n");
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
	unsigned long wq_complete_count = MAX_ENQUEUE_COUNT;
	unsigned long flags;
	int ep_index;
	int slot_id;
	int ret = 0;

	dbg("+\n");

	if (!HCD_HW_ACCESSIBLE(hcd)) {
		err("hcd not hw accessible!\n");
		return -ESHUTDOWN;
	}

wait_enqueue_complete:
	spin_lock_irqsave(&phcd->lock, flags);

	ep_index = phcd_get_ep_index(urb->pipe);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];

	slot_id = urb->dev->slot_id;
	if (!phcd->phcd_udev.udev || (slot_id != phcd->phcd_udev.udev->slot_id)) {
		err("slot_Id not match\n");
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
		if (wq_complete_count-- == 0) {
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
			msleep(10); /* 10ms delay */
		}
		goto wait_enqueue_complete;
	}

	if (wq_complete_count != MAX_ENQUEUE_COUNT)
		info("wq_complete_count:%lu\n",
				wq_complete_count);

	dbg("ep %d, urb %pK dequeue\n", ep_index, urb);

	/* urb maybe unlinked just before here */
	ret = usb_hcd_check_unlink_urb(hcd, urb, status);
	if (ret) {
		err("usb_hcd_check_unlink_urb urb_dequeue_err %d\n", ret);
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

	dbg("-\n");
	return 0;
urb_dequeue_err:
	urb_dequeue_fail_add_stat(&phcd_ep->urb_stat);
	spin_unlock_irqrestore(&phcd->lock, flags);
	dbg("- ret %d\n", ret);
	return ret;
}

static int phcd_map_urb_for_dma(struct usb_hcd *hcd, struct urb *urb,
			   gfp_t mem_flags)
{
	dbg("+-\n");
	return 0;
}

static void phcd_unmap_urb_for_dma(struct usb_hcd *hcd, struct urb *urb)
{
	dbg("+-\n");
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
	dbg("ep addr 0x%02x\n", ep->desc.bEndpointAddress);
}

static void phcd_endpoint_disable(struct usb_hcd *hcd,
		struct usb_host_endpoint *ep)
{
	dbg("ep addr 0x%02x\n", ep->desc.bEndpointAddress);
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

	dbg("+\n");

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

	dbg("-\n");

	return 1;
}

void phcd_giveback_all_urbs(struct proxy_hcd_client *client)
{
	struct proxy_hcd *phcd = client_to_phcd(client);
	struct usb_hcd *hcd = NULL;
	unsigned long flags;
	int i;

	info("+\n");

	if (phcd == NULL) {
		err("phcd NULL\n");
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
				dbg("ep %d urb %pK unlinked, giveback\n", i, urb);
			else
				dbg("ep %d urb %pK not unlinked, giveback\n", i, urb);

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

	info("-\n");
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

	dbg("+\n");

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
			dbg("ep%d has remaining urb\n", i);
		}
	}

	spin_unlock_irqrestore(&phcd->lock, flags);
	mutex_unlock(&phcd->mutex);

	dbg("-\n");
}

static int phcd_add_endpoint(struct usb_hcd *hcd, struct usb_device *udev,
		struct usb_host_endpoint *ep)
{
	struct proxy_hcd *phcd = NULL;
	struct proxy_hcd_ep *phcd_ep = NULL;
	int ep_index;
	int ret;

	dbg("+\n");

	if ((hcd == NULL) || (ep == NULL) || (udev == NULL)) {
		dbg("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		dbg("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_add_endpoint(phcd, udev, ep);
	if (ret < 0) {
		err("proxy_add_endpoint failed %d\n", ret);
		return ret;
	}

	ep_index = phcd_get_ep_index_from_desc(&ep->desc);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	phcd_ep->added = 1;

	/* Store the usb_device pointer for later use */
	ep->hcpriv = udev;

	dbg("add ep 0x%x, slot id %d\n", (unsigned int) ep->desc.bEndpointAddress, udev->slot_id);
	dbg("-\n");

	return 0;
}

void phcd_mark_all_endpoint_dropped(struct proxy_hcd *phcd)
{
	int i;

	dbg("+\n");
	for (i = 0; i < PROXY_HCD_DEV_MAX_EPS; i++)
		phcd->phcd_udev.phcd_eps[i].added = 0;
	dbg("-\n");
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

	dbg("+\n");
	if ((hcd == NULL) || (ep == NULL) || (udev == NULL)) {
		dbg("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		dbg("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	ep_index = phcd_get_ep_index_from_desc(&ep->desc);
	phcd_ep = &phcd->phcd_udev.phcd_eps[ep_index];
	if (!phcd_ep->added) {
		dbg("ep %d not added\n", ep_index);
		return 0;
	}

	might_sleep();

	ret = proxy_drop_endpoint(phcd, udev, ep);
	if (ret < 0) {
		err("proxy_drop_endpoint failed %d\n", ret);
		return ret;
	}
	phcd_ep->added = 0;

	dbg("drop ep 0x%x, slot id %d\n", (unsigned int) ep->desc.bEndpointAddress, udev->slot_id);
	dbg("-\n");

	return 0;
}

static int phcd_check_bandwidth(struct usb_hcd *hcd, struct usb_device *udev)
{
	int ret;
	struct proxy_hcd *phcd = NULL;

	dbg("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		dbg("invalid args\n");
		return -EINVAL;
	}
	if (udev->parent == NULL) {
		dbg("for root hub\n");
		return 0;
	}

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_check_bandwidth(phcd, udev->slot_id);
	if (ret < 0) {
		err("proxy_check_bandwidth failed %d\n", ret);
		return ret;
	}
	dbg("-\n");

	return ret;
}

static void phcd_reset_bandwidth(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = NULL;
	int ret;

	dbg("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		dbg("invalid args\n");
		return;
	}
	if (udev->parent == NULL) {
		dbg("for root hub\n");
		return;
	}
	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_reset_bandwidth(phcd, udev->slot_id);
	if (ret < 0)
		err("proxy_add_endpoint failed %d\n", ret);
	dbg("-\n");
}

static int phcd_address_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	dbg("+\n");
	if (!udev->slot_id)
		return -EINVAL;

	might_sleep();

	ret = proxy_address_device(phcd, udev->slot_id);
	if (ret < 0)
		err("proxy_address_device failed %d\n", ret);
	dbg("-\n");
	return ret;
}

static int phcd_enable_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	dbg("+\n");
	if (!udev->slot_id)
		return -EINVAL;

	might_sleep();

	ret = proxy_enable_device(phcd, udev->slot_id, udev->speed,
		udev->ep0.desc.wMaxPacketSize);
	if (ret < 0)
		err("proxy_enable_device failed %d\n", ret);
	dbg("-\n");

	return ret;
}

static int phcd_update_hub_device(struct usb_hcd *hcd, struct usb_device *hdev,
			struct usb_tt *tt, gfp_t mem_flags)
{
	dbg("+\n");
	if (hdev->parent == NULL)
		return 0;
	dbg("-\n");
	return -ENODEV;
}

static int phcd_reset_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	int ret;
	struct proxy_hcd *phcd = NULL;

	dbg("+\n");
	if ((hcd == NULL) || (udev == NULL)) {
		dbg("invalid args\n");
		return -EINVAL;
	}

	if (udev->parent == NULL) {
		dbg("for root hub\n");
		return 0;
	}

	dbg("udev udev->route 0x%x\n", udev->route);
	dbg("udev udev->portnum 0x%x\n", udev->portnum);

	phcd = hcd_to_phcd(hcd);

	might_sleep();

	ret = proxy_reset_device(phcd, udev->slot_id);
	if (ret < 0)
		err("proxy_reset_device failed %d\n", ret);
	dbg("-\n");

	return ret;
}

#ifdef CONFIG_PM
static int phcd_update_device(struct usb_hcd *hcd, struct usb_device *udev)
{
	struct proxy_hcd *phcd = NULL;
	int ret = 0;

	dbg("+\n");
	dbg("lpm_capable %u\n", udev->lpm_capable);

	if ((hcd->speed == HCD_USB3) || !udev->lpm_capable)
		return 0;

	/* we only support lpm for non-hub device connected to root hub yet */
	if ((udev->parent == NULL) || (udev->parent->parent != NULL) ||
			(udev->descriptor.bDeviceClass == USB_CLASS_HUB))
		return 0;

	udev->usb2_hw_lpm_capable = 1; /* enable LPM mode */
	udev->l1_params.timeout = 512; /* LPM default timeout value:512 */
	udev->l1_params.besl = 4; /* indicate the latency resume from L1:4 */
	udev->usb2_hw_lpm_besl_capable = 1; /* enable latency resume from L1 mode */

	phcd = hcd_to_phcd(hcd);
	if (udev->bos && udev->bos->ext_cap) {
		ret = proxy_update_device(phcd, udev->slot_id, udev->bos->ext_cap);
		if (ret)
			pr_err("proxy_update_device return %d\n", ret);
	}

	dbg("-\n");

	return ret;
}

static int phcd_set_usb2_hardware_lpm(struct usb_hcd *hcd,
			struct usb_device *udev, int enable)
{
	dbg("+\n");
	if (hcd->speed == HCD_USB3 || !udev->lpm_capable)
		return -EPERM;

	if (!udev->parent || udev->parent->parent ||
			udev->descriptor.bDeviceClass == USB_CLASS_HUB)
		return -EPERM;

	if (udev->usb2_hw_lpm_capable != 1)
		return -EPERM;
	dbg("-\n");

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

	desc->bDescLength = USB_DT_HUB_NONVAR_SIZE + 2; /* 2 is the header length */
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

	if ((new_status & USB_PORT_STAT_CONNECTION) != (old_status & USB_PORT_STAT_CONNECTION))
		new_status |= (USB_PORT_STAT_C_CONNECTION << offset);

	if ((new_status & USB_PORT_STAT_ENABLE) != (old_status & USB_PORT_STAT_ENABLE))
		new_status |= (USB_PORT_STAT_C_ENABLE << offset);

	if ((new_status & USB_PORT_STAT_RESET) != (old_status & USB_PORT_STAT_RESET))
		new_status |= (USB_PORT_STAT_C_RESET << offset);

	phcd->port_status[port_index - 1] = new_status;
	dbg("old status 0x%x, new status 0x%x, mask 0x%x\n",
		old_status, new_status, mask);

	if ((phcd->port_status[port_index - 1] & PHCD_PORT_STATUS_CHANGE_MASK) == 0) {
		dbg("clear port_bitmap\n");
		phcd->port_bitmap &= ~(1U << port_index);
	}

	dbg("port_status 0x%x\n", phcd->port_status[port_index - 1]);
}

/* Used by hub_control, report the port status. */
static __u32 phcd_get_port_status(struct proxy_hcd *phcd, __u16 w_index)
{
	u32 status = 0;
	int retval;

	dbg("+\n");
	retval = proxy_hub_control(phcd, GetPortStatus, 0, w_index,
		(char *)&status, (__u16)sizeof(status));
	if (retval < 0) {
		status = USB_PORT_STAT_POWER;
		return status;
	}
	dbg("-\n");

	return status;
}

/* Used by hub_control, clear change bits in port status. */
static void phcd_clear_port_change_bit(struct proxy_hcd *phcd, __u16 w_value,
		__u16 w_index)
{
	__u32 port_change_bits = 0;

	switch (w_value) {
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
		err("clear unknown change 0x%x\n", w_value);
		return;
	}

	port_change_bits <<= 16; /* move change bits to high word:16bits shift */
	phcd->port_status[w_index - 1] &= ~port_change_bits;

	dbg("phcd->port_status 0x%x\n", phcd->port_status[w_index - 1]);

	if ((phcd->port_status[w_index - 1] & PHCD_PORT_STATUS_CHANGE_MASK) == 0) {
		dbg("set port_bitmap 0\n");
		phcd->port_bitmap = 0;
	}
}

static int phcd_hub_set_port_feature(u16 w_value, __u16 w_index, u16 type_req,
			u16 w_length, struct proxy_hcd *phcd, unsigned long *flags)
{
	u32 status;
	__u16 offset = 16;
	int ret = 0;

	switch (w_value) {
	case USB_PORT_FEAT_SUSPEND:
		status = phcd->port_status[w_index - 1];
		if ((status & USB_PORT_STAT_SUSPEND) == 0) {
			status |= USB_PORT_STAT_SUSPEND;
			status |= USB_PORT_STAT_C_SUSPEND << offset;
		}
		phcd->port_status[w_index - 1] = status;
		break;

	case USB_PORT_FEAT_POWER:
		status = phcd->port_status[w_index - 1];
		if ((status & USB_PORT_STAT_POWER) == 0) {
			status |= USB_PORT_STAT_POWER;
			dbg("enable port power\n");
		}
		phcd->port_status[w_index - 1] = status;
		break;

	case USB_PORT_FEAT_RESET:
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, type_req, w_value,
			w_index, (char *)&status, w_length);
		spin_lock_irqsave(&phcd->lock, *flags);
		break;

	default:
		ret = -1;
	}

	return ret;
}

static int phcd_hud_clear_port_feature(u16 w_value, u16 w_index, u16 type_req,
			u16 w_length, struct proxy_hcd *phcd, unsigned long *flags)
{
	u32 status;
	__u16 offset = 16;
	int ret = 0;

	switch (w_value) {
	case USB_PORT_FEAT_SUSPEND:
		status = phcd->port_status[w_index - 1];
		if ((status & USB_PORT_STAT_SUSPEND) != 0) {
			status &= ~USB_PORT_STAT_SUSPEND;
			status |= USB_PORT_STAT_C_SUSPEND << offset;
		}
		phcd->port_status[w_index - 1] = status;
		break;

	case USB_PORT_FEAT_C_CONNECTION:
	case USB_PORT_FEAT_C_ENABLE:
	case USB_PORT_FEAT_C_RESET:
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, type_req,
			w_value, w_index, (char *)&status, w_length);
		spin_lock_irqsave(&phcd->lock, *flags);
		phcd_clear_port_change_bit(phcd, w_value, w_index);
		break;
	case USB_PORT_FEAT_C_SUSPEND:
	case USB_PORT_FEAT_C_OVER_CURRENT:
	case USB_PORT_FEAT_C_PORT_L1:
		phcd_clear_port_change_bit(phcd, w_value, w_index);
		break;

	case USB_PORT_FEAT_ENABLE:
		status = phcd->port_status[w_index - 1];
		if ((status & USB_PORT_STAT_ENABLE) != 0) {
			status &= ~USB_PORT_STAT_ENABLE;
			status |= USB_PORT_STAT_C_ENABLE << offset;
		}
		phcd->port_status[w_index - 1] = status;
		spin_unlock_irqrestore(&phcd->lock, *flags);
		ret = proxy_hub_control(phcd, type_req,
			w_value, w_index, (char *)&status, w_length);
		spin_lock_irqsave(&phcd->lock, *flags);
		break;

	case USB_PORT_FEAT_POWER:
		if ((phcd->port_status[w_index - 1] & USB_PORT_STAT_POWER) != 0)
			phcd->port_status[w_index - 1] &= ~USB_PORT_STAT_POWER;
		break;

	default:
		ret = -1;
	}
	return ret;
}

static int phcd_hub_get_port_status(struct proxy_hcd *phcd, u16 w_value, u16 w_index,
			u16 w_length, unsigned long *flags, char *buf)
{
	u32 status;
	int max_ports = phcd->port_count;

	dbg("GetPortStatus\n");
	if (!w_index || w_index > max_ports || w_value)
		return -1;

	if ((size_t)w_length < sizeof(status))
		return -1;

	spin_unlock_irqrestore(&phcd->lock, *flags);
	status = phcd_get_port_status(phcd, w_index);
	spin_lock_irqsave(&phcd->lock, *flags);

	/* update phcd->port_status[i] by status */
	phcd_update_port_status(phcd, w_index, status);
	put_unaligned(cpu_to_le32(phcd->port_status[w_index - 1]), (__le32 *) buf);
	dbg("Get port status: 0x%x\n", phcd->port_status[w_index - 1]);

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
static int phcd_hub_control(struct usb_hcd *hcd, u16 type_req, u16 w_value, u16 w_index, char *buf, u16 w_length)
{
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int max_ports = phcd->port_count;
	unsigned long flags;
	int ret = 0;

	dbg("+\n");
	mutex_lock(&phcd->mutex);
	spin_lock_irqsave(&phcd->lock, flags);

	hub_control_add_stat(&phcd->stat);

	switch (type_req) {
	case GetHubStatus:
		dbg("GetHubStatus\n");
		/* No power source, over-current reported per port */
		memset(buf, 0, 4); /* clear buf 4bytes */
		break;

	case GetHubDescriptor:
		dbg("GetHubDescriptor\n");
		phcd_usb2_hub_descriptor(phcd, (struct usb_hub_descriptor *) buf);
		break;

	case GetPortStatus:
		ret = phcd_hub_get_port_status(phcd, w_value, w_index, w_length, &flags, buf);
		if (ret != 0)
			goto port_proc_err;
		break;

	case SetPortFeature:
		dbg("SetPortFeature, w_value %u\n", w_value);
		w_index &= 0xff;
		if (!w_index || w_index > max_ports)
			goto port_proc_err;

		ret = phcd_hub_set_port_feature(w_value, w_index, type_req, w_length, phcd, &flags);
		if (ret == -1)
			goto port_proc_err;
		break;

	case ClearPortFeature:
		dbg("ClearPortFeature, w_value %u\n", w_value);
		if (!w_index || w_index > max_ports || w_length)
			goto port_proc_err;

		ret = phcd_hud_clear_port_feature(w_value, w_index, type_req, w_length, phcd, &flags);
		if (ret == -1)
			goto port_proc_err;
		break;

	default:
port_proc_err:
		err("Error! type_req 0x%04x, w_value 0x%04x, w_index 0x%04x, w_length 0x%04x\n",
				type_req, w_value, w_index, w_length);
		/* "stall" on error */
		ret = -EPIPE;
	}
	spin_unlock_irqrestore(&phcd->lock, flags);
	mutex_unlock(&phcd->mutex);
	dbg("ret %d\n", ret);
	dbg("-\n");
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

	dbg("+\n");

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

	dbg("retval %d, buf 0x%x\n", retval, *buf);
	dbg("-\n");

	return retval;
}

#ifdef CONFIG_PM
static int phcd_bus_suspend(struct usb_hcd *hcd)
{
	dbg("+\n");
	bus_suspend_add_stat(&hcd_to_phcd(hcd)->stat);
	dbg("-\n");
	return 0;
}

static int phcd_bus_resume(struct usb_hcd *hcd)
{
	dbg("+\n");
	bus_resume_add_stat(&hcd_to_phcd(hcd)->stat);
	dbg("-\n");
	return 0;
}
#endif /* CONFIG_PM */

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
#ifdef CONFIG_PM
	.bus_suspend =		phcd_bus_suspend,
	.bus_resume =		phcd_bus_resume,
#else
	.bus_suspend =		NULL,
	.bus_resume =		NULL,
#endif /* CONFIG_PM */

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

	info("+\n");
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
		err("no client!!!\n");
		kfree(phcd);
		return -ENODEV;
	}

	if (!pdev->dev.dma_mask)
		ret = dma_coerce_mask_and_coherent(&pdev->dev,
			DMA_BIT_MASK(64)); /* mask all 64bits */
	else
		ret = dma_set_mask_and_coherent(&pdev->dev, DMA_BIT_MASK(64)); /* mask all 64bits */
	if (ret) {
		err("set dmamask to MASK 64 failed\n");
		kfree(phcd);
		return ret;
	}

	ret = phcd_client_init(phcd->client, phcd);
	if (ret) {
		err("phcd_client_init failed\n");
		kfree(phcd);
		return ret;
	}

	/* create & register usb_hcd */
	ret = phcd_plat_hcd_create_register(pdev, phcd);
	if (ret != 0)
		return ret;

	if (phcd_debugfs_init(phcd))
		dbg("phcd_debugfs_init failed\n");

	info("-\n");

	return 0;
}

static int phcd_plat_remove(struct platform_device *dev)
{
	struct usb_hcd *hcd = platform_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);

	info("+\n");

	phcd_debugfs_exit(phcd);
	usb_remove_hcd(hcd);
	usb_put_hcd(hcd);
	phcd_client_exit(phcd->client);
	kfree(phcd);

	info("-\n");

	return 0;
}

#ifdef CONFIG_PM_SLEEP
static int phcd_plat_suspend(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);
	int ret;

	info("+\n");

	/* barrier from alloc_dev and free_dev */
	mutex_lock(&phcd->mutex);
	if (phcd->client->client_suspend)
		ret = phcd->client->client_suspend(phcd->client);
	else
		ret = 0;
	mutex_unlock(&phcd->mutex);

	mailbox_usb_suspend(false);

	info("-\n");
	return ret;
}

static int phcd_plat_resume(struct device *dev)
{
	struct usb_hcd *hcd = dev_get_drvdata(dev);
	struct proxy_hcd *phcd = hcd_to_phcd(hcd);

	info("+\n");

	/* barrier from alloc_dev and free_dev */
	mutex_lock(&phcd->mutex);
	if (phcd->client->client_resume)
		phcd->client->client_resume(phcd->client);
	mutex_unlock(&phcd->mutex);

	info("-\n");
	return 0;
}

int phcd_plat_prepare(struct device *dev)
{
	info("+\n");
	mailbox_usb_suspend(true);
	info("-\n");
	return 0;
}
void phcd_plat_complete(struct device *dev)
{
	info("+\n");
	info("-\n");
}

int phcd_plat_suspend_late(struct device *dev)
{
	dbg("+\n");
	dbg("-\n");
	return 0;
}
int phcd_plat_resume_early(struct device *dev)
{
	dbg("+\n");
	dbg("-\n");
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
