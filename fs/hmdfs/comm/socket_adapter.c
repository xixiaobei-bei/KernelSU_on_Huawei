/* SPDX-License-Identifier: GPL-2.0 */
/*
 * socket_adapter.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: transport adapt for hmdfs
 * Author: maojingjing1@huawei.com chenjinglong1@huawei.com
 * Create: 2020-03-26
 *
 */

#include "socket_adapter.h"

#include <linux/file.h>
#include <linux/module.h>
#include <linux/namei.h>
#include <linux/net.h>
#include <linux/pagemap.h>
#include <net/sock.h>

#include "authority/authentication.h"
#include "comm/device_node.h"
#include "hmdfs_client.h"
#include "hmdfs_server.h"
#include "hmdfs_trace.h"
#include "message_verify.h"
#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#endif

#define ACQUIRE_WFIRED_INTVAL_USEC_MIN 10
#define ACQUIRE_WFIRED_INTVAL_USEC_MAX 30

typedef void (*request_callback)(struct hmdfs_peer *, struct hmdfs_head_cmd *,
				 void *);
typedef void (*response_callback)(struct hmdfs_peer *,
				  struct sendmsg_wait_queue *, void *, size_t);

static const request_callback s_recv_callbacks[F_SIZE] = {
	[F_OPEN] = hmdfs_server_open,
	[F_READPAGE] = hmdfs_server_readpage,
	[F_RELEASE] = hmdfs_server_release,
	[F_WRITEPAGE] = hmdfs_server_writepage,
	[F_ITERATE] = hmdfs_server_readdir,
	[F_MKDIR] = hmdfs_server_mkdir,
	[F_CREATE] = hmdfs_server_create,
	[F_RMDIR] = hmdfs_server_rmdir,
	[F_UNLINK] = hmdfs_server_unlink,
	[F_RENAME] = hmdfs_server_rename,
	[F_SETATTR] = hmdfs_server_setattr,
	[F_CONNECT_ECHO] = connection_recv_echo,
	[F_STATFS] = hmdfs_server_statfs,
	[F_DROP_PUSH] = hmdfs_server_get_drop_push,
	[F_GETATTR] = hmdfs_server_getattr,
	[F_FSYNC] = hmdfs_server_fsync,
	[F_SYNCFS] = hmdfs_server_syncfs,
	[F_GETXATTR] = hmdfs_server_getxattr,
	[F_SETXATTR] = hmdfs_server_setxattr,
	[F_LISTXATTR] = hmdfs_server_listxattr,
	[F_READPAGES] = hmdfs_server_readpages,
	[F_READPAGES_OPEN] = hmdfs_server_readpages_open,
	[F_ATOMIC_OPEN] = hmdfs_server_atomic_open,
};

typedef void (*file_request_callback)(struct hmdfs_peer *,
				      struct hmdfs_send_command *);

struct async_req_callbacks {
	void (*on_wakeup)(struct hmdfs_peer *peer, const struct hmdfs_req *req,
			  const struct hmdfs_resp *resp);
};

static const struct async_req_callbacks g_async_req_callbacks[F_SIZE] = {
	[F_CONNECT_ECHO] = { .on_wakeup = connection_send_echo_async_cb },
	[F_SYNCFS] = { .on_wakeup = hmdfs_recv_syncfs_cb },
	[F_WRITEPAGE] = { .on_wakeup = hmdfs_writepage_cb },
};

static void msg_release(struct kref *kref)
{
	struct sendmsg_wait_queue *msg_wq;
	struct hmdfs_peer *con;

	msg_wq = (struct sendmsg_wait_queue *)container_of(kref,
			struct hmdfs_msg_idr_head, ref);
	con = msg_wq->head.peer;
	idr_remove(&con->msg_idr, msg_wq->head.msg_id);
	spin_unlock(&con->idr_lock);

	kfree(msg_wq->buf);
	if (msg_wq->recv_info.local_filp)
		fput(msg_wq->recv_info.local_filp);
	kfree(msg_wq);
}

// Always remember to find before put, and make sure con is avilable
void msg_put(struct sendmsg_wait_queue *msg_wq)
{
	kref_put_lock(&msg_wq->head.ref, msg_release,
		      &msg_wq->head.peer->idr_lock);
}

static void recv_info_init(struct file_recv_info *recv_info)
{
	memset(recv_info, 0, sizeof(struct file_recv_info));
	atomic_set(&recv_info->local_fslices, 0);
	atomic_set(&recv_info->state, FILE_RECV_PROCESS);
}

static int msg_init(struct hmdfs_peer *con, struct sendmsg_wait_queue *msg_wq)
{
	int ret = 0;
	struct file_recv_info *recv_info = &msg_wq->recv_info;

	ret = hmdfs_alloc_msg_idr(con, MSG_IDR_MESSAGE_SYNC, msg_wq);
	if (unlikely(ret))
		return ret;

	atomic_set(&msg_wq->valid, MSG_Q_SEND);
	init_waitqueue_head(&msg_wq->response_q);
	recv_info_init(recv_info);
	msg_wq->start = jiffies;
	return 0;
}

