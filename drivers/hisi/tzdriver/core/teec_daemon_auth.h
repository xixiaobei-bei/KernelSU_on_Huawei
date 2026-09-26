/*
 * teec_daemon_auth.h
 *
 * function definition for teecd or hidl process auth
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
#ifndef TEEC_DAEMON_AUTH_H
#define TEEC_DAEMON_AUTH_H

#ifdef CONFIG_TEECD_AUTH
#include <linux/sched.h>
#include "auth_base_impl.h"

int check_teecd_access(void);

#else

static inline int check_teecd_access(void)
{
	return 0;
}

#endif

#endif
