/*
 * om_debug.h
 *
 * debug for socdsp
 *
 * Copyright (c) 2013-2020 Huawei Technologies Co., Ltd.
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

#ifndef __OM_DEBUG_H__
#define __OM_DEBUG_H__

#include <linux/kernel.h>
#include "dsp_om.h"

enum dump_dsp_index {
	NORMAL_LOG = 0,
	NORMAL_BIN,
	PANIC_LOG,
	PANIC_BIN,
	DUMP_INDEX_MAX
};

struct socdsp_str_cmd {
	unsigned short msg_id;
	unsigned short str_len; /* length of string, include \0 */
	char str[0];
};

int om_debug_init(unsigned char *unsec_virt_addr);
void om_debug_deinit(void);

#ifdef ENABLE_HIFI_DEBUG
void socdsp_debug_data_handle(enum socdsp_om_work_id work_id,
	const unsigned char *addr, unsigned int len);
#endif

void socdsp_dump_dsp(enum dump_dsp_index index, const char *log_path);
void socdsp_dump_panic_log(void);
int socdsp_dump(uintptr_t arg);
int socdsp_get_dmesg(uintptr_t arg);
bool om_get_resetting_state(void);

#ifdef ENABLE_HIFI_DEBUG
#define can_reset_system() \
do { \
	if (om_get_resetting_state()) { \
		printk("soc dsp reset, reset all system by reset option"); \
		BUG_ON(true); \
	} \
} while (0)
#else
#define can_reset_system()
#endif

#endif /*__OM_DEBUG_H__ */

