/*
 * dfu.c
 *
 * driver for dfu
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#include <uapi/linux/dfu.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/err.h>
#include <linux/types.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/firmware.h>
#include <linux/usb.h>
#include <linux/mutex.h>

/* dfu class requests */
#define	USB_REQ_DFU_DETACH		0
#define	USB_REQ_DFU_DNLOAD		1
#define	USB_REQ_DFU_UPLOAD		2
#define	USB_REQ_DFU_GETSTATUS		3
#define	USB_REQ_DFU_CLRSTATUS		4
#define	USB_REQ_DFU_GETSTATE		5
#define	USB_REQ_DFU_ABORT		6

/* dfu status */
#define	DFU_STATUS_OK				0x00

/* dfu state */
#define	DFU_STATE_LEN				1
#define	DFU_STATE_DFU_IDLE			2
#define	DFU_STATE_DFU_DNLOAD_IDLE		5
#define	DFU_STATE_DFU_UPLOAD_IDLE		9

/* dfu function descriptor code */
#define	DFU_DT_FUNC			(USB_TYPE_CLASS | 0x01)

struct dfu_descriptor {
	__u8  bLength;
	__u8  bDescriptorType;
	__u8  bmAttributes;
#define BIT_WILL_DETACH			(1 << 3)
#define BIT_MANIFESTATION_TOLERANT	(1 << 2)
#define BIT_CAN_UPLOAD			(1 << 1)
#define BIT_CAN_DNLOAD			(1 << 0)
	__le16  wDetachTimeOut;
	__le16  wTransferSize;
	__le16  bcdDFUVersion;
} __attribute__ ((packed));

#define miscdev_to_dfu(__misc) (container_of(__misc, struct usb_dfu, dfu_dev))

/* check idle state */
#define CHECK_IDLE		1
#define CHECK_DNLOAD_IDLE	2
#define CHECK_UPLOAD_IDLE	4

enum {
	RUNTIME_DUF_PROCOTOL = 1,
	DUF_MODE_PROCOTOL = 2,
};

#define DFU_CONTROL_MAXPACKET 512
struct usb_dfu {
	struct miscdevice dfu_dev;
	struct usb_interface *usb_intf;
	struct usb_device *usb_dev;

	int proctol; /* 0: runtime mode, 1: dfu mode */
	int will_detach;
	int manifestation_tolerant;
	int can_upload;
	int can_dnload;
	int dev_open;
	int dev_exist;

	unsigned int transfer_size;
	unsigned int detach_timeout;

	void *load_buf;
	u8 control_data[DFU_CONTROL_MAXPACKET];
};

static DEFINE_MUTEX(dfu_access_lock);

static void dfu_reset(struct usb_dfu *dfu)
{
	usb_queue_reset_device(dfu->usb_intf);
}

static int dfu_detach(struct usb_dfu *dfu)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);
	struct usb_host_interface *interface = dfu->usb_intf->cur_altsetting;

	if (!dfu->will_detach) {
		dev_info(&usb_dev->dev, "device not support detach\n");
		return 0;
	}

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_DETACH,
		USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		5000, /* wValue : dfu detach timeout */
		interface->desc.bInterfaceNumber, /* interface */
		NULL, 0, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB send error: %zd\n",
			__func__, result);
		return result;
	}

	msleep(200); /* default wait time */

	dfu_reset(dfu);

	return 0;
}

static int dfu_getstatus(struct usb_dfu *dfu, struct dfu_status *status)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);
	struct usb_interface *usb_intf = dfu->usb_intf;

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_GETSTATUS,
		USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, usb_intf->cur_altsetting->desc.bInterfaceNumber,
		&dfu->control_data, sizeof(*status), 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB receive error: %zd\n",
			__func__, result);
		return result;
	}

	if (result == sizeof(*status))
		memcpy(status, &dfu->control_data, sizeof(*status));

	return result;
}


static int dfu_clearstatus(struct usb_dfu *dfu)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);
	struct usb_interface *usb_intf = dfu->usb_intf;

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_CLRSTATUS,
		USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, usb_intf->cur_altsetting->desc.bInterfaceNumber,
		NULL, 0, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB receive error: %zd\n",
			__func__, result);
		return result;
	}

	return result;
}

