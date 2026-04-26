/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: the hook_charact.c implements the thread count code
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include "hook_charact.h"
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/sched/user.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "event.h"

struct ks_cha_threshold {
	int process_max;
	int futex_waiters;
	unsigned long clear_tick;
};

struct ks_cha_weight {
	unsigned int threshold;
	unsigned int weight[HOOK_CHA_MAX - HOOK_CHA_START];
};

struct cha_t {
	bool ks_cha_enable;
	struct ks_cha_threshold ks_thre;
	struct ks_cha_weight ks_wei;
};

#define EXTEND_INFO_LEN 200

static struct cha_t *ks_cha __ro_after_init;

static inline unsigned int get_ks_weight(enum hook_id id)
{
	return ks_cha->ks_wei.weight[id - HOOK_CHA_START];
}

static bool need_report(struct user_struct *user, unsigned int hook_id)
{
	unsigned int hook_flag;

	hook_flag = (1 << (hook_id - HOOK_CHA_START));

	if (!(user->kshield_flags & hook_flag)) {
		user->kshield_flags |= hook_flag;
		user->kshield_weight += get_ks_weight(hook_id);
		ks_debug("[kshield] User-%d has hook %u, current weight is %d\n",
			user->uid.val, hook_id, user->kshield_weight);
	}

	if ((user->kshield_weight >= ks_cha->ks_wei.threshold) &&
		!user->kshield_cha_reported) {
		user->kshield_cha_reported = true;
		ks_debug("[kshield] User-%d current weight %d >= threshold %d\n",
			user->uid.val, user->kshield_weight, ks_cha->ks_wei.threshold);
		return true;
	}
	return false;
}

static void update_user_tick(struct user_struct *user, unsigned long tick)
{
	unsigned long update_tick;
	/*
	 * clear_tick is 0 means DO NOT clear the flag and weight of user
	 * and they will be kept until user structure being destroyed
	 */
	if (!ks_cha->ks_thre.clear_tick)
		return;
	if (user->last_mod_tick) {
		update_tick = user->last_mod_tick + ks_cha->ks_thre.clear_tick;
		if (!time_before(tick, update_tick)) {
			user->kshield_flags = 0;
			user->kshield_weight = 0;
		}
	}
	user->last_mod_tick = tick;
}

static int init_cha_extend_info(char *extend_info, struct user_struct *user,
			unsigned int size)
{
	int ret = snprintf_s(extend_info, size, size - 1,
		"user_w:%u threshold:%u heap:%u sock:%u userfaultfd:%u kinfo:%u mlock:%u affinity:%u futex:%u tnum:%u warn:%u",
		user->kshield_weight,
		ks_cha->ks_wei.threshold,
		get_ks_weight(HOOK_CHA_HEAP_SPRAY),
		get_ks_weight(HOOK_CHA_SOCK_SPRAY),
		get_ks_weight(HOOK_CHA_USERFAULTFD),
		get_ks_weight(HOOK_CHA_KINFO),
		get_ks_weight(HOOK_CHA_MLOCK),
		get_ks_weight(HOOK_CHA_SET_AFFINITY),
		get_ks_weight(HOOK_CHA_FUTEX_WAKE),
		get_ks_weight(HOOK_CHA_PROCESS_NUM),
		get_ks_weight(HOOK_CHA_WARN));
	if (ret == -1) {
		ks_debug("[kshield] init cha extend info snprintf_s error\n");
		return -1;
	}
	return 0;
}

bool kshield_chk_suspect(unsigned int hook_id)
{
	kuid_t uid;
	struct user_struct *user = NULL;
	unsigned long cur_tick;
	char extend_info[EXTEND_INFO_LEN];
	int ret;

	user = get_current_user();
	uid = user->uid;
	cur_tick = jiffies;

	if (unlikely(in_interrupt())) {
		ks_debug("Detect in irq 0x%x\n", hook_id);
		goto ret_false;
	}

	update_user_tick(user, cur_tick);

	if (!need_report(user, hook_id))
		goto ret_false;
	ret = init_cha_extend_info(extend_info, user, sizeof(extend_info));
	if (ret != 0)
		goto ret_false;
	gen_cha_event(hook_id, 0, cur_tick, extend_info);
	free_uid(user);
	return true;
ret_false:
	free_uid(user);
	return false;
}

