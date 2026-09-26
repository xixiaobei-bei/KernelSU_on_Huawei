/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_server.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 *         chenyi77@huawei.com
 * Create: 2020-04-01
 *
 */

#include "adapter_server.h"

#include <linux/mm.h>
#include <linux/namei.h>
#include <linux/time.h>

#include "adapter_file_id_generator.h"
#include "authority/authentication.h"
#include "comm/connection.h"

#define ADAPTER_CACHE_BLOCK_SIZE (1024 * 1024)
#define MAX_READ_SIZE		 (32 * HMDFS_PAGE_SIZE)

#define MAX_PATH_BUF	  256
#define MAX_DEVICE_LENGTH 21

/* DFS 1.0 defines (err base + unix err code) as customized err code */
#define DFS_1_0_ERRNO_BASE ((13UL << 21) + (1UL << 16))

static void fill_ack_header(struct hmdfs_adapter_head *head, int operations,
			    int datasize, __u64 source, int msg_id)
{
	memset(head, 0, sizeof(struct hmdfs_adapter_head));
	head->magic = HMDFS_MSG_MAGIC;
	head->version = DFS_2_0;
	head->operations = (__u8)operations;
	head->datasize = (__u32)datasize;
	head->source = source;
	head->msg_id = 0;
	head->request_id = (__u16)msg_id;
	head->flags |= HMDFS_FLAG_ENCRYPTED;
}

static int err_data_len[META_PULL_END] = {
	[READ_RESPONSE] = sizeof(struct adapter_read_ack),
	[WRITE_RESPONSE] = sizeof(struct adapter_write_ack),
	[CLOSE_RESPONSE] = sizeof(struct adapter_close_ack),
	[READ_META_RESPONSE] = sizeof(struct adapter_read_meta_ack),
	[DELETE_RESPONSE] = sizeof(__u32),
	[SYNC_RESPONSE] = sizeof(struct adapter_close_ack),
	[SET_FSIZE_RESPONSE] = sizeof(__u32),
};

static void send_err_response(struct hmdfs_peer *con, int operations,
			      __u64 source, int msg_id, int err)
{
	__u32 *status = NULL;
	int ret_len;
	struct hmdfs_send_data msg;
	struct hmdfs_adapter_head *ack_head = NULL;

	if (err > 0) {
		hmdfs_warning("invalid err %d, operations %d", err, operations);
		return;
	}

	ret_len = sizeof(struct hmdfs_adapter_head) + err_data_len[operations];

	ack_head = kzalloc(ret_len, GFP_KERNEL);
	if (!ack_head)
		return;

	fill_ack_header(ack_head, operations, ret_len, source, msg_id);
	status =
		(__u32 *)((void *)ack_head + sizeof(struct hmdfs_adapter_head));
	*status = DFS_1_0_ERRNO_BASE - err;

	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = status;
	msg.len = ret_len - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	hmdfs_sendmessage(con, &msg);
	kfree(ack_head);
}

void server_recv_handshake(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *data)
{
	int ret_size, len;
	char buf[MAX_DEVICE_LENGTH] = { 0 };
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_handshake_ack *ack_data = NULL;
	struct hmdfs_send_data msg;
	__u64 local_dno = con->sbi->local_info.iid;

	len = scnprintf(buf, MAX_DEVICE_LENGTH, "%llu", local_dno);
	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_handshake_ack) + len;
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		return;
	}
	fill_ack_header(ack_head, HANDSHAKE_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data = (struct adapter_handshake_ack
			    *)((void *)ack_head +
			       sizeof(struct hmdfs_adapter_head));
	ack_data->len = len;
	memcpy(ack_data->dev_id, buf, len);

	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	hmdfs_sendmessage(con, &msg);
	kfree(ack_head);
}

void server_recv_handshake_response(struct hmdfs_peer *con,
				    struct hmdfs_adapter_head *head, void *data)
{
	con->iid = head->source;
	connection_to_working(con);
}

