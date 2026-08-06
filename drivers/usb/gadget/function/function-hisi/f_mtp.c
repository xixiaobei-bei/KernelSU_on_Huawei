/*
 * Copyright (c) Google, Inc. 2010-2020. All rights reserved.
 * Descriptiron: Gadget Function Driver for MTP
 * Author: Mike Lockwood <lockwood@android.com>
 * Create: 2010-1-27
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/err.h>
#include <linux/interrupt.h>

#include <linux/types.h>
#include <linux/file.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#include <linux/usb.h>
#include <linux/usb_usual.h>
#include <linux/usb/ch9.h>
#include <linux/usb/f_mtp.h>
#include <linux/configfs.h>
#include <linux/usb/composite.h>
#include <securec.h>

#include "configfs.h"

#ifdef CONFIG_DEC_USB
#define MTP_BULK_BUFFER_SIZE       32768
#else
#define MTP_BULK_BUFFER_SIZE       1048576
#endif
#define MTP_BULK_DEFAULT_BUFFER_SIZE       16384
#define INTR_BUFFER_SIZE           28
#define MAX_INST_NAME_LEN          40
#define MTP_MAX_FILE_SIZE          0xFFFFFFFFL

/* mtp descripotr para */
#define MTP_INTERFACE_NUM 0
#define MTP_ENDPOINTS_NUM 3
#define MTP_INTERFACE_PROTO 0
#define PTP_INTERFACE_SUB_CLASS 1
#define PTP_INTERFACE_PROTO 1
#define MTP_SS_BULK_MPK_SIZE 1024
#define MTP_HS_BULK_MPK_SIZE 512
#define MTP_INTR_INTERVAL 6
#define MTP_EXT_CONFIG_INDEX 4
#define MTP_EXT_CONFIG_COUNT 1
#define MTP_EXT_CONFIG_FIRST_INTERFACE_NUM 0
#define MTP_EXT_CONFIG_INTERFACE_COUNT 1
#define MTP_EXT_COMPAT_ID_SIZE 16

/* String IDs */
#define INTERFACE_STRING_INDEX	0

/* values for mtp_dev.state */
#define STATE_OFFLINE               0 /* initial state, disconnected */
#define STATE_READY                 1 /* ready for userspace calls */
#define STATE_BUSY                  2 /* processing userspace calls */
#define STATE_CANCELED              3 /* transaction canceled by host */
#define STATE_ERROR                 4 /* error from completion routine */

/* number of tx and rx requests to allocate */
#define TX_REQ_MAX 8
#define RX_REQ_MAX 8
#define INTR_REQ_MAX 5

/* ID for Microsoft MTP OS String */
#define MTP_OS_STRING_ID   0xEE

/* MTP class reqeusts */
#define MTP_REQ_CANCEL              0x64
#define MTP_REQ_GET_EXT_EVENT_DATA  0x65
#define MTP_REQ_RESET               0x66
#define MTP_REQ_GET_DEVICE_STATUS   0x67

/* constants for device status */
#define MTP_RESPONSE_OK             0x2001
#define MTP_RESPONSE_DEVICE_BUSY    0x2019
#define DRIVER_NAME "mtp"

static const char mtp_shortname[] = DRIVER_NAME "_usb";

struct mtp_dev {
	struct usb_function function;
	struct usb_composite_dev *cdev;
	spinlock_t lock;

	struct usb_ep *ep_in;
	struct usb_ep *ep_out;
	struct usb_ep *ep_intr;

	int state;

	/* synchronize access to our device file */
	atomic_t open_excl;
	/* to enforce only one ioctl at a time */
	atomic_t ioctl_excl;

	struct list_head tx_idle;
	struct list_head intr_idle;

	wait_queue_head_t read_wq;
	wait_queue_head_t write_wq;
	wait_queue_head_t intr_wq;
	wait_queue_head_t open_wq;
	struct mutex req_mutex;
	struct usb_request *rx_req[RX_REQ_MAX];
	int rx_done;

	/*
	 * for processing MTP_SEND_FILE, MTP_RECEIVE_FILE and
	 * MTP_SEND_FILE_WITH_HEADER ioctls on a work queue
	 */
	struct workqueue_struct *wq;
	struct work_struct send_file_work;
	struct work_struct receive_file_work;
	struct file *xfer_file;
	loff_t xfer_file_offset;
	int64_t xfer_file_length;
	unsigned int xfer_send_header;
	uint16_t xfer_command;
	uint32_t xfer_transaction_id;
	int xfer_result;
	u32 bulk_buffer_size;
};

static struct usb_interface_descriptor mtp_interface_desc = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = MTP_INTERFACE_NUM,
	.bNumEndpoints = MTP_ENDPOINTS_NUM,
	.bInterfaceClass = USB_CLASS_VENDOR_SPEC,
	.bInterfaceSubClass = USB_SUBCLASS_VENDOR_SPEC,
	.bInterfaceProtocol = MTP_INTERFACE_PROTO,
};

static struct usb_interface_descriptor ptp_interface_desc = {
	.bLength = USB_DT_INTERFACE_SIZE,
	.bDescriptorType = USB_DT_INTERFACE,
	.bInterfaceNumber = MTP_INTERFACE_NUM,
	.bNumEndpoints = MTP_ENDPOINTS_NUM,
	.bInterfaceClass = USB_CLASS_STILL_IMAGE,
	.bInterfaceSubClass = PTP_INTERFACE_SUB_CLASS,
	.bInterfaceProtocol = PTP_INTERFACE_PROTO,
};

static struct usb_endpoint_descriptor mtp_ss_in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __cpu_to_le16(MTP_SS_BULK_MPK_SIZE),
};

static struct usb_ss_ep_comp_descriptor mtp_ss_in_comp_desc = {
	.bLength = sizeof(mtp_ss_in_comp_desc),
	.bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_endpoint_descriptor mtp_ss_out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __cpu_to_le16(MTP_SS_BULK_MPK_SIZE),
};

static struct usb_ss_ep_comp_descriptor mtp_ss_out_comp_desc = {
	.bLength = sizeof(mtp_ss_out_comp_desc),
	.bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
};

static struct usb_endpoint_descriptor mtp_highspeed_in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __cpu_to_le16(MTP_HS_BULK_MPK_SIZE),
};

static struct usb_endpoint_descriptor mtp_highspeed_out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
	.wMaxPacketSize = __cpu_to_le16(MTP_HS_BULK_MPK_SIZE),
};

static struct usb_endpoint_descriptor mtp_fullspeed_in_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor mtp_fullspeed_out_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_OUT,
	.bmAttributes = USB_ENDPOINT_XFER_BULK,
};

static struct usb_endpoint_descriptor mtp_intr_desc = {
	.bLength = USB_DT_ENDPOINT_SIZE,
	.bDescriptorType = USB_DT_ENDPOINT,
	.bEndpointAddress = USB_DIR_IN,
	.bmAttributes = USB_ENDPOINT_XFER_INT,
	.wMaxPacketSize = __cpu_to_le16(INTR_BUFFER_SIZE),
	.bInterval = MTP_INTR_INTERVAL,
};

static struct usb_ss_ep_comp_descriptor mtp_intr_ss_comp_desc = {
	.bLength = sizeof(mtp_intr_ss_comp_desc),
	.bDescriptorType = USB_DT_SS_ENDPOINT_COMP,
	.wBytesPerInterval = cpu_to_le16(INTR_BUFFER_SIZE),
};

static struct usb_descriptor_header *fs_mtp_descs[] = {
	(struct usb_descriptor_header *)&mtp_interface_desc,
	(struct usb_descriptor_header *)&mtp_fullspeed_in_desc,
	(struct usb_descriptor_header *)&mtp_fullspeed_out_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	NULL,
};

