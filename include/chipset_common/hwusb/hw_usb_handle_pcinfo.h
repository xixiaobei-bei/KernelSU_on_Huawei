/*
 * hw_usb_handle_pcinfo.h
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

#ifndef _HW_USB_HANDLE_PCINFO_H_
#define _HW_USB_HANDLE_PCINFO_H_

#include <linux/kobject.h>
#include <linux/device.h>
#include <linux/stat.h>
#include <linux/fs.h>

/* USB vendor customized request to handle pcinfo */
#define USB_REQ_VENDOR_PCINFO 0x06

void hw_usb_handle_pcinfo(u16 w_value, u16 w_index, u16 w_length);
int hw_pcinfo_create_device(struct device *dev, struct class *usb_class);
void hw_pcinfo_get_device(struct device *dev);
#endif /* _HW_USB_HANDLE_PCINFO_H_ */
