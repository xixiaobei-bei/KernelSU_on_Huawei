/*
 * hiusbc_req.c -- Device Request Related for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/usb/ch9.h>
#include <linux/usb/composite.h>
#include <linux/usb/gadget.h>
#include <securec.h>
#include "hiusbc_core.h"
#include "hiusbc_debug.h"
#include "hiusbc_gadget.h"
#include "hiusbc_transfer.h"
#include "hiusbc_setup.h"

int hiusbc_req_set_test_mode(struct hiusbc *hiusbc, u8 mode)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+: mode = %u\n", mode);

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2PORT_TEST_MODE_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_DEV_TEST_MODE_MASK;

	switch (mode) {
	case TEST_J:
	case TEST_K:
	case TEST_SE0_NAK:
	case TEST_PACKET:
	case TEST_FORCE_EN:
		reg |= hiusbc_u2_test_mode(mode);
		break;
	case 0:
		break;
	default:
		return -EINVAL;
	}

	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2PORT_TEST_MODE_OFFSET);
	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");
	return 0;
}

static u16 _req_get_status_dev(struct hiusbc *hiusbc)
{
	u16 status;

	status = hiusbc->gadget.is_selfpowered << USB_DEVICE_SELF_POWERED;
	if (hiusbc->gadget.speed == USB_SPEED_SUPER ||
		hiusbc->gadget.speed == USB_SPEED_SUPER_PLUS) {
		status |= (hiusbc->u1_enable << USB_DEV_STAT_U1_ENABLED) |
			(hiusbc->u2_enable << USB_DEV_STAT_U2_ENABLED);
	} else {
		/*
		 * 3.1 Spec 9.4.5: For Enhanced Super Speed,
		 * mote Wakeup is reserved and must be set to zero.
		 */
		status |= hiusbc->remote_wakeup << USB_DEVICE_REMOTE_WAKEUP;
	}
	return status;
}

static int _req_get_status_ep(struct hiusbc *hiusbc,
			const struct usb_ctrlrequest *setup, u16 *status)
{
	u8 epnum;
	struct hiusbc_ep *hep = NULL;
	enum usb_device_state state = hiusbc->gadget.state;

	*status = 0;
	epnum = (setup->wIndex & USB_ENDPOINT_NUMBER_MASK) << 1;
	if ((setup->wIndex & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
		epnum += 1;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: EPNum: %u\n", epnum);

	if ((state == USB_STATE_ADDRESS) && (epnum > 1)) {
		/* USB3.1 Spec 9.4.5 */
		dev_err(hiusbc->dev, "Get Status: don't support non-ep0\n");
		return -EINVAL;
	}

	if (epnum >= HIUSBC_EP_NUMS) {
		dev_err(hiusbc->dev,
			"Get Status: EPNum=%u is invalid\n", epnum);
		return -EINVAL;
	}

	hep = hiusbc->eps[epnum];

	if (!hep) {
		dev_err(hiusbc->dev,
			"Get Status: EP%u still not allcated\n", epnum);
		return -EINVAL;
	}

	if (hep->stalled || hep->set_halt_pending)
		*status = 1 << USB_ENDPOINT_HALT;

	return 0;
}

static int hiusbc_req_get_status(struct hiusbc *hiusbc,
				const struct usb_ctrlrequest *setup)
{
	int ret;
	u16 status;
	struct hiusbc_ep *hep = NULL;
	__le16 *buf = (__le16 *)hiusbc->dev_req_response_buf;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Get Status:\n");

	if (setup->wValue == 0x01) {
		dev_err(hiusbc->dev, "don't support PTM in device mode.\n");
		return -EINVAL;
	}

	switch (setup->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: DEVICE\n");
		status = _req_get_status_dev(hiusbc);
		break;
	case USB_RECIP_INTERFACE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: INTERFACE\n");
		if (hiusbc->gadget.speed == USB_SPEED_SUPER ||
			hiusbc->gadget.speed == USB_SPEED_SUPER_PLUS)
			dev_err(hiusbc->dev,
				"Get Status Interface do nothing\n");

		break;
	case USB_RECIP_ENDPOINT:
		ret = _req_get_status_ep(hiusbc, setup, &status);
		if (ret)
			return ret;
		break;
	default:
		dev_err(hiusbc->dev, "Get Status: unknown recipient!\n");
		return -EINVAL;
	}

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Returned status: 0x%x\n", status);

	*buf = cpu_to_le16(status);

	/*
	 * we always queue ctrl request on EP0,
	 * the same as what usb core do.
	 */
	hep = hiusbc->eps[0];
	hiusbc->dev_req.hep = hep;
	hiusbc->dev_req.epnum = hep->epnum;
	hiusbc->dev_req.req.length = sizeof(*buf);
	hiusbc->dev_req.req.buf = hiusbc->dev_req_response_buf;
	hiusbc->dev_req.req.complete = NULL;

	return hiusbc_ep_queue(&hep->ep, &hiusbc->dev_req.req);
}

static int _req_feature_device_testmode(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	u32 reg;
	u8 test_mode;

	if (!set || (setup->wIndex & 0xff))
		return -EINVAL;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "disable USB2 LPM for test mode!\n");
	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_LPM_ENABLE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);

	test_mode = setup->wIndex >> 8;
	switch (test_mode) {
	case TEST_J:
	case TEST_K:
	case TEST_SE0_NAK:
	case TEST_PACKET:
	case TEST_FORCE_EN:
		hiusbc->test_mode = test_mode;
		break;
	default:
		return -EINVAL;
	}
	return 0;
}

