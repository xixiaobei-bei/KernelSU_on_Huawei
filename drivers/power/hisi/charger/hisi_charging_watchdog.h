/*
 * hisi_charging_watchdog.h
 *
 * Hisi charge watchdog driver header
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#define UNLOCK                          0x1ACCE551
#define LOCK                            0x00000001
#define WATCHDOG_CLOCK_COUNT            32768
#define CHG_WATCHDOG_TIME               60
#define CHG_WATCHDOG_EN                 0x03
#define CHG_WATCHDOG_DIS                0x0
#define CHG_WATHDOG_WDLOCK_ADDR(base)   SOC_RTCTIMERWDTV100_WDLOCK_ADDR(base)
#define CHG_WATHDOG_WDLOAD_ADDR(base)   SOC_RTCTIMERWDTV100_WDLOAD_ADDR(base)
#define CHG_WATHDOG_WDVALUE_ADDR(base)  SOC_RTCTIMERWDTV100_WDVALUE_ADDR(base)
#define CHG_WATHDOG_WDINTCLR_ADDR(base) SOC_RTCTIMERWDTV100_WDINTCLR_ADDR(base)
#define CHG_WATHDOG_WDCTRL_ADDR(base)   SOC_RTCTIMERWDTV100_WDCONTROL_ADDR(base)

struct hisi_chgwdg_device {
	void __iomem *base;
	unsigned int irq;
};

#endif
