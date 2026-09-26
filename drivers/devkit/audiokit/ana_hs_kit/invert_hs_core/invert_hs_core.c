/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description: invert headset driver
 * Author: lijinkui
 * Create: 2019-7-16
 */

#include "invert_hs_core.h"
#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/mutex.h>
#include <linux/string.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/pm_wakeup.h>
#include <linux/workqueue.h>
#include <linux/regulator/consumer.h>
#include <linux/of_irq.h>
#include <linux/of_gpio.h>
#include <linux/uaccess.h>
#include <linux/pinctrl/consumer.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG invert_hs
HWLOG_REGIST();

#define WAKE_LOCK_TIMEOUT               1000 /* ms */
#define WORK_DELAY_TIME                   3000 /* ms */

enum invert_hs_gpio_type {
	INVERT_HS_GPIO_SOC           = 0,
	INVERT_HS_GPIO_CODEC         = 1,
};

struct invert_hs_data {
	/* switch chip control gpio */
	int gpio_mic_gnd;
	int gpio_type;
	struct mutex invert_hs_lock;
	struct wakeup_source wake_lock;
	struct workqueue_struct *anc_hs_invert_ctl_delay_wq;
	struct delayed_work anc_hs_invert_ctl_delay_work;
};

static struct invert_hs_data *invert_hs_pdata;

static void invert_hs_gpio_set_value(int gpio, int value)
{
	if (invert_hs_pdata->gpio_type == INVERT_HS_GPIO_CODEC)
		gpio_set_value_cansleep(gpio, value);
	else
		gpio_set_value(gpio, value);
}

static void anc_hs_invert_ctl_work(struct work_struct *work)
{
	__pm_stay_awake(&invert_hs_pdata->wake_lock);
	mutex_lock(&invert_hs_pdata->invert_hs_lock);

	invert_hs_gpio_set_value(invert_hs_pdata->gpio_mic_gnd,
		INVERT_HS_MIC_GND_CONNECT);

	mutex_unlock(&invert_hs_pdata->invert_hs_lock);
	__pm_relax(&invert_hs_pdata->wake_lock);
}

void invert_hs_core_control(int connect)
{
	/* invert_hs driver not be probed, just return */
	if (!invert_hs_pdata) {
		hwlog_info("invert_hs_pdata is null\n");
		return;
	}

	__pm_wakeup_event(&invert_hs_pdata->wake_lock, WAKE_LOCK_TIMEOUT);

	switch (connect) {
	case INVERT_HS_MIC_GND_DISCONNECT:
		cancel_delayed_work(
			&invert_hs_pdata->anc_hs_invert_ctl_delay_work);
		flush_workqueue(invert_hs_pdata->anc_hs_invert_ctl_delay_wq);
		__pm_stay_awake(&invert_hs_pdata->wake_lock);
		mutex_lock(&invert_hs_pdata->invert_hs_lock);
		invert_hs_gpio_set_value(invert_hs_pdata->gpio_mic_gnd,
			INVERT_HS_MIC_GND_DISCONNECT);
		mutex_unlock(&invert_hs_pdata->invert_hs_lock);
		__pm_relax(&invert_hs_pdata->wake_lock);
		hwlog_info("%s: disconnect MIC and GND", __func__);
		break;
	case INVERT_HS_MIC_GND_CONNECT:
		queue_delayed_work(invert_hs_pdata->anc_hs_invert_ctl_delay_wq,
				&invert_hs_pdata->anc_hs_invert_ctl_delay_work,
				msecs_to_jiffies(WORK_DELAY_TIME));
		hwlog_info("%s: queue delay work", __func__);
		break;
	default:
		hwlog_info("%s: unknown connect type", __func__);
		break;
	}

	return;
}

struct ana_hs_dev invert_hs_core_ops = {
	.tag = TAG_INVERT_HS,
	.ops.invert_hs_control = invert_hs_core_control,
};

static const struct of_device_id invert_hs_core_of_match[] = {
	{ .compatible = "huawei,invert_hs_core", },
	{},
};
MODULE_DEVICE_TABLE(of, invert_hs_core_of_match);

/* load dts config for board difference */
static void load_invert_hs_config(struct device_node *node)
{
	int temp;

	if (!of_property_read_u32(node, "gpio_type", &temp))
		invert_hs_pdata->gpio_type = temp;
	else
		invert_hs_pdata->gpio_type = INVERT_HS_GPIO_SOC;
}

