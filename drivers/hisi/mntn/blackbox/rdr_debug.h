/*
 * rdr_debug.h
 *
 * blackbox header file (blackbox: kernel run data recorder.)
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

#ifndef __BB_DEBUG_H__
#define __BB_DEBUG_H__

#include <linux/types.h>

#ifdef CONFIG_HISI_BB_DEBUG
int rdr_debug_init(void);
void hisi_syserr_loop_test(void);
#else
static inline int rdr_debug_init(void)
{
	return 0;
}
static inline void hisi_syserr_loop_test(void)
{
}
#endif /* End #define CONFIG_HISI_BB_DEBUG */
#endif /* End #define __BB_DEBUG_H__ */
