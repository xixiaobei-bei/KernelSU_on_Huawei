/*
 * Hisilicon Printk.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */
#ifndef _HISI_PRINTK_H_
#define _HISI_PRINTK_H_

#include <linux/spinlock.h>

extern raw_spinlock_t *g_logbuf_level_lock_ex;

#define TIME_LOG_SIZE 80
#define TS_BUF_SIZE 80

#define FPGA_CONFIGED 1
#define FPGA_SERIAL_CLOCK 19200000

#define NS_INTERVAL 1000000000
#define FPGA_SCBBPD_RX_STAT1 0x1008
#define FPGA_SCBBPD_RX_STAT2 0x100c

#endif