static int dfu_abort(struct usb_dfu *dfu)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);
	struct usb_interface *usb_intf = dfu->usb_intf;

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_ABORT,
		USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, usb_intf->cur_altsetting->desc.bInterfaceNumber,
		NULL, 0, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB receive error: %zd\n",
			__func__, result);
		return result;
	}

	return result;
}

#ifdef SUPPORT_DFU_GETSTATE
static int dfu_getstate(struct usb_dfu *dfu, u8 *state)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);
	struct usb_interface *usb_intf = dfu->usb_intf;

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_GETSTATE,
		USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		0, usb_intf->cur_altsetting->desc.bInterfaceNumber,
		&dfu->control_data, DFU_STATE_LEN, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB receive error: %zd\n",
			__func__, result);
		return result;
	}

	if (result == DFU_STATE_LEN)
		*state = dfu->control_data[0];

	return result;
}
#endif

static ssize_t dfu_download(struct usb_dfu *dfu, void *data,
	size_t size, int blocknum)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);

	if (size > dfu->transfer_size) {
		dev_err(&usb_dev->dev, "invalid size %u\n", (unsigned int)size);
		return -EINVAL;
	}

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_DNLOAD,
		USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		blocknum, /* block num */
		dfu->usb_intf->cur_altsetting->desc.bInterfaceNumber,
		data, size, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB send error: %zd\n",
			__func__, result);
		return result;
	}

	return result;
}

static ssize_t dfu_upload(struct usb_dfu *dfu,
	void *data, size_t size, int blocknum)
{
	ssize_t result;
	struct usb_device *usb_dev = interface_to_usbdev(dfu->usb_intf);

	result = usb_control_msg(usb_dev, usb_rcvctrlpipe(usb_dev, 0),
		USB_REQ_DFU_UPLOAD,
		USB_DIR_IN | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
		blocknum, dfu->usb_intf->cur_altsetting->desc.bInterfaceNumber,
		data, size, 5000); /* 5000: timeout */
	if (result < 0) {
		dev_err(&dfu->usb_intf->dev, "%s: USB receive error: %zd\n",
			__func__, result);
		return result;
	}

	return result;
}

static int wait_for_state_idle(struct usb_dfu *dfu, unsigned int check_item)
{
	struct dfu_status status;
	int count = 3;
	int ret;

	while (count-- > 0) {
		msleep(40); /* default wait time */

		ret = dfu_getstatus(dfu, &status);
		if (ret < 0) {
			dev_err(&dfu->usb_dev->dev, "dfu_getstatus failed\n");
			return ret;
		}
		if (ret != sizeof(status)) {
			dev_err(&dfu->usb_dev->dev,
				"dfu_getstatus return len error\n");
			return -EIO;
		}

		if (status.bStatus != DFU_STATUS_OK) {
			ret = dfu_clearstatus(dfu);
			if (ret) {
				dev_err(&dfu->usb_dev->dev,
					"dfu_clearstatus failed\n");
				return ret;
			}
		}

		if ((check_item & CHECK_IDLE) &&
			status.bState == DFU_STATE_DFU_IDLE)
			return 0;

		if ((check_item & CHECK_DNLOAD_IDLE) &&
			status.bState == DFU_STATE_DFU_DNLOAD_IDLE)
			return 0;

		if ((check_item & CHECK_UPLOAD_IDLE) &&
			status.bState == DFU_STATE_DFU_UPLOAD_IDLE)
			return 0;
	}

	return -EBUSY;
}

static int __dfu_read_check_input(char __user *buf, size_t count)
{
	if (!buf) {
		pr_err("dfu_read buf invalid!\n");
		return -EINVAL;
	}

	if (count > DFU_MAX_READ_SIZE || count == 0) {
		pr_err("dfu_read count invalid!\n");
		return -EINVAL;
	}
	return 0;
}

