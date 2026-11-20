/*
 * sourcesink_ext_core.c
 *
 * Support for sourcesink device
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#include "sourcesink_ext_core.h"
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/mm.h>
#include <linux/moduleparam.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/timer.h>

static void complicated_callback(struct urb *urb);

int get_endpoints(struct sse_usbtest_dev *dev, struct usb_interface *intf)
{
	struct usb_host_interface *alt = intf->cur_altsetting;
	struct usb_device *udev = NULL;
	struct usb_host_endpoint *e = NULL;
	unsigned 			ep;
	int				edi;

	if (!alt)
		return -EINVAL;

	memset(dev->desc, 0, sizeof(dev->desc));
	memset(dev->pipe, 0, sizeof(dev->pipe));
	udev = testdev_to_usbdev(dev);

	for (ep = 0; ep < alt->desc.bNumEndpoints; ep++) {
		e = alt->endpoint + ep;
		edi = usb_endpoint_dir_in(&e->desc);

		switch (usb_endpoint_type(&e->desc)) {
		case USB_ENDPOINT_XFER_BULK:
			if (edi) {
				dev->desc[SS_BULK_IN_EP_IDX] = &e->desc;
				dev->pipe[SS_BULK_IN_EP_IDX] =
					usb_rcvbulkpipe(udev,
						usb_endpoint_num(&e->desc));
			} else {
				dev->desc[SS_BULK_OUT_EP_IDX] = &e->desc;
				dev->pipe[SS_BULK_OUT_EP_IDX] =
					usb_sndbulkpipe(udev,
						usb_endpoint_num(&e->desc));
			}
			break;
		case USB_ENDPOINT_XFER_INT:
			if (edi) {
				dev->desc[SS_INTR_IN_EP_IDX] = &e->desc;
				dev->pipe[SS_INTR_IN_EP_IDX] =
					usb_rcvintpipe(udev,
						usb_endpoint_num(&e->desc));
			} else {
				dev->desc[SS_INTR_OUT_EP_IDX] = &e->desc;
				dev->pipe[SS_INTR_OUT_EP_IDX] =
					usb_sndintpipe(udev,
						usb_endpoint_num(&e->desc));
			}
			break;
		case USB_ENDPOINT_XFER_ISOC:
			if (edi) {
				dev->desc[SS_ISOC_IN_EP_IDX] = &e->desc;
				dev->pipe[SS_ISOC_IN_EP_IDX] =
					usb_rcvisocpipe(udev,
						usb_endpoint_num(&e->desc));
			} else {
				dev->desc[SS_ISOC_OUT_EP_IDX] = &e->desc;
				dev->pipe[SS_ISOC_OUT_EP_IDX] =
					usb_sndisocpipe(udev,
						usb_endpoint_num(&e->desc));
			}
			/* FALLTHROUGH */
		default:
			break;
		}
	}
	if ((dev->desc[SS_BULK_IN_EP_IDX] && dev->desc[SS_BULK_OUT_EP_IDX])
		|| (dev->desc[SS_INTR_IN_EP_IDX] && dev->desc[SS_INTR_OUT_EP_IDX])
		|| (dev->desc[SS_ISOC_IN_EP_IDX] && dev->desc[SS_ISOC_OUT_EP_IDX]))
		return 0;

	return -EINVAL;
}

/*
 * Support for testing basic non-queued I/O streams.
 *
 * These just package urbs as requests that can be easily canceled.
 * Each urb's data buffer is dynamically allocated; callers can fill
 * them with non-zero test data (or test for it) when appropriate.
 */
static void simple_callback(struct urb *urb)
{
	complete(urb->context);
}

static struct urb *sse_usbtest_alloc_urb(
	struct usb_device *udev,
	int			pipe,
	unsigned long		bytes,
	unsigned		transfer_flags,
	unsigned		offset,
	u8			binterval,
	usb_complete_t		complete_fn)
{
	struct urb *urb = NULL;

	urb = usb_alloc_urb(0, GFP_KERNEL);
	if (!urb)
		return urb;

	if (binterval)
		usb_fill_int_urb(urb, udev, pipe, NULL, bytes, complete_fn,
				NULL, binterval);
	else
		usb_fill_bulk_urb(urb, udev, pipe, NULL, bytes, complete_fn,
				NULL);

	urb->transfer_flags = transfer_flags;
	if (usb_pipein(pipe))
		urb->transfer_flags |= URB_SHORT_NOT_OK;

	if ((bytes + offset) == 0)
		return urb;

	if (urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP)
		urb->transfer_buffer = usb_alloc_coherent(udev, bytes + offset,
			GFP_KERNEL, &urb->transfer_dma);
	else
		urb->transfer_buffer = kmalloc(bytes + offset, GFP_KERNEL);

	if (!urb->transfer_buffer) {
		usb_free_urb(urb);
		return NULL;
	}

	/* To test unaligned transfers add an offset and fill the
		unused memory with a guard value */
	if (offset) {
		memset(urb->transfer_buffer, GUARD_BYTE, offset);
		urb->transfer_buffer += offset;
		if (urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP)
			urb->transfer_dma += offset;
	}

	/* For inbound transfers use guard byte so that test fails if
		data not correctly copied */
	memset(urb->transfer_buffer,
			usb_pipein(urb->pipe) ? GUARD_BYTE : 0,
			bytes);
	return urb;
}

struct urb *simple_alloc_urb_no_dma(
	struct usb_device *udev,
	int			pipe,
	unsigned long		bytes,
	u8			binterval)
{
	return sse_usbtest_alloc_urb(udev, pipe, bytes, URB_NO_TRANSFER_DMA_MAP, 0,
			binterval, simple_callback);
}

static struct urb *complicated_alloc_urb_no_dma(
	struct usb_device *udev,
	int			pipe,
	unsigned long		bytes,
	u8			binterval)
{
	return sse_usbtest_alloc_urb(udev, pipe, bytes, URB_NO_TRANSFER_DMA_MAP, 0,
			binterval, complicated_callback);
}

static unsigned get_maxpacket(struct usb_device *udev, int pipe)
{
	struct usb_host_endpoint *ep = NULL;

	ep = usb_pipe_endpoint(udev, pipe);
	if (!ep)
		return 0;

	return usb_endpoint_maxp(&ep->desc);
}

static void simple_fill_buf(struct sse_usbtest_dev *tdev, struct urb *urb)
{
	unsigned	i;
	u8 *buf = urb->transfer_buffer;
	unsigned	len = urb->transfer_buffer_length;
	unsigned	maxpacket;

	switch (tdev->info->pattern) {
	case 0:
		for (i = 0; i < len; i++)
			*buf++ = 0;
		break;
	case 1: /* mod63 */
		maxpacket = get_maxpacket(urb->dev, urb->pipe);
		if (!maxpacket)
			break;

		for (i = 0; i < len; i++)
			/* 63: if maxpacket = 64 */
			*buf++ = (u8) ((i % maxpacket) % 63);
		break;
	default:
		return;
	}
}

static inline unsigned long buffer_offset(u8 *buf)
{
	return (unsigned long)(uintptr_t)buf & (ARCH_KMALLOC_MINALIGN - 1);
}

static int check_guard_bytes(struct sse_usbtest_dev *tdev, struct urb *urb)
{
	u8 *buf = urb->transfer_buffer;
	u8 *guard = buf - buffer_offset(buf);
	unsigned i;

	for (i = 0; guard < buf; i++, guard++) {
		if (*guard != GUARD_BYTE) {
			pr_err("guard byte[%d] %d (not %d)\n",
				i, *guard, GUARD_BYTE);
			return -EINVAL;
		}
	}
	return 0;
}

static int simple_check_buf(struct sse_usbtest_dev *tdev, struct urb *urb)
{
	unsigned	i;
	u8		expected;
	u8 *buf = urb->transfer_buffer;
	unsigned	len = urb->actual_length;
	unsigned	maxpacket = get_maxpacket(urb->dev, urb->pipe);

	int ret = check_guard_bytes(tdev, urb);
	if (ret)
		return ret;

	if (!maxpacket)
		return -EINVAL;

	for (i = 0; i < len; i++, buf++) {
		switch (tdev->info->pattern) {
		/* all-zeroes has no synchronization issues */
		case 0:
			expected = 0;
			break;
		/*
		 * mod63 stays in sync with short-terminated transfers,
		 * or otherwise when host and gadget agree on how large
		 * each usb transfer request should be.  resync is done
		 * with set_interface or set_config.
		 */
		case 1: /* mod63 */
			expected = (i % maxpacket) % 63;
			break;
		/* always success unsupported patterns */
		default:
			return 0;
		}
		if (*buf == expected)
			continue;
		pr_err("buf[%d] = %d (not %d)\n", i, *buf, expected);
		return -EINVAL;
	}
	return 0;
}

