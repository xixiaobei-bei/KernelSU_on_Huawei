/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 * Description: the hook_sock_spray.c implements the thread count code
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include "hook_sock_spray.h"
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/rbtree.h>
#include <linux/uaccess.h>
#include <linux/rculist.h>
#include <linux/sched/user.h>
#include <linux/cred.h>
#include "hook_charact.h"

struct sock_spray_t {
	struct ks_spray_meta *meta;
	bool enable;
	spinlock_t io_lock;
};

static struct sock_spray_t *ks_sock_spray __ro_after_init;

void sub_chk_sock_spray(unsigned int count)
{
	unsigned long spray_cnt;
	unsigned long dead_tick;
	struct ks_spray_meta *meta = NULL;
	struct user_struct *user = NULL;

	if (!ks_sock_spray->enable)
		return;

	meta = ks_sock_spray->meta;
	user = get_current_user();
	spray_cnt = atomic64_add_return(count, &user->sock_spray_count);

	if (unlikely(!user->sock_spray_tick)) {
		user->sock_spray_tick = jiffies;
		goto free_uid;
	}

	if (likely(spray_cnt < meta->threshold))
		goto free_uid;

	ks_debug("Sock spray in sub chk sock spray\n");

	dead_tick = user->sock_spray_tick + meta->spray_window;
	user->sock_spray_tick = jiffies;
	atomic64_set(&user->sock_spray_count, count);

	if (unlikely(time_before(jiffies, dead_tick))) {
		if (kshield_chk_suspect(HOOK_CHA_SOCK_SPRAY))
			ks_debug("Spray socket uid: %d\n", user->uid.val);
	}

free_uid:
	free_uid(user);
}

int ks_enable_sock_spray(bool enable)
{
	if (enable == ks_sock_spray->enable)
		return 0;
	if (enable) {
		spin_lock(&ks_sock_spray->io_lock);
		if (!ks_sock_spray->meta) {
			spin_unlock(&ks_sock_spray->io_lock);
			return -EINVAL;
		}
		spin_unlock(&ks_sock_spray->io_lock);
	}

	ks_sock_spray->enable = enable;
	ks_debug("%s sock spray checking\n", enable ? "Enable" : "Disable");
	return 0;
}

int ks_sock_spray_update(struct spray_meta meta)
{
	spin_lock(&ks_sock_spray->io_lock);
	ks_sock_spray->meta->spray_window = msecs_to_jiffies(meta.spray_ms);
	ks_sock_spray->meta->threshold = meta.threshold;
	spin_unlock(&ks_sock_spray->io_lock);
	return 0;
}

int hook_sock_spray_init(void)
{
	ks_sock_spray = kzalloc(sizeof(*ks_sock_spray), GFP_KERNEL);
	if (!ks_sock_spray)
		return -ENOMEM;

	ks_sock_spray->meta = kzalloc(sizeof(struct ks_spray_meta), GFP_KERNEL);
	if (!ks_sock_spray->meta) {
		kfree(ks_sock_spray);
		ks_sock_spray = NULL;
		return -ENOMEM;
	}

	spin_lock_init(&ks_sock_spray->io_lock);
	return 0;
}

void hook_sock_spray_deinit(void)
{
	if (ks_sock_spray == NULL)
		return;

	if (ks_sock_spray->meta) {
		kfree(ks_sock_spray->meta);
		ks_sock_spray->meta = NULL;
	}
	kfree(ks_sock_spray);
	ks_sock_spray = NULL;
}
