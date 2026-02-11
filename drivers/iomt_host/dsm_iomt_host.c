/*
 * file_name
 * dsm_iomt_host.c
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

#include <linux/printk.h>
#include <log/log_usertype.h>
#include <linux/iomt_host/dsm_iomt_host.h>
#include <linux/kernel.h>

#define IOMT_MS_TO_SEC		1000

/*
 * if modify IOMT_LATENCY_GAP_ARRAY_SIZE,
 * must be modify the default gap array
 */
static const unsigned long
	iomt_latency_gap_array[IOMT_LATENCY_GAP_ARRAY_SIZE] = {
	1,				/* 0 */
	2,				/* 1 */
	4,				/* 2 */
	8,				/* 3 */
	16,				/* 4 */
	(100 * HZ) / IOMT_MS_TO_SEC,	/* 5 */
	(200 * HZ) / IOMT_MS_TO_SEC,	/* 6 */
	(300 * HZ) / IOMT_MS_TO_SEC,	/* 7 */
	(500 * HZ) / IOMT_MS_TO_SEC,	/* 8 */
	(700 * HZ) / IOMT_MS_TO_SEC,	/* 9 */
	(1000 * HZ) / IOMT_MS_TO_SEC,	/* 10 */
	(2000 * HZ) / IOMT_MS_TO_SEC	/* 11 */
};

static void iomt_host_io_timeout_dsm_init(
		struct iomt_host_info *iomt_host_info,
		const struct iomt_host_ops *iomt_host_ops)
{
	struct iomt_host_io_timeout_dsm *io_timeout_dsm;

	io_timeout_dsm = &(iomt_host_info->io_timeout_dsm);

	io_timeout_dsm->dsm_func = iomt_host_ops->dsm_func;

	io_timeout_dsm->judge_slot = IOMT_IO_TIMEOUT_DSM_JUDGE_SLOT;
}

static void iomt_host_io_timeout_dsm_exit(
		struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_io_timeout_dsm *io_timeout_dsm;

	io_timeout_dsm = &(iomt_host_info->io_timeout_dsm);

	io_timeout_dsm->judge_slot = IOMT_IO_TIMEOUT_DSM_JUDGE_SLOT;
}

static void iomt_latency_user_type_obtain(
	struct iomt_host_latency_stat *latency_scatter)
{
	unsigned int log_usertype;

	log_usertype = get_logusertype_flag();

	if (log_usertype != 0) {
		latency_scatter->usertype_obtain_flag = 1;
		if ((log_usertype == COMMERCIAL_USER) ||
		    (log_usertype == OVERSEA_COMMERCIAL_USER))
			latency_scatter->latency_stat_enable = 0;
	}
}

static void iomt_latency_log_show(
		struct iomt_host_latency_stat_ring_node *ring_node,
		struct iomt_host_latency_stat *latency_scatter,
		char *host_name)
{
	unsigned long *gap_array = latency_scatter->gap_array;

	/*
	 * for perfmance, if modify IOMT_LATENCY_GAP_ARRAY_SIZE,
	 * must be modify the below format string
	 */
	pr_info("[IO_LATENCY_SCATTER]%s/%u: "
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"%lu:[%lu/%lu/%lu],%lu:[%lu/%lu/%lu],"
		"above:[%lu/%lu/%lu]\n",
		host_name, HZ,
		gap_array[0],
		ring_node->stat_array[0].read_count,
		ring_node->stat_array[0].write_count,
		ring_node->stat_array[0].other_count,
		gap_array[1],
		ring_node->stat_array[1].read_count,
		ring_node->stat_array[1].write_count,
		ring_node->stat_array[1].other_count,
		gap_array[2],
		ring_node->stat_array[2].read_count,
		ring_node->stat_array[2].write_count,
		ring_node->stat_array[2].other_count,
		gap_array[3],
		ring_node->stat_array[3].read_count,
		ring_node->stat_array[3].write_count,
		ring_node->stat_array[3].other_count,
		gap_array[4],
		ring_node->stat_array[4].read_count,
		ring_node->stat_array[4].write_count,
		ring_node->stat_array[4].other_count,
		gap_array[5],
		ring_node->stat_array[5].read_count,
		ring_node->stat_array[5].write_count,
		ring_node->stat_array[5].other_count,
		gap_array[6],
		ring_node->stat_array[6].read_count,
		ring_node->stat_array[6].write_count,
		ring_node->stat_array[6].other_count,
		gap_array[7],
		ring_node->stat_array[7].read_count,
		ring_node->stat_array[7].write_count,
		ring_node->stat_array[7].other_count,
		gap_array[8],
		ring_node->stat_array[8].read_count,
		ring_node->stat_array[8].write_count,
		ring_node->stat_array[8].other_count,
		gap_array[9],
		ring_node->stat_array[9].read_count,
		ring_node->stat_array[9].write_count,
		ring_node->stat_array[9].other_count,
		gap_array[10],
		ring_node->stat_array[10].read_count,
		ring_node->stat_array[10].write_count,
		ring_node->stat_array[10].other_count,
		gap_array[11],
		ring_node->stat_array[11].read_count,
		ring_node->stat_array[11].write_count,
		ring_node->stat_array[11].other_count,
		ring_node->stat_array[12].read_count,
		ring_node->stat_array[12].write_count,
		ring_node->stat_array[12].other_count);
}

