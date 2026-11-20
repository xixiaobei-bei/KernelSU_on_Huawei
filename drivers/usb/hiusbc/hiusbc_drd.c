/*
 * hiusbc_drd.c -- DRD Mode for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/extcon.h>
#include <linux/io.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include "hiusbc_core.h"
#include "hiusbc_gadget.h"
#include "hiusbc_drd.h"

static int hiusbc_drd_notifier(struct notifier_block *nb,
			     unsigned long event, void *ptr)
{
	struct hiusbc *hiusbc = container_of(nb, struct hiusbc, edev_nb);
	int host_attach = extcon_get_state(hiusbc->edev, EXTCON_USB_HOST);
	int device_attach = extcon_get_state(hiusbc->edev, EXTCON_USB);

	if (host_attach && device_attach)
		dev_err(hiusbc->dev, "host and device both attach!\n");

	else if (device_attach)
		hiusbc_set_mode(hiusbc, HIUSBC_DR_MODE_DEVICE);
	else if (host_attach)
		hiusbc_set_mode(hiusbc, HIUSBC_DR_MODE_HOST);
	else
		hiusbc_set_mode(hiusbc, HIUSBC_DR_MODE_UNKNOWN);

	return NOTIFY_DONE;
}

int hiusbc_drd_init(struct hiusbc *hiusbc)
{
	int ret;

	if (hiusbc->dev->of_node) {
		if (of_property_read_bool(hiusbc->dev->of_node, "extcon"))
			hiusbc->edev =
				extcon_get_edev_by_phandle(hiusbc->dev, 0);

		if (IS_ERR(hiusbc->edev)) {
			dev_err(hiusbc->dev, "fail to get edev\n");
			return PTR_ERR(hiusbc->edev);
		}

		hiusbc->edev_nb.notifier_call = hiusbc_drd_notifier;
		ret = extcon_register_notifier(hiusbc->edev, EXTCON_USB,
					       &hiusbc->edev_nb);
		if (ret < 0) {
			dev_err(hiusbc->dev, "fail to register EXTCON_USB\n");
			return ret;
		}

		ret = extcon_register_notifier(hiusbc->edev, EXTCON_USB_HOST,
						   &hiusbc->edev_nb);
		if (ret < 0) {
			dev_err(hiusbc->dev,
				"fail to register EXTCON_USB_HOST\n");
			return ret;
		}
	}

	return 0;
}

void hiusbc_drd_exit(struct hiusbc *hiusbc)
{
	extcon_unregister_notifier(hiusbc->edev,
			EXTCON_USB, &hiusbc->edev_nb);

	extcon_unregister_notifier(hiusbc->edev,
			EXTCON_USB_HOST, &hiusbc->edev_nb);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_UNKNOWN) {
		hiusbc_set_mode(hiusbc, HIUSBC_DR_MODE_DEVICE);
		/* Make sure enter device mode success. */
		if (hiusbc->current_dr_mode == HIUSBC_DR_MODE_DEVICE)
			hiusbc_gadget_exit(hiusbc);
	}
}
