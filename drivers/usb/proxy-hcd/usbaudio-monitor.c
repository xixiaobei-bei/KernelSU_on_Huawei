/*
 * usbaudio-monitor.c
 *
 * utilityies for monitoring hifi-usb audio
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

#include "hifi-usb.h"
#include "usbaudio-monitor.h"
#include <linux/hisi/usb/hifi_usb.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/module.h>
#include <linux/usb.h>
#include <linux/usb/audio.h>
#include <securec.h>

#define err(format, arg...) pr_err("[usbaudio-minitor]%s: " format, __func__, ##arg)
#define info(format, arg...) pr_info("[usbaudio-minitor]%s: " format, __func__, ##arg)

#define VID_MASK	0xffff0000u
#define PID_MASK	0xffffu
#define LOW_WORD_SHIFT	16

u32 hifi_usb_base_quirk_devices[MAX_QUIRK_DEVICES_ONE_GROUP + 1];
u32 hifi_usb_ext_quirk_devices[MAX_QUIRK_DEVICES_ONE_GROUP + 1];

u32 *get_ptr_usb_base_quirk(void)
{
	return hifi_usb_base_quirk_devices;
}

void init_data_usb_base_quirk(void)
{
	(void)memset_s(hifi_usb_base_quirk_devices, sizeof(hifi_usb_base_quirk_devices), 0, sizeof(hifi_usb_base_quirk_devices));
	return;
}

u32 *get_ptr_usb_ext_quirk(void)
{
	return hifi_usb_ext_quirk_devices;
}

void init_data_usb_ext_quirk(void)
{
	(void)memset_s(hifi_usb_ext_quirk_devices, sizeof(hifi_usb_ext_quirk_devices), 0, sizeof(hifi_usb_ext_quirk_devices));
	return;
}

static bool match_reset_power_devices(struct usb_device *udev)
{
	int i;
	u32 *quirk_id = hifi_usb_base_quirk_devices;

	for (i = 0; *quirk_id && i < MAX_QUIRK_DEVICES_ONE_GROUP; quirk_id++, i++) {
		if (((u16)((*quirk_id & VID_MASK) >> LOW_WORD_SHIFT) ==
				le16_to_cpu(udev->descriptor.idVendor)) &&
		    (((u16)(*quirk_id & PID_MASK) ==
				le16_to_cpu(udev->descriptor.idProduct))))
			return true;
	}

	quirk_id = hifi_usb_ext_quirk_devices;

	for (i = 0; *quirk_id && i < MAX_QUIRK_DEVICES_ONE_GROUP; quirk_id++, i++) {
		if (((u16)((*quirk_id & VID_MASK) >> LOW_WORD_SHIFT) ==
				le16_to_cpu(udev->descriptor.idVendor)) &&
		    (((u16)(*quirk_id & PID_MASK) ==
				le16_to_cpu(udev->descriptor.idProduct))))
			return true;
	}

	return false;
}

static int never_hifi_usb;
static int always_hifi_usb;

#ifdef CONFIG_HISI_DEBUG_FS
int never_use_hifi_usb(int val)
{
	never_hifi_usb = val;
	return val;
}

int always_use_hifi_usb(int val)
{
	always_hifi_usb = val;
	return val;
}
#endif

int get_never_hifi_usb_value(void)
{
	return never_hifi_usb;
}
EXPORT_SYMBOL(get_never_hifi_usb_value);

int get_always_hifi_usb_value(void)
{
	return always_hifi_usb;
}
EXPORT_SYMBOL(get_always_hifi_usb_value);

#ifdef CONFIG_USB_384K_AUDIO_ADAPTER_SUPPORT
bool is_customized_384k_audio_adapter(struct usb_device *udev)
{
	if (udev->parent == NULL) {
		WARN_ON(1);
		return false;
	}
	if (udev->parent->parent != NULL) {
		WARN_ON(1);
		return false;
	}

	if ((le16_to_cpu(udev->descriptor.idVendor) != HUAWEI_EARPHONE_VENDOR_ID)
			|| (le16_to_cpu(udev->descriptor.idProduct) != HUAWEI_EARPHONE_PRODUCT_ID))
		return false;

	if (udev->product) {
		if (strncmp(udev->product, HUAWEI_USB_C_AUDIO_ADAPTER,
			sizeof(HUAWEI_USB_C_AUDIO_ADAPTER)) == 0)
			return true;
	}
	return false;
}
#endif

static bool is_usbaudio_device(struct usb_device *udev, int configuration)
{
	struct usb_host_config *config = NULL;
	int hid_intf_num = 0;
	int audio_intf_num = 0;
	int other_intf_num = 0;
	int nintf;
	int i;

	for (i = 0; i < udev->descriptor.bNumConfigurations; i++) {
		if (udev->config[i].desc.bConfigurationValue == configuration) {
			config = &udev->config[i];
			break;
		}
	}

	if (config == NULL) {
		WARN_ON(true);
		return false;
	}

	info("configuration %d %d\n", configuration, config->desc.bConfigurationValue);

	nintf = config->desc.bNumInterfaces;
	if ((nintf < 0) || (nintf > USB_MAXINTERFACES)) {
		err("nintf invalid %d\n", nintf);
		return false;
	}

	for (i = 0; i < nintf; ++i) {
		struct usb_interface_cache *intfc = NULL;
		struct usb_host_interface *alt = NULL;

		intfc = config->intf_cache[i];
		alt = &intfc->altsetting[0];

		if (alt->desc.bInterfaceClass == USB_CLASS_AUDIO) {
			if (alt->desc.bInterfaceSubClass == USB_SUBCLASS_AUDIOCONTROL)
				audio_intf_num++;
		} else if (alt->desc.bInterfaceClass == USB_CLASS_HID) {
			hid_intf_num++;
		} else {
			other_intf_num++;
		}
	}

	info("audio_intf_num %d, hid_intf_num %d, other_intf_num %d\n",
		audio_intf_num, hid_intf_num, other_intf_num);

	if ((audio_intf_num == 1) && (hid_intf_num <= 1) && (other_intf_num == 0)) {
		info("[%s]this is usb audio device\n", __func__);
		return true;
	}

	return false;
}

static bool is_non_usbaudio_device(struct usb_device *udev, int configuration)
{
	info("\n");

	if (udev->parent == NULL) {
		WARN_ON(1);
		return false;
	}
	if (udev->parent->parent != NULL) {
		WARN_ON(1);
		return false;
	}

	if (configuration <= 0) {
		WARN_ON(1);
		return false;
	}

#ifdef CONFIG_USB_384K_AUDIO_ADAPTER_SUPPORT
	if (is_customized_384k_audio_adapter(udev))
		return false;
#endif
	return !is_usbaudio_device(udev, configuration);
}

bool stop_hifi_usb_when_non_usbaudio(struct usb_device *udev, int configuration)
{
	if (udev == NULL)
		return false;

	if (get_always_hifi_usb_value())
		return false;

	if (is_non_usbaudio_device(udev, configuration)) {
		info("it need call usb_stop_hifi_usb\n");
		if (match_reset_power_devices(udev))
			usb_stop_hifi_usb_reset_power();
		else
			usb_stop_hifi_usb();
		return true;
	}
	return false;
}

static void hifi_usb_notifier_stop(struct usb_device *udev)
{
	int configuration;

	if ((udev != NULL) && (udev->actconfig != NULL)) {
		configuration = udev->actconfig->desc.bConfigurationValue;
		info("configuration %d\n", configuration);
		stop_hifi_usb_when_non_usbaudio(udev, configuration);
	} else {
		/*
		 * Some error happened, host does not
		 * send SET_CONFIGURATION to device.
		 * Just stop hifi usb.
		 */
		info("device has no actconfig, switch to arm usb\n");
		usb_stop_hifi_usb();
	}
}

