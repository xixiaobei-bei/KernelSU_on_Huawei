/*
 * ext_vibrator.c
 * ext vibrator driver
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
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

#include "ext_vibrator.h"
#include <securec.h>
#include <linux/cdev.h>
#include <linux/leds.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/string.h>
#include <linux/suspend.h>
#include <linux/switch.h>
#include <linux/syscalls.h>
#include <linux/workqueue.h>
#include <huawei_platform/log/hw_log.h>
#include "ext_sensorhub_api.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define VIBRATOR_PRIORITY (INT_MAX - 1)
#define VIBRATOR_SUSPEND_RESUME      0
#define HWLOG_TAG ext_vibrator
HWLOG_REGIST();

#if defined(CONFIG_HISI_VIBRATOR)
extern volatile int vibrator_shake;
#else
volatile int vibrator_shake;
#endif

struct hisi_pmic_vibrator_dev {
	struct device *dev;
	struct class *class;
	struct led_classdev led_dev;
	struct cdev cdev;
	struct switch_dev sw_dev;
	struct mutex lock;
	struct wakeup_source wakelock;
	dev_t version;
	struct work_struct vibrator_off_work;
	struct work_struct vibrator_enable_work;
	u32 haptics_counts;
	u32 state;
};

static struct hisi_pmic_vibrator_dev *g_vdev;

static int ext_vibrator_on_off(unsigned char mode)
{
	int ret;

	struct command cmd = {
		.service_id = EXT_VIBRATOR_SID,
		.command_id = EXT_VIBRATOR_WORK_CID,
		.send_buffer = &mode,
		.send_buffer_len = sizeof(mode),
	};

	ret = send_command(VIBRATOR_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		hwlog_err("send vibrator cmd failed ret %d\n", ret);

	return ret;
}

static int ext_vibra_set_haptics(unsigned char haptic)
{
	int ret;

	struct command cmd = {
		.service_id = EXT_VIBRATOR_SID,
		.command_id = EXT_VIBR_HAP_CID,
		.send_buffer = &haptic,
		.send_buffer_len = sizeof(haptic),
	};

	ret = send_command(VIBRATOR_CHANNEL, &cmd, false, NULL);
	if (ret < 0)
		hwlog_err("send vibrator haptics cmd failed ret %d\n", ret);

	return ret;
}

static void haptics_num_set(u32 type)
{
	for (int i = 0; i < array_size(g_haptics_table); i++) {
		if (type == g_haptics_table[i].haptics_num) {
			hwlog_info("HISI_PMIC_VIBRATOR_HAPTIC typ %d\n", type);
			ext_vibra_set_haptics(g_haptics_table[i].command);
			break;
		}
	}
}

static s32 ext_vibrator_set_type(struct hisi_pmic_vibrator_dev *vdev, u32 type)
{
	if (vdev == NULL)
		return -EINVAL;

	if (type > vdev->haptics_counts || !type) {
		hwlog_err("type:%d is invaild\n", type);
		return -EINVAL;
	}

	haptics_num_set(type);
	hwlog_info("%s complete\n",  __func__);

	return 0;
}

static int hisi_pmic_vibrator_power_on_off(int pmic_vibrator_power_onoff)
{
	int ret;
	if (pmic_vibrator_power_onoff) {
		ret = ext_vibrator_on_off(EXT_VIBRATOR_ON);
		if (ret < 0)
			hwlog_err("ext_vibrator on failed\n");
	} else {
		ret = ext_vibrator_on_off(EXT_VIBRATOR_OFF);
		if (ret < 0)
			hwlog_err("ext_vibrator off failed\n");
	}
	return ret;
}

static void hisi_pmic_vibrator_off(struct hisi_pmic_vibrator_dev *vdev)
{
	hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_OFF);
}

static void hisi_pmic_vibrator_off_work(struct work_struct *work)
{
	hisi_pmic_vibrator_off(g_vdev);
}

static void hisi_pmic_vibrator_enable(struct hisi_pmic_vibrator_dev *vdev)
{
	if (vdev == NULL)
		return;

	mutex_lock(&vdev->lock);

	if (vdev->state) {
		hisi_pmic_vibrator_power_on_off(PMIC_VIBRATOR_POWER_ON);
		vibrator_shake = 1;
		hwlog_info("hisi_pmic_vibrator_RTP is running\n");
	} else {
		hisi_pmic_vibrator_off(vdev);
		vibrator_shake = 0;
	}

	mutex_unlock(&vdev->lock);
}

static void hisi_pmic_vibrator_enable_work(struct work_struct *work)
{
	hisi_pmic_vibrator_enable(g_vdev);
}

/* echo 0 or 1 to /sys/class/leds/vibrator/enable */
static void hisi_pmic_vibrator_enable_ctrl(
	struct led_classdev *led_dev, enum led_brightness state)
{
	struct hisi_pmic_vibrator_dev *vdev =
		container_of(led_dev, struct hisi_pmic_vibrator_dev, led_dev);

	vdev->state = state;
	schedule_work(&vdev->vibrator_enable_work);
}