static struct usb_descriptor_header *hs_mtp_descs[] = {
	(struct usb_descriptor_header *)&mtp_interface_desc,
	(struct usb_descriptor_header *)&mtp_highspeed_in_desc,
	(struct usb_descriptor_header *)&mtp_highspeed_out_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	NULL,
};

static struct usb_descriptor_header *ss_mtp_descs[] = {
	(struct usb_descriptor_header *)&mtp_interface_desc,
	(struct usb_descriptor_header *)&mtp_ss_in_desc,
	(struct usb_descriptor_header *)&mtp_ss_in_comp_desc,
	(struct usb_descriptor_header *)&mtp_ss_out_desc,
	(struct usb_descriptor_header *)&mtp_ss_out_comp_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	(struct usb_descriptor_header *)&mtp_intr_ss_comp_desc,
	NULL,
};

static struct usb_descriptor_header *fs_ptp_descs[] = {
	(struct usb_descriptor_header *)&ptp_interface_desc,
	(struct usb_descriptor_header *)&mtp_fullspeed_in_desc,
	(struct usb_descriptor_header *)&mtp_fullspeed_out_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	NULL,
};

static struct usb_descriptor_header *hs_ptp_descs[] = {
	(struct usb_descriptor_header *)&ptp_interface_desc,
	(struct usb_descriptor_header *)&mtp_highspeed_in_desc,
	(struct usb_descriptor_header *)&mtp_highspeed_out_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	NULL,
};

static struct usb_descriptor_header *ss_ptp_descs[] = {
	(struct usb_descriptor_header *)&ptp_interface_desc,
	(struct usb_descriptor_header *)&mtp_ss_in_desc,
	(struct usb_descriptor_header *)&mtp_ss_in_comp_desc,
	(struct usb_descriptor_header *)&mtp_ss_out_desc,
	(struct usb_descriptor_header *)&mtp_ss_out_comp_desc,
	(struct usb_descriptor_header *)&mtp_intr_desc,
	(struct usb_descriptor_header *)&mtp_intr_ss_comp_desc,
	NULL,
};

static struct usb_string mtp_string_defs[] = {
	/* Naming interface "MTP" so libmtp will recognize us */
	[INTERFACE_STRING_INDEX].s = "MTP",
	{}, /* end of list */
};

static struct usb_gadget_strings mtp_string_table = {
	.language = 0x0409, /* en-US */
	.strings = mtp_string_defs,
};

static struct usb_gadget_strings *mtp_strings[] = {
	&mtp_string_table,
	NULL,
};

/* Microsoft MTP OS String */
static u8 mtp_os_string[] = {
	18, /* sizeof(mtp_os_string) */
	USB_DT_STRING,
	/* Signature field: "MSFT100" */
	'M', 0, 'S', 0, 'F', 0, 'T', 0, '1', 0, '0', 0, '0', 0,
	/* vendor code */
	1,
	/* padding */
	0
};

/* Microsoft Extended Configuration Descriptor Header Section */
struct mtp_ext_config_desc_header {
	__le32 dw_length;
	__u16 bcd_version;
	__le16 w_index;
	__u8 b_count;
	__u8 reserved[7]; /* just follow spec, align 8 bytes */
};

/* Microsoft Extended Configuration Descriptor Function Section */
struct mtp_ext_config_desc_function {
	__u8 b_first_interface_number;
	__u8 b_interface_count;
	__u8 compatible_id[8]; /* just follow spec */
	__u8 sub_compatible_id[8]; /* just follow spec */
	__u8 reserved[6]; /* just follow spec, align 8 bytes */
};

/* MTP Extended Configuration Descriptor */
struct {
	struct mtp_ext_config_desc_header header;
	struct mtp_ext_config_desc_function function;
} mtp_ext_config_desc = {
	.header = {
		.dw_length = __cpu_to_le32(sizeof(mtp_ext_config_desc)),
		.bcd_version = __cpu_to_le16(0x0100), /* bcd version, follow spec */
		.w_index =  __cpu_to_le16(MTP_EXT_CONFIG_INDEX),
		.b_count = MTP_EXT_CONFIG_COUNT,
	},
	.function = {
		.b_first_interface_number = MTP_EXT_CONFIG_FIRST_INTERFACE_NUM,
		.b_interface_count = MTP_EXT_CONFIG_INTERFACE_COUNT,
		.compatible_id = {'M', 'T', 'P'},
	},
};

struct mtp_device_status {
	__le16 wLength;
	__le16 wCode;
};

struct mtp_data_header {
	/* length of packet, including this header */
	__le32 length;
	/* container type (2 for data packet) */
	__le16 type;
	/* MTP command code */
	__le16 command;
	/* MTP transaction ID */
	__le32 transaction_id;
};

struct mtp_instance {
	struct usb_function_instance func_inst;
	const char *name;
	struct mtp_dev *dev;
	char mtp_ext_compat_id[MTP_EXT_COMPAT_ID_SIZE];
	struct usb_os_desc mtp_os_desc;
};

/*
 * temporary variable used between
 * mtp_open() and mtp_gadget_bind()
 */
static struct mtp_dev *_mtp_dev;

static inline struct mtp_dev *func_to_mtp(struct usb_function *f)
{
	return container_of(f, struct mtp_dev, function);
}

static struct usb_request *mtp_request_new(struct usb_ep *ep, int buffer_size)
{
	struct usb_request *req = usb_ep_alloc_request(ep, GFP_KERNEL);

	if (req == NULL)
		goto err_exit;

	if (buffer_size <= 0 || buffer_size > MTP_BULK_BUFFER_SIZE)
		goto free_req_exit;

	/* now allocate buffers for the requests */
	req->buf = kmalloc(buffer_size, GFP_KERNEL);
	if (req->buf == NULL)
		goto free_req_exit;

	return req;

free_req_exit:
	usb_ep_free_request(ep, req);
err_exit:
	return NULL;
}

static void mtp_request_free(struct usb_request *req, struct usb_ep *ep)
{
	if (req != NULL) {
		kfree(req->buf);
		req->buf = NULL;
		usb_ep_free_request(ep, req);
	}
}

static int mtp_lock(atomic_t *excl)
{
	if (atomic_inc_return(excl) == 1)
		return 0;

	atomic_dec(excl);
	return -1;
}

static void mtp_unlock(atomic_t *excl)
{
	atomic_dec(excl);
}

/* add a request to the tail of a list */
static void mtp_req_put(struct mtp_dev *dev, struct list_head *head, struct usb_request *req)
{
	unsigned long flags;

	spin_lock_irqsave(&dev->lock, flags);
	list_add_tail(&req->list, head);
	spin_unlock_irqrestore(&dev->lock, flags);
}

/* remove a request from the head of a list */
static struct usb_request *mtp_req_get(struct mtp_dev *dev, struct list_head *head)
{
	unsigned long flags;
	struct usb_request *req = NULL;

	spin_lock_irqsave(&dev->lock, flags);
	if (!list_empty(head)) {
		req = list_first_entry(head, struct usb_request, list);
		list_del(&req->list);
	}
	spin_unlock_irqrestore(&dev->lock, flags);

	return req;
}

static void mtp_complete_in(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	if (req->status != 0 && dev->state != STATE_CANCELED)
		dev->state = STATE_ERROR;

	mtp_req_put(dev, &dev->tx_idle, req);

	wake_up(&dev->write_wq);
}

static void mtp_complete_out(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	dev->rx_done = 1;
	if (req->status != 0 && dev->state != STATE_CANCELED)
		dev->state = STATE_ERROR;

	wake_up(&dev->read_wq);
}

static void mtp_complete_intr(struct usb_ep *ep, struct usb_request *req)
{
	struct mtp_dev *dev = _mtp_dev;

	if (req->status != 0 && dev->state != STATE_CANCELED)
		dev->state = STATE_ERROR;

	mtp_req_put(dev, &dev->intr_idle, req);

	wake_up(&dev->intr_wq);
}

