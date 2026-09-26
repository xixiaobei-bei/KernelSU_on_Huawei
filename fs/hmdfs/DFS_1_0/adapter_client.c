/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_client.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: koujilong@huawei.com
 *         chenyi77@huawei.com
 * Create: 2020-04-17
 *
 */

#include "adapter_client.h"

#include <linux/highmem.h>
#include <linux/slab.h>

#include "dentry_syncer.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_device_view.h"

static void msg_remove_id(struct hmdfs_peer *con, int id)
{
	spin_lock(&con->idr_lock);
	idr_remove(&con->msg_idr, id);
	spin_unlock(&con->idr_lock);
}

static int msg_init(struct hmdfs_peer *con, struct sendmsg_wait_queue *msg_wq)
{
	int ret;

	ret = hmdfs_alloc_msg_idr(con, MSG_IDR_1_0_MESSAGE_SYNC, msg_wq);
	if (!ret) {
		atomic_set(&msg_wq->valid, MSG_Q_SEND);
		init_waitqueue_head(&msg_wq->response_q);
	}

	return ret;
}

void client_recv_read(struct sendmsg_wait_queue *msg_info, void *buf,
		      size_t len)
{
	struct adapter_read_ack *recv = buf;
	struct hmdfs_async_work *async_work =
		(struct hmdfs_async_work *)msg_info;
	struct page *page = async_work->page;
	void *addr;

	if (recv->size == 0)
		hmdfs_warning("recv size is 0");

	if (recv->status != 0)
		goto out_err;

	if (len != recv->size + sizeof(struct adapter_read_ack)) {
		hmdfs_warning("recv size %llu but data size is %llu",
			      recv->size, len - recv->size);
		goto out_err;
	}

	addr = kmap_atomic(page);
	memcpy(addr, recv->data_buf,
	       recv->size <= HMDFS_PAGE_SIZE ? recv->size : HMDFS_PAGE_SIZE);
	kunmap_atomic(addr);
	SetPageUptodate(page);
out_err:
	asw_done(async_work);
}

void client_recv_writepage(struct sendmsg_wait_queue *msg_info, void *buf,
			   size_t len)
{
	struct adapter_write_ack *recv = buf;

	if (recv->status != 0)
		hmdfs_warning("the write request fail");
	head_put(&msg_info->head);
}

static struct metabase *lookup_file(struct file *file)
{
	int err = 0;
	struct hmdfs_inode_info *info = hmdfs_i(file_inode(file));
	struct metabase *lookup_result = NULL;
	struct lookup_request *req = NULL;
	char *account = info->conn->sbi->local_info.account;
	char *relative_path = NULL;
	char *path = NULL;
	int relative_path_len;
	int path_len;
	int real_len;

	relative_path = hmdfs_get_dentry_relative_path(file->f_path.dentry);
	if (!relative_path) {
		hmdfs_warning("get relative path fail");
		return NULL;
	}

	relative_path_len = strlen(relative_path);
	path_len = relative_path_len + strlen(account) + 2;
	path = kzalloc(path_len, GFP_KERNEL);
	if (!path)
		goto out_get_path;

	snprintf(path, path_len, "/%s%s", account, relative_path);
	real_len = strlen(path);

	req = kzalloc(sizeof(*req) + real_len + 1, GFP_KERNEL);
	if (!req)
		goto out_get_req;

	req->device_id = info->conn->iid;
	req->name_len = 0;
	req->path_len = real_len;
	memcpy(req->path, path, real_len + 1);

	path_len += sizeof(*lookup_result);
	lookup_result = kzalloc(path_len, GFP_KERNEL);
	if (!lookup_result)
		goto out_get_result;

	err = dentry_syncer_lookup(req, lookup_result, path_len);
	if (err) {
		kfree(lookup_result);
		lookup_result = NULL;
		hmdfs_warning("look up file fail");
	}

out_get_result:
	kfree(req);
out_get_req:
	kfree(path);
out_get_path:
	kfree(relative_path);
	return lookup_result;
}

void client_recv_close(struct sendmsg_wait_queue *msg_info, void *buf,
		       size_t len)
{
	struct file *file = (struct file *)(msg_info->buf);
	struct inode *inode = file_inode(file);
	struct adapter_close_ack *recv = buf;
	struct metabase *result = NULL;

	if (recv->status == 0) {
		inode_lock(inode);
		i_size_write(inode, recv->size);
		inode->i_atime.tv_sec = recv->atime;
		inode->i_mtime.tv_sec = recv->mtime;
		inode_unlock(inode);

		result = lookup_file(file);
		if (result) {
			result->size = i_size_read(inode);
			result->atime = recv->atime;
			result->mtime = recv->mtime;
			dentry_syncer_update(result);
		}
	} else {
		hmdfs_err("close remote file fail, status = %u", recv->status);
	}

	kfree(result);
	msg_info->ret = recv->status;
}