static int invert_hs_init_mic_gnd_gpio(struct device_node *node)
{
	int ret;

	/* get mic->gnd control gpio */
	invert_hs_pdata->gpio_mic_gnd =  of_get_named_gpio(node, "gpios", 0);
	if (invert_hs_pdata->gpio_mic_gnd < 0) {
		hwlog_err("gpio_mic_gnd is unvalid\n");
		return -ENOENT;
	}

	if (!gpio_is_valid(invert_hs_pdata->gpio_mic_gnd)) {
		hwlog_err("gpio is unvalid\n");
		return -ENOENT;
	}

	/* apply for mic->gnd gpio */
	ret = gpio_request(invert_hs_pdata->gpio_mic_gnd, "gpio_mic_gnd");
	if (ret) {
		hwlog_err("error request GPIO for mic_gnd fail %d\n", ret);
		return -ENOENT;
	}
	gpio_direction_output(invert_hs_pdata->gpio_mic_gnd,
		INVERT_HS_MIC_GND_CONNECT);
	return 0;
}

static int invert_hs_create_workqueue(void)
{
	mutex_init(&invert_hs_pdata->invert_hs_lock);
	wakeup_source_init(&invert_hs_pdata->wake_lock, "invert_hs_wakelock");

	invert_hs_pdata->anc_hs_invert_ctl_delay_wq =
		create_singlethread_workqueue("anc_hs_invert_ctl_delay_wq");
	if (!(invert_hs_pdata->anc_hs_invert_ctl_delay_wq)) {
		hwlog_err("%s: invert_ctl wq create failed\n", __func__);
		return -ENOMEM;
	}
	INIT_DELAYED_WORK(&invert_hs_pdata->anc_hs_invert_ctl_delay_work,
			anc_hs_invert_ctl_work);
	return 0;
}

static void invert_hs_remove_workqueue(void)
{
	if (!invert_hs_pdata->anc_hs_invert_ctl_delay_wq)
		return;
	cancel_delayed_work(&invert_hs_pdata->anc_hs_invert_ctl_delay_work);
	flush_workqueue(invert_hs_pdata->anc_hs_invert_ctl_delay_wq);
	destroy_workqueue(invert_hs_pdata->anc_hs_invert_ctl_delay_wq);
}

static int invert_hs_core_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct device_node *node = dev->of_node;
	int ret;
	struct pinctrl *p = NULL;
	struct pinctrl_state *pinctrl_def = NULL;

	invert_hs_pdata = devm_kzalloc(dev, sizeof(*invert_hs_pdata),
		GFP_KERNEL);
	if (!invert_hs_pdata)
		return -ENOMEM;

	ret = invert_hs_create_workqueue();
	if (ret)
		goto err_out;

	p = devm_pinctrl_get(dev);
	hwlog_info("invert_hs:node name is %s\n", node->name);
	if (IS_ERR(p)) {
		hwlog_err("could not get pinctrl dev\n");
		ret = -ENOENT;
		goto err_invert_ctl_delay_wq;
	}
	pinctrl_def = pinctrl_lookup_state(p, "default");
	if (IS_ERR(pinctrl_def))
		hwlog_info("could not get defstate\n");

	ret = pinctrl_select_state(p, pinctrl_def);
	if (ret)
		hwlog_info("could not set pins to default state\n");

	ret = invert_hs_init_mic_gnd_gpio(node);
	if (ret)
		goto err_invert_ctl_delay_wq;

	load_invert_hs_config(node);

	ana_hs_dev_register(&invert_hs_core_ops, ANA_PRIORITY_H);

	hwlog_info("invert_hs probe success\n");
	return 0;

err_invert_ctl_delay_wq:
	invert_hs_remove_workqueue();
err_out:
	return ret;
}


static int invert_hs_core_remove(struct platform_device *pdev)
{
	if (!invert_hs_pdata)
		return 0;

	ana_hs_dev_deregister(TAG_INVERT_HS);
	invert_hs_remove_workqueue();

	gpio_free(invert_hs_pdata->gpio_mic_gnd);

	return 0;
}

static struct platform_driver invert_hs_core_driver = {
	.driver = {
		.name   = "invert_hs_core",
		.owner  = THIS_MODULE,
		.of_match_table = invert_hs_core_of_match,
	},
	.probe  = invert_hs_core_probe,
	.remove = invert_hs_core_remove,
};

static int __init invert_hs_core_init(void)
{
	return platform_driver_register(&invert_hs_core_driver);
}

static void __exit invert_hs_core_exit(void)
{
	platform_driver_unregister(&invert_hs_core_driver);
}

device_initcall(invert_hs_core_init);
module_exit(invert_hs_core_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("invert headset driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
