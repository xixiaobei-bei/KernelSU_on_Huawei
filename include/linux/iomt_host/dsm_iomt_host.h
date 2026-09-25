/*
 * file_name
 * dsm_iomt_host.h
 * description
 * stat io latency scatter at driver level,
 * show it in kernel log and host attr node.
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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

#ifndef __DSM_IOMT_HOST
#define __DSM_IOMT_HOST

#include <linux/workqueue.h>
#include <linux/sched.h>

/*
 * ifmodify IOMT_LATENCY_GAP_ARRAY_SIZE
 * must modify the format string in function iomt_latency_log_show
 */
#define IOMT_LATENCY_GAP_ARRAY_SIZE		12
#define IOMT_LATENCY_GAP_MID_IDX	\
	(IOMT_LATENCY_GAP_ARRAY_SIZE / 2)
#define IOMT_LATENCY_STAT_ARRAY_SIZE		\
	(IOMT_LATENCY_GAP_ARRAY_SIZE + 1)

#define IOMT_LATENCY_INVALID_INDEX		\
	IOMT_LATENCY_STAT_ARRAY_SIZE

#define IOMT_LATENCY_STAT_RING_BUFFER_SIZE	12
#define IOMT_STAT_RING_SIZE			\
	IOMT_LATENCY_STAT_RING_BUFFER_SIZE
#define IOMT_LATENCY_SCATTER_TIMER_INTERVAL	(10 * HZ)
#define IOMT_IO_TIMEOUT_DSM_JUDGE_SLOT	\
	(10 % IOMT_LATENCY_STAT_ARRAY_SIZE)
#define IOMT_LATENCY_LOG_SETTING_MASK	10
#define IOMT_LATENCY_LOG_SETTING_ENABLE	1
#define IOMT_LATENCY_LOG_SETTING_DISABLE	0
#define IOMT_IO_TIMEOUT_DSM_SETTING_MASK	100

#define IOMT_OP_BLOCK_SHIFT		16

#define IOMT_DIR_READ	0
#define IOMT_DIR_WRITE	1
#define IOMT_DIR_OTHER	2

typedef void (*iomt_dsm_func)(void *iomt_host_info);
typedef void (*iomt_create_dev_attr_func)(void *iomt_host_info);
typedef void (*iomt_delete_dev_attr_func)(void *iomt_host_info);
typedef void *(*iomt_host_to_info_func)(void *host);
typedef char *(*iomt_host_name_func)(void *host);
typedef unsigned int (*iomt_host_blksz_func)(void *host);

typedef void (*iomt_reinit_gap_array_func)(
		unsigned long *array, unsigned long array_size);

struct iomt_host_ops {
	iomt_host_to_info_func host_to_iomt_func;
	iomt_host_name_func host_name_func;
	iomt_create_dev_attr_func create_latency_attr_func;
	iomt_delete_dev_attr_func delete_latency_attr_func;
	iomt_reinit_gap_array_func reinit_latency_gap_array_func;
	iomt_dsm_func dsm_func;
	iomt_host_blksz_func host_blk_size_fun;
	iomt_create_dev_attr_func create_rw_size_attr_func;
	iomt_delete_dev_attr_func delete_rw_size_attr_func;
	iomt_reinit_gap_array_func reinit_blk_size_gap_array_func;
};

struct iomt_latency_stat_element {
	unsigned long read_count;
	unsigned long write_count;
	unsigned long other_count;
};

struct iomt_host_latency_stat_ring_node {
	unsigned long stat_tick;
	struct iomt_latency_stat_element
		stat_array[IOMT_LATENCY_STAT_ARRAY_SIZE];
};

struct iomt_host_latency_stat {
	struct delayed_work latency_scatter_work;
	struct workqueue_struct *latency_scatter_workqueue;
	unsigned long queue_run_interval;
	void *host;
	struct iomt_latency_stat_element
		current_stat_array[IOMT_LATENCY_STAT_ARRAY_SIZE];
	struct iomt_latency_stat_element
		last_stat_array[IOMT_LATENCY_STAT_ARRAY_SIZE];
	unsigned long gap_array[IOMT_LATENCY_GAP_ARRAY_SIZE];
	struct iomt_host_latency_stat_ring_node
		stat_ring_buffer[IOMT_STAT_RING_SIZE];
	unsigned int ring_buffer_current_index;
	unsigned int may_stat_error_count;
	unsigned char latency_stat_enable;
	unsigned char usertype_obtain_flag;
	char *host_name;
	iomt_create_dev_attr_func attr_create_func;
	iomt_delete_dev_attr_func attr_delete_func;
};

struct iomt_host_io_timeout_dsm {
	unsigned int judge_slot;
	unsigned int op_arg;
	unsigned int block_ticks;
	unsigned char data_dir;
	iomt_dsm_func dsm_func;
};