static int mtp_alloc_requests_tx(struct mtp_dev *dev)
{
	int i;
	struct usb_request *req = NULL;
	struct usb_request *tx_reqs[TX_REQ_MAX] = {NULL};

	for (i = 0; i < TX_REQ_MAX; i++) {
		req = mtp_request_new(dev->ep_in, dev->bulk_buffer_size);
		if (req == NULL) {
			pr_err("there is no %u buf,so we need alloc 16K\n", dev->bulk_buffer_size);
			dev->bulk_buffer_size = MTP_BULK_DEFAULT_BUFFER_SIZE;
			req = mtp_request_new(dev->ep_in, dev->bulk_buffer_size);
			if (req == NULL)
				goto fail_tx;
		}
		req->complete = mtp_complete_in;
		tx_reqs[i] = req;
	}

	for (i = 0; i < TX_REQ_MAX; i++)
		mtp_req_put(dev, &dev->tx_idle, tx_reqs[i]);

	return 0;

fail_tx:
	for (i = 0; i < TX_REQ_MAX; i++) {
		if (tx_reqs[i] != NULL)
			mtp_request_free(tx_reqs[i], dev->ep_in);
		else
			break;
	}

	return -1;
}

static int mtp_alloc_requests_rx(struct mtp_dev *dev)
{
	int i;
	struct usb_request *req = NULL;

	for (i = 0; i < RX_REQ_MAX; i++)
		dev->rx_req[i] = NULL;

	for (i = 0; i < RX_REQ_MAX; i++) {
		req = mtp_request_new(dev->ep_out, dev->bulk_buffer_size);
		if (req == NULL) {
			pr_err("there is no %u buf,so we need alloc 16K\n", dev->bulk_buffer_size);
			dev->bulk_buffer_size = MTP_BULK_DEFAULT_BUFFER_SIZE;
			req = mtp_request_new(dev->ep_out, dev->bulk_buffer_size);
			if (req == NULL)
				goto fail_rx;
		}
		req->complete = mtp_complete_out;
		dev->rx_req[i] = req;
	}
	return 0;

fail_rx:
	for (i = 0; i < RX_REQ_MAX; i++) {
		if (dev->rx_req[i] != NULL) {
			mtp_request_free(dev->rx_req[i], dev->ep_out);
			dev->rx_req[i] = NULL;
		} else {
			break;
		}
	}

	return -1;
}

static int mtp_alloc_requests_intr(struct mtp_dev *dev)
{
	int i;
	struct usb_request *req = NULL;
	struct usb_request *intr_reqs[INTR_REQ_MAX] = {NULL};

	for (i = 0; i < INTR_REQ_MAX; i++) {
		req = mtp_request_new(dev->ep_intr, INTR_BUFFER_SIZE);
		if (req == NULL)
			goto fail_intr;
		req->complete = mtp_complete_intr;
		intr_reqs[i] = req;
	}

	for (i = 0; i < INTR_REQ_MAX; i++)
		mtp_req_put(dev, &dev->intr_idle, intr_reqs[i]);

	return 0;

fail_intr:
	for (i = 0; i < INTR_REQ_MAX; i++) {
		if (intr_reqs[i] != NULL)
			mtp_request_free(intr_reqs[i], dev->ep_intr);
		else
			break;
	}

	return -1;
}

static void mtp_free_requests_tx(struct mtp_dev *dev)
{
	struct usb_request *req = NULL;

	for (req = mtp_req_get(dev, &dev->tx_idle); req != NULL;
			req = mtp_req_get(dev, &dev->tx_idle))
		mtp_request_free(req, dev->ep_in);
}

static void mtp_free_requests_rx(struct mtp_dev *dev)
{
	int i;

	for (i = 0; i < RX_REQ_MAX; i++) {
		mtp_request_free(dev->rx_req[i], dev->ep_out);
		dev->rx_req[i] = NULL;
	}
}

static int mtp_alloc_requests(struct mtp_dev *dev)
{
	pr_info("[MTP]%s: +\n", __func__);
	if (mtp_alloc_requests_tx(dev))
		return -1;

	if (mtp_alloc_requests_rx(dev))
		goto fail_rx;

	if (mtp_alloc_requests_intr(dev))
		goto fail_intr;

	pr_info("[MTP]%s: -\n", __func__);
	return 0;

fail_intr:
	mtp_free_requests_rx(dev);
fail_rx:
	mtp_free_requests_tx(dev);
	return -1;
}

static int mtp_create_bulk_endpoints(struct mtp_dev *dev,
				struct usb_endpoint_descriptor *in_desc,
				struct usb_endpoint_descriptor *out_desc,
				struct usb_endpoint_descriptor *intr_desc)
{
	int ret;
	struct usb_ep *ep = NULL;
	struct usb_composite_dev *cdev = dev->cdev;

	DBG(cdev, "create_bulk_endpoints dev: %pK\n", dev);

