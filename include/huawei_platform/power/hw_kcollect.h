/*
 * hw_kcollect.h
 *
 * This file use to collect kernel state and report it
 *
 * Copyright (c) 2014-2020 Huawei Technologies Co., Ltd.
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

#ifndef _HW_KCOLLECT_H
#define _HW_KCOLLECT_H

#define KCOLLECT_BUFFER_SIZE			(256)
#define KCOLLECT_CLOSE_ALL_MASK			(0)

/* edc count */
#define KCOLLECT_FPS_MASK				(1 << 0)
/* reserve */
#define KCOLLECT_LOG_MASK				(1 << 1)
/* suspend, resume, wake src */
#define KCOLLECT_SUSPEND_MASK			(1 << 2)

#define KCOLLECT_FREEZER_MASK			(1 << 3)
#define KCOLLECT_BINDERCALL_MASK		(1 << 4)
#define KCOLLECT_NETPACKET_MASK			(1 << 5)
#define KCOLLECT_ALL_MASK				(0x7fffffff)

int kcollect(int mask, const char *fmt, ...);
int hwkillinfo(int pid, int seg);
int hwbinderinfo(int callingpid, int calledpid);
int hw_packet_cb(int uid, int pid);
#endif