/* write type to /dev/haptics */
static ssize_t hisi_pmic_vibrator_haptics_write(
	struct file *filp, const char *buff, size_t len, loff_t *off)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;
	uint64_t type;
	s32 ret;
	char write_buf[PMIC_VIBRATOR_HAP_BUF_LENGTH] = {0};

	if (filp == NULL)
		return 0;

	vdev = (struct hisi_pmic_vibrator_dev *)filp->private_data;

	mutex_lock(&vdev->lock);
	vibrator_shake = 1;

	if (len > PMIC_VIBRATOR_HAP_BUF_LENGTH || buff == NULL) {
		hwlog_err("hisi pmic vibrator haptic buf is bad\n");
		goto out;
	}

	if (copy_from_user(write_buf, buff, len)) {
		hwlog_err("hisi pmic vibrator haptics copy from user failed\n");
		goto out;
	}

	if (kstrtoull(write_buf, 10, &type)) {
		hwlog_err("hisi pmic vibrator haptics read value error\n");
		goto out;
	}
	ret = ext_vibrator_set_type(vdev, (u32)type);
	if (ret < 0) {
		hwlog_err("hisi pmic vibrator haptics cfg failed\n");
		goto out;
	}

out:
	vibrator_shake = 0;
	mutex_unlock(&vdev->lock);

	return len;
}

void hisi_pmic_vibrator_haptics_set_type(int type)
{
	s32 ret;

	mutex_lock(&g_vdev->lock);
	ret = ext_vibrator_set_type(g_vdev, (u32)type);
	if (ret) {
		hwlog_err("%s hisi_haptic_test error\n", __func__);
		mutex_unlock(&g_vdev->lock);
		return;
	}
	hwlog_info("%s is running\n", __func__);
	mutex_unlock(&g_vdev->lock);
}
EXPORT_SYMBOL(hisi_pmic_vibrator_haptics_set_type);

static s32 hisi_pmic_vibrator_haptics_open(
	struct inode *i_node, struct file *filp)
{
	if (filp == NULL)
		return -1;
	filp->private_data = g_vdev;
	hwlog_err("%s:haptics open\n", __func__);
	return 0;
}

static const struct file_operations hisi_pmic_vibrator_fops = {
	.open = hisi_pmic_vibrator_haptics_open,
	.write = hisi_pmic_vibrator_haptics_write,
};

static s32 hisi_pmic_vibrator_parse_dt(struct hisi_pmic_vibrator_dev *vdev)
{
	struct device *dev = vdev->dev;
	int ret;
	int temp = 0;

	ret = of_property_read_u32(dev->of_node, "haptics-mun", &temp);
	if (ret < 0) {
		hwlog_err("get haptics-mun fail!\n");
	} else {
		vdev->haptics_counts = temp;
		hwlog_info("haptics_counts is %d\n", vdev->haptics_counts);
	}

	return 0;
}

