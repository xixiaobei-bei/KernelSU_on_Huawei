/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: Interface for usb.
 * Create: 2019-6-16
 *
 * This software is distributed under the terms of the GNU General
 * Public License ("GPL") as published by the Free Software Foundation,
 * either version 2 of that License or (at your option) any later version.
 */
#include <linux/hisi/usb/dwc3_usb_interface.h>
#include <linux/kernel.h>
#include <linux/ulpi/driver.h>

#include "core.h"
#include "io.h"
static struct dwc3 *g_dwc;

static atomic_t chip_dwc3_power_on = ATOMIC_INIT(0);

int dwc3_read_ulpi_reg(u8 addr)
{
	if (!g_dwc)
		return 0;

#ifdef CONFIG_USB_ULPI_BUS
	return ulpi_read(g_dwc->ulpi, addr);
#else
	return 0;
#endif
}

int dwc3_write_ulpi_reg(u8 addr, u8 val)
{
	if (!g_dwc)
		return 0;

#ifdef CONFIG_USB_ULPI_BUS
	return ulpi_write(g_dwc->ulpi, addr, val);
#else
	return 0;
#endif
}

void set_chip_dwc3_power_flag(int val)
{
	unsigned long flags = 0;

	if (g_dwc) {
		spin_lock_irqsave(&g_dwc->lock, flags);
		pr_info("get dwc3 lock\n");
	}

	atomic_set(&chip_dwc3_power_on, val);
	pr_info("set chip_dwc3_power_flag %d\n", val);
	if (g_dwc) {
		spin_unlock_irqrestore(&g_dwc->lock, flags);
		pr_info("put dwc3 lock\n");
	}
}

int get_chip_dwc3_power_flag(void)
{
	int val;

	val = atomic_read(&chip_dwc3_power_on);
	return val;
}

static void dwc3_disable_pipe_clock(void)
{
	u32 value;

	value = dwc3_readl(g_dwc->regs, DWC3_GUCTL1);
	value |= DWC3_GUCTL1_DISREFCLKGTNG_BIT | DWC3_GUCTL1_RESERVED26;
	dwc3_writel(g_dwc->regs, DWC3_GUCTL1, value);
}

int dwc3_compliance_mode_enable(void)
{
	u32 value;
	u32 reg;

	value = dwc3_readl(g_dwc->regs, DWC3_DCTL);
	value &= DWC3_DCTL_RUN_STOP;
	dwc3_writel(g_dwc->regs, DWC3_DCTL, value);

	value = dwc3_readl(g_dwc->regs, DWC3_GCTL);
	value &= ~DWC3_GCTL_PRTCAP_MASK;
	value |= DWC3_GCTL_PRTCAPDIR(1);
	dwc3_writel(g_dwc->regs, DWC3_GCTL, value);

	/* reset host controoler */
	value = 0x2;
	dwc3_writel(g_dwc->regs, 0x20, value);

	/* set port 3 to poweroff */
	reg = 0x420;
	value = dwc3_readl(g_dwc->regs, reg);
	value &= ~(1u << 9); /* Operation Parameter */
	dwc3_writel(g_dwc->regs, reg, value);

	value = dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0));
	value |= DWC3_GUSB3PIPECTL_HSTPRTCMPl;
	dwc3_writel(g_dwc->regs, DWC3_GUSB3PIPECTL(0), value);
	return 0;
}

void dwc3_cptest_next_pattern(void)
{
	u32 value;

	value = dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0));
	value &= ~DWC3_GUSB3PIPECTL_HSTPRTCMPl;
	dwc3_writel(g_dwc->regs, DWC3_GUSB3PIPECTL(0), value);
	mdelay(10); /* mdelay time */
	value = dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0));
	value |= DWC3_GUSB3PIPECTL_HSTPRTCMPl;
	dwc3_writel(g_dwc->regs, DWC3_GUSB3PIPECTL(0), value);
}

static int dwc3_enable_u3(void)
{
	u32 value;
	int ret = 0;
	/* Set REFCLKPER */
	/* DP4 + usb2.0 need */
	value = dwc3_readl(g_dwc->regs, DWC3_GUCTL);
	value |= (1ul << 26); /* Operation Parameter */
	dwc3_writel(g_dwc->regs, DWC3_GUCTL, value);

	/* enable U3 */
	value = dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0));
	value |= DWC3_GUSB3PIPECTL_SUSPHY;
	dwc3_writel(g_dwc->regs, DWC3_GUSB3PIPECTL(0), value);

	value = dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0));
	if (!(value & DWC3_GUSB3PIPECTL_SUSPHY)) {
		pr_err("USB 0xc2c0[0x%x]  err\n", value);
		ret = -1;
	}
	return ret;
}

static void dwc3_dump_regs(void)
{
	pr_info("[c2c0:0x%x], [c118:0x%x], [d050:0x%x]\n",
		dwc3_readl(g_dwc->regs, DWC3_GUSB3PIPECTL(0)),
		dwc3_readl(g_dwc->regs, DWC3_GSTS),
		dwc3_readl(g_dwc->regs, DWC3_LINK_GDBGLTSSM(0)));
}

