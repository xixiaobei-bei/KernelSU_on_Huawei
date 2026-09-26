/*
 * blpwm.c
 *
 * blpwm driver
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
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

#include "blpwm.h"
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/suspend.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <huawei_platform/log/hw_log.h>
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG blpwm
HWLOG_REGIST();

static struct blpwm_dev *g_blpwm_dev;

int blpwm_register_ops(struct blpwm_ops *p_ops)
{
	if (!g_blpwm_dev || !p_ops) {
		hwlog_info("%s: g_blpwm_dev or ops null\n", __func__);
		return -1;
	}

	g_blpwm_dev->p_ops = p_ops;
	schedule_delayed_work(&g_blpwm_dev->blpwm_boot_work,
		msecs_to_jiffies(BLPWM_BOOT_DELAY));

	return 0;
}

void blpwm_notify_by_sns(pkt_blpwm_sensor_req_t *blpwm_event)
{
	if (!g_blpwm_dev || !g_blpwm_dev->p_ops || !blpwm_event) {
		hwlog_info("no g_blpwm_dev or P_ops or blpwm_event");
		return;
	}

	g_blpwm_dev->p_ops->blpwm_notiby_by_sns(blpwm_event);
}

static int blpwm_cover_detect(void)
{
	return g_blpwm_dev->p_ops->blpwm_cover_detect();
}

static void blpwm_set_duty(int duty)
{
	g_blpwm_dev->p_ops->blpwm_set_duty(duty);
}

static void blpwm_set_usage(int usage)
{
	g_blpwm_dev->p_ops->blpwm_set_usage(usage);
}

static void blpwm_set_usage_stop(int usage_stop)
{
	g_blpwm_dev->p_ops->blpwm_set_usage_stop(usage_stop);
}

int blpwm_get_color(void)
{
	if (!g_blpwm_dev)
		return -ENODEV;

	return g_blpwm_dev->color;
}

int blpwm_get_support(void)
{
	if (!g_blpwm_dev)
		return -ENODEV;

	return g_blpwm_dev->support;
}

int blpwm_get_cover_status(void)
{
	if (!g_blpwm_dev)
		return -ENODEV;

	return g_blpwm_dev->cover_status;
}

#ifdef FACTORY_BLPWM
static void blpwm_enable_sensorhub_by_at(int sub_cmd)
{
	int ret;
	struct write_info pkg_ap = {0};
	pkt_subcmd_req_t cpkt = { {0} };
	struct pkt_header *hd = (struct pkt_header *)&cpkt;

	pkg_ap.tag = TAG_BLPWM;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(int);

	if (sub_cmd == AT_PLAN1_CMD) {
		cpkt.subcmd = SUB_CMD_BLPWM_FACTORY_AT1;
	} else if (sub_cmd == AT_PLAN2_CMD) {
		cpkt.subcmd = SUB_CMD_BLPWM_FACTORY_AT2;
	} else if (sub_cmd == AT_PLAN3_CMD) {
		cpkt.subcmd = SUB_CMD_BLPWM_FACTORY_AT3;
	} else if (sub_cmd == AT_PLAN4_CMD) {
		cpkt.subcmd = SUB_CMD_BLPWM_FACTORY_AT4;
	} else if (sub_cmd == AT_RESET_CMD) {
		cpkt.subcmd = SUB_CMD_BLPWM_FACTORY_RESET;
	} else {
		hwlog_info("invalid atCmd");
		return;
	}
	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret)
		hwlog_err("%s: send config req fail, ret = %d", __func__, ret);
	hwlog_info("%s", __func__);
}

void blpwm_set_dbc_plan(int plan)
{
	blpwm_enable_sensorhub_by_at(plan);
}
#endif /* FACTORY_BLPWM */

void blpwm_open_sensorhub(void)
{
	int ret;
	struct write_info pkg_ap = {0};

	pkg_ap.tag = TAG_BLPWM;
	pkg_ap.cmd = CMD_CMN_OPEN_REQ;

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret)
		hwlog_err(" %s: send open_req fail, ret = %d", __func__, ret);
}

