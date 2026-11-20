/*
 * hisi_charging_watchdog.c
 *
 * Description: Hisi charge watchdog driver
 *
 * Copyright (c) 2015-2020 Huawei Technologies Co., Ltd.
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

#include "hisi_charging_watchdog.h"
#include "soc_rtctimerwdtv100_interface.h"
#include <huawei_platform/log/hw_log.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/kern_levels.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/version.h>
#include <securec.h>
#ifdef CONFIG_HISI_BB
#include <linux/hisi/rdr_pub.h>
#include <linux/hisi/util.h>
#include <linux/hisi/rdr_hisi_platform.h>
#endif
#define CHGWDT_LOG_INFO
#ifndef CHGWDT_LOG_INFO
#define chgwdt_debug(fmt, args...)do {} while (0)
#define chgwdt_info(fmt, args...) do {} while (0)
#define chgwdt_warn(fmt, args...) do {} while (0)
#define chgwdt_err(fmt, args...)  do {} while (0)
#else
#define chgwdt_debug(fmt, args...) pr_debug("[hisi_chgwdt]" fmt, ## args)

#define chgwdt_info(fmt, args...) pr_info("[hisi_chgwdt]" fmt, ## args)

#define chgwdt_warn(fmt, args...) pr_warn("[hisi_chgwdt]" fmt, ## args)

#define chgwdt_err(fmt, args...)  pr_err("[hisi_chgwdt]" fmt, ## args)

#endif
static struct hisi_chgwdg_device *g_chgwdg_di;
static unsigned char g_watchdog_enable;
static unsigned int watchdog_kick_cpu;
static u64 watchdog_kick_time;
static inline void __chg_wdt_lock(const void __iomem *base, unsigned int lock_key)
{
	__raw_writel(lock_key, CHG_WATHDOG_WDLOCK_ADDR((char *)base));
}

#ifdef CONFIG_CHARGER_WDG
static inline void __chg_wdt_unlock(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDUNLOCK_ADDR((char *)base));
}
static inline unsigned int __chg_wdt_unlock_check(const void __iomem *base)
{
	return !(__raw_readl(CHG_WATHDOG_WDUNLOCK_CHECK_ADDR((char *)base)));
}
static inline void __chg_wdt_intset(const void __iomem *base, unsigned int value)
{
	__raw_writel(WATCHDOG_CLOCK_COUNT * value, CHG_WATHDOG_WDINTCNT_ADDR((char *)base));
}
static inline void __chg_wdt_misrst(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDMIRST_ADDR((char *)base));
}
static inline void __chg_wdt_misintr(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDMIS_ADDR((char *)base));
}
static inline unsigned int __chg_wdt_kick(const void __iomem *base)
{
	__raw_writel(0x1, CHG_WATHDOG_WDKICK_ADDR((char *)base));
}
#else
static inline void __chg_wdt_unlock(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDLOCK_ADDR((char *)base));
}

static inline unsigned int __chg_wdt_unlock_check(const void __iomem *base)
{
	return __raw_readl(CHG_WATHDOG_WDLOCK_ADDR((char *)base));
}

#endif
static inline void __chg_wdt_load(const void __iomem *base, unsigned int timeout)
{
	__raw_writel(WATCHDOG_CLOCK_COUNT * timeout, CHG_WATHDOG_WDLOAD_ADDR((char *)base));
}

static inline unsigned int __chg_wdt_get_cnt(const void __iomem *base)
{
	return __raw_readl(CHG_WATHDOG_WDVALUE_ADDR((char *)base));
}

static inline void __chg_wdt_intclr(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDINTCLR_ADDR((char *)base));
}

static inline void __chg_wdt_control(const void __iomem *base, unsigned int value)
{
	__raw_writel(value, CHG_WATHDOG_WDCTRL_ADDR((char *)base));
}

#ifdef CONFIG_HISI_BB
/* init charger sys watchdog rdr */
static int rdr_charge_syswdt_init(void)
{
	struct rdr_exception_info_s einfo;
	unsigned int ret;
	errno_t ret_s;

	ret_s = memset_s(&einfo, sizeof(struct rdr_exception_info_s), 0,
		sizeof(struct rdr_exception_info_s));
	if (ret_s)
		chgwdt_err("memset_s einfo failed\n");

	einfo.e_modid = MODID_CHARGER_S_WDT;
	einfo.e_modid_end = MODID_CHARGER_S_WDT;
	/* The highest operation priority */
	einfo.e_process_priority = RDR_ERR;
	/* restart instantaneously */
	einfo.e_reboot_priority = RDR_REBOOT_NOW;
	/* notify AP to save log, to be modified */
	einfo.e_notify_core_mask = RDR_AP;
	/* notify AP to reset state and restore the whole system */
	einfo.e_reset_core_mask = RDR_AP;
	einfo.e_from_core = RDR_AP;
	/*
	 * the present exception is not allowed to reentry,
	 * (multiple exceptions won't be handled)
	 */
	einfo.e_reentrant = (u32)RDR_REENTRANT_DISALLOW;
	/* exception type initialization failure */
	einfo.e_exce_type = CHARGER_S_WDT;
	einfo.e_upload_flag = (u32)RDR_UPLOAD_YES;
	ret_s = memcpy_s(einfo.e_from_module, sizeof(einfo.e_from_module),
		"RDR_CHG_SYSWDT", sizeof("RDR_CHG_SYSWDT"));
	if (ret_s) {
		chgwdt_err("memcpy e_from_module failed\n");
		return ret_s;
	}
	ret_s = memcpy_s(einfo.e_desc, sizeof(einfo.e_desc),
		"RDR_CHG_SYSWDT", sizeof("RDR_CHG_SYSWDT"));
	if (ret_s) {
		chgwdt_err("memcpy e_desc failed\n");
		return ret_s;
	}
	ret = rdr_register_exception(&einfo);
	if (ret != MODID_CHARGER_S_WDT) {
		chgwdt_err("register rdr_charge_syswdt failed\n");
		return -1;
	}
	return 0;
}
#endif