static ssize_t dfu_read(struct file *file, char __user *buf,
	size_t count, loff_t *pos)
{
	struct miscdevice *miscdev = file->private_data;
	struct usb_dfu *dfu = miscdev_to_dfu(miscdev);
	size_t left = count;
	int ret;
	int blocknum = 1;

	if (__dfu_read_check_input(buf, count))
		return -EINVAL;

	mutex_lock(&dfu_access_lock);
	if (!dfu->dev_exist) {
		pr_err("%s when no dev!\n", __func__);
		mutex_unlock(&dfu_access_lock);
		return -ENODEV;
	}

	while (left > 0) {
		size_t len;
		size_t size = (left < dfu->transfer_size) ?
				left : dfu->transfer_size;

		ret = wait_for_state_idle(dfu, CHECK_IDLE | CHECK_UPLOAD_IDLE);
		if (ret) {
			pr_err("wait for dfu idle failed\n");
			goto err;
		}

		/* quirk: vr's dfu protocol require blocknum start from 2 */
		ret = dfu_upload(dfu, dfu->load_buf, size, blocknum + 1);
		if (ret < 0) {
			pr_err("dfu_upload failed\n");
			goto err;
		} else if (ret == 0) {
			/* upload done */
			break;
		}

		len = ret < size ? ret : size;
		if (copy_to_user(buf + count - left, dfu->load_buf, len)) {
			pr_err("copy to user failed\n");
			ret = -EFAULT;
			goto err;
		}

		left -= len;
		/* return len less than asked */
		if (len < size)
			break;

		blocknum++;
	}

	mutex_unlock(&dfu_access_lock);
	return count - left;

err:
	mutex_unlock(&dfu_access_lock);
	return ret;
}

static int dfu_update_firmware(struct usb_dfu *dfu, void *image, size_t size)
{
	void *buffer = image;
	size_t left = size;
	int blocknum = 1;
	int retry_count = 3;
	int ret;

	while (left > 0) {
		size_t xfer_size = (left < dfu->transfer_size) ?
					left : dfu->transfer_size;

		ret = wait_for_state_idle(dfu, CHECK_IDLE | CHECK_DNLOAD_IDLE);
		if (ret) {
			pr_err("wait for dfu idle failed\n");
			return ret;
		}

		memcpy(dfu->load_buf, buffer + size - left, xfer_size);

		/* quirk: vr's dfu protocol require blocknum start from 2 */
		ret = dfu_download(dfu, dfu->load_buf, xfer_size, blocknum + 1);
		if (ret < 0) {
			pr_err("dfu_download failed\n");
			if (--retry_count)
				continue;
			else
				return ret;
		}

		blocknum++;
		left -= xfer_size;
	}

	ret = wait_for_state_idle(dfu, CHECK_IDLE | CHECK_DNLOAD_IDLE);
	if (ret) {
		pr_err("wait for dfu idle failed\n");
		return ret;
	}

	return 0;
}

static ssize_t dfu_write(struct file *file, const char __user *buf,
	size_t count, loff_t *pos)
{
	struct miscdevice *miscdev = file->private_data;
	struct usb_dfu *dfu = miscdev_to_dfu(miscdev);
	void *buffer = NULL;
	int ret;

	if (!buf) {
		pr_err("%s buf invalid!\n", __func__);
		return -EINVAL;
	}

	if (count > DFU_MAX_WRITE_SIZE || count == 0) {
		pr_err("%s count invalid!\n", __func__);
		return -EINVAL;
	}

	mutex_lock(&dfu_access_lock);
	if (!dfu->dev_exist) {
		pr_err("%s when no dev!\n", __func__);
		mutex_unlock(&dfu_access_lock);
		return -ENODEV;
	}

	buffer = vmalloc(count);
	if (!buffer) {
		dev_err(&dfu->usb_dev->dev, "vmalloc failed\n");
		mutex_unlock(&dfu_access_lock);
		return -ENOMEM;
	}

	if (copy_from_user(buffer, buf, count)) {
		dev_err(&dfu->usb_dev->dev, "copy_form_user failed\n");
		ret = -EFAULT;
		goto err;
	}

	ret = dfu_update_firmware(dfu, buffer, count);
	if (ret)
		goto err;

	vfree(buffer);
	mutex_unlock(&dfu_access_lock);
	return count;

err:
	vfree(buffer);
	mutex_unlock(&dfu_access_lock);
	return ret;
}

static int dfu_ioctl_get_status(struct usb_dfu *dfu, void __user *user_arg)
{
	struct dfu_status status;
	int ret;

	ret = dfu_getstatus(dfu, &status);
	if (ret < 0) {
		dev_err(&dfu->usb_dev->dev, "dfu_getstatus failed\n");
		return ret;
	}

	if (ret != sizeof(status)) {
		dev_err(&dfu->usb_dev->dev, "dfu_getstatus return len error\n");
		ret = -EIO;
		return ret;
	}

	if (!user_arg) {
		dev_err(&dfu->usb_dev->dev, "arg is null\n");
		ret = -EINVAL;
		return ret;
	}

	if (copy_to_user(user_arg, &status, sizeof(status))) {
		ret = -EFAULT;
		return ret;
	}

	return ret;
}

