/*
 * saudit.c
 *
 * security audit log functions
 *
 * Copyright (C) 2019-2019, Huawei Tech. Co., Ltd.
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

#ifndef _SAUDIT_H_
#define _SAUDIT_H_

#include <chipset_common/security/hw_kernel_stp_interface.h>

#define SAUDIT_BUFSIZ 1024

struct saudit_buffer {
	struct stp_item item;
	char            info[SAUDIT_BUFSIZ];
	size_t          offset;
	unsigned int    flags;
	unsigned int    delay;
};

enum saudit_flags {
	SAUDIT_ASYNC   = (1 << 0),
	SAUDIT_ATOMIC  = (1 << 1),
	SAUDIT_TRACE   = (1 << 2),
	SAUDIT_REF     = (1 << 3),
};

#ifdef CONFIG_HUAWEI_PROC_CHECK_ROOT
extern struct saudit_buffer *saudit_log_start(unsigned int idx);
extern void saudit_log_end(struct saudit_buffer *sab);
extern void saudit_log_stack(struct saudit_buffer *sab);
extern __printf(2, 3)
void saudit_log_format(struct saudit_buffer *sab, const char *fmt, ...);
extern __printf(4, 5)
void saudit_log(int idx, int status, unsigned int flags, const char *fmt, ...);
#else
static inline struct saudit_buffer *saudit_log_start(unsigned int flags)
{
	return NULL;
}
static inline void saudit_log_end(struct saudit_buffer *sab) {}
static inline void saudit_log_stack(struct saudit_buffer *sab) {}
static inline void saudit_log_format(struct saudit_buffer *sab,
	const char *fmt, ...) {}
static inline void saudit_log(int idx, int status, unsigned int flags,
	const char *fmt, ...) {}
#endif // CONFIG_HUAWEI_PROC_CHECK_ROOT

#endif // _SAUDIT_H_
