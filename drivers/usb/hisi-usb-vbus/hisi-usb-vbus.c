/*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Description: support for usb-vbus driver
 * Author: Hisilicon
 * Create: 2015-12-09
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <huawei_platform/log/log_jank.h>
#include <linux/hisi/usb/hisi_usb.h>
#include <linux/hisi-spmi.h>
#include <linux/interrupt.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/mfd/hisi_pmic.h>
#include <linux/of_hisi_spmi.h>

static irqreturn_t charger_connect_interrupt(int irq, void *p)
{
	LOG_JANK_D(JLID_USBCHARGING_START, "JL_USBCHARGING_START");
	chip_usb_otg_event(CHARGER_CONNECT_EVENT);
	return IRQ_HANDLED;
}

static irqreturn_t charger_disconnect_interrupt(int irq, void *p)
{
	LOG_JANK_D(JLID_USBCHARGING_END, "JL_USBCHARGING_END");
	chip_usb_otg_event(CHARGER_DISCONNECT_EVENT);
	return IRQ_HANDLED;
}

static int vbus_connect_irq, vbus_disconnect_irq;

static int hisi_usb_spmi_vbus_probe(struct spmi_device *pdev)
{
	int ret;

	pr_info("[%s]+\n", __func__);

	vbus_connect_irq = spmi_get_irq_byname(pdev, &pdev->res, "connect");
	if (vbus_connect_irq == 0) {
		pr_err("failed to get connect irq\n");
		return -ENOENT;
	}
	vbus_disconnect_irq = spmi_get_irq_byname(pdev,
		&pdev->res, "disconnect");
	if (vbus_disconnect_irq == 0) {
		pr_err("failed to get disconnect irq\n");
		return -ENOENT;
	}

	pr_info("vbus_connect_irq: %d, vbus_disconnect_irq: %d\n",
		vbus_connect_irq, vbus_disconnect_irq);

	ret = request_irq(vbus_connect_irq, charger_connect_interrupt,
		IRQF_NO_SUSPEND, "hiusb_in_interrupt", pdev);
	if (ret) {
		pr_err("request charger connect irq failed\n");
		return ret;
	}

	ret = request_irq(vbus_disconnect_irq, charger_disconnect_interrupt,
		IRQF_NO_SUSPEND, "hiusb_in_interrupt", pdev);
	if (ret) {
		free_irq(vbus_disconnect_irq, pdev);
		pr_err("request charger connect irq failed\n");
		return ret;
	}

	/* avoid lose intrrupt */
	if (pmic_get_vbus_status()) {
		pr_info("%s: vbus high, issue a charger connect event\n",
			__func__);
		charger_connect_interrupt(vbus_connect_irq, pdev);
	} else {
		pr_info("%s: vbus low, issue a charger disconnect event\n",
			__func__);
		charger_disconnect_interrupt(vbus_disconnect_irq, pdev);
	}

	pr_info("[%s]-\n", __func__);

	return ret;
}

static int hisi_usb_spmi_vbus_remove(struct spmi_device *pdev)
{
	free_irq(vbus_connect_irq, pdev);
	free_irq(vbus_disconnect_irq, pdev);
	return 0;
}

static const struct of_device_id hisi_usb_vbus_of_match_table[] = {
	{ .compatible = "hisilicon-usbvbus", },
	{ },
};

static const struct spmi_device_id hisi_usb_vbus_of_match[] = {
	{ "hisilicon-usbvbus", 0 },
	{}
};

static struct spmi_driver hisi_usb_vbus_drv = {
	.probe		= hisi_usb_spmi_vbus_probe,
	.remove		= hisi_usb_spmi_vbus_remove,
	.driver		= {
		.owner		= THIS_MODULE,
		.name		= "hisi_spmi_usb_vbus",
		.of_match_table	= hisi_usb_vbus_of_match_table,
	},
	.id_table = hisi_usb_vbus_of_match,
};
module_driver(hisi_usb_vbus_drv, spmi_driver_register, spmi_driver_unregister);

MODULE_AUTHOR("Yu Liu <liuyu712@hisilicon.com>");
MODULE_DESCRIPTION("This module detect USB VBUS connection/disconnection use spmi interface");
MODULE_LICENSE("GPL v2");