void simple_free_urb(struct urb *urb)
{
	unsigned long offset = buffer_offset(urb->transfer_buffer);

	if (urb->transfer_flags & URB_NO_TRANSFER_DMA_MAP)
		usb_free_coherent(
			urb->dev,
			urb->transfer_buffer_length + offset,
			urb->transfer_buffer - offset,
			urb->transfer_dma - offset);
	else
		kfree(urb->transfer_buffer - offset);
	usb_free_urb(urb);
}

int simple_io(
	struct sse_usbtest_dev *tdev,
	struct urb *urb,
	int			iterations,
	int			vary,
	int			expected,
	const char *label
)
{
	struct usb_device *udev = urb->dev;
	int			max = urb->transfer_buffer_length;
	struct completion	completion;
	int			retval = 0;
	unsigned long		expire;

	// cppcheck-suppress *
	urb->context = &completion;
	while (retval == 0 && iterations-- > 0) {
		init_completion(&completion);
		if (usb_pipeout(urb->pipe)) {
			simple_fill_buf(tdev, urb);
			urb->transfer_flags |= URB_ZERO_PACKET;
		}
		retval = usb_submit_urb(urb, GFP_KERNEL);
		if (retval != 0)
			break;

		expire = msecs_to_jiffies(SIMPLE_IO_TIMEOUT);
		if (!wait_for_completion_timeout(&completion, expire)) {
			usb_kill_urb(urb);
			retval = (urb->status == -ENOENT ?
				  -ETIMEDOUT : urb->status);
		} else {
			retval = urb->status;
		}

		urb->dev = udev;
		if (retval == 0 && usb_pipein(urb->pipe))
			retval = simple_check_buf(tdev, urb);

		if (vary) {
			int	len = urb->transfer_buffer_length;

			len += vary;
			len %= max;
			if (len == 0)
				len = (vary < max) ? vary : max;
			urb->transfer_buffer_length = len;
		}
	}
	urb->transfer_buffer_length = max;

	if (expected != retval)
		pr_err("%s failed, iterations left %d, status %d (not %d)\n",
			label, iterations, retval, expected);
	return retval;
}

/*
 * We use scatterlist primitives to test queued I/O.
 * Yes, this also tests the scatterlist primitives.
 */
void free_sglist(struct scatterlist *sg, int nents)
{
	int		i;

	if (!sg)
		return;
	for (i = 0; i < nents; i++) {
		if (!sg_page(&sg[i]))
			continue;
		kfree(sg_virt(&sg[i]));
	}
	kfree(sg);
}

struct scatterlist *alloc_sglist(int nents, int max, int vary,
				struct sse_usbtest_dev *dev, int pipe)
{
	struct scatterlist *sg = NULL;
	unsigned int		n_size = 0;
	int			i;
	int			size = max;
	unsigned		maxpacket =
		get_maxpacket(interface_to_usbdev(dev->intf), pipe);

	if (max == 0)
		return NULL;

	sg = kmalloc_array(nents, sizeof(*sg), GFP_KERNEL);
	if (!sg)
		return NULL;
	sg_init_table(sg, nents);

	for (i = 0; i < nents; i++) {
		char *buf;
		int j;

		buf = kzalloc(size, GFP_KERNEL);
		if (!buf) {
			free_sglist(sg, i);
			return NULL;
		}

		/* kmalloc pages are always physically contiguous! */
		sg_set_buf(&sg[i], buf, size);

		switch (dev->info->pattern) {
		case 0:
			/* already zeroed */
			break;
		case 1:
			if (maxpacket == 0)
				break;

			for (j = 0; j < size; j++)
				/* 63: if maxpacket = 64 */
				*buf++ = (u8) (((j + n_size) % maxpacket) % 63);
			n_size += size;
			break;
		default:
			break;
		}

		if (vary) {
			size += vary;
			size %= max;
			if (size == 0)
				size = (vary < max) ? vary : max;
		}
	}

	return sg;
}

static void sg_timeout(unsigned long _req)
{
	struct usb_sg_request *req = (struct usb_sg_request *)_req;

	usb_sg_cancel(req);
}

int perform_sglist(
	struct sse_usbtest_dev *tdev,
	unsigned		iterations,
	int			pipe,
	struct usb_sg_request *req,
	struct scatterlist *sg,
	int			nents
)
{
	struct usb_device *udev = testdev_to_usbdev(tdev);
	int			retval = 0;
	struct timer_list	sg_timer;

	setup_timer_on_stack(&sg_timer, sg_timeout, (unsigned long)(uintptr_t)req);

	while (retval == 0 && iterations-- > 0) {
		retval = usb_sg_init(req, udev, pipe,
				(udev->speed == USB_SPEED_HIGH)
					? (INTERRUPT_RATE << 3) /* 3 for high speed */
					: INTERRUPT_RATE,
				sg, nents, 0, GFP_KERNEL);
		if (retval)
			break;
		mod_timer(&sg_timer, jiffies +
				msecs_to_jiffies(SIMPLE_IO_TIMEOUT));
		usb_sg_wait(req);
		if (!del_timer_sync(&sg_timer))
			retval = -ETIMEDOUT;
		else
			retval = req->status;
	}

	if (retval)
		pr_err("perform_sglist failed, "
				"iterations left %d, status %d\n",
				iterations, retval);
	return retval;
}

/*
 * unqueued control message testing
 *
 * there's a nice set of device functional requirements in chapter 9 of the
 * usb 2.0 spec, which we can apply to ANY device, even ones that don't use
 * special test firmware.
 *
 * we know the device is configured (or suspended) by the time it's visible
 * through usbfs.  we can't change that, so we won't test enumeration (which
 * worked 'well enough' to get here, this time), power management (ditto),
 * or remote wakeup (which needs human interaction).
 */
static unsigned realworld = 1;
module_param(realworld, uint, 0);
MODULE_PARM_DESC(realworld, "clear to demand stricter spec compliance");

static int get_altsetting(struct sse_usbtest_dev *dev)
{
	struct usb_interface *iface = dev->intf;
	struct usb_device *udev = interface_to_usbdev(iface);
	int			retval;

	retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			USB_REQ_GET_INTERFACE, USB_DIR_IN | USB_RECIP_INTERFACE,
			0, iface->altsetting[0].desc.bInterfaceNumber,
			dev->buf, 1, USB_CTRL_GET_TIMEOUT);
	switch (retval) {
	case 1:
		return dev->buf[0];
	case 0:
		retval = -ERANGE;
		/* FALLTHROUGH */
	default:
		return retval;
	}
}

int set_altsetting(struct sse_usbtest_dev *dev, int alternate)
{
	struct usb_interface *iface = dev->intf;
	struct usb_device *udev = NULL;

	/* 256: max alternate setting */
	if (alternate < 0 || alternate >= 256)
		return -EINVAL;

	udev = interface_to_usbdev(iface);
	return usb_set_interface(udev,
			iface->altsetting[0].desc.bInterfaceNumber,
			alternate);
}

static int is_good_config(struct sse_usbtest_dev *tdev, int len)
{
	struct usb_config_descriptor *config = NULL;

	if (len < (int)sizeof(*config))
		return 0;

	config = (struct usb_config_descriptor *)tdev->buf;

	switch (config->bDescriptorType) {
	case USB_DT_CONFIG:
	case USB_DT_OTHER_SPEED_CONFIG:
		/* 9: config length */
		if (config->bLength != 9) {
			pr_err("bogus config descriptor length\n");
			return 0;
		}
		/* this bit 'must be 1' but often isn't */
		if (!realworld && !(config->bmAttributes & 0x80)) {
			pr_err("high bit of config attributes not set\n");
			return 0;
		}
		if (config->bmAttributes & 0x1f) { /* reserved == 0 */
			pr_err("reserved config bits set\n");
			return 0;
		}
		break;
	default:
		return 0;
	}

	if (le16_to_cpu(config->wTotalLength) == len) /* read it all */
		return 1;
	if (le16_to_cpu(config->wTotalLength) >= TBUF_SIZE) /* max partial read */
		return 1;

	pr_err("bogus config descriptor read size\n");
	return 0;
}

