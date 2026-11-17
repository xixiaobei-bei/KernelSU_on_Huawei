/*
 * hw_usb_rwswitch.c
 *
 * driver file for usb port switch
 *
 * Copyright (c) 2019-2021 Huawei Technologies Co., Ltd.
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

#include <chipset_common/hwusb/hw_usb_rwswitch.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/workqueue.h>

#define EVENT_BUF_LEN 32

static int switch_request;
static int port_mode;
static struct device *hw_usb_device;

void hw_usb_get_device(struct device *dev)
{
	hw_usb_device = dev;
}
EXPORT_SYMBOL(hw_usb_get_device);

static int usb_port_switch(int usb_switch)
{
	int ret = 0;
	char event_buf[EVENT_BUF_LEN];
	char *envp[2] = { event_buf, NULL };

	snprintf(event_buf, sizeof(event_buf),
		"HW_USB_PORT_SWITCH=%d", usb_switch);
	if (!hw_usb_device) {
		pr_info("%s: hw_usb_device is NULL\n", __func__);
		return ret;
	}

	ret = kobject_uevent_env(&hw_usb_device->kobj, KOBJ_CHANGE, envp);
	if (ret == 0)
		pr_info("uevent HW_USB_PORT_SWITCH=%d sending ok\n",
			usb_switch);
	else
		pr_err("%s: uevent HW_USB_PORT_SWITCH=%d sending failed %d\n",
			__func__, usb_switch, ret);

	return ret;
}

static void usb_port_switch_wq(struct work_struct *data)
{
	usb_port_switch(switch_request);
}
DECLARE_WORK(usb_port_switch_work, usb_port_switch_wq);

int hw_usb_port_switch_request(int usb_switch_index)
{
	if (usb_switch_index == 0) {
		pr_info("%s: uevent HW_USB_PORT_SWITCH=%d not send\n",
			__func__, usb_switch_index);
		return -1;
	}

	switch_request = usb_switch_index;

	if (in_interrupt())
		schedule_work(&usb_port_switch_work);
	else
		return usb_port_switch(switch_request);

	return 0;
}
EXPORT_SYMBOL(hw_usb_port_switch_request);

static ssize_t port_mode_show(struct device *pdev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", port_mode);
}

static ssize_t port_mode_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t size)
{
	pr_info("%s: port_mode buf=%s, size=%d\n",
		__func__, buf, (int)size);

	if (sscanf(buf, "%2d", &port_mode) == 1)
		return strlen(buf);

	return -1;
}

int hw_usb_port_mode_get(void)
{
	pr_info("current port_mode is %d\n", port_mode);
	return port_mode;
}
EXPORT_SYMBOL(hw_usb_port_mode_get);

static ssize_t switch_index_show(struct device *pdev,
	struct device_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%d\n", switch_request);
}

DEVICE_ATTR(switch_index, 0444, switch_index_show, NULL);
DEVICE_ATTR(port_mode, 0660, port_mode_show, port_mode_store);

struct device_attribute *huawei_usb_attributes[] = {
	&dev_attr_port_mode,
	&dev_attr_switch_index,
	NULL
};

int hw_rwswitch_create_device(struct device *dev,
	struct class *usb_class)
{
	struct device_attribute **attrs = huawei_usb_attributes;
	struct device_attribute *attr = NULL;
	int err;

	if (!attrs)
		return 0;

	while ((attr = *attrs++)) {
		err = device_create_file(dev, attr);
		if (err) {
			device_destroy(usb_class, dev->devt);
			return err;
		}
	}

	return 0;
}
EXPORT_SYMBOL(hw_rwswitch_create_device);
