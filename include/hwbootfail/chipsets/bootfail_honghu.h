/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: bootfail_honghu.h
 *
 * define platform-dependent bootfail interfaces
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

#ifndef BOOTFAIL_HONGHU_H
#define BOOTFAIL_HONGHU_H

/* ---- includes ---- */
#include <hwbootfail/core/boot_detector.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
/* ---- export prototypes ---- */
/* ---- export function prototypes ---- */
int process_bootfail(enum bootfail_errno bootfail_errno,
	enum suggest_recovery_method suggested_method,
	const char *detail_info);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
