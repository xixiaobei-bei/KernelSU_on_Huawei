/*
 * hw_controlrequest_handle.h
 *
 * header file for usb port switch
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

#ifndef _HW_CONTROLREQUEST_HANDLE_H_
#define _HW_CONTROLREQUEST_HANDLE_H_

#include <linux/usb/composite.h>
#include <chipset_common/hwusb/hw_usb_rwswitch.h>
#include <chipset_common/hwusb/hw_usb_sync_host_time.h>
#include <chipset_common/hwusb/hw_usb_handle_pcinfo.h>

#ifdef CONFIG_HW_GADGET
extern int hw_ep0_handler(struct usb_composite_dev *cdev,
	const struct usb_ctrlrequest *ctrl);
#else
static inline int hw_ep0_handler(struct usb_composite_dev *cdev,
	const struct usb_ctrlrequest *ctrl)
{
	return -1;
}
#endif /* CONFIG_HW_GADGET */

#endif /* _HW_CONTROLREQUEST_HANDLE_H_ */