void blpwm_config_sensorhub(enum obj_sub_cmd sub_cmd)
{
	int ret = 0;

	if (!g_blpwm_dev)
		return;

	struct write_info pkg_ap = {0};
	pkt_blpwm_req_t pkt_blpwm = { {0} };
	struct pkt_header *hd = (struct pkt_header *)&pkt_blpwm;

	pkg_ap.tag = TAG_BLPWM;
	pkg_ap.cmd = CMD_CMN_CONFIG_REQ;
	pkg_ap.wr_buf = &hd[1];
	pkg_ap.wr_len = sizeof(pkt_blpwm) - sizeof(*hd);
	pkt_blpwm.sub_cmd = sub_cmd;
	pkt_blpwm.pwm_cycle = g_blpwm_dev->duty_cycle;
	pkt_blpwm.usage = g_blpwm_dev->usage;
	pkt_blpwm.usage_stop = g_blpwm_dev->usage_stop;
	pkt_blpwm.color = g_blpwm_dev->color;

	hwlog_info("%s: sub_cmd is %d, duty is %d, usage is %d, usage_stop is %d, color is %d\n",
		__func__, pkt_blpwm.sub_cmd, pkt_blpwm.pwm_cycle,
		pkt_blpwm.usage, pkt_blpwm.usage_stop, pkt_blpwm.color);

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret)
		hwlog_err("%s: set pwm_cycle fail, ret = %d", __func__, ret);
}

void blpwm_close_sensorhub(void)
{
	int ret;
	struct write_info pkg_ap = {0};

	pkg_ap.tag = TAG_BLPWM;
	pkg_ap.cmd = CMD_CMN_CLOSE_REQ;

	ret = write_customize_cmd(&pkg_ap, NULL, true);
	if (ret)
		hwlog_err("%s: send close_req fail, ret = %d", __func__, ret);
}

static int blpwm_nv_read_color(int *pcolor)
{
	int ret;
	struct hisi_nve_info_user user_info = {0};

	user_info.nv_operation = NV_READ;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = NV_VALID_SIZE;
	strncpy(user_info.nv_name, "BLPWM", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	memset(user_info.nv_data, 0, sizeof(user_info.nv_data));
	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access read error %d\n", ret);
		return -1;
	}

	memcpy(pcolor, user_info.nv_data, sizeof(int));

	hwlog_info("%s: color read from nv is: %d", __func__, *pcolor);
	return 0;
}

static int blpwm_nv_write_color(int color)
{
	int ret;
	struct hisi_nve_info_user user_info = {0};

	user_info.nv_operation = NV_WRITE;
	user_info.nv_number = NV_NUMBER;
	user_info.valid_size = NV_VALID_SIZE;

	strncpy(user_info.nv_name, "BLPWM", sizeof(user_info.nv_name) - 1);
	user_info.nv_name[sizeof(user_info.nv_name) - 1] = '\0';
	memcpy(user_info.nv_data, &color, sizeof(color));

	ret = hisi_nve_direct_access(&user_info);
	if (ret) {
		hwlog_err("hisi_nve_direct_access write error %d\n", ret);
		return -1;
	}

	hwlog_info("%s: color %d write to nv", __func__, color);
	return 0;
}

bool blpwm_is_valid_color(int color)
{
	return (color > COLOR_ID_MIN && color <= COLOR_ID_MAX);
}

bool blpwm_is_support(void)
{
	if (!g_blpwm_dev || !g_blpwm_dev->p_ops)
		return false;

	if (g_blpwm_dev->cover_status == COVER_INFO_NOT_READ) {
		blpwm_nv_read_color(&g_blpwm_dev->color);
		g_blpwm_dev->support = g_blpwm_dev->p_ops->blpwm_cover_detect();
		g_blpwm_dev->cover_status = COVER_INFO_ALREADY_READ;
	}

	return g_blpwm_dev->support == SUPPORT_WITH_COLOR;
}

static ssize_t blpwm_hal_get_nv(struct file *file, char __user *buf, size_t count, loff_t *pos)
{
	char ret_buf[DEBUG_BUFFER_SIZE] = {0};

	if (!buf) {
		hwlog_err("buf is null\n");
		return -1;
	}

	blpwm_is_support();

	ret_buf[0] = g_blpwm_dev->color;
	ret_buf[1] = g_blpwm_dev->support;
	hwlog_info("%s: color is %d, support is %d",
		__func__, g_blpwm_dev->color, g_blpwm_dev->support);

	if (copy_to_user(buf, ret_buf, sizeof(ret_buf))) {
		hwlog_err("%s: copy_to_user failed\n", __func__);
		return -1;
	}

	hwlog_info("%s: get nv info from hal\n", __func__);
	return 0;
}

