/*
 * pcie2usb.c
 *
 * pcie2usb driver
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/ioctl.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/usb.h>
#include <linux/hisi/usb/tca.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/hisi/pcie-kport-api.h>
#include "pcie_misc.h"

static int g_wake_gpio = 0;
#define UPD720202_RST_GPIO 259
#define PULL_DOWN 0

enum {
	PCIE_DEVICE_NONE = 0,
	PCIE_DEVICE_UPD720202 = 1,
};

enum {
	PCIE0 = 0,
	PCIE1 = 1,
};

static irqreturn_t pcie2usb_wakeup_ep_handler(int irq, void *dev_id)
{
	pciedev_info("PCIe wakeup endpoint rc");
	return IRQ_HANDLED;
}

int pcie2usb_register_wakeup_gpio(struct platform_device *pdev)
{
	int ret;
	int irqn;

	g_wake_gpio = of_get_named_gpio(pdev->dev.of_node, "wake-gpio", 0);
	if (g_wake_gpio < 0) {
		pciedev_err("no wake gpio exist");
		ret = g_wake_gpio;
		return ret;
	}
	ret = gpio_request(g_wake_gpio, "pcie_wake");
	if (ret) {
		pciedev_err("Failed to request gpio-%d", g_wake_gpio);
		return ret;
	}
	gpio_direction_input(g_wake_gpio);
	gpio_to_irq(0);
	irqn = gpio_to_irq(g_wake_gpio);
	if (irqn < 0) {
		ret = irqn;
		gpio_free(g_wake_gpio);
		return ret;
	}

	pciedev_info("requesting IRQ %d with IRQF_NO_SUSPEND\n", irqn);

	ret = request_irq(irqn, pcie2usb_wakeup_ep_handler,
					IRQF_TRIGGER_FALLING | IRQF_NO_SUSPEND, "PCIE_WAKE", NULL);
	if (ret) {
		pciedev_err("request_irq failed\n");
		return ret;
	}
	enable_irq_wake(irqn);

	pciedev_info("pcie2usb_register_wakeup_gpio success");
	return ret;
}

static void pcie_enumerate_pcie1(struct platform_device *pdev)
{
	gpio_set_value(UPD720202_RST_GPIO, PULL_DOWN);
	mdelay(400);
	pcie_kport_enumerate(PCIE1);
	pcie2usb_register_wakeup_gpio(pdev);
	return;
}

static int pcie_misc_probe(struct platform_device *pdev)
{
	int ret;
	int pcie_type = 0;
	struct device *dev = &pdev->dev;
	struct device_node *np = dev->of_node;

	pciedev_info("pcie_misc probe ++\n");
	ret = of_property_read_u32(np, "pcie_device_type", (u32 *)&pcie_type);
	if (ret) {
		pciedev_err("get pcie_device_type failed\n");
	} else {
		pciedev_info("get pcie_device_type succeed\n");
	}

	if (pcie_type == PCIE1) {
		pcie_enumerate_pcie1(pdev);
	} else if (pcie_type == PCIE0) {
		pcie_kport_enumerate(PCIE0);
		mdelay(400);
	} else {
		pciedev_err("pcie_device_type failed\n");
		return -1;
	}

	pciedev_info("pcie_misc probe --\n");
	return ret;
}

static int pcie_misc_remove(struct platform_device *pdev)
{
	pciedev_info("pcie2usb remove\n");
	gpio_free(g_wake_gpio);

	return 0;
}

static const struct of_device_id pcie_misc_match[] = {
	{ .compatible = "huawei,pcie_0" },
	{ .compatible = "huawei,pcie_1" },
	{},
};


static struct platform_driver pcie_misc_driver = {
	.probe          = pcie_misc_probe,
	.remove         = pcie_misc_remove,
	.driver         = {
		.name   = "pcie_misc",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(pcie_misc_match),
		},
};

static int __init pcie_misc_init(void)
{
	return platform_driver_register(&pcie_misc_driver);
}

static void __exit pcie_misc_exit(void)
{
	platform_driver_unregister(&pcie_misc_driver);
}

module_init(pcie_misc_init);
module_exit(pcie_misc_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("huawei pcie misc control module driver");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