static inline void statistic_con_sb_dirty(struct hmdfs_peer *con,
					  const struct hmdfs_cmd *op)
{
	if (op->command == F_WRITEPAGE && op->cmd_flag == C_REQUEST)
		atomic64_inc(&con->sb_dirty_count);
}

int hmdfs_sendmessage(struct hmdfs_peer *node, struct hmdfs_send_data *msg)
{
	int ret = 0;
	struct connection *connect = NULL;
	struct tcp_handle *tcp = NULL;
	struct hmdfs_head_cmd *head = msg->head;
	const struct cred *old_cred;

	if (!node) {
		hmdfs_err("node NULL when send cmd %d",
			  head->operations.command);
		ret = -EAGAIN;
		goto out_err;
	} else if (node->status != NODE_STAT_ONLINE) {
		hmdfs_err("device %llu OFFLINE %d when send cmd %d",
			  node->device_id, node->status,
			  head->operations.command);
		ret = -EAGAIN;
		goto out;
	}

	if (hmdfs_should_fail_sendmsg(&node->sbi->fault_inject, node, msg,
				      &ret))
		goto out;

	old_cred = hmdfs_override_creds(node->sbi->system_cred);

	do {
		connect = get_conn_impl(node, CONNECT_TYPE_TCP);
		if (!connect) {
			hmdfs_info_ratelimited(
				"device %llu no connection available when send cmd %d, get new session",
				node->device_id, head->operations.command);
			if (node->status != NODE_STAT_OFFLINE) {
				struct notify_param param;

				memcpy(param.remote_cid, node->cid,
				       HMDFS_CID_SIZE);
				param.notify = NOTIFY_OFFLINE;
				param.remote_iid = node->iid;
				param.fd = INVALID_SOCKET_FD;
				notify(node, &param);
			}
			ret = -EAGAIN;
			goto revert_cred;
		}

		ret = connect->send_message(connect, msg);
		if (ret == -ESHUTDOWN) {
			hmdfs_info("device %llu send cmd %d message fail, connection stop",
				   node->device_id, head->operations.command);
			connect->status = CONNECT_STAT_STOP;
			tcp = connect->connect_handle;
			if (node->status != NODE_STAT_OFFLINE) {
				connection_get(connect);
				if (!queue_work(node->reget_conn_wq,
						&connect->reget_work))
					connection_put(connect);
			}
			connection_put(connect);
			/*
			 * node->status is OFFLINE can not ensure
			 * node_seq will be increased before
			 * hmdfs_sendmessage() returns.
			 */
			hmdfs_node_inc_evt_seq(node);
		} else {
			connection_put(connect);
			goto revert_cred;
		}
	} while (node->status != NODE_STAT_OFFLINE);
revert_cred:
	hmdfs_revert_creds(old_cred);

	if (!ret)
		statistic_con_sb_dirty(node, &head->operations);
out:
	if (node->version == DFS_2_0 &&
	    head->operations.cmd_flag == C_REQUEST)
		hmdfs_client_snd_statis(node->sbi,
					head->operations.command, ret);
	else if (node->version == DFS_2_0 &&
		 head->operations.cmd_flag == C_RESPONSE)
		hmdfs_server_snd_statis(node->sbi,
					head->operations.command, ret);
out_err:
	return ret;
}

int hmdfs_sendmessage_response(struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *cmd, __u32 data_len,
			       void *buf, __u32 ret_code)
{
	int ret;
	struct hmdfs_send_data msg;
	struct hmdfs_head_cmd head;

	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.operations = cmd->operations;
	head.operations.cmd_flag = C_RESPONSE;
	head.data_len = cpu_to_le32(data_len + sizeof(struct hmdfs_head_cmd));
	head.ret_code = cpu_to_le32(ret_code);
	head.msg_id = cmd->msg_id;
	head.reserved = cmd->reserved;
	head.reserved1 = cmd->reserved1;
	msg.head = &head;
	msg.head_len = sizeof(struct hmdfs_head_cmd);
	msg.data = buf;
	msg.len = data_len;
	msg.sdesc = NULL;
	msg.sdesc_len = 0;

	ret = hmdfs_sendmessage(con, &msg);
	return ret;
}

static void mp_release(struct kref *kref)
{
	struct hmdfs_msg_parasite *mp = NULL;
	struct hmdfs_peer *peer = NULL;

	mp = (struct hmdfs_msg_parasite *)container_of(kref,
			struct hmdfs_msg_idr_head, ref);
	peer = mp->head.peer;
	idr_remove(&peer->msg_idr, mp->head.msg_id);
	spin_unlock(&peer->idr_lock);

	peer_put(peer);
	kfree(mp->resp.out_buf);
	kfree(mp);
}

void mp_put(struct hmdfs_msg_parasite *mp)
{
	kref_put_lock(&mp->head.ref, mp_release, &mp->head.peer->idr_lock);
}

