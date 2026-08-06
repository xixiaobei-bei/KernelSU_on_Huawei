/*
 * erecovery_transtation.h
 *
 * This file is header file for erecovery transtation
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#ifndef __ERECOVERY_TRANSTATION_H_
#define __ERECOVERY_TRANSTATION_H_

#include <chipset_common/hwerecovery/erecovery.h>
#include <erecovery_common.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ERECOVERY_TOTAL_BUF_SIZE (8 * 1024)

long erecovery_write_event_user(const void __user *argp);
long erecovery_write_event_kernel(void *argp);
long erecovery_read_event(void __user *argp);

#ifdef __cplusplus
}
#endif
#endif
