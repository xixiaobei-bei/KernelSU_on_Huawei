/*
 * frame_timer.h
 *
 * frame freq timer header
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#ifndef FRAME_TIMER_H
#define FRAME_TIMER_H

#include <linux/cred.h>

void init_frame_timer(void);
void deinit_frame_timer(void);
void start_boost_timer(u32 duration, u32 min_util);

#endif

