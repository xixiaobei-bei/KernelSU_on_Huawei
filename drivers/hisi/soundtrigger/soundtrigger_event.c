/*
 * soundtrigger_event.c
 *
 * soundtrigger event to userspace implement
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/miscdevice.h>
#include <linux/input.h>
#include <linux/hisi/audio_log.h>

/*lint -e750*/
#define LOG_TAG "soundtrigger"

#define SOUNDTRIGGER_EVENT_BASE KEY_F14
#define SOUNDTRIGGER_EVENT_NUM 5
#define ENVP_LENTH 16

static struct input_dev *g_soundtrigger_input_dev;

static struct miscdevice hw_soundtrigger_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "hw_soundtrigger_uevent",
};

void hw_soundtrigger_event_input(uint32_t soundtrigger_event)
{
	if (soundtrigger_event >= SOUNDTRIGGER_EVENT_NUM)
		return;

	input_report_key(g_soundtrigger_input_dev, SOUNDTRIGGER_EVENT_BASE + soundtrigger_event, 1);
	input_sync(g_soundtrigger_input_dev);

	input_report_key(g_soundtrigger_input_dev, SOUNDTRIGGER_EVENT_BASE + soundtrigger_event, 0);
	input_sync(g_soundtrigger_input_dev);
}
EXPORT_SYMBOL(hw_soundtrigger_event_input);

void hw_soundtrigger_event_uevent(uint32_t soundtrigger_event)
{
	char envp_ext0[ENVP_LENTH] = { 0 };
	char *envp_ext[2] = { envp_ext0, NULL };

	snprintf(envp_ext0, ENVP_LENTH, "soundtrigger=%d", soundtrigger_event);
	kobject_uevent_env(&hw_soundtrigger_miscdev.this_device->kobj, KOBJ_CHANGE, envp_ext);
}
EXPORT_SYMBOL(hw_soundtrigger_event_uevent);

static const struct of_device_id hw_soundtrigger_event_of_match[] = {
	{
		.compatible = "hisilicon,hw-soundtrigger-event",
	},
	{ },
};
MODULE_DEVICE_TABLE(of, hi6402_vad_of_match);

static int soundtrigger_input_init(void)
{
	int error;

	g_soundtrigger_input_dev = input_allocate_device();
	if (!g_soundtrigger_input_dev) {
		AUDIO_LOGE("failed to allocate memory for input dev");
		return -ENOMEM;
	}

	g_soundtrigger_input_dev->name = "soundtrigger_input_dev";

	g_soundtrigger_input_dev->evbit[0] = BIT_MASK(EV_KEY);
	g_soundtrigger_input_dev->keybit[BIT_WORD(KEY_F14)] |= BIT_MASK(KEY_F14);
	g_soundtrigger_input_dev->keybit[BIT_WORD(KEY_F15)] |= BIT_MASK(KEY_F15);
	g_soundtrigger_input_dev->keybit[BIT_WORD(KEY_F16)] |= BIT_MASK(KEY_F16);
	g_soundtrigger_input_dev->keybit[BIT_WORD(KEY_F17)] |= BIT_MASK(KEY_F17);
	g_soundtrigger_input_dev->keybit[BIT_WORD(KEY_F18)] |= BIT_MASK(KEY_F18);

	error = input_register_device(g_soundtrigger_input_dev);
	if (error < 0) {
		AUDIO_LOGE("input register device failed, error_no is %d", error);
		/*
		 * This function should only be used if input_register_device was not called yet or if it failed.
		 * Once device was registered use input_unregister_device
		 * and memory will be freed once last reference to the device is dropped
		 */
		input_free_device(g_soundtrigger_input_dev);
		g_soundtrigger_input_dev = NULL;
		return error;
	}

	AUDIO_LOGI("input register device successful");
	return 0;
}

static void soundtrigger_input_deinit(void)
{
	/*
	 * Once device has been successfully registered it can be unregistered with input_unregister_device;
	 * input_free_device should not be called in this case.
	 */
	input_unregister_device(g_soundtrigger_input_dev);
	g_soundtrigger_input_dev = NULL;
}

static int hw_soundtrigger_event_probe(struct platform_device *pdev)
{
	int ret;

	AUDIO_LOGI("in");

	ret = soundtrigger_input_init();
	if (ret) {
		AUDIO_LOGE("input registor failed: %d", ret);
		return ret;
	}

	ret = misc_register(&hw_soundtrigger_miscdev);
	if (ret) {
		AUDIO_LOGE("misc_register failed");
		soundtrigger_input_deinit();
		return ret;
	}

	AUDIO_LOGI("successfully");

	return 0;
}

static int hw_soundtrigger_event_remove(struct platform_device *pdev)
{
	misc_deregister(&hw_soundtrigger_miscdev);
	soundtrigger_input_deinit();

	return 0;
}

static struct platform_driver hw_soundtrigger_event_driver = {
	.driver = {
		.name = "hw_soundtrigger_event",
		.owner = THIS_MODULE,
		.of_match_table = hw_soundtrigger_event_of_match,
	},
	.probe = hw_soundtrigger_event_probe,
	.remove = hw_soundtrigger_event_remove,
};

static int __init hw_soundtrigger_event_init(void)
{
	return platform_driver_register(&hw_soundtrigger_event_driver);
}

static void __exit hw_soundtrigger_event_exit(void)
{
	platform_driver_unregister(&hw_soundtrigger_event_driver);
}

fs_initcall(hw_soundtrigger_event_init);
module_exit(hw_soundtrigger_event_exit);

MODULE_DESCRIPTION("hw_soundtrigger_event driver");
MODULE_LICENSE("GPL");

