/*
 * npu_debug_resource_count.c
 *
 * about npu debug resource count
 *
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
#include "npu_debug_resource_count.h"

#include <linux/of.h>
#include <linux/io.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>
#include <linux/list.h>
#include <linux/debugfs.h>

#include "npu_dfx.h"
#include "npu_resmem.h"
#include "npu_log.h"
#include "npu_platform_register.h"
#include "npu_common.h"

struct hisi_npu_reource_debugfs_fops {
	struct file_operations fops;
	const char *file_name;
};

static struct dentry *hisi_npu_resource_debug_dir = NULL;

static const struct hisi_npu_reource_debugfs_fops g_hisi_npu_reource_debugfs[] = {
	{
		{
			.owner   = THIS_MODULE,
			.open = hisi_npu_resource_debugfs_open,
			.read = seq_read,
			.llseek = seq_lseek,
			.release = single_release,
		},
		.file_name = "resource",
	},
};

int devdrv_npu_debug_init()
{
	unsigned int i;

	if (hisi_npu_resource_debug_dir != NULL)
		return -EINVAL;

	hisi_npu_resource_debug_dir = debugfs_create_dir("hisi_npu_debug", NULL);
	if (IS_ERR_OR_NULL(hisi_npu_resource_debug_dir)) {
		NPU_DRV_ERR("create hisi_npu_debug_dir fail\n");
		return -EINVAL;
	}

	for (i = 0; i < sizeof(g_hisi_npu_reource_debugfs) / sizeof(g_hisi_npu_reource_debugfs[0]); i++)
		debugfs_create_file(g_hisi_npu_reource_debugfs[i].file_name, 00660,
			hisi_npu_resource_debug_dir, NULL, &(g_hisi_npu_reource_debugfs[i].fops));

	return 0;
}

int list_count(struct list_head *list)
{
	struct list_head *pos = NULL;
	struct list_head *n = NULL;
	int counter = 0;
	if (list_empty_careful(list)) {
		NPU_DRV_WARN("list is empty\n");
		return 0;
	}
	list_for_each_safe(pos, n, list)
		counter++;
	return counter;
}

static int hisi_npu_resource_debugfs_show(struct seq_file *s, void *data)
{
	const u8 dev_id = 0;
	struct devdrv_dev_ctx *cur_dev_ctx = NULL;

	int sink_stream_available_list_counter;
	int stream_available_list_counter;
	int event_available_list_counter;
	int model_available_list_counter;
	int task_available_list_counter;

	cur_dev_ctx = get_dev_ctx_by_id(dev_id);
	if (IS_ERR_OR_NULL(cur_dev_ctx))
		return 0;

	spin_lock(&cur_dev_ctx->spinlock);
	sink_stream_available_list_counter = list_count(&cur_dev_ctx->sink_stream_available_list);
	stream_available_list_counter = list_count(&cur_dev_ctx->stream_available_list);
	spin_unlock(&cur_dev_ctx->spinlock);

	spin_lock(&cur_dev_ctx->event_spinlock);
	event_available_list_counter = list_count(&cur_dev_ctx->event_available_list);
	spin_unlock(&cur_dev_ctx->event_spinlock);

	spin_lock(&cur_dev_ctx->model_spinlock);
	model_available_list_counter = list_count(&cur_dev_ctx->model_available_list);
	spin_unlock(&cur_dev_ctx->model_spinlock);

	spin_lock(&cur_dev_ctx->task_spinlock);
	task_available_list_counter = list_count(&cur_dev_ctx->task_available_list);
	spin_unlock(&cur_dev_ctx->task_spinlock);

	seq_printf(s, "devid: %d\n", cur_dev_ctx->devid);
	seq_printf(s, "plat_type: %d\n", cur_dev_ctx->plat_type);
	seq_printf(s, "sink_stream_num: %d/%d\n", cur_dev_ctx->sink_stream_num, DEVDRV_MAX_SINK_STREAM_ID);
	seq_printf(s, "non_sink_stream_num: %d/%d\n", cur_dev_ctx->stream_num, DEVDRV_MAX_NON_SINK_STREAM_ID);
	seq_printf(s, "event_num: %d/%d\n", cur_dev_ctx->event_num, DEVDRV_MAX_EVENT_ID);
	seq_printf(s, "sq_num: %d/%d\n", cur_dev_ctx->sq_num, DEVDRV_MAX_SQ_NUM);
	seq_printf(s, "cq_num: %d/%d\n", cur_dev_ctx->cq_num, DEVDRV_MAX_CQ_NUM);
	seq_printf(s, "model_id_num: %d/%d\n", cur_dev_ctx->model_id_num, DEVDRV_MAX_MODEL_ID);
	seq_printf(s, "task_id_num: %d/%d\n", cur_dev_ctx->task_id_num, DEVDRV_MAX_TASK_ID);

	seq_printf(s, "sink_stream_available_list_counter: %d/%d\n", sink_stream_available_list_counter,
		DEVDRV_MAX_SINK_STREAM_ID);
	seq_printf(s, "non_sink_stream_available_list_counter: %d/%d\n", stream_available_list_counter,
		DEVDRV_MAX_NON_SINK_STREAM_ID);
	seq_printf(s, "event_available_list_counter: %d/%d\n", event_available_list_counter, DEVDRV_MAX_EVENT_ID);
	seq_printf(s, "model_available_list_counter: %d/%d\n", model_available_list_counter, DEVDRV_MAX_MODEL_ID);
	seq_printf(s, "task_available_list_counter: %d/%d\n", task_available_list_counter, DEVDRV_MAX_TASK_ID);

	return 0;
}

static int hisi_npu_resource_debugfs_open(struct inode *inode, struct file *file)
{
	return single_open(file, hisi_npu_resource_debugfs_show, inode->i_private);
}
