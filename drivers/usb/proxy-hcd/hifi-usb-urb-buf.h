/*
 * hifi-usb-urb-buf.h
 *
 * utilityies for operating urb buffers
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

#ifndef _HIFI_USB_URB_BUF_H_
#define _HIFI_USB_URB_BUF_H_

#include <linux/types.h>

#define URB_BUF_NUM 8

struct urb_buffers {
	unsigned int urb_buf_num;
	unsigned int urb_buf_len;
	void *urb_buf[URB_BUF_NUM];
	dma_addr_t urb_buf_dma[URB_BUF_NUM];
	unsigned long urb_buf_bitmap;
};

void free_urb_buf(struct urb_buffers *urb_bufs, const void *addr);
void *alloc_urb_buf(struct urb_buffers *urb_bufs, dma_addr_t *dma_addr);
int urb_buf_init(struct urb_buffers *urb_bufs);
void urb_buf_destroy(struct urb_buffers *urb_bufs);
void urb_buf_reset(struct urb_buffers *urb_bufs);

#endif /* _HIFI_USB_URB_BUF_H_ */