void server_recv_read(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		      void *data)
{
	int err, ret_size, read_size;
	loff_t offp;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_read_ack *ack_data = NULL;
	struct file *file = NULL;
	struct adapter_read_request *req_data =
		(struct adapter_read_request *)data;
	struct hmdfs_send_data msg;
	__u32 fno = req_data->req_info.fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	file = filp_open(pathname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		hmdfs_err("open fail err %d", err);
		goto out_failed;
	}

	if (req_data->size > MAX_READ_SIZE) {
		hmdfs_warning("req_size %llu exceeds max size", req_data->size);
		read_size = MAX_READ_SIZE;
	} else {
		read_size = req_data->size;
	}
	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_read_ack) + read_size;
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, READ_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data =
		(struct adapter_read_ack *)((void *)ack_head +
					    sizeof(struct hmdfs_adapter_head));
	memset(ack_data->data_buf, 0, read_size);
	offp = vfs_llseek(file, req_data->offset, SEEK_SET);
	err = kernel_read(file, ack_data->data_buf, read_size, &offp);
	if (err < 0) {
		hmdfs_err("kernel_read failed");
		err = -EINVAL;
		goto out_failed;
	}
	ack_data->status = 0;
	ack_data->size = (__u64)err;
	ret_size -= read_size - ack_data->size;
	ack_head->datasize = ret_size;

	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, READ_RESPONSE, local_dno, head->msg_id, err);
out:
	kfree(ack_head);
	if (!IS_ERR_OR_NULL(file))
		filp_close(file, NULL);
	kfree(pathname);
}

void server_recv_writepages(struct hmdfs_peer *con,
			    struct hmdfs_adapter_head *head, void *buf)
{
	int ret_size, pos, size, err = 0, real_size = 0;
	loff_t offp;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_write_ack *ack_data = NULL;
	struct file *file = NULL;
	struct adapter_write_request *req = buf;
	struct adapter_write_data_buffer *wbuf = NULL;
	struct hmdfs_send_data msg;
	__u32 fno = req->req_info.fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	file = filp_open(pathname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		hmdfs_err("open file fail err %d", err);
		goto out_failed;
	}

	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_write_ack);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, WRITE_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data = (void *)ack_head + sizeof(struct hmdfs_adapter_head);

	for (pos = 0; pos < req->size; ) {
		int end = 0;

		end = pos + sizeof(struct adapter_write_data_buffer);
		if (end > req->size) {
			hmdfs_warning("no data for write buffer head");
			break;
		}

		wbuf = (struct adapter_write_data_buffer *)(req->data_buf + pos);
		if (wbuf->size == 0)
			break;

		size = wbuf->payload_size;
		end += size;
		if (end > req->size) {
			hmdfs_warning("no data for write buffer data");
			break;
		}

		offp = vfs_llseek(file, wbuf->offset, SEEK_SET);
		err = kernel_write(file, wbuf->buf, size, &offp);
		pos += wbuf->size;
		if (err > 0)
			real_size += err;
		else
			break;
	}

	if (err < 0) {
		err = -EINVAL;
		goto out_failed;
	}
	ack_data->status = 0;
	ack_data->size = real_size;
	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, WRITE_RESPONSE, local_dno, head->msg_id, err);
out:
	kfree(ack_head);
	if (!IS_ERR_OR_NULL(file))
		filp_close(file, NULL);
	kfree(pathname);
}

void server_recv_close(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		       void *buf)
{
	int err, ret_size;
	__u64 size, mtime, atime;
	struct file *file = NULL;
	struct inode *ino = NULL;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_close_ack *ack_data = NULL;
	struct hmdfs_send_data msg;
	struct adapter_req_info *close_req = buf;
	__u32 fno = close_req->fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	file = filp_open(pathname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		hmdfs_err("open file fail err %d", err);
		goto out_failed;
	}
	ino = file_inode(file);
	size = (__u64)i_size_read(ino);
	mtime = (__u64)(ino->i_mtime.tv_sec);
	atime = mtime;

	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_close_ack);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		err = -ENOMEM;
		hmdfs_err("ack_head alloc fail");
		goto out_failed;
	}
	fill_ack_header(ack_head, CLOSE_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data =
		(struct adapter_close_ack *)((void *)ack_head +
					     sizeof(struct hmdfs_adapter_head));
	ack_data->status = 0;
	ack_data->size = size;
	ack_data->mtime = mtime;
	ack_data->atime = atime;
	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, CLOSE_RESPONSE, local_dno, head->msg_id, err);
