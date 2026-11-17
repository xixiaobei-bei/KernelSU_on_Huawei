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

#include <linux/printk.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <linux/ktime.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
/*
 * ifmodify IOMT_LATENCY_GAP_ARRAY_SIZE
 * must modify the format string in function iomt_latency_log_show
 */
#define IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE 9

#define IOMT_LATENCY_ARRAY_SIZE	16


#define IOMT_LATENCY_INVALID_INDEX	\
	IOMT_LATENCY_ARRAY_SIZE

#define IOMT_LATENCY_STAT_RING_BUFFER_SIZE	12
#define IOMT_STAT_RING_SIZE			\
	IOMT_LATENCY_STAT_RING_BUFFER_SIZE
#define IOMT_LATENCY_SCATTER_TIMER_INTERVAL	(10 * HZ)
#define IOMT_IO_TIMEOUT_DSM_JUDGE_SLOT	\
	(11 % IOMT_LATENCY_ARRAY_SIZE)
#define IOMT_LATENCY_LOG_SETTING_MASK	10
#define IOMT_LATENCY_LOG_SETTING_ENABLE	1
#define IOMT_LATENCY_LOG_SETTING_DISABLE	0
#define IOMT_IO_TIMEOUT_DSM_SETTING_MASK	100

#define IOMT_OP_BLOCK_SHIFT		16

#define IOMT_DIR_READ	0
#define IOMT_DIR_WRITE	1
#define IOMT_DIR_OTHER	2
#define IOMT_DIR_UNMAP  3
#define IOMT_DIR_SYNC   4

#define USER_ROOT_DIR "iomt"
#define USER_ENTRY "io_latency_scatter"

#define IOMT_MS_TO_SEC		1000ul
#define IOMT_MS_TO_US		1000ul

typedef void (*iomt_dsm_func)(void *iomt_host_info);
typedef void (*iomt_create_dev_attr_func)(void *iomt_host_info);
typedef void (*iomt_delete_dev_attr_func)(void *iomt_host_info);
typedef void *(*iomt_host_to_info_func)(const void *host);
typedef char *(*iomt_host_name_func)(const void *host);
typedef unsigned int (*iomt_host_blksz_func)(void *host);

typedef void (*iomt_reinit_gap_array_func)(
		unsigned long *array, unsigned long array_size);

static const long
	iomt_latency_scatter_array[IOMT_LATENCY_ARRAY_SIZE] = {
	0,
	1,
	4,
	8,
	16,
	32,
	64,
	100,
	200,
	300,
	500,
	700,
	1000,
	2000,
	5000,
	10000,
};
static const long
	iomt_chunksize_array[IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE] = {
	0,
	4,
	8,
	16,
	64,
	256,
	512,
	1024,
	2048,
};

struct iomt_timestamp {
	unsigned char type;
	unsigned long ticks;
	ktime_t ktime;
};
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
	unsigned long unmap_count;
	unsigned long sync_count;
	unsigned long other_count;
};
struct iomt_host_latency_scatter_node {
	struct iomt_latency_stat_element
		stat_array[IOMT_LATENCY_ARRAY_SIZE];
};

struct iomt_host_latency_stat_ring_node {
	unsigned long stat_tick;
	struct iomt_latency_stat_element
		stat_array[IOMT_LATENCY_ARRAY_SIZE];
};

struct iomt_host_latency_stat {
	struct delayed_work latency_scatter_work;
	struct workqueue_struct *latency_scatter_workqueue;
	unsigned long queue_run_interval;
	void *host;
	struct iomt_host_latency_scatter_node
		io_latency_scatter_buffer[IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE];
	struct iomt_latency_stat_element
		current_stat_array[IOMT_LATENCY_ARRAY_SIZE];
	struct iomt_latency_stat_element
		last_stat_array[IOMT_LATENCY_ARRAY_SIZE];
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

static inline unsigned int iomt_get_array_index(const long *array,
	int array_len, unsigned long number, int unit)
{
	unsigned int i;

	for (i = 0; i <= array_len - 1; i++) {
		if (number <= (array[i + 1] * unit))
			break;
	}

	return i;
}

static inline int iomt_host_io_latency(
	struct iomt_host_info *iomt_host_info,
	struct iomt_timestamp *begin_time,
	unsigned char dir,
	unsigned int chunk_size)
{
	struct iomt_host_latency_stat *latency_scatter = NULL;
	struct iomt_host_latency_scatter_node *latency_scatter_node = NULL;
	unsigned int latency_index;
	unsigned int chunk_size_index;
	ktime_t used_time;

	latency_scatter = &(iomt_host_info->latency_scatter);
	if ((begin_time->ktime == 0) || (begin_time->ktime > ktime_get())) {
		latency_scatter->may_stat_error_count++;
		return IOMT_LATENCY_INVALID_INDEX;
	}

	used_time = ktime_to_us(ktime_sub(ktime_get(), begin_time->ktime));

	chunk_size_index = iomt_get_array_index(iomt_chunksize_array,
		IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE, chunk_size, 1);
	latency_index = iomt_get_array_index(iomt_latency_scatter_array,
		IOMT_LATENCY_ARRAY_SIZE, (unsigned long)used_time, IOMT_MS_TO_US);

	latency_scatter_node = &(latency_scatter->io_latency_scatter_buffer[chunk_size_index]);

	switch(dir) {
	case IOMT_DIR_READ:
		latency_scatter_node->stat_array[latency_index].read_count++;
		latency_scatter->current_stat_array[latency_index].read_count++;
		break;
	case IOMT_DIR_WRITE:
		latency_scatter_node->stat_array[latency_index].write_count++;
		latency_scatter->current_stat_array[latency_index].write_count++;
		break;
	case IOMT_DIR_UNMAP:
		latency_scatter_node->stat_array[latency_index].unmap_count++;
		latency_scatter->current_stat_array[latency_index].unmap_count++;
		break;
	case IOMT_DIR_SYNC:
		latency_scatter_node->stat_array[latency_index].sync_count++;
		latency_scatter->current_stat_array[latency_index].sync_count++;
		break;
	default:
		latency_scatter_node->stat_array[latency_index].other_count++;
		latency_scatter->current_stat_array[latency_index].other_count++;
		break;
	}
	return 0;
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

void dsm_iomt_host_exit(const void *host,
			const struct iomt_host_ops *iomt_host_ops);

ssize_t io_latency_scatter_store(struct iomt_host_info *iomt_host_info,
		const char *buf, size_t count);

ssize_t io_latency_scatter_show(struct iomt_host_info *iomt_host_info,
					char *buf);

ssize_t rw_size_scatter_show(struct iomt_host_info *iomt_host_info,
	char *buf);
int io_latency_creat_proc_entry(struct iomt_host_info *iomt_host_info);
int io_latency_proc_remove(void);

#endif
