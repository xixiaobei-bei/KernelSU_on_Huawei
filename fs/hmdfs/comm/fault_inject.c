/* SPDX-License-Identifier: GPL-2.0 */
/*
 * fault_inject.c
 *
 * Copyright (c) 2020-2020 Huawei Technologies Co., Ltd.
 * Author: yuyufen@huawei.com
 * Create: 2020-12-17
 *
 */

#include "hmdfs.h"
#include "fault_inject.h"
#include "connection.h"

static DECLARE_FAULT_ATTR(fail_default_attr);
static struct dentry *hmdfs_debugfs_root;

void __init hmdfs_create_debugfs_root(void)
{
	hmdfs_debugfs_root = debugfs_create_dir("hmdfs", NULL);
	if (!hmdfs_debugfs_root)
		hmdfs_warning("failed to create debugfs directory");
}

void hmdfs_destroy_debugfs_root(void)
{
	debugfs_remove_recursive(hmdfs_debugfs_root);
	hmdfs_debugfs_root = NULL;
}

void hmdfs_fault_inject_init(struct hmdfs_fault_inject *fault_inject,
			     const char *name)
{
	struct dentry *dir = NULL;
	struct dentry *parent = NULL;
	struct fault_attr *attr = &fault_inject->attr;

	if (!hmdfs_debugfs_root)
		return;

	parent = debugfs_create_dir(name, hmdfs_debugfs_root);
	if (!parent) {
		hmdfs_warning("failed to create %s debugfs directory", name);
		return;
	}

	*attr = fail_default_attr;
	dir = fault_create_debugfs_attr("fault_inject", parent, attr);
	if (IS_ERR(dir)) {
		hmdfs_warning("hmdfs: failed to create debugfs attr");
		debugfs_remove_recursive(parent);
		return;
	}
	fault_inject->parent = parent;
	debugfs_create_ulong("op_mask", 0600, dir, &fault_inject->op_mask);
	debugfs_create_ulong("fail_send_message", 0600, dir,
			     &fault_inject->fail_send_message);
	debugfs_create_ulong("fake_fid_ver", 0600, dir,
			     &fault_inject->fake_fid_ver);
	debugfs_create_bool("fail_req", 0600, dir, &fault_inject->fail_req);
}

void hmdfs_fault_inject_fini(struct hmdfs_fault_inject *fault_inject)
{
	debugfs_remove_recursive(fault_inject->parent);
}

bool hmdfs_should_fail_sendmsg(struct hmdfs_fault_inject *fault_inject,
			       struct hmdfs_peer *con,
			       struct hmdfs_send_data *msg, int *err)
{
	struct hmdfs_head_cmd *head = (struct hmdfs_head_cmd *)msg->head;
	unsigned long type = fault_inject->fail_send_message;

	if (!test_bit(head->operations.command, &fault_inject->op_mask))
		return false;

	if (type != T_MSG_FAIL && type != T_MSG_DISCARD)
		return false;

	if (!should_fail(&fault_inject->attr, 1))
		return false;

	if (type == T_MSG_FAIL)
		*err = -EINVAL;
	else if (type == T_MSG_DISCARD)
		*err = 0;

	hmdfs_err(
		"fault injection err %d, %s message, device_id %llu, msg_id %u, cmd %d",
		*err, (type == T_MSG_FAIL) ? "fail" : "discard", con->device_id,
		le32_to_cpu(head->msg_id), head->operations.command);
	return true;
}

bool hmdfs_should_fail_req(struct hmdfs_fault_inject *fault_inject,
			   struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			   int *err)
{
	if (!test_bit(cmd->operations.command, &fault_inject->op_mask))
		return false;

	if (!fault_inject->fail_req)
		return false;

	if (!should_fail(&fault_inject->attr, 1))
		return false;

	*err = -EIO;
	hmdfs_err("fault injection err %d, device_id %llu, msg_id %u, cmd %d",
		  *err, con->device_id, le32_to_cpu(cmd->msg_id),
		  cmd->operations.command);
	return true;
}

bool hmdfs_should_fake_fid_ver(struct hmdfs_fault_inject *fault_inject,
			       struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *cmd,
			       enum CHANGE_FID_VER_TYPE fake_type)
{
	unsigned long type = fault_inject->fake_fid_ver;

	if (!test_bit(cmd->operations.command, &fault_inject->op_mask))
		return false;

	if (type != fake_type)
		return false;

	if (!should_fail(&fault_inject->attr, 1))
		return false;

	hmdfs_err(
		"fault injection to change fid ver by %s cookie, device_id %llu, msg_id %u, cmd %d",
		(type == T_BOOT_COOKIE) ? "boot" : "con", con->device_id,
		le32_to_cpu(cmd->msg_id), cmd->operations.command);
	return true;
}