static int dfu_ioctl_finish_download(struct usb_dfu *dfu)
{
	struct dfu_status status;
	int ret;

	/*
	 * After the final block of firmware has been sent to
	 * the device and the status solicited, the host sends
	 * a DFU_DNLOAD request with the wLengthfield cleared
	 * to 0 and then solicits the status again.
	 */
	ret = dfu_abort(dfu);
	if (ret < 0) {
		dev_err(&dfu->usb_dev->dev, "dfu abort failed\n");
		return ret;
	}

	ret = wait_for_state_idle(dfu, CHECK_IDLE);
	if (ret) {
		dev_err(&dfu->usb_dev->dev, "wait for dfu idle failed\n");
		return ret;
	}

	ret = dfu_download(dfu, NULL, 0, 0);
	if (ret < 0)
		return ret;

	/*
	 * There is no response for last get_status,
	 * vr device will reset itself
	 */
	(void)dfu_getstatus(dfu, &status);
	ret = 0;
	return ret;
}

static int dfu_ioctl_abort(struct usb_dfu *dfu)
{
	int ret;

	ret = dfu_abort(dfu);
	if (ret < 0) {
		dev_err(&dfu->usb_dev->dev, "dfu_abort failed\n");
		return ret;
	}

	ret = wait_for_state_idle(dfu, CHECK_IDLE);
	if (ret)
		dev_err(&dfu->usb_dev->dev, "wait for dfu idle failed\n");

	return ret;
}

static int __dfu_ioctl_default(struct usb_dfu *dfu, unsigned int cmd, int len,
	const void __user *user_arg)
{
	struct dfu_status status;
	void *buf = NULL;
	int ret;

	buf = kcalloc(len, sizeof(__u8), GFP_KERNEL);
	if (!buf) {
		ret = -ENOMEM;
		return ret;
	}
	if (!user_arg) {
		dev_err(&dfu->usb_dev->dev, "arg is null\n");
		ret = -EINVAL;
		kfree(buf);
		return ret;
	}

	if (copy_from_user(buf, user_arg, len)) {
		ret = -EFAULT;
		kfree(buf);
		return ret;
	}

	ret = wait_for_state_idle(dfu, CHECK_IDLE | CHECK_DNLOAD_IDLE);
	if (ret) {
		dev_err(&dfu->usb_dev->dev, "wait for dfu idle failed\n");
		kfree(buf);
		return ret;
	}

	ret = dfu_download(dfu, buf, len, 0);
	kfree(buf);
	if (ret < 0) {
		dev_err(&dfu->usb_dev->dev, "dfu_download failed\n");
		return ret;
	}

	/* check dfu status */
	ret = dfu_getstatus(dfu, &status);
	if (ret < 0) {
		dev_err(&dfu->usb_dev->dev, "dfu_getstatus failed\n");
		return ret;
	}
	if (ret != sizeof(status)) {
		dev_err(&dfu->usb_dev->dev, "dfu_getstatus return len error\n");
		ret = -EIO;
		return ret;
	}

	if (status.bStatus != DFU_STATUS_OK) {
		ret = dfu_clearstatus(dfu);
		if (ret)
			dev_err(&dfu->usb_dev->dev, "dfu_clearstatus failed\n");
	}

	return ret;
}