#define IOMT_RW_SIZE_GAP_ARRAY_SIZE		8
#define IOMT_RW_GAP_MID_IDX	\
	(IOMT_RW_SIZE_GAP_ARRAY_SIZE / 2)
#define IOMT_RW_SIZE_STAT_ARRAY_SIZE		\
	(IOMT_RW_SIZE_GAP_ARRAY_SIZE + 1)

struct iomt_size_stat_element {
	unsigned long read_count;
	unsigned long write_count;
};

struct iomt_host_rw_size_stat {
	struct iomt_size_stat_element
		current_stat_array[IOMT_RW_SIZE_STAT_ARRAY_SIZE];
	unsigned long gap_array[IOMT_RW_SIZE_GAP_ARRAY_SIZE];
	unsigned int blksize;
	unsigned long total_read_blk_cnt;
	unsigned long total_write_blk_cnt;
	void *host;
	iomt_create_dev_attr_func attr_create_func;
	iomt_delete_dev_attr_func attr_delete_func;
};

struct iomt_host_info {
	struct iomt_host_latency_stat latency_scatter;
	struct iomt_host_io_timeout_dsm io_timeout_dsm;
	struct iomt_host_rw_size_stat rw_size_scatter;
};

#define iomt_info_from_host(host)		\
	((struct iomt_host_info *)((host)->iomt_host_info))

static inline unsigned long iomt_calculate_ul_diff(unsigned long before,
						unsigned long after)
{
	return after - before;
}

static  inline unsigned int iomt_host_stat_latency(
		struct iomt_host_info *iomt_host_info,
		unsigned long begin_time,
		unsigned char dir)
{
	struct iomt_host_latency_stat *latency_scatter = NULL;
	unsigned int i;
	unsigned long diff;

	latency_scatter = &(iomt_host_info->latency_scatter);

	if ((begin_time == 0) || (begin_time > jiffies)) {
		latency_scatter->may_stat_error_count++;
		return IOMT_LATENCY_INVALID_INDEX;
	}

	diff = iomt_calculate_ul_diff(begin_time, jiffies);

	if (likely(diff <
		latency_scatter->gap_array[IOMT_LATENCY_GAP_MID_IDX])) {
		for (i = 0; i < IOMT_LATENCY_GAP_MID_IDX; i++) {
			if (diff <= latency_scatter->gap_array[i])
				break;
		}
	} else {
		for (i = IOMT_LATENCY_GAP_MID_IDX;
			i < IOMT_LATENCY_GAP_ARRAY_SIZE; i++) {
			if (diff <= latency_scatter->gap_array[i])
				break;
		}
	}

	if (likely(dir == IOMT_DIR_READ))
		latency_scatter->current_stat_array[i].read_count++;
	else if (dir == IOMT_DIR_WRITE)
		latency_scatter->current_stat_array[i].write_count++;
	else
		latency_scatter->current_stat_array[i].other_count++;

	return i;
}

static  inline void iomt_host_stat_rw_size(
		struct iomt_host_info *iomt_host_info,
		unsigned long block_cnt,
		unsigned char dir)
{
	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(iomt_host_info->rw_size_scatter);
	unsigned int i;

	if (block_cnt < rw_size_scatter->gap_array[IOMT_RW_GAP_MID_IDX]) {
		for (i = 0; i < IOMT_RW_GAP_MID_IDX; i++) {
			if (block_cnt <= rw_size_scatter->gap_array[i])
				break;
		}
	} else {
		for (i = IOMT_RW_GAP_MID_IDX;
			i < IOMT_RW_SIZE_GAP_ARRAY_SIZE; i++) {
			if (block_cnt <= rw_size_scatter->gap_array[i])
				break;
		}
	}

	if (unlikely(dir == IOMT_DIR_WRITE)) {
		rw_size_scatter->current_stat_array[i].write_count++;
		rw_size_scatter->total_write_blk_cnt += block_cnt;
	} else {
		rw_size_scatter->current_stat_array[i].read_count++;
		rw_size_scatter->total_read_blk_cnt += block_cnt;
	}
}

void dsm_iomt_host_init(void *host,
			const struct iomt_host_ops *iomt_host_ops);

void dsm_iomt_host_exit(void *host,
			const struct iomt_host_ops *iomt_host_ops);

ssize_t io_latency_scatter_store(struct iomt_host_info *iomt_host_info,
		const char *buf, size_t count);

ssize_t io_latency_scatter_show(struct iomt_host_info *iomt_host_info,
					char *buf);

ssize_t rw_size_scatter_show(struct iomt_host_info *iomt_host_info,
					char *buf);

#endif
