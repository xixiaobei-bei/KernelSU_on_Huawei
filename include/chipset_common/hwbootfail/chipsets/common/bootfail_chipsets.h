/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bootfail_chipsets.h
 *
 * define platform-dependent common bootfail interfaces
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

#ifndef BOOTFAIL_CHIPSETS_H
#define BOOTFAIL_CHIPSETS_H

/* ---- includes ---- */
#include <linux/types.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
/* ---- export prototypes ---- */
/*---- export function prototypes ----*/
unsigned int get_bootup_time(void);
unsigned int get_long_timer_timeout_value(void);
unsigned int get_short_timer_timeout_value(void);
unsigned int get_action_timer_timeout_value(void);
char *get_bfi_dev_path(void);
size_t get_bfi_part_size(void);
long simulate_storge_rdonly(unsigned long arg);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
