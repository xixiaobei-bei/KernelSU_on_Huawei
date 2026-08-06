/*
 * zrhung_transtation.h
 *
 * This file is header file for zero hung transaction
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ZRHUNG_TRANSTATION_H_
#define __ZRHUNG_TRANSTATION_H_

#ifdef __cplusplus
extern "C" {
#endif

#define HTRANS_TOTAL_BUF_SIZE (64 * 1024)

long htrans_write_event(const void __user *argp);
long htrans_write_event_kernel(void *argp);
long htrans_read_event(void __user *argp);
long htrans_read_lastword(void __user *argp);

#ifdef __cplusplus
}
#endif
#endif /* __ZRHUNG_TRANSTATION_H_ */