/* callback function for chargerIC fault irq in charging process */
static irqreturn_t charger_watchdog_interrupt(int irq, void *_di)
{
#ifdef CONFIG_HISI_BB
	u64 a_wdt_kick_time = rdr_get_last_wdt_kick_slice();

	if (_di == NULL || irq == 0)
		chgwdt_err("%s:di is null\n", __func__);
	/* 32768: total gear, 2^15 = 32768, 15 bits store the kick time */
	chgwdt_err("charge wdt timeout,last kick cpu[%u],time:%llu ns, a_wdt_kick_time:%llu s\n",
		   watchdog_kick_cpu, watchdog_kick_time, (a_wdt_kick_time / 32768));
	rdr_syserr_process_for_ap((u32)MODID_CHARGER_S_WDT, (u64)0, (u64)0);
#endif
	return IRQ_HANDLED;
}

/* set timeout for charge watchdog */
void charge_feed_sys_wdt(unsigned int timeout)
{
	unsigned int cnt;

	if (g_chgwdg_di == NULL || g_chgwdg_di->base == NULL) {
		chgwdt_err("%s:NULL POINTER\n", __func__);
		return;
	}
	if (g_watchdog_enable == 0)
		return;

	__chg_wdt_unlock(g_chgwdg_di->base, UNLOCK);
	if (__chg_wdt_unlock_check(g_chgwdg_di->base) != 0)
		chgwdt_err("wdt unlock fail\n");

	cnt = __chg_wdt_get_cnt(g_chgwdg_di->base);
	__chg_wdt_load(g_chgwdg_di->base, timeout);
#ifdef CONFIG_CHARGER_WDG
	__chg_wdt_intset(g_chgwdg_di->base, timeout);
	__chg_wdt_kick(g_chgwdg_di->base);
#endif
	__chg_wdt_lock(g_chgwdg_di->base, LOCK);
	watchdog_kick_cpu = get_cpu();
	put_cpu();
	watchdog_kick_time = hisi_getcurtime();
	chgwdt_info("++charge watchdog feed, current cnt:%u, timeout is:%u ++\n",
		cnt / WATCHDOG_CLOCK_COUNT, timeout);
}
EXPORT_SYMBOL(charge_feed_sys_wdt);

void charge_enable_sys_wdt(void)
{
	if (g_chgwdg_di == NULL || g_chgwdg_di->base == NULL) {
		chgwdt_err("%s:NULL POINTER\n", __func__);
		return;
	}
	if (g_watchdog_enable == 0)
		return;

	__chg_wdt_unlock(g_chgwdg_di->base, UNLOCK);
	if (__chg_wdt_unlock_check(g_chgwdg_di->base) != 0)
		chgwdt_err("wdt unlock fail\n");

	__chg_wdt_load(g_chgwdg_di->base, CHG_WATCHDOG_TIME);
#ifdef CONFIG_CHARGER_WDG
	__chg_wdt_intset(g_chgwdg_di->base, CHG_WATCHDOG_TIME);
	__chg_wdt_misrst(g_chgwdg_di->base, 0x0);
	__chg_wdt_misintr(g_chgwdg_di->base, 0x0);
	__chg_wdt_control(g_chgwdg_di->base, CHG_WATCHDOG_EN);
	__chg_wdt_intclr(g_chgwdg_di->base, 0x1);
#else
	__chg_wdt_intclr(g_chgwdg_di->base, 0x0);
	__chg_wdt_control(g_chgwdg_di->base, CHG_WATCHDOG_EN);
#endif
	__chg_wdt_lock(g_chgwdg_di->base, LOCK);
	chgwdt_info("++charge watchdog start++\n");
}
EXPORT_SYMBOL(charge_enable_sys_wdt);

