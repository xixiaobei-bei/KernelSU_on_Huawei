/*
 * usb_audio_common.c
 *
 * usb audio common driver
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#include "huawei_platform/audio/usb_audio_common.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/wakelock.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <huawei_platform/log/hw_log.h>
#include <huawei_platform/usb/hw_pd_dev.h>

#define HWLOG_TAG usb_audio_common
HWLOG_REGIST();

struct usb_audio_common_data {
	unsigned int usb_typec_plugin;
	struct notifier_block usb_device_notifier;
	struct mutex notifier_lock;
};

static struct usb_audio_common_data *usb_audio_com_pdata;

static const struct of_device_id usb_audio_common_of_match[] = {
	{ .compatible = "huawei,usb_audio_common", },
	{},
};
MODULE_DEVICE_TABLE(of, usb_audio_common_of_match);

static long usb_audio_common_ioctl(struct file *file, unsigned int cmd,
				unsigned long arg)
{
	int ret;
	unsigned int __user *p_user = (unsigned int __user *) arg;

	if (!p_user || !usb_audio_com_pdata)
		return -EBUSY;

	switch (cmd) {
	case IOCTL_USB_AUDIO_COMMON_GET_TYPEC_STATE:
		hwlog_info("usb typec plugin:%d\n",
			usb_audio_com_pdata->usb_typec_plugin);
		ret = put_user((__u32)(usb_audio_com_pdata->usb_typec_plugin),
			p_user);
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations usb_audio_common_fops = {
	.owner           = THIS_MODULE,
	.open            = simple_open,
	.unlocked_ioctl  = usb_audio_common_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl    = usb_audio_common_ioctl,
#endif
};

static struct miscdevice usb_audio_common_miscdev = {
	.minor =    MISC_DYNAMIC_MINOR,
	.name =     "usb_audio_common",
	.fops =     &usb_audio_common_fops,
};

static int usb_audio_common_notifier_call(struct notifier_block *typec_nb,
					unsigned long event, void *data)
{
	char envp_ext0[ENVP_LENGTH];
	char *envp_ext[2] = { envp_ext0, NULL };

	hwlog_info("%s %lu\n", __func__, event);
	mutex_lock(&usb_audio_com_pdata->notifier_lock);
	if (usb_audio_com_pdata->usb_typec_plugin &&
		(event == PD_DPM_USB_TYPEC_DETACHED ||
		event == PD_DPM_USB_TYPEC_NONE)) {
		usb_audio_com_pdata->usb_typec_plugin = false;
		snprintf(envp_ext0, ENVP_LENGTH, "plugout");
	} else if (!usb_audio_com_pdata->usb_typec_plugin &&
		(event != PD_DPM_USB_TYPEC_DETACHED &&
		 event != PD_DPM_USB_TYPEC_NONE)) {
		usb_audio_com_pdata->usb_typec_plugin = true;
		snprintf(envp_ext0, ENVP_LENGTH, "plugin");
	} else {
		mutex_unlock(&usb_audio_com_pdata->notifier_lock);
		return 0;
	}

	kobject_uevent_env(&usb_audio_common_miscdev.this_device->kobj,
		KOBJ_CHANGE, envp_ext);
	mutex_unlock(&usb_audio_com_pdata->notifier_lock);

	return 0;
}

static int usb_audio_common_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;
	int event = PD_DPM_USB_TYPEC_NONE;

	usb_audio_com_pdata = devm_kzalloc(dev, sizeof(*usb_audio_com_pdata),
		GFP_KERNEL);
	if (!usb_audio_com_pdata)
		return -ENOMEM;

	mutex_init(&usb_audio_com_pdata->notifier_lock);
	usb_audio_com_pdata->usb_typec_plugin = false;
	usb_audio_com_pdata->usb_device_notifier.notifier_call =
		usb_audio_common_notifier_call;
	ret = misc_register(&usb_audio_common_miscdev);
	if (ret != 0) {
		hwlog_err("%s:misc_register failed, ret:%d\n", __func__, ret);
		goto err_out;
	}
	register_pd_dpm_portstatus_notifier(
		&usb_audio_com_pdata->usb_device_notifier);
	pd_dpm_get_typec_state(&event);
	usb_audio_common_notifier_call(NULL, (unsigned long)event, NULL);
	hwlog_info("usb_audio_common probe success\n");
	return 0;

err_out:
	devm_kfree(dev, usb_audio_com_pdata);
	usb_audio_com_pdata = NULL;
	return ret;
}

static int usb_audio_common_remove(struct platform_device *pdev)
{
	if (usb_audio_com_pdata) {
		unregister_pd_dpm_portstatus_notifier(
			&usb_audio_com_pdata->usb_device_notifier);
		hwlog_info("%s: unregister_pd_dpm_portstatus_notifier\n",
			__func__);
		devm_kfree(&pdev->dev, usb_audio_com_pdata);
		usb_audio_com_pdata = NULL;
	}
	misc_deregister(&usb_audio_common_miscdev);
	hwlog_info("%s: exit\n", __func__);
	return 0;
}

static struct platform_driver usb_audio_common_driver = {
	.driver = {
		.name   = "usb_audio_common",
		.owner  = THIS_MODULE,
		.of_match_table = usb_audio_common_of_match,
	},
	.probe  = usb_audio_common_probe,
	.remove = usb_audio_common_remove,
};

static int __init usb_audio_common_init(void)
{
	return platform_driver_register(&usb_audio_common_driver);
}

static void __exit usb_audio_common_exit(void)
{
	platform_driver_unregister(&usb_audio_common_driver);
}

device_initcall_sync(usb_audio_common_init);
module_exit(usb_audio_common_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("usb audio common control driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
