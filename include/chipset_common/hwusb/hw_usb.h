/*
 * hw_usb.h
 *
 * usb driver
 *
 * Copyright (c) 2012-2021 Huawei Technologies Co., Ltd.
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

#ifndef _HW_USB_H_
#define _HW_USB_H_

#define HW_USB_STR_MAX_LEN           16

/* event types notify user-space host abnormal event */
enum hw_usb_host_abnormal_event_type {
	USB_HOST_EVENT_NORMAL,
	USB_HOST_EVENT_POWER_INSUFFICIENT,
	USB_HOST_EVENT_HUB_TOO_DEEP,
	USB_HOST_EVENT_UNKNOW_DEVICE
};

enum hw_usb_cable_event_type {
	USB31_CABLE_IN_EVENT, /* usb 3.1 plug in */
	DP_CABLE_IN_EVENT, /* dp plug in */
	ANA_AUDIO_IN_EVENT, /* analog audio plug in */
	USB31_CABLE_OUT_EVENT, /* usb 3.1 plug out */
	DP_CABLE_OUT_EVENT, /* dp plug out */
	ANA_AUDIO_OUT_EVENT, /* analog audio plug out */
};

#define HW_USB_VOTE_OBJECT             "hw_usb"
#define HW_USB_LDO_CTRL_USB            "usb"
#define HW_USB_LDO_CTRL_COMBOPHY       "combphy"
#define HW_USB_LDO_CTRL_DIRECT_CHARGE  "direct_charge"
#define HW_USB_LDO_CTRL_HIFIUSB        "hifiusb"
#define HW_USB_LDO_CTRL_TYPECPD        "typecpd"

struct hw_usb_device {
	struct platform_device *pdev;
	struct device *dev;
	struct device *child_usb_dev;
	struct regulator *usb_phy_ldo; /* usb phy 3.3v ldo */
	unsigned int speed;
	unsigned int abnormal_event;
};

#ifdef CONFIG_HUAWEI_USB
extern void hw_usb_host_abnormal_event_notify(unsigned int event);
extern void hw_usb_set_usb_speed(unsigned int usb_speed);
extern int hw_usb_ldo_supply_enable(const char *client_name);
extern int hw_usb_ldo_supply_disable(const char *client_name);
extern void hw_usb_send_event(enum hw_usb_cable_event_type event);
#else
static inline void hw_usb_host_abnormal_event_notify(unsigned int event)
{
}

static inline void hw_usb_set_usb_speed(unsigned int usb_speed)
{
}

static inline int hw_usb_ldo_supply_enable(const char *client_name)
{
	return 0;
}

static inline int hw_usb_ldo_supply_disable(const char *client_name)
{
	return 0;
}

static inline void hw_usb_send_event(enum hw_usb_cable_event_type event)
{
}
#endif /* CONFIG_HUAWEI_USB */

#endif /* _HW_USB_H_ */