static void async_request_cb_on_wakeup_fn(struct work_struct *w)
{
	struct hmdfs_msg_parasite *mp =
		container_of(w, struct hmdfs_msg_parasite, d_work.work);
	struct async_req_callbacks cbs;
	const struct cred *old_cred =
		hmdfs_override_creds(mp->head.peer->sbi->cred);

	if (mp->resp.ret_code == -ETIME)
		hmdfs_client_resp_statis(mp->head.peer->sbi,
					 mp->req.operations.command,
					 HMDFS_RESP_TIMEOUT, 0, 0);

	cbs = g_async_req_callbacks[mp->req.operations.command];
	if (cbs.on_wakeup)
		(*cbs.on_wakeup)(mp->head.peer, &mp->req, &mp->resp);
	mp_put(mp);
	hmdfs_revert_creds(old_cred);
}

static struct hmdfs_msg_parasite *mp_alloc(struct hmdfs_peer *peer,
					   const struct hmdfs_req *req)
{
	struct hmdfs_msg_parasite *mp = kzalloc(sizeof(*mp), GFP_KERNEL);
	int ret;

	if (unlikely(!mp))
		return ERR_PTR(-ENOMEM);

	ret = hmdfs_alloc_msg_idr(peer, MSG_IDR_MESSAGE_ASYNC, mp);
	if (unlikely(ret)) {
		kfree(mp);
		return ERR_PTR(ret);
	}

	mp->start = jiffies;
	peer_get(mp->head.peer);
	mp->resp.ret_code = -ETIME;
	INIT_DELAYED_WORK(&mp->d_work, async_request_cb_on_wakeup_fn);
	mp->wfired = false;
	mp->req = *req;
	return mp;
}

/**
 * hmdfs_send_async_request - sendout a async request
 * @peer: target device node
 * @req: request descriptor + necessary contexts
 *
 * Sendout a request synchronously and wait for its response asynchronously
 * Return -ESHUTDOWN when the device node is unachievable
 * Return -EAGAIN if the network is recovering
 * Return -ENOMEM if out of memory
 *
 * Register g_async_req_callbacks to recv the response
 */
int hmdfs_send_async_request(struct hmdfs_peer *peer,
			     const struct hmdfs_req *req)
{
	int ret = 0;
	struct hmdfs_send_data msg;
	struct hmdfs_head_cmd head;
	struct hmdfs_msg_parasite *mp = NULL;
	size_t msg_len = req->data_len + sizeof(struct hmdfs_head_cmd);
	unsigned int timeout;

	if (req->timeout == TIMEOUT_CONFIG)
		timeout = get_cmd_timeout(peer->sbi, req->operations.command);
	else
		timeout = req->timeout;
	if (timeout == TIMEOUT_UNINIT || timeout == TIMEOUT_NONE) {
		hmdfs_err("send msg %d with uninitialized/invalid timeout",
			  req->operations.command);
		return -EINVAL;
	}

	if (!hmdfs_is_node_online(peer))
		return -EAGAIN;

	mp = mp_alloc(peer, req);
	if (unlikely(IS_ERR(mp)))
		return PTR_ERR(mp);
	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.data_len = cpu_to_le32(msg_len);
	head.operations = mp->req.operations;
	head.msg_id = cpu_to_le32(mp->head.msg_id);
	head.reserved = 0;
	head.reserved1 = 0;

	msg.head = &head;
	msg.head_len = sizeof(head);
	msg.data = mp->req.data;
	msg.len = mp->req.data_len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	ret = hmdfs_sendmessage(peer, &msg);
	if (unlikely(ret)) {
		mp_put(mp);
		goto out;
	}

	queue_delayed_work(peer->async_wq, &mp->d_work, timeout * HZ);
	/*
	 * The work may havn't been queued upon the arriving of it's response,
	 * resulting in meaningless waiting. So we use the membar to tell the
	 * recv thread if the work has been queued
	 */
	smp_store_release(&mp->wfired, true);
out:
	hmdfs_dec_msg_idr_process(peer);
	return ret;
}

static int hmdfs_record_async_readdir(struct hmdfs_peer *con,
				      struct sendmsg_wait_queue *msg_wq)
{
	struct hmdfs_sb_info *sbi = con->sbi;

	spin_lock(&sbi->async_readdir_msg_lock);
	if (sbi->async_readdir_prohibit) {
		spin_unlock(&sbi->async_readdir_msg_lock);
		return -EINTR;
	}

	list_add(&msg_wq->async_msg, &sbi->async_readdir_msg_list);
	spin_unlock(&sbi->async_readdir_msg_lock);

	return 0;
}

static void hmdfs_untrack_async_readdir(struct hmdfs_peer *con,
					struct sendmsg_wait_queue *msg_wq)
{
	struct hmdfs_sb_info *sbi = con->sbi;

	spin_lock(&sbi->async_readdir_msg_lock);
	list_del(&msg_wq->async_msg);
	spin_unlock(&sbi->async_readdir_msg_lock);
}

int hmdfs_sendmessage_request(struct hmdfs_peer *con,
			      struct hmdfs_send_command *sm)
{
	int time_left;
	int ret = 0;
	struct sendmsg_wait_queue *msg_wq = NULL;
	struct hmdfs_send_data msg;
	size_t outlen = sm->len + sizeof(struct hmdfs_head_cmd);
	unsigned int timeout =
		get_cmd_timeout(con->sbi, sm->operations.command);
	struct hmdfs_head_cmd *head = NULL;
	bool dec = false;

