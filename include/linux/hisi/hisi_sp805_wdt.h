/*
 * hisi_sp805_wdt.h
 *
 * Watchdog driver for ARM SP805 watchdog module.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
#ifndef __HISI_SP805_WDT_H
#define __HISI_SP805_WDT_H

#include <linux/types.h>

struct rdr_arctimer_s {
	u32 cntv_ctl_el0;
	u32 cntv_tval_el0;
	u32 cntp_ctl_el0;
	u32 cntp_tval_el0;
	u32 cntfrq_el0;

	u64 cntv_cval_el0;
	u64 cntp_cval_el0;
	u64 cntvct_el0;
	u64 cntpct_el0;
};

#ifdef CONFIG_HISI_SP805_WATCHDOG
extern unsigned int get_wdt_kick_time(void);
extern unsigned long get_wdt_expires_time(void);
extern void sp805_wdt_dump(void);
extern void rdr_arctimer_register_read(struct rdr_arctimer_s *arctimer);
extern void rdr_archtime_register_print(struct rdr_arctimer_s *arctimer, bool after);

extern struct rdr_arctimer_s g_rdr_arctimer_record;
static inline struct rdr_arctimer_s *rdr_get_arctimer_record(void)
{
	return &g_rdr_arctimer_record;
}

#else
static inline unsigned int get_wdt_kick_time(void) { return 0; }
static inline unsigned long get_wdt_expires_time(void) { return 0; }
static inline void sp805_wdt_dump(void) { return; }
static inline void rdr_arctimer_register_read(struct rdr_arctimer_s *arctimer) { return; }
static inline void rdr_archtime_register_print(struct rdr_arctimer_s *arctimer, bool after) { return; }
static inline struct rdr_arctimer_s *rdr_get_arctimer_record(void){ return NULL; }
#endif

#endif /* __HISI_SP805_WDT_H */
