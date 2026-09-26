/*
 * hiusbc_gadget.c -- Device Mode for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>
#include <linux/dmapool.h>
#include <linux/interrupt.h>
#include <linux/pm_runtime.h>
#include <linux/usb/ch9.h>
#include <linux/usb/gadget.h>
#include <securec.h>
#include "hiusbc_core.h"
#include "hiusbc_debug.h"
#include "hiusbc_event.h"
#include "hiusbc_transfer.h"
#include "hiusbc_gadget.h"

#define HIUSBC_STARTXFER_TRIES	5
#define HIUSBC_CMD_CMPLT_TIMEOUT	2000

int hiusbc_get_frame_id(const struct hiusbc *hiusbc)
{
	return hiusbc_get_bicnt(hiusbc_readl(hiusbc->com_regs,
			MPI_APP_COM_REG_GLOBAL_TIMESTAMP_OFFSET));
}

static void hiusbc_enable_events(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = MPI_APP_DEVICE_REG_DISCONNECT_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_CONNECT_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_USB_RESET_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_SUSPEND_ENTRY_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_U3_RESUME_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_L1_ENTRY_EVENT_EN_MASK |
		MPI_APP_DEVICE_REG_L1_RESUME_EVENT_EN_MASK |
		HIUSBC_DEVT_EN_CMD_DISABLE_EP |
		HIUSBC_DEVT_EN_CMD_END_XFER |
		HIUSBC_DEVT_EN_CMD_SET_HALT |
		MPI_APP_DEVICE_REG_VENDOR_TEST_EVENT_EN_MASK;

	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_EVENT_EN_OFFSET);
}

static void hiusbc_disable_events(struct hiusbc *hiusbc)
{
	int reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc_writel(0, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_EVENT_EN_OFFSET);

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_DEV_EWE_MASK;
	hiusbc_writel(reg, &hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CFG_OFFSET);
}

static void hiusbc_event_buffer_init(struct hiusbc_evt_ring *event_ring)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	unsigned int index = event_ring->int_idx;
	u32 reg;
	struct hiusbc_evt_trb *temp_trb = NULL;
	dma_addr_t dma_deq;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	hiusbc_dbg(HIUSBC_DEBUG_EVENT,
		"Clear all the CCS to 0 of event buffer.\n");
	for (reg = 0, temp_trb = &event_ring->trbs->event;
			reg < event_ring->size;
			reg++, temp_trb++)
		temp_trb->info &= ~HIUSBC_TRB_CYCLE;

	event_ring->dequeue = event_ring->trbs;
	event_ring->ccs = 1;

	dma_deq = hiusbc_evt_trb_vrt_to_dma(event_ring, event_ring->dequeue);

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Set EBSZ = %u\n",  event_ring->size);
	hiusbc_writel(event_ring->size,
		hiusbc->dev_regs, hiusbc_dev_ebsz_offset(index));

	/* must write high 32bit after low 32bit. */
	reg = lower_32_bits(dma_deq);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Set EBDP0 = 0x%x\n",  reg);
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_ebdp0_offset(index));
	reg = upper_32_bits(dma_deq);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Set EBDP1 = 0x%x\n",  reg);
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_ebdp1_offset(index));

	reg = lower_32_bits(event_ring->dma);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Set EBBA0 = 0x%x\n",  reg);
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_ebba0_offset(index));
	reg = upper_32_bits(event_ring->dma);
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Set EBBA1 = 0x%x\n",  reg);
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_ebba1_offset(index));

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "Read EB Registers:\n");

	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebdp0_offset(index));
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "New EBDP0 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebdp1_offset(index));
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "New EBDP1 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebba0_offset(index));
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "New EBBA0 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebba1_offset(index));
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "New EBBA1 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebsz_offset(index));
	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "New EBSZ = %u\n",  reg);
}

static void hiusbc_event_buffer_exit(struct hiusbc_evt_ring *event_ring)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	unsigned int index = event_ring->int_idx;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc_writel(0, hiusbc->dev_regs, hiusbc_dev_ebdp0_offset(index));
	hiusbc_writel(0, hiusbc->dev_regs, hiusbc_dev_ebdp1_offset(index));

	hiusbc_writel(0, hiusbc->dev_regs, hiusbc_dev_ebba0_offset(index));
	hiusbc_writel(0, hiusbc->dev_regs, hiusbc_dev_ebba1_offset(index));

	hiusbc_writel(0, hiusbc->dev_regs, hiusbc_dev_ebsz_offset(index));
}

static void hiusbc_enable_intr(struct hiusbc_evt_ring *event_ring)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	unsigned int index = event_ring->int_idx;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_iman_offset(index));
	reg |= MPI_APP_DEVICE_REG_DEV_INT_ENABLE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_iman_offset(index));

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static void hiusbc_disable_intr(struct hiusbc_evt_ring *event_ring)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	unsigned int index = event_ring->int_idx;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_iman_offset(index));
	reg &= ~MPI_APP_DEVICE_REG_DEV_INT_ENABLE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_iman_offset(index));

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static void hiusbc_enable_sys_err_intr(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	reg |= MPI_APP_DEVICE_REG_DSEE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static void hiusbc_disable_sys_err_intr(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	reg &= ~MPI_APP_DEVICE_REG_DSEE_MASK;
	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

void hiusbc_set_ux_exit(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	u32 reg;
	u32 ux_exit_interval = 0;
	u8 epnum;

	if (!hiusbc->usb3_dev_lpm_ux_exit)
		return;

	if (hiusbc->gadget.speed < USB_SPEED_SUPER)
		return;

	for (epnum = 0; epnum < HIUSBC_EP_NUMS; epnum++) {
		hep = hiusbc->eps[epnum];
		if (hep->si && hep->enabled) {
			if ((!ux_exit_interval) ||
					(hep->si < ux_exit_interval))
				ux_exit_interval = hep->si;
		}
	}

	if (!ux_exit_interval) {
		hiusbc_writel(0, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_USB3_U1EL_OFFSET);
		hiusbc_writel(0, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_USB3_U2EL_OFFSET);
	} else {
		if (ux_exit_interval > 128)
			ux_exit_interval = 128;

		hiusbc_dbg(HIUSBC_DEBUG_SYS,
			"Set Ux Exit Interval: %u BI.\n", ux_exit_interval);

		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_U3_UX_EXIT_CFG_OFFSET);
		reg &= ~MPI_APP_DEVICE_REG_DEV_UX_EXIT_INTERVAL_MASK;
		reg |= ((ux_exit_interval
			<< MPI_APP_DEVICE_REG_DEV_UX_EXIT_INTERVAL_SHIFT) &
				MPI_APP_DEVICE_REG_DEV_UX_EXIT_INTERVAL_MASK);
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_U3_UX_EXIT_CFG_OFFSET);

		hiusbc_dbg(HIUSBC_DEBUG_SYS,
			"Set SEL params: U1_SEL=%uus, U1_PEL=%uus, "
			"U2_SEL=%uus, U2_PEL=%uus.\n",
			hiusbc->u1_sel, hiusbc->u1_pel,
			hiusbc->u2_sel, hiusbc->u2_pel);

		hiusbc_dbg(HIUSBC_DEBUG_SYS,
			"LPM State now: U1 is %s, U2 is %s.",
			hiusbc->u1_enable ? "Enabled" : "Disabled",
			hiusbc->u2_enable ? "Enabled" : "Disabled");

		reg = (((u32)hiusbc->u1_pel <<
			MPI_APP_DEVICE_REG_DEV_USB3_U1PEL_SHIFT) |
			hiusbc->u1_sel);
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_USB3_U1EL_OFFSET);

		reg = (((u32)hiusbc->u2_pel <<
			MPI_APP_DEVICE_REG_DEV_USB3_U2PEL_SHIFT) |
			hiusbc->u2_sel);
		hiusbc_writel(reg, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_USB3_U2EL_OFFSET);
	}
}