	ep = usb_ep_autoconfig(cdev->gadget, in_desc);
	if (ep == NULL) {
		DBG(cdev, "usb_ep_autoconfig for ep_in failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for ep_in got %s\n", ep->name);
	ep->driver_data = dev; /* claim the endpoint */
	dev->ep_in = ep;

	ep = usb_ep_autoconfig(cdev->gadget, out_desc);
	if (ep == NULL) {
		DBG(cdev, "usb_ep_autoconfig for ep_out failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for mtp ep_out got %s\n", ep->name);
	ep->driver_data = dev; /* claim the endpoint */
	dev->ep_out = ep;

	ep = usb_ep_autoconfig(cdev->gadget, intr_desc);
	if (ep == NULL) {
		DBG(cdev, "usb_ep_autoconfig for ep_intr failed\n");
		return -ENODEV;
	}
	DBG(cdev, "usb_ep_autoconfig for mtp ep_intr got %s\n", ep->name);
	ep->driver_data = dev; /* claim the endpoint */
	dev->ep_intr = ep;

	/* now allocate requests for our endpoints */
	ret = mtp_alloc_requests(dev);
	if (ret)
		pr_err("mtp_bind() could not allocate requests\n");
	return ret;
}

static int mtp_queue_wait_req(struct mtp_dev *dev, struct usb_request *req)
{
	int ret;
	struct usb_composite_dev *cdev = dev->cdev;

	ret = usb_ep_queue(dev->ep_out, req, GFP_KERNEL);
	if (ret < 0)
		return -EIO;
	else
		DBG(cdev, "rx %pK queue\n", req);

	/* wait for a request to complete */
	ret = wait_event_interruptible(dev->read_wq, dev->rx_done || dev->state != STATE_BUSY);
	if (dev->state == STATE_CANCELED) {
		if (!dev->rx_done)
			usb_ep_dequeue(dev->ep_out, req);
		spin_lock_irq(&dev->lock);
		dev->state = STATE_CANCELED;
		spin_unlock_irq(&dev->lock);
		return -ECANCELED;
	}
	if (ret < 0)
		usb_ep_dequeue(dev->ep_out, req);
	return ret;
}

static ssize_t mtp_read_data(struct mtp_dev *dev, char __user *buf, size_t count, size_t len)
{
	int ret;
	ssize_t r;
	unsigned int xfer;
	struct usb_request *req = NULL;
	struct usb_composite_dev *cdev = dev->cdev;

	mutex_lock(&dev->req_mutex);
requeue_req:
	/* queue a request */
	req = dev->rx_req[0];
	if (!req) {
		pr_err("[MTP]%s:rx_req has been freed return ENODEV\n", __func__);
		r = -ENODEV;
		goto exit;
	}
	req->length = len;
	dev->rx_done = 0;
	ret = mtp_queue_wait_req(dev, req);
	if (ret) {
		r = ret;
		goto exit;
	}

	if (dev->state == STATE_BUSY) {
		/* If we got a 0-len packet, throw it back and try again. */
		if (req->actual == 0)
			goto requeue_req;

		DBG(cdev, "rx %pK %u\n", req, req->actual);
		xfer = (req->actual < count) ? req->actual : count;
		r = xfer;
		if (copy_to_user(buf, req->buf, xfer))
			r = -EFAULT;
	} else {
		r = -EIO;
	}

exit:
	mutex_unlock(&dev->req_mutex);
	return r;
}

static ssize_t mtp_read(struct file *fp, char __user *buf, size_t count, loff_t *pos)
{
	ssize_t r;
	int ret;
	size_t len;
	struct mtp_dev *dev = fp->private_data;
	struct usb_composite_dev *cdev = dev->cdev;

	DBG(cdev, "%s %zu\n", __func__, count);

	if ((buf == NULL) || (count > dev->bulk_buffer_size))
		return -EINVAL;

	/* we will block until we're online */
	DBG(cdev, "%s: waiting for online state\n", __func__);
	ret = wait_event_interruptible(dev->read_wq, dev->state != STATE_OFFLINE);
	if (ret < 0) {
		r = ret;
		goto done;
	}

	spin_lock_irq(&dev->lock);
	if (dev->ep_out->desc == NULL) {
		r = -EINVAL;
		goto err_exit;
	}

	len = usb_ep_align_maybe(cdev->gadget, dev->ep_out, count);
	if (len > dev->bulk_buffer_size) {
		r = -EINVAL;
		goto err_exit;
	}

	if (dev->state == STATE_CANCELED) {
		/* report cancellation to userspace */
		dev->state = STATE_READY;
		r = -ECANCELED;
		goto err_exit;
	}

	dev->state = STATE_BUSY;
	spin_unlock_irq(&dev->lock);
	r = mtp_read_data(dev, buf, count, len);

done:
	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED) {
		r = -ECANCELED;
	} else if (dev->state != STATE_OFFLINE) {
		dev->state = STATE_READY;
	} else {
		pr_info("[MTP]%s: STATE_OFFLINE return -ENODEV\n", __func__);
		r = -ENODEV;
	}

err_exit:
	spin_unlock_irq(&dev->lock);
	DBG(cdev, "%s returning %zd\n", __func__, ret);
	return r;
}

static int mtp_write_data(struct mtp_dev *dev, const char __user *buf, size_t count)
{
	unsigned int xfer;
	int send_zlp = 0;
	int ret = 0;
	struct usb_request *req = NULL;
	struct usb_composite_dev *cdev = dev->cdev;
	/*
	 * we need to send a zero length packet to signal the end of transfer
	 * if the transfer size is aligned to a packet boundary.
	 */
	if ((count & (dev->ep_in->maxpacket - 1)) == 0)
		send_zlp = 1;

	while (count > 0 || send_zlp) {
		/* so we exit after sending ZLP */
		if (count == 0)
			send_zlp = 0;

		if (dev->state != STATE_BUSY) {
			DBG(cdev, "%s dev->error\n", __func__);
			ret = -EIO;
			break;
		}

		/* get an idle tx request to use */
		req = NULL;
		ret = wait_event_interruptible(dev->write_wq,
				((req = mtp_req_get(dev, &dev->tx_idle))
				 || dev->state != STATE_BUSY));
		if (ret < 0)
			break;

		if (req == NULL)
			break;

		if (count > dev->bulk_buffer_size)
			xfer = dev->bulk_buffer_size;
		else
			xfer = count;

		if (xfer && copy_from_user(req->buf, buf, xfer)) {
			ret = -EFAULT;
			break;
		}

		req->length = xfer;
		ret = usb_ep_queue(dev->ep_in, req, GFP_KERNEL);
		if (ret < 0) {
			DBG(cdev, "%s: xfer error %d\n", __func__, ret);
			ret = -EIO;
			break;
		}

		buf += xfer;
		count -= xfer;

		/* zero this so we don't try to free it on error exit */
		req = NULL;
	}

	if (req != NULL)
		mtp_req_put(dev, &dev->tx_idle, req);

	return ret;
}

static ssize_t mtp_write(struct file *fp, const char __user *buf, size_t count, loff_t *pos)
{
	ssize_t r = count;
	int ret;
	struct mtp_dev *dev = fp->private_data;
	struct usb_composite_dev *cdev = dev->cdev;

	DBG(cdev, "%s(%zu)\n", __func__, count);

	if (buf == NULL)
		return -EINVAL;

	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED) {
		/* report cancellation to userspace */
		dev->state = STATE_READY;
		spin_unlock_irq(&dev->lock);
		return -ECANCELED;
	}
	if (dev->state == STATE_OFFLINE) {
		spin_unlock_irq(&dev->lock);
		return -ENODEV;
	}
	dev->state = STATE_BUSY;
	spin_unlock_irq(&dev->lock);

	ret = mtp_write_data(dev, buf, count);
	if (ret < 0)
		r  = ret;

	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED) {
		r = -ECANCELED;
	} else if (dev->state != STATE_OFFLINE) {
		dev->state = STATE_READY;
	} else {
		pr_info("[MTP]%s: STATE_OFFLINE return -ENODEV\n", __func__);
		r = -ENODEV;
	}
	spin_unlock_irq(&dev->lock);

	DBG(cdev, "%s returning %zd\n", __func__, r);
	return r;
}

static void mtp_prepend_header(struct mtp_dev *dev, char *buf, int64_t count)
{
	struct mtp_data_header *header = NULL;

	/* prepend MTP data header */
	header = (struct mtp_data_header *)buf;
	/*
	 * set file size with header according to
	 * MTP Specification v1.0
	 */
	header->length = (count > MTP_MAX_FILE_SIZE) ?
		MTP_MAX_FILE_SIZE : __cpu_to_le32(count);
	header->type = __cpu_to_le16(2); /* data packet */
	header->command = __cpu_to_le16(dev->xfer_command);
	header->transaction_id =
	    __cpu_to_le32(dev->xfer_transaction_id);
}

static int mtp_prepare_req_for_write(struct mtp_dev *dev, struct usb_request **req)
{
	int ret;
	struct usb_request *r = NULL;

	/* get an idle tx request to use */
	*req = NULL;
	ret = wait_event_interruptible(dev->write_wq,
			(r = mtp_req_get(dev, &dev->tx_idle)) ||
			dev->state != STATE_BUSY);
	if (dev->state == STATE_CANCELED)
		return -ECANCELED;

	if (ret < 0 || r == NULL)
		return ret;

	*req = r;
	return ret;
}

static int mtp_send_file_to_usb(struct mtp_dev *dev, struct file *filp,
			int64_t count, loff_t *offset, u32 hs)
{
	int ret;
	u32 xfer;
	int r = 0;
	int send_zlp = 0;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request *req = NULL;

	/*
	 * we need to send a zero length packet to signal the end of transfer
	 * if the transfer size is aligned to a packet boundary.
	 */
	if (((uint64_t)count & (dev->ep_in->maxpacket - 1)) == 0)
		send_zlp = 1;

	while (count > 0 || send_zlp) {
		/* so we exit after sending ZLP */
		if (count == 0)
			send_zlp = 0;

		/* get an idle tx request to use */
		ret = mtp_prepare_req_for_write(dev, &req);
		if (ret < 0 || req == NULL) {
			r = ret;
			break;
		}

		if (count > dev->bulk_buffer_size)
			xfer = dev->bulk_buffer_size;
		else
			xfer = count;

		if (hs)
			mtp_prepend_header(dev, req->buf, count);

		ret = vfs_read(filp, req->buf + hs, xfer - hs, offset);
		if (ret < 0) {
			r = ret;
			break;
		}

		xfer = ret + hs;
		hs = 0;

		req->length = xfer;
		ret = usb_ep_queue(dev->ep_in, req, GFP_KERNEL);
		if (ret < 0) {
			DBG(cdev, "%s: xfer error %d\n", __func__, ret);
			dev->state = STATE_ERROR;
			r = -EIO;
			break;
		}

		count -= xfer;
		/* zero this so we don't try to free it on error exit */
		req = NULL;
	}

	if (req != NULL)
		mtp_req_put(dev, &dev->tx_idle, req);

	return r;
}

