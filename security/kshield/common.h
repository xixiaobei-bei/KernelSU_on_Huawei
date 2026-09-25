/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the common.h provides common data structure for the kshield module
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#ifndef _KSHIELD_COMMON_H_
#define _KSHIELD_COMMON_H_

#include <linux/types.h>
#include <securec.h>

#define KS_DEV_NAME "kshield"

#ifdef CONFIG_KSHIELD_TEST
#define ks_debug(fmt, args...) \
	pr_info("[%s] "fmt, KS_DEV_NAME, ##args)
#else
#define ks_debug(fmt, args...)
#endif

#define ks_info(fmt, args...) \
	pr_info("[%s] "fmt, KS_DEV_NAME, ##args)
#define ks_err(fmt, args...) \
	pr_err("[%s] "fmt, KS_DEV_NAME, ##args)

#define HOOK_TRACE_START 0
#define HOOK_EXP_START 1000
#define HOOK_CHA_START 2000

enum kshield_ioctl_cmd {
	KS_START_INSPECTION = 1,
	KS_STOP_INSPECTION,
	KS_CHA_ALL_ENABLE,
	KS_CHA_CONFIG_UPDATE,
};

enum hook_id {
	/* Hooks hold rules */
	HOOK_TRACE_SYSCALL = HOOK_TRACE_START,
	/* Test only */
	HOOK_TEST,
	HOOK_HOLD_RULES,

	HOOK_TRACE_PROC,

	/* Exploit Methods */
	HOOK_EXP_CHG_DOMAIN = HOOK_EXP_START,
	HOOK_EXP_CHG_MEM,
	HOOK_EXP_FOPS,
	HOOK_EXP_IOVEC,
	HOOK_EXP_LOAD_POLICY,
	HOOK_EXP_REG_SYSCTL,
	HOOK_EXP_RET2DIR,
	HOOK_EXP_SEC_EXEC,
	HOOK_EXP_MAX,

	/* Exploit Characteristic */
	HOOK_CHA_HEAP_SPRAY = HOOK_CHA_START,
	HOOK_CHA_SOCK_SPRAY,
	HOOK_CHA_USERFAULTFD,
	HOOK_CHA_KINFO,
	HOOK_CHA_MLOCK,
	HOOK_CHA_SET_AFFINITY,
	HOOK_CHA_FUTEX_WAKE,
	HOOK_CHA_PROCESS_NUM,
	HOOK_CHA_WARN,
	HOOK_CHA_MAX,
};

enum action {
	PERMISSIVE = 0,
	FREEZE,
	KILL,
	IGNORE,
};

struct spray_meta {
	unsigned int spray_ms; /* millisecond */
	unsigned int threshold;
};

struct weight_meta {
	unsigned int threshold;
	unsigned int w_heap_spray;
	unsigned int w_sock_spray;
	unsigned int w_userfaultfd;
	unsigned int w_kinfo;
	unsigned int w_mlock;
	unsigned int w_affinity;
	unsigned int w_futex;
	unsigned int w_tnum;
	unsigned int w_warn;
};

struct ks_spray_meta {
	unsigned long spray_window; /* jiffies */
	unsigned int threshold;
};

struct thres_meta {
	struct spray_meta heap_spray;
	struct spray_meta sock_spray;
	unsigned int futex_waiters;
	unsigned int process_max;
	unsigned int clear_ms; /* millisecond*/
};

struct cha_conf_meta {
	struct thres_meta tmeta;
	struct weight_meta wmeta;
};

#endif