out:
	kfree(ack_head);
	if (!IS_ERR_OR_NULL(file))
		filp_close(file, NULL);
	kfree(pathname);
}

void server_recv_delete(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
			void *buf)
{
	int err, ret_size;
	struct path path;
	__u32 *status = NULL;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct hmdfs_send_data msg;
	struct adapter_req_info *del_req = buf;
	__u32 fno = del_req->fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	err = kern_path(pathname, LOOKUP_FOLLOW, &path);
	if (err) {
		hmdfs_info("path not exist and return");
		goto out_failed;
	}

	err = vfs_unlink(d_inode(path.dentry->d_parent), path.dentry, NULL);
	path_put(&path);
	if (err) {
		hmdfs_info("unlink path error");
		goto out_failed;
	}

	ret_size = sizeof(struct hmdfs_adapter_head) + sizeof(__u32);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, DELETE_RESPONSE, ret_size, local_dno,
			head->msg_id);
	status =
		(__u32 *)((void *)ack_head + sizeof(struct hmdfs_adapter_head));
	*status = 0;
	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = status;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, DELETE_RESPONSE, local_dno, head->msg_id, err);
out:
	kfree(ack_head);
	kfree(pathname);
}

void server_recv_read_meta(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *buf)
{
	int err, i, ret_size;
	__u32 block_count;
	__u64 size, mtime, remain_size, cur_size;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_read_meta_ack *ack_data = NULL;
	struct adapter_block_meta_base block_meta;
	struct file *file = NULL;
	struct inode *ino;
	struct hmdfs_send_data msg;
	__u32 fno = *(__u32 *)buf;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);
	file = filp_open(pathname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		/*
		 * Do not return -ENOENT here to avoid invalid meta being
		 * removed by devices with DFS 1.0.
		 * Invalid file meta (in userspace meta_service) shall only be
		 * removed by the local dfs_checker.
		 */
		hmdfs_err("open_fp fail err %d", (int)PTR_ERR(file));
		err = -EINVAL;
		goto out_failed;
	}
	ino = file_inode(file);
	size = (__u64)i_size_read(ino);
	mtime = (__u64)(ino->i_mtime.tv_sec);

	block_count = size / ADAPTER_CACHE_BLOCK_SIZE +
		      (size % ADAPTER_CACHE_BLOCK_SIZE ? 1 : 0);
	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_read_meta_ack) +
		   block_count * sizeof(struct adapter_block_meta_base);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, READ_META_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data = (struct adapter_read_meta_ack
			    *)((void *)ack_head +
			       sizeof(struct hmdfs_adapter_head));
	ack_data->mtime = mtime;
	ack_data->size = size;
	ack_data->block_count = block_count;

	remain_size = size;
	for (i = 0; i < block_count; i++) {
		cur_size = remain_size < ADAPTER_CACHE_BLOCK_SIZE ?
				   remain_size :
				   ADAPTER_CACHE_BLOCK_SIZE;
		block_meta.block_index = i;
		block_meta.size = cur_size;
		block_meta.mtime = mtime;
		memcpy((void *)(ack_data->block_list +
				i * sizeof(struct adapter_block_meta_base)),
		       &block_meta, sizeof(struct adapter_block_meta_base));
		remain_size -= cur_size;
	}

	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, READ_META_RESPONSE, local_dno, head->msg_id,
			  err);
out:
	kfree(ack_head);
	if (!IS_ERR_OR_NULL(file))
		filp_close(file, NULL);
	kfree(pathname);
}

void server_recv_sync(struct hmdfs_peer *con, struct hmdfs_adapter_head *head,
		      void *buf)
{
	int err, ret_size;
	__u64 size, mtime, atime;
	struct file *file = NULL;
	struct inode *ino = NULL;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_close_ack *ack_data = NULL;
	struct hmdfs_send_data msg;
	struct adapter_req_info *req = buf;
	__u32 fno = req->fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	file = filp_open(pathname, O_RDWR | O_LARGEFILE, 0);
	if (IS_ERR(file)) {
		err = PTR_ERR(file);
		hmdfs_err("open file fail err %d", err);
		goto out_failed;
	}
	vfs_fsync(file, 0);
	ino = file_inode(file);
	size = (__u64)i_size_read(ino);
	mtime = (__u64)(ino->i_mtime.tv_sec);
	atime = mtime;

	ret_size = sizeof(struct hmdfs_adapter_head) +
		   sizeof(struct adapter_close_ack);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, SYNC_RESPONSE, ret_size, local_dno,
			head->msg_id);

	ack_data =
		(struct adapter_close_ack *)((void *)ack_head +
					     sizeof(struct hmdfs_adapter_head));
	ack_data->status = 0;
	ack_data->size = size;
	ack_data->mtime = mtime;
	ack_data->atime = atime;
	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = ack_data;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, SYNC_RESPONSE, local_dno, head->msg_id, err);
