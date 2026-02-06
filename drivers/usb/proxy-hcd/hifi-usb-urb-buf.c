/*
 * hifi-usb-urb-buf.c
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

#include "hifi-usb-urb-buf.h"

#include <linux/io.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/types.h>

#include "dsp_misc.h"

#define HIFI_USB_SHARE_BUFFER_ADDR HIFI_USB_DRIVER_SHARE_MEM_ADDR
#define HIFI_USB_SHARE_BUFFER_LEN HIFI_USB_DRIVER_SHARE_MEM_SIZE

void free_urb_buf(struct urb_buffers *urb_bufs, const void *addr)
{
	unsigned int index;

	if ((urb_bufs == NULL) || (addr == NULL) || (urb_bufs->urb_buf_len == 0))
		return;

	index = (addr - urb_bufs->urb_buf[0]) / urb_bufs->urb_buf_len;
	if (index < urb_bufs->urb_buf_num)
		clear_bit(index, &urb_bufs->urb_buf_bitmap);
	else
		WARN_ON(1);
}

void *alloc_urb_buf(struct urb_buffers *urb_bufs, dma_addr_t *dma_addr)
{
	unsigned int index;

	if ((urb_bufs == NULL) || (dma_addr == NULL))
		return NULL;

	while (1) {
		index = (unsigned int)find_first_zero_bit(&urb_bufs->urb_buf_bitmap,
				urb_bufs->urb_buf_num);
		if (unlikely(index >= urb_bufs->urb_buf_num))
			return NULL;

		if (!test_and_set_bit(index, &urb_bufs->urb_buf_bitmap))
			break;
	}

	*dma_addr = urb_bufs->urb_buf_dma[index];
	return urb_bufs->urb_buf[index];
}

int urb_buf_init(struct urb_buffers *urb_bufs)
{
	unsigned int i;

	if (urb_bufs == NULL)
		return -ENOMEM;

	urb_bufs->urb_buf_num = URB_BUF_NUM;
	urb_bufs->urb_buf_len = HIFI_USB_SHARE_BUFFER_LEN / URB_BUF_NUM;

	if ((urb_bufs->urb_buf_num * urb_bufs->urb_buf_len) > HIFI_USB_SHARE_BUFFER_LEN)
		urb_bufs->urb_buf_num = HIFI_USB_SHARE_BUFFER_LEN / urb_bufs->urb_buf_len;

	if (urb_bufs->urb_buf_num > (sizeof(urb_bufs->urb_buf_bitmap) * URB_BUF_NUM))
		urb_bufs->urb_buf_num = sizeof(urb_bufs->urb_buf_bitmap) * URB_BUF_NUM;

	urb_bufs->urb_buf_dma[0] = 0; /* caution: this value is offset */
	urb_bufs->urb_buf[0] = ioremap(HIFI_USB_SHARE_BUFFER_ADDR, HIFI_USB_SHARE_BUFFER_LEN);
	if (urb_bufs->urb_buf[0] == NULL)
		return -ENOMEM;

	for (i = 1; i < urb_bufs->urb_buf_num; i++) {
		urb_bufs->urb_buf_dma[i] = urb_bufs->urb_buf_dma[i - 1] + urb_bufs->urb_buf_len;
		urb_bufs->urb_buf[i] = urb_bufs->urb_buf[i - 1] + urb_bufs->urb_buf_len;
	}

	urb_bufs->urb_buf_bitmap = 0;

	return 0;
}

void urb_buf_destroy(struct urb_buffers *urb_bufs)
{
	if (urb_bufs == NULL)
		return;
	urb_bufs->urb_buf_bitmap = ~0UL;
	iounmap(urb_bufs->urb_buf[0]);
}

void urb_buf_reset(struct urb_buffers *urb_bufs)
{
	if (urb_bufs == NULL)
		return;

	urb_bufs->urb_buf_bitmap = 0;
}
