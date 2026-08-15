/*
 * erecovery_transtation.c
 *
 * kernel process of erecovery event
 *
 * Copyright (c) 2018-2019 Huawei Technologies Co., Ltd.
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

#include "erecovery_transtation.h"

#include <linux/sched.h>
#include <linux/module.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/vmalloc.h>
#include <linux/aio.h>
#include <uapi/linux/uio.h>
#include <asm/ioctls.h>
#include <linux/errno.h>

#include <chipset_common/hwerecovery/erecovery.h>
#include <erecovery_common.h>

#ifdef GTEST
#define static
#define inline
#endif

#define ERECOVERY_EVENT_SIZE sizeof(struct erecovery_write_event)
#define ERECOVERY_RING_BUF_SIZE_MAX (ERECOVERY_TOTAL_BUF_SIZE - ERECOVERY_EVENT_SIZE)

struct erecovery_trans_pos {
	uint32_t w;
	uint32_t r;
};

static DEFINE_MUTEX(erecovery_mutex);
static wait_queue_head_t erecovery_wq;
static char *erecovery_trans_buf;
static struct erecovery_trans_pos *erecovery_pos;

static inline uint32_t erecovery_get_pos(uint32_t pos, uint32_t len)
{
	uint32_t new_pos = pos + len;

	if (new_pos >= ERECOVERY_RING_BUF_SIZE_MAX)
		new_pos -= ERECOVERY_RING_BUF_SIZE_MAX;

	return new_pos;
}

static int erecovery_write_from_kernel(uint32_t pos, void *k_data, uint32_t len)
{
	uint32_t part_len;

	if ((k_data == NULL) || (pos >= ERECOVERY_RING_BUF_SIZE_MAX))
		return -1;

	part_len = min((size_t)len, (size_t)(ERECOVERY_RING_BUF_SIZE_MAX - pos));
	memcpy(erecovery_trans_buf + pos, k_data, part_len);
	if (part_len < len)
		memcpy(erecovery_trans_buf, k_data + part_len, len - part_len);

	return 0;
}

static inline int erecovery_is_full(uint32_t len)
{
	if (erecovery_pos->w > erecovery_pos->r)
		return (erecovery_pos->w + len - erecovery_pos->r) >= ERECOVERY_RING_BUF_SIZE_MAX;
	else if (erecovery_pos->w < erecovery_pos->r)
		return (erecovery_pos->w + len) >= erecovery_pos->r;
	else
		return 0;
}

static inline int erecovery_is_empty(void)
{
	return (erecovery_pos->w == erecovery_pos->r);
}

static int erecovery_read_one_event(void __user *u_data)
{
	uint32_t pos;
	uint32_t part_len;

	if (erecovery_is_empty()) {
		ERECOVERY_ERROR("buffer empty\n");
		return -1;
	}
	pos = erecovery_get_pos(erecovery_pos->r, 0);
	part_len = min(ERECOVERY_EVENT_SIZE, (size_t)(ERECOVERY_RING_BUF_SIZE_MAX - pos));
	if (copy_to_user(u_data, erecovery_trans_buf + pos, part_len)) {
		ERECOVERY_ERROR("copy_to_user error\n");
		return -1;
	}

	if (part_len < ERECOVERY_EVENT_SIZE) {
		if (copy_to_user(u_data + part_len, erecovery_trans_buf, ERECOVERY_EVENT_SIZE - part_len)) {
			ERECOVERY_ERROR("copy_to_user error\n");
			return -1;
		}
	}
	erecovery_pos->r = erecovery_get_pos(erecovery_pos->r, ERECOVERY_EVENT_SIZE);
	return 0;
}

static long erecovery_write_event_internal(void *kernel_event)
{
	int ret;
	struct erecovery_write_event *we = NULL;

	if (!kernel_event) {
		ERECOVERY_ERROR("param error\n");
		return -1;
	}
	we = kernel_event;
	if (mutex_lock_interruptible(&erecovery_mutex))
		return -1;

	if (erecovery_is_full(sizeof(*we))) {
		ERECOVERY_ERROR("erecovery buffer full");
		goto end;
	}
	ret = erecovery_write_from_kernel(erecovery_pos->w, (char *)we, sizeof(*we));
	if (ret)
		goto end;

	erecovery_pos->w = erecovery_get_pos(erecovery_pos->w, sizeof(*we));
	mutex_unlock(&erecovery_mutex);
	wake_up_interruptible(&erecovery_wq);
	return 0;
end:
	mutex_unlock(&erecovery_mutex);
	return -1;
}

long erecovery_write_event_user(const void __user *argp)
{
	struct erecovery_write_event *we = NULL;
	int ret;

	if (!argp) {
		ERECOVERY_ERROR("param error!");
		return -1;
	}
	we = vmalloc(ERECOVERY_EVENT_SIZE);
	if (!we)
		return -1;

	if (copy_from_user((void *)we, argp, sizeof(*we))) {
		ERECOVERY_ERROR("can not copy data from user space\n");
		vfree(we);
		return -1;
	}
	ret = erecovery_write_event_internal(we);
	vfree(we);
	return ret;
}

long erecovery_write_event_kernel(void *argp)
{
	return erecovery_write_event_internal(argp);
}

long erecovery_read_event(void __user *argp)
{
	if (!argp) {
		ERECOVERY_ERROR("param error\n");
		return -1;
	}
	if (mutex_lock_interruptible(&erecovery_mutex))
		return -1;

	if (erecovery_is_empty()) {
		mutex_unlock(&erecovery_mutex);
		wait_event_interruptible(erecovery_wq, erecovery_pos->w != erecovery_pos->r);
		if (mutex_lock_interruptible(&erecovery_mutex))
			return -1;
	}
	if (erecovery_read_one_event(argp)) {
		ERECOVERY_ERROR("etrans read buffer failed\n");
		mutex_unlock(&erecovery_mutex);
		return -1;
	}

	mutex_unlock(&erecovery_mutex);
	return 0;
}

static int __init erecovery_trans_init(void)
{
	int ret;

	erecovery_trans_buf = vmalloc(ERECOVERY_TOTAL_BUF_SIZE);
	if (!erecovery_trans_buf) {
		ERECOVERY_ERROR("transtion not init\n");
		ret = -ENOMEM;
		goto _error;
	}
	memset(erecovery_trans_buf, 0, ERECOVERY_TOTAL_BUF_SIZE);
	erecovery_pos = (struct erecovery_trans_pos *)(erecovery_trans_buf +
						       ERECOVERY_TOTAL_BUF_SIZE -
						       sizeof(*erecovery_pos));
	init_waitqueue_head(&erecovery_wq);
	return 0;

_error:
	if (erecovery_trans_buf) {
		vfree(erecovery_trans_buf);
		erecovery_trans_buf = NULL;
	}

	return ret;
}

static void __exit erecovery_trans_exit(void)
{
	if (erecovery_trans_buf) {
		vfree(erecovery_trans_buf);
		erecovery_trans_buf = NULL;
	}
}

module_init(erecovery_trans_init);
module_exit(erecovery_trans_exit);
MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("kernel process of erecovery event");
MODULE_AUTHOR("Huawei Technologies Co., Ltd.");
