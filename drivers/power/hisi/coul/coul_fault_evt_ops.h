/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: fault event handler function for coul module
 *
 * This software is licensed under the terms of the GNU General Public
 * License, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef _COUL_FAULT_EVT_OPS_H_
#define _COUL_FAULT_EVT_OPS_H_

#include "coul_core.h"

void coul_lock_init(void);
void coul_lock_trash(void);
void coul_fault_work(struct work_struct *work);

#endif