static int is_good_ext(struct sse_usbtest_dev *tdev, u8 *buf)
{
	struct usb_ext_cap_descriptor *ext
			 = (struct usb_ext_cap_descriptor *)buf;
	u32 attr;

	if (ext->bLength != USB_DT_USB_EXT_CAP_SIZE) {
		pr_err("bogus usb 2.0 extension descriptor length\n");
		return 0;
	}

	attr = le32_to_cpu(ext->bmAttributes);
	/* bits[1:15] is used and others are reserved */
	if (attr & ~0xfffe) { /* reserved == 0 */
		pr_err("reserved bits set\n");
		return 0;
	}

	return 1;
}

static int is_good_ss_cap(struct sse_usbtest_dev *tdev, u8 *buf)
{
	struct usb_ss_cap_descriptor *ss = (struct usb_ss_cap_descriptor *)buf;

	if (ss->bLength != USB_DT_USB_SS_CAP_SIZE) {
		pr_err("bogus superspeed device capability descriptor length\n");
		return 0;
	}

	/*
	 * only bit[1] of bmAttributes is used for LTM and others are
	 * reserved
	 */
	if (ss->bmAttributes & ~0x02) { /* reserved == 0 */
		pr_err("reserved bits set in bmAttributes\n");
		return 0;
	}

	/* bits[0:3] of wSpeedSupported is used and others are reserved */
	if (le16_to_cpu(ss->wSpeedSupported) & ~0x0f) { /* reserved == 0 */
		pr_err("reserved bits set in wSpeedSupported\n");
		return 0;
	}

	return 1;
}

static int is_good_con_id(struct sse_usbtest_dev *tdev, u8 *buf)
{
	struct usb_ss_container_id_descriptor *con_id
			 = (struct usb_ss_container_id_descriptor *)buf;

	if (con_id->bLength != USB_DT_USB_SS_CONTN_ID_SIZE) {
		pr_err("bogus container id descriptor length\n");
		return 0;
	}

	if (con_id->bReserved) { /* reserved == 0 */
		pr_err("reserved bits set\n");
		return 0;
	}

	return 1;
}

/*
 * sanity test for standard requests working with usb_control_mesg() and some
 * of the utility functions which use it.
 *
 * this doesn't test how endpoint halts behave or data toggles get set, since
 * we won't do I/O to bulk/interrupt endpoints here (which is how to change
 * halt or toggle).  toggle testing is impractical without support from hcds.
 *
 * this avoids failing devices linux would normally work with, by not testing
 * config/altsetting operations for devices that only support their defaults.
 * such devices rarely support those needless operations.
 *
 * NOTE that since this is a sanity test, it's not examining boundary cases
 * to see if usbcore, hcd, and device all behave right.  such testing would
 * involve varied read sizes and other operation sequences.
 */
static int ch9_setting_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface *iface = dev->intf;
	int			alt;
	int			retval;
	u32			i;

	/*
	 * [9.2.3] if there's more than one altsetting, we need to be able to
	 * set and get each one.  mostly trusts the descriptors from usbcore.
	 */
	for (i = 0; i < iface->num_altsetting; i++) {
		/* 9.2.3 constrains the range here */
		alt = iface->altsetting[i].desc.bAlternateSetting;
		if (alt < 0 || alt >= (int)iface->num_altsetting)
			pr_err("invalid alt [%d].bAltSetting = %u\n", i, alt);

		/* [real world] get/set unimplemented if there's only one */
		if (realworld && iface->num_altsetting == 1)
			continue;

		/* [9.4.10] set_interface */
		retval = set_altsetting(dev, (int)alt);
		if (retval) {
			pr_err("can't set_interface = %d, %d\n", alt, retval);
			return retval;
		}

		/* [9.4.4] get_interface always works */
		retval = get_altsetting(dev);
		if (retval != alt) {
			pr_err("get alt should be %d, was %d\n", alt, retval);
			return (retval < 0) ? retval : -EDOM;
		}
	}

	return 0;
}

int ch9_config_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface *iface = dev->intf;
	struct usb_device *udev = interface_to_usbdev(iface);
	int	expected = udev->actconfig->desc.bConfigurationValue;
	int	retval;

	/* [real world] get_config unimplemented if there's only one */
	if (!realworld || udev->descriptor.bNumConfigurations != 1) {
		/*
		 * [9.4.2] get_configuration always works
		 * ... although some cheap devices (like one TI Hub I've got)
		 * won't return config descriptors except before set_config.
		 */
		retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
				USB_REQ_GET_CONFIGURATION,
				USB_DIR_IN | USB_RECIP_DEVICE,
				0, 0, dev->buf, 1, USB_CTRL_GET_TIMEOUT);
		if (retval != 1 || dev->buf[0] != expected) {
			pr_err("get config --> %d %d (1 %d)\n",
				retval, dev->buf[0], expected);
			return (retval < 0) ? retval : -EDOM;
		}
	}

	return 0;
}

static int ch9_desc_bos_test(struct sse_usbtest_dev *dev,
			struct usb_interface *iface,
			struct usb_device *udev)
{
	struct usb_bos_descriptor *bos = NULL;
	struct usb_dev_cap_header *header = NULL;
	unsigned total, num, length;
	u8 *buf = NULL;
	int	retval;

	/*
	 * there's always [9.4.3] a bos device descriptor [9.6.2] in USB
	 * 3.0 spec
	 */
	if (le16_to_cpu(udev->descriptor.bcdUSB) < 0x0210)
		return 0;

	retval = usb_get_descriptor(udev, USB_DT_BOS, 0, dev->buf,
			sizeof(*udev->bos->desc));
	if (retval != sizeof(*udev->bos->desc)) {
		pr_err("bos descriptor --> %d\n", retval);
		return (retval < 0) ? retval : -EDOM;
	}

	bos = (struct usb_bos_descriptor *)dev->buf;
	total = le16_to_cpu(bos->wTotalLength);
	num = bos->bNumDeviceCaps;

	if (total > TBUF_SIZE)
		total = TBUF_SIZE;

	/*
	 * get generic device-level capability descriptors [9.6.2]
	 * in USB 3.0 spec
	 */
	retval = usb_get_descriptor(udev, USB_DT_BOS, 0, dev->buf, total);
	if (retval != total) {
		pr_err("bos descriptor set --> %d\n", retval);
		return (retval < 0) ? retval : -EDOM;
	}

	length = sizeof(*udev->bos->desc);
	buf = dev->buf;

	while (num--) {
		buf += length;

		if (buf + sizeof(struct usb_dev_cap_header) > dev->buf + total)
			break;

		header = (struct usb_dev_cap_header *)buf;
		length = header->bLength;

		if (header->bDescriptorType !=  USB_DT_DEVICE_CAPABILITY) {
			pr_warn("not device capability descriptor, skip\n");
			continue;
		}

		switch (header->bDevCapabilityType) {
		case USB_CAP_TYPE_EXT:
			if (buf + USB_DT_USB_EXT_CAP_SIZE > dev->buf + total ||
				!is_good_ext(dev, buf)) {
				pr_err("bogus usb 2.0 extension descriptor\n");
				return -EDOM;
			}

			break;

		case USB_SS_CAP_TYPE:
			if (buf + USB_DT_USB_SS_CAP_SIZE > dev->buf + total ||
				!is_good_ss_cap(dev, buf)) {
				pr_err("bogus superspeed device capability descriptor\n");
				return -EDOM;
			}

			break;

		case CONTAINER_ID_TYPE:
			if (buf + USB_DT_USB_SS_CONTN_ID_SIZE > dev->buf + total ||
				!is_good_con_id(dev, buf)) {
				pr_err("bogus container id descriptor\n");
				return -EDOM;
			}

			break;

		default:
			break;
		}
	}

	return 0;
}

static int ch9_desc_config_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface	 *iface = dev->intf;
	struct usb_device	 *udev = interface_to_usbdev(iface);
	int	 retval;
	u32	 i;

	/* there's always [9.4.3] at least one config descriptor [9.6.3] */
	for (i = 0; i < udev->descriptor.bNumConfigurations; i++) {
		retval = usb_get_descriptor(udev, USB_DT_CONFIG, i,
				dev->buf, TBUF_SIZE);
		if (!is_good_config(dev, retval)) {
			pr_err("config [%d] descriptor --> %d\n",
				i, retval);
			return (retval < 0) ? retval : -EDOM;
		}

		/*
		 * cross-checking udev->config[i] to make sure usbcore
		 * parsed it right (etc) would be good testing paranoia
		 */
	}

	return 0;
}