static void iomt_host_latency_stat_update(struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);
	struct iomt_latency_stat_element *current_stat_array =
		latency_scatter->current_stat_array;
	unsigned long current_stat_tmp;
	struct iomt_latency_stat_element *last_stat_array =
		latency_scatter->last_stat_array;
	unsigned int i;

	struct iomt_host_latency_stat_ring_node *ring_buffer =
		latency_scatter->stat_ring_buffer;
	struct iomt_host_latency_stat_ring_node *current_ring_node = NULL;
	latency_scatter->ring_buffer_current_index++;
	latency_scatter->ring_buffer_current_index %=
		IOMT_LATENCY_STAT_RING_BUFFER_SIZE;

	current_ring_node =
		&ring_buffer[latency_scatter->ring_buffer_current_index];

	for (i = 0; i < IOMT_LATENCY_STAT_ARRAY_SIZE; i++) {
		current_stat_tmp = current_stat_array[i].read_count;
		current_ring_node->stat_array[i].read_count =
			iomt_calculate_ul_diff(last_stat_array[i].read_count,
				current_stat_tmp);
		last_stat_array[i].read_count = current_stat_tmp;

		current_stat_tmp = current_stat_array[i].write_count;
		current_ring_node->stat_array[i].write_count =
			iomt_calculate_ul_diff(last_stat_array[i].write_count,
				current_stat_tmp);
		last_stat_array[i].write_count = current_stat_tmp;

		current_stat_tmp = current_stat_array[i].other_count;
		current_ring_node->stat_array[i].other_count =
			iomt_calculate_ul_diff(last_stat_array[i].other_count,
				current_stat_tmp);
		last_stat_array[i].other_count = current_stat_tmp;
	}

	current_ring_node->stat_tick =
		((unsigned long)ktime_to_ms(ktime_get()))/IOMT_MS_TO_SEC;

	if (unlikely(!latency_scatter->usertype_obtain_flag))
		iomt_latency_user_type_obtain(latency_scatter);

	if (latency_scatter->latency_stat_enable)
		iomt_latency_log_show(current_ring_node,
			latency_scatter, latency_scatter->host_name);
}

static void iomt_host_io_dsm_process(struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);
	struct iomt_host_io_timeout_dsm *io_timeout_dsm =
		&(iomt_host_info->io_timeout_dsm);
	unsigned char is_io_timeout_dsm_trigger;
	unsigned int i;

	unsigned int current_ring_buffer_index =
		latency_scatter->ring_buffer_current_index %
		IOMT_LATENCY_STAT_RING_BUFFER_SIZE;

	struct iomt_host_latency_stat_ring_node *current_ring_node =
		&latency_scatter->stat_ring_buffer[current_ring_buffer_index];

	unsigned int judge_slot =
		io_timeout_dsm->judge_slot %
		IOMT_LATENCY_STAT_ARRAY_SIZE;

	if (io_timeout_dsm->dsm_func == NULL)
		return;

	is_io_timeout_dsm_trigger = 0;
	for (i = judge_slot; i < IOMT_LATENCY_STAT_ARRAY_SIZE; i++) {
		if ((current_ring_node->stat_array[i].read_count != 0) ||
		    (current_ring_node->stat_array[i].write_count != 0) ||
		    (current_ring_node->stat_array[i].other_count != 0)) {
			is_io_timeout_dsm_trigger = 1;
			break;
		}
	}

	if (unlikely(is_io_timeout_dsm_trigger))
		io_timeout_dsm->dsm_func(iomt_host_info);
}