static int _req_feature_device_u1_enable(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	u32 reg;

	if (((hiusbc->gadget.speed != USB_SPEED_SUPER) &&
		(hiusbc->gadget.speed != USB_SPEED_SUPER_PLUS)) ||
		(hiusbc->gadget.state != USB_STATE_CONFIGURED))
		return -EINVAL;

	if (!hiusbc->usb3_dev_lpm_capable) {
		hiusbc->u1_enable = false;
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "usb3_dev_lpm_capable is off\n");
		return 0;
	}

	hiusbc->u1_enable = set;
	if (hiusbc->usb3_dev_lpm_u1_initiate) {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
		if (set)
			reg |= MPI_APP_DEVICE_REG_ENABLE_U1_INITIATE_MASK;
		else
			reg &= ~MPI_APP_DEVICE_REG_ENABLE_U1_INITIATE_MASK;
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	}
	return 0;
}

static int _req_feature_device_u2_enable(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	u32 reg;

	if (((hiusbc->gadget.speed != USB_SPEED_SUPER) &&
		(hiusbc->gadget.speed != USB_SPEED_SUPER_PLUS)) ||
		(hiusbc->gadget.state != USB_STATE_CONFIGURED))
		return -EINVAL;

	if (!hiusbc->usb3_dev_lpm_capable) {
		hiusbc->u2_enable = false;
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"usb3_dev_lpm_capable is off\n");
		return 0;
	}

	hiusbc->u2_enable = set;
	if (hiusbc->usb3_dev_lpm_u2_initiate) {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
		if (set)
			reg |= MPI_APP_DEVICE_REG_ENABLE_U2_INITIATE_MASK;
		else
			reg &= ~MPI_APP_DEVICE_REG_ENABLE_U2_INITIATE_MASK;

		hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	}
	return 0;
}

static int hiusbc_req_feature_device(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	int ret;

	switch (setup->wValue) {
	case USB_DEVICE_REMOTE_WAKEUP:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Feature: REMOTE_WAKEUP\n");
		/*
		 * USB3.1 Spec 9.4.5: For Enhanced Super Speed,
		 * Remote Wakeup is reserved and must be set to zero.
		 * Do nothing for USB3
		 * USB3 use FUNCTION_SUSPEND feature,
		 * which is only defined for an interface recipient.
		 */
		if (hiusbc->gadget.speed != USB_SPEED_SUPER &&
			hiusbc->gadget.speed != USB_SPEED_SUPER_PLUS)
			hiusbc->remote_wakeup = set;

		break;
	case USB_DEVICE_TEST_MODE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
			"Feature: TEST_MODE = %u.\n", setup->wIndex >> 8);
		ret = _req_feature_device_testmode(hiusbc, setup, set);
		break;
	case USB_DEVICE_U1_ENABLE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Feature: U1_ENABLE\n");
		ret = _req_feature_device_u1_enable(hiusbc, setup, set);
		break;
	case USB_DEVICE_U2_ENABLE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
			"Feature: U2_ENABLE, Speed: %d, State: %d\n",
			hiusbc->gadget.speed, hiusbc->gadget.state);
		ret = _req_feature_device_u2_enable(hiusbc, setup, set);
		break;
	case USB_DEVICE_LTM_ENABLE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
				"Feature: LTM_ENABLE, State: %d\n",
				hiusbc->gadget.state);
		ret = -EINVAL;
		break;
	default:
		dev_err(hiusbc->dev, "%s Device Feature unknown Selector!\n",
				set ? "Set" : "Clear");
		ret = -EINVAL;
		break;
	}

	return ret;
}

