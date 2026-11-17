/*
 * hid-huawei.c
 *
 * HID driver for some huawei "special" devices
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * either version 2 of that License or (at your option) any later version.
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/device.h>
#include <linux/usb.h>
#include <linux/hid.h>
#include <linux/module.h>

#include "hid-ids.h"

#define huawei_typec_headset_map_key_clear(c) \
	hid_map_usage_clear(hi, usage, bit, max, EV_KEY, (c))

static int huawei_typec_headset_input_mapping(struct hid_device *hdev,
	struct hid_input *hi, struct hid_field *field, struct hid_usage *usage,
	unsigned long **bit, int *max)
{
	if (HID_UP_CONSUMER != (usage->hid & HID_USAGE_PAGE))
		return 0;

	hid_dbg(hdev, "huawei typeC headset input mapping event [0x%x]\n",
		usage->hid & HID_USAGE);

	/* 0x0cd:hid usage code means play/pause */
	switch (usage->hid & HID_USAGE) {
	case 0x0cd:
		huawei_typec_headset_map_key_clear(KEY_MEDIA);
		break;
	default:
		return 0;
	}

	return 1;
}

static int huawei_input_mapping(struct hid_device *hdev, struct hid_input *hi,
	struct hid_field *field, struct hid_usage *usage,
	unsigned long **bit, int *max)
{
	int ret = 0;

	if (hdev->product == USB_DEVICE_ID_HUAWEI_HEADSET)
		ret = huawei_typec_headset_input_mapping(hdev,
			hi, field, usage, bit, max);

	return ret;
}

static int huawei_probe(struct hid_device *hdev,
		const struct hid_device_id *id)
{
	int ret;

	ret = hid_parse(hdev);
	if (ret) {
		hid_err(hdev, "parse failed\n");
		return ret;
	}

	ret = hid_hw_start(hdev, HID_CONNECT_DEFAULT);
	if (ret) {
		hid_err(hdev, "hw start failed\n");
		return ret;
	}

	hid_info(hdev, "huawei typeC headset product 0x%x probe\n",
			hdev->product);
	return 0;
}


static const struct hid_device_id huawei_devices[] = {
	{ HID_USB_DEVICE(USB_VENDOR_ID_HUAWEI, USB_DEVICE_ID_HUAWEI_HEADSET) },
	{ }
};
MODULE_DEVICE_TABLE(hid, samsung_devices);

static struct hid_driver huawei_driver = {
	.name = "huawei",
	.id_table = huawei_devices,
	.input_mapping = huawei_input_mapping,
	.probe = huawei_probe,
};
module_hid_driver(huawei_driver);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Huawei HID special device driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd");
