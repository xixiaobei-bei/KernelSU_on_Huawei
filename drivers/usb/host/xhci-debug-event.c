 /*
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Description: Support for xhci debug notify
 * Author: Hisilicon
 * Create: 2020-08-31
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */

#include "xhci-debug-event.h"

#include <linux/hisi/usb/chip_usb_debug_framework.h>
#include <linux/hisi/usb/chip_usb_log.h>

static int xhci_host_event_notifier_nb(struct notifier_block *nb,
			unsigned long action, void *data)
{
	struct xhci_hcd *xhci = NULL;

	hiusb_pr_err("+\n");

	xhci = container_of(nb, struct xhci_hcd, host_event_nb);
	if (xhci == NULL) {
		hiusb_pr_err("xhc is NULL\n");
		return NOTIFY_OK;
	}

	if (action == USB_CORE_HOST_ENUM_ERR) {
		hiusb_pr_err("usb host enum error\n");
		xhci_dump_portsc(xhci);
	}

	if (action == USB_CORE_HOST_TRANS_TIMEOUT) {
		hiusb_pr_err("usb host transfer timeout\n");
		xhci_print_registers(xhci);
	}

	if (action == USB_CORE_HOST_RESUME_ERR) {
		hiusb_pr_err("usb host resume device error\n");
		xhci_dump_op_regs(xhci);
	}

	hiusb_pr_err("-\n");
	return NOTIFY_OK;
}

int xhci_host_register_eventnb(struct xhci_hcd *xhci)
{
	int ret;

	if (!xhci) {
		hiusb_pr_err("xhc is NULL\n");
		return -EINVAL;
	}

	xhci->host_event_nb.notifier_call = xhci_host_event_notifier_nb;
	ret = usb_blockerr_register_notify(&xhci->host_event_nb);
	if (ret) {
		hiusb_pr_err("xhci host event register failed\n");
		xhci->host_event_nb.notifier_call = NULL;
	}

	return ret;
}

void xhci_host_unregister_eventnb(struct xhci_hcd *xhci)
{
	if (!xhci) {
		hiusb_pr_err("xhc is NULL\n");
		return;
	}

	if (xhci->host_event_nb.notifier_call == NULL)
		return;

	usb_blockerr_unregister_notify(&xhci->host_event_nb);
	xhci->host_event_nb.notifier_call = NULL;
}