	if (!hmdfs_is_node_online(con))
		return -EAGAIN;

	if (timeout == TIMEOUT_UNINIT) {
		hmdfs_err_ratelimited("send msg %d with uninitialized timeout",
				      sm->operations.command);
		return -EINVAL;
	}

	head = kzalloc(sizeof(struct hmdfs_head_cmd), GFP_KERNEL);
	if (!head)
		return -ENOMEM;

	sm->out_buf = NULL;
	head->magic = HMDFS_MSG_MAGIC;
	head->version = DFS_2_0;
	head->operations = sm->operations;
	head->data_len = cpu_to_le32(outlen);
	head->ret_code = cpu_to_le32(sm->ret_code);
	head->reserved = 0;
	head->reserved1 = 0;
	if (timeout != TIMEOUT_NONE) {
		msg_wq = kzalloc(sizeof(*msg_wq), GFP_KERNEL);
		if (!msg_wq) {
			ret = -ENOMEM;
			goto free;
		}
		ret = msg_init(con, msg_wq);
		if (ret) {
			kfree(msg_wq);
			msg_wq = NULL;
			goto free;
		}
		dec = true;
		head->msg_id = cpu_to_le32(msg_wq->head.msg_id);
		if (sm->operations.command == F_ITERATE)
			msg_wq->recv_info.local_filp = sm->local_filp;
	}
	msg.head = head;
	msg.head_len = sizeof(struct hmdfs_head_cmd);
	msg.data = sm->data;
	msg.len = sm->len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;
	ret = hmdfs_sendmessage(con, &msg);
	if (ret) {
		hmdfs_err_ratelimited("send err sm->device_id, %lld, msg_id %u",
				      con->device_id, head->msg_id);
		goto free;
	}

	if (timeout == TIMEOUT_NONE)
		goto free;

	hmdfs_dec_msg_idr_process(con);
	dec = false;

	if (sm->operations.command == F_ITERATE) {
		ret = hmdfs_record_async_readdir(con, msg_wq);
		if (ret) {
			atomic_set(&msg_wq->recv_info.state, FILE_RECV_ERR_SPC);
			goto free;
		}
	}

	time_left = wait_event_interruptible_timeout(
		msg_wq->response_q,
		(atomic_read(&msg_wq->valid) == MSG_Q_END_RECV), timeout * HZ);

	if (sm->operations.command == F_ITERATE)
		hmdfs_untrack_async_readdir(con, msg_wq);

	if (time_left == -ERESTARTSYS || time_left == 0) {
		hmdfs_err("timeout err sm->device_id %lld,  msg_id %d cmd %d",
			  con->device_id, head->msg_id,
			  head->operations.command);
		if (sm->operations.command == F_ITERATE)
			atomic_set(&msg_wq->recv_info.state, FILE_RECV_ERR_NET);
		ret = -ETIME;
		hmdfs_client_resp_statis(con->sbi, sm->operations.command,
					 HMDFS_RESP_TIMEOUT, 0, 0);
		goto free;
	}
	sm->out_buf = msg_wq->buf;
	msg_wq->buf = NULL;
	sm->out_len = msg_wq->size - sizeof(struct hmdfs_head_cmd);
	ret = msg_wq->ret;

free:
	if (msg_wq)
		msg_put(msg_wq);
	if (dec)
		hmdfs_dec_msg_idr_process(con);
	kfree(head);
	return ret;
}

static int hmdfs_send_slice(struct hmdfs_peer *con, struct hmdfs_head_cmd *cmd,
			    struct slice_descriptor *sdesc, void *slice_buf)
{
	int ret;
	struct hmdfs_send_data msg;
	struct hmdfs_head_cmd head;
	int content_size = le32_to_cpu(sdesc->content_size);
	int msg_len = sizeof(struct hmdfs_head_cmd) + content_size +
		      sizeof(struct slice_descriptor);

	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.operations = cmd->operations;
	head.operations.cmd_flag = C_RESPONSE;
	head.data_len = cpu_to_le32(msg_len);
	head.ret_code = cpu_to_le32(0);
	head.msg_id = cmd->msg_id;
	head.reserved = cmd->reserved;
	head.reserved1 = cmd->reserved1;

	msg.head = &head;
	msg.head_len = sizeof(struct hmdfs_head_cmd);
	msg.sdesc = sdesc;
	msg.sdesc_len = le32_to_cpu(sizeof(struct slice_descriptor));
	msg.data = slice_buf;
	msg.len = content_size;

	ret = hmdfs_sendmessage(con, &msg);

	return ret;
}

int hmdfs_readfile_response(struct hmdfs_peer *con, struct hmdfs_head_cmd *head,
			    struct file *filp)
{
	int ret;
	const unsigned int slice_size = PAGE_SIZE;
	char *slice_buf = NULL;
	loff_t file_offset = 0, size, file_size;
	struct slice_descriptor sdesc;
	unsigned int slice_sn = 0;

	if (!filp)
		return hmdfs_sendmessage_response(con, head, 0, NULL, 0);

