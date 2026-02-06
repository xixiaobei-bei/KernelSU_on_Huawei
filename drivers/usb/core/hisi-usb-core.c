/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: driver for usb-core
 * Create: 2019-03-04
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include "hisi-usb-core.h"
#include "hub.h"
#include <chipset_common/hwusb/hw_usb.h>

void notify_hub_too_deep(void)
{
	hw_usb_host_abnormal_event_notify(USB_HOST_EVENT_HUB_TOO_DEEP);
}

void notify_power_insufficient(void)
{
	hw_usb_host_abnormal_event_notify(USB_HOST_EVENT_POWER_INSUFFICIENT);
}

int usb_device_read_mutex_trylock(void)
{
	unsigned long jiffies_expire = jiffies + HZ;

	while (!mutex_trylock(&usb_bus_idr_lock)) {
		/*
		 * If we can't acquire the lock after waiting one second,
		 * we're probably deadlocked
		 */
		if (time_after(jiffies, jiffies_expire)) {
			pr_err("%s:get usb_bus_idr_lock timeout, probably deadlocked\n",
				__func__);
			return -EFAULT;
		}
		msleep(20); /* according to system design */
	}

	return 0;
}

int usb_device_read_usb_trylock_device(struct usb_device *udev)
{
	unsigned long jiffies_expire = jiffies + HZ;

	if (!udev)
		return -EFAULT;

	while (!usb_trylock_device(udev)) {
		/*
		 * If we can't acquire the lock after waiting one second,
		 * we're probably deadlocked
		 */
		if (time_after(jiffies, jiffies_expire))
			return -EFAULT;

		msleep(20); /* according to system design */
	}

	return 0;
}

#ifdef CONFIG_USB_DOCK_HEADSET_QUIRK

#include <huawei_platform/usb/hw_pd_dev.h>

#define VENDOR_DOCK_VID 0x0bda
#define VENDOR_DOCK_PID 0x5411

bool check_vendor_dock_quirk(struct usb_device *hdev,
	struct usb_hub *hub, int port1)
{
	struct usb_port *port2_dev = NULL;

	if (!hdev || !hub)
		return false;

	/* port1 is 3 just fix dock quirk condition */
	if (unlikely(port1 == 3 &&
		hdev->descriptor.idVendor == VENDOR_DOCK_VID &&
				hdev->descriptor.idProduct == VENDOR_DOCK_PID &&
				pd_dpm_get_hw_dock_svid_exist())) {
		/* dock quirk check dev(hub port1) */
		port2_dev = hub->ports[1];
		if (port2_dev->child)
			return false;
	}

	return true;
}

#endif /* CONFIG_USB_DOCK_HEADSET_QUIRK */