enum usb_device_speed hiusbc_get_link_speed(
			const struct hiusbc *hiusbc)
{
	u32 u2_portsc;
	u32 u3_portsc;
	bool u2_ccs = false;
	bool u3_ccs = false;
	enum usb_device_speed ret = USB_SPEED_UNKNOWN;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	u2_portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2_PORTSC_OFFSET);
	u3_portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U3_PORTSC_OFFSET);

	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"Confirm u2_portsc = 0x%x, u3_portsc = 0x%x\n",
		u2_portsc, u3_portsc);

	u2_ccs = !!(u2_portsc & MPI_APP_DEVICE_REG_DEV_U2_PORT_SPEED_MASK);
	u3_ccs = !!(u3_portsc & MPI_APP_DEVICE_REG_DEV_U3_PORT_SPEED_MASK);

	if (!(u2_ccs ^ u3_ccs)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"WTF! U2 and U3 both %s?\n",
			u2_ccs ? "connected" : "disconnected");
		return ret;
	}

	if (u2_ccs) {
		switch (hiusbc_get_u2portsc_speed(u2_portsc)) {
		case HIUSBC_PORTSC_SPEED_LOW:
			ret = USB_SPEED_LOW;
			break;
		case HIUSBC_PORTSC_SPEED_FULL:
			ret = USB_SPEED_FULL;
			break;
		case HIUSBC_PORTSC_SPEED_HIGH:
			ret = USB_SPEED_HIGH;
			break;
		default:
			break;
		}
	} else {
		switch (hiusbc_get_u3portsc_speed(u3_portsc)) {
		case HIUSBC_PORTSC_SPEED_SUPER_PLUS:
			ret = USB_SPEED_SUPER_PLUS;
			break;
		case HIUSBC_PORTSC_SPEED_SUPER:
			ret = USB_SPEED_SUPER;
			break;
		default:
			break;
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-: link speed = %d\n", ret);

	return ret;
}

enum hiusbc_link_state hiusbc_get_link_state(
			const struct hiusbc *hiusbc,
			enum usb_device_speed speed)
{
	u32 portsc;
	enum hiusbc_link_state ret = HIUSBC_LINK_STATE_UNKNOWN;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	switch (speed) {
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
		portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2_PORTSC_OFFSET);
		ret = hiusbc_get_u2portsc_pls(portsc);
		break;

	case USB_SPEED_SUPER:
	case USB_SPEED_SUPER_PLUS:
		portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U3_PORTSC_OFFSET);
		ret = hiusbc_get_u3portsc_pls(portsc);
		break;

	default:
		break;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-: link state = %d\n", ret);

	return ret;
}

void hiusbc_set_link_state(const struct hiusbc *hiusbc,
		enum usb_device_speed speed, enum hiusbc_link_state state)
{
	u32 portsc;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	switch (speed) {
	case USB_SPEED_FULL:
	case USB_SPEED_HIGH:
		portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U2_PORTSC_OFFSET);
		portsc &= ~MPI_APP_DEVICE_REG_DEV_U2_PLS_MASK;
		portsc |= MPI_APP_DEVICE_REG_DEV_U2_LWS_MASK |
			hiusbc_u2portsc_pls(state);
		hiusbc_writel(portsc, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_U2_PORTSC_OFFSET);
		break;

	case USB_SPEED_SUPER:
	case USB_SPEED_SUPER_PLUS:
		portsc = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_U3_PORTSC_OFFSET);
		portsc &= ~MPI_APP_DEVICE_REG_DEV_U3_PLS_MASK;
		portsc |= MPI_APP_DEVICE_REG_DEV_U3_LWS_MASK |
			hiusbc_u3portsc_pls(state);
		hiusbc_writel(portsc, hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_U3_PORTSC_OFFSET);
		break;

	default:
		break;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-: link state = %d\n", state);
}

static void hiusbc_set_imodi(struct hiusbc_evt_ring *event_ring,
					u16 imodi)
{
	struct hiusbc *hiusbc = event_ring->hiusbc;
	u8 index = event_ring->int_idx;
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_imod_offset(index));
	reg &= ~MPI_APP_DEVICE_REG_DEV_INT_MOD_INTERVAL_MASK;
	reg |= hiusbc_imodi(imodi);
	hiusbc_writel(reg, hiusbc->dev_regs, hiusbc_dev_imod_offset(index));
}

/* send cmd */
static int hiusbc_send_cmd(struct hiusbc *hiusbc,
		const struct hiusbc_cmd *parm)
{
	u32 reg;
	u32 timeout = 1000;
	int ret = -EINVAL;

	hiusbc_writel(parm->word0, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CMD_WORD0_OFFSET);
	hiusbc_writel(parm->word1, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CMD_WORD1_OFFSET);
	hiusbc_writel(parm->word2, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CMD_WORD2_OFFSET);
	hiusbc_writel(parm->word3, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CMD_WORD3_OFFSET);


	hiusbc_writel(MPI_APP_DEVICE_REG_DEV_CMD_VLD_MASK,
		hiusbc->dev_regs, MPI_APP_DEVICE_REG_DEV_CMD_CTRL_OFFSET);

	do {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_CMD_CTRL_OFFSET);
		if (reg & MPI_APP_DEVICE_REG_DEV_CMD_DONE_MASK) {
			switch (hiusbc_get_cmd_done_info(reg)) {
			case HIUSBC_CMD_FINISH:
				ret = 0;
				break;
			case HIUSBC_CMD_PARM_ERROR:
				pr_err("Cmd done info: Params Err.\n");
				ret = -EINVAL;
				break;
			case HIUSBC_CMD_ABORT:
				pr_err("Cmd done info: Aborted.\n");
				ret = -EAGAIN;
				break;
			case HIUSBC_CMD_FAIL:
				pr_err("Cmd done info: Failed.\n");
				ret = -EAGAIN;
				break;
			case HIUSBC_CMD_FRM_EXPIRED:
				pr_err("Cmd done info: FrameID expired.\n");
				ret = -EINVAL;
				break;
			case HIUSBC_CMD_EVT_LOSS:
				pr_err("Cmd done info: Event Loss.\n");
				ret = -EINVAL;
				break;
			default:
				pr_err("Unknown: done_info = 0x%x\n",
					hiusbc_get_cmd_done_info(reg));
				break;
			}
			break;
		}
	} while (--timeout);

	if (timeout == 0) {
		pr_err("Cmd timeout!\n");
		ret = -ETIMEDOUT;
	}

	return ret;
}