static long dfu_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct miscdevice *miscdev = fp->private_data;
	struct usb_dfu *dfu = miscdev_to_dfu(miscdev);
	void __user *user_arg = (void __user *)(uintptr_t)arg;
	int ret;

	pr_info("[%s] cmd 0x%08x\n", __func__, cmd);

	mutex_lock(&dfu_access_lock);
	if (!dfu->dev_exist) {
		pr_err("%s when no dev!\n", __func__);
		mutex_unlock(&dfu_access_lock);
		return -ENODEV;
	}

	switch (cmd) {
	case DFUIOCDETACH:
		ret = dfu_detach(dfu);
		break;

	case DFUIOCGETSTATUS:
		ret = dfu_ioctl_get_status(dfu, user_arg);
		break;

	case DFUIOCABORT:
		ret = dfu_ioctl_abort(dfu);
		break;

	case DFUIOCFINISHDNLOAD:
		ret = dfu_ioctl_finish_download(dfu);
		break;

	default:
		if (_IOC_TYPE(cmd) != 'D') {
			ret = -EINVAL;
			break;
		}

		if (_IOC_NR(cmd) == _IOC_NR(DFUIOCDOWNLOAD(0))) {
			int len = _IOC_SIZE(cmd);
			if (len > dfu->transfer_size || len == 0) {
				ret = -EINVAL;
				break;
			}

			ret = __dfu_ioctl_default(dfu, cmd, len, user_arg);
			break;
		}

		dev_err(&dfu->usb_dev->dev, "unhandled cmd 0x%08x\n", cmd);
		ret = -ENOTTY;
		break;
	}

	mutex_unlock(&dfu_access_lock);
	return ret;
}

static int dfu_open(struct inode *ip, struct file *file)
{
	struct miscdevice *miscdev = (struct miscdevice *)file->private_data;
	struct usb_dfu *dfu = miscdev_to_dfu(miscdev);
	int ret = 0;

	pr_info("dfu open device\n");
	mutex_lock(&dfu_access_lock);
	if (dfu->dev_open)
		ret = -EBUSY;
	else
		dfu->dev_open = 1;
	mutex_unlock(&dfu_access_lock);

	return ret;
}

static int dfu_release(struct inode *ip, struct file *file)
{
	struct miscdevice *miscdev = (struct miscdevice *)file->private_data;
	struct usb_dfu *dfu = miscdev_to_dfu(miscdev);

	pr_info("dfu release device\n");
	mutex_lock(&dfu_access_lock);
	dfu->dev_open = 0;

	if (!dfu->dev_exist) {
		if (dfu->dfu_dev.name)
			misc_deregister(&dfu->dfu_dev);

		kfree(dfu->load_buf);
		kfree(dfu);
	}
	mutex_unlock(&dfu_access_lock);

	return 0;
}

/* file operations for DFU device /dev/hwvrdfu */
static const struct file_operations dfu_fops = {
	.owner = THIS_MODULE,
	.read = dfu_read,
	.write = dfu_write,
	.unlocked_ioctl = dfu_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl   = dfu_ioctl,
#endif
	.open = dfu_open,
	.release = dfu_release,
};

static int dfu_parse_descriptor(struct usb_dfu *dfu)
{
	struct usb_interface *intf = dfu->usb_intf;
	struct usb_device *udev = dfu->usb_dev;
	struct device *dev = &udev->dev;
	struct usb_host_interface *interface = intf->cur_altsetting;
	struct dfu_descriptor *dfu_desc = NULL;
	char *p = NULL;

	if (usb_get_extra_descriptor(interface, DFU_DT_FUNC, &dfu_desc)) {
		dev_err(dev, "class descriptor not present\n");
		return -ENODEV;
	}

	dev_info(dev, "bmAttributes: 0x%x\n", dfu_desc->bmAttributes);

	p = (char *)dfu_desc;
	/* dfu_desc max length is 8 */
	pr_info("dfu desc: %02x %02x %02x %02x %02x %02x %02x %02x %02x\n",
			p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8]);

	dfu->will_detach = !!(dfu_desc->bmAttributes & BIT_WILL_DETACH);
	dfu->manifestation_tolerant = !!(dfu_desc->bmAttributes
					& BIT_MANIFESTATION_TOLERANT);
	dfu->can_upload = !!(dfu_desc->bmAttributes & BIT_CAN_UPLOAD);
	dfu->can_dnload = !!(dfu_desc->bmAttributes & BIT_CAN_DNLOAD);

	dfu->detach_timeout = le16_to_cpu(dfu_desc->wDetachTimeOut);
	dfu->transfer_size = le16_to_cpu(dfu_desc->wTransferSize);

	dev_info(dev, "transfersize %u, detachtimeout %u\n",
		dfu->transfer_size, dfu->detach_timeout);

	return 0;
}

