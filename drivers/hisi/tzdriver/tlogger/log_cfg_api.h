/*
 * log_cfg_api.h
 *
 * for log cfg api define
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#ifndef LOG_CFG_API_H
#define LOG_CFG_API_H

#include <linux/types.h>

#ifdef CONFIG_TEELOG
int register_log_mem(u64 *addr, u32 *len);
int register_log_exception(void);
void report_log_system_error(void);
int *map_log_mem(u64 mem_addr, u32 mem_len);
void unmap_log_mem(int *log_buffer);
void get_log_chown(uid_t *user, gid_t *group);
void unregister_log_exception(void);
void ta_crash_report_log(void);
#else
static inline int register_log_mem(const u64 *addr, const u32 *len)
{
	(void)addr;
	(void)len;
	return 0;
}

static inline int register_log_exception(void)
{
	return 0;
}

void report_log_system_error(void)
{
}

int *map_log_mem(u64 mem_addr, u32 mem_len)
{
	(void)mem_addr;
	(void)mem_len;
	return NULL;
}
void unmap_log_mem(const int *log_buffer)
{
	(void)log_buffer;
}

void get_log_chown(const uid_t *user, const gid_t *group)
{
	(void)user;
	(void)group;
}
void unregister_log_exception(void)
{
}

void ta_crash_report_log(void)
{
}
#endif
#endif