/* read from a local file and write to USB */
static void send_file_work(struct work_struct *data)
{
	loff_t offset;
	int64_t count;
	int ret;
	u32 hdr_size;
	struct file *filp = NULL;
	struct mtp_dev *dev = container_of(data, struct mtp_dev, send_file_work);
	struct usb_composite_dev *cdev = dev->cdev;
	/* read our parameters */
	smp_rmb();

	filp = dev->xfer_file;
	offset = dev->xfer_file_offset;
	count = dev->xfer_file_length;

	if (count < 0) {
		dev->xfer_result = -EINVAL;
		return;
	}

	DBG(cdev, "%s(%lld %lld)\n", __func__, offset, count);

	if (dev->xfer_send_header) {
		hdr_size = sizeof(struct mtp_data_header);
		count += hdr_size;
	} else {
		hdr_size = 0;
	}

	ret = mtp_send_file_to_usb(dev, filp, count, &offset, hdr_size);

	DBG(cdev, "%s returning %d\n", __func__, ret);
	/* write the result */
	dev->xfer_result = ret;
	/* write the result completed */
	smp_wmb();
}

static int receive_file_from_usb(struct mtp_dev *dev, int64_t count,
			struct usb_request **read_req, int cur)
{
	int64_t len;
	int ret;
	struct usb_request *req = NULL;
	struct usb_composite_dev *cdev = dev->cdev;

	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_OFFLINE) {
		spin_unlock_irq(&dev->lock);
		return -EPERM;
	}

	len = usb_ep_align_maybe(cdev->gadget, dev->ep_out, count);
	spin_unlock_irq(&dev->lock);
	if (len > dev->bulk_buffer_size)
		len = dev->bulk_buffer_size;

	/* queue a request */
	req = dev->rx_req[cur];
	req->length = len;
	dev->rx_done = 0;
	ret = usb_ep_queue(dev->ep_out, req, GFP_KERNEL);
	if (ret < 0) {
		ret = -EIO;
		dev->state = STATE_ERROR;
	}

	*read_req = req;
	return ret;
}

static int mtp_receive_file_wait_req(struct mtp_dev *dev,
			struct usb_request *req)
{
	int ret;
	/* wait for our last read to complete */
	ret = wait_event_interruptible(dev->read_wq,
			dev->rx_done ||
			dev->state != STATE_BUSY);
	if (ret < 0)
		goto err_exit;

	if (dev->state == STATE_CANCELED) {
		ret = -ECANCELED;
		goto err_exit;
	}

	return req->status;

err_exit:
	if (!dev->rx_done)
		usb_ep_dequeue(dev->ep_out, req);

	return ret;
}

static int mtp_receive_file(struct mtp_dev *dev, struct file *filp,
			int64_t count, loff_t offset)
{
	loff_t os = offset;
	int ret = 0;
	int cur_buf = 0;
	struct usb_composite_dev *cdev = dev->cdev;
	struct usb_request *read_req = NULL;
	struct usb_request *write_req = NULL;

	while (count > 0 || write_req) {
		if (count > 0) {
			/* use from [1] to [MAX] req because mtp_read use [0] */
			if (cur_buf == 0)
				cur_buf = 1;
			ret = receive_file_from_usb(dev, count, &read_req, cur_buf);
			if (ret == -EPERM) {
				ret = 0;
				break;
			}
			if (ret < 0)
				break;

			cur_buf = (cur_buf + 1) % RX_REQ_MAX;
		}

		if (write_req != NULL) {
			DBG(cdev, "rx %pK %u\n", write_req, write_req->actual);
			ret = vfs_write(filp, write_req->buf, write_req->actual, &os);
			DBG(cdev, "vfs_write %d\n", ret);
			if (ret != write_req->actual) {
				ret = -EIO;
				dev->state = STATE_ERROR;
				break;
			}
			write_req = NULL;
		}

		if (read_req != NULL) {
			ret = mtp_receive_file_wait_req(dev, read_req);
			if (ret < 0)
				break;
			/*
			 * if xfer_file_length is 0xFFFFFFFF, then we read until
			 * we get a zero length packet
			 */
			if (count != 0xFFFFFFFF)
				count -= read_req->actual;
			if (read_req->actual < read_req->length) {
				/*
				 * short packet is used to signal EOF for
				 * sizes > 4 gig
				 */
				DBG(cdev, "got short packet\n");
				count = 0;
			}

			write_req = read_req;
			read_req = NULL;
		}
	}
	return ret;
}

/* read from USB and write to a local file */
static void receive_file_work(struct work_struct *data)
{
	loff_t offset;
	int64_t count;
	int ret;
	struct file *filp = NULL;
	struct mtp_dev *dev = container_of(data, struct mtp_dev, receive_file_work);
	struct usb_composite_dev *cdev = dev->cdev;

	/* read our parameters */
	smp_rmb();
	filp = dev->xfer_file;
	offset = dev->xfer_file_offset;
	count = dev->xfer_file_length;

	if (count < 0) {
		dev->xfer_result = -EINVAL;
		return;
	}

	DBG(cdev, "%s %lld\n", __func__, count);

	ret = mtp_receive_file(dev, filp, count, offset);

	DBG(cdev, "%s returning %d\n", __func__, ret);
	/* write the result */
	dev->xfer_result = ret;
	/* write the result completed */
	smp_wmb();
}

static int mtp_send_event(struct mtp_dev *dev, struct mtp_event *event)
{
	int ret;
	int length = event->length;
	struct usb_request *req = NULL;
	struct usb_composite_dev *cdev = dev->cdev;

	DBG(cdev, "%s %zu\n", __func__, event->length);

	if (event->data == NULL)
		return -EINVAL;

	if (length < 0 || length > INTR_BUFFER_SIZE)
		return -EINVAL;

	if (dev->state == STATE_OFFLINE)
		return -ENODEV;

	ret = wait_event_interruptible_timeout(dev->intr_wq,
			(req = mtp_req_get(dev, &dev->intr_idle)),
			msecs_to_jiffies(1000));
	if (ret < 0)
		return ret;

	if (req == NULL)
		return -ETIME;

	if (copy_from_user(req->buf, (void __user *)event->data, length)) {
		mtp_req_put(dev, &dev->intr_idle, req);
		return -EFAULT;
	}

	req->length = length;
	ret = usb_ep_queue(dev->ep_intr, req, GFP_KERNEL);
	if (ret)
		mtp_req_put(dev, &dev->intr_idle, req);

	return ret;
}

static int mtp_ioctl_device_state_check(struct mtp_dev *dev)
{
	int ret = 0;

	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED) {
		/* report cancellation to userspace */
		dev->state = STATE_READY;
		ret = -ECANCELED;
		goto exit;
	}

	if (dev->state == STATE_OFFLINE) {
		ret = -ENODEV;
		goto exit;
	}

	dev->state = STATE_BUSY;
exit:
	spin_unlock_irq(&dev->lock);
	return ret;
}