static void iomt_host_latency_scatter_timer(
			struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);

	iomt_host_latency_stat_update(iomt_host_info);

	iomt_host_io_dsm_process(iomt_host_info);

	queue_delayed_work(latency_scatter->latency_scatter_workqueue,
		&latency_scatter->latency_scatter_work,
		latency_scatter->queue_run_interval);
}

static void iomt_host_latency_scatter_work(struct work_struct *work)
{
	struct iomt_host_latency_stat *latency_scatter =
		container_of(work, struct iomt_host_latency_stat,
			latency_scatter_work.work);
	struct iomt_host_info *iomt_host_info =
		container_of(latency_scatter, struct iomt_host_info,
			latency_scatter);

	iomt_host_latency_scatter_timer(iomt_host_info);
}

ssize_t io_latency_scatter_show(struct iomt_host_info *iomt_host_info,
					char *buf)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);
	struct iomt_host_latency_stat_ring_node *ring_node;
	int bytes_written = 0;
	int i;
	int j;
	unsigned int  ring_buffer_index =
		latency_scatter->ring_buffer_current_index %
		IOMT_LATENCY_STAT_RING_BUFFER_SIZE;

	/* total */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "total:\t");
	for (i = 0; i < IOMT_LATENCY_STAT_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu/%lu/%lu\t",
			latency_scatter->current_stat_array[i].read_count,
			latency_scatter->current_stat_array[i].write_count,
			latency_scatter->current_stat_array[i].other_count);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* gap */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "gap:\t");
	for (i = 0; i < IOMT_LATENCY_GAP_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu\t", latency_scatter->gap_array[i]);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* ring buffer */
	for (i = 0; i < IOMT_LATENCY_STAT_RING_BUFFER_SIZE; i++) {
		ring_node = &latency_scatter->stat_ring_buffer
				[(++ring_buffer_index) %
				IOMT_LATENCY_STAT_RING_BUFFER_SIZE];
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written, "stat:\t");
		for (j = 0; j < IOMT_LATENCY_STAT_ARRAY_SIZE; j++)
			bytes_written += scnprintf(buf + bytes_written,
				PAGE_SIZE - bytes_written,
				"%lu/%lu/%lu\t",
				ring_node->stat_array[j].read_count,
				ring_node->stat_array[j].write_count,
				ring_node->stat_array[j].other_count);
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"(%lu)\n", ring_node->stat_tick);
	}

	/* setting */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"log_enable=%u\n", latency_scatter->latency_stat_enable);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"queue_run_interval=%lu\n",
		latency_scatter->queue_run_interval);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"io_timeout_judge_slot=%u\n",
		(iomt_host_info->io_timeout_dsm).judge_slot);

	/* extra */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"may_error=%u,HZ=%u\n",
		latency_scatter->may_stat_error_count, HZ);

	return bytes_written;
}


ssize_t io_latency_scatter_store(struct iomt_host_info *iomt_host_info,
		const char *buf, size_t count)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);
	struct iomt_host_io_timeout_dsm *io_timeout_dsm =
		&(iomt_host_info->io_timeout_dsm);
	long value;

	if (kstrtol(buf, 0, &value))
		return -EINVAL;

	/* log enable setting */
	if ((value % IOMT_LATENCY_LOG_SETTING_MASK) ==
	    IOMT_LATENCY_LOG_SETTING_ENABLE)
		latency_scatter->latency_stat_enable =
			IOMT_LATENCY_LOG_SETTING_ENABLE;
	else
		latency_scatter->latency_stat_enable =
			IOMT_LATENCY_LOG_SETTING_DISABLE;

	value /= IOMT_LATENCY_LOG_SETTING_MASK;

	/* io timeout setting */
	if ((value % IOMT_IO_TIMEOUT_DSM_SETTING_MASK) <
	    IOMT_LATENCY_STAT_ARRAY_SIZE)
		io_timeout_dsm->judge_slot =
			value % IOMT_LATENCY_STAT_ARRAY_SIZE;

	value /= IOMT_IO_TIMEOUT_DSM_SETTING_MASK;

	return count;
}

