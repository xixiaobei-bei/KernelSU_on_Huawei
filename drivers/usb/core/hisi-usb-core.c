#include <linux/jiffies.h>
#include <linux/mutex.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/usb.h>
#include "hub.h"
#include "hisi-usb-core.h"

int usb_device_read_mutex_trylock(void)
{
	unsigned long jiffies_expire = jiffies + HZ;

	while (!mutex_trylock(&usb_bus_idr_lock)) {

		/* If we can't acquire the lock after waiting one second,
		 * we're probably deadlocked */
		if (time_after(jiffies, jiffies_expire)) {
			pr_err("%s:get usb_bus_idr_lock timeout, probably deadlocked\n",
					__func__);
			return -EFAULT;
		}
		msleep(20);
	}

	return 0;
}

int usb_device_read_usb_trylock_device(struct usb_device *udev)
{
	unsigned long jiffies_expire = jiffies + HZ;

	while (!usb_trylock_device(udev)) {
		/* If we can't acquire the lock after waiting one second,
		 * we're probably deadlocked */
		if (time_after(jiffies, jiffies_expire)) {
			return -EFAULT;
		}
		msleep(20);
	}

	return 0;
}

#ifdef CONFIG_HUAWEI_DOCK_HEADSET_QUIRK

#include <huawei_platform/usb/hw_pd_dev.h>

#define HUAWEI_DOCK_VID 0x0bda
#define HUAWEI_DOCK_PID 0x5411

bool check_huawei_dock_quirk(struct usb_device *hdev,
		struct usb_hub *hub, int port1)
{
	struct usb_port *port2_dev = NULL;

	if (unlikely(port1 == 3 &&
                hdev->descriptor.idVendor == HUAWEI_DOCK_VID &&
				hdev->descriptor.idProduct == HUAWEI_DOCK_PID &&
				pd_dpm_get_hw_dock_svid_exist())) {
		port2_dev = hub->ports[1];
		if (port2_dev->child)
			return false;
	}

	return true;
}

#endif /* CONFIG_HUAWEI_DOCK_HEADSET_QUIRK */