int hiusbc_send_cmd_set_addr(struct hiusbc *hiusbc, u8 addr)
{
	struct hiusbc_cmd parm = {0};

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a Set Address cmd, addr=%u.\n", addr);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_SET_ADDR);
	parm.word3 |= hiusbc_cmd_device_address(addr);

	return hiusbc_send_cmd(hiusbc, &parm);
}

static int hiusbc_send_cmd_enable_ep(struct hiusbc_ep *hep)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};
	const struct usb_endpoint_descriptor *ep_desc = hep->ep.desc;
	u8 interval = ep_desc->bInterval;

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a Enable EP cmd, EPNum = %u.\n", hep->epnum);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_ENABLE_EP) |
				hiusbc_cmd_ep_number(hep->epnum) |
				hiusbc_cmd_ep_type(usb_endpoint_type(ep_desc));

	if (usb_endpoint_xfer_int(ep_desc) ||
		usb_endpoint_xfer_isoc(ep_desc)) {
		if (interval) {
			if (usb_endpoint_xfer_int(ep_desc) &&
				(hiusbc->gadget.speed == USB_SPEED_FULL))
				interval = fls(interval);

			interval -= 1;
			if (hiusbc->gadget.speed == USB_SPEED_FULL)
				interval += 3;

			parm.word2 |= hiusbc_cmd_interval(interval);
			hep->si = 1 << interval;
		} else {
			pr_err("bInterval for EP%u is 0? should be 1~16.\n",
				hep->epnum);
			hep->si = 1;
		}
	}

	if (hiusbc->gadget.speed >= USB_SPEED_SUPER)
		parm.word2 |= hiusbc_cmd_max_burst_size(hep->ep.maxburst - 1);

#ifdef CONFIG_HIUSBC_EDA_TEST_CASE
	if (hep->epnum > 1) {
		parm.word2 |= hiusbc_cmd_max_packet_size(
			usb_endpoint_maxp(ep_desc) / hiusbc->force_maxp);
		pr_err("EDA_TEST: force maxp to %d\n",
			usb_endpoint_maxp(ep_desc) / hiusbc->force_maxp);
	} else {
		parm.word2 |= hiusbc_cmd_max_packet_size(
			usb_endpoint_maxp(ep_desc));
	}
#else
	parm.word2 |= hiusbc_cmd_max_packet_size(usb_endpoint_maxp(ep_desc));
#endif
	return hiusbc_send_cmd(hiusbc, &parm);
}

static int hiusbc_send_cmd_disable_ep(struct hiusbc_ep *hep)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a Disable EP cmd, EPNum = %u.\n", hep->epnum);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_DISABLE_EP);
	parm.word3 |= hiusbc_cmd_ep_number(hep->epnum);

	return hiusbc_send_cmd(hiusbc, &parm);
}

static int hiusbc_send_cmd_set_halt(struct hiusbc_ep *hep)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a Set Halt cmd, EPNum = %u.\n", hep->epnum);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_SET_HALT);
	parm.word3 |= hiusbc_cmd_ep_number(hep->epnum);

	return hiusbc_send_cmd(hiusbc, &parm);
}

int hiusbc_send_cmd_clear_halt(struct hiusbc_ep *hep)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a Clear Halt cmd, EPNum = %u.\n", hep->epnum);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_CLEAR_HALT);
	parm.word3 |= hiusbc_cmd_ep_number(hep->epnum);

	return hiusbc_send_cmd(hiusbc, &parm);
}

int hiusbc_send_cmd_start_transfer(struct hiusbc_ep *hep,
	u32 start_mf, dma_addr_t buffer_ptr, u8 dcs, u8 init, u8 ctrl_num)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};
	int retries;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_CMD, "+\n");

	if (!hep->enabled) {
		dev_err(hiusbc->dev,
			"EP%u is not enabled, can't send StartXfer cmd!\n",
			hep->epnum);
		return -EINVAL;
	}

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"Queuing a StartXfer cmd, EPNum = %u, "
		"TRB dma = 0x%llx, dcs = %u, init = %u.\n",
		hep->epnum, buffer_ptr, dcs, init);

	hiusbc_dbg(HIUSBC_DEBUG_CMD,
		"  and, cur_mf = %d, start_mf = %u, ctrl_num = %u.\n",
		hiusbc_get_frame_id(hiusbc), start_mf, ctrl_num);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_START_TRANSFER) |
					hiusbc_cmd_ep_number(hep->epnum) |
					hiusbc_cmd_dcs(dcs) |
					hiusbc_cmd_init(init) |
					hiusbc_ctrl_num(ctrl_num);

	parm.word2 = upper_32_bits(buffer_ptr);
	parm.word1 = lower_32_bits(buffer_ptr);

	parm.word0 = start_mf;

	for (retries = 0; retries < HIUSBC_STARTXFER_TRIES; retries++) {
		ret = hiusbc_send_cmd(hiusbc, &parm);
		if (ret != -EAGAIN)
			break;
		mdelay(1);
	}

	hiusbc_dbg(HIUSBC_DEBUG_CMD, "-\n");

	return ret;
}

int hiusbc_send_cmd_end_transfer(struct hiusbc_ep *hep,
					bool wait_event)
{
	struct hiusbc *hiusbc = hep->hiusbc;
	struct hiusbc_cmd parm = {0};
	int ret;
	long time_remain;
	unsigned long timeout;

	pr_err("Queuing a EndXfer cmd, EPNum = %u.\n", hep->epnum);

	parm.word3 |= hiusbc_cmd_type(HIUSBC_CMD_TYPE_END_TRANSFER);
	parm.word3 |= hiusbc_cmd_ep_number(hep->epnum);

	ret = hiusbc_send_cmd(hiusbc, &parm);
	if (!ret) {
		hep->xfer_state = HIUSBC_XFER_END;
	} else {
		pr_err("Failed to send EndXfer to EP%u\n", hep->epnum);
		return ret;
	}

	if (!wait_event) {
		udelay(100);
		hep->xfer_state = HIUSBC_XFER_IDLE;
		hep->nrdy_received = false;
		hep->nrdy_mf = 0;
		return ret;
	}

	hep->end_xfer_pending = true;

	timeout = msecs_to_jiffies(HIUSBC_CMD_CMPLT_TIMEOUT);
	time_remain = wait_event_interruptible_lock_irq_timeout(
		hep->wait_end_xfer_cmd,
		(hep->xfer_state != HIUSBC_XFER_END), hiusbc->lock,
		msecs_to_jiffies(HIUSBC_CMD_CMPLT_TIMEOUT));

	if (time_remain == 0) {
		pr_err("Wait endxfer cmplt event timeout!\n");
		ret = -ETIMEDOUT;
	} else if (time_remain == -ERESTARTSYS) {
		pr_err("Wait endxfer cmplt event was interrupted!\n");
		ret = -ETIMEDOUT;
	}

	return ret;
}

