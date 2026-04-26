/*
 * hiusbc_host.c -- Host Mode for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/platform_device.h>
#include <linux/ioport.h>

#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <linux/usb/of.h>
#include <securec.h>
#include "hiusbc_debug.h"
#include "hiusbc_gadget.h"
#include "hiusbc_host.h"

static struct platform_device *hiusbc_alloc_xhci_pdev(
			struct hiusbc *hiusbc,
			struct platform_device *hiusbc_pdev)
{
	struct platform_device *xhci = NULL;
	struct resource host_res[HIUSBC_HOST_RESOURCE_NUM];
	struct resource *res_temp = NULL;
	int irq;
	int ret;

	res_temp = platform_get_resource(hiusbc_pdev, IORESOURCE_MEM, 0);
	if (!res_temp) {
		dev_err(hiusbc->dev, "Can't get memory resource of xhci.\n");
		return NULL;
	}

	if (memset_s(host_res, sizeof(host_res), 0, sizeof(host_res)) != EOK)
		return NULL;

	host_res[0].start = res_temp->start - HIUSBC_DEVICE_MODE_REG_BASE;
	host_res[0].end = host_res[0].start + HIUSBC_HOST_XHCI_REG_END;
	host_res[0].flags = res_temp->flags;
	host_res[0].name = res_temp->name;

	irq = hiusbc_get_irq(hiusbc);
	if (irq < 0) {
		dev_err(hiusbc->dev, "Can't get irq of xhci.\n");
		return NULL;
	}

	res_temp = platform_get_resource_byname(
		hiusbc_pdev, IORESOURCE_IRQ, "host");
	if (!res_temp)
		res_temp = platform_get_resource_byname(
			hiusbc_pdev, IORESOURCE_IRQ, "hiusbc");

	if (!res_temp)
		res_temp = platform_get_resource(
			hiusbc_pdev, IORESOURCE_IRQ, 0);

	if (!res_temp) {
		dev_err(hiusbc->dev, "Can't get io resource of xhci.\n");
		return NULL;
	}

	host_res[1].start = irq;
	host_res[1].end = irq;
	host_res[1].flags = res_temp->flags;
	host_res[1].name = res_temp->name;

	xhci = platform_device_alloc("xhci-hcd", PLATFORM_DEVID_AUTO);
	if (!xhci)
		return NULL;

	ret = platform_device_add_resources(
		xhci, host_res, HIUSBC_HOST_RESOURCE_NUM);
	if (ret) {
		dev_err(hiusbc->dev, "Can't add resources to xhci.\n");
		platform_device_put(xhci);
		return NULL;
	}

	return xhci;
}
int hiusbc_host_init(struct hiusbc *hiusbc)
{
	struct platform_device *hiusbc_pdev = to_platform_device(hiusbc->dev);
	struct platform_device *xhci = NULL;
	struct property_entry props[2];
	int prop_idx = 0;

	int ret;

	xhci = hiusbc_alloc_xhci_pdev(hiusbc, hiusbc_pdev);
	if (!xhci)
		return -ENOMEM;

	xhci->dev.parent = hiusbc->dev;
	hiusbc->xhci = xhci;

	if (memset_s(props, sizeof(props), 0, sizeof(props)) != EOK)
		goto exit;

	if (hiusbc->usb3_host_lpm_capable)
		props[prop_idx++].name = "usb3-lpm-capable";

	if (prop_idx) {
		ret = platform_device_add_properties(xhci, props);
		if (ret) {
			dev_err(hiusbc->dev, "Can't add properties to xhci\n");
			goto exit;
		}
	}

	ret = platform_device_add(xhci);
	if (ret) {
		dev_err(hiusbc->dev, "Failed to register xhci.\n");
		goto exit;
	}
	return 0;
exit:
	platform_device_put(xhci);
	return ret;
}

void hiusbc_host_exit(struct hiusbc *hiusbc)
{
	platform_device_unregister(hiusbc->xhci);
}
