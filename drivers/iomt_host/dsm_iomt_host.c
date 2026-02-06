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

static struct iomt_host_latency_stat *g_latency_scatter;
#define IOMT_BUF_SIZE 2048

/*
 * if modify IOMT_LATENCY_ARRAY_SIZE,
 * must be modify the default gap array
 */
static void iomt_host_io_timeout_dsm_init(
		struct iomt_host_info *iomt_host_info,
		const struct iomt_host_ops *iomt_host_ops)
{
	struct iomt_host_io_timeout_dsm *io_timeout_dsm = NULL;

	io_timeout_dsm = &(iomt_host_info->io_timeout_dsm);

	io_timeout_dsm->dsm_func = iomt_host_ops->dsm_func;

	io_timeout_dsm->judge_slot = IOMT_IO_TIMEOUT_DSM_JUDGE_SLOT;
}

static void iomt_host_io_timeout_dsm_exit(
		struct iomt_host_info *iomt_host_info)
{
	struct iomt_host_io_timeout_dsm *io_timeout_dsm = NULL;

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
	/*
	 * for perfmance, if modify IOMT_LATENCY_GAP_ARRAY_SIZE,
	 * must be modify the below format string
	 */
	int i;
	int bytes_written = 0;
	char *buf = NULL;

	buf = (char *)kzalloc(IOMT_BUF_SIZE * sizeof(char), GFP_KERNEL);
	if (buf == NULL) {
		pr_info("[IO_LATENCY_SCATTER] malloc failed\n");
		return;
	}

	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written,
		"[IO_LATENCY_SCATTER]%s/%u: ",
		host_name, HZ);
	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu:[%lu/%lu/%lu/%lu/%lu],",
			iomt_latency_scatter_array[i],
			ring_node->stat_array[i].read_count,
			ring_node->stat_array[i].write_count,
			ring_node->stat_array[i].unmap_count,
			ring_node->stat_array[i].sync_count,
			ring_node->stat_array[i].other_count);
	pr_info("%s\n", buf);

	kfree(buf);
	buf = NULL;
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

	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++) {
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

		current_stat_tmp = current_stat_array[i].unmap_count;
		current_ring_node->stat_array[i].unmap_count =
			iomt_calculate_ul_diff(last_stat_array[i].unmap_count,
				current_stat_tmp);
		last_stat_array[i].unmap_count = current_stat_tmp;

		current_stat_tmp = current_stat_array[i].sync_count;
		current_ring_node->stat_array[i].sync_count =
			iomt_calculate_ul_diff(last_stat_array[i].sync_count,
				current_stat_tmp);
		last_stat_array[i].sync_count = current_stat_tmp;

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
		IOMT_LATENCY_ARRAY_SIZE;

	if (io_timeout_dsm->dsm_func == NULL)
		return;

	is_io_timeout_dsm_trigger = 0;
	for (i = judge_slot; i < IOMT_LATENCY_ARRAY_SIZE; i++) {
		if ((current_ring_node->stat_array[i].read_count != 0) ||
			(current_ring_node->stat_array[i].write_count != 0) ||
			(current_ring_node->stat_array[i].sync_count != 0) ||
			(current_ring_node->stat_array[i].unmap_count != 0) ||
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
	struct iomt_host_latency_stat_ring_node *ring_node = NULL;
	int bytes_written = 0;
	int i;
	int j;
	unsigned int  ring_buffer_index =
		latency_scatter->ring_buffer_current_index %
		IOMT_LATENCY_STAT_RING_BUFFER_SIZE;

	/* total */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "total:\t");
	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu/%lu/%lu/%lu\t",
			latency_scatter->current_stat_array[i].read_count,
			latency_scatter->current_stat_array[i].write_count,
			latency_scatter->current_stat_array[i].unmap_count,
			latency_scatter->current_stat_array[i].sync_count);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* gap */
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "gap:\t");
	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written,
			"%lu\t", iomt_latency_scatter_array[i]);
	bytes_written += scnprintf(buf + bytes_written,
		PAGE_SIZE - bytes_written, "\n");

	/* ring buffer */
	for (i = 0; i < IOMT_LATENCY_STAT_RING_BUFFER_SIZE; i++) {
		ring_node = &latency_scatter->stat_ring_buffer
				[(++ring_buffer_index) %
				IOMT_LATENCY_STAT_RING_BUFFER_SIZE];
		bytes_written += scnprintf(buf + bytes_written,
			PAGE_SIZE - bytes_written, "stat:\t");
		for (j = 0; j < IOMT_LATENCY_ARRAY_SIZE; j++)
			bytes_written += scnprintf(buf + bytes_written,
				PAGE_SIZE - bytes_written,
				"%lu/%lu/%lu/%lu\t",
				ring_node->stat_array[j].read_count,
				ring_node->stat_array[j].write_count,
				ring_node->stat_array[j].unmap_count,
				ring_node->stat_array[j].sync_count);
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
		IOMT_LATENCY_ARRAY_SIZE)
		io_timeout_dsm->judge_slot =
			value % IOMT_LATENCY_ARRAY_SIZE;

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
	struct iomt_host_latency_stat_ring_node *ring_node = NULL;
	struct iomt_host_latency_stat *latency_scatter =
		&(iomt_host_info->latency_scatter);
	struct iomt_host_latency_scatter_node *latency_scatter_node = NULL;

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

	for (i = 0; i < IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE; i++) {
		latency_scatter_node = &(latency_scatter->io_latency_scatter_buffer[i]);
		memset(latency_scatter_node->stat_array,
			0, sizeof(struct iomt_latency_stat_element) * IOMT_LATENCY_ARRAY_SIZE);
	}

	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++) {
		latency_scatter->current_stat_array[i].read_count = 0;
		latency_scatter->current_stat_array[i].write_count = 0;
		latency_scatter->current_stat_array[i].sync_count = 0;
		latency_scatter->current_stat_array[i].unmap_count = 0;
		latency_scatter->current_stat_array[i].other_count = 0;
		latency_scatter->last_stat_array[i].read_count = 0;
		latency_scatter->last_stat_array[i].write_count = 0;
		latency_scatter->last_stat_array[i].sync_count = 0;
		latency_scatter->last_stat_array[i].unmap_count = 0;
		latency_scatter->last_stat_array[i].other_count = 0;
	}

	for (i = 0; i < IOMT_LATENCY_STAT_RING_BUFFER_SIZE; i++) {
		ring_node = &(latency_scatter->stat_ring_buffer[i]);
		ring_node->stat_tick = 0;
		for (j = 0; j < IOMT_LATENCY_ARRAY_SIZE; j++) {
			ring_node->stat_array[j].read_count = 0;
			ring_node->stat_array[j].write_count = 0;
			ring_node->stat_array[j].sync_count = 0;
			ring_node->stat_array[j].unmap_count = 0;
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

/* proc iomt latency show */
static struct proc_dir_entry *proc_root = NULL;
static struct proc_dir_entry *proc_entry = NULL;
static int io_latency_proc_show(struct seq_file *m, void *v)
{
	struct iomt_host_latency_scatter_node *latency_scatter_node = NULL;
	int bytes_written = 0;
	int i;
	int j;
	char *buf = NULL;
	static int reset_flag = 1;

	buf = (char *)kzalloc(IOMT_BUF_SIZE * sizeof(char), GFP_KERNEL);
	if (NULL == buf) {
		pr_info("[IO_LATENCY_SCATTER] malloc failed\n");
		return -EINVAL;
	}
	/* IO Latency gap */
	bytes_written += scnprintf(buf + bytes_written,
		IOMT_BUF_SIZE - bytes_written, "Init:%d\n", reset_flag);
	bytes_written += scnprintf(buf + bytes_written,
		IOMT_BUF_SIZE - bytes_written, "GAP:\t");
	for (i = 0; i < IOMT_LATENCY_ARRAY_SIZE; i++)
		bytes_written += scnprintf(buf + bytes_written,
			IOMT_BUF_SIZE - bytes_written,
			"%lu\t", iomt_latency_scatter_array[i]);
	seq_printf(m, "%s\n", buf);
	/* IO latency scatter array */
	for (i = 0; i < IOMT_LATENCY_CHUNKSIZE_ARRAY_SIZE; i++) {
		memset(buf, 0, IOMT_BUF_SIZE * sizeof(char));
		bytes_written = 0;
		latency_scatter_node = &g_latency_scatter->io_latency_scatter_buffer[i];
		bytes_written += scnprintf(buf + bytes_written,
			IOMT_BUF_SIZE - bytes_written, "IO[%lu]:\t", iomt_chunksize_array[i]);
		for (j = 0; j < IOMT_LATENCY_ARRAY_SIZE; j++)
			bytes_written += scnprintf(buf + bytes_written,
				IOMT_BUF_SIZE - bytes_written,
				"%lu/%lu/%lu/%lu\t",
				latency_scatter_node->stat_array[j].read_count,
				latency_scatter_node->stat_array[j].write_count,
				latency_scatter_node->stat_array[j].unmap_count,
				latency_scatter_node->stat_array[j].sync_count);
		seq_printf(m, "%s\n", buf);
	}
	/* Clear the reset flag, 0 means no reset occurred */
	reset_flag = 0;
	kfree(buf);
	buf = NULL;
	return 0;
}
static int io_latency_proc_open(struct inode *inode, struct file *file)
{
	return single_open(file, io_latency_proc_show, NULL);
}

static const struct file_operations iomt_proc_fops = {
	.open = io_latency_proc_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.release = single_release,
};

int io_latency_creat_proc_entry(struct iomt_host_info *iomt_host_info)
{
	int ret = 0;

	if (iomt_host_info == NULL) {
		pr_err("%s:Invalid host info\n", __func__);
		return -EINVAL;
	}
	proc_root = proc_mkdir(USER_ROOT_DIR, NULL);
	if (!proc_root) {
		pr_err("%s:iomt proc mkdir fail\n", __func__);
		return -EINVAL;
	}
	proc_entry = proc_create(USER_ENTRY, 0444, proc_root, &iomt_proc_fops);
	if (!proc_entry) {
		pr_err("%s:iomt proc create fail\n", __func__);
		ret = -EINVAL;
		goto OUT;
	}
	pr_err("%s:Create /proc/%s/%s\n", __func__, USER_ROOT_DIR, USER_ENTRY);
	g_latency_scatter = &(iomt_host_info->latency_scatter);
	return ret;
OUT:
	io_latency_proc_remove();
	return ret;
}

/* Remove /proc/iomt/io_latency_scatter */
int io_latency_proc_remove()
{
	g_latency_scatter = NULL;
	remove_proc_entry(USER_ENTRY, proc_root);
	remove_proc_entry(USER_ROOT_DIR, NULL);
	return 0;
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
		rw_size_scatter->gap_array[i] = 2 << gap_shift; //lint !e647
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

void dsm_iomt_host_exit(const void *host,
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