	sdesc.slice_size = cpu_to_le32(slice_size);
	file_size = i_size_read(file_inode(filp));
	file_size = round_up(file_size, slice_size);
	sdesc.num_slices = cpu_to_le32(file_size / slice_size);

	slice_buf = kmalloc(slice_size, GFP_KERNEL);
	if (!slice_buf) {
		ret = -ENOMEM;
		goto out;
	}

	while (1) {
		sdesc.slice_sn = cpu_to_le32(slice_sn++);
		size = kernel_read(filp, slice_buf, (size_t)slice_size,
				   &file_offset);
		if (IS_ERR_VALUE(size)) {
			ret = (int)size;
			goto out;
		}
		sdesc.content_size = cpu_to_le32(size);
		ret = hmdfs_send_slice(con, head, &sdesc, slice_buf);
		if (ret) {
			hmdfs_info("Cannot send file slice %d ",
				   le32_to_cpu(sdesc.slice_sn));
			break;
		}
		if (file_offset >= i_size_read(file_inode(filp)))
			break;
	}

out:
	kfree(slice_buf);
	if (ret)
		hmdfs_sendmessage_response(con, head, 0, NULL, ret);
	return ret;
}

static void asw_release(struct kref *kref)
{
	struct hmdfs_async_work *asw = NULL;
	struct hmdfs_peer *peer = NULL;

	asw = (struct hmdfs_async_work *)container_of(kref,
			struct hmdfs_msg_idr_head, ref);
	peer = asw->head.peer;
	idr_remove(&peer->msg_idr, asw->head.msg_id);
	spin_unlock(&peer->idr_lock);
	kfree(asw);
}

void asw_put(struct hmdfs_async_work *asw)
{
	kref_put_lock(&asw->head.ref, asw_release, &asw->head.peer->idr_lock);
}

void hmdfs_recv_page_work_fn(struct work_struct *ptr)
{
	struct hmdfs_async_work *async_work =
		container_of(ptr, struct hmdfs_async_work, d_work.work);

	if (async_work->head.peer->version >= DFS_2_0)
		hmdfs_client_resp_statis(async_work->head.peer->sbi,
					 F_READPAGE, HMDFS_RESP_TIMEOUT, 0, 0);
	hmdfs_err_ratelimited("timeout and release page, msg_id:%u",
			      async_work->head.msg_id);
	asw_done(async_work);
}

int hmdfs_sendpage_request(struct hmdfs_peer *con,
			   struct hmdfs_send_command *sm)
{
	int ret = 0;
	struct hmdfs_send_data msg;
	struct hmdfs_async_work *async_work = NULL;
	size_t outlen = sm->len + sizeof(struct hmdfs_head_cmd);
	struct hmdfs_head_cmd head;
	unsigned int timeout;
	unsigned long start = jiffies;

	WARN_ON(!sm->out_buf);

	timeout = get_cmd_timeout(con->sbi, sm->operations.command);
	if (timeout == TIMEOUT_UNINIT) {
		hmdfs_err("send msg %d with uninitialized timeout",
			  sm->operations.command);
		ret = -EINVAL;
		goto unlock;
	}

	if (!hmdfs_is_node_online(con)) {
		ret = -EAGAIN;
		goto unlock;
	}

	memset(&head, 0, sizeof(head));
	head.magic = HMDFS_MSG_MAGIC;
	head.version = DFS_2_0;
	head.operations = sm->operations;
	head.data_len = cpu_to_le32(outlen);
	head.ret_code = cpu_to_le32(sm->ret_code);
	head.reserved = 0;
	head.reserved1 = 0;

	msg.head = &head;
	msg.head_len = sizeof(struct hmdfs_head_cmd);
	msg.data = sm->data;
	msg.len = sm->len;
	msg.sdesc_len = 0;
	msg.sdesc = NULL;

	async_work = kzalloc(sizeof(*async_work), GFP_KERNEL);
	if (!async_work) {
		ret = -ENOMEM;
		goto unlock;
	}
	async_work->start = start;
	ret = hmdfs_alloc_msg_idr(con, MSG_IDR_PAGE, async_work);
	if (ret) {
		hmdfs_err("alloc msg_id failed, err %d", ret);
		goto unlock;
	}
	head.msg_id = cpu_to_le32(async_work->head.msg_id);
	async_work->page = sm->out_buf;
	asw_get(async_work);
	INIT_DELAYED_WORK(&async_work->d_work, hmdfs_recv_page_work_fn);
	ret = queue_delayed_work(con->async_wq, &async_work->d_work,
				 timeout * HZ);
	if (!ret) {
		hmdfs_err("queue_delayed_work failed, msg_id %u", head.msg_id);
		goto fail_and_unlock_page;
	}
	ret = hmdfs_sendmessage(con, &msg);
	if (ret) {
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
	return ret;
unlock:
	kfree(async_work);
	unlock_page(sm->out_buf);
out:
	return ret;
}

static void hmdfs_request_handle_sync(struct hmdfs_peer *con,
				      struct hmdfs_head_cmd *head, void *buf)
{
	unsigned long start = jiffies;
	const struct cred *saved_cred = hmdfs_override_fsids(true);