static void iomt_host_latency_stat_init(
		struct iomt_host_info *iomt_host_info,
		void *host,
		const struct iomt_host_ops *iomt_host_ops)
{
	unsigned int i;
	unsigned int j;
	struct iomt_host_latency_stat_ring_node *ring_node;
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);

	latency_scatter->host = host;
	latency_scatter->ring_buffer_current_index = 0;
	latency_scatter->may_stat_error_count = 0;
	latency_scatter->latency_stat_enable = 1;
	latency_scatter->usertype_obtain_flag = 0;
	latency_scatter->host_name = "NA";
	latency_scatter->queue_run_interval =
		IOMT_LATENCY_SCATTER_TIMER_INTERVAL;
	latency_scatter->attr_create_func =
		iomt_host_ops->create_latency_attr_func;
	latency_scatter->attr_delete_func =
		iomt_host_ops->delete_latency_attr_func;

	if (iomt_host_ops->host_name_func)
		latency_scatter->host_name =
			iomt_host_ops->host_name_func(host);

	INIT_DELAYED_WORK(&latency_scatter->latency_scatter_work,
		iomt_host_latency_scatter_work);
	latency_scatter->latency_scatter_workqueue =
		create_workqueue(latency_scatter->host_name);

	for (i = 0; i < IOMT_LATENCY_STAT_ARRAY_SIZE; i++) {
		latency_scatter->current_stat_array[i].read_count = 0;
		latency_scatter->current_stat_array[i].write_count = 0;
		latency_scatter->current_stat_array[i].other_count = 0;
		latency_scatter->last_stat_array[i].read_count = 0;
		latency_scatter->last_stat_array[i].write_count = 0;
		latency_scatter->last_stat_array[i].other_count = 0;
	}

	for (i = 0; i < IOMT_LATENCY_GAP_ARRAY_SIZE; i++)
		latency_scatter->gap_array[i] = iomt_latency_gap_array[i];

	if (iomt_host_ops->reinit_latency_gap_array_func)
		iomt_host_ops->reinit_latency_gap_array_func(
				latency_scatter->gap_array,
				IOMT_LATENCY_GAP_ARRAY_SIZE);

	for (i = 0; i < IOMT_LATENCY_STAT_RING_BUFFER_SIZE; i++) {
		ring_node = &(latency_scatter->stat_ring_buffer[i]);
		ring_node->stat_tick = 0;
		for (j = 0; j < IOMT_LATENCY_STAT_ARRAY_SIZE; j++) {
			ring_node->stat_array[j].read_count = 0;
			ring_node->stat_array[j].write_count = 0;
			ring_node->stat_array[j].other_count = 0;
		}
	}

	if (latency_scatter->attr_create_func)
		latency_scatter->attr_create_func((void *)iomt_host_info);

	if (latency_scatter->latency_scatter_workqueue != NULL)
		queue_delayed_work(latency_scatter->latency_scatter_workqueue,
			&latency_scatter->latency_scatter_work,
			latency_scatter->queue_run_interval);
}

static void iomt_host_latency_stat_exit(struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);

	cancel_delayed_work_sync(&latency_scatter->latency_scatter_work);

	if (latency_scatter->latency_scatter_workqueue != NULL)
		destroy_workqueue(latency_scatter->latency_scatter_workqueue);

	if (latency_scatter->attr_delete_func)
		latency_scatter->attr_delete_func((void *)iomt_host_info);
}