static int mtp_ioctl_send_file(struct mtp_dev *dev,
			const void __user *r_file, unsigned int code)
{
	struct mtp_file_range mfr;
	struct work_struct *work = NULL;
	struct file *filp = NULL;

	if (copy_from_user(&mfr, r_file, sizeof(mfr)))
		return -EFAULT;
	/*
	 * hold a reference to the file
	 * while we are working with it
	 */
	filp = fget(mfr.fd);
	if (filp == NULL)
		return -EBADF;

	/* write the parameters */
	dev->xfer_file = filp;
	dev->xfer_file_offset = mfr.offset;
	dev->xfer_file_length = mfr.length;
	dev->xfer_command = mfr.command;
	dev->xfer_transaction_id = mfr.transaction_id;

	/* write the parameters completed */
	smp_wmb();

	work = &dev->send_file_work;
	if (code == MTP_SEND_FILE_WITH_HEADER)
		dev->xfer_send_header = 1;
	else if (code == MTP_SEND_FILE)
		dev->xfer_send_header = 0;
	else
		work = &dev->receive_file_work;

	/*
	 * We do the file transfer on a work queue so it will run
	 * in kernel context, which is necessary for vfs_read and
	 * vfs_write to use our buffers in the kernel address space.
	 */
	queue_work(dev->wq, work);
	/* wait for operation to complete */
	flush_workqueue(dev->wq);
	fput(filp);

	/* read the result */
	smp_rmb();
	return 0;
}

static long mtp_ioctl(struct file *fp, unsigned int code, unsigned long value)
{
	int ret = -EINVAL;
	void __user *r_value = (void __user *)(uintptr_t) value;
	struct mtp_dev *dev = fp->private_data;
	struct usb_composite_dev *cdev = dev->cdev;

	if (r_value == NULL)
		return ret;

	if (mtp_lock(&dev->ioctl_excl))
		return -EBUSY;

	switch (code) {
	case MTP_SEND_FILE:
	case MTP_RECEIVE_FILE:
	case MTP_SEND_FILE_WITH_HEADER:
		{
			ret = mtp_ioctl_device_state_check(dev);
			if (ret < 0)
				goto out;

			ret = mtp_ioctl_send_file(dev, r_value, code);
			if (ret < 0)
				goto fail;

			ret = dev->xfer_result;
			break;
		}
	case MTP_SEND_EVENT:
		{
			struct mtp_event event;
			/*
			 * return here so we don't change dev->state below,
			 * which would interfere with bulk transfer state.
			 */
			if (copy_from_user(&event, r_value, sizeof(event)))
				ret = -EFAULT;
			else
				ret = mtp_send_event(dev, &event);
			goto out;
		}
	default:
		break;
	}

fail:
	spin_lock_irq(&dev->lock);
	if (dev->state == STATE_CANCELED)
		ret = -ECANCELED;
	else if (dev->state != STATE_OFFLINE)
		dev->state = STATE_READY;
	spin_unlock_irq(&dev->lock);
out:
	mtp_unlock(&dev->ioctl_excl);
	DBG(cdev, "ioctl returning %d\n", ret);
	return ret;
}

static int mtp_open(struct inode *ip, struct file *fp)
{
	pr_info("%s\n", __func__);
	if (mtp_lock(&_mtp_dev->open_excl))
		return -EBUSY;

	/* clear any error condition */
	if (_mtp_dev->state != STATE_OFFLINE)
		_mtp_dev->state = STATE_READY;

	fp->private_data = _mtp_dev;
	return 0;
}

static int mtp_release(struct inode *ip, struct file *fp)
{
	pr_info("%s\n", __func__);

	mtp_unlock(&_mtp_dev->open_excl);
	wake_up(&_mtp_dev->open_wq);
	return 0;
}

/* file operations for /dev/mtp_usb */
static const struct file_operations mtp_fops = {
	.owner = THIS_MODULE,
	.read = mtp_read,
	.write = mtp_write,
	.unlocked_ioctl = mtp_ioctl,
	.open = mtp_open,
	.release = mtp_release,
};

static struct miscdevice mtp_device = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = mtp_shortname,
	.fops = &mtp_fops,
};

static int mtp_class_reqest(struct mtp_dev *dev,
	struct usb_composite_dev *cdev, const struct usb_ctrlrequest *ctrl)
{
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);
	unsigned long flags;
	int value = -EOPNOTSUPP;

	if (ctrl->bRequest == MTP_REQ_CANCEL && w_index == 0 && w_value == 0) {
		DBG(cdev, "MTP_REQ_CANCEL\n");

		spin_lock_irqsave(&dev->lock, flags);
		if (dev->state == STATE_BUSY) {
			wake_up(&dev->read_wq);
			wake_up(&dev->write_wq);
		}
		dev->state = STATE_CANCELED;
		spin_unlock_irqrestore(&dev->lock, flags);

		/*
		 * We need to queue a request to read the remaining
		 * bytes, but we don't actually need to look at
		 * the contents.
		 */
		value = w_length;
	} else if (ctrl->bRequest == MTP_REQ_GET_DEVICE_STATUS
		   && w_index == 0 && w_value == 0) {
		struct mtp_device_status *status = cdev->req->buf;

		status->wLength =
		    __cpu_to_le16(sizeof(*status));

		DBG(cdev, "MTP_REQ_GET_DEVICE_STATUS\n");
		spin_lock_irqsave(&dev->lock, flags);
		/*
		 * device status is "busy" until we report
		 * the cancellation to userspace
		 */
		if (dev->state == STATE_CANCELED)
			status->wCode =
			    __cpu_to_le16(MTP_RESPONSE_DEVICE_BUSY);
		else
			status->wCode = __cpu_to_le16(MTP_RESPONSE_OK);
		spin_unlock_irqrestore(&dev->lock, flags);
		value = sizeof(*status);
	}

	return value;
}

static int mtp_ctrlrequest(struct usb_composite_dev *cdev, const struct usb_ctrlrequest *ctrl)
{
	int value = -EOPNOTSUPP;
	u16 w_index = le16_to_cpu(ctrl->wIndex);
	u16 w_value = le16_to_cpu(ctrl->wValue);
	u16 w_length = le16_to_cpu(ctrl->wLength);
	errno_t ret;
	struct mtp_dev *dev = _mtp_dev;

	if (dev == NULL)
		return value;

	VDBG(cdev, "%s %02x.%02x v%04x i%04x l%u\n",
		__func__, ctrl->bRequestType, ctrl->bRequest, w_value, w_index, w_length);

	/* Handle MTP OS string */
	if (ctrl->bRequestType ==
	    (USB_DIR_IN | USB_TYPE_STANDARD | USB_RECIP_DEVICE) &&
	    ctrl->bRequest == USB_REQ_GET_DESCRIPTOR  &&
	    (w_value >> 8) == USB_DT_STRING &&
	    (w_value & 0xFF) == MTP_OS_STRING_ID) {
		value = ((w_length < sizeof(mtp_os_string)) ?
				w_length : sizeof(mtp_os_string));
		ret = memcpy_s(cdev->req->buf, USB_COMP_EP0_BUFSIZ, mtp_os_string, value);
		if (ret != EOK) {
			ERROR(cdev, "%s: memcpy_s failed\n", __func__);
			return -EOPNOTSUPP;
		}
	} else if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_VENDOR) {
		/* Handle MTP OS descriptor */
		DBG(cdev, "vendor request: %d index: %d value: %d length: %d\n",
			ctrl->bRequest, w_index, w_value, w_length);
		/* follow spec */
		if (ctrl->bRequest == 1 && (ctrl->bRequestType & USB_DIR_IN)
		    && (w_index == 4 || w_index == 5)) {
			value = ((w_length < sizeof(mtp_ext_config_desc)) ?
				 w_length : sizeof(mtp_ext_config_desc));
			ret = memcpy_s(cdev->req->buf, USB_COMP_EP0_BUFSIZ, &mtp_ext_config_desc, value);
			if (ret != EOK) {
				ERROR(cdev, "%s: memcpy_s failed\n", __func__);
				return -EOPNOTSUPP;
			}
		}
	} else if ((ctrl->bRequestType & USB_TYPE_MASK) == USB_TYPE_CLASS) {
		DBG(cdev, "class request: %d index: %d value: %d length: %d\n",
		    ctrl->bRequest, w_index, w_value, w_length);
		value = mtp_class_reqest(dev, cdev, ctrl);
	}

	/* respond with data transfer or status phase? */
	if (value >= 0) {
		cdev->req->zero = value < w_length;
		cdev->req->length = value;
		if (usb_ep_queue(cdev->gadget->ep0, cdev->req, GFP_ATOMIC) < 0)
			ERROR(cdev, "%s: response queue error\n", __func__);
	}
	return value;
}