	if (!saved_cred) {
		hmdfs_err("prepare cred failed!");
		kfree(buf);
		return;
	}

	s_recv_callbacks[head->operations.command](con, head, buf);
	hmdfs_statistic(con->sbi, head->operations.command, jiffies - start);

	kfree(buf);

	hmdfs_revert_fsids(saved_cred);
}

static void hmdfs_msg_handle_sync(struct hmdfs_peer *con,
				 struct hmdfs_head_cmd *head, void *buf)
{
	const struct cred *old_cred = hmdfs_override_creds(con->sbi->cred);

	/*
	 * Reuse PF_NPROC_EXCEEDED as an indication of hmdfs server context:
	 * 1. PF_NPROC_EXCEEDED will set by setreuid()/setuid()/setresuid(),
	 *    we assume kwork will not call theses syscalls.
	 * 2. PF_NPROC_EXCEEDED will be cleared by execv(), and kworker
	 *    will not call it.
	 */
	current->flags |= PF_NPROC_EXCEEDED;
	hmdfs_request_handle_sync(con, head, buf);
	current->flags &= ~PF_NPROC_EXCEEDED;

	hmdfs_revert_creds(old_cred);
}


static void hmdfs_request_work_fn(struct work_struct *ptr)
{
	struct work_handler_desp *desp =
		container_of(ptr, struct work_handler_desp, work);

	hmdfs_msg_handle_sync(desp->peer, desp->head, desp->buf);
	peer_put(desp->peer);
	kfree(desp->head);
	kfree(desp);
}

static int hmdfs_msg_handle_async(struct hmdfs_peer *con,
				  struct hmdfs_head_cmd *head, void *buf,
				  struct workqueue_struct *wq,
				  void (*work_fn)(struct work_struct *ptr))
{
	struct work_handler_desp *desp = NULL;
	struct hmdfs_head_cmd *dup_head = NULL;
	int ret;

	desp = kzalloc(sizeof(*desp), GFP_KERNEL);
	if (!desp) {
		ret = -ENOMEM;
		goto exit_desp;
	}

	dup_head = kzalloc(sizeof(*dup_head), GFP_KERNEL);
	if (!dup_head) {
		ret = -ENOMEM;
		goto exit_desp;
	}

	*dup_head = *head;
	desp->peer = con;
	desp->head = dup_head;
	desp->buf = buf;
	INIT_WORK(&desp->work, work_fn);

	peer_get(con);
	queue_work(wq, &desp->work);

	ret = 0;
	return ret;

exit_desp:
	kfree(desp);
	return ret;
}

static int hmdfs_request_recv(struct hmdfs_peer *con,
			      struct hmdfs_head_cmd *head, void *buf)
{
	int ret;

	if (head->operations.command >= F_SIZE ||
	    !s_recv_callbacks[head->operations.command]) {
		ret = -EINVAL;
		hmdfs_err("NULL callback, command %d",
			  head->operations.command);
		goto out;
	}

	switch (head->operations.command) {
	case F_OPEN:
	case F_RELEASE:
	case F_ITERATE:
	case F_MKDIR:
	case F_RMDIR:
	case F_CREATE:
	case F_UNLINK:
	case F_RENAME:
	case F_SETATTR:
	case F_CONNECT_ECHO:
	case F_STATFS:
	case F_CONNECT_REKEY:
	case F_DROP_PUSH:
	case F_GETATTR:
	case F_FSYNC:
	case F_SYNCFS:
	case F_GETXATTR:
	case F_SETXATTR:
	case F_LISTXATTR:
	case F_READPAGES_OPEN:
	case F_ATOMIC_OPEN:
		ret = hmdfs_msg_handle_async(con, head, buf, con->req_handle_wq,
					     hmdfs_request_work_fn);
		break;
	case F_WRITEPAGE:
	case F_READPAGE:
	case F_READPAGES:
		hmdfs_msg_handle_sync(con, head, buf);
		ret = 0;
		break;
	default:
		hmdfs_err("Fatal! Unexpected request command %d",
			  head->operations.command);
		ret = -EINVAL;
	}

out:
	return ret;
}

void hmdfs_response_wakeup(struct sendmsg_wait_queue *msg_info,
			   __u32 ret_code, __u32 data_len, void *buf)
{
	msg_info->ret = ret_code;
	msg_info->size = data_len;
	msg_info->buf = buf;
	atomic_set(&msg_info->valid, MSG_Q_END_RECV);
	wake_up_interruptible(&msg_info->response_q);
}

static int hmdfs_readfile_slice(struct sendmsg_wait_queue *msg_info,
				struct work_handler_desp *desp)
{
	struct slice_descriptor *sdesc = desp->buf;
	void *slice_buf = sdesc + 1;
	struct file_recv_info *recv_info = &msg_info->recv_info;
	struct file *filp = recv_info->local_filp;
	loff_t offset, written_size;

	if (atomic_read(&recv_info->state) != FILE_RECV_PROCESS)
		return -EBUSY;

	offset = le32_to_cpu(sdesc->slice_size) * le32_to_cpu(sdesc->slice_sn);

