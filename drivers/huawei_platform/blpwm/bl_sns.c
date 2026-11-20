/*
 * bl_sns.c
 *
 * bl_sns driver
 *
 * Copyright (c) 2021-2021 Huawei Technologies Co., Ltd.
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
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_gpio.h>
#include <linux/of_platform.h>
#include <linux/suspend.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/types.h>
#include <linux/completion.h>
#include <huawei_platform/log/hw_log.h>
#include "contexthub_boot.h"
#include "contexthub_recovery.h"
#include "contexthub_route.h"

#ifdef HWLOG_TAG
#undef HWLOG_TAG
#endif

#define HWLOG_TAG bl_sns
HWLOG_REGIST();
#define BLPWM_RSP_TIMEOUT 500

struct bl_sns_dev {
	int sns_support_rsp;
	struct completion blpwm_support_rsp;
};

static struct bl_sns_dev *g_bl_sns_dev;

static int bl_sns_cover_detect(void)
{
	reinit_completion(&g_bl_sns_dev->blpwm_support_rsp);

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_ADC_V2);
	if (!wait_for_completion_interruptible_timeout(&g_bl_sns_dev->blpwm_support_rsp,
		msecs_to_jiffies(BLPWM_RSP_TIMEOUT)))
		hwlog_info("%s: wait for sensorhub rsp time out\n", __func__);

	hwlog_info("%s done, support is %d\n", __func__, g_bl_sns_dev->sns_support_rsp);
	return g_bl_sns_dev->sns_support_rsp;
}

static void bl_sns_set_duty(int duty)
{
	if (!blpwm_is_support())
		return;

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_PWM);
}

static void bl_sns_set_usage(int usage)
{
	if (!blpwm_is_support())
		return;

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_USAGE);
}

static void bl_sns_set_usage_stop(int usage_stop)
{
	if (!blpwm_is_support())
		return;

	blpwm_open_sensorhub();
	blpwm_config_sensorhub(SUB_CMD_BLPWM_USAGE_STOP);
}

static void bl_sns_notify(pkt_blpwm_sensor_req_t *msg)
{
	if (!g_bl_sns_dev)
		return;

	if (msg->status == BLPWM_SUPPORT_RSP) {
		g_bl_sns_dev->sns_support_rsp = msg->support;
		hwlog_info("%s: get support:%d info from sns\n",
			__func__, msg->support);
		complete_all(&g_bl_sns_dev->blpwm_support_rsp);
	} else {
		hwlog_err("%s: invalid status\n", __func__);
	}
}

static struct blpwm_ops g_ops = {
	.blpwm_cover_detect = bl_sns_cover_detect,
	.blpwm_set_duty = bl_sns_set_duty,
	.blpwm_set_usage = bl_sns_set_usage,
	.blpwm_set_usage_stop = bl_sns_set_usage_stop,
	.blpwm_notiby_by_sns = bl_sns_notify,
};

static int bl_sns_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;

	g_bl_sns_dev = devm_kzalloc(dev, sizeof(*g_bl_sns_dev), GFP_KERNEL);
	if (!g_bl_sns_dev)
		return -ENOMEM;

	blpwm_register_ops(&g_ops);
	init_completion(&g_bl_sns_dev->blpwm_support_rsp);

	return 0;
}

static int bl_sns_remove(struct platform_device *pdev)
{
	if (!g_bl_sns_dev)
		return -1;

	g_bl_sns_dev = NULL;

	return 0;
}

static const struct of_device_id bl_sns_of_match[] = {
	{ .compatible = "huawei,bl_sns", },
	{},
};
MODULE_DEVICE_TABLE(of, bl_sns_of_match);

static struct platform_driver bl_sns_driver = {
	.driver = {
		.name = "bl_sns",
		.owner = THIS_MODULE,
		.of_match_table = bl_sns_of_match,
	},
	.probe = bl_sns_probe,
	.remove = bl_sns_remove,
};

static int __init bl_sns_init(void)
{
	return platform_driver_register(&bl_sns_driver);
}

static void __exit bl_sns_exit(void)
{
	platform_driver_unregister(&bl_sns_driver);
}

device_initcall(bl_sns_init);
module_exit(bl_sns_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("bl_sns driver");
MODULE_AUTHOR("Huawei Technologies Co, Ltd");