static int hiusbc_ep_enable(struct hiusbc_ep *hep)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_EP, "+: EP = %u\n", hep->epnum);

	if (hep->enabled) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EP%u is already enabled.\n", hep->epnum);
		return 0;
	}

	ret = hiusbc_send_cmd_enable_ep(hep);
	if (ret)
		return ret;

	init_waitqueue_head(&hep->wait_end_xfer_cmd);
	init_waitqueue_head(&hep->wait_disable_ep_cmd);

	hep->enabled = true;
	hep->wedged = false;
	hep->stalled = false;
	hep->end_xfer_pending = false;
	hep->disable_pending = false;
	hep->xfer_state = HIUSBC_XFER_IDLE;
	hep->flow_control = false;
	hiusbc_clear_xfer_event_counter(hep);

	hiusbc_dbg(HIUSBC_DEBUG_EP, "-: ret = %d\n", ret);

	return ret;
}

/*
 * Needn't EndXfer before Disable EP.
 * And must giveback request after Disable EP cmd done.
 */
static int hiusbc_ep_disable(struct hiusbc_ep *hep, bool wait_event)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_EP, "+: EP = %u\n", hep->epnum);

	if (!hep->enabled) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EP%u is already disabled.\n", hep->epnum);
		return 0;
	}

	ret = hiusbc_send_cmd_disable_ep(hep);
	if (!ret) {
		if (wait_event) {
			hep->disable_pending = true;
		} else {
			udelay(100);
			hiusbc_cleanup_exist_req_on_ep(hep, -ESHUTDOWN);
			hep->enabled = false;
			hep->wedged = false;
			hep->stalled = false;
			hep->xfer_state = HIUSBC_XFER_IDLE;
			hep->flow_control = false;
			hep->nrdy_received = false;
			hep->nrdy_mf = 0;
			hep->si = 0;

			if (hep->epnum > 1) {
				hep->ep.desc = NULL;
				hep->ep.comp_desc = NULL;
			}
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_EP, "-: ret = %d\n", ret);

	return ret;
}

static int hiusbc_gadget_ep_enable(struct usb_ep *ep,
			const struct usb_endpoint_descriptor *desc)
{
	struct hiusbc_ep *hep = NULL;
	struct hiusbc *hiusbc = NULL;
	int ret;
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	if (!ep || !desc || desc->bDescriptorType != USB_DT_ENDPOINT) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "ep or desc null!\n");
		return -EINVAL;
	}

	if (!desc->wMaxPacketSize) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "desc->wMaxPacketSize null!\n");
		return -EINVAL;
	}

	hep = to_hiusbc_ep(ep);
	hiusbc = hep->hiusbc;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "  EPNum = %u\n", hep->epnum);

	/* Ctrl EP always enable. */
	if (hep->epnum <= 1) {
		dev_err(hiusbc->dev,
			"Ctrl EP always enable!\n");
		return -EINVAL;
	}

	if (usb_endpoint_num(desc) != (hep->epnum >> 1)) {
		dev_err(hiusbc->dev, "EPNum mismatch!\n");
		return -EINVAL;
	}

	if (!!usb_endpoint_dir_in(desc) ^ !!hep->is_in) {
		dev_err(hiusbc->dev, "EP dir mismatch!\n");
		return -EINVAL;
	}

	if (hep->enabled) {
		dev_err(hiusbc->dev,
			"EP = %u is already enabled!\n", hep->epnum);
		return 0;
	}

	spin_lock_irqsave(&hiusbc->lock, flags);
	ret = hiusbc_ep_enable(hep);
	if (ret) {
		dev_err(hiusbc->dev, "Failed to enable ep %u\n", hep->epnum);
		goto error;
	}

	if (usb_endpoint_xfer_int(desc) || usb_endpoint_xfer_isoc(desc))
		hiusbc_set_ux_exit(hiusbc);

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "- EPNum = %u\n", hep->epnum);

	return ret;
}

static int hiusbc_gadget_ep_disable(struct usb_ep *ep)
{
	struct hiusbc_ep *hep = NULL;
	struct hiusbc *hiusbc = NULL;
	int ret = 0;
	unsigned long flags;
	unsigned long time_remain;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	if (!ep) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "+ ep null!\n");
		return -EINVAL;
	}

	hep = to_hiusbc_ep(ep);
	hiusbc = hep->hiusbc;

	spin_lock_irqsave(&hiusbc->lock, flags);

	/* Ctrl EP always enable. */
	if (hep->epnum <= 1) {
		dev_err(hiusbc->dev, "Ctrl EP always enable!\n");
		ret = -EINVAL;
		goto error;
	}

	if (!hep->enabled) {
		dev_err(hiusbc->dev,
			"EP = %u is already disabled!\n", hep->epnum);
		ret = 0;
		goto error;
	}

	ret = hiusbc_ep_disable(hep, false);
	if (ret) {
		dev_err(hiusbc->dev,
			"Failed to disable ep = %u.\n", hep->epnum);
		goto error;
	}

	hiusbc_set_ux_exit(hiusbc);

	if (hep->disable_pending) {
		hiusbc_dbg(HIUSBC_DEBUG_CMD,
			"Start to wait disable ep cmd cmplt event.\n");
		time_remain = wait_event_interruptible_lock_irq_timeout(
			hep->wait_disable_ep_cmd,
			(hep->disable_pending == false), hiusbc->lock,
			msecs_to_jiffies(HIUSBC_CMD_CMPLT_TIMEOUT));

		if (time_remain == 0 || time_remain == -ERESTARTSYS) {
			hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"Wait disable ep cmplt event err %d!\n",
				time_remain);
			ret = -ETIMEDOUT;
		}
		hiusbc_dbg(HIUSBC_DEBUG_CMD,
			"time_remain = %u.\n", time_remain);
	}

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "- EPNum = %u\n", hep->epnum);

	return ret;
}

struct usb_request *hiusbc_gadget_ep_alloc_request(
				struct usb_ep *ep, gfp_t gfp_flags)
{
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_ep *hep = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "+\n");

	if (!ep) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "ep NULL!");
		return NULL;
	}

	hreq = kzalloc(sizeof(struct hiusbc_req), gfp_flags);
	if (!hreq)
		return NULL;

	INIT_LIST_HEAD(&hreq->queue);

	hep = to_hiusbc_ep(ep);
	hreq->epnum = hep->epnum;
	hreq->hep = hep;
	hreq->req.dma = DMA_ADDR_INVALID;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP,
		"Alloc a usb request = 0x%p, for EP%u.\n",
		&hreq->req, hep->epnum);

	return &hreq->req;
}

void hiusbc_gadget_ep_free_request(
			struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_req *hreq = NULL;
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);

	hiusbc_dbg(HIUSBC_DEBUG_XFER, "Free a usb request = 0x%p, for EP%u.\n",
		req, hep->epnum);

	hreq = to_hiusbc_req(req);
	kfree(hreq);
}

