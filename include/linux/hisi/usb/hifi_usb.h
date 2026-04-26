/*
 * hifi_usb.h
 *
 * Hisilicon hifi usb interface defination
 *
 * Copyright (c) 2017-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#ifndef _LINUX_HIFI_USB_H_
#define _LINUX_HIFI_USB_H_
#include <linux/usb.h>

#ifdef CONFIG_USB_PROXY_HCD
/**
 * switch to hifi usb.
 * return 0 means the request was accepted, others means rejected.
 */
int usb_start_hifi_usb(void);

/**
 * Similarly to the usb_start_hifi_usb, with vbus power-off and power-on.
 */
int usb_start_hifi_usb_reset_power(void);

/**
 * switch to AP usb host from hifi usb.
 */
void usb_stop_hifi_usb(void);
void usb_stop_hifi_usb_reset_power(void);

/**
 * Wether a usb_device using hifi usb.
 * return true means the usb device is using hifi usb, others means the usb
 * device is not using hifi usb.
 */
bool usb_using_hifi_usb(struct usb_device *udev);

/**
 * Start hifi usb, but will not configure the usb phy.
 * This function should used only by hisi usb.
 */
int start_hifi_usb(void);

/**
 * Stop the hifi usb, but will not configure the usb phy.
 * This function should used only by hisi usb.
 */
void stop_hifi_usb(void);

/**
 * Rest hifi usb, including reset hifi usb states and freeing its resources.
 * This function should used only by hisi usb when switch to poweroff mode.
 */
void reset_hifi_usb(void);

int get_never_hifi_usb_value(void);

int get_usbaudio_nv_is_ready(void);

int get_always_hifi_usb_value(void);

int get_hifi_usb_retry_count(void);

void export_usbhid_key_pressed(struct usb_device *udev, bool key_pressed);
void hifi_usb_hifi_reset_inform(void);
#else
static inline int usb_start_hifi_usb(void)
{
	return -1;
}
static inline int usb_start_hifi_usb_reset_power(void)
{
	return -1;
}
static inline void usb_stop_hifi_usb(void)
{
}
static inline bool usb_using_hifi_usb(struct usb_device *udev)
{
	return false;
}
static inline int start_hifi_usb(void)
{
	return -1;
}
static inline void stop_hifi_usb(void)
{
}
static inline void reset_hifi_usb(void)
{
}
static inline int get_never_hifi_usb_value(void)
{
	return 0;
}
static inline int get_always_hifi_usb_value(void)
{
	return 0;
}
static inline int get_hifi_usb_retry_count(void)
{
	return 0;
}
static inline int get_usbaudio_nv_is_ready(void)
{
	return 1;
}
static inline void export_usbhid_key_pressed(struct usb_device *udev, bool key_pressed)
{
}
static inline void hifi_usb_hifi_reset_inform(void)
{
}
#endif /* CONFIG_USB_PROXY_HCD */


/* parameters passed to check_hifi_usb_status */
enum hifi_usb_status_trigger {
	HIFI_USB_AUDIO = 1,
	HIFI_USB_TCPC,
	HIFI_USB_FB,
	HIFI_USB_PROXY,
	HIFI_USB_URB_ENQUEUE,
};

#ifdef CONFIG_USB_PROXY_HCD_HIBERNATE
int hifi_usb_hibernate(void);

int hifi_usb_revive(void);

/* This function is used to make sure hifiusb alive. If hifiusb hibernated,
 * revive it. */
int check_hifi_usb_status(enum hifi_usb_status_trigger trigger);
void check_customized_earphone_device(struct usb_device *dev);
#else
static inline int hifi_usb_hibernate(void)
{
	return -1;
}
static inline int hifi_usb_revive(void)
{
	return -1;
}
static inline int check_hifi_usb_status(enum hifi_usb_status_trigger trigger)
{
	return -1;
}
static inline void check_customized_earphone_device(struct usb_device *dev)
{
}
#endif

#ifdef CONFIG_SND
extern int usbaudio_nv_is_ready(void);
#else
static inline int usbaudio_nv_is_ready(void)
{
	return 1;
}
#endif

#endif /* _LINUX_HIFI_USB_H_ */
