/*
 * multi_frame_info.c
 *
 * multi frame rtg manager
 *
 * Copyright (c) 2019-2020 Huawei Technologies Co., Ltd.
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

#include "frame.h"
#include <linux/atomic.h>
#include <linux/kernel.h>
#include <linux/printk.h>
#include <linux/cred.h>
#include <linux/sched/task.h>
#include <../kernel/sched/sched.h>
#include <hwrtg/proc_state.h>
#include <trace/events/sched.h>

static struct multi_frame_id_manager g_id_manager = {
	.id_map = {0},
	.offset = 0,
	.lock = __RW_LOCK_UNLOCKED(g_id_manager.lock)
};

static struct frame_info g_multi_frame_info[MULTI_FRAME_NUM];

static int alloc_rtg_id(void)
{
	unsigned int id_offset;
	int id;

	write_lock(&g_id_manager.lock);
	id_offset = find_next_zero_bit(g_id_manager.id_map, MULTI_FRAME_NUM,
				       g_id_manager.offset);
	if (id_offset >= MULTI_FRAME_NUM) {
		id_offset = find_first_zero_bit(g_id_manager.id_map,
						MULTI_FRAME_NUM);
		if (id_offset >= MULTI_FRAME_NUM) {
			write_unlock(&g_id_manager.lock);
			return -NO_FREE_MULTI_FRAME;
		}
	}

	set_bit(id_offset, g_id_manager.id_map);
	g_id_manager.offset = id_offset;
	id = id_offset + MULTI_FRAME_ID;
	write_unlock(&g_id_manager.lock);

	pr_debug("[MULTI_FRAME] %s id_offset=%u, id=%d\n", __func__, id_offset, id);
	return id;
}

static void free_rtg_id(int id)
{
	unsigned int id_offset = id - MULTI_FRAME_ID;

	if (id_offset >= MULTI_FRAME_NUM) {
		pr_err("[MULTI_FRAME] %s id_offset is invalid, id=%d, id_offset=%u.\n", __func__, id, id_offset);
		return;
	}

	pr_debug("[MULTI_FRAME] %s id=%d id_offset=%u\n", __func__, id, id_offset);
	write_lock(&g_id_manager.lock);
	clear_bit(id_offset, g_id_manager.id_map);
	write_unlock(&g_id_manager.lock);
}

int alloc_multi_frame_info(void)
{
	struct frame_info *frame_info = NULL;
	int id;

	id = alloc_rtg_id();
	if (id < 0)
		return id;

	frame_info = rtg_frame_info(id);
	if (!frame_info) {
		free_rtg_id(id);
		return -INVALID_ARG;
	}

	set_frame_rate(frame_info, DEFAULT_FRAME_RATE);
	atomic_set(&frame_info->curr_rt_thread_num, 0);
	atomic_set(&frame_info->max_rt_thread_num, DEFAULT_MAX_RT_THREAD);

	return id;
}

void clear_multi_frame_info(void)
{
	write_lock(&g_id_manager.lock);
	bitmap_zero(g_id_manager.id_map, MULTI_FRAME_NUM);
	g_id_manager.offset = 0;
	write_unlock(&g_id_manager.lock);
}

void release_multi_frame_info(int id)
{
	if ((id < MULTI_FRAME_ID) || (id >= MULTI_FRAME_ID + MULTI_FRAME_NUM)) {
		pr_err("[MULTI_FRAME] %s frame(id=%d) not found.\n", __func__, id);
		return;
	}

	read_lock(&g_id_manager.lock);
	if (!test_bit(id - MULTI_FRAME_ID, g_id_manager.id_map)) {
		read_unlock(&g_id_manager.lock);
		return;
	}
	read_unlock(&g_id_manager.lock);

	pr_debug("[MULTI_FRAME] %s release frame(id=%d).\n", __func__, id);
	free_rtg_id(id);
}

struct frame_info *rtg_multi_frame_info(int id)
{
	if ((id < MULTI_FRAME_ID) || (id >= MULTI_FRAME_ID + MULTI_FRAME_NUM))
		return NULL;

	return &g_multi_frame_info[id - MULTI_FRAME_ID];
}

struct frame_info *rtg_active_multi_frame_info(int id)
{
	struct frame_info *frame_info = NULL;

	if ((id < MULTI_FRAME_ID) || (id >= MULTI_FRAME_ID + MULTI_FRAME_NUM))
		return NULL;

	read_lock(&g_id_manager.lock);
	if (test_bit(id - MULTI_FRAME_ID, g_id_manager.id_map))
		frame_info = &g_multi_frame_info[id - MULTI_FRAME_ID];
	read_unlock(&g_id_manager.lock);
	if (!frame_info)
		pr_debug("[MULTI_FRAME] %s frame %d has been released\n", __func__, id);

	return frame_info;
}