static int mtp_interface_id(struct usb_configuration *c, struct usb_function *f)
{
	int id;
	int ret;

	/* allocate interface ID(s) */
	id = usb_interface_id(c, f);
	if (id < 0)
		return id;
	mtp_interface_desc.bInterfaceNumber = id;

	if (mtp_string_defs[INTERFACE_STRING_INDEX].id == 0) {
		ret = usb_string_id(c->cdev);
		if (ret < 0)
			return ret;
		mtp_string_defs[INTERFACE_STRING_INDEX].id = ret;
		mtp_interface_desc.iInterface = ret;
	}
	return 0;
}

static int mtp_fill_os_desc_table(struct usb_configuration *c, struct usb_function *f)
{
	struct usb_composite_dev *cdev = c->cdev;
	struct mtp_instance *fi_mtp = NULL;

	fi_mtp = container_of(f->fi, struct mtp_instance, func_inst);

	if (cdev->use_os_string) {
		f->os_desc_table = kzalloc(sizeof(*f->os_desc_table),
					GFP_KERNEL);
		if (f->os_desc_table == NULL)
			return -ENOMEM;
		f->os_desc_n = 1;
		f->os_desc_table[0].os_desc = &fi_mtp->mtp_os_desc;
	}
	return 0;
}

static void mpt_support_hs_ss(struct usb_configuration *c, struct usb_function *f)
{
	struct mtp_dev *dev = func_to_mtp(f);
	/* support high speed hardware */
	if (gadget_is_dualspeed(c->cdev->gadget)) {
		mtp_highspeed_in_desc.bEndpointAddress =
			mtp_fullspeed_in_desc.bEndpointAddress;
		mtp_highspeed_out_desc.bEndpointAddress =
			mtp_fullspeed_out_desc.bEndpointAddress;
	}
	/* support super speed hardware */
	if (gadget_is_superspeed(c->cdev->gadget)
			|| gadget_is_superspeed_plus(c->cdev->gadget)) {
		unsigned int max_burst;

		/* Calculate bMaxBurst, we know packet size is 1024 */
		max_burst = min_t(unsigned int, dev->bulk_buffer_size / 1024, 15);
		mtp_ss_in_desc.bEndpointAddress =
			mtp_fullspeed_in_desc.bEndpointAddress;
		mtp_ss_in_comp_desc.bMaxBurst = max_burst;
		mtp_ss_out_desc.bEndpointAddress =
			mtp_fullspeed_out_desc.bEndpointAddress;
		mtp_ss_out_comp_desc.bMaxBurst = max_burst;
	}
}

static int mtp_function_bind(struct usb_configuration *c, struct usb_function *f)
{
	int ret;
	struct usb_composite_dev *cdev = c->cdev;
	struct mtp_dev *dev = func_to_mtp(f);

	dev->cdev = cdev;
	INFO(cdev, "%s dev: %pk\n", __func__, dev);
	dev->bulk_buffer_size = MTP_BULK_BUFFER_SIZE;

	ret = mtp_interface_id(c, f);
	if (ret)
		return ret;

	ret = mtp_fill_os_desc_table(c, f);
	if (ret)
		return ret;

	/* allocate endpoints */
	ret = mtp_create_bulk_endpoints(dev, &mtp_fullspeed_in_desc,
					&mtp_fullspeed_out_desc,
					&mtp_intr_desc);
	if (ret) {
		if (cdev->use_os_string) {
			kfree(f->os_desc_table);
			f->os_desc_table = NULL;
		}
		return ret;
	}

	mpt_support_hs_ss(c, f);

	DBG(cdev, "%s speed %s: IN/%s, OUT/%s bulk_buffer_size/0x%x\n",
		gadget_is_superspeed_plus(c->cdev->gadget) ? "super-plus" :
		gadget_is_superspeed(c->cdev->gadget) ? "super" :
		(gadget_is_dualspeed(c->cdev->gadget) ? "dual" : "full"),
		f->name, dev->ep_in->name, dev->ep_out->name,
		dev->bulk_buffer_size);
	return 0;
}

static void mtp_function_unbind(struct usb_configuration *c, struct usb_function *f)
{
	int ret;
	int i;
	struct usb_request *req = NULL;
	struct mtp_dev *dev = func_to_mtp(f);

	pr_info("[MTP]%s: +\n", __func__);

	/* flush wq before unbind */
	flush_workqueue(dev->wq);

	mtp_string_defs[INTERFACE_STRING_INDEX].id = 0;

	for (req = mtp_req_get(dev, &dev->tx_idle); req != NULL;
			req = mtp_req_get(dev, &dev->tx_idle))
		mtp_request_free(req, dev->ep_in);

	mutex_lock(&dev->req_mutex);
	for (i = 0; i < RX_REQ_MAX; i++) {
		mtp_request_free(dev->rx_req[i], dev->ep_out);
		dev->rx_req[i] = NULL;
	}
	mutex_unlock(&dev->req_mutex);

	for (req = mtp_req_get(dev, &dev->intr_idle); req != NULL;
			req = mtp_req_get(dev, &dev->intr_idle))
		mtp_request_free(req, dev->ep_intr);

	dev->state = STATE_OFFLINE;
	kfree(f->os_desc_table);
	f->os_desc_table = NULL;
	f->os_desc_n = 0;

	ret = wait_event_interruptible_timeout(dev->open_wq,
			(atomic_read(&dev->open_excl) == 0),
			msecs_to_jiffies(1000));
	if (ret <= 0)
		pr_warn("[MTP]%s: wait for open_excl cleared timeout\n", __func__);

	pr_info("[MTP]%s: -\n", __func__);
}

static int mtp_function_set_alt(struct usb_function *f, unsigned int intf, unsigned int alt)
{
	int ret;
	struct mtp_dev *dev = func_to_mtp(f);
	struct usb_composite_dev *cdev = f->config->cdev;

	DBG(cdev, "%s intf: %d alt: %d\n", __func__, intf, alt);

	ret = config_ep_by_speed(cdev->gadget, f, dev->ep_in);
	if (ret)
		return ret;

	ret = usb_ep_enable(dev->ep_in);
	if (ret)
		return ret;

	ret = config_ep_by_speed(cdev->gadget, f, dev->ep_out);
	if (ret)
		return ret;

	ret = usb_ep_enable(dev->ep_out);
	if (ret) {
		usb_ep_disable(dev->ep_in);
		return ret;
	}

	ret = config_ep_by_speed(cdev->gadget, f, dev->ep_intr);
	if (ret)
		return ret;

	ret = usb_ep_enable(dev->ep_intr);
	if (ret) {
		usb_ep_disable(dev->ep_out);
		usb_ep_disable(dev->ep_in);
		return ret;
	}
	dev->state = STATE_READY;

	/* readers may be blocked waiting for us to go online */
	wake_up(&dev->read_wq);
	return 0;
}

static void mtp_function_disable(struct usb_function *f)
{
	unsigned long flags;
	struct mtp_dev *dev = func_to_mtp(f);
	struct usb_composite_dev *cdev = dev->cdev;

	INFO(cdev, "%s\n", __func__);

	spin_lock_irqsave(&dev->lock, flags);
	dev->state = STATE_OFFLINE;
	spin_unlock_irqrestore(&dev->lock, flags);

	usb_ep_disable(dev->ep_in);
	usb_ep_disable(dev->ep_out);
	usb_ep_disable(dev->ep_intr);

	/* readers may be blocked waiting for us to go online */
	wake_up(&dev->read_wq);

	INFO(cdev, "%s disabled\n", dev->function.name);
}

