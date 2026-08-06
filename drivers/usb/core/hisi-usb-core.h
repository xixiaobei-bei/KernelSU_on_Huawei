/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: interface defination for usb-core driver
 * Create: 2019-03-04
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _VENDOR_USB_CORE_H_
#define _VENDOR_USB_CORE_H_

#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/usb.h>
#include <linux/hisi/usb/hisi_usb.h>

struct usb_device;
struct usb_hub;

void notify_hub_too_deep(void);
void notify_power_insufficient(void);

int usb_device_read_mutex_trylock(void);
int usb_device_read_usb_trylock_device(struct usb_device *udev);

bool check_vendor_dock_quirk(struct usb_device *hdev,
		struct usb_hub *hub, int port1);

#endif /* _VENDOR_USB_CORE_H_ */
