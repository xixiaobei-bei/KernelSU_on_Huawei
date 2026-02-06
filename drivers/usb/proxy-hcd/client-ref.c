/*
 * client-ref.c
 *
 * utilityies for hifi-usb client
 *
 * Copyright (c) 2019-2019 Huawei Technologies Co., Ltd.
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

#include "client-ref.h"

#include <linux/completion.h>
#include <linux/errno.h>
#include <linux/spinlock.h>
#include <linux/usb/ch11.h>
#include <linux/usb/ch9.h>

void client_ref_put(struct client_ref *ref)
{
	unsigned long flags;
	unsigned long count;

	if (ref == NULL)
		return;

	spin_lock_irqsave(&ref->lock, flags);
	ref->count--;
	count = ref->count;
	spin_unlock_irqrestore(&ref->lock, flags);
	if ((count == 0) && (ref->release != NULL))
		ref->release(ref);
}

bool client_ref_tryget_live(struct client_ref *ref)
{
	unsigned long flags;

	if (ref == NULL)
		return false;

	spin_lock_irqsave(&ref->lock, flags);
	if (ref->live >= 1) {
		ref->count++;
		spin_unlock_irqrestore(&ref->lock, flags);
		return true;
	}

	spin_unlock_irqrestore(&ref->lock, flags);
	return false;
}

unsigned long client_ref_read(struct client_ref *ref)
{
	unsigned long flags;
	unsigned long count;

	if (ref == NULL)
		return 0;

	spin_lock_irqsave(&ref->lock, flags);
	count = ref->count;
	spin_unlock_irqrestore(&ref->lock, flags);

	return count;
}

bool client_ref_is_zero(struct client_ref *ref)
{
	if (ref == NULL)
		return false;

	return (client_ref_read(ref) == 0);
}

void client_ref_kill(struct client_ref *ref)
{
	unsigned long flags;

	if (ref == NULL)
		return;

	spin_lock_irqsave(&ref->lock, flags);
	ref->live--;
	ref->count--;
	if (ref->count == 0) {
		spin_unlock_irqrestore(&ref->lock, flags);
		if (ref->release != NULL)
			ref->release(ref);
		return;
	}
	spin_unlock_irqrestore(&ref->lock, flags);
}

/*
 * return 0 on sucess, others failed.
 */
int client_ref_kill_sync(struct client_ref *client_ref)
{
	if (client_ref == NULL)
		return -ENODEV;

	init_completion(&client_ref->kill_completion);
	client_ref_kill(client_ref);
	if (wait_for_completion_timeout(&client_ref->kill_completion, CLIENT_REF_KILL_SYNC_TIMEOUT) == 0) {
		pr_err("[%s]wait for client killed timeout\n", __func__);
		WARN_ON(1);
		return -ETIME;
	}

	WARN_ON(!client_ref_is_zero(client_ref));

	return 0;
}

void client_ref_release(struct client_ref *ref)
{
	if (ref == NULL)
		return;

	complete(&ref->kill_completion);
}

void client_ref_start(struct client_ref *ref,
		void (*release)(struct client_ref *))
{
	unsigned long flags;

	if ((ref == NULL) || (release == NULL))
		return;

	spin_lock_irqsave(&ref->lock, flags);
	ref->count = 1UL;
	ref->live = 1;
	ref->release = release;
	spin_unlock_irqrestore(&ref->lock, flags);
}

void client_ref_init(struct client_ref *ref)
{
	if (ref == NULL)
		return;

	spin_lock_init(&ref->lock);
	ref->count = 0UL;
	ref->live = 0;
	ref->release = NULL;
}