ssize_t rw_size_scatter_show(struct iomt_host_info *iomt_host_info,
					char *buf)
{
	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(iomt_host_info->rw_size_scatter);
	struct iomt_size_stat_element *current_stat_array =
		rw_size_scatter->current_stat_array;
	unsigned long *gap_array =
		rw_size_scatter->gap_array;
	int bytes_written;
	int i;

	bytes_written = 0;

	/* total */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "total:\t");
	for (i = 0; i < IOMT_RW_SIZE_STAT_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu/%lu\t",
			current_stat_array[i].read_count,
			current_stat_array[i].write_count);

	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* gap */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "gap:\t");
	for (i = 0; i < IOMT_RW_SIZE_GAP_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu\t", gap_array[i]);

	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* extra */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"block_size=%u\n",
		rw_size_scatter->blksize);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"total_read_block_count=%lu\n",
		rw_size_scatter->total_read_blk_cnt);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"total_write_block_count=%lu\n",
		rw_size_scatter->total_write_blk_cnt);

	return bytes_written;
}

static void iomt_host_rw_size_stat_init(
		struct iomt_host_info *iomt_host_info,
		void *host,
		const struct iomt_host_ops *iomt_host_ops)
{
	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(iomt_host_info->rw_size_scatter);
	unsigned int i;
	unsigned int gap_shift;

	rw_size_scatter->blksize = 0;
	rw_size_scatter->host = host;
	rw_size_scatter->attr_create_func =
		iomt_host_ops->create_rw_size_attr_func;
	rw_size_scatter->attr_delete_func =
		iomt_host_ops->delete_rw_size_attr_func;

	if (iomt_host_ops->host_blk_size_fun)
		rw_size_scatter->blksize =
			iomt_host_ops->host_blk_size_fun(host);

	for (i = 0; i < IOMT_RW_SIZE_STAT_ARRAY_SIZE; i++) {
		rw_size_scatter->current_stat_array[i].read_count = 0;
		rw_size_scatter->current_stat_array[i].write_count = 0;
	}

	gap_shift = 0;
	for (i = 0; i < IOMT_RW_SIZE_GAP_ARRAY_SIZE; i++) {
		rw_size_scatter->gap_array[i] = 2 << gap_shift;
		gap_shift += (i % 2 ? 1 : 2);
	}

	if (iomt_host_ops->reinit_blk_size_gap_array_func)
		iomt_host_ops->reinit_blk_size_gap_array_func(
				rw_size_scatter->gap_array,
				IOMT_RW_SIZE_GAP_ARRAY_SIZE);

	rw_size_scatter->total_read_blk_cnt = 0;
	rw_size_scatter->total_write_blk_cnt = 0;

	if (rw_size_scatter->attr_create_func)
		rw_size_scatter->attr_create_func((void *)iomt_host_info);
}

static void iomt_host_rw_size_stat_exit(struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_rw_size_stat *rw_size_scatter =
		&(iomt_host_info->rw_size_scatter);

	if (rw_size_scatter->attr_delete_func)
		rw_size_scatter->attr_delete_func((void *)iomt_host_info);
}

void dsm_iomt_host_init(void *host,
			const struct iomt_host_ops *iomt_host_ops)
{
	struct iomt_host_info *iomt_host_info = NULL;

	if (iomt_host_ops->host_to_iomt_func == NULL)
		return;

	iomt_host_info =
		(struct iomt_host_info *)iomt_host_ops->host_to_iomt_func(host);

	if (iomt_host_info != NULL) {
		iomt_host_io_timeout_dsm_init(iomt_host_info,
				iomt_host_ops);
		iomt_host_latency_stat_init(iomt_host_info,
				host, iomt_host_ops);
		iomt_host_rw_size_stat_init(iomt_host_info,
				host, iomt_host_ops);
	}
}

void dsm_iomt_host_exit(void *host,
			const struct iomt_host_ops *iomt_host_ops)
{
	struct iomt_host_info *iomt_host_info = NULL;

	if (iomt_host_ops->host_to_iomt_func == NULL)
		return;

	iomt_host_info =
		(struct iomt_host_info *)iomt_host_ops->host_to_iomt_func(host);

	if (iomt_host_info != NULL) {
		iomt_host_rw_size_stat_exit(iomt_host_info);
		iomt_host_latency_stat_exit(iomt_host_info);
		iomt_host_io_timeout_dsm_exit(iomt_host_info);
	}
}

