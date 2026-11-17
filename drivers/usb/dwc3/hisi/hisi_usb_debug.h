/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Header file or chip_usb_dubug.c.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef _CHIP_USB_DEBUG_H_
#define _CHIP_USB_DEBUG_H_

#include <linux/device.h>
#include <linux/kernel.h>
#include "dwc3-hisi.h"

typedef ssize_t (*hiusb_debug_show_ops)(void *, char *, size_t);
typedef ssize_t (*hiusb_debug_store_ops)(void *, const char *, size_t);

#ifdef CONFIG_HISI_DEBUG_FS
int create_attr_file(struct chip_dwc3_device *chip_dwc3);
void remove_attr_file(struct chip_dwc3_device *chip_dwc3);
#else
static inline int create_attr_file(struct chip_dwc3_device *chip_dwc3)
{
	return 0;
}

static inline void remove_attr_file(struct chip_dwc3_device *chip_dwc3) {}
#endif
#endif /* _CHIP_USB_DEBUG_H_ */