out:
	kfree(ack_head);
	if (!IS_ERR_OR_NULL(file))
		filp_close(file, NULL);
	kfree(pathname);
}

void server_recv_set_fsize(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head, void *buf)
{
	int err, ret_size;
	struct path path;
	__u32 *status = NULL;
	struct hmdfs_adapter_head *ack_head = NULL;
	struct adapter_set_fsize_request *req_data =
		(struct adapter_set_fsize_request *)buf;
	struct hmdfs_send_data msg;
	struct adapter_req_info *req = buf;
	__u32 fno = req->fno;
	__u64 local_dno = con->sbi->local_info.iid;
	char *pathname = kmalloc(PATH_MAX, GFP_KERNEL);

	if (!pathname) {
		err = -ENOMEM;
		goto out_failed;
	}

	hmdfs_adapter_convert_file_id_to_path(fno, pathname);

	ret_size = sizeof(struct hmdfs_adapter_head) + sizeof(__u32);
	ack_head = kzalloc(ret_size, GFP_KERNEL);
	if (!ack_head) {
		hmdfs_err("ack_head alloc fail");
		err = -ENOMEM;
		goto out_failed;
	}
	fill_ack_header(ack_head, SET_FSIZE_RESPONSE, ret_size, local_dno,
			head->msg_id);
	status =
		(__u32 *)((void *)ack_head + sizeof(struct hmdfs_adapter_head));

	err = kern_path(pathname, LOOKUP_FOLLOW, &path);
	if (err) {
		hmdfs_err("pathwalk fail, errno=%d", err);
		goto out_failed;
	}
	err = vfs_truncate(&path, req_data->size);
	path_put(&path);
	if (err) {
		hmdfs_err("truncate fail, errno=%d", err);
		goto out_failed;
	}
	*status = 0;
	msg.head = ack_head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.data = status;
	msg.len = ret_size - msg.head_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	hmdfs_sendmessage(con, &msg);
	goto out;

out_failed:
	send_err_response(con, SET_FSIZE_RESPONSE, local_dno, head->msg_id,
			  err);
out:
	kfree(ack_head);
	kfree(pathname);
}

static int verify_request(struct hmdfs_peer *con,
			  struct hmdfs_adapter_head *head,
			  void *buf)
{
	int ret = 0;
	const struct hmdfs_adapter_req_cb *cb = NULL;

	cb = &adapter_s_recv_callbacks[head->operations];
	switch (cb->type) {
	case HMDFS_MSG_LEN_ANY:
		break;
	case HMDFS_MSG_LEN_FIXED:
		if (head->datasize - sizeof(*head) != cb->len)
			ret = -EINVAL;
		break;
	case HMDFS_MSG_LEN_MIN:
		if (head->datasize - sizeof(*head) < cb->len)
			ret = -EINVAL;
		break;
	default:
		ret = -EINVAL;
		break;
	}

	if (ret)
		hmdfs_err("verify failed on message %d op %d expect %d but got %u",
			  head->msg_id, head->operations,
			  cb->len, head->datasize);

	return ret;
}

void server_request_recv(struct hmdfs_peer *con,
			 struct hmdfs_adapter_head *head, void *buf)
{
	const struct cred *saved_cred = hmdfs_override_fsids(true);

	if (!saved_cred) {
		hmdfs_err("prepare cred failed!");
		return;
	}

	if (head->operations >= META_PULL_END ||
	    !adapter_s_recv_callbacks[head->operations].cb ||
	    verify_request(con, head, buf))
		return;

	adapter_s_recv_callbacks[head->operations].cb(con, head, buf);

	hmdfs_revert_fsids(saved_cred);
}