static int hiusbc_gadget_ep_queue(
		struct usb_ep *ep, struct usb_request *req, gfp_t gfp_flags)
{
	struct hiusbc_ep *hep = NULL;
	struct hiusbc *hiusbc = NULL;

	unsigned long flags;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	if (!ep || !req) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Target ep or req not allocated!\n");
		return -EINVAL;
	}

	hep = to_hiusbc_ep(ep);
	hiusbc = hep->hiusbc;

	spin_lock_irqsave(&hiusbc->lock, flags);
	ret = hiusbc_ep_queue(ep, req);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");

	return ret;
}

static int hiusbc_gadget_ep_dequeue(
			struct usb_ep *ep, struct usb_request *req)
{
	struct hiusbc_ep *hep = NULL;
	struct hiusbc *hiusbc = NULL;

	unsigned long flags;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	if (!ep) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "Target ep not allocated!\n");
		return -EINVAL;
	}

	hep = to_hiusbc_ep(ep);
	hiusbc = hep->hiusbc;

	spin_lock_irqsave(&hiusbc->lock, flags);
	ret = hiusbc_ep_dequeue(ep, req);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	hiusbc_dbg(HIUSBC_DEBUG_ERR, "-\n");
	return ret;
}

static int hiusbc_handle_set_ep_stall(struct hiusbc_ep *hep)
{
	int ret;

	if (hep->stalled)
		return 0;

#ifndef CONFIG_HIUSBC_EDA_TEST_CASE
	if (!list_empty(&hep->ring->queue)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"EP%u xfer ring is not empty, can't set halt!\n",
			hep->epnum);
		return -EAGAIN;
	}
#endif

	ret = hiusbc_send_cmd_set_halt(hep);
	if (ret)
		return ret;

	hep->set_halt_pending = true;
	return ret;
}

static int hiusbc_handle_clear_ep_stall(struct hiusbc_ep *hep)
{
	int ret;

	if (!hep->stalled && !hep->set_halt_pending) {
		ret = hiusbc_send_cmd_set_halt(hep);
		if (!ret)
			hep->set_halt_pending = true;
	}

	ret = hiusbc_send_cmd_clear_halt(hep);
	if (!ret) {
		hep->stalled = false;

		if (hep->set_halt_pending) {
			hep->set_halt_pending = false;
			hep->xfer_state = HIUSBC_XFER_IDLE;
		}

		if (!list_empty(&hep->queue))
			hiusbc_queue_req_bulk_int(hep, GFP_ATOMIC);

#ifdef CONFIG_HIUSBC_EDA_TEST_CASE
		pr_err("EDA TEST: Send StartXfer after Clear Stall to EP%u!\n",
			hep->epnum);
		ret = hiusbc_send_cmd_start_transfer(hep, 0, 0, 0, 0, 0);

		if (ret < 0) {
			pr_err("Failed to send StartXfer! ret = %d\n", ret);
			hiusbc_cleanup_exist_req_on_ep(hep, -EAGAIN);
			return ret;
		}
#endif
	}
	return ret;
}

int hiusbc_set_ep_stall(struct hiusbc_ep *hep, bool set)
{
	hiusbc_dbg(HIUSBC_DEBUG_EP, "+\n");

	if (usb_endpoint_xfer_isoc(hep->ep.desc)) {
		pr_err("Can't %s Halt to Isoc EP%u.\n",
			set ? "Set" : "Clear", hep->epnum);
		return -EINVAL;
	}

	if (hep->epnum <= 1)
		return 0;

	if (set)
		return hiusbc_handle_set_ep_stall(hep);

	return hiusbc_handle_clear_ep_stall(hep);
}

static int hiusbc_gadget_ep_set_halt(struct usb_ep *ep, int value)
{
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);
	struct hiusbc *hiusbc = hep->hiusbc;

	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);
	if (value) {
		hiusbc_set_ep_stall(hep, 1);
	} else {
		hiusbc_set_ep_stall(hep, 0);
		if (hep->epnum > 1) {
			hep->wedged = 0;
		} else {
			hiusbc->eps[0]->wedged = 0;
			hiusbc->eps[1]->wedged = 0;
		}
	}
	spin_unlock_irqrestore(&hiusbc->lock, flags);
	return 0;
}

static int hiusbc_gadget_ep_set_wedge(struct usb_ep *ep)
{
	struct hiusbc_ep *hep = to_hiusbc_ep(ep);
	struct hiusbc *hiusbc = hep->hiusbc;
	unsigned long flags;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hep->epnum > 1) {
		hep->wedged = 1;
	} else {
		hiusbc->eps[0]->wedged = 1;
		hiusbc->eps[1]->wedged = 1;
	}

	ret = hiusbc_set_ep_stall(hep, 1);

	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return ret;
}

static const struct usb_ep_ops hiusbc_gadget_ep_ops = {
	.enable		= hiusbc_gadget_ep_enable,
	.disable	= hiusbc_gadget_ep_disable,
	.alloc_request	= hiusbc_gadget_ep_alloc_request,
	.free_request	= hiusbc_gadget_ep_free_request,
	.queue		= hiusbc_gadget_ep_queue,
	.dequeue	= hiusbc_gadget_ep_dequeue,
	.set_halt	= hiusbc_gadget_ep_set_halt,
	.set_wedge	= hiusbc_gadget_ep_set_wedge,
};

static int hiusbc_gadget_get_frame(struct usb_gadget *g)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);
	ret = hiusbc_get_frame_id(hiusbc);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return ret;
}

int hiusbc_wakeup(const struct hiusbc *hiusbc)
{
	enum usb_device_speed speed;
	enum hiusbc_link_state state;
	int retries = 1000;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	speed = hiusbc_get_link_speed(hiusbc);

	if (speed == USB_SPEED_HIGH || speed == USB_SPEED_FULL) {
		if (!hiusbc->remote_wakeup)
			return -EOPNOTSUPP;

		state = hiusbc_get_link_state(hiusbc, speed);
		if ((state == HIUSBC_LINK_STATE_L1) ||
			(state == HIUSBC_LINK_STATE_L2)) {
			while (retries--) {
				hiusbc_set_link_state(
					hiusbc, speed, HIUSBC_LINK_STATE_L0);
				if (hiusbc_get_link_state(
					hiusbc, speed) == HIUSBC_LINK_STATE_L0)
					break;
				mdelay(5);
			}
		}

		if (hiusbc_get_link_state(
			hiusbc, speed) != HIUSBC_LINK_STATE_L0) {
			hiusbc_dbg(HIUSBC_DEBUG_ERR,
				"Failed to send remote wakeup\n");
			ret = -EINVAL;
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
	return ret;
}

static int hiusbc_gadget_wakeup(struct usb_gadget *g)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_DEVICE) {
		ret = -EINVAL;
		dev_err(hiusbc->dev,
			"it's not in device mode now or failed!\n");
		goto error;
	}

	ret = hiusbc_wakeup(hiusbc);

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);
	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-: ret = %d\n", ret);
	return ret;
}

