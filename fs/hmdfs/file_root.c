/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_root.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: lvhao13@huawei.com
 * Create: 2021-01-11
 *
 */

#include <linux/fs_stack.h>
#include <linux/mount.h>
#include <linux/namei.h>

#include "authority/authentication.h"
#include "comm/socket_adapter.h"
#include "comm/transport.h"
#include "hmdfs.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"
#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif

#define DEVICE_VIEW_CTX_POS 2
#define MERGE_VIEW_CTX_POS  3
#define ROOT_DIR_INO_START  20000000

// used by hmdfs_device_iterate functions
#define DEVICE_VIEW_INO_START 20000002
#define LOCAL_DEVICE_CTX_POS  2

struct hmdfs_peer *get_next_con(struct hmdfs_sb_info *sbi,
				unsigned long current_dev_id)
{
	struct hmdfs_peer *con = NULL;
	struct hmdfs_peer *next_con = NULL;
	struct list_head *head, *node;

	mutex_lock(&sbi->connections.node_lock);
	head = &sbi->connections.node_list;
	if (current_dev_id == 0) {
		node = head->next;
		if (node == head)
			goto done;
		next_con = container_of(node, struct hmdfs_peer, list);
		if (next_con->status == NODE_STAT_ONLINE)
			goto done;
		current_dev_id = next_con->device_id;
		next_con = NULL;
	}

	list_for_each_entry(con, &sbi->connections.node_list, list) {
		if ((con->device_id & 0xFFFF) == (current_dev_id & 0xFFFF)) {
			node = con->list.next;
			if (node == head)
				goto done;
			next_con = container_of(node, struct hmdfs_peer, list);
			if (next_con->status == NODE_STAT_ONLINE)
				goto done;
			current_dev_id = next_con->device_id;
			next_con = NULL;
		}
	}
done:
	if (next_con)
		peer_get(next_con);
	mutex_unlock(&sbi->connections.node_lock);
	return next_con;
}

int hmdfs_device_iterate(struct file *file, struct dir_context *ctx)
{
	int err = 0;
	uint64_t ino_start = DEVICE_VIEW_INO_START;
	struct hmdfs_peer *next_con = NULL;
	unsigned long dev_id = 0;
	struct hmdfs_peer *con = NULL;
	char *remote_device_name = NULL;

	if (ctx->pos != 0)
		goto out;
	dir_emit_dots(file, ctx);

	if (ctx->pos == LOCAL_DEVICE_CTX_POS) {
		err = dir_emit(ctx, DEVICE_VIEW_LOCAL,
			       sizeof(DEVICE_VIEW_LOCAL) - 1, ino_start++,
			       DT_DIR);
		if (!err)
			goto out;
		(ctx->pos)++;
	}
	next_con = get_next_con(file->f_inode->i_sb->s_fs_info, 0);
	if (!next_con)
		goto out;

	dev_id = next_con->device_id;
	peer_put(next_con);
	con = hmdfs_lookup_from_devid(file->f_inode->i_sb->s_fs_info, dev_id);
	remote_device_name = kmalloc(HMDFS_CID_SIZE + 1, GFP_KERNEL);
	if (!remote_device_name) {
		err = -ENOMEM;
		goto out;
	}
	while (con) {
		peer_put(con);
		snprintf(remote_device_name, HMDFS_CID_SIZE + 1, "%s",
			 con->cid);
		if (!dir_emit(ctx, remote_device_name,
			      strlen(remote_device_name), ino_start++, DT_DIR))
			goto done;

		(ctx->pos)++;
		con = get_next_con(file->f_inode->i_sb->s_fs_info, dev_id);
		if (!con)
			goto done;

		dev_id = con->device_id;
	}
done:
	kfree(remote_device_name);
out:
	if (err <= 0)
		ctx->pos = -1;

	return err;
}

int hmdfs_root_iterate(struct file *file, struct dir_context *ctx)
{
	uint64_t ino_start = ROOT_DIR_INO_START;
	struct hmdfs_sb_info *sbi = file_inode(file)->i_sb->s_fs_info;

	if (!dir_emit_dots(file, ctx))
		return 0;
	if (ctx->pos == DEVICE_VIEW_CTX_POS) {
		if (!dir_emit(ctx, DEVICE_VIEW_ROOT,
			      sizeof(DEVICE_VIEW_ROOT) - 1, ino_start, DT_DIR))
			return 0;
		ino_start++;
		ctx->pos = MERGE_VIEW_CTX_POS;
	}
	if (sbi->s_merge_switch && ctx->pos == MERGE_VIEW_CTX_POS) {
		if (!dir_emit(ctx, MERGE_VIEW_ROOT, sizeof(MERGE_VIEW_ROOT) - 1,
			      ino_start, DT_DIR))
			return 0;
		(ctx->pos)++;
	}
	return 0;
}

const struct file_operations hmdfs_root_fops = {
	.owner = THIS_MODULE,
	.iterate = hmdfs_root_iterate,
};

const struct file_operations hmdfs_device_fops = {
	.owner = THIS_MODULE,
	.iterate = hmdfs_device_iterate,
};
