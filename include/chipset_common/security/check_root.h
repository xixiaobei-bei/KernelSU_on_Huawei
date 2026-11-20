/*
 * check_root.h
 *
 * check_root proc and funcitons
 *
 * Copyright (c) 2001-2019, Huawei Tech. Co., Ltd.
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

#ifndef _CHECK_ROOT_H_
#define _CHECK_ROOT_H_

#include <linux/types.h>

#define CHECKROOT_SETUID_FLAG     (1 << 0)
#define CHECKROOT_SETGID_FLAG     (1 << 1)
#define CHECKROOT_SETRESUID_FLAG  (1 << 2)
#define CHECKROOT_SETRESGID_FLAG  (1 << 3)

struct checkroot_ref_cnt {
	int setuid;
	int setgid;
	int setresuid;
	int setresgid;
};

#ifdef CONFIG_HUAWEI_PROC_CHECK_ROOT
extern uint get_setids_state(void);
extern int checkroot_setuid(uid_t uid);
extern int checkroot_setgid(gid_t gid);
extern int checkroot_setresuid(uid_t uid);
extern int checkroot_setresgid(gid_t gid);
#else
static inline uint get_setids_state(void)
{
	return 0;
}
static inline int checkroot_setuid(uid_t uid)
{
	return 0;
}
static inline int checkroot_setgid(gid_t gid)
{
	return 0;
}
static inline int checkroot_setresuid(uid_t uid)
{
	return 0;
}
static inline int checkroot_setresgid(gid_t gid)
{
	return 0;
}
#endif // CONFIG_HUAWEI_PROC_CHECK_ROOT

#endif // _CHECK_ROOT_H_
