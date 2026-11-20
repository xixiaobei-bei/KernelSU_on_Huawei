/*
 * hiusbc_debug.c -- Driver for Hisilicon USB Controller.
 *
 * Copyright (c) 2019 Hisilicon Technologies CO., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#include <linux/delay.h>
#include <securec.h>
#include "hiusbc_core.h"
#include "hiusbc_gadget.h"
#include "hiusbc_debug.h"

void hiusbc_clear_xfer_event_counter(struct hiusbc_ep *hep)
{
	hep->succ_num = 0;
	hep->short_num = 0;
	hep->nrdy_num = 0;
	hep->babble_num = 0;
	hep->fifo_err_num = 0;
	hep->isoc_overrun_num = 0;
	hep->trans_err_num = 0;
	hep->miss_service_num = 0;
	hep->ctrl_short_num = 0;
}

void hiusbc_show_xfer_event_counter(struct hiusbc_ep *hep)
{
	pr_err("ep%u is %s\n",
		hep->epnum, hep->enabled ? "enabled" : "disabled");
	pr_err("event counter on ep%u:\n"
			"success: %u\n"
			"short: %u\n"
			"nrdy: %u\n"
			"babble: %u\n"
			"fifo err: %u\n"
			"isoc overrun: %u\n"
			"trans err: %u\n"
			"miss service: %u\n"
			"ctrl short: %u\n",
			hep->epnum,
			hep->succ_num,
			hep->short_num,
			hep->nrdy_num,
			hep->babble_num,
			hep->fifo_err_num,
			hep->isoc_overrun_num,
			hep->trans_err_num,
			hep->miss_service_num,
			hep->ctrl_short_num);
}

void hiusbc_clear_dev_event_counter(struct hiusbc *hiusbc)
{
	if (memset_s(&hiusbc->stat, sizeof(hiusbc->stat),
		0, sizeof(hiusbc->stat)) != EOK)
		pr_err("%s memset err!\n");

}

void hiusbc_show_dev_event_counter(struct hiusbc *hiusbc)
{
	u32 dev_event_sum;

	dev_event_sum = hiusbc->stat.disconnect_num +
			hiusbc->stat.connect_num +
			hiusbc->stat.rst_num + hiusbc->stat.plc_num +
			hiusbc->stat.suspend_num + hiusbc->stat.resume_num +
			hiusbc->stat.l1_suspend_num +
			hiusbc->stat.l1_resume_num +
			hiusbc->stat.sof_num + hiusbc->stat.phy_err_num +
			hiusbc->stat.eb_full_num + hiusbc->stat.unsuccess_num +
			hiusbc->stat.event_after_discon_num;

	pr_err("dev event counter:\n"
			"disconnect: %u\n"
			"connect: %u\n"
			"rst: %u\n"
			"plc: %u\n"
			"suspend: %u\n"
			"resume: %u\n"
			"l1_suspend: %u\n"
			"l1_resume: %u\n"
			"sof: %u\n"
			"phy_err: %u\n"
			"eb_full: %u\n"
			"unsuccess: %u\n"
			"non-rst event after discon: %u\n"
			"  sum of above = %u\n"
			"dev_evt_interrupt: %u\n"
			"combo: %u\n",
			hiusbc->stat.disconnect_num,
			hiusbc->stat.connect_num,
			hiusbc->stat.rst_num,
			hiusbc->stat.plc_num,
			hiusbc->stat.suspend_num,
			hiusbc->stat.resume_num,
			hiusbc->stat.l1_suspend_num,
			hiusbc->stat.l1_resume_num,
			hiusbc->stat.sof_num,
			hiusbc->stat.phy_err_num,
			hiusbc->stat.eb_full_num,
			hiusbc->stat.unsuccess_num,
			hiusbc->stat.event_after_discon_num,
			dev_event_sum,
			hiusbc->stat.dev_event_total_num,
			hiusbc->stat.combo_num);
}

int hiusbc_debug_knock_ep(struct hiusbc_ep *hep)
{
	int ret;

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "+: EP%u\n", hep->epnum);
	if (!hep->enabled) {
		hiusbc_dbg(HIUSBC_DEBUG_TEMP,
			"EP%u not enabled!\n", hep->epnum);
		return 0;
	}

	ret = hiusbc_send_cmd_start_transfer(hep, 0, 0, 0, 0, 0);

	if (ret) {
		pr_err("Failed to send StartXfer cmd on EP%u! ret = %d\n",
			hep->epnum, ret);
		return ret;
	}

	hiusbc_dbg(HIUSBC_DEBUG_TEMP, "-\n", hep->epnum);
	return ret;
}

void hiusbc_cptest_set_to_cp0(struct hiusbc *hiusbc)
{
	u32 reg;

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SEL_OFFSET);
	reg |= MPI_U3_LINK_REG_CSR_CP_PAT_SEL_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SEL_OFFSET);

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_OFFSET);
	reg &= ~MPI_U3_LINK_REG_CSR_CP_PAT_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_OFFSET);

	mdelay(10);

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SEL_OFFSET);
	reg &= ~MPI_U3_LINK_REG_CSR_CP_PAT_SEL_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SEL_OFFSET);
}

void hiusbc_cptest_next_pattern(struct hiusbc *hiusbc)
{
	u32 reg;

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_OFFSET);
	reg |= MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_OFFSET);

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_OFFSET);
	reg |= MPI_U3_LINK_REG_CSR_CP_PAT_SW_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_OFFSET);

	mdelay(10);

	reg = hiusbc_readl(hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_OFFSET);
	reg &= ~MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_MASK;
	hiusbc_writel(reg, hiusbc->u3_link_regs,
		MPI_U3_LINK_REG_CSR_CP_PAT_SW_CTRL_OFFSET);
}