static int ch9_desc_other_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface	 *iface = dev->intf;
	struct usb_device	 *udev = interface_to_usbdev(iface);
	struct usb_qualifier_descriptor *d = NULL;
	int	 retval;
	u32	 i;
	unsigned max;

	if (le16_to_cpu(udev->descriptor.bcdUSB) != 0x0200)
		return 0;

	/* and sometimes [9.2.6.6] speed dependent descriptors */
	/* device qualifier [9.6.2] */
	retval = usb_get_descriptor(udev,
			USB_DT_DEVICE_QUALIFIER, 0, dev->buf,
			sizeof(struct usb_qualifier_descriptor));
	if (retval == -EPIPE) {
		if (udev->speed == USB_SPEED_HIGH) {
			pr_err("hs dev qualifier --> %d\n", retval);
			return retval;
		}

		/* usb2.0 but not high-speed capable; fine */
	} else if (retval != sizeof(struct usb_qualifier_descriptor)) {
		pr_err("dev qualifier --> %d\n", retval);
		return (retval < 0) ? retval : -EDOM;
	} else {
		d = (struct usb_qualifier_descriptor *)dev->buf;
	}

	/* might not have [9.6.2] any other-speed configs [9.6.4] */
	if (d) {
		max = d->bNumConfigurations;
		for (i = 0; i < max; i++) {
			retval = usb_get_descriptor(udev,
					USB_DT_OTHER_SPEED_CONFIG, i,
					dev->buf, TBUF_SIZE);
			if (!is_good_config(dev, retval)) {
				pr_err("other speed config --> %d\n",
					retval);
				return (retval < 0) ? retval : -EDOM;
			}
		}
	}

	return 0;
}

static int ch9_desc_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface *iface = dev->intf;
	struct usb_device *udev = interface_to_usbdev(iface);
	int	retval;

	/* there's always [9.4.3] a device descriptor [9.6.1] */
	retval = usb_get_descriptor(udev, USB_DT_DEVICE, 0,
			dev->buf, sizeof(udev->descriptor));
	if (retval != sizeof(udev->descriptor)) {
		pr_err("dev descriptor --> %d\n", retval);
		return (retval < 0) ? retval : -EDOM;
	}

	retval = ch9_desc_bos_test(dev, iface, udev);
	if (retval)
		return retval;

	retval = ch9_desc_config_test(dev);

	return retval;
}

static int ch9_status_test(struct sse_usbtest_dev *dev)
{
	struct usb_interface *iface = dev->intf;
	struct usb_device *udev = interface_to_usbdev(iface);
	int	retval;

	/* fetch strings from at least the device descriptor */
	/* [9.4.5] get_status always works */
	retval = usb_get_status(udev, USB_RECIP_DEVICE, 0, dev->buf);
	if (retval) {
		pr_err("get dev status --> %d\n", retval);
		return retval;
	}

	/*
	 * configuration.bmAttributes says if we could try to set/clear
	 * the device's remote wakeup feature ... if we can, test that here
	 */
	retval = usb_get_status(udev, USB_RECIP_INTERFACE,
			iface->altsetting[0].desc.bInterfaceNumber, dev->buf);
	if (retval) {
		pr_err("get interface status --> %d\n", retval);
		return retval;
	}
	return 0;
}

int ch9_postconfig(struct sse_usbtest_dev *dev)
{
	int			retval;

	retval = ch9_setting_test(dev);
	if (retval)
		return retval;

	retval = ch9_config_test(dev);
	if (retval)
		return retval;

	retval = ch9_desc_test(dev);
	if (retval)
		return retval;

	retval = ch9_desc_other_test(dev);
	if (retval)
		return retval;

	retval = ch9_status_test(dev);

	/* get status for each endpoint in the interface */
	return retval;
}

/*
 * use ch9 requests to test whether:
 *   (a) queues work for control, keeping N subtests queued and
 *       active (auto-resubmit) for M loops through the queue.
 *   (b) protocol stalls (control-only) will autorecover.
 *       it's not like bulk/intr; no halt clearing.
 *   (c) short control reads are reported and handled.
 *   (d) queues are always processed in-order
 */

struct ctrl_ctx {
	spinlock_t		lock;
	struct sse_usbtest_dev *dev;
	struct completion	complete;
	unsigned		count;
	unsigned		pending;
	int			status;
	struct urb **urb;
	struct sse_usbtest_param_32 *param;
	int			last;
};

struct subcase {
	struct usb_ctrlrequest	setup;
	int			number;
	int			expected;
};

struct subcase_handler {
	u32 (* func)(struct usb_device *udev,
		struct usb_ctrlrequest *req,
		int *pipe, int *expected);
};

static void _ctrl_complete_unlink(struct urb *urb,
					struct ctrl_ctx *ctx,
					struct subcase *subcase)
{
	u32		i;
	int		status;
	struct urb *u = NULL;

	for (i = 1; i < ctx->param->sglen; i++) {
		u = ctx->urb[(i + subcase->number) % ctx->param->sglen];

		if (u == urb || !u->dev)
			continue;

		spin_unlock(&ctx->lock);
		status = usb_unlink_urb(u);
		spin_lock(&ctx->lock);
		switch (status) {
		case -EINPROGRESS:
		case -EBUSY:
		case -EIDRM:
			continue;
		default:
			pr_err("urb unlink --> %d\n", status);
		}
	}
}

static void ctrl_complete(struct urb *urb)
{
	struct ctrl_ctx *ctx = urb->context;
	struct usb_ctrlrequest *reqp = NULL;
	struct subcase *subcase = NULL;
	int			status = urb->status;

	reqp = (struct usb_ctrlrequest *)urb->setup_packet;
	subcase = container_of(reqp, struct subcase, setup);

	if (!ctx)
		return;

	spin_lock(&ctx->lock);
	ctx->count--;
	ctx->pending--;

	/*
	 * queue must transfer and complete in fifo order, unless
	 * usb_unlink_urb() is used to unlink something not at the
	 * physical queue head (not tested).
	 */
	if (subcase->number > 0) {
		if ((subcase->number - ctx->last) != 1) {
			pr_err("subcase %d completed out of order, last %d\n",
				subcase->number, ctx->last);
			status = -EDOM;
			ctx->last = subcase->number;
			goto error;
		}
	}
	ctx->last = subcase->number;

	/* succeed or fault in only one way? */
	if (status == subcase->expected) {
		status = 0;
	/* async unlink for cleanup? */
	} else if (status != -ECONNRESET) {
		/* some faults are allowed, not required */
		if (subcase->expected > 0 && (
			  ((status == -subcase->expected /* happened */
			   || status == 0)))) /* didn't */
			status = 0;
		/* sometimes more than one fault is allowed */
		/* 12: max case */
		else if (subcase->number == 12 && status == -EPIPE)
			status = 0;
		else
			pr_err("subtest %d error, status %d\n",
					subcase->number, status);
	}

	/* unexpected status codes mean errors; ideally, in hardware */
	if (status) {
error:
		if (ctx->status == 0) {
			pr_err("control queue %02x.%02x, err %d, "
					"%d left, subcase %d, len %d/%d\n",
					reqp->bRequestType, reqp->bRequest,
					status, ctx->count, subcase->number,
					urb->actual_length,
					urb->transfer_buffer_length);

			/* unlink whatever's still pending */
			_ctrl_complete_unlink(urb, ctx, subcase);
		}
	}

	/* resubmit if we need to, else mark this as done */
	if ((status == 0) && (ctx->pending < ctx->count)) {
		status = usb_submit_urb(urb, GFP_ATOMIC);
		if (status != 0) {
			pr_err("can't resubmit ctrl %02x.%02x, err %d\n",
				reqp->bRequestType, reqp->bRequest, status);
			urb->dev = NULL;
		} else {
			ctx->pending++;
		}
	} else {
		urb->dev = NULL;
	}

	/* signal completion when nothing's queued */
	if (ctx->pending == 0)
		complete(&ctx->complete);
	spin_unlock(&ctx->lock);
}

static u32 subcase0(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get device descriptor, 8: change to 16bits */
	req->wValue = cpu_to_le16(USB_DT_DEVICE << 8);

	return sizeof(struct usb_device_descriptor);
}

