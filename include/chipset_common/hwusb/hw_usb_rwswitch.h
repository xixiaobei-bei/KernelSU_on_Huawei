/*
 * hw_usb_rwswitch.h
 *
 * header file for usb port switch
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef __DRIVERS_USB_HW_RWSWITCH_H__
#define __DRIVERS_USB_HW_RWSWITCH_H__

#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/interrupt.h> /* For in_interrupt() */
#include <linux/fs.h>


/* USB vendor customized request to switch USB port mode */
#define USB_REQ_VENDOR_SWITCH_MODE 0xA5

/* USB_PortMode switch index define */
/* set the mode of the usb port to 17: "pcmodem2" */
#define INDEX_ENDUSER_SWITCH 0x11
/* set the mode of the usb port to 14: "manufacture,adb" */
#define INDEX_FACTORY_REWORK 0x0E
/* set the mode of the usb port to 13 */
#define INDEX_USB_REWORK_SN 0x0D

/*
 * submit usb switch request by sending uevent.
 *
 * Notice : return -1 is exception,other is normal.
 */
int hw_usb_port_switch_request(int usb_switch_index);

int hw_usb_port_mode_get(void);

int hw_rwswitch_create_device(struct device *dev, struct class *usb_class);

void hw_usb_get_device(struct device *dev);

#endif /* __DRIVERS_USB_HW_RWSWITCH_H__ */