void blpwm_init_work(struct work_struct *work)
{
	if (!blpwm_is_support()) {
		blpwm_close_sensorhub();
		hwlog_err("%s: blpwm unsupport", __func__);
		return;
	}

	blpwm_open_sensorhub();
	hwlog_info("%s: adc boot detect done\n", __func__);
}

static int blpwm_sensorhub_recovery_notifier(struct notifier_block *nb,
	unsigned long foo, void *bar)
{
	hwlog_info("%s %lu\n", __func__, foo);
	switch (foo) {
	case IOM3_RECOVERY_IDLE:
	case IOM3_RECOVERY_START:
	case IOM3_RECOVERY_MINISYS:
	case IOM3_RECOVERY_3RD_DOING:
	case IOM3_RECOVERY_FAILED:
		break;
	case IOM3_RECOVERY_DOING:
		if (blpwm_is_support())
			blpwm_open_sensorhub();
		break;
	default:
		hwlog_err("%s -unknow state %lu\n", __func__, foo);
		break;
	}
	hwlog_info("%s end\n", __func__);
	return 0;
}

static struct notifier_block sensorhub_recovery_notify = {
	.notifier_call = blpwm_sensorhub_recovery_notifier,
	.priority = -2, /* SENSORHUB_RECOVERY_PRIORITY */
};

static ssize_t blpwm_support_store(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int ret;
	int value = -1;

	if (!buf)
		return -EINVAL;

	if (!g_blpwm_dev || !g_blpwm_dev->p_ops)
		return -ENODEV;

	ret = kstrtoint(buf, DEBUG_BUFFER_SIZE, &value);
	if (ret) {
		hwlog_err("%s: convert to int type failed\n", __func__);
		return ret;
	}
	if ((value < SUPPORT_WITH_NOCOLOR) || (value > SUPPORT_WITH_COLOR)) {
		hwlog_err("invalid value\n");
		return -EINVAL;
	} else {
		g_blpwm_dev->support = value;
		g_blpwm_dev->cover_status = COVER_STATUS_DEBUG;
		hwlog_info("value set to %d\n", value);
	}

	return count;
}

static DEVICE_ATTR(blpwm_support, 0220, NULL, blpwm_support_store);

static struct attribute *blpwm_attributes[] = {
	&dev_attr_blpwm_support.attr,
	NULL
};

static const struct attribute_group blpwm_attr_group = {
	.attrs = blpwm_attributes,
};