	written_size = kernel_write(filp, slice_buf,
				    le32_to_cpu(sdesc->content_size), &offset);
	if (IS_ERR_VALUE(written_size)) {
		atomic_set(&recv_info->state, FILE_RECV_ERR_SPC);
		hmdfs_info("Fatal! Cannot store a file slice %d/%d, ret = %d",
			   le32_to_cpu(sdesc->slice_sn),
			   le32_to_cpu(sdesc->num_slices), (int)written_size);
		return (int)written_size;
	}

	if (atomic_inc_return(&recv_info->local_fslices) >=
	    le32_to_cpu(sdesc->num_slices))
		atomic_set(&recv_info->state, FILE_RECV_SUCC);
	return 0;
}

static void hmdfs_file_response_work_fn(struct work_struct *ptr)
{
	struct work_handler_desp *desp =
		container_of(ptr, struct work_handler_desp, work);
	struct sendmsg_wait_queue *msg_info = NULL;
	int ret;
	atomic_t *pstate = NULL;
	u8 cmd = desp->head->operations.command;
	const struct cred *old_cred =
		hmdfs_override_creds(desp->peer->sbi->cred);

	msg_info = (struct sendmsg_wait_queue *)hmdfs_find_msg_head(desp->peer,
					le32_to_cpu(desp->head->msg_id));
	if (!msg_info || atomic_read(&msg_info->valid) != MSG_Q_SEND) {
		hmdfs_client_resp_statis(desp->peer->sbi, cmd, HMDFS_RESP_DELAY,
					 0, 0);
		hmdfs_info("cannot find msg(id %d)",
			   le32_to_cpu(desp->head->msg_id));
		goto free;
	}

	ret = le32_to_cpu(desp->head->ret_code);
	if (ret || le32_to_cpu(desp->head->data_len) == sizeof(*desp->head))
		goto wakeup;
	ret = hmdfs_readfile_slice(msg_info, desp);
	pstate = &msg_info->recv_info.state;
	if (ret || atomic_read(pstate) != FILE_RECV_PROCESS)
		goto wakeup;
	goto free;

wakeup:
	hmdfs_response_wakeup(msg_info, ret, sizeof(struct hmdfs_head_cmd),
			      NULL);
	hmdfs_client_resp_statis(desp->peer->sbi, cmd, HMDFS_RESP_NORMAL,
				 msg_info->start, jiffies);
free:
	if (msg_info)
		msg_put(msg_info);
	peer_put(desp->peer);
	hmdfs_revert_creds(old_cred);

	kfree(desp->buf);
	kfree(desp->head);
	kfree(desp);
}

static void hmdfs_wait_mp_wfired(struct hmdfs_msg_parasite *mp)
{
	/* We just cancel queued works */
	while (unlikely(!smp_load_acquire(&mp->wfired)))
		usleep_range(ACQUIRE_WFIRED_INTVAL_USEC_MIN,
			     ACQUIRE_WFIRED_INTVAL_USEC_MAX);
}

int hmdfs_response_handle_sync(struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *head, void *buf)
{
	struct sendmsg_wait_queue *msg_info = NULL;
	struct hmdfs_msg_parasite *mp = NULL;
	struct hmdfs_msg_idr_head *msg_head = NULL;
	u32 msg_id = le32_to_cpu(head->msg_id);
	bool woke = false;
	u8 cmd = head->operations.command;

	msg_head = hmdfs_find_msg_head(con, msg_id);
	if (!msg_head)
		goto out;

	switch (msg_head->type) {
	case MSG_IDR_MESSAGE_SYNC:
		msg_info = (struct sendmsg_wait_queue *)msg_head;
		if (atomic_read(&msg_info->valid) == MSG_Q_SEND) {
			hmdfs_response_wakeup(msg_info,
					      le32_to_cpu(head->ret_code),
					      le32_to_cpu(head->data_len), buf);
			hmdfs_client_resp_statis(con->sbi, cmd,
						 HMDFS_RESP_NORMAL,
						 msg_info->start, jiffies);
			woke = true;
		}

		msg_put(msg_info);
		break;
	case MSG_IDR_MESSAGE_ASYNC:
		mp = (struct hmdfs_msg_parasite *)msg_head;

		hmdfs_wait_mp_wfired(mp);
		if (cancel_delayed_work(&mp->d_work)) {
			mp->resp.out_buf = buf;
			mp->resp.out_len =
				le32_to_cpu(head->data_len) - sizeof(*head);
			mp->resp.ret_code = le32_to_cpu(head->ret_code);
			queue_delayed_work(con->async_wq, &mp->d_work, 0);
			hmdfs_client_resp_statis(con->sbi, cmd,
						 HMDFS_RESP_NORMAL, mp->start,
						 jiffies);
			woke = true;
		}
		mp_put(mp);
		break;
	default:
		hmdfs_err("receive incorrect msg type %d msg_id %d cmd %d",
			  msg_head->type, msg_id, cmd);
		break;
	}

	if (likely(woke))
		return 0;
out:
	hmdfs_client_resp_statis(con->sbi, cmd, HMDFS_RESP_DELAY, 0, 0);
	hmdfs_info("cannot find msg_id %d cmd %d", msg_id, cmd);
	return -EINVAL;
}

static int hmdfs_response_recv(struct hmdfs_peer *con,
			       struct hmdfs_head_cmd *head, void *buf)
{
	__u16 command = head->operations.command;
	int ret;