#ifdef CONFIG_HISI_DEBUG_FS
static void usb3_link_state_print(void)
{
	u32 value;
	unsigned long long int count6 = 0;
	int flg6 = 1;
	int flg3 = 1;
	unsigned long long int count3 = 0;
	unsigned long long int count8 = 0;
	u32 old = 0;

	while (!chip_dwc3_is_powerdown()) {
		value = dwc3_readl(g_dwc->regs, DWC3_LINK_GDBGLTSSM(0));
		switch ((value >> 22) & 0xf) { /* LTDB Link State */
		case 6:
			count6++;
			if ((count6 % 100000) == 0) /* calculation remainder */
				pr_debug("[LINK]:%x, %llu\n", value, count6);
			if (flg6) {
				pr_debug("[LINK]:%x, %llu\n", value, count6);
				count6 = 0;
				flg6 = 0;
				flg3 = 1;
			}
			break;
		case 0:
		case 1:
		case 2:
		case 3:
			count3++;
			if ((count3 % 100000) == 0) /* calculation remainder */
				pr_debug("[LINK]:%x, %llu\n", value, count3);
			if (flg3) {
				pr_debug("[LINK]:0x%x, %llu\n", value, count3);
				count3 = 0;
				flg3 = 0;
				flg6 = 1;
			}
			break;
		case 4:
		case 5:
		case 7:
		case 8:
			flg3 = 1;
			flg6 = 1;
			if (((value >> 18) & 0xf) == old) { /* LTDB Sub-State */
				count8++;
				break;
			}
			old = ((value >> 18) & 0xf); /* LTDB Sub-State */
			pr_debug("[USB.DBG.LINK]:0x%x\n", value);
			break;
		default:
			flg3 = 1;
			flg6 = 1;
			pr_debug("[USB.DBG.LINK]:0x%x\n", value);
		}
	}
}
#endif

static void dwc3_logic_analyzer_trace_set(u32 value)
{
	u32 regvalue;

	regvalue = dwc3_readl(g_dwc->regs, DWC3_GDBGLSPMUX_DEV);
	regvalue = DWC3_GDBGLSPMUX_DEV_ANALYZER_SET(regvalue, value);
	dwc3_writel(g_dwc->regs, DWC3_GDBGLSPMUX_DEV, regvalue);
}

void dwc3_lscdtimer_set(void)
{
#if defined(CONFIG_USB_DWC3_FEB)
	u32 value;

	value = 0x47e1f4;
	dwc3_writel(g_dwc->regs, DWC3_LSCDTIM1(0), value);

	value = 0x753546;
	dwc3_writel(g_dwc->regs, DWC3_LSCDTIM2(0), value);

	value = dwc3_readl(g_dwc->regs, DWC3_LLUCTL(0));
	value &= ~DWC_LLUCTL_PENDING_HP_TIMER_US_MASK;
	value |= DWC_LLUCTL_PENDING_HP_TIMER_US(11);
	dwc3_writel(g_dwc->regs, DWC3_LLUCTL(0), value);

	value = dwc3_readl(g_dwc->regs, DWC3_LLUCTL(0));
	value |= DWC_LLUCTL_ENABLE_HP_TIMER;
	dwc3_writel(g_dwc->regs, DWC3_LLUCTL(0), value);

	value = 0x940060;
	dwc3_writel(g_dwc->regs, DWC3_LU1LFPSTXTIM(0), value);

	value = dwc3_readl(g_dwc->regs, DWC3_GUCTL);
	value &= ~DWC3_GUCTL_ENOVERLAPCHK;
	dwc3_writel(g_dwc->regs, DWC3_GUCTL, value);

	value = dwc3_readl(g_dwc->regs, DWC3_LSKIPFREQ(0));
	value &= (~SET_NBITS_MASK(20, 27));
	value |= (0xd9 << 20);
	dwc3_writel(g_dwc->regs, DWC3_LSKIPFREQ(0), value);

	/* fix chip bug 9001103026 */
	value = dwc3_readl(g_dwc->regs, DWC3_GUCTL1);
	value |= (1 << 13);
	dwc3_writel(g_dwc->regs, DWC3_GUCTL1, value);
#endif

#if defined(CONFIG_USB_DWC3_OCT) || defined(CONFIG_USB_DWC3_NOV)
	u32 value;

	/* Set Global User Control */
	value = dwc3_readl(g_dwc->regs, DWC3_GUCTL2);
	value |= (1 << 20);
	value &= ~(0x1Fu << 21);
	value |= 11;
	value |= (1 << 19);
	dwc3_writel(g_dwc->regs, DWC3_GUCTL2, value);
#endif
}

static struct usb3_core_ops dwc3_core_ops = {
	.disable_pipe_clock = dwc3_disable_pipe_clock,
	.enable_u3 = dwc3_enable_u3,
	.dump_regs = dwc3_dump_regs,
#ifdef CONFIG_HISI_DEBUG_FS
	.link_state_print = usb3_link_state_print,
#endif
	.logic_analyzer_trace_set = dwc3_logic_analyzer_trace_set,
};

struct usb3_core_ops *get_usb3_core_ops(void)
{
	return &dwc3_core_ops;
}

int usb3_interface_register(struct dwc3 *dwc)
{
	g_dwc = dwc;
	return 0;
}

int usb3_interface_unregister(struct dwc3 *dwc)
{
	g_dwc = NULL;
	return 0;
}