static long blpwm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	void __user *data = (void __user *)(uintptr_t)arg;

	if (!data)
		hwlog_err("%s: input buff is null\n", __func__);

	if (!g_blpwm_dev || !g_blpwm_dev->p_ops)
		return (long)-ENODEV;

	switch (cmd) {
#ifdef FACTORY_BLPWM
	case IOCTL_BLPWM_PLAN1_CMD:
		blpwm_set_dbc_plan(AT_PLAN1_CMD);
		hwlog_info("blpwm_at_plan1 send\n");
		break;
	case IOCTL_BLPWM_PLAN2_CMD:
		blpwm_set_dbc_plan(AT_PLAN2_CMD);
		hwlog_info("blpwm_at_plan2 send\n");
		break;
	case IOCTL_BLPWM_PLAN3_CMD:
		blpwm_set_dbc_plan(AT_PLAN3_CMD);
		hwlog_info("blpwm_at_plan3 send\n");
		break;
	case IOCTL_BLPWM_PLAN4_CMD:
		blpwm_set_dbc_plan(AT_PLAN4_CMD);
		hwlog_info("blpwm_at_plan4 send\n");
		break;
	case IOCTL_BLPWM_RESET_CMD:
		blpwm_set_dbc_plan(AT_RESET_CMD);
		hwlog_info("blpwm_at_reset send\n");
		break;
#endif /* FACTORY_BLPWM */
	case IOCTL_BLPWM_DUTY:
		if (copy_from_user(&g_blpwm_dev->duty_cycle, (void __user *)arg, sizeof(int))) {
			hwlog_err("%s: copy from user fail\n", __func__);
			return -EINVAL;
		}
		g_blpwm_dev->duty_cycle = APP_PWM_CYCLE_MAX - g_blpwm_dev->duty_cycle;
		blpwm_set_duty(g_blpwm_dev->duty_cycle);
		break;
	case IOCTL_BLPWM_USEAGE:
		if (copy_from_user(&g_blpwm_dev->usage, (void __user *)arg, sizeof(int))) {
			hwlog_err("%s: copy from user fail\n", __func__);
			return -EINVAL;
		}
		blpwm_set_usage(g_blpwm_dev->usage);
		break;
	case IOCTL_BLPWM_USEAGE_STOP:
		if (copy_from_user(&g_blpwm_dev->usage_stop, (void __user *)arg, sizeof(int))) {
			hwlog_err("%s: copy from user fail\n", __func__);
			return -EINVAL;
		}
		blpwm_set_usage_stop(g_blpwm_dev->usage_stop);
		break;
	case IOCTL_COVER_COLOR_CMD:
		g_blpwm_dev->color = (uint32_t)(arg);
		hwlog_info("g_blpwm_dev->color is %d\n", g_blpwm_dev->color);
		ret = blpwm_nv_write_color(g_blpwm_dev->color);
		if (ret < 0)
			hwlog_err("write B2 color info to nv fail\n");
		hwlog_info("write B2 color to nv succ\n");
		break;
	case IOCTL_COVER_LOOKUP_CMD:
		g_blpwm_dev->support = blpwm_cover_detect();
		hwlog_info("look up cmd send, support is %d\n", g_blpwm_dev->support);
		if (copy_to_user((void __user *)arg, &g_blpwm_dev->support, sizeof(g_blpwm_dev->support)))
			hwlog_info("copy to user fail\n");
		break;
	default:
		hwlog_err("unsupport cmd\n");
		ret = -EINVAL;
		break;
	}

	return (long)ret;
}

static const struct file_operations blpwm_fops = {
	.owner = THIS_MODULE,
	.open = simple_open,
	.read = blpwm_hal_get_nv,
	.unlocked_ioctl = blpwm_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl = blpwm_ioctl,
#endif /* CONFIG_COMPAT */
};

static struct miscdevice blpwm_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "blpwm",
	.fops = &blpwm_fops,
};

static int blpwm_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	int ret;

	g_blpwm_dev = devm_kzalloc(dev, sizeof(*g_blpwm_dev), GFP_KERNEL);
	if (!g_blpwm_dev)
		return -ENOMEM;

	ret = misc_register(&blpwm_misc_dev);
	if (ret) {
		hwlog_err("%s: misc register failed %d", __func__, ret);
		goto misc_register_fail;
	}

	ret = sysfs_create_group(&dev->kobj, &blpwm_attr_group);
	if (ret < 0)
		hwlog_err("%s: failed to register sysfs blpwm\n", __func__);

	INIT_DELAYED_WORK(&g_blpwm_dev->blpwm_boot_work, blpwm_init_work);
	register_iom3_recovery_notifier(&sensorhub_recovery_notify);

	return ret;

misc_register_fail:
	devm_kfree(dev, g_blpwm_dev);
	g_blpwm_dev = NULL;
}

static int blpwm_remove(struct platform_device *pdev)
{
	if (!g_blpwm_dev)
		return -1;

	sysfs_remove_group(&pdev->dev.kobj, &blpwm_attr_group);
	g_blpwm_dev = NULL;

	return 0;
}

static const struct of_device_id blpwm_of_match[] = {
	{ .compatible = "huawei,blpwm", },
	{},
};
MODULE_DEVICE_TABLE(of, blpwm_of_match);

static struct platform_driver blpwm_driver = {
	.driver = {
		.name = "blpwm",
		.owner = THIS_MODULE,
		.of_match_table = blpwm_of_match,
	},
	.probe = blpwm_probe,
	.remove = blpwm_remove,
};

static int __init blpwm_init(void)
{
	return platform_driver_register(&blpwm_driver);
}

static void __exit blpwm_exit(void)
{
	platform_driver_unregister(&blpwm_driver);
}

subsys_initcall(blpwm_init);
module_exit(blpwm_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("blpwm driver");
MODULE_AUTHOR("Huawei Technologies Co, Ltd");
