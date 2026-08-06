/*
 * hifi-usb-hibernate.h
 *
 * utilityies for hifi-usb hibernate
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

#ifndef _HIFI_USB_HIBERNATE_H_
#define _HIFI_USB_HIBERNATE_H_

enum hibernation_ctrl_type {
	USB_CTRL = 0,
	FB_CTRL = 1,
	AUDIO_CTRL = 2 /* corresponding bit set to 1 means allowing hibernate */
};

enum hibernation_policy {
	HIFI_USB_HIBERNATION_ALLOW = 0,
	HIFI_USB_HIBERNATION_FORBID,
	HIFI_USB_HIBERNATION_FORCE,
};

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
void hifi_usb_hibernation_ctrl(enum hibernation_ctrl_type type, bool set);
void hifi_usb_hibernation_init(struct hifi_usb_proxy *hifiusb);
void hifi_usb_hibernation_exit(struct hifi_usb_proxy *hifiusb);
static inline bool hifi_usb_in_hibernation(struct hifi_usb_proxy *hifiusb)
{
	if (hifiusb->hibernation_support && hifiusb->hibernation_state)
		return true;

	return false;
}
#else
static inline void hifi_usb_hibernation_ctrl(enum hibernation_ctrl_type type,
			       bool set)
{
}
static inline void hifi_usb_hibernation_init(struct hifi_usb_proxy *hifiusb)
{
}
static inline void hifi_usb_hibernation_exit(struct hifi_usb_proxy *hifiusb)
{
}
static inline void hifi_usb_hibernation_state(struct hifi_usb_proxy *hifiusb)
{
}
#endif /* CONFIG_USB_PROXY_HCD_HIBERNATE */

#endif /* _HIFI_USB_HIBERNATE_H_ */
