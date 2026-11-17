/*
 * sourcesink_ext_core.h
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

#ifndef SOURCESINK_EXT_CORE_H
#define SOURCESINK_EXT_CORE_H

#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/scatterlist.h>
#include <linux/usb.h>
#include <linux/workqueue.h>

#define SIMPLE_IO_TIMEOUT 10000 /* in milliseconds */

/* Parameter for sse_usbtest driver. */
struct sse_usbtest_param_32 {
	/* inputs */
	__u32		test_num; /* 0..(TEST_CASES-1) */
	__u32		iterations;
	__u32		length;
	__u32		vary;
	__u32		sglen;

	/* output */
	int		ret;
	__s32		duration_sec;
	__s32		duration_usec;
};

/*
 * Compat parameter to the sse_usbtest driver.
 * This supports older user space binaries compiled with 64 bit compiler.
 */
struct sse_usbtest_param_64 {
	/* inputs */
	__u32		test_num; /* 0..(TEST_CASES-1) */
	__u32		iterations;
	__u32		length;
	__u32		vary;
	__u32		sglen;

	/* output */
	int		ret;
	__s64		duration_sec;
	__s64		duration_usec;
};

/* IOCTL interface to the driver. */
#define SSE_USBTEST_REQUEST_32	_IOWR('U', 100, struct sse_usbtest_param_32)
/* COMPAT IOCTL interface to the driver. */
#define SSE_USBTEST_REQUEST_64	_IOWR('U', 100, struct sse_usbtest_param_64)

#define SSE_USBTEST_REQUEST_ALT	_IOWR('U', 101, int)
#define SSE_USBTEST_REQUEST_PATTERN	_IOWR('U', 102, uint32_t)
#define SSE_USBTEST_REQUEST_SET_HALT	_IOWR('U', 103, uint32_t)
#define SSE_USBTEST_REQUEST_CLEAR_HALT	_IOWR('U', 104, uint32_t)

/*-------------------------------------------------------------------------*/

struct sse_usbtest_info {
	const char *name;
	unsigned		pattern;
	int			alt;
};

/*
 * this is accessed only through usbfs ioctl calls.
 * one ioctl to issue a test ... one lock per device.
 * tests create other threads if they need them.
 * urbs and buffers are allocated dynamically,
 * and data generated deterministically.
 */
struct sse_usbtest_dev {
	struct usb_interface *intf;
	struct sse_usbtest_info *info;
#define SS_EP_NUM 6
#define SS_BULK_IN_EP_IDX 0
#define SS_BULK_OUT_EP_IDX 1
#define SS_INTR_IN_EP_IDX 2
#define SS_INTR_OUT_EP_IDX 3
#define SS_ISOC_IN_EP_IDX 4
#define SS_ISOC_OUT_EP_IDX 5
	struct usb_endpoint_descriptor *desc[SS_EP_NUM];
	int				pipe[SS_EP_NUM];
	int				cur_alt;
	struct sse_usbtest_param_32	param;
	struct work_struct		handler_work;
	struct mutex			lock;

#define TBUF_SIZE	256
	u8 *buf;
};

static struct usb_device *testdev_to_usbdev(struct sse_usbtest_dev *test)
{
	return interface_to_usbdev(test->intf);
}

#define INTERRUPT_RATE		1 /* msec/transfer */

#undef pr_fmt
#define pr_fmt(fmt) "[SSE_CLASS]%s: " fmt, __func__

#define unused(x) (void)(x)

/* user app could sent test case which case num=200 to get test result */
#define SSE_GET_RES_CASE_NUM 200

#define SS_CONTROL_WRITE 0x5b
#define SS_CONTROL_READ 0x5c
#define SS_CONTROL_WRITE_BABBLE 0x5d
#define SS_CONTROL_READ_BABBLE 0x5e


#define GUARD_BYTE	0xA5
#define MAX_SGLEN	128

int get_endpoints(struct sse_usbtest_dev *dev, struct usb_interface *intf);
struct urb *simple_alloc_urb_no_dma(
		struct usb_device *udev,
		int pipe,
		unsigned long bytes,
		u8 binterval);
int simple_io(struct sse_usbtest_dev *tdev, struct urb *urb,
		int iterations,
		int vary,
		int expected,
		const char *label);
void simple_free_urb(struct urb *urb);
struct scatterlist *alloc_sglist(int nents, int max, int vary,
		struct sse_usbtest_dev *dev, int pipe);
int perform_sglist(struct sse_usbtest_dev *tdev,
		unsigned iterations,
		int pipe,
		struct usb_sg_request *req,
		struct scatterlist *sg,
		int nents);
void free_sglist(struct scatterlist *sg, int nents);
int ch9_postconfig(struct sse_usbtest_dev *dev);
int test_ctrl_queue(struct sse_usbtest_dev *dev,
			struct sse_usbtest_param_32 *param);
int unlink_simple(struct sse_usbtest_dev *dev, int pipe,
			int len, u8 interval);
int unlink_queued(struct sse_usbtest_dev *dev, int pipe,
			unsigned num, unsigned size);
int halt_simple(struct sse_usbtest_dev *dev, bool is_bulk, u32 len);
int ctrl_out(struct sse_usbtest_dev *dev,
		unsigned count, unsigned length,
		unsigned vary, unsigned offset,
		bool is_babble);
int test_queue(struct sse_usbtest_dev *dev,
		struct sse_usbtest_param_32 *param,
		int pipe, struct usb_endpoint_descriptor *desc,
		unsigned offset);
int test_unaligned_bulk(struct sse_usbtest_dev *tdev, int pipe,
		unsigned length,
		int iterations,
		unsigned transfer_flags,
		const char *label);
int sse_set_clear_halt(struct sse_usbtest_dev *dev,
		struct usb_device *udev,
		uint32_t ep_idx, bool is_set);
int set_altsetting(struct sse_usbtest_dev *dev, int alternate);

#endif