static void hifi_usb_notifier_start(struct usb_device *udev)
{
	int configuration;

	if ((udev != NULL) && (udev->actconfig != NULL)) {
		configuration = udev->actconfig->desc.bConfigurationValue;
#ifdef CONFIG_USB_384K_AUDIO_ADAPTER_SUPPORT
		if (is_customized_384k_audio_adapter(udev) &&
				!is_usbaudio_device(udev, configuration)) {
			info("%s, to start hifi usb\n", HUAWEI_USB_C_AUDIO_ADAPTER);
			(void)usb_start_hifi_usb();
			return;
		}
#endif
#ifdef CONFIG_HISI_DEBUG_FS
		if (get_always_hifi_usb_value()) {
			info("always_hifi_usb, to start hifi usb\n");
			(void)usb_start_hifi_usb();
		}
#endif
	}
}

static int usb_notifier_call(struct notifier_block *nb,
				unsigned long action, void *data)
{
	struct usb_device *udev = data;

	if (action != USB_DEVICE_ADD)
		return 0;

	if ((udev->parent != NULL) && (udev->parent->parent == NULL)) {
		if (usb_using_hifi_usb(udev))
			hifi_usb_notifier_stop(udev);
		else
			hifi_usb_notifier_start(udev);
	}

	hifi_usb_announce_udev(udev);

	return 0;
}

struct notifier_block usbaudio_monirot_nb = {
	.notifier_call = usb_notifier_call,
};

static int __init usbaudio_monitor_init(void)
{
	usb_register_notify(&usbaudio_monirot_nb);
	return 0;
}

static void __exit usbaudio_monitor_exit(void)
{
	usb_unregister_notify(&usbaudio_monirot_nb);
}

module_init(usbaudio_monitor_init);
module_exit(usbaudio_monitor_exit);
