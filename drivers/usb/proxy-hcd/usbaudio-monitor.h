/*
 * usbaudio-monitor.h
 *
 * utilityies for hifi-usb
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

#ifndef _USBAUDIO_MONITOR_H_
#define _USBAUDIO_MONITOR_H_

#include <linux/stddef.h>
#include <linux/usb.h>

int always_use_hifi_usb(int val);
int never_use_hifi_usb(int val);


bool stop_hifi_usb_when_non_usbaudio(struct usb_device *udev, int configuration);
u32 *get_ptr_usb_base_quirk(void);
u32 *get_ptr_usb_ext_quirk(void);
void init_data_usb_base_quirk(void);
void init_data_usb_ext_quirk(void);

#ifdef CONFIG_USB_384K_AUDIO_ADAPTER_SUPPORT
#define HUAWEI_USB_C_AUDIO_ADAPTER "HUAWEI USB-C TO 3.5MM AUDIO ADAPTER"

#define HUAWEI_EARPHONE_VENDOR_ID	0x12d1
#define HUAWEI_EARPHONE_PRODUCT_ID	0x3a07

bool is_customized_384k_audio_adapter(struct usb_device *udev);
#else
static inline bool is_customized_384k_audio_adapter(struct usb_device *udev)
{
	return false;
}
#endif

#endif /* _USBAUDIO_MONITOR_H_ */