static s32 hisi_pmic_vibrator_haptics_probe(struct hisi_pmic_vibrator_dev *vdev)
{
	s32 ret;

	vdev->version = MKDEV(0, 0);
	ret = alloc_chrdev_region(&vdev->version, 0, 1, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (ret < 0) {
		hwlog_err("[%s]:failed to alloc chrdev region, ret[%d]\n", __func__, ret);
		return ret;
	}

	/* /dev/haptics */
	vdev->class = class_create(THIS_MODULE, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (!vdev->class) {
		hwlog_err("[%s]:failed to create class\n", __func__);
		ret = ENOMEM;
		goto unregister_cdev_region;
	}

	vdev->dev = device_create(vdev->class, NULL, vdev->version, NULL, HISI_PMIC_VIBRATOR_CDEVIE_NAME);
	if (vdev->dev == NULL) {
		ret = ENOMEM;
		hwlog_err("[%s]:failed to create device\n", __func__);
		goto destory_class;
	}

	cdev_init(&vdev->cdev, &hisi_pmic_vibrator_fops);
	vdev->cdev.owner = THIS_MODULE;
	vdev->cdev.ops = &hisi_pmic_vibrator_fops;
	ret = cdev_add(&vdev->cdev, vdev->version, 1);
	if (ret) {
		hwlog_err("[%s]:failed to add cdev\n", __func__);
		goto destory_device;
	}

	vdev->sw_dev.name = "haptics";
	ret = switch_dev_register(&vdev->sw_dev);
	if (ret < 0) {
		hwlog_err("[%s]:failed to register sw_dev\n", __func__);
		goto unregister_cdev;
	}

	dev_info(vdev->dev, "haptics setup ok\n");

	return 0;

unregister_cdev:
	cdev_del(&vdev->cdev);
destory_device:
	device_destroy(vdev->class, vdev->version);
destory_class:
	class_destroy(vdev->class);
unregister_cdev_region:
	unregister_chrdev_region(vdev->version, 1);
	return ret;
}

static void hisi_pmic_vibrator_haptics_remove(struct hisi_pmic_vibrator_dev *vdev)
{
	cdev_del(&vdev->cdev);
	device_destroy(vdev->class, vdev->version);
	class_destroy(vdev->class);
	unregister_chrdev_region(vdev->version, 1);
	switch_dev_unregister(&vdev->sw_dev);
}

/* /sys/class/leds/vibrator/ */
static int hisi_pmic_vibrator_register_led_classdev(struct hisi_pmic_vibrator_dev *vdev)
{
	struct led_classdev *led_cdev = &vdev->led_dev;

	led_cdev->name = "vibrator";
	led_cdev->flags = VIBRATOR_SUSPEND_RESUME;
	led_cdev->brightness_set = hisi_pmic_vibrator_enable_ctrl;
	led_cdev->default_trigger = "transient";

	return devm_led_classdev_register(vdev->dev, led_cdev);
}

static int vibrator_notify(struct notifier_block *notify_block,
			unsigned long mode, void *unused)
{
	int ret;
	if (mode == PM_SUSPEND_PREPARE) {
		ret = ext_vibrator_on_off(EXT_VIBRATOR_OFF);
		if (ret < 0) {
			hwlog_err("[%s]: vibrator off fail\n", __func__);
			return -EINVAL;
		}
		vibrator_shake = 0;
		hwlog_info("vibrator has closed\n");
	}
	return 0;
}

static struct notifier_block g_vibrator_notify = {
	.notifier_call = vibrator_notify,
	.priority = VIBRATOR_PRIORITY,
};

static int hisi_pmic_vibrator_probe(struct platform_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;
	s32 ret;

	vdev = devm_kzalloc(
		&pdev->dev, sizeof(struct hisi_pmic_vibrator_dev), GFP_KERNEL);
	if (vdev == NULL)
		return -ENOMEM;

	vdev->dev = &pdev->dev;
	g_vdev = vdev;
	dev_set_drvdata(&pdev->dev, vdev);

	/* parse DT */
	ret = hisi_pmic_vibrator_parse_dt(vdev);
	if (ret) {
		hwlog_err("[%s]:DT parsing failed\n", __func__);
		return ret;
	}

	mutex_init(&vdev->lock);

	INIT_WORK(&vdev->vibrator_enable_work, hisi_pmic_vibrator_enable_work);
	INIT_WORK(&vdev->vibrator_off_work, hisi_pmic_vibrator_off_work);
	ret = hisi_pmic_vibrator_register_led_classdev(vdev);
	if (ret) {
		hwlog_err("[%s]:unable to register with timed_output\n", __func__);
		goto fail_register_led_classdev;
	}
	ret = hisi_pmic_vibrator_haptics_probe(vdev);
	if (ret) {
		hwlog_err("[%s]:failed to register haptics dev\n", __func__);
		goto fail_register_led_classdev;
	}
	hwlog_info("ext_vibrator probe succeed\n");

	return 0;

fail_register_led_classdev:
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->vibrator_enable_work);
	mutex_destroy(&vdev->lock);

	return ret;
}

static s32 hisi_pmic_vibrator_remove(struct platform_device *pdev)
{
	struct hisi_pmic_vibrator_dev *vdev = NULL;

	vdev = dev_get_drvdata(&pdev->dev);
	if (vdev == NULL) {
		pr_err("%s:failed to get drvdata\n", __func__);
		return -ENODEV;
	}
	cancel_work_sync(&vdev->vibrator_off_work);
	cancel_work_sync(&vdev->vibrator_enable_work);

	hisi_pmic_vibrator_haptics_remove(vdev);
	mutex_destroy(&vdev->lock);
	wakeup_source_trash(&vdev->wakelock);
	dev_set_drvdata(&pdev->dev, NULL);

	return 0;
}

static const struct of_device_id hisi_pmic_vibrator_match[] = {
	{
		.compatible = "hisilicon,ext-vibrator",
		.data = NULL,
	},
	{},
};
MODULE_DEVICE_TABLE(of, hisi_pmic_vibrator_match);

static struct platform_driver hisi_pmic_vibrator_driver = {
	.probe = hisi_pmic_vibrator_probe,
	.remove = hisi_pmic_vibrator_remove,
	.driver = {
			.name = "ext-vibrator",
			.owner = THIS_MODULE,
			.of_match_table = of_match_ptr(hisi_pmic_vibrator_match),
		},
};

static int __init hisi_pmic_vibrator_init(void)
{
	int ret;
	ret = register_pm_notifier(&g_vibrator_notify);
	if (ret < 0) {
		hwlog_err("[%s]:register vibrator_notifier failed, ret = %d!",
			__func__, ret);
	}
	return platform_driver_register(&hisi_pmic_vibrator_driver);
}

static void __exit hisi_pmic_vibrator_exit(void)
{
	unregister_pm_notifier(&g_vibrator_notify);
	platform_driver_unregister(&hisi_pmic_vibrator_driver);
}

module_init(hisi_pmic_vibrator_init);
module_exit(hisi_pmic_vibrator_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("HISI ext_vibrator driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