#ifdef CONFIG_USERFAULTFD
void sub_chk_userfaultfd(void)
{
	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	if (kshield_chk_suspect(HOOK_CHA_USERFAULTFD))
		ks_debug("Detect userfaultfd");
}
#endif

void sub_chk_kinfo(void)
{
	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	if (!strncmp(current->comm, "main", strlen("main")) &&
		!strncmp(current->real_parent->comm, "main", strlen("main")))
		return;
	if (kshield_chk_suspect(HOOK_CHA_KINFO))
		ks_debug("Detect kernel info");
}

void sub_chk_mlock(void)
{
	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	if (kshield_chk_suspect(HOOK_CHA_MLOCK))
		ks_debug("Detect mlock");
}

void sub_chk_set_affinity(void)
{
	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	if (kshield_chk_suspect(HOOK_CHA_SET_AFFINITY))
		ks_debug("Detect set_affinity");
}

void sub_chk_futex(int nr_waiters)
{
	int futex_waiters;

	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	futex_waiters = ks_cha->ks_thre.futex_waiters;
	if (!futex_waiters || nr_waiters < futex_waiters)
		return;
	if (kshield_chk_suspect(HOOK_CHA_FUTEX_WAKE))
		ks_debug("Detect futex wake");
}

void sub_chk_process_num(void)
{
	int processes;
	int process_max;
	struct user_struct *user = NULL;

	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;

	process_max = ks_cha->ks_thre.process_max;
	if (!process_max)
		return;

	user = get_current_user();
	processes = atomic_read(&user->processes);
	free_uid(user);
	if (processes < process_max)
		return;

	if (kshield_chk_suspect(HOOK_CHA_PROCESS_NUM))
		ks_debug("User-%d process num reaches threshold",
			 user->uid.val);
}

void sub_chk_warn(void)
{
	if (!ks_cha || !ks_cha->ks_cha_enable)
		return;
	if (kshield_chk_suspect(HOOK_CHA_WARN))
		ks_debug("Detect warn");
}

int hook_cha_enable(bool enable)
{
	if (!ks_cha)
		return -ENOMEM;

	ks_cha->ks_cha_enable = enable;
	return 0;
}

static inline void set_ks_weight(enum hook_id id, unsigned int value)
{
	ks_cha->ks_wei.weight[id - HOOK_CHA_START] = value;
}

int hook_cha_update(struct cha_conf_meta meta)
{
	if (!ks_cha)
		return -ENOMEM;

	set_ks_weight(HOOK_CHA_HEAP_SPRAY, meta.wmeta.w_heap_spray);
	set_ks_weight(HOOK_CHA_SOCK_SPRAY, meta.wmeta.w_sock_spray);
	set_ks_weight(HOOK_CHA_USERFAULTFD, meta.wmeta.w_userfaultfd);
	set_ks_weight(HOOK_CHA_KINFO, meta.wmeta.w_kinfo);
	set_ks_weight(HOOK_CHA_MLOCK, meta.wmeta.w_mlock);
	set_ks_weight(HOOK_CHA_SET_AFFINITY, meta.wmeta.w_affinity);
	set_ks_weight(HOOK_CHA_FUTEX_WAKE, meta.wmeta.w_futex);
	set_ks_weight(HOOK_CHA_PROCESS_NUM, meta.wmeta.w_tnum);
	set_ks_weight(HOOK_CHA_WARN, meta.wmeta.w_warn);
	ks_cha->ks_wei.threshold = meta.wmeta.threshold;
	ks_cha->ks_thre.futex_waiters = (int)meta.tmeta.futex_waiters;
	ks_cha->ks_thre.process_max = (int)meta.tmeta.process_max;
	ks_cha->ks_thre.clear_tick = msecs_to_jiffies(meta.tmeta.clear_ms);
	return 0;
}

int hook_cha_init(void)
{
	ks_cha = kzalloc(sizeof(*ks_cha), GFP_KERNEL);
	if (!ks_cha)
		return -ENOMEM;
	return 0;
}

void hook_cha_deinit(void)
{
	if (ks_cha == NULL)
		return;

	kfree(ks_cha);
	ks_cha = NULL;
}
