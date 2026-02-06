/*
 *
 * hardware diaginfo record module.
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef __HISI_HW_DIAG_H__
#define __HISI_HW_DIAG_H__

#include <linux/hisi/hisi_bbox_diaginfo.h>
#include <linux/spinlock_types.h>

struct hisi_diag_noc_info {
	char *init_flow;
	char *target_flow;
};

struct hisi_diag_panic_info {
	unsigned int cpu_num;
};

union hisi_hw_diag_info {
	struct hisi_diag_noc_info noc_info;
	struct hisi_diag_panic_info cpu_info;
};

struct hisi_hw_diag_dev {
	spinlock_t record_lock;
	struct hisi_hw_diag_trace *trace_addr;
	unsigned int trace_size;
	unsigned int trace_max_num;
};

#ifdef CONFIG_HISI_HW_DIAG
void hisi_hw_diaginfo_trace(unsigned int err_id, const union hisi_hw_diag_info *diaginfo);
void hisi_hw_diaginfo_record(const char *date);
void hisi_hw_diag_init(void);
#else
static inline void hisi_hw_diaginfo_trace(unsigned int err_id, const union hisi_hw_diag_info *diaginfo) { return; }
static inline void hisi_hw_diaginfo_record(const char *date) { return; }
static inline void hisi_hw_diag_init(void) { return; }
#endif

#endif
