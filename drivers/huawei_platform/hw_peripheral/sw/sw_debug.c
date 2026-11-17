/*
 * sw_debug.c
 *
 * sw debug function
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

#include "sw_debug.h"
#include <huawei_platform/log/log_exception.h>

#define SW_DEBUG_BUFSIZE             512
#define SW_DEBUG_DATA_COUNT_ONE_LINE 10

enum sw_loglevel_type g_sw_loglevel = SW_LOG_DEBUG;

void sw_debug_dump_data(const u8 *data, int count)
{
	int i;

	if ((count < 1) || !data)
		return;

	sw_print_dbg("[sw_report]len %d\n", count);
	sw_print_dbg("{ ");
	for (i = 0; i < count; i++) {
		sw_print_dbg("%x ", data[i]);
		if ((i > 0) && (i % SW_DEBUG_DATA_COUNT_ONE_LINE == 0))
			sw_print_dbg("\n");
	}
	sw_print_dbg("}\n");
}