static int hiusbc_req_feature_interface(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	int ret = 0;

	switch (setup->wValue) {
	case USB_INTRF_FUNC_SUSPEND:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
			"Feature: FUNC_SUSPEND, Speed: %d, State: %d\n",
			hiusbc->gadget.speed, hiusbc->gadget.state);
		if (((hiusbc->gadget.speed != USB_SPEED_SUPER) &&
			(hiusbc->gadget.speed != USB_SPEED_SUPER_PLUS)) ||
			(hiusbc->gadget.state != USB_STATE_CONFIGURED))
			ret = -EINVAL;
		break;
	default:
		dev_err(hiusbc->dev,
			"%s Interface Feature unknown Selector!\n",
			set ? "Set" : "Clear");
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int _req_feature_ep_halt(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	struct hiusbc_ep *hep = NULL;
	enum usb_device_state state = hiusbc->gadget.state;
	u8 epnum;

	epnum = (setup->wIndex & USB_ENDPOINT_NUMBER_MASK) << 1;
	if ((setup->wIndex & USB_ENDPOINT_DIR_MASK) == USB_DIR_IN)
		epnum += 1;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"EPNum: %u, Device State: %d\n", epnum, state);

	if ((state == USB_STATE_ADDRESS) && (epnum > 1))
		/* USB3.1 Spec 9.4.5 */
		return -EINVAL;

	if (epnum >= HIUSBC_EP_NUMS)
		return -EINVAL;

	/*
	 * Host can't set halt to EP0&EP1.
	 * and STALL on EP0&EP1 are cleared when Setup pkt received.
	 */
	if (epnum <= 1 && set) {
		pr_err("Host can't set halt to ctrl EP%u.\n", epnum);
		return -EINVAL;
	}

	hep = hiusbc->eps[epnum];

	if (!hep)
		return -EINVAL;

	/* Ignore request if ep is wedget. */
	if (hep->wedged)
		return 0;

	return hiusbc_set_ep_stall(hep, set);
}

static int hiusbc_req_feature_endpoint(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	int ret;

	switch (setup->wValue) {
	case USB_ENDPOINT_HALT:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Feature: HALT\n");
		ret = _req_feature_ep_halt(hiusbc, setup, set);
		break;
	default:
		dev_err(hiusbc->dev,
			"%s Interface Feature unknown Selector!\n",
			set ? "Set" : "Clear");
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int hiusbc_req_feature(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup, bool set)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Start handling std req: %s Feature:\n",
		set ? "Set" : "Clear");

	switch (setup->bRequestType & USB_RECIP_MASK) {
	case USB_RECIP_DEVICE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: DEVICE\n");
		ret = hiusbc_req_feature_device(hiusbc, setup, set);
		break;
	case USB_RECIP_INTERFACE:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: INTERFACE\n");
		ret = hiusbc_req_feature_interface(hiusbc, setup, set);
		break;
	case USB_RECIP_ENDPOINT:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Recipient: ENDPOINT\n");
		ret = hiusbc_req_feature_endpoint(hiusbc, setup, set);
		break;
	default:
		dev_err(hiusbc->dev, "%s Feature: Unknown recipient!\n",
			set ? "Set" : "Clear");
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int hiusbc_req_set_addr(struct hiusbc *hiusbc,
				const struct usb_ctrlrequest *setup)
{
	enum usb_device_state state = hiusbc->gadget.state;
	u8 addr;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"Start handling std req: Set Address=%u\n", setup->wValue);

	if (setup->wValue > 127) {
		dev_err(hiusbc->dev, "address % is greater than 127!\n",
			setup->wValue);
		return -EINVAL;
	}

	addr = (u8)(setup->wValue & 0x7f);

	if (state == USB_STATE_DEFAULT || state == USB_STATE_ADDRESS) {
		dev_err(hiusbc->dev,
			"set addr to 0x%x in Status Stage.\n", addr);
		return 0;
	}

	dev_err(hiusbc->dev,
		"Set Address: wrong device state = %i!\n", state);
	return -EINVAL;
}

static void hiusbc_enable_accept_lpm(struct hiusbc *hiusbc)
{
	u32 reg;

	if ((hiusbc->gadget.speed >= USB_SPEED_SUPER) &&
		hiusbc->usb3_dev_lpm_capable) {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

		if (hiusbc->usb3_dev_lpm_u1_accept)
			reg |= MPI_APP_DEVICE_REG_ENABLE_U1_ACCEPT_MASK;

		if (hiusbc->usb3_dev_lpm_u2_accept)
			reg |= MPI_APP_DEVICE_REG_ENABLE_U2_ACCEPT_MASK;

		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "Enable U1 U2 Accept.\n");
	}
}

