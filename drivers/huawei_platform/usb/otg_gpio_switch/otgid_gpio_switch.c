/*
 * otgid_gpio_switch.c
 *
 * gpio switch for otgid driver
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

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/switch.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#include <linux/of_gpio.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <huawei_platform/log/hw_log.h>

#define HWLOG_TAG otgid_gpio_switch
HWLOG_REGIST();

#define GPIO_HIGH     1
#define GPIO_LOW      0
#define DEBOUNCE      500

struct gpio_switch_device_info {
	int gpio_int;
	int irq_int;
	int last_state;
	struct timer_list timer;
	struct delayed_work timer_work;
};

static struct gpio_switch_device_info *g_gpio_switch_dev;

static void gpio_switch_timer_work_func(struct work_struct *work)
{
	int state;
	struct gpio_switch_device_info *di = g_gpio_switch_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	state = gpio_get_value(di->gpio_int);
	hwlog_info("gpio_int state=%d\n", state);

	if (di->last_state == GPIO_HIGH && state == GPIO_LOW) {
		hwlog_err("otgid fall event\n");
		hisi_usb_id_change(ID_FALL_EVENT);
		di->last_state = state;
		return;
	}

	if (di->last_state == GPIO_LOW && state == GPIO_HIGH) {
		hwlog_err("otgid rise event\n");
		hisi_usb_id_change(ID_RISE_EVENT);
		di->last_state = state;
		return;
	}
}

static void gpio_switch_timer(unsigned long data)
{
	struct gpio_switch_device_info *di = g_gpio_switch_dev;

	if (!di) {
		hwlog_err("di is null\n");
		return;
	}

	cancel_delayed_work(&di->timer_work);
	schedule_delayed_work(&di->timer_work, msecs_to_jiffies(0));
}

static irqreturn_t gpio_switch_interrupt(int irq, void *_di)
{
	struct gpio_switch_device_info *di = _di;

	if (!di) {
		hwlog_err("di is null\n");
		return -1;
	}

	hwlog_info("gpio_switch int happened\n");

	if (irq == di->irq_int)
		mod_timer(&di->timer, jiffies + msecs_to_jiffies(DEBOUNCE));

	return IRQ_HANDLED;
}

static int gpio_switch_probe(struct platform_device *pdev)
{
	int ret;
	struct gpio_switch_device_info *di = NULL;
	struct device_node *np = NULL;

	hwlog_info("probe begin\n");

	if (!pdev || !pdev->dev.of_node)
		return -ENODEV;

	di = kzalloc(sizeof(*di), GFP_KERNEL);
	if (!di)
		return -ENOMEM;

	g_gpio_switch_dev = di;
	np = pdev->dev.of_node;

	di->gpio_int = of_get_named_gpio(np, "otg_int_gpio", 0);
	hwlog_info("gpio_int=%d\n", di->gpio_int);

	if (!gpio_is_valid(di->gpio_int)) {
		hwlog_err("gpio is not valid\n");
		ret = -EINVAL;
		goto fail_free_mem;
	}

	ret = gpio_request(di->gpio_int, "otg_int_gpio");
	if (ret < 0) {
		hwlog_err("gpio request fail\n");
		goto fail_free_mem;
	}

	ret = gpio_direction_input(di->gpio_int);
	if (ret) {
		hwlog_err("gpio set input fail\n");
		goto fail_free_gpio_int;
	}

	di->irq_int = gpio_to_irq(di->gpio_int);
	if (di->irq_int < 0) {
		hwlog_err("gpio map to irq fail\n");
		ret = -EINVAL;
		goto fail_free_gpio_int;
	}

	INIT_DELAYED_WORK(&di->timer_work, gpio_switch_timer_work_func);
	setup_timer(&di->timer, gpio_switch_timer, (unsigned long)di);

	ret = request_irq(di->irq_int, gpio_switch_interrupt,
		IRQF_NO_SUSPEND | IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		"otg_int_irq", di);
	if (ret) {
		hwlog_err("gpio irq request fail\n");
		goto fail_free_gpio_int;
	}

	/* perform initial detection */
	di->last_state = GPIO_HIGH;
	if (!gpio_get_value_cansleep(di->gpio_int)) {
		hwlog_err("gpio get value fail\n");
		mod_timer(&di->timer, jiffies + msecs_to_jiffies(DEBOUNCE));
	}

	hwlog_info("probe end\n");
	return 0;

fail_free_gpio_int:
	gpio_free(di->gpio_int);
fail_free_mem:
	kfree(di);
	g_gpio_switch_dev = NULL;

	return ret;
}

static int gpio_switch_remove(struct platform_device *pdev)
{
	struct gpio_switch_device_info *di = g_gpio_switch_dev;

	hwlog_info("remove begin\n");

	if (!di)
		return -ENODEV;

	gpio_free(di->gpio_int);
	free_irq(di->irq_int, di);
	cancel_delayed_work_sync(&di->timer_work);
	kfree(di);
	g_gpio_switch_dev = NULL;

	hwlog_info("remove end\n");
	return 0;
}

static const struct of_device_id gpio_switch_otg_ids[] = {
	{
		.compatible = "huawei,gpio_switch_otg",
		.data = NULL,
	},
	{},
};

static struct platform_driver gpio_switch_driver = {
	.probe = gpio_switch_probe,
	.remove = gpio_switch_remove,
	.driver = {
		.name = "switch-gpio",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(gpio_switch_otg_ids),
	},
};

static int __init gpio_switch_init(void)
{
	return platform_driver_register(&gpio_switch_driver);
}

static void __exit gpio_switch_exit(void)
{
	platform_driver_unregister(&gpio_switch_driver);
}

module_init(gpio_switch_init);
module_exit(gpio_switch_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("gpio switch for otgid driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
