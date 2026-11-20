/*
 * client-ref.h
 *
 * utilityies for hifi-usb client
 *
 * Copyright (c) 2017-2019 Huawei Technologies Co., Ltd.
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

#ifndef _CLIENT_REF_H_
#define _CLIENT_REF_H_

#include <linux/completion.h>
#include <linux/spinlock.h>

#define CLIENT_REF_KILL_SYNC_TIMEOUT (5 * HZ)

struct client_ref {
	unsigned long count;
	int live;
	spinlock_t lock;
	struct completion kill_completion;
	void (*release)(struct client_ref *);
};

void client_ref_put(struct client_ref *ref);
bool client_ref_tryget_live(struct client_ref *ref);
unsigned long client_ref_read(struct client_ref *ref);
bool client_ref_is_zero(struct client_ref *ref);
void client_ref_kill(struct client_ref *ref);
int client_ref_kill_sync(struct client_ref *client_ref);
void client_ref_release(struct client_ref *ref);
void client_ref_start(struct client_ref *ref, void (*release)(struct client_ref *));
void client_ref_init(struct client_ref *ref);
#endif /* _CLIENT_REF_H_ */
