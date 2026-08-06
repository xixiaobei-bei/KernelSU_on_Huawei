/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: adapter_common.h
 *
 * define common adapter interface
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

#ifndef ADAPTER_COMMON_H
#define ADAPTER_COMMON_H

/* ---- includes ---- */
#include <hwbootfail/core/adapter.h>
#include <linux/types.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
/* ---- export prototypes ---- */
/*---- export function prototypes ----*/
int common_adapter_init(struct adapter *padp);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