static int hiusbc_gadget_set_selfpowered(struct usb_gadget *g,
		int is_selfpowered)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "is_selfpowered = %d", is_selfpowered);

	spin_lock_irqsave(&hiusbc->lock, flags);
	g->is_selfpowered = (is_selfpowered != 0);
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return 0;
}

static int hiusbc_gadget_pullup(struct usb_gadget *g, int is_on)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "is_on = %d\n", is_on);

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_DEVICE) {
		ret = -EINVAL;
		dev_err(hiusbc->dev,
			"it's not in device mode now or failed!\n");
		goto error;
	}

	ret = hiusbc_run_stop(hiusbc, is_on);

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return ret;
}

int hiusbc_enable_ctrl_eps(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	int ret;
	const struct usb_endpoint_descriptor *ep_desc = hiusbc->ep0_desc;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (!hiusbc->eps[0] || !hiusbc->eps[1] ||
		!ep_desc || ep_desc->bDescriptorType != USB_DT_ENDPOINT) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR, "Ctrl EP or desc null!\n");
		return -EINVAL;
	}

	if (!ep_desc->wMaxPacketSize) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Ctrl EP desc->wMaxPacketSize null!\n");
		return -EINVAL;
	}

	hep = hiusbc->eps[0];
	ret = hiusbc_ep_enable(hep);
	if (ret) {
		dev_err(hiusbc->dev, "Failed to enable ep %u\n", hep->epnum);
		return ret;
	}

	hep = hiusbc->eps[1];
	ret = hiusbc_ep_enable(hep);
	if (ret) {
		hiusbc_ep_disable(hiusbc->eps[0], false);
		dev_err(hiusbc->dev, "Failed to enable ep %u\n", hep->epnum);
		return ret;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

int hiusbc_disable_ctrl_eps(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hep = hiusbc->eps[0];
	ret = hiusbc_ep_disable(hep, false);
	if (ret) {
		dev_err(hiusbc->dev,
			"Failed to disable ep = %u.\n", hep->epnum);
		return ret;
	}

	hep = hiusbc->eps[1];
	ret = hiusbc_ep_disable(hep, false);
	if (ret) {
		dev_err(hiusbc->dev,
			"Failed to disable ep = %u.\n", hep->epnum);
		return ret;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

int hiusbc_run_stop(struct hiusbc *hiusbc, int is_on)
{
	u32 reg;
	u32 timeout = 500;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "is_on = %d\n", is_on);

	if (pm_runtime_suspended(hiusbc->dev))
		return 0;

	reg = hiusbc_readl(hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);
	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"Original value of R/S is = %d\n",
		!!(reg & MPI_APP_DEVICE_REG_DEV_RUN_STOP_MASK));


	if (is_on && !(reg & MPI_APP_DEVICE_REG_DEV_RUN_STOP_MASK)) {
		/* We need to reset and reinitial hw before RS 0 to 1. */
		ret = hiusbc_restart(hiusbc);
		if (ret)
			return ret;
	}

	if (is_on)
		reg |= MPI_APP_DEVICE_REG_DEV_RUN_STOP_MASK;
	else
		reg &= ~MPI_APP_DEVICE_REG_DEV_RUN_STOP_MASK;

	hiusbc_writel(reg, hiusbc->dev_regs,
		MPI_APP_DEVICE_REG_DEV_CTL_OFFSET);

	do {
		reg = hiusbc_readl(hiusbc->dev_regs,
			MPI_APP_DEVICE_REG_DEV_STATUS_OFFSET);
		reg &= MPI_APP_DEVICE_REG_DCHALTED_MASK;
	} while (--timeout && !(!is_on ^ !reg));

	if (!timeout) {
		dev_err(hiusbc->dev, "Timeout to set R/S.\n");
		return -ETIMEDOUT;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;
}

static int hiusbc_start(struct hiusbc *hiusbc)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc->ep0_changed_by_gadget_api = true;
	hiusbc->ep0_desc->wMaxPacketSize = cpu_to_le16(512);
	ret = hiusbc_enable_ctrl_eps(hiusbc);
	if (ret)
		return ret;

	hiusbc->cur_stage = HIUSBC_SETUP_STAGE;
	hiusbc->next_stage = HIUSBC_SETUP_STAGE;

	hiusbc_enable_events(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
	return ret;
}

int hiusbc_restart(struct hiusbc *hiusbc)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	ret = hiusbc_reset_controller(hiusbc);
	if (ret)
		return ret;

	ret = hiusbc_reinit_interrupt(hiusbc);
	if (ret)
		return ret;

	ret = hiusbc_start(hiusbc);
	if (ret) {
		dev_err(hiusbc->dev, "Fail to start ctrl ep during\n");
		return ret;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}


static int hiusbc_gadget_start(struct usb_gadget *g,
		struct usb_gadget_driver *driver)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_DEVICE) {
		ret = -EINVAL;
		dev_err(hiusbc->dev,
			"it's not in device mode now or failed\n");
		goto exit;
	}

	if (hiusbc->gadget_driver) {
		ret = -EBUSY;
		dev_err(hiusbc->dev, "hiusbc: %s is already bound to %s\n",
				hiusbc->gadget.name,
				hiusbc->gadget_driver->driver.name);
		goto exit;
	}

	hiusbc->gadget_driver = driver;

	if (pm_runtime_active(hiusbc->dev))
		hiusbc_start(hiusbc);
	else
		dev_err(hiusbc->dev,
			"Don't start, because pm state not active.\n");

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");

exit:
	spin_unlock_irqrestore(&hiusbc->lock, flags);
	return ret;
}

void hiusbc_stop(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");
	hiusbc_disable_events(hiusbc);
	hiusbc_disable_ctrl_eps(hiusbc);
}

static int hiusbc_gadget_stop(struct usb_gadget *g)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_DEVICE) {
		ret = -EINVAL;
		dev_err(hiusbc->dev,
			"it's not in device mode now or failed\n");
		goto exit;
	}

	if (pm_runtime_suspended(hiusbc->dev)) {
		hiusbc_dbg(HIUSBC_DEBUG_ERR,
			"Skip stop, pm state is suspend.\n");
		goto exit;
	}

	hiusbc_stop(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n");

exit:
	hiusbc->gadget_driver = NULL;
	spin_unlock_irqrestore(&hiusbc->lock, flags);

	return ret;
}

static void hiusbc_gadget_set_speed(struct usb_gadget *g,
				enum usb_device_speed speed)
{
	struct hiusbc *hiusbc = to_hiusbc(g);
	unsigned long flags;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+\n");

	spin_lock_irqsave(&hiusbc->lock, flags);

	if (hiusbc->current_dr_mode != HIUSBC_DR_MODE_DEVICE) {
		dev_err(hiusbc->dev,
			"it's not in device mode now or failed\n");
		goto error;
	}

	hiusbc_set_speed(hiusbc, speed);

error:
	spin_unlock_irqrestore(&hiusbc->lock, flags);
}