int usbdfu_update_firmware(struct usb_dfu *dfu)
{
	struct firmware *fw = NULL;
	struct device *dev = NULL;
	int ret;

	if (!dfu)
		return -EINVAL;

	dev = &dfu->usb_dev->dev;

	ret = request_firmware((const struct firmware **)&fw, "hwvr.img", dev);
	if (ret) {
		dev_err(dev, "request firmware failed!\n");
		return ret;
	}

	if (fw == NULL) {
		dev_err(dev, "fw is NULL!\n");
		WARN_ON(1);
		return -EINVAL;
	}

	dev_info(dev, "firmware: data %pK, size %zd\n", fw->data, fw->size);

	ret = dfu_update_firmware(dfu, (void *)fw->data, fw->size);
	if (ret)
		dev_err(dev, "update firmware failed!\n");

	release_firmware(fw);

	return ret;
}


static int usbdfu_probe(struct usb_interface *intf,
	const struct usb_device_id *id)
{
	int ret;
	struct usb_dfu *dfu = NULL;

	dfu = kzalloc(sizeof(*dfu), GFP_KERNEL);
	if (!dfu) {
		pr_err("alloc dfu failed\n");
		return -ENOMEM;
	}

	dfu->usb_intf = intf;
	dfu->usb_dev = interface_to_usbdev(intf);
	usb_set_intfdata(intf, dfu);

	dfu->proctol = intf->cur_altsetting->desc.bInterfaceProtocol;

	ret = dfu_parse_descriptor(dfu);
	if (ret) {
		kfree(dfu);
		return ret;
	}

	if (dfu->transfer_size) {
		dfu->load_buf = kzalloc(dfu->transfer_size, GFP_KERNEL);
		if (!dfu->load_buf) {
			kfree(dfu);
			return -ENOMEM;
		}
	}

	if (usbdfu_update_firmware(dfu) == 0) {
		dev_info(&intf->dev, "firmware updated from hwvr.img\n");
		kfree(dfu->load_buf);
		kfree(dfu);
		return 0;
	}

	dfu->dev_open = 0;
	dfu->dev_exist = 1;

	/* register char device */
	dfu->dfu_dev.minor = MISC_DYNAMIC_MINOR;
	dfu->dfu_dev.name = "hwvrdfu";
	dfu->dfu_dev.fops = &dfu_fops;
	ret = misc_register(&dfu->dfu_dev);
	if (ret) {
		pr_err("register misc device failed\n");
		kfree(dfu->load_buf);
		kfree(dfu);
		return ret;
	}

	return 0;
}

static void usbdfu_disconnect(struct usb_interface *intf)
{
	struct usb_dfu *dfu = NULL;

	dfu = usb_get_intfdata(intf);
	if (!dfu)
		return;

	mutex_lock(&dfu_access_lock);

	dfu->dev_exist = 0;

	if (!dfu->dev_open) {
		if (dfu->dfu_dev.name)
			misc_deregister(&dfu->dfu_dev);

		kfree(dfu->load_buf);
		kfree(dfu);
	}

	mutex_unlock(&dfu_access_lock);
}

/* Treat USB reset pretty much the same as suspend/resume */
static int usbdfu_pre_reset(struct usb_interface *intf)
{
	return 0;
}

/* Same routine used for post_reset and reset_resume */
static int usbdfu_post_reset(struct usb_interface *intf)
{
	return 0;
}

static const struct usb_device_id dfu_usb_ids[] = {
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_APP_SPEC,
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 1, /* Runtime protocol */
	},
	{
		.match_flags = USB_DEVICE_ID_MATCH_INT_CLASS,
		.bInterfaceClass = USB_CLASS_APP_SPEC,
		.bInterfaceSubClass = 1,
		.bInterfaceProtocol = 2, /* DFU mode protocol */
	},
	{ }
};

MODULE_DEVICE_TABLE(usb, dfu_usb_ids);

static struct usb_driver dfu_driver = {
	.name =		"usbdfu",
	.probe =	usbdfu_probe,
	.disconnect =	usbdfu_disconnect,
	.id_table =	dfu_usb_ids,
	.pre_reset =	usbdfu_pre_reset,
	.post_reset =	usbdfu_post_reset,
};

static int __init dfu_init(void)
{
	int retval = -ENOMEM;

	retval = usb_register(&dfu_driver);
	if (retval) {
		pr_err("register dfu_driver failed\n");
		return retval;
	}

	return 0;
}

static void __exit dfu_exit(void)
{
	usb_deregister(&dfu_driver);
}

module_init(dfu_init);
module_exit(dfu_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("driver for dfu");
MODULE_AUTHOR("Huawei Technologies Co., Ltd");