static u32 subcase1(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get first config descriptor (only), 8: change to 16bits */
	req->wValue = cpu_to_le16((USB_DT_CONFIG << 8) | 0);

	return sizeof(struct usb_config_descriptor);
}

static u32 subcase2(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);

	/* get altsetting (OFTEN STALLS) */
	req->bRequest = USB_REQ_GET_INTERFACE;
	req->bRequestType = USB_DIR_IN | USB_RECIP_INTERFACE;
	*expected = EPIPE;
	/* index = 0 means first interface */

	return 1;
}

static u32 subcase3(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get interface status */
	req->bRequest = USB_REQ_GET_STATUS;
	req->bRequestType = USB_DIR_IN | USB_RECIP_INTERFACE;
	/* interface 0 */

	return 2; /* 2: current req.wLength */
}

static u32 subcase4(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get device status */
	req->bRequest = USB_REQ_GET_STATUS;
	req->bRequestType = USB_DIR_IN | USB_RECIP_DEVICE;

	return 2; /* 2: current req.wLength */
}

static u32 subcase5(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(pipe);

	/* get device qualifier (MAY STALL), shift 8 for 16bits */
	req->wValue = cpu_to_le16 (USB_DT_DEVICE_QUALIFIER << 8);
	if (udev->speed != USB_SPEED_HIGH)
		*expected = EPIPE;

	return sizeof(struct usb_qualifier_descriptor);
}

static u32 subcase6(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get first config descriptor, plus interface,shift 8 for 16bits */
	req->wValue = cpu_to_le16((USB_DT_CONFIG << 8) | 0);

	return (sizeof(struct usb_config_descriptor)
		+ sizeof(struct usb_interface_descriptor));
}

static u32 subcase7(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);

	/* get interface descriptor (ALWAYS STALLS), shift 8 for 16bits */
	req->wValue = cpu_to_le16 (USB_DT_INTERFACE << 8);
	/* interface == 0 */
	*expected = -EPIPE;

	return sizeof(struct usb_interface_descriptor);
}

static u32 subcase8(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	/*
	 * NOTE: two consecutive stalls in the queue here.
	 *  that tests fault recovery a bit more aggressively.
	 */
	/* clear endpoint halt (MAY STALL) */
	req->bRequest = USB_REQ_CLEAR_FEATURE;
	req->bRequestType = USB_RECIP_ENDPOINT;
	/* wValue 0 == ep halt */
	/* wIndex 0 == ep0 (shouldn't halt!) */
	*pipe = usb_sndctrlpipe(udev, 0);
	*expected = EPIPE;

	return 0;
}

static u32 subcase9(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);
	unused(expected);

	/* get endpoint status */
	req->bRequest = USB_REQ_GET_STATUS;
	req->bRequestType = USB_DIR_IN | USB_RECIP_ENDPOINT;
	/* endpoint 0 */

	return 2; /* 2: current req.wLength */
}

static u32 subcase10(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);

	/* trigger short read (EREMOTEIO),shift 8bits */
	req->wValue = cpu_to_le16((USB_DT_CONFIG << 8) | 0);
	*expected = -EREMOTEIO;

	return 1024; /* 1024: current req.wLength */
}

static u32 subcase11(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);

	/* NOTE: two consecutive _different_ faults in the queue. */
	/* get endpoint descriptor (ALWAYS STALLS),shift 8 for 16bits */
	req->wValue = cpu_to_le16(USB_DT_ENDPOINT << 8);
	/* endpoint == 0 */
	*expected = EPIPE;

	return sizeof(struct usb_interface_descriptor);
}

static u32 subcase12(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	unused(udev);
	unused(pipe);

	/* NOTE: sometimes even a third fault in the queue! */
	/* get string 0 descriptor (MAY STALL),shift 8 for 16bits */
	req->wValue = cpu_to_le16(USB_DT_STRING << 8);
	/* may succeed when > 4 languages */
	*expected = EREMOTEIO; /* or EPIPE, if no strings */
	/* string == 0, for language IDs */

	return sizeof(struct usb_interface_descriptor);
}

static u32 subcase13(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	u32 len;

	unused(pipe);
	/* short read, resembling case 10,shift 8 for 16bits */
	req->wValue = cpu_to_le16((USB_DT_CONFIG << 8) | 0);
	/* last data packet "should" be DATA1, not DATA0 */
	if (udev->speed == USB_SPEED_SUPER)
		/* 1024:max bMaxPacketSize0, 512: set bMaxPacketSize0 */
		len = 1024 - 512;
	else
		/* 1024:max bMaxPacketSize0 */
		len = 1024 - udev->descriptor.bMaxPacketSize0;
	*expected = -EREMOTEIO;

	return len;
}

static u32 subcase14(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	u32 len;

	unused(pipe);
	/* short read; try to fill the last packet, shift 8 for 16bits */
	req->wValue = cpu_to_le16((USB_DT_DEVICE << 8) | 0);
	/* device descriptor size == 18 bytes */
	len = udev->descriptor.bMaxPacketSize0;
	if (udev->speed == USB_SPEED_SUPER)
		len = 512; /* 512: for supper speed */

	switch (len) {
	case 8: /* 8: bMaxPacketSize0 */
		len = 24; /* 24: current req.wLength */
		break;
	case 16: /* 16: bMaxPacketSize0 */
		len = 32; /* 32: current req.wLength */
		break;
	default:
		break;
	}
	*expected = -EREMOTEIO;

	return len;
}

static u32 subcase15(struct usb_device *udev,
			struct usb_ctrlrequest *req,
			int *pipe, int *expected)
{
	u32 len;

	unused(pipe);
	/* shift 8 for 16bits */
	req->wValue = cpu_to_le16(USB_DT_BOS << 8);
	if (udev->bos)
		len = le16_to_cpu(udev->bos->desc->wTotalLength);
	else
		len = sizeof(struct usb_bos_descriptor);
	if (le16_to_cpu(udev->descriptor.bcdUSB) < 0x0201)
		*expected = -EPIPE;

	return len;
}

/* how many test subcases here */
#define NUM_SUBCASES	16
static struct subcase_handler subcases[NUM_SUBCASES] = {
	{.func = subcase0},
	{.func = subcase1},
	{.func = subcase2},
	{.func = subcase3},
	{.func = subcase4},
	{.func = subcase5},
	{.func = subcase6},
	{.func = subcase7},
	{.func = subcase8},
	{.func = subcase9},
	{.func = subcase10},
	{.func = subcase11},
	{.func = subcase12},
	{.func = subcase13},
	{.func = subcase14},
	{.func = subcase15}
};

int test_ctrl_queue(struct sse_usbtest_dev *dev,
				struct sse_usbtest_param_32 *param)
{
	struct usb_device *udev = testdev_to_usbdev(dev);
	struct urb **urb = NULL;
	struct ctrl_ctx		context;
	u32			i;

	if (param->sglen == 0 || param->iterations > UINT_MAX / param->sglen)
		return -EOPNOTSUPP;

	spin_lock_init(&context.lock);
	context.dev = dev;
	init_completion(&context.complete);
	context.count = param->sglen * param->iterations;
	context.pending = 0;
	context.status = -ENOMEM;
	context.param = param;
	context.last = -1;

	/*
	 * allocate and init the urbs we'll queue.
	 * as with bulk/intr sglists, sglen is the queue depth; it also
	 * controls which subtests run (more tests than sglen) or rerun.
	 */
	urb = kcalloc(param->sglen, sizeof(struct urb *), GFP_KERNEL);
	if (!urb)
		return -ENOMEM;
	for (i = 0; i < param->sglen; i++) {
		int			pipe = usb_rcvctrlpipe(udev, 0);
		unsigned		len;
		struct urb *u = NULL;
		struct usb_ctrlrequest	req;
		struct subcase *reqp = NULL;

		/*
		 * sign of this variable means:
		 *  -: tested code must return this (negative) error code
		 *  +: tested code may return this (negative too) error code
		 */
		int			expected = 0;

		/*
		 * requests here are mostly expected to succeed on any
		 * device, but some are chosen to trigger protocol stalls
		 * or short reads.
		 */
		memset(&req, 0, sizeof(req));
		req.bRequest = USB_REQ_GET_DESCRIPTOR;
		req.bRequestType = USB_DIR_IN | USB_RECIP_DEVICE;

		len = subcases[i % NUM_SUBCASES].func(udev, &req, &pipe, &expected);

		req.wLength = cpu_to_le16(len);
		urb[i] = u = simple_alloc_urb_no_dma(udev, pipe, len, 0);
		if (!u)
			goto cleanup;

		reqp = kmalloc(sizeof(*reqp), GFP_KERNEL);
		if (!reqp)
			goto cleanup;
		reqp->setup = req;
		reqp->number = i % NUM_SUBCASES;
		reqp->expected = expected;
		u->setup_packet = (char *) &reqp->setup;

		u->context = &context;
		u->complete = ctrl_complete;
	}