static const struct usb_gadget_ops hiusbc_gadget_ops = {
	.get_frame		= hiusbc_gadget_get_frame,
	.wakeup			= hiusbc_gadget_wakeup,
	.set_selfpowered	= hiusbc_gadget_set_selfpowered,
	.pullup			= hiusbc_gadget_pullup,
	.udc_start		= hiusbc_gadget_start,
	.udc_stop		= hiusbc_gadget_stop,
	.udc_set_speed		= hiusbc_gadget_set_speed,
};

int hiusbc_gadget_delegate_req(struct hiusbc *hiusbc,
				const struct usb_ctrlrequest *setup)
{
	int ret;

	spin_unlock(&hiusbc->lock);
	ret = hiusbc->gadget_driver->setup(&hiusbc->gadget, setup);
	spin_lock(&hiusbc->lock);

	return ret;
}

/* called when VBUS drops below session threshold, and in other cases */
void hiusbc_gadget_disconnect(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->gadget_driver && hiusbc->gadget_driver->disconnect) {
		spin_unlock(&hiusbc->lock);
		hiusbc->gadget_driver->disconnect(&hiusbc->gadget);
		spin_lock(&hiusbc->lock);
	}
}

/* called when SOF packets stop for 3+ msec or enters U3 */
void hiusbc_gadget_suspend(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->gadget_driver && hiusbc->gadget_driver->suspend) {
		spin_unlock(&hiusbc->lock);
		hiusbc->gadget_driver->suspend(&hiusbc->gadget);
		spin_lock(&hiusbc->lock);
	}
}

void hiusbc_gadget_resume(struct hiusbc *hiusbc)
{
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->gadget_driver && hiusbc->gadget_driver->resume) {
		spin_unlock(&hiusbc->lock);
		hiusbc->gadget_driver->resume(&hiusbc->gadget);
		spin_lock(&hiusbc->lock);
	}
}

void hiusbc_gadget_reset(struct hiusbc *hiusbc)
{
	if (!hiusbc->gadget_driver)
		return;

	hiusbc_dbg(HIUSBC_DEBUG_EVENT, "+\n");

	if (hiusbc->gadget.speed != USB_SPEED_UNKNOWN) {
		spin_unlock(&hiusbc->lock);
		usb_gadget_udc_reset(&hiusbc->gadget, hiusbc->gadget_driver);
		spin_lock(&hiusbc->lock);
	}
}


