/*
 * hisi_charging_watchdog.h
 *
 * Description: Hisi charge watchdog driver header
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#ifndef _HISI_CHARGING_WATCHDOG
#define _HISI_CHARGING_WATCHDOG

#define LOCK                            0x00000001
#define WATCHDOG_CLOCK_COUNT            32768
#define CHG_WATCHDOG_TIME               60
#define CHG_WATCHDOG_EN                 0x03
#define CHG_WATCHDOG_DIS                0x0

#ifdef CONFIG_CHARGER_WDG
#define UNLOCK                         0x1AAEE533
#define CHG_WATHDOG_WDUNLOCK_ADDR(base)         SOC_HIWDGV500_WDUNLOCK_ADDR(base)
#define CHG_WATHDOG_WDUNLOCK_CHECK_ADDR(base)   SOC_HIWDGV500_WDUNLOCK_CHECK_ADDR(base)
#define CHG_WATHDOG_WDLOAD_ADDR(base)           SOC_HIWDGV500_WDRST_ADDR(base)
#define CHG_WATHDOG_WDINTCNT_ADDR(base)         SOC_HIWDGV500_WDINTCNT_ADDR(base)
#define CHG_WATHDOG_WDMIS_ADDR(base)            SOC_HIWDGV500_WDMIS_ADDR(base)
#define CHG_WATHDOG_WDMIRST_ADDR(base)          SOC_HIWDGV500_WDMIRST_ADDR(base)
#define CHG_WATHDOG_WDKICK_ADDR(base)           SOC_HIWDGV500_WDKICK_ADDR(base)
#define CHG_WATHDOG_WDLOCK_ADDR(base)           SOC_HIWDGV500_WDLOCK_ADDR(base)
#define CHG_WATHDOG_WDINTCLR_ADDR(base)         SOC_HIWDGV500_WDINTCLR_ADDR(base)
#define CHG_WATHDOG_WDCTRL_ADDR(base)           SOC_HIWDGV500_WDCONTROL_ADDR(base)
#define CHG_WATHDOG_WDVALUE_ADDR(base)          SOC_HIWDGV500_WDVALUE_ADDR(base)
#else
#define UNLOCK                          0x1ACCE551
#define CHG_WATHDOG_WDLOCK_ADDR(base)   SOC_RTCTIMERWDTV100_WDLOCK_ADDR(base)
#define CHG_WATHDOG_WDLOAD_ADDR(base)   SOC_RTCTIMERWDTV100_WDLOAD_ADDR(base)
#define CHG_WATHDOG_WDVALUE_ADDR(base)  SOC_RTCTIMERWDTV100_WDVALUE_ADDR(base)
#define CHG_WATHDOG_WDINTCLR_ADDR(base) SOC_RTCTIMERWDTV100_WDINTCLR_ADDR(base)
#define CHG_WATHDOG_WDCTRL_ADDR(base)   SOC_RTCTIMERWDTV100_WDCONTROL_ADDR(base)
#endif

struct hisi_chgwdg_device {
	void __iomem *base;
	unsigned int irq;
};

#endif