	if (command >= F_SIZE) {
		ret = -EINVAL;
		return ret;
	}

	switch (head->operations.command) {
	case F_OPEN:
	case F_RELEASE:
	case F_READPAGE:
	case F_WRITEPAGE:
	case F_MKDIR:
	case F_RMDIR:
	case F_CREATE:
	case F_UNLINK:
	case F_RENAME:
	case F_SETATTR:
	case F_CONNECT_ECHO:
	case F_STATFS:
	case F_CONNECT_REKEY:
	case F_DROP_PUSH:
	case F_GETATTR:
	case F_FSYNC:
	case F_SYNCFS:
	case F_GETXATTR:
	case F_SETXATTR:
	case F_LISTXATTR:
		ret = hmdfs_response_handle_sync(con, head, buf);
		return ret;

	case F_ITERATE:
		ret = hmdfs_msg_handle_async(con, head, buf, con->async_wq,
					     hmdfs_file_response_work_fn);
		return ret;

	default:
		hmdfs_err("Fatal! Unexpected response command %d",
			  head->operations.command);
		ret = -EINVAL;
		return ret;
	}
}

static void hmdfs_recv_mesg_callback(struct hmdfs_peer *con, void *head,
				     void *buf)
{
	struct hmdfs_head_cmd *hmdfs_head = (struct hmdfs_head_cmd *)head;

	trace_hmdfs_recv_mesg_callback(hmdfs_head);

	if (hmdfs_message_verify(con, hmdfs_head, buf) < 0) {
		hmdfs_info("Message %d has been abandoned", hmdfs_head->msg_id);
		goto out_err;
	}

	switch (hmdfs_head->operations.cmd_flag) {
	case C_REQUEST:
		if (hmdfs_request_recv(con, hmdfs_head, buf) < 0)
			goto out_err;
		break;

	case C_RESPONSE:
		if (hmdfs_response_recv(con, hmdfs_head, buf) < 0)
			goto out_err;
		break;

	default:
		hmdfs_err("Fatal! Unexpected msg cmd %d",
			  hmdfs_head->operations.cmd_flag);
		break;
	}
	return;

out_err:
	kfree(buf);
}

static inline void hmdfs_recv_page_callback(struct hmdfs_peer *con,
					    struct hmdfs_head_cmd *head,
					    int err, void *data)
{
	if (head->operations.command == F_READPAGE)
		hmdfs_client_recv_readpage(head, err, data);
}

static const struct connection_operations conn_operations[] = {
#ifdef CONFIG_HMDFS_1_0
	[USERDFS_VERSION] = {
		.recvmsg = hmdfs_adapter_recv_mesg_callback,
		.remote_file_fops = &hmdfs_adapter_remote_file_fops,
		.remote_file_iops = &hmdfs_adapter_remote_file_iops,
		.remote_file_aops = &hmdfs_adapter_remote_file_aops,
		.remote_unlink = hmdfs_adapter_remote_unlink,
		.remote_readdir = hmdfs_adapter_remote_readdir,
		.remote_lookup = hmdfs_adapter_remote_lookup,
	},
#endif
	[PROTOCOL_VERSION] = {
		.recvmsg = hmdfs_recv_mesg_callback,
		.recvpage = hmdfs_recv_page_callback,
		/* remote device operations */
		.remote_file_fops =
			&hmdfs_dev_file_fops_remote,
		.remote_file_iops =
			&hmdfs_dev_file_iops_remote,
		.remote_file_aops =
			&hmdfs_dev_file_aops_remote,
		.remote_unlink =
			hmdfs_dev_unlink_from_con,
		.remote_readdir =
			hmdfs_dev_readdir_from_con,
	}
};

const struct connection_operations *hmdfs_get_peer_operation(__u8 version)
{
	if (version <= INVALID_VERSION || version >= MAX_VERSION)
		return NULL;

	if (version <= USERSPACE_MAX_VER)
		return &(conn_operations[USERDFS_VERSION]);
	else
		return &(conn_operations[PROTOCOL_VERSION]);
}

void hmdfs_wakeup_parasite(struct hmdfs_msg_parasite *mp)
{
	hmdfs_wait_mp_wfired(mp);
	if (!cancel_delayed_work(&mp->d_work))
		hmdfs_err("cancel parasite work err msg_id=%d cmd=%d",
			  mp->head.msg_id, mp->req.operations.command);
	else
		async_request_cb_on_wakeup_fn(&mp->d_work.work);
}

void hmdfs_wakeup_async_work(struct hmdfs_async_work *async_work)
{
	if (!cancel_delayed_work(&async_work->d_work))
		hmdfs_err("cancel async work err msg_id=%d",
			  async_work->head.msg_id);
	else
		hmdfs_recv_page_work_fn(&async_work->d_work.work);
}