static int __mtp_setup(struct mtp_instance *fi_mtp)
{
	int ret;
	struct mtp_dev *dev = NULL;

	dev = kzalloc(sizeof(*dev), GFP_KERNEL);

	if (fi_mtp != NULL)
		fi_mtp->dev = dev;

	if (dev == NULL)
		return -ENOMEM;

	spin_lock_init(&dev->lock);
	init_waitqueue_head(&dev->read_wq);
	init_waitqueue_head(&dev->write_wq);
	init_waitqueue_head(&dev->intr_wq);
	init_waitqueue_head(&dev->open_wq);
	mutex_init(&dev->req_mutex);
	atomic_set(&dev->open_excl, 0);
	atomic_set(&dev->ioctl_excl, 0);
	INIT_LIST_HEAD(&dev->tx_idle);
	INIT_LIST_HEAD(&dev->intr_idle);

	dev->wq = create_singlethread_workqueue("f_mtp");
	if (dev->wq == NULL) {
		ret = -ENOMEM;
		goto err_create_workqueue;
	}
	INIT_WORK(&dev->send_file_work, send_file_work);
	INIT_WORK(&dev->receive_file_work, receive_file_work);

	_mtp_dev = dev;

	ret = misc_register(&mtp_device);
	if (ret)
		goto err_misc_register;

	return 0;

err_misc_register:
	destroy_workqueue(dev->wq);

err_create_workqueue:
	_mtp_dev = NULL;
	kfree(dev);
	fi_mtp->dev = NULL;
	pr_err("mtp gadget driver failed to initialize\n");
	return ret;
}

static int mtp_setup_configfs(struct mtp_instance *fi_mtp)
{
	return __mtp_setup(fi_mtp);
}

static void mtp_cleanup(void)
{
	struct mtp_dev *dev = _mtp_dev;

	if (dev == NULL)
		return;

	misc_deregister(&mtp_device);
	destroy_workqueue(dev->wq);
	_mtp_dev = NULL;
	kfree(dev);
}

static struct mtp_instance *to_mtp_instance(struct config_item *item)
{
	return container_of(to_config_group(item), struct mtp_instance, func_inst.group);
}

static void mtp_attr_release(struct config_item *item)
{
	struct mtp_instance *fi_mtp = to_mtp_instance(item);

	usb_put_function_instance(&fi_mtp->func_inst);
}

static struct configfs_item_operations mtp_item_ops = {
	.release = mtp_attr_release,
};

static struct config_item_type mtp_func_type = {
	.ct_item_ops = &mtp_item_ops,
	.ct_owner = THIS_MODULE,
};

static struct mtp_instance *to_fi_mtp(struct usb_function_instance *fi)
{
	return container_of(fi, struct mtp_instance, func_inst);
}

static int mtp_set_inst_name(struct usb_function_instance *fi, const char *name)
{
	int name_len;
	struct mtp_instance *fi_mtp = NULL;
	char *ptr = NULL;

	name_len = strlen(name) + 1;
	if (name_len > MAX_INST_NAME_LEN)
		return -ENAMETOOLONG;

	ptr = kstrndup(name, name_len, GFP_KERNEL);
	if (ptr == NULL)
		return -ENOMEM;

	fi_mtp = to_fi_mtp(fi);
	fi_mtp->name = ptr;

	return 0;
}

static void mtp_free_inst(struct usb_function_instance *fi)
{
	struct mtp_instance *fi_mtp;

	fi_mtp = to_fi_mtp(fi);
	kfree(fi_mtp->name);
	mtp_cleanup();
	kfree(fi_mtp);
}

struct usb_function_instance *alloc_inst_mtp_ptp(bool mtp_config)
{
	int ret;
	struct mtp_instance *fi_mtp = NULL;
	struct usb_os_desc *descs[1] = {NULL};
	char *names[1] = {NULL};

	fi_mtp = kzalloc(sizeof(*fi_mtp), GFP_KERNEL);
	if (fi_mtp == NULL)
		return ERR_PTR(-ENOMEM);

	fi_mtp->func_inst.set_inst_name = mtp_set_inst_name;
	fi_mtp->func_inst.free_func_inst = mtp_free_inst;

	fi_mtp->mtp_os_desc.ext_compat_id = fi_mtp->mtp_ext_compat_id;
	INIT_LIST_HEAD(&fi_mtp->mtp_os_desc.ext_prop);
	descs[0] = &fi_mtp->mtp_os_desc;
	names[0] = "MTP";

	if (mtp_config) {
		ret = mtp_setup_configfs(fi_mtp);
		if (ret) {
			kfree(fi_mtp);
			pr_err("Error setting MTP\n");
			return ERR_PTR(ret);
		}
	} else {
		fi_mtp->dev = _mtp_dev;
	}

	config_group_init_type_name(&fi_mtp->func_inst.group, "", &mtp_func_type);
	usb_os_desc_prepare_interf_dir(&fi_mtp->func_inst.group, 1, descs, names, THIS_MODULE);

	return &fi_mtp->func_inst;
}
EXPORT_SYMBOL_GPL(alloc_inst_mtp_ptp);

static struct usb_function_instance *mtp_alloc_inst(void)
{
	return alloc_inst_mtp_ptp(true);
}

static int mtp_ctrlreq_configfs(struct usb_function *f,
			const struct usb_ctrlrequest *ctrl)
{
	return mtp_ctrlrequest(f->config->cdev, ctrl);
}

static void mtp_free(struct usb_function *f)
{
	/* NO-OP: no function specific resource allocation in mtp_alloc */
}

struct usb_function *function_alloc_mtp_ptp(struct usb_function_instance *fi, bool mtp_config)
{
	struct mtp_instance *fi_mtp = to_fi_mtp(fi);
	struct mtp_dev *dev = NULL;

	/*
	 * PTP piggybacks on MTP function so make sure we have
	 * created MTP function before we associate this PTP
	 * function with a gadget configuration.
	 */
	if (fi_mtp->dev == NULL) {
		pr_err("Error: Create MTP function before linking PTP function with a gadget configuration\n");
		pr_err("\t1: Delete existing PTP function if any\n");
		pr_err("\t2: Create MTP function\n");
		pr_err("\t3: Create and symlink PTP function with a gadget configuration\n");
		return ERR_PTR(-EINVAL); /* Invalid Configuration */
	}

	dev = fi_mtp->dev;
	dev->function.name = DRIVER_NAME;
	dev->function.strings = mtp_strings;

	if (mtp_config) {
		dev->function.fs_descriptors = fs_mtp_descs;
		dev->function.hs_descriptors = hs_mtp_descs;
		dev->function.ss_descriptors = ss_mtp_descs;
		dev->function.ssp_descriptors = ss_mtp_descs;
	} else {
		dev->function.fs_descriptors = fs_ptp_descs;
		dev->function.hs_descriptors = hs_ptp_descs;
		dev->function.ss_descriptors = ss_ptp_descs;
		dev->function.ssp_descriptors = ss_ptp_descs;
	}

	dev->function.bind = mtp_function_bind;
	dev->function.unbind = mtp_function_unbind;
	dev->function.set_alt = mtp_function_set_alt;
	dev->function.disable = mtp_function_disable;
	dev->function.setup = mtp_ctrlreq_configfs;
	dev->function.free_func = mtp_free;

	return &dev->function;
}
EXPORT_SYMBOL_GPL(function_alloc_mtp_ptp);

static struct usb_function *mtp_alloc(struct usb_function_instance *fi)
{
	return function_alloc_mtp_ptp(fi, true);
}

DECLARE_USB_FUNCTION_INIT(mtp, mtp_alloc_inst, mtp_alloc);
MODULE_LICENSE("GPL");
