/*
 * hw_usb_sync_host_time.h
 *
 * header file for usb sync host time
 *
 * Copyright (c) 2019-2021 Huawei Technologies Co., Ltd.
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

#ifndef __DRIVERS_USB_HW_TIMESYNC_H__
#define __DRIVERS_USB_HW_TIMESYNC_H__

#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/interrupt.h> /* For in_interrupt() */
#include <linux/fs.h>
#include <linux/usb/composite.h>
/*
 * Get PC time, the system calls the kernel function, set the system time.
 */
void hw_usb_handle_host_time(struct usb_ep *ep, struct usb_request *req);

/*
 * Initialize the data structure and work queue.
 */
void hw_usb_sync_host_time_init(void);

#define USB_REQ_SEND_HOST_TIME 0xA2

#endif /* __DRIVERS_USB_HW_TIMESYNC_H__ */
