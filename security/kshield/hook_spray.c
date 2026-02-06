/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: the hook_spray.c implements the thread count code
 * Author: ducuixia <ducuixia@huawei.com>
 * Create: 2019-11-09
 */

#include "hook_spray.h"
#include <linux/cred.h>
#include <linux/fs.h>
#include <linux/rbtree.h>
#include <linux/rculist.h>
#include <linux/sched/user.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "hook_charact.h"

struct spray_t {
	struct ks_spray_meta *meta;
	bool enable;
	spinlock_t io_lock;
};

static struct spray_t *ks_spray __ro_after_init;

void sub_chk_heap_spray(unsigned int count)
{
	unsigned long spray_cnt;
	unsigned long dead_tick;
	struct ks_spray_meta *meta = NULL;
	struct user_struct *user = NULL;

	if (!ks_spray->enable)
		return;

	meta = ks_spray->meta;
	user = get_current_user();
	spray_cnt = atomic64_add_return(count, &user->spray_count);

	if (unlikely(!user->spray_tick)) {
		user->spray_tick = jiffies;
		goto free_uid;
	}

	if (likely(spray_cnt < meta->threshold))
		goto free_uid;

	dead_tick = user->spray_tick + meta->spray_window;
	user->spray_tick = jiffies;
	atomic64_set(&user->spray_count, count);

	if (unlikely(time_before(jiffies, dead_tick))) {
		if (kshield_chk_suspect(HOOK_CHA_HEAP_SPRAY))
			ks_debug("Spray uid: %d\n", user->uid.val);
	}
free_uid:
	free_uid(user);
}

int ks_enable_heap_spray(bool enable)
{
	if (enable == ks_spray->enable)
		return 0;
	if (enable) {
		spin_lock(&ks_spray->io_lock);
		if (!ks_spray->meta) {
			spin_unlock(&ks_spray->io_lock);
			return -EINVAL;
		}
		spin_unlock(&ks_spray->io_lock);
	}

	ks_spray->enable = enable;
	ks_debug("%s spray checking\n", enable ? "Enable" : "Disable");
	return 0;
}

int ks_heap_spray_update(struct spray_meta meta)
{
	spin_lock(&ks_spray->io_lock);
	ks_spray->meta->spray_window = msecs_to_jiffies(meta.spray_ms);
	ks_spray->meta->threshold = meta.threshold;
	spin_unlock(&ks_spray->io_lock);
	return 0;
}

int hook_spray_init(void)
{
	ks_spray = kzalloc(sizeof(*ks_spray), GFP_KERNEL);
	if (!ks_spray)
		return -ENOMEM;
	ks_spray->meta = kzalloc(sizeof(struct ks_spray_meta), GFP_KERNEL);
	if (!ks_spray->meta) {
		kfree(ks_spray);
		ks_spray = NULL;
		return -ENOMEM;
	}

	spin_lock_init(&ks_spray->io_lock);
	return 0;
}

void hook_spray_deinit(void)
{
	if (ks_spray == NULL)
		return;

	if (ks_spray->meta) {
		kfree(ks_spray->meta);
		ks_spray->meta = NULL;
	}
	kfree(ks_spray);
	ks_spray = NULL;
}
