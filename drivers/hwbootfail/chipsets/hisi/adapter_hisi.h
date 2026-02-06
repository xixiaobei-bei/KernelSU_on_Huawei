/*
 * copyright: Huawei Technologies Co., Ltd. 2016-2019. All rights reserved.
 *
 * file: adapter_hisi.h
 *
 * define adapter for HISI
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

#ifndef ADAPTER_HISI_H
#define ADAPTER_HISI_H

/* ---- includes ---- */
#include <hwbootfail/core/adapter.h>

/* ---- c++ support ---- */
#ifdef __cplusplus
extern "C" {
#endif

/* ---- export macroes ---- */
#define BFI_DEV_PATH "/dev/block/by-name/rrecord"

/* ---- export prototypes ---- */
/*---- export function prototypes ----*/
int hisi_adapter_init(struct adapter *padp);

/* ---- c++ support ---- */
#ifdef __cplusplus
}
#endif
#endif