	/* queue the urbs */
	context.urb = urb;
	spin_lock_irq(&context.lock);
	for (i = 0; i < param->sglen; i++) {
		context.status = usb_submit_urb(urb[i], GFP_ATOMIC);
		if (context.status != 0) {
			pr_err("can't submit urb[%d], status %d\n",
					i, context.status);
			context.count = context.pending;
			break;
		}
		context.pending++;
	}
	spin_unlock_irq(&context.lock);

	/* could set timer and time out; provide a disconnect hook */

	/* wait for the last one to complete */
	if (context.pending > 0)
		wait_for_completion(&context.complete);

cleanup:
	for (i = 0; i < param->sglen; i++) {
		if (!urb[i])
			continue;
		urb[i]->dev = udev;
		kfree(urb[i]->setup_packet);
		simple_free_urb(urb[i]);
	}
	kfree(urb);
	return context.status;
}
#undef NUM_SUBCASES

static void unlink1_callback(struct urb *urb)
{
	int	status = urb->status;

	/* we "know" -EPIPE (stall) never happens */
	if (!status)
		status = usb_submit_urb(urb, GFP_ATOMIC);
	if (status) {
		urb->status = status;
		complete(urb->context);
	}
}

static int unlink1(struct sse_usbtest_dev *dev, int pipe,
		int size, int async, u8 interval)
{
	struct urb *urb = NULL;
	struct completion	completion;
	int			retval;

	init_completion(&completion);
	urb = simple_alloc_urb_no_dma(testdev_to_usbdev(dev), pipe, size, interval);
	if (!urb)
		return -ENOMEM;
	urb->context = &completion;
	urb->complete = unlink1_callback;

	if (usb_pipeout(urb->pipe)) {
		simple_fill_buf(dev, urb);
		urb->transfer_flags |= URB_ZERO_PACKET;
	}

	/*
	 * keep the endpoint busy.  there are lots of hc/hcd-internal
	 * states, and testing should get to all of them over time.
	 *
	 * want additional tests for when endpoint is STALLing
	 * due to errors, or is just NAKing requests.
	 */
	retval = usb_submit_urb(urb, GFP_KERNEL);
	if (retval != 0) {
		pr_err("submit fail %d\n", retval);
		return retval;
	}

	/*
	 * unlinking that should always work.  variable delay tests more
	 * hcd states and code paths, even with little other system load.
	 */
	/* 2: for get sleep ms */
	msleep(jiffies % (2 * INTERRUPT_RATE));
	if (async) {
		while (!completion_done(&completion)) {
			retval = usb_unlink_urb(urb);
			if (retval == 0 && usb_pipein(urb->pipe))
				retval = simple_check_buf(dev, urb);

			switch (retval) {
			case -EBUSY:
			case -EIDRM:
				/*
				 * we can't unlink urbs while they're completing
				 * or if they've completed, and we haven't
				 * resubmitted. "normal" drivers would prevent
				 * resubmission, but since we're testing unlink
				 * paths, we can't.
				 */
				pr_err("unlink retry\n");
				continue;
			case 0:
			case -EINPROGRESS:
				break;

			default:
				pr_err("unlink fail %d\n", retval);
				return retval;
			}

			break;
		}
	} else {
		usb_kill_urb(urb);
	}

	wait_for_completion(&completion);
	retval = urb->status;
	simple_free_urb(urb);

	if (async)
		return (retval == -ECONNRESET) ? 0 : retval - 1000; /* 1000: error info */
	else
		return (retval == -ENOENT || retval == -EPERM) ?
				0 : retval - 2000; /* 2000: error info */
}

int unlink_simple(struct sse_usbtest_dev *dev, int pipe,
				int len, u8 interval)
{
	int			retval;

	/* test sync and async paths */
	retval = unlink1(dev, pipe, len, 1, interval);
	if (!retval)
		retval = unlink1(dev, pipe, len, 0, interval);
	return retval;
}

struct queued_ctx {
	struct completion	complete;
	atomic_t		pending;
	unsigned		num;
	int			status;
	struct urb **urbs;
};

static void unlink_queued_callback(struct urb *urb)
{
	int			status = urb->status;
	struct queued_ctx *ctx = urb->context;

	if (ctx->status)
		goto done;
	/* 4,2: urb number */
	if (urb == ctx->urbs[ctx->num - 4] || urb == ctx->urbs[ctx->num - 2]) {
		if (status == -ECONNRESET)
			goto done;
		/* What error should we report if the URB completed normally? */
	}
	if (status != 0)
		ctx->status = status;

done:
	if (atomic_dec_and_test(&ctx->pending))
		complete(&ctx->complete);
}

int unlink_queued(struct sse_usbtest_dev *dev, int pipe, unsigned num,
		unsigned size)
{
	struct queued_ctx	ctx;
	struct usb_device *udev = testdev_to_usbdev(dev);
	void *buf = NULL;
	dma_addr_t		buf_dma;
	u32			i;
	int			retval = -ENOMEM;

	init_completion(&ctx.complete);
	atomic_set(&ctx.pending, 1); /* One more than the actual value */
	ctx.num = num;
	ctx.status = 0;

	buf = usb_alloc_coherent(udev, size, GFP_KERNEL, &buf_dma);
	if (!buf)
		return retval;
	memset(buf, 0, size);

	/* Allocate and init the urbs we'll queue */
	ctx.urbs = kcalloc(num, sizeof(struct urb *), GFP_KERNEL);
	if (!ctx.urbs)
		goto free_buf;
	for (i = 0; i < num; i++) {
		ctx.urbs[i] = usb_alloc_urb(0, GFP_KERNEL);
		if (!ctx.urbs[i])
			goto free_urbs;
		usb_fill_bulk_urb(ctx.urbs[i], udev, pipe, buf, size,
				unlink_queued_callback, &ctx);
		ctx.urbs[i]->transfer_dma = buf_dma;
		ctx.urbs[i]->transfer_flags = URB_NO_TRANSFER_DMA_MAP;

		if (usb_pipeout(ctx.urbs[i]->pipe)) {
			simple_fill_buf(dev, ctx.urbs[i]);
			ctx.urbs[i]->transfer_flags |= URB_ZERO_PACKET;
		}
	}

	/* Submit all the URBs and then unlink URBs num - 4 and num - 2. */
	for (i = 0; i < num; i++) {
		atomic_inc(&ctx.pending);
		retval = usb_submit_urb(ctx.urbs[i], GFP_KERNEL);
		if (retval != 0) {
			pr_err("submit urbs[%d] fail %d\n", i, retval);
			atomic_dec(&ctx.pending);
			ctx.status = retval;
			break;
		}
	}
	if (i == num) {
		usb_unlink_urb(ctx.urbs[num - 4]); /* 4: urb number */
		usb_unlink_urb(ctx.urbs[num - 2]); /* 2: urb number */
	} else {
		while (--i >= 0)
			usb_unlink_urb(ctx.urbs[i]);
	}

	if (atomic_dec_and_test(&ctx.pending)) /* The extra count */
		complete(&ctx.complete);
	wait_for_completion(&ctx.complete);
	retval = ctx.status;

free_urbs:
	for (i = 0; i < num; i++)
		usb_free_urb(ctx.urbs[i]);
	kfree(ctx.urbs);
free_buf:
	usb_free_coherent(udev, size, buf, buf_dma);
	return retval;
}

/*-------------------------------------------------------------------------*/

static int verify_not_halted(struct sse_usbtest_dev *tdev, int ep, struct urb *urb)
{
	int	retval;
	u16	status;

	/* shouldn't look or act halted */
	retval = usb_get_status(urb->dev, USB_RECIP_ENDPOINT, ep, &status);
	if (retval < 0) {
		pr_err("ep %02x couldn't get no-halt status, %d\n", ep, retval);
		return retval;
	}
	if (status != 0) {
		pr_err("ep %02x bogus status: %04x != 0\n", ep, status);
		return -EINVAL;
	}
	retval = simple_io(tdev, urb, 1, 0, 0, __func__);
	if (retval != 0)
		return -EINVAL;
	return 0;
}