static int hiusbc_gadget_init_interrupt(struct hiusbc *hiusbc)
{
	struct hiusbc_evt_ring *ring = NULL;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	/* lloc a event ring, int_idx is 0. */
	ring = hiusbc_event_ring_alloc(hiusbc, HIUSBC_TRBS_PER_EVT_RING, 0);
	if (!ring) {
		dev_err(hiusbc->dev, "Failed to alloc Event Ring!\n");
		ret = -ENOMEM;
		goto error;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"Successful alloc Event Ring, "
		"start TRB = %p (0x%llx dma), size = %uTRBs\n",
		ring->trbs, ring->dma, ring->size);

	hiusbc->event_ring = ring;

	hiusbc_event_buffer_init(ring);
	if (hiusbc->imodi)
		hiusbc_set_imodi(ring, hiusbc->imodi);

	hiusbc_enable_intr(ring);
	hiusbc_enable_sys_err_intr(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

error:
	return ret;
}

void hiusbc_gadget_exit_interrupt(struct hiusbc *hiusbc)
{
	struct hiusbc_evt_ring *ring = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	ring = hiusbc->event_ring;

	hiusbc_disable_sys_err_intr(hiusbc);
	hiusbc_disable_intr(ring);
	hiusbc_set_imodi(ring, 0);
	hiusbc_event_buffer_exit(ring);
	hiusbc_event_ring_free(ring);

	hiusbc->event_ring = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

int hiusbc_reinit_interrupt(struct hiusbc *hiusbc)
{
	struct hiusbc_evt_ring *ring = NULL;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	if (hiusbc->event_ring == NULL) {
		dev_err(hiusbc->dev, "event ring not allocated!\n");
		return -EINVAL;
	}

	ring = hiusbc->event_ring;

	hiusbc_disable_sys_err_intr(hiusbc);
	hiusbc_disable_intr(ring);
	hiusbc_set_imodi(ring, 0);
	hiusbc_event_buffer_exit(ring);

	hiusbc_event_buffer_init(ring);
	if (hiusbc->imodi)
		hiusbc_set_imodi(ring, hiusbc->imodi);

	hiusbc_enable_intr(ring);
	hiusbc_enable_sys_err_intr(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;
}

static int hiusbc_init_eps(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	u8 epnum;
	bool is_in = false;
	int ret = 0;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	INIT_LIST_HEAD(&hiusbc->gadget.ep_list);

	for (epnum = 0; epnum < HIUSBC_EP_NUMS; epnum++) {
		is_in = epnum & 1;

		hep = kzalloc(sizeof(*hep), GFP_KERNEL);
		if (!hep) {
			ret = -ENOMEM;
			break;
		}

		hep->hiusbc = hiusbc;
		hep->epnum = epnum;
		hep->is_in = is_in;
		INIT_LIST_HEAD(&hep->queue);
		hiusbc->eps[epnum] = hep;

		if (sprintf_s(hep->name, sizeof(hep->name), "ep%u%s",
			epnum >> 1, is_in ? "in" : "out") == -1)
			return -ENOMEM;

		hep->ep.name = hep->name;

		hep->ep.ops = &hiusbc_gadget_ep_ops;
		hep->ep.caps.dir_in = is_in;
		hep->ep.caps.dir_out = !is_in;

		if (epnum <= 1) {
			usb_ep_set_maxpacket_limit(&hep->ep, 512);
			hep->ep.maxburst = 1;
			hep->ep.caps.type_control = true;
			hep->ep.desc = hiusbc->ep0_desc;
			hep->ep.comp_desc = NULL;

			if (!epnum)
				hiusbc->gadget.ep0 = &hep->ep;

			hep->ring = hiusbc_xfer_ring_alloc(
				hiusbc, 1, 1, GFP_KERNEL);
			if (!hep->ring) {
				kfree(hep);
				ret = -ENOMEM;
				break;
			}
		} else {
			usb_ep_set_maxpacket_limit(&hep->ep, 1024);
			hep->ep.caps.type_iso = true;
			hep->ep.caps.type_bulk = true;
			hep->ep.caps.type_int = true;
			list_add_tail(&hep->ep.ep_list,
					&hiusbc->gadget.ep_list);

			hep->ring = hiusbc_xfer_ring_alloc(
				hiusbc, 2, 1, GFP_KERNEL);
			if (!hep->ring) {
				kfree(hep);
				ret = -ENOMEM;
				break;
			}
		}

		hiusbc_dbg(HIUSBC_DEBUG_SYS,
			"Successful init EP%u, Xfe Ring = 0x%p, "
			"start TRB = 0x%p (0x%llx dma), "
			"num_segs = %u, num_trbs = %u.\n",
			epnum, hep->ring,
			hep->ring->enqueue,
			hiusbc_xfer_trb_vrt_to_dma(
			hep->ring->enqueue_seg, hep->ring->enqueue),
			hep->ring->num_segs, hep->ring->num_trbs_free);
		hiusbc_dbg(HIUSBC_DEBUG_SYS, "  ep = %p\n", &hep->ep);
	}

	if (epnum > 0 && epnum < HIUSBC_EP_NUMS) {
		epnum--;
		while (epnum >= 0) {
			hep = hiusbc->eps[epnum];
			hiusbc_xfer_ring_free(hiusbc, hep->ring);
			list_del(&hep->ep.ep_list);
			kfree(hep);
			hiusbc->eps[epnum] = NULL;
			epnum--;
		}
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return ret;
}

static void hiusbc_exit_eps(struct hiusbc *hiusbc)
{
	struct hiusbc_ep *hep = NULL;
	u8 epnum;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	for (epnum = 0; epnum < HIUSBC_EP_NUMS; epnum++) {
		hep = hiusbc->eps[epnum];
		if (!hep)
			continue;

		hiusbc_xfer_ring_free(hiusbc, hep->ring);
		if (epnum > 1)
			list_del(&hep->ep.ep_list);

		kfree(hep);
		hiusbc->eps[epnum] = NULL;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}

static int hiusbc_mem_init(struct hiusbc *hiusbc)
{
	int ret = 0;

	hiusbc->dev_req_response_buf =
			kzalloc(HIUSBC_DEV_REQ_RESPONSE_BUF_SIZE, GFP_KERNEL);
	if (!hiusbc->dev_req_response_buf) {
		ret = -ENOMEM;
		goto exit;
	}

	hiusbc->xfer_seg_pool =
		dma_pool_create("hiusbc transfer ring segments",
			hiusbc->sysdev, HIUSBC_XFER_SEG_SIZE,
			HIUSBC_XFER_SEG_SIZE, 0);
	if (!hiusbc->xfer_seg_pool) {
		ret = -ENOMEM;
		goto alloc_dma_pool_fail;
	}

	hiusbc->ep0_desc = kzalloc(sizeof(*hiusbc->ep0_desc), GFP_KERNEL);
	if (!hiusbc->ep0_desc) {
		ret = -ENOMEM;
		goto alloc_ep0_desc_fail;
	}
	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"kzalloc ep_desc = 0x%p\n", hiusbc->ep0_desc);
	hiusbc->ep0_desc->bLength = USB_DT_ENDPOINT_SIZE;
	hiusbc->ep0_desc->bDescriptorType = USB_DT_ENDPOINT;
	hiusbc->ep0_desc->bmAttributes = USB_ENDPOINT_XFER_CONTROL;

	hiusbc->gadget.ops = &hiusbc_gadget_ops;
	hiusbc->gadget.speed = USB_SPEED_UNKNOWN;
	hiusbc->gadget.sg_supported = true;
	hiusbc->gadget.lpm_capable = true;
	hiusbc->gadget.name = "hiusbc-gadget";

	hiusbc->gadget.max_speed = hiusbc->max_speed;
	hiusbc_dbg(HIUSBC_DEBUG_ERR,
		"init: gadget.max_speed = %d\n", hiusbc->gadget.max_speed);
	return ret;

alloc_ep0_desc_fail:
	dma_pool_destroy(hiusbc->xfer_seg_pool);
	hiusbc->xfer_seg_pool = NULL;

alloc_dma_pool_fail:
	kfree(hiusbc->dev_req_response_buf);
	hiusbc->dev_req_response_buf = NULL;
exit:
	return ret;
}

static void hiusbc_mem_deinit(struct hiusbc *hiusbc)
{
	kfree(hiusbc->ep0_desc);
	hiusbc->ep0_desc = NULL;

	dma_pool_destroy(hiusbc->xfer_seg_pool);
	hiusbc->xfer_seg_pool = NULL;

	kfree(hiusbc->dev_req_response_buf);
	hiusbc->dev_req_response_buf = NULL;
}

int hiusbc_gadget_init(struct hiusbc *hiusbc)
{
	int irq;
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	ret = hiusbc_run_stop(hiusbc, 0);
	if (ret)
		goto exit;

	ret = hiusbc_reset_controller(hiusbc);
	if (ret)
		goto exit;

	irq = hiusbc_get_irq(hiusbc);
	if (irq < 0) {
		ret = irq;
		goto exit;
	}
	hiusbc->irq = irq;

	ret = hiusbc_gadget_init_interrupt(hiusbc);
	if (ret)
		goto exit;

	ret = hiusbc_mem_init(hiusbc);
	if (ret)
		goto mem_init_fail;

	ret = hiusbc_init_eps(hiusbc);
	if (ret)
		goto init_eps_fail;

	ret = request_irq(hiusbc->irq, hiusbc_interrupt,
			IRQF_TRIGGER_HIGH | IRQF_SHARED, "hiusbc", hiusbc);
	if (ret) {
		dev_err(hiusbc->dev, "Failed to request irq!\n");
		goto irq_err;
	}

	ret = usb_add_gadget_udc(hiusbc->dev, &hiusbc->gadget);
	if (ret) {
		dev_err(hiusbc->dev, "Failed to usb_add_gadget_udc!\n");
		goto add_udc_fail;
	}

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");

	return 0;

add_udc_fail:
	free_irq(hiusbc->irq, hiusbc);

irq_err:
	hiusbc_exit_eps(hiusbc);

init_eps_fail:
	hiusbc_mem_deinit(hiusbc);

mem_init_fail:
	hiusbc_gadget_exit_interrupt(hiusbc);

exit:
	hiusbc_dbg(HIUSBC_DEBUG_ERR, "%s-: ret = %d", ret);
	return ret;
}

void hiusbc_gadget_exit(struct hiusbc *hiusbc)
{
	u32 reg;

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "+\n");

	hiusbc_dbg(HIUSBC_DEBUG_SYS,
		"Before gadget exit, print EB Registers:\n");
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_iman_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  IMAN = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_imod_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  IMOD = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebsz_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  EBSZ = %u\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebdp0_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  EBDP0 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebdp1_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  EBDP1 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebba0_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  EBBA0 = 0x%x\n",  reg);
	reg = hiusbc_readl(hiusbc->dev_regs, hiusbc_dev_ebba1_offset(0));
	hiusbc_dbg(HIUSBC_DEBUG_SYS, "  EBBA1 = 0x%x\n",  reg);

	usb_del_gadget_udc(&hiusbc->gadget);

	hiusbc_stop(hiusbc);

	free_irq(hiusbc->irq, hiusbc);
	free_irq(hiusbc->sys_err_irq, hiusbc);

	hiusbc_exit_eps(hiusbc);

	kfree(hiusbc->ep0_desc);
	hiusbc->ep0_desc = NULL;

	dma_pool_destroy(hiusbc->xfer_seg_pool);
	hiusbc->xfer_seg_pool = NULL;

	kfree(hiusbc->dev_req_response_buf);

	hiusbc_gadget_exit_interrupt(hiusbc);

	hiusbc_dbg(HIUSBC_DEBUG_SYS, "-\n");
}
