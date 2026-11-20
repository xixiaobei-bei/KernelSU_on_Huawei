/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: hiusb debug framework
 * Create: 2020-08-13
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#ifndef __USB_DEBUG_MODULE_H
#define __USB_DEBUG_MODULE_H

#include <linux/kernel.h>
#include <linux/notifier.h>

/* add usb debug event */
enum usb_debug_event_type {
	USB_GADGET_DEVICE_RESET = 0,
	USB_CORE_HOST_ENUM_ERR,
	USB_CORE_HOST_RESUME_ERR,
	USB_CORE_HOST_TRANS_TIMEOUT,
	USB_DEBUG_EVENT_MAX
};

#ifdef CONFIG_CHIP_USB_DEBUG_MODULE
int usb_atomicerr_register_notify(struct notifier_block *nb);
int usb_atomicerr_unregister_notify(struct notifier_block *nb);
int usb_notify_atomicerr_occur(unsigned int event_type);
int usb_blockerr_register_notify(struct notifier_block *nb);
int usb_blockerr_unregister_notify(struct notifier_block *nb);
int usb_notify_blockerr_occur(unsigned int event_type);
void usb_debug_event_notify(unsigned int event_type);
#else
static inline int usb_atomicerr_register_notify(struct notifier_block *nb)
{
	return 0;
}
static inline int usb_atomicerr_unregister_notify(struct notifier_block *nb)
{
	return 0;
}
static inline int usb_blockerr_register_notify(struct notifier_block *nb)
{
	return 0;
}
static inline int usb_blockerr_unregister_notify(struct notifier_block *nb)
{
	return 0;
}
static inline int usb_notify_blockerr_occur(unsigned int event_type)
{
	return 0;
}
static inline void usb_debug_event_notify(unsigned int event_type)
{
	return;
}
static inline int usb_notify_atomicerr_occur(unsigned int event_type)
{
	return 0;
}
#endif /* HISI_CONFIG_USB_DEBUG_MODULE */

#endif /* __USB_DEBUG_MODULE_H */