void client_recv_delete(struct sendmsg_wait_queue *msg_info, void *buf,
			size_t len)
{
	__u32 *status = buf;

	if (*status != 0)
		hmdfs_info("delete remote file fail, status = %u", *status);
	msg_info->ret = *status;
}

void client_recv_setfssize(struct sendmsg_wait_queue *msg_info, void *buf,
			   size_t len)
{
	__u32 *status = buf;

	if (*status != 0)
		hmdfs_info("set remote file truncate fail, status = %u",
			   *status);
	msg_info->ret = *status;
}

int client_sendpage_request(struct hmdfs_peer *con, struct adapter_sendmsg *sm)
{
	int err = 0;
	struct hmdfs_send_data msg;
	struct hmdfs_async_work *async_work = NULL;
	size_t outlen = sm->len + sizeof(struct hmdfs_adapter_head);
	struct hmdfs_adapter_head head = {0};

	if (!sm->outmsg) {
		hmdfs_err("PAGE request send err sm->device_id = %lld",
			  con->device_id);
		return -EINVAL;
	}

	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.operations = sm->operations;
	head.datasize = outlen;
	head.source = con->sbi->local_info.iid;
	head.flags |= HMDFS_FLAG_ENCRYPTED;

	msg.data = sm->data;
	msg.len = sm->len;
	msg.head = &head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	async_work = kzalloc(sizeof(*async_work), GFP_KERNEL);
	if (!async_work) {
		err = -ENOMEM;
		goto unlock;
	}
	err = hmdfs_alloc_msg_idr(con, MSG_IDR_1_0_PAGE, async_work);
	if (err) {
		hmdfs_err("id alloc failed, err=%d", err);
		goto unlock;
	}
	head.msg_id = async_work->head.msg_id;
	async_work->page = sm->outmsg;
	asw_get(async_work);
	INIT_DELAYED_WORK(&async_work->d_work, hmdfs_recv_page_work_fn);
	err = queue_delayed_work(con->async_wq, &async_work->d_work,
				 sm->timeout * HZ);
	if (!err) {
		hmdfs_err("queue_delayed_work failed, msg_id %u", head.msg_id);
		goto fail_and_unlock_page;
	}
	err = hmdfs_sendmessage(con, &msg);
	if (err) {
		hmdfs_err("send err sm->device_id, %lld, msg_id %u",
			  con->device_id, head.msg_id);
		if (!cancel_delayed_work(&async_work->d_work)) {
			hmdfs_err("cancel async work err");
			asw_put(async_work);
			hmdfs_dec_msg_idr_process(con);
			goto out;
		}
		goto fail_and_unlock_page;
	}
	asw_put(async_work);
	hmdfs_dec_msg_idr_process(con);
	return 0;

fail_and_unlock_page:
	asw_put(async_work);
	asw_done(async_work);
	hmdfs_dec_msg_idr_process(con);
	return err;
unlock:
	kfree(async_work);
	unlock_page(sm->outmsg);
out:
	return err;
}

int client_sendmessage_request(struct hmdfs_peer *con,
			       struct adapter_sendmsg *sm)
{
	int err = 0;
	int time_left;
	struct hmdfs_send_data msg;
	struct sendmsg_wait_queue *msg_wq = NULL;
	struct hmdfs_msg_idr_head *idr_head = NULL;
	size_t outlen = sm->len + sizeof(struct hmdfs_adapter_head);
	struct hmdfs_adapter_head head = {0};
	bool dec = false;

	if (sm->timeout == 0) {
		idr_head = kzalloc(sizeof(*idr_head), GFP_KERNEL);
		if (!idr_head) {
			err = -ENOMEM;
			goto out;
		}
		/* Now, only write request will rearch here, and sm->outmsg is
		 * NULL, so just pass NULL. For future, if you want to add ptr,
		 * you must pass ptr which begin with hmdfs_msg_idr_head to
		 * hmdfs_alloc_msg_idr, and you can add new MSG_IDR type. We
		 * mange ptr with kref
		 */
		err = hmdfs_alloc_msg_idr(con, MSG_IDR_1_0_NONE, idr_head);
		if (err) {
			hmdfs_err("id alloc failed, err=%d", err);
			goto out;
		}
		head.msg_id = idr_head->msg_id;
	} else {
		msg_wq = kzalloc(sizeof(*msg_wq), GFP_KERNEL);
		if (!msg_wq) {
			err = -ENOMEM;
			goto out;
		}

		err = msg_init(con, msg_wq);
		if (err) {
			hmdfs_err("msg_init failed");
			goto out;
		}
		if (sm->operations == CLOSE_REQUEST)
			msg_wq->buf = sm->outmsg;
		head.msg_id = msg_wq->head.msg_id;
	}