void charge_stop_sys_wdt(void)
{
	if (g_chgwdg_di == NULL || g_chgwdg_di->base == NULL) {
		chgwdt_err("%s:NULL POINTER\n", __func__);
		return;
	}
	__chg_wdt_unlock(g_chgwdg_di->base, UNLOCK);
	if (__chg_wdt_unlock_check(g_chgwdg_di->base) != 0)
		chgwdt_err("wdt unlock fail\n");

	__chg_wdt_intclr(g_chgwdg_di->base, 0x0);
	__chg_wdt_control(g_chgwdg_di->base, CHG_WATCHDOG_DIS);
	__chg_wdt_lock(g_chgwdg_di->base, LOCK);
	chgwdt_info("++charge watchdog stop++\n");
}
EXPORT_SYMBOL(charge_stop_sys_wdt);

static int hisi_chg_wdg_probe(struct platform_device *pdev)
{
	struct device_node *node = pdev->dev.of_node;
	int ret;

	g_chgwdg_di = kzalloc(sizeof(*g_chgwdg_di), GFP_KERNEL);
	if (g_chgwdg_di == NULL) {
		ret = -ENOMEM;
		goto out;
	}
	g_chgwdg_di->base = of_iomap(node, 0);
	if (!g_chgwdg_di->base) {
		chgwdt_err("iomap error\n");
		ret = -ENOMEM;
		goto free_dev;
	}

	g_chgwdg_di->irq = irq_of_parse_and_map(node, 0);
	if (g_chgwdg_di->irq == 0) {
		chgwdt_err("charge wdt irq is %u\n", g_chgwdg_di->irq);
		ret = -ENODEV;
		goto free_iomap;
	}
	charge_stop_sys_wdt();
	ret = request_irq(g_chgwdg_di->irq, charger_watchdog_interrupt,
		(unsigned long)IRQF_NO_SUSPEND, "charge_syswdt_irq", g_chgwdg_di);
	if (ret) {
		chgwdt_err("register charge wdt irq failed!\n");
		goto free_iomap;
	}
#ifdef CONFIG_HISI_BB
	if (!strstr(saved_command_line, "androidboot.swtype=factory") &&
		check_himntn(HIMNTN_GLOBAL_WDT))
		g_watchdog_enable = 1;
	else
		g_watchdog_enable = 0;
	ret = rdr_charge_syswdt_init();
	if (ret) {
		chgwdt_err("init charge wdt rdr failed!\n");
		goto free_iomap;
	}
#else
	disable_irq_nosync((unsigned int)g_chgwdg_di->irq);
#endif
	platform_set_drvdata(pdev, g_chgwdg_di);
	chgwdt_info("Hisi charge watchdog ready\n");
	return 0;
free_iomap:
	iounmap(g_chgwdg_di->base);
free_dev:
	kfree(g_chgwdg_di);
	g_chgwdg_di = NULL;
out:
	return ret;
}

static int hisi_chg_wdg_remove(struct platform_device *pdev)
{
	struct hisi_chgwdg_device *di = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	if (di != NULL) {
		iounmap(di->base);
		free_irq(di->irq, pdev);
		kfree(di);
	}
	return 0;
}

static const struct of_device_id hisi_chgwdg_of_match[] = {
	{
		.compatible = "hisilicon,chgwdg",
		.data = NULL
	},
	{},
};

MODULE_DEVICE_TABLE(of, hisi_chgwdg_of_match);

static struct platform_driver hisi_chgwdg_driver = {
	.probe = hisi_chg_wdg_probe,
	.remove = hisi_chg_wdg_remove,
	.driver = {
		.name = "Chg-watchdog",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(hisi_chgwdg_of_match),
	},
};

static int __init hisi_chgwdg_init(void)
{
	platform_driver_register(&hisi_chgwdg_driver);
	return 0;
}

module_init(hisi_chgwdg_init);

static void __exit hisi_chgwdg_exit(void)
{
	platform_driver_unregister(&hisi_chgwdg_driver);
}

module_exit(hisi_chgwdg_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("Hisi charge watchdog driver");
MODULE_AUTHOR("Wen Qi <qiwen4@hisilicon.com>");