static int verify_halted(struct sse_usbtest_dev *tdev, int ep, struct urb *urb)
{
	int	retval;
	u16	status;

	/* should look and act halted */
	retval = usb_get_status(urb->dev, USB_RECIP_ENDPOINT, ep, &status);
	if (retval < 0) {
		pr_err("ep %02x couldn't get halt status, %d\n", ep, retval);
		return retval;
	}
	if (status != 1) {
		pr_err("ep %02x bogus status: %04x != 1\n", ep, status);
		return -EINVAL;
	}
	retval = simple_io(tdev, urb, 1, 0, -EPIPE, __func__);
	if ((retval != -EPIPE) && (retval != -EPROTO))
		return -EINVAL;
	retval = simple_io(tdev, urb, 1, 0, -EPIPE, "verify_still_halted");
	if ((retval != -EPIPE) && (retval != -EPROTO))
		return -EINVAL;
	return 0;
}

static int test_halt(struct sse_usbtest_dev *tdev, int ep, struct urb *urb)
{
	int	retval;

	/* shouldn't look or act halted now */
	retval = verify_not_halted(tdev, ep, urb);
	if (retval < 0)
		return retval;

	/* set halt (protocol test only), verify it worked */
	retval = usb_control_msg(urb->dev, usb_sndctrlpipe(urb->dev, 0),
			USB_REQ_SET_FEATURE, USB_RECIP_ENDPOINT,
			USB_ENDPOINT_HALT, ep,
			NULL, 0, USB_CTRL_SET_TIMEOUT);
	if (retval < 0) {
		pr_err("ep %02x couldn't set halt, %d\n", ep, retval);
		return retval;
	}
	retval = verify_halted(tdev, ep, urb);
	if (retval < 0) {
		int ret;

		/* clear halt anyways, else further tests will fail */
		ret = usb_clear_halt(urb->dev, urb->pipe);
		if (ret)
			pr_err("ep %02x couldn't clear halt, %d\n", ep, ret);

		return retval;
	}

	/* clear halt (tests API + protocol), verify it worked */
	retval = usb_clear_halt(urb->dev, urb->pipe);
	if (retval < 0) {
		pr_err("ep %02x couldn't clear halt, %d\n", ep, retval);
		return retval;
	}
	retval = verify_not_halted(tdev, ep, urb);
	if (retval < 0)
		return retval;

	/* NOTE:  could also verify SET_INTERFACE clear halts ... */

	return 0;
}

int halt_simple(struct sse_usbtest_dev *dev, bool is_bulk, u32 len)
{
	int			ep;
	int			retval = 0;
	struct urb *urb = NULL;
	struct usb_device *udev = testdev_to_usbdev(dev);
	u8			interval = 0;

	if (!is_bulk)
		interval = dev->desc[SS_INTR_OUT_EP_IDX]->bInterval;

	urb = simple_alloc_urb_no_dma(udev, 0, len, interval);
	if (urb == NULL)
		return -ENOMEM;

	if (is_bulk) {
		if (dev->pipe[SS_BULK_IN_EP_IDX]) {
			ep = usb_pipeendpoint(dev->pipe[SS_BULK_IN_EP_IDX])
					| USB_DIR_IN;
			urb->pipe = dev->pipe[SS_BULK_IN_EP_IDX];
			retval = test_halt(dev, ep, urb);
			if (retval < 0)
				goto done;
		}

		if (dev->pipe[SS_BULK_OUT_EP_IDX]) {
			ep = usb_pipeendpoint(dev->pipe[SS_BULK_OUT_EP_IDX]);
			urb->pipe = dev->pipe[SS_BULK_OUT_EP_IDX];
			retval = test_halt(dev, ep, urb);
		}
	} else {
		if (dev->pipe[SS_INTR_IN_EP_IDX]) {
			ep = usb_pipeendpoint(dev->pipe[SS_INTR_IN_EP_IDX])
					| USB_DIR_IN;
			urb->pipe = dev->pipe[SS_INTR_IN_EP_IDX];
			retval = test_halt(dev, ep, urb);
			if (retval < 0)
				goto done;
		}

		if (dev->pipe[SS_INTR_OUT_EP_IDX]) {
			ep = usb_pipeendpoint(dev->pipe[SS_INTR_OUT_EP_IDX]);
			urb->pipe = dev->pipe[SS_INTR_OUT_EP_IDX];
			retval = test_halt(dev, ep, urb);
		}
	}

done:
	simple_free_urb(urb);
	return retval;
}

/*
 * Control OUT tests use the vendor control requests from Intel's
 * USB 2.0 compliance test device:  write a buffer, read it back.
 *
 * Intel's spec only _requires_ that it work for one packet, which
 * is pretty weak.   Some HCDs place limits here; most devices will
 * need to be able to handle more than one OUT data packet.  We'll
 * try whatever we're told to try.
 */
int ctrl_out(struct sse_usbtest_dev *dev,
		unsigned count, unsigned length,
		unsigned vary, unsigned offset,
		bool is_babble)
{
	unsigned		i, j, len;
	int			retval;
	u8 *buf = NULL;
	char *what = "?";
	struct usb_device *udev = NULL;

	if (length < 1 || length > 0xffff || vary >= length)
		return -EINVAL;

	buf = kmalloc(length + offset, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	buf += offset;
	udev = testdev_to_usbdev(dev);
	len = length;
	retval = 0;

	/*
	 * NOTE:  hardware might well act differently if we pushed it
	 * with lots back-to-back queued requests.
	 */
	for (i = 0; i < count; i++) {
		/* write patterned data */
		for (j = 0; j < len; j++)
			buf[j] = (u8)(i + j);
		retval = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
			is_babble ? SS_CONTROL_WRITE_BABBLE : SS_CONTROL_WRITE,
			USB_DIR_OUT | USB_TYPE_VENDOR, 0, 0,
			buf, len, USB_CTRL_SET_TIMEOUT);
		if (retval != len) {
			what = "write";
			if (retval >= 0) {
				pr_err("ctrl_out, wlen %d (expected %d)\n",
						retval, len);
				retval = -EBADMSG;
			}
			break;
		}

		/* read it back -- assuming nothing intervened!!  */
		retval = usb_control_msg(udev, usb_rcvctrlpipe(udev, 0),
			is_babble ? SS_CONTROL_READ_BABBLE : SS_CONTROL_READ,
			USB_DIR_IN | USB_TYPE_VENDOR, 0, 0,
			buf, len, USB_CTRL_GET_TIMEOUT);
		if (retval != len) {
			what = "read";
			if (retval >= 0) {
				pr_err("ctrl_out, rlen %d (expected %d)\n",
						retval, len);
				retval = -EBADMSG;
			}
			break;
		}

		/* fail if we can't verify */
		for (j = 0; j < len; j++) {
			if (buf[j] != (u8)(i + j)) {
				pr_err("ctrl_out, byte %d is %d not %d\n",
					j, buf[j], (u8)(i + j));
				retval = -EBADMSG;
				break;
			}
		}
		if (retval < 0) {
			what = "verify";
			break;
		}

		len += vary;

		/*
		 * [real world] the "zero bytes IN" case isn't really used.
		 * hardware can easily trip up in this weird case, since its
		 * status stage is IN, not OUT like other ep0in transfers.
		 */
		if (len > length)
			len = realworld ? 1 : 0;
	}

	if (retval < 0)
		pr_err("ctrl_out %s failed, code %d, count %d\n",
			what, retval, i);

	kfree(buf - offset);
	return retval;
}

/*
 * ISO/BULK tests ... mimics common usage
 *  - buffer length is split into N packets (mostly maxpacket sized)
 *  - multi-buffers according to sglen
 */

struct transfer_context {
	unsigned		count;
	unsigned		pending;
	spinlock_t		lock;
	struct completion	done;
	int			submit_error;
	unsigned long		errors;
	unsigned long		packet_count;
	struct sse_usbtest_dev *dev;
	bool			is_iso;
};