static int hiusbc_req_set_config(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup)
{
	enum usb_device_state state = hiusbc->gadget.state;
	u16 config = le16_to_cpu(setup->wValue);
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"config = %u, state = %d\n", config, state);

	if (state != USB_STATE_ADDRESS && state != USB_STATE_CONFIGURED) {
		dev_err(hiusbc->dev,
			"Set Config: wrong device state = %d!\n", state);
		return -EINVAL;
	}

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Delegate Set Configfgfg to usbcore.\n");
	ret = hiusbc_gadget_delegate_req(hiusbc, setup);

	if (state == USB_STATE_CONFIGURED) {
		if (!config && !ret)
			usb_gadget_set_state(&hiusbc->gadget,
					USB_STATE_ADDRESS);
		return ret;
	}

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Ret from usbcore = %d.\n", ret);
	if (config && (!ret || ret == USB_GADGET_DELAYED_STATUS)) {
		if (!ret) {
			usb_gadget_set_state(&hiusbc->gadget,
					USB_STATE_CONFIGURED);
			hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"usbcore Set Configuration success.\n");
		}

		hiusbc_enable_accept_lpm(hiusbc);
	}
	return ret;
}

static void hiusbc_req_set_sel_cmplt(struct usb_ep *ep,
					struct usb_request *req)
{
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);
	struct hiusbc *hiusbc = hep->hiusbc;
	struct usb_set_sel_req sel;

	if (req->status < 0)
		return;

	if (memcpy_s(&sel, sizeof(sel), req->buf, sizeof(sel)) != EOK)
		return;

	hiusbc->u1_sel = sel.u1_sel;
	hiusbc->u1_pel = sel.u1_pel;
	hiusbc->u2_sel = le16_to_cpu(sel.u2_sel);
	hiusbc->u2_pel = le16_to_cpu(sel.u2_pel);

	hiusbc_set_ux_exit(hiusbc);
}

static int hiusbc_req_set_sel(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup)
{
	enum usb_device_state state = hiusbc->gadget.state;
	struct hiusbc_ep *hep = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Start handling std req: Set SEL:\n");
	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Device State: %d\n", state);
	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "wLength: %u\n", setup->wLength);

	if (state == USB_STATE_DEFAULT || setup->wLength != 6)
		return -EINVAL;

	hep = hiusbc->eps[0];
	hiusbc->dev_req.hep = hep;
	hiusbc->dev_req.epnum = hep->epnum;
	hiusbc->dev_req.req.length = hep->ep.maxpacket;
	hiusbc->dev_req.req.buf = hiusbc->dev_req_response_buf;
	hiusbc->dev_req.req.complete = hiusbc_req_set_sel_cmplt;

	return hiusbc_ep_queue(&hep->ep, &hiusbc->dev_req.req);
}

static int hiusbc_req_set_isoc_delay(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_CTRL,
		"Start handling std req: Set Isoc Delay:\n");
	hiusbc_dbg(HIUSBC_DEBUG_CTRL, "Isoc Delay: %u\n", setup->wValue);

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U3_ISOCH_DELAY_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_DEV_ISOCH_DELAY_MASK;
	reg |= (setup->wValue & MPI_APP_DEVICE_REG_DEV_ISOCH_DELAY_MASK);
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U3_ISOCH_DELAY_OFFSET);

	return 0;
}

int hiusbc_handle_std_req(struct hiusbc *hiusbc,
		const struct usb_ctrlrequest *setup)
{
	int ret;

	switch (setup->bRequest) {
	case USB_REQ_GET_STATUS:
		ret = hiusbc_req_get_status(hiusbc, setup);
		break;
	case USB_REQ_CLEAR_FEATURE:
		ret = hiusbc_req_feature(hiusbc, setup, 0);
		break;
	case USB_REQ_SET_FEATURE:
		ret = hiusbc_req_feature(hiusbc, setup, 1);
		break;
	case USB_REQ_SET_ADDRESS:
		ret = hiusbc_req_set_addr(hiusbc, setup);
		break;
	case USB_REQ_SET_CONFIGURATION:
		ret = hiusbc_req_set_config(hiusbc, setup);
		break;
	case USB_REQ_SET_SEL:
		ret = hiusbc_req_set_sel(hiusbc, setup);
		break;
	case USB_REQ_SET_ISOCH_DELAY:
		ret = hiusbc_req_set_isoc_delay(hiusbc, setup);
		break;
	default:
		hiusbc_dbg(HIUSBC_DEBUG_CTRL,
			"Delegate std req: %u to usbcore.\n", setup->bRequest);
		ret = hiusbc_gadget_delegate_req(hiusbc, setup);
		break;
	}

	return ret;
}
