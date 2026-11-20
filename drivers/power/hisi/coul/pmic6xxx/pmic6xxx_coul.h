/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2015-2020. All rights reserved.
 * Description: coulometer hardware driver headfile
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _HI6XXX_COUL_H_
#define _HI6XXX_COUL_H_

#include <asm/irq.h>
#include <linux/delay.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/jiffies.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/notifier.h>
#include <linux/of.h>
#include <linux/of_address.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/workqueue.h>
#include <linux/hisi/hisi_adc.h>
#include <linux/mfd/hisi_pmic.h>
#include <pmic_interface.h>
#if defined(CONFIG_COUL_PMIC6X55V200)
#include "pmic6555v200_coul.h"
#elif defined(CONFIG_COUL_PMIC6X55V300)
#include "pmic6555v300_coul.h"
#elif defined(CONFIG_COUL_PMIC6X21V600)
#include "pmic6421v600_coul.h"
#elif defined(CONFIG_COUL_PMIC6X21V700)
#include "pmic6421v700_coul.h"
#elif defined(CONFIG_COUL_PMIC6X21V800)
#include "pmic6421v800_coul.h"
#elif defined(CONFIG_COUL_PMIC6X55V500)
#include "pmic6555v500_coul.h"
#elif defined(CONFIG_COUL_PMIC6X21V900)
#include "pmic6421v900_coul.h"
#else
#include "pmic6421v900_coul.h"
#endif

#define PMU_ENABLE                              1
#define PMU_DISABLE                             0

#define LOCK                                    1
#define UNLOCK                                  0

#define R_COUL_MOHM                             10 /* resisitance mohm */
#define INVALID_TEMP                            (-99)
#define COUL_HARDWARE                             0x36
#define COUL_PMIC6X21V700                         0x700
#define DEFAULT_BATTERY_VOL_2_PERCENT           3350
#define DEFAULT_BATTERY_VOL_0_PERCENT           3150
#define DEFAULT_I_GATE_VALUE                    5000 /* 5000 mA */
#define COUL_BIT_MASK                           0x800000
#define COUL_NEG_EXPAND                         0xff000000
#define COUL_FIFO_VOL_DEFAULT                   0xffffff
#define PERMILLAGE                              1000
#define SECONDS_PER_HOUR                        3600
#define COUL_REG_READ(regAddr) \
	pmic_read_reg(regAddr)
#define COUL_REG_WRITE(regAddr, regval) \
	pmic_write_reg((int)(regAddr), (int)(regval))
#define COUL_REGS_READ(regAddr, buf, size) \
	pmic_array_read((int)(regAddr), (char *)(buf), (int)(size))
#define COUL_REGS_WRITE(regAddr, buf, size) \
	pmic_array_write((int)(regAddr), (char *)(buf), (int)(size))

#define COUL_HARDWARE_ERR(fmt, args...) \
	printk(KERN_ERR    "[coul_hardware]" fmt, ## args)
#define COUL_HARDWARE_EVT(fmt, args...) \
	printk(KERN_WARNING"[coul_hardware]" fmt, ## args)
#define COUL_HARDWARE_INF(fmt, args...) \
	printk(KERN_INFO   "[coul_hardware]" fmt, ## args)
#define COUL_HARDWARE_DBG(fmt, args...) do {} while (0)

struct coul_device_info {
	struct device *dev;
	struct delayed_work irq_work;
	int irq;
	unsigned char irq_mask;
	u16 chip_proj;
	u16 chip_version;
};

#endif
