/*
 * Copyright (c) 2019-2023 Huawei Technologies Co., Ltd.
 * Description: define macros and structs for smartcare.c
 * Create: 2019-08-15
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */
#ifndef __NSTACK_H_INCLUDED__
#define __NSTACK_H_INCLUDED__

#include <net/sock.h>
#include <linux/types.h>

void nstack_event_process(int32_t event, uint8_t *pdata, uint16_t len);
void nstack_init(struct sock *nlfd);
void nstack_deinit(void);

#endif // __NSTACK_H_INCLUDED__