	dec = true;
	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.operations = sm->operations;
	head.datasize = outlen;
	head.source = con->sbi->local_info.iid;
	head.flags |= HMDFS_FLAG_ENCRYPTED;

	msg.data = sm->data;
	msg.len = sm->len;
	msg.head = &head;
	msg.head_len = sizeof(struct hmdfs_adapter_head);
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	err = hmdfs_sendmessage(con, &msg);
	if (err) {
		hmdfs_err("send err to device_id, %lld, msg_id %u",
			  con->device_id, head.msg_id);
		goto free;
	}

	hmdfs_dec_msg_idr_process(con);
	dec = false;
	if (sm->timeout == 0)
		return 0;

	time_left = wait_event_interruptible_timeout(
		msg_wq->response_q,
		(atomic_read(&msg_wq->valid) == MSG_Q_END_RECV),
		sm->timeout * HZ);

	if (time_left == -ERESTARTSYS || time_left == 0) {
		hmdfs_err("timeout err device_id %llu, msg_id %d cmd %d",
			  con->device_id, head.msg_id, head.operations);
		err = -ETIMEDOUT;
		goto free;
	}

	err = (int)msg_wq->ret;
free:
	if (msg_wq) {
		msg_wq->buf = NULL;
		msg_put(msg_wq);
	}
	if (idr_head)
		head_put(idr_head);
	if (dec)
		hmdfs_dec_msg_idr_process(con);

	return err;
out:
	kfree(msg_wq);
	kfree(idr_head);

	return err;
}

static int verify_response(struct hmdfs_peer *con,
			   struct hmdfs_adapter_head *head,
			   void *buf)
{
	int ret = 0;
	const struct hmdfs_adapter_resp_cb *cb = NULL;

	cb = &recv_callbacks[head->operations];
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
		hmdfs_err("verify failed on message %d with op %d expect %d but got %u",
			  head->msg_id, head->operations,
			  cb->len, head->datasize);

	return ret;
}

void client_response_recv(struct hmdfs_peer *con,
			  struct hmdfs_adapter_head *head, void *buf)
{
	int id = head->request_id;
	struct sendmsg_wait_queue *msg_info = NULL;
	struct hmdfs_async_work *async_work = NULL;
	size_t len = head->datasize - sizeof(*head);
	enum ADAPTER_MSG_OPERATION op = head->operations;

	spin_lock(&con->idr_lock);
	msg_info = idr_find(&con->msg_idr, id);
	if (!msg_info || op >= META_PULL_END || !recv_callbacks[op].cb) {
		idr_remove(&con->msg_idr, id);
		kfree(msg_info);
		msg_info = NULL;
	} else {
		kref_get(&msg_info->head.ref);
	}
	spin_unlock(&con->idr_lock);

	if (!msg_info) {
		hmdfs_err("idr_head/callback is NULL op=%u msg_id=%d", op, id);
		return;
	}

	if (verify_response(con, head, buf)) {
		hmdfs_info("Message %d has been abandoned", id);
		return;
	}

	switch (op) {
	case READ_RESPONSE:
		async_work = (struct hmdfs_async_work *)msg_info;
		if (!cancel_delayed_work(&async_work->d_work))
			hmdfs_err("cancel delay work err");
		else
			recv_callbacks[op].cb(msg_info, buf, len);
		asw_put(async_work);
		break;
	case WRITE_RESPONSE:
		recv_callbacks[op].cb(msg_info, buf, len);
		head_put((struct hmdfs_msg_idr_head *)msg_info);
		break;
	case CLOSE_RESPONSE:
	case DELETE_RESPONSE:
	case SET_FSIZE_RESPONSE:
		if (atomic_read(&msg_info->valid) == MSG_Q_SEND) {
			recv_callbacks[op].cb(msg_info, buf, len);
			atomic_set(&msg_info->valid, MSG_Q_END_RECV);
			wake_up_interruptible(&msg_info->response_q);
		}
		msg_put(msg_info);
		break;
	default:
		msg_remove_id(con, id);
		kfree(msg_info);
		hmdfs_err("Receive unsupport msg op=%d id=%d", op, id);
		break;
	}
}
