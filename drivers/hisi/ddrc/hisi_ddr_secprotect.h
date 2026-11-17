/*
 * hisi_ddr_secprotect.h
 *
 * Hisilicon DDR TEST driver (master only).
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __DDR_SECPROTECT_H
#define __DDR_SECPROTECT_H

#ifdef CONFIG_HISI_SP805_WATCHDOG
#define IRQ_WDT_INTR_FIQ		76
#endif
#ifdef CONFIG_HI_V500_WATCHDOG
#define IRQ_WDT_INTR_FIQ		431
#endif
#define IRQ_DMSS_INTR_FIQ		163
#define DMSS_INTR_FIQ_FLAG		0x5a5a5a5a5a5a5a00
#define DRM_SHARE_MEM_OFFSET	0x100
#define DMSS_SEM_INIT_SUCCESS   0x1

#endif