static void complicated_callback(struct urb *urb)
{
	struct transfer_context *ctx = urb->context;

	spin_lock(&ctx->lock);
	ctx->count--;

	ctx->packet_count += urb->number_of_packets;
	if (urb->error_count > 0)
		ctx->errors += urb->error_count;
	else if (urb->status != 0)
		ctx->errors += (ctx->is_iso ? urb->number_of_packets : 1);
	else if (urb->actual_length != urb->transfer_buffer_length)
		ctx->errors++;
	else if (usb_pipein(urb->pipe) && simple_check_buf(ctx->dev, urb) != 0)
		ctx->errors++;

	if (urb->status == 0 && ctx->count > (ctx->pending - 1)
			&& !ctx->submit_error) {
		int status = usb_submit_urb(urb, GFP_ATOMIC);
		switch (status) {
		case 0:
			goto done;
		default:
			pr_err("resubmit err %d\n", status);
			/* FALLTHROUGH */
		case -ENODEV: /* disconnected */
		case -ESHUTDOWN: /* endpoint disabled */
			ctx->submit_error = 1;
			break;
		}
	}

	ctx->pending--;
	if (ctx->pending == 0) {
		if (ctx->errors)
			pr_err("during the test, %lu errors out of %lu\n",
				ctx->errors, ctx->packet_count);
		complete(&ctx->done);
	}
done:
	spin_unlock(&ctx->lock);
}

static struct urb *iso_alloc_urb(
	struct usb_device *udev,
	int			pipe,
	struct usb_endpoint_descriptor *desc,
	long			bytes,
	unsigned offset
)
{
	struct urb *urb = NULL;
	unsigned		i, maxp, packets;

	if (bytes < 0 || !desc)
		return NULL;
	maxp = 0x7ff & usb_endpoint_maxp(desc);
	maxp *= usb_endpoint_maxp_mult(desc);
	packets = DIV_ROUND_UP(bytes, maxp);

	urb = usb_alloc_urb(packets, GFP_KERNEL);
	if (!urb)
		return urb;
	urb->dev = udev;
	urb->pipe = pipe;

	urb->number_of_packets = packets;
	urb->transfer_buffer_length = bytes;
	urb->transfer_buffer = usb_alloc_coherent(udev,
					bytes + offset, GFP_KERNEL,
					&urb->transfer_dma);
	if (!urb->transfer_buffer) {
		usb_free_urb(urb);
		return NULL;
	}
	if (offset) {
		memset(urb->transfer_buffer, GUARD_BYTE, offset);
		urb->transfer_buffer += offset;
		urb->transfer_dma += offset;
	}
	/* For inbound transfers use guard byte so that test fails if
		data not correctly copied */
	memset(urb->transfer_buffer,
			usb_pipein(urb->pipe) ? GUARD_BYTE : 0,
			bytes);

	for (i = 0; i < packets; i++) {
		/* here, only the last packet will be short */
		urb->iso_frame_desc[i].length = min((unsigned) bytes, maxp);
		bytes -= urb->iso_frame_desc[i].length;

		urb->iso_frame_desc[i].offset = maxp * i;
	}

	urb->complete = complicated_callback;
	/* urb->context = SET BY CALLER */
	urb->interval = 1 << (desc->bInterval - 1);
	urb->transfer_flags = URB_ISO_ASAP | URB_NO_TRANSFER_DMA_MAP;
	return urb;
}

static int _test_queue_submit_urb(struct sse_usbtest_dev *dev,
					struct urb **urbs, uint32_t urb_num,
					struct transfer_context *context)
{
	unsigned		i;
	int			status;

	for (i = 0; i < urb_num; i++) {
		++context->pending;
		status = usb_submit_urb(urbs[i], GFP_ATOMIC);
		if (status < 0) {
			pr_err("submit iso[%d], error %d\n", i, status);
			if (i == 0)
				goto fail;

			simple_free_urb(urbs[i]);
			urbs[i] = NULL;
			context->pending--;
			context->submit_error = 1;
			break;
		}
	}
	return 0;

fail:
	for (i = 0; i < urb_num; i++) {
		if (urbs[i])
			simple_free_urb(urbs[i]);
	}
	return status;
}

int test_queue(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param,
			int pipe, struct usb_endpoint_descriptor *desc,
			unsigned offset)
{
	struct transfer_context	context;
	struct usb_device *udev = NULL;
	unsigned		i;
	unsigned long		packets = 0;
	int			status = 0;
	struct urb *urbs[param->sglen];

	if (!param->sglen || param->iterations > UINT_MAX / param->sglen)
		return -EINVAL;

	memset(&context, 0, sizeof(context));
	context.count = param->iterations * param->sglen;
	context.dev = dev;
	context.is_iso = !!desc;
	init_completion(&context.done);
	spin_lock_init(&context.lock);

	udev = testdev_to_usbdev(dev);

	for (i = 0; i < param->sglen; i++) {
		if (context.is_iso)
			urbs[i] = iso_alloc_urb(udev, pipe, desc,
					param->length, offset);
		else
			urbs[i] = complicated_alloc_urb_no_dma(udev, pipe,
					param->length, 0);

		if (!urbs[i]) {
			status = -ENOMEM;
			goto fail;
		}
		simple_fill_buf(dev, urbs[i]);
		packets += urbs[i]->number_of_packets;
		urbs[i]->context = &context;
	}
	packets *= param->iterations;

	if (context.is_iso) {
		pr_err("iso period %u %sframes, wMaxPacket %d, transactions: %d\n",
			(unsigned)(1 << (desc->bInterval - 1)),
			(udev->speed == USB_SPEED_HIGH) ? "micro" : "",
			usb_endpoint_maxp(desc),
			usb_endpoint_maxp_mult(desc));

		pr_err("total %lu msec (%lu packets)\n",
			(packets * (unsigned)(1 << (desc->bInterval - 1))) /
				((udev->speed == USB_SPEED_HIGH) ? 8 : 1), /* 8: for 125us, 1 for 1ms */
			packets);
	}

	spin_lock_irq(&context.lock);
	status = _test_queue_submit_urb(dev, urbs, param->sglen, &context);
	spin_unlock_irq(&context.lock);

	if (status)
		goto exit;

	wait_for_completion(&context.done);

fail:
	for (i = 0; i < param->sglen; i++) {
		if (urbs[i])
			simple_free_urb(urbs[i]);
	}

exit:
	/*
	 * Isochronous transfers are expected to fail sometimes.  As an
	 * arbitrary limit, we will report an error if any submissions
	 * fail or if the transfer failure rate is > 10%.
	 */
	if (status != 0)
		;
	else if (context.submit_error)
		status = -EACCES;
	else if (context.errors >
			(context.is_iso ? context.packet_count / 10 : 0)) /* 10,0: packet_count */
		status = -EIO;

	return status;
}

int test_unaligned_bulk(
			struct sse_usbtest_dev *tdev,
			int pipe,
			unsigned length,
			int iterations,
			unsigned transfer_flags,
			const char *label)
{
	int retval;
	struct urb *urb = sse_usbtest_alloc_urb(testdev_to_usbdev(tdev),
			pipe, length, transfer_flags, 1, 0, simple_callback);

	if (!urb)
		return -ENOMEM;

	retval = simple_io(tdev, urb, iterations, 0, 0, label);
	simple_free_urb(urb);
	return retval;
}

int sse_set_clear_halt(struct sse_usbtest_dev *dev,
				struct usb_device *udev,
				uint32_t ep_idx, bool is_set)
{
	int retval, ep;

	if (ep_idx >= SS_EP_NUM)
		return -EOPNOTSUPP;

	if (!dev->pipe[ep_idx])
		return -EOPNOTSUPP;

	if (usb_pipeisoc(dev->pipe[ep_idx]))
		return -EOPNOTSUPP;

	ep = usb_pipeendpoint(dev->pipe[ep_idx]);

	if (usb_pipein(dev->pipe[ep_idx]))
		ep |= USB_DIR_IN;

	if (is_set) {
		/* set halt (protocol test only), verify it worked */
		retval = usb_control_msg(udev, usb_sndctrlpipe(udev, 0),
				USB_REQ_SET_FEATURE, USB_RECIP_ENDPOINT,
				USB_ENDPOINT_HALT, ep,
				NULL, 0, USB_CTRL_SET_TIMEOUT);
		if (retval < 0)
			pr_err("ep %02x couldn't set halt, %d\n", ep, retval);
	} else {
		retval = usb_clear_halt(udev, dev->pipe[ep_idx]);
		if (retval < 0)
			pr_err("ep %02x couldn't clear halt, %d\n", ep, retval);
	}

	return retval;
}
