/*
 * hwqos_sysctl.h
 *
 * Qos schedule declaration
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#ifndef HWQOS_SYSCTL_H
#define HWQOS_SYSCTL_H

extern unsigned int g_sysctl_qos_sched;
#define QOS_SCHED_SET_ENABLE (g_sysctl_qos_sched & 0x0001)
#define QOS_SCHED_ENQUEUE_ENABLE (g_sysctl_qos_sched & 0x0002)
#define RTG_TRANS_ENABLE (g_sysctl_qos_sched & 0x0004)
#define QOS_SCHED_LOCK_ENABLE (g_sysctl_qos_sched & 0x0008)
#define BINDER_SCHED_ENABLE (g_sysctl_qos_sched & 0x0010)
#define BINDER_SCHED_DEBUG_ENABLE (g_sysctl_qos_sched & 0x0020)
#define BINDER_SET_PRIO_ENABLE (g_sysctl_qos_sched & 0x0040)
#define VIP_PRIO_SET_ENABLE (g_sysctl_qos_sched & 0x0080)
#define VIP_PRIO_TRANS_ENABLE (g_sysctl_qos_sched & 0x0100)
#define MIN_UTIL_SET_ENABLE (g_sysctl_qos_sched & 0x0200)
#define MIN_UTIL_TRANS_ENABLE (g_sysctl_qos_sched & 0x0400)
#endif
