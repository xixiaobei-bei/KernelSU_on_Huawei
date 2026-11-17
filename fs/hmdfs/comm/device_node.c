/* SPDX-License-Identifier: GPL-2.0 */
/*
 * device_node.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Description: send and recv msg for hmdfs
 * Author: liuxuesong3@huawei.com
 * Create: 2020-04-14
 *
 */

#include "device_node.h"

#include <linux/errno.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/kfifo.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/types.h>
#include <linux/backing-dev.h>

#include "client_writeback.h"
#include "server_writeback.h"
#include "connection.h"
#include "hmdfs_client.h"
#include "socket_adapter.h"
#include "authority/authentication.h"

DEFINE_MUTEX(hmdfs_sysfs_mutex);
static struct kset *hmdfs_kset;

struct hmdfs_disconnect_node_work {
	struct hmdfs_peer *conn;
	struct work_struct work;
	atomic_t *cnt;
	struct wait_queue_head *waitq;
};

static void ctrl_cmd_init_handler(const char *buf, size_t len,
				  struct hmdfs_sb_info *sbi)
{
	struct init_param cmd;

	if (unlikely(!buf || len != sizeof(cmd))) {
		hmdfs_err("len/buf error");
		return;
	}
	memcpy(&cmd, buf, sizeof(cmd));
	sbi->local_info.iid = cmd.local_iid;
	strncpy(sbi->local_info.account, cmd.current_account,
		HMDFS_ACCOUNT_HASH_MAX_LEN);
}

static void ctrl_cmd_update_socket_handler(const char *buf, size_t len,
					   struct hmdfs_sb_info *sbi)
{
	struct update_socket_param cmd;
	struct hmdfs_peer *node = NULL;
	struct connection *conn = NULL;

	if (unlikely(!buf || len != sizeof(cmd))) {
		hmdfs_err("len/buf error");
		goto out;
	}
	memcpy(&cmd, buf, sizeof(cmd));

	node = hmdfs_get_peer(sbi, cmd.cid, cmd.local_iid);
	if (unlikely(!node)) {
		hmdfs_err("failed to update ctrl node: cannot get peer");
		goto out;
	}

	conn = hmdfs_get_conn_tcp(node, cmd.newfd, cmd.masterkey, cmd.status);
	if (unlikely(!conn)) {
		hmdfs_err("failed to update ctrl node: cannot get conn");
	} else if (!sbi->system_cred) {
		const struct cred *system_cred = get_cred(current_cred());

		if (cmpxchg_relaxed(&sbi->system_cred, NULL, system_cred))
			put_cred(system_cred);
		else
			hmdfs_check_cred(system_cred);
	}
out:
	if (conn)
		connection_put(conn);
	if (node)
		peer_put(node);
}

static void inline hmdfs_disconnect_node_marked(struct hmdfs_peer *conn)
{
	hmdfs_start_process_offline(conn);
	hmdfs_disconnect_node(conn);
	hmdfs_stop_process_offline(conn);
}

static void ctrl_cmd_off_line_handler(const char *buf, size_t len,
				      struct hmdfs_sb_info *sbi)
{
	struct offline_param cmd;
	struct hmdfs_peer *node = NULL;
	struct notify_param param;

	if (unlikely(!buf || len != sizeof(cmd))) {
		hmdfs_err("Recved a invalid userbuf");
		return;
	}
	memcpy(&cmd, buf, sizeof(cmd));
	node = hmdfs_lookup_from_cid(sbi, cmd.remote_cid);
	if (unlikely(!node)) {
		hmdfs_err("Cannot find node by device");
		return;
	}
	hmdfs_info("Found peer: device_id = %llu", node->device_id);
	hmdfs_disconnect_node_marked(node);
	// notify remote iid
	param.notify = NOTIFY_OFFLINE_IID;
	param.fd = INVALID_SOCKET_FD;
	param.remote_iid = node->iid;
	memcpy(param.remote_cid, node->cid, HMDFS_CID_SIZE);
	notify(node, &param);
	peer_put(node);
}

static void hmdfs_disconnect_node_work_fn(struct work_struct *base)
{
	struct hmdfs_disconnect_node_work *work =
		container_of(base, struct hmdfs_disconnect_node_work, work);

	hmdfs_disconnect_node_marked(work->conn);
	if (atomic_dec_and_test(work->cnt))
		wake_up(work->waitq);
	kfree(work);
}

static void ctrl_cmd_off_line_all_handler(const char *buf, size_t len,
					  struct hmdfs_sb_info *sbi)
{
	struct hmdfs_peer *node = NULL;
	struct hmdfs_disconnect_node_work *work = NULL;
	atomic_t cnt = ATOMIC_INIT(0);
	wait_queue_head_t waitq;

	if (unlikely(len != sizeof(struct offline_all_param))) {
		hmdfs_err("Recved a invalid userbuf, len %zu, expect %zu\n",
			  len, sizeof(struct offline_all_param));
		return;
	}

	init_waitqueue_head(&waitq);
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(node, &sbi->connections.node_list, list) {
		mutex_unlock(&sbi->connections.node_lock);
		work = kmalloc(sizeof(*work), GFP_KERNEL);
		if (work) {
			atomic_inc(&cnt);
			work->conn = node;
			work->cnt = &cnt;
			work->waitq = &waitq;
			INIT_WORK(&work->work, hmdfs_disconnect_node_work_fn);
			schedule_work(&work->work);
		} else {
			hmdfs_disconnect_node_marked(node);
		}
		mutex_lock(&sbi->connections.node_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);

	wait_event(waitq, !atomic_read(&cnt));
}

static void ctrl_cmd_set_account(const char *buf, size_t len,
				 struct hmdfs_sb_info *sbi)
{
	struct set_account_param cmd;

	if (unlikely(!buf || len != sizeof(cmd))) {
		hmdfs_err("len/buf error");
		return;
	}
	memcpy(&cmd, buf, sizeof(cmd));
	strncpy(sbi->local_info.account, cmd.current_account,
		HMDFS_ACCOUNT_HASH_MAX_LEN);
}

typedef void (*ctrl_cmd_handler)(const char *buf, size_t len,
				 struct hmdfs_sb_info *sbi);

static const ctrl_cmd_handler cmd_handler[CMD_CNT] = {
	[CMD_INIT] = ctrl_cmd_init_handler,
	[CMD_UPDATE_SOCKET] = ctrl_cmd_update_socket_handler,
	[CMD_OFF_LINE] = ctrl_cmd_off_line_handler,
	[CMD_SET_ACCOUNT] = ctrl_cmd_set_account,
	[CMD_OFF_LINE_ALL] = ctrl_cmd_off_line_all_handler,
};

static ssize_t sbi_cmd_show(struct kobject *kobj, struct sbi_attribute *attr,
			    char *buf)
{
	struct notify_param param;
	int out_len;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	memset(&param, 0, sizeof(param));
	spin_lock(&sbi->notify_fifo_lock);
	out_len = kfifo_out(&sbi->notify_fifo, &param, sizeof(param));
	spin_unlock(&sbi->notify_fifo_lock);
	if (out_len != sizeof(param))
		param.notify = NOTIFY_NONE;
	memcpy(buf, &param, sizeof(param));
	return sizeof(param);
}

static const char *cmd2str(int cmd)
{
	switch (cmd) {
	case 0:
		return "CMD_INIT";
	case 1:
		return "CMD_UPDATE_SOCKET";
	case 2:
		return "CMD_OFF_LINE";
	case 3:
		return "CMD_SET_ACCOUNT";
	case 4:
		return "CMD_OFF_LINE_ALL";
	default:
		return "illegal cmd";
	}
}

static ssize_t sbi_cmd_store(struct kobject *kobj, struct sbi_attribute *attr,
			     const char *buf, size_t len)
{
	int cmd;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	if (!sbi) {
		hmdfs_info("Fatal! Empty sbi. Mount fs first");
		return len;
	}
	if (len < sizeof(int)) {
		hmdfs_err("Illegal cmd: cmd len = %lu", len);
		return len;
	}
	cmd = *(int *)buf;
	if (cmd < 0 || cmd >= CMD_CNT) {
		hmdfs_err("Illegal cmd : cmd = %d", cmd);
		return len;
	}
	hmdfs_info("Recved cmd: %s", cmd2str(cmd));
	if (cmd_handler[cmd])
		cmd_handler[cmd](buf, len, sbi);
	return len;
}

static struct sbi_attribute sbi_cmd_attr =
	__ATTR(cmd, 0664, sbi_cmd_show, sbi_cmd_store);

static ssize_t sbi_status_show(struct kobject *kobj, struct sbi_attribute *attr,
			       char *buf)
{
	ssize_t size = 0;
	struct hmdfs_sb_info *sbi = NULL;
	struct hmdfs_peer *peer = NULL;
	struct connection *conn_impl = NULL;
	struct tcp_handle *tcp = NULL;

	sbi = to_sbi(kobj);
	size += sprintf(buf + size, "peers  version  status\n");

	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(peer, &sbi->connections.node_list, list) {
		size += sprintf(buf + size, "%llu  %d  %d\n", peer->device_id,
				peer->version, peer->status);
		// connection information
		size += sprintf(
			buf + size,
			"\t socket_fd  connection_status  tcp_status  ... refcnt\n");
		mutex_lock(&peer->conn_impl_list_lock);
		list_for_each_entry(conn_impl, &peer->conn_impl_list, list) {
			tcp = conn_impl->connect_handle;
			size += sprintf(buf + size, "\t %d  \t%d  \t%d  \t%p  \t%ld\n",
					tcp->fd, conn_impl->status,
					tcp->sock->state, tcp->sock, file_count(tcp->sock->file));
		}
		mutex_unlock(&peer->conn_impl_list_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);
	return size;
}

static ssize_t sbi_status_store(struct kobject *kobj,
				struct sbi_attribute *attr, const char *buf,
				size_t len)
{
	return len;
}

static struct sbi_attribute sbi_status_attr =
	__ATTR(status, 0664, sbi_status_show, sbi_status_store);

static ssize_t sbi_stat_show(struct kobject *kobj, struct sbi_attribute *attr,
			     char *buf)
{
	ssize_t size = 0;
	struct hmdfs_sb_info *sbi = NULL;
	struct hmdfs_peer *peer = NULL;
	struct connection *conn_impl = NULL;
	struct tcp_handle *tcp = NULL;

	sbi = to_sbi(kobj);
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(peer, &sbi->connections.node_list, list) {
		// connection information
		mutex_lock(&peer->conn_impl_list_lock);
		list_for_each_entry(conn_impl, &peer->conn_impl_list, list) {
			tcp = conn_impl->connect_handle;
			size += sprintf(buf + size, "socket_fd: %d\n", tcp->fd);
			size += sprintf(buf + size,
					"\tsend_msg %d \tsend_bytes %llu\n",
					conn_impl->stat.send_message_count,
					conn_impl->stat.send_bytes);
			size += sprintf(buf + size,
					"\trecv_msg %d \trecv_bytes %llu\n",
					conn_impl->stat.recv_message_count,
					conn_impl->stat.recv_bytes);
		}
		mutex_unlock(&peer->conn_impl_list_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);
	return size;
}

static ssize_t sbi_stat_store(struct kobject *kobj, struct sbi_attribute *attr,
			      const char *buf, size_t len)
{
	struct hmdfs_sb_info *sbi = NULL;
	struct hmdfs_peer *peer = NULL;
	struct connection *conn_impl = NULL;

	sbi = to_sbi(kobj);
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(peer, &sbi->connections.node_list, list) {
		// connection information
		mutex_lock(&peer->conn_impl_list_lock);
		list_for_each_entry(conn_impl, &peer->conn_impl_list, list) {
			conn_impl->stat.send_message_count = 0;
			conn_impl->stat.send_bytes = 0;
			conn_impl->stat.recv_message_count = 0,
			conn_impl->stat.recv_bytes = 0;
		}
		mutex_unlock(&peer->conn_impl_list_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);
	return len;
}

static struct sbi_attribute sbi_statistic_attr =
	__ATTR(statistic, 0664, sbi_stat_show, sbi_stat_store);

static ssize_t sbi_dcache_precision_show(struct kobject *kobj,
					 struct sbi_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", to_sbi(kobj)->dcache_precision);
}

#define PRECISION_MAX 3600000

static ssize_t sbi_dcache_precision_store(struct kobject *kobj,
					  struct sbi_attribute *attr,
					  const char *buf, size_t len)
{
	int ret;
	unsigned int precision;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	ret = kstrtouint(skip_spaces(buf), 0, &precision);
	if (!ret) {
		if (precision <= PRECISION_MAX)
			sbi->dcache_precision = precision;
		else
			ret = -EINVAL;
	}

	return ret ? ret : len;
}

static struct sbi_attribute sbi_dcache_precision_attr =
	__ATTR(dcache_precision, 0664, sbi_dcache_precision_show,
	       sbi_dcache_precision_store);

static ssize_t sbi_dcache_threshold_show(struct kobject *kobj,
					 struct sbi_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%lu\n",
			to_sbi(kobj)->dcache_threshold);
}

static ssize_t sbi_dcache_threshold_store(struct kobject *kobj,
					  struct sbi_attribute *attr,
					  const char *buf, size_t len)
{
	int ret;
	unsigned long threshold;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	ret = kstrtoul(skip_spaces(buf), 0, &threshold);
	if (!ret)
		sbi->dcache_threshold = threshold;

	return ret ? ret : len;
}

static struct sbi_attribute sbi_dcache_threshold_attr =
	__ATTR(dcache_threshold, 0664, sbi_dcache_threshold_show,
	       sbi_dcache_threshold_store);

static ssize_t server_statistic_show(struct kobject *kobj,
				     struct sbi_attribute *attr, char *buf)
{
	int i, ret;
	const size_t size = PAGE_SIZE - 1;
	ssize_t pos = 0;
	struct server_statistic *stat = to_sbi(kobj)->s_server_statis;

	for (i = 0; i < F_SIZE; i++) {
		unsigned long long avg = 0;

		if (stat[i].cnt)
			avg = stat[i].total / stat[i].cnt;

		ret = snprintf(buf + pos, size - pos,
			       "%llu %u %u %llu %llu\n",
			       stat[i].cnt, jiffies_to_msecs(avg),
			       jiffies_to_msecs(stat[i].max),
			       stat[i].snd_cnt, stat[i].snd_fail_cnt);
		if (ret > size - pos)
			break;
		pos += ret;
	}

	/* If break, we should add a new line */
	if (i < F_SIZE) {
		ret = snprintf(buf + pos, size + 1 - pos, "\n");
		pos += ret;
	}
	return pos;
}

static struct sbi_attribute sbi_local_op_attr = __ATTR_RO(server_statistic);

static ssize_t client_statistic_show(struct kobject *kobj,
				     struct sbi_attribute *attr, char *buf)
{
	int i, ret;
	const size_t size = PAGE_SIZE - 1;
	ssize_t pos = 0;
	struct client_statistic *stat = to_sbi(kobj)->s_client_statis;

	for (i = 0; i < F_SIZE; i++) {
		unsigned long long avg = 0;

		if (stat[i].resp_cnt)
			avg = stat[i].total / stat[i].resp_cnt;

		ret = snprintf(buf + pos, size - pos,
			       "%llu %llu %llu %llu %llu %u %u\n",
			       stat[i].snd_cnt,
			       stat[i].snd_fail_cnt,
			       stat[i].resp_cnt,
			       stat[i].timeout_cnt,
			       stat[i].delay_resp_cnt,
			       jiffies_to_msecs(avg),
			       jiffies_to_msecs(stat[i].max));
		if (ret > size - pos)
			break;
		pos += ret;
	}

	/* If break, we should add a new line */
	if (i < F_SIZE) {
		ret = snprintf(buf + pos, size + 1 - pos, "\n");
		pos += ret;
	}

	return pos;
}

static struct sbi_attribute sbi_delay_resp_attr = __ATTR_RO(client_statistic);

static inline unsigned long pages_to_kbytes(unsigned long page)
{
	return page << (PAGE_SHIFT - 10);
}

static ssize_t dirty_writeback_stats_show(struct kobject *kobj,
					  struct sbi_attribute *attr,
					  char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	struct hmdfs_writeback *hwb = sbi->h_wb;
	unsigned long avg;
	unsigned long max;
	unsigned long min;

	spin_lock(&hwb->write_bandwidth_lock);
	avg = hwb->avg_write_bandwidth;
	max = hwb->max_write_bandwidth;
	min = hwb->min_write_bandwidth;
	spin_unlock(&hwb->write_bandwidth_lock);

	if (min == ULONG_MAX)
		min = 0;

	return snprintf(buf, PAGE_SIZE,
			"%10lu\n"
			"%10lu\n"
			"%10lu\n",
			pages_to_kbytes(avg),
			pages_to_kbytes(max),
			pages_to_kbytes(min));
}

static struct sbi_attribute sbi_dirty_writeback_stats_attr =
	__ATTR_RO(dirty_writeback_stats);

static ssize_t sbi_wb_timeout_ms_show(struct kobject *kobj,
				      struct sbi_attribute *attr,
				      char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n", sbi->wb_timeout_ms);
}

static ssize_t sbi_wb_timeout_ms_store(struct kobject *kobj,
				       struct sbi_attribute *attr,
				       const char *buf, size_t len)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned int val;
	int err;

	err = kstrtouint(buf, 10, &val);
	if (err)
		return err;

	if (!val || val > HMDFS_MAX_WB_TIMEOUT_MS)
		return -EINVAL;

	sbi->wb_timeout_ms = val;

	return len;
}

static struct sbi_attribute sbi_wb_timeout_ms_attr =
	__ATTR(wb_timeout_ms, 0664, sbi_wb_timeout_ms_show,
	       sbi_wb_timeout_ms_store);

static ssize_t sbi_dirty_writeback_centisecs_show(struct kobject *kobj,
						  struct sbi_attribute *attr,
						  char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n",
			sbi->h_wb->dirty_writeback_interval);
}

static ssize_t sbi_dirty_writeback_centisecs_store(struct kobject *kobj,
						   struct sbi_attribute *attr,
						   const char *buf, size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	int err;

	err = kstrtouint(buf, 10, &sbi->h_wb->dirty_writeback_interval);
	if (err)
		return err;
	return len;
}

static struct sbi_attribute sbi_dirty_writeback_centisecs_attr =
	__ATTR(dirty_writeback_centisecs, 0664,
	       sbi_dirty_writeback_centisecs_show,
	       sbi_dirty_writeback_centisecs_store);

static ssize_t sbi_dirty_file_background_bytes_show(struct kobject *kobj,
						    struct sbi_attribute *attr,
						    char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%lu\n",
			sbi->h_wb->dirty_file_bg_bytes);
}

static ssize_t sbi_dirty_file_background_bytes_store(struct kobject *kobj,
						     struct sbi_attribute *attr,
						     const char *buf,
						     size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned long file_background_bytes = 0;
	int err;

	err = kstrtoul(buf, 10, &file_background_bytes);
	if (err)
		return err;
	if (file_background_bytes == 0)
		return -EINVAL;

	sbi->h_wb->dirty_fs_bytes =
		max(sbi->h_wb->dirty_fs_bytes, file_background_bytes);
	sbi->h_wb->dirty_fs_bg_bytes =
		max(sbi->h_wb->dirty_fs_bg_bytes, file_background_bytes);
	sbi->h_wb->dirty_file_bytes =
		max(sbi->h_wb->dirty_file_bytes, file_background_bytes);

	sbi->h_wb->dirty_file_bg_bytes = file_background_bytes;
	hmdfs_calculate_dirty_thresh(sbi->h_wb);
	hmdfs_update_ratelimit(sbi->h_wb);
	return len;
}

static ssize_t sbi_dirty_fs_background_bytes_show(struct kobject *kobj,
						  struct sbi_attribute *attr,
						  char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%lu\n", sbi->h_wb->dirty_fs_bg_bytes);
}

static ssize_t sbi_dirty_fs_background_bytes_store(struct kobject *kobj,
						   struct sbi_attribute *attr,
						   const char *buf, size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned long fs_background_bytes = 0;
	int err;

	err = kstrtoul(buf, 10, &fs_background_bytes);
	if (err)
		return err;
	if (fs_background_bytes == 0)
		return -EINVAL;

	sbi->h_wb->dirty_file_bg_bytes =
		min(sbi->h_wb->dirty_file_bg_bytes, fs_background_bytes);
	sbi->h_wb->dirty_fs_bytes =
		max(sbi->h_wb->dirty_fs_bytes, fs_background_bytes);

	sbi->h_wb->dirty_fs_bg_bytes = fs_background_bytes;
	hmdfs_calculate_dirty_thresh(sbi->h_wb);
	hmdfs_update_ratelimit(sbi->h_wb);
	return len;
}

static struct sbi_attribute sbi_dirty_file_background_bytes_attr =
	__ATTR(dirty_file_background_bytes, 0644,
	       sbi_dirty_file_background_bytes_show,
	       sbi_dirty_file_background_bytes_store);
static struct sbi_attribute sbi_dirty_fs_background_bytes_attr =
	__ATTR(dirty_fs_background_bytes, 0644,
	       sbi_dirty_fs_background_bytes_show,
	       sbi_dirty_fs_background_bytes_store);

static ssize_t sbi_dirty_file_bytes_show(struct kobject *kobj,
					 struct sbi_attribute *attr, char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%lu\n", sbi->h_wb->dirty_file_bytes);
}

static ssize_t sbi_dirty_file_bytes_store(struct kobject *kobj,
					  struct sbi_attribute *attr,
					  const char *buf, size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned long file_bytes = 0;
	int err;

	err = kstrtoul(buf, 10, &file_bytes);
	if (err)
		return err;
	if (file_bytes == 0)
		return -EINVAL;

	sbi->h_wb->dirty_file_bg_bytes =
		min(sbi->h_wb->dirty_file_bg_bytes, file_bytes);
	sbi->h_wb->dirty_fs_bytes = max(sbi->h_wb->dirty_fs_bytes, file_bytes);

	sbi->h_wb->dirty_file_bytes = file_bytes;
	hmdfs_calculate_dirty_thresh(sbi->h_wb);
	hmdfs_update_ratelimit(sbi->h_wb);
	return len;
}

static ssize_t sbi_dirty_fs_bytes_show(struct kobject *kobj,
				       struct sbi_attribute *attr, char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%lu\n", sbi->h_wb->dirty_fs_bytes);
}

static ssize_t sbi_dirty_fs_bytes_store(struct kobject *kobj,
					struct sbi_attribute *attr,
					const char *buf, size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned long fs_bytes = 0;
	int err;

	err = kstrtoul(buf, 10, &fs_bytes);
	if (err)
		return err;
	if (fs_bytes == 0)
		return -EINVAL;

	sbi->h_wb->dirty_file_bg_bytes =
		min(sbi->h_wb->dirty_file_bg_bytes, fs_bytes);
	sbi->h_wb->dirty_file_bytes =
		min(sbi->h_wb->dirty_file_bytes, fs_bytes);
	sbi->h_wb->dirty_fs_bg_bytes =
		min(sbi->h_wb->dirty_fs_bg_bytes, fs_bytes);

	sbi->h_wb->dirty_fs_bytes = fs_bytes;
	hmdfs_calculate_dirty_thresh(sbi->h_wb);
	hmdfs_update_ratelimit(sbi->h_wb);
	return len;
}

static struct sbi_attribute sbi_dirty_file_bytes_attr =
	__ATTR(dirty_file_bytes, 0644, sbi_dirty_file_bytes_show,
	       sbi_dirty_file_bytes_store);
static struct sbi_attribute sbi_dirty_fs_bytes_attr =
	__ATTR(dirty_fs_bytes, 0644, sbi_dirty_fs_bytes_show,
	       sbi_dirty_fs_bytes_store);

static ssize_t sbi_dirty_writeback_timelimit_show(struct kobject *kobj,
						  struct sbi_attribute *attr,
						  char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n",
			sbi->h_wb->writeback_timelimit / HZ);
}

static ssize_t sbi_dirty_writeback_timelimit_store(struct kobject *kobj,
						   struct sbi_attribute *attr,
						   const char *buf,
						   size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned int time_limit = 0;
	int err;

	err = kstrtouint(buf, 10, &time_limit);
	if (err)
		return err;
	if (time_limit == 0 || time_limit > (HMDFS_MAX_WB_TIMELIMIT / HZ))
		return -EINVAL;

	sbi->h_wb->writeback_timelimit = time_limit * HZ;
	return len;
}

static struct sbi_attribute sbi_dirty_writeback_timelimit_attr =
__ATTR(dirty_writeback_timelimit, 0644, sbi_dirty_writeback_timelimit_show,
	sbi_dirty_writeback_timelimit_store);

static ssize_t sbi_dirty_thresh_lowerlimit_show(struct kobject *kobj,
						struct sbi_attribute *attr,
						char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%lu\n",
			sbi->h_wb->bw_thresh_lowerlimit << PAGE_SHIFT);
}

static ssize_t sbi_dirty_thresh_lowerlimit_store(struct kobject *kobj,
						 struct sbi_attribute *attr,
						 const char *buf,
						 size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned long bw_thresh_lowerbytes = 0;
	unsigned long bw_thresh_lowerlimit;
	int err;

	err = kstrtoul(buf, 10, &bw_thresh_lowerbytes);
	if (err)
		return err;

	bw_thresh_lowerlimit = DIV_ROUND_UP(bw_thresh_lowerbytes, PAGE_SIZE);
	if (bw_thresh_lowerlimit < HMDFS_BW_THRESH_MIN_LIMIT ||
	    bw_thresh_lowerlimit > HMDFS_BW_THRESH_MAX_LIMIT)
		return -EINVAL;

	sbi->h_wb->bw_thresh_lowerlimit = bw_thresh_lowerlimit;
	return len;
}

static struct sbi_attribute sbi_dirty_thresh_lowerlimit_attr =
__ATTR(dirty_thresh_lowerlimit, 0644, sbi_dirty_thresh_lowerlimit_show,
	sbi_dirty_thresh_lowerlimit_store);

static ssize_t sbi_dirty_writeback_autothresh_show(struct kobject *kobj,
						   struct sbi_attribute *attr,
						   char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			sbi->h_wb->dirty_auto_threshold);
}

static ssize_t sbi_dirty_writeback_autothresh_store(struct kobject *kobj,
						    struct sbi_attribute *attr,
						    const char *buf,
						    size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	bool dirty_auto_threshold = false;
	int err;

	err = kstrtobool(buf, &dirty_auto_threshold);
	if (err)
		return err;

	sbi->h_wb->dirty_auto_threshold = dirty_auto_threshold;
	return len;
}

static struct sbi_attribute sbi_dirty_writeback_autothresh_attr =
__ATTR(dirty_writeback_autothresh, 0644, sbi_dirty_writeback_autothresh_show,
	sbi_dirty_writeback_autothresh_store);

static ssize_t sbi_dirty_writeback_control_show(struct kobject *kobj,
						struct sbi_attribute *attr,
						char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			sbi->h_wb->dirty_writeback_control);
}

static ssize_t sbi_dirty_writeback_control_store(struct kobject *kobj,
						 struct sbi_attribute *attr,
						 const char *buf, size_t len)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned int dirty_writeback_control = 0;
	int err;

	err = kstrtouint(buf, 10, &dirty_writeback_control);
	if (err)
		return err;

	sbi->h_wb->dirty_writeback_control = (bool)dirty_writeback_control;
	return len;
}

static struct sbi_attribute sbi_dirty_writeback_control_attr =
	__ATTR(dirty_writeback_control, 0644, sbi_dirty_writeback_control_show,
	       sbi_dirty_writeback_control_store);

static ssize_t sbi_srv_dirty_thresh_show(struct kobject *kobj,
						struct sbi_attribute *attr,
						char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			sbi->h_swb->dirty_thresh_pg >> HMDFS_MB_TO_PAGE_SHIFT);
}

static ssize_t sbi_srv_dirty_thresh_store(struct kobject *kobj,
						struct sbi_attribute *attr,
						const char *buf,
						size_t len)
{
	struct hmdfs_server_writeback *hswb = to_sbi(kobj)->h_swb;
	int dirty_thresh_mb;
	unsigned long long pages;
	int err;

	err = kstrtoint(buf, 10, &dirty_thresh_mb);
	if (err)
		return err;

	if (dirty_thresh_mb <= 0)
		return -EINVAL;

	pages = dirty_thresh_mb;
	pages <<= HMDFS_MB_TO_PAGE_SHIFT;
	if (pages > INT_MAX) {
		hmdfs_err("Illegal dirty_thresh_mb %d, its page count beyonds max int",
			  dirty_thresh_mb);
		return -EINVAL;
	}

	hswb->dirty_thresh_pg = (unsigned int)pages;
	return len;
}

static struct sbi_attribute sbi_srv_dirty_thresh_attr =
__ATTR(srv_dirty_thresh, 0644, sbi_srv_dirty_thresh_show,
						sbi_srv_dirty_thresh_store);


static ssize_t sbi_srv_dirty_wb_control_show(struct kobject *kobj,
						struct sbi_attribute *attr,
						char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%d\n",
			sbi->h_swb->dirty_writeback_control);
}

static ssize_t sbi_srv_dirty_wb_conctrol_store(struct kobject *kobj,
					       struct sbi_attribute *attr,
					       const char *buf,
					       size_t len)
{
	struct hmdfs_server_writeback *hswb = to_sbi(kobj)->h_swb;
	bool dirty_writeback_control = true;
	int err;

	err = kstrtobool(buf, &dirty_writeback_control);
	if (err)
		return err;

	hswb->dirty_writeback_control = dirty_writeback_control;

	return len;
}

static struct sbi_attribute sbi_srv_dirty_wb_control_attr =
__ATTR(srv_dirty_writeback_control, 0644, sbi_srv_dirty_wb_control_show,
					sbi_srv_dirty_wb_conctrol_store);

static ssize_t sbi_dcache_timeout_show(struct kobject *kobj,
				       struct sbi_attribute *attr, char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n", sbi->dcache_timeout);
}

static ssize_t sbi_dcache_timeout_store(struct kobject *kobj,
					struct sbi_attribute *attr,
					const char *buf, size_t len)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned int timeout;
	int err;

	err = kstrtouint(buf, 0, &timeout);
	if (err)
		return err;

	/* zero is invalid, and it doesn't mean no cache */
	if (timeout == 0 || timeout > MAX_DCACHE_TIMEOUT)
		return -EINVAL;

	sbi->dcache_timeout = timeout;

	return len;
}

static struct sbi_attribute sbi_dcache_timeout_attr =
	__ATTR(dcache_timeout, 0644, sbi_dcache_timeout_show,
	       sbi_dcache_timeout_store);

static ssize_t sbi_write_cache_timeout_sec_show(struct kobject *kobj,
			struct sbi_attribute *attr, char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n",
			to_sbi(kobj)->write_cache_timeout);
}

static ssize_t sbi_write_cache_timeout_sec_store(struct kobject *kobj,
		struct sbi_attribute *attr, const char *buf, size_t len)
{
	int ret;
	unsigned int timeout;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	ret = kstrtouint(buf, 0, &timeout);
	if (ret)
		return ret;

	/* set write_cache_timeout to 0 means this functionality is disabled */
	sbi->write_cache_timeout = timeout;

	return len;
}

static struct sbi_attribute sbi_write_cache_timeout_sec_attr =
	__ATTR(write_cache_timeout_sec, 0664, sbi_write_cache_timeout_sec_show,
	       sbi_write_cache_timeout_sec_store);

static ssize_t sbi_node_evt_cb_delay_show(struct kobject *kobj,
					  struct sbi_attribute *attr,
					  char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n", sbi->async_cb_delay);
}

static ssize_t sbi_node_evt_cb_delay_store(struct kobject *kobj,
					   struct sbi_attribute *attr,
					   const char *buf,
					   size_t len)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);
	unsigned int delay = 0;
	int err;

	err = kstrtouint(buf, 10, &delay);
	if (err)
		return err;

	sbi->async_cb_delay = delay;

	return len;
}

static struct sbi_attribute sbi_node_evt_cb_delay_attr =
__ATTR(node_event_delay, 0644, sbi_node_evt_cb_delay_show,
	sbi_node_evt_cb_delay_store);

static int calc_idr_number(struct idr *idr)
{
	void *entry = NULL;
	int id;
	int number = 0;

	idr_for_each_entry(idr, entry, id) {
		number++;
		if (number % HMDFS_IDR_RESCHED_COUNT == 0)
			cond_resched();
	}

	return number;
}

static ssize_t sbi_show_idr_stats(struct kobject *kobj,
				  struct sbi_attribute *attr,
				  char *buf, bool showmsg)
{
	ssize_t size = 0;
	int count;
	struct hmdfs_sb_info *sbi = NULL;
	struct hmdfs_peer *peer = NULL;
	struct idr *idr = NULL;

	sbi = to_sbi(kobj);

	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(peer, &sbi->connections.node_list, list) {
		idr = showmsg ? &peer->msg_idr : &peer->file_id_idr;
		count = calc_idr_number(idr);
		size += snprintf(buf + size, PAGE_SIZE - size,
				"device-id\tcount\tnext-id\n\t%llu\t\t%d\t%u\n",
				 peer->device_id, count, idr_get_cursor(idr));
		if (size >= PAGE_SIZE) {
			size = PAGE_SIZE;
			break;
		}
	}
	mutex_unlock(&sbi->connections.node_lock);

	return size;
}

static ssize_t pending_message_show(struct kobject *kobj,
				    struct sbi_attribute *attr,
				    char *buf)
{
	return sbi_show_idr_stats(kobj, attr, buf, true);
}

static struct sbi_attribute sbi_pending_message_attr =
	__ATTR_RO(pending_message);

static ssize_t peer_opened_fd_show(struct kobject *kobj,
				   struct sbi_attribute *attr, char *buf)
{
	return sbi_show_idr_stats(kobj, attr, buf, false);
}

static struct sbi_attribute sbi_peer_opened_fd_attr = __ATTR_RO(peer_opened_fd);

static ssize_t sbi_srv_req_max_active_attr_show(struct kobject *kobj,
						struct sbi_attribute *attr,
						char *buf)
{
	const struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return snprintf(buf, PAGE_SIZE, "%u\n", sbi->async_req_max_active);
}

static ssize_t sbi_srv_req_max_active_attr_store(struct kobject *kobj,
		struct sbi_attribute *attr, const char *buf, size_t len)
{
	int ret;
	unsigned int max_active;
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	ret = kstrtouint(buf, 0, &max_active);
	if (ret)
		return ret;

	sbi->async_req_max_active = max_active;

	return len;
}

static struct sbi_attribute sbi_srv_req_max_active_attr =
__ATTR(srv_req_handle_max_active, 0644, sbi_srv_req_max_active_attr_show,
	sbi_srv_req_max_active_attr_store);


static ssize_t cache_file_show(struct hmdfs_sb_info *sbi,
			       struct list_head *head, char *buf)
{
	struct cache_file_node *cfn = NULL;
	ssize_t pos = 0;

	mutex_lock(&sbi->cache_list_lock);
	list_for_each_entry(cfn, head, list) {
		pos += snprintf(buf + pos, PAGE_SIZE - pos,
			"dev_id: %s relative_path: %s \n",
			cfn->cid, cfn->relative_path);
		if (pos >= PAGE_SIZE) {
			pos = PAGE_SIZE;
			break;
		}
	}
	mutex_unlock(&sbi->cache_list_lock);

	return pos;
}

static ssize_t client_cache_file_show(struct kobject *kobj,
			struct sbi_attribute *attr, char *buf)
{
	return cache_file_show(to_sbi(kobj), &to_sbi(kobj)->client_cache, buf);
}
static ssize_t server_cache_file_show(struct kobject *kobj,
			struct sbi_attribute *attr, char *buf)
{
	return cache_file_show(to_sbi(kobj), &to_sbi(kobj)->server_cache, buf);
}

static struct sbi_attribute sbi_server_cache_file_attr =
	__ATTR_RO(server_cache_file);
static struct sbi_attribute sbi_client_cache_file_attr =
	__ATTR_RO(client_cache_file);

static ssize_t sb_seq_show(struct kobject *kobj, struct sbi_attribute *attr,
			    char *buf)
{
	return snprintf(buf, PAGE_SIZE, "%u\n", to_sbi(kobj)->seq);
}

static struct sbi_attribute sbi_seq_attr = __ATTR_RO(sb_seq);

static ssize_t peers_sum_attr_show(struct kobject *kobj,
				   struct sbi_attribute *attr, char *buf)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);
	struct hmdfs_peer *node = NULL;
	unsigned int stash_ok = 0, stash_fail = 0, restore_ok = 0,
	restore_fail = 0, rebuild_ok = 0, rebuild_fail = 0, rebuild_invalid = 0,
	rebuild_time = 0;
	unsigned long long stash_ok_pages = 0, stash_fail_pages = 0,
	restore_ok_pages = 0, restore_fail_pages = 0;

	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(node, &sbi->connections.node_list, list) {
		peer_get(node);
		mutex_unlock(&sbi->connections.node_lock);
		stash_ok += node->stats.stash.total_ok;
		stash_fail += node->stats.stash.total_fail;
		stash_ok_pages += node->stats.stash.ok_pages;
		stash_fail_pages += node->stats.stash.fail_pages;
		restore_ok += node->stats.restore.total_ok;
		restore_fail += node->stats.restore.total_fail;
		restore_ok_pages += node->stats.restore.ok_pages;
		restore_fail_pages += node->stats.restore.fail_pages;
		rebuild_ok += node->stats.rebuild.total_ok;
		rebuild_fail += node->stats.rebuild.total_fail;
		rebuild_invalid += node->stats.rebuild.total_invalid;
		rebuild_time += node->stats.rebuild.time;
		peer_put(node);
		mutex_lock(&sbi->connections.node_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);

	return snprintf(buf, PAGE_SIZE,
			"%u %u %llu %llu\n"
			"%u %u %llu %llu\n"
			"%u %u %u %u\n",
			stash_ok, stash_fail, stash_ok_pages, stash_fail_pages,
			restore_ok, restore_fail, restore_ok_pages,
			restore_fail_pages, rebuild_ok, rebuild_fail,
			rebuild_invalid, rebuild_time);
}

static struct sbi_attribute sbi_peers_attr = __ATTR_RO(peers_sum_attr);

const char * const flag_name[] = {
	"READPAGES",
	"READPAGES_OPEN",
	"ATOMIC_OPEN",
};

static ssize_t fill_features(char *buf, unsigned long long flag)
{
	int i;
	ssize_t pos = 0;
	bool sep = false;
	int flag_name_count = sizeof(flag_name) / sizeof(flag_name[0]);

	for (i = 0; i < sizeof(flag) * BITS_PER_BYTE; ++i) {
		if (!(flag & BIT(i)))
			continue;

		if (sep)
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "|");
		sep = true;

		if (pos >= PAGE_SIZE) {
			pos = PAGE_SIZE;
			break;
		}

		if (i < flag_name_count && flag_name[i])
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%s",
					flag_name[i]);
		else
			pos += snprintf(buf + pos, PAGE_SIZE - pos, "%d", i);

		if (pos >= PAGE_SIZE) {
			pos = PAGE_SIZE;
			break;
		}
	}
	pos += snprintf(buf + pos, PAGE_SIZE - pos, "\n");
	if (pos >= PAGE_SIZE)
		pos = PAGE_SIZE;

	return pos;
}

static ssize_t sbi_features_show(struct kobject *kobj,
				 struct sbi_attribute *attr, char *buf)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	return fill_features(buf, sbi->s_features);
}

static struct sbi_attribute sbi_features_attr = __ATTR(features, 0444,
	sbi_features_show, NULL);

static struct attribute *sbi_attrs[] = {
	&sbi_cmd_attr.attr,
	&sbi_status_attr.attr,
	&sbi_statistic_attr.attr,
	&sbi_dcache_precision_attr.attr,
	&sbi_dcache_threshold_attr.attr,
	&sbi_dcache_timeout_attr.attr,
	&sbi_write_cache_timeout_sec_attr.attr,
	&sbi_local_op_attr.attr,
	&sbi_delay_resp_attr.attr,
	&sbi_wb_timeout_ms_attr.attr,
	&sbi_dirty_writeback_centisecs_attr.attr,
	&sbi_dirty_file_background_bytes_attr.attr,
	&sbi_dirty_fs_background_bytes_attr.attr,
	&sbi_dirty_file_bytes_attr.attr,
	&sbi_dirty_fs_bytes_attr.attr,
	&sbi_dirty_writeback_autothresh_attr.attr,
	&sbi_dirty_writeback_timelimit_attr.attr,
	&sbi_dirty_thresh_lowerlimit_attr.attr,
	&sbi_dirty_writeback_control_attr.attr,
	&sbi_dirty_writeback_stats_attr.attr,
	&sbi_srv_dirty_thresh_attr.attr,
	&sbi_srv_dirty_wb_control_attr.attr,
	&sbi_node_evt_cb_delay_attr.attr,
	&sbi_srv_req_max_active_attr.attr,
	&sbi_pending_message_attr.attr,
	&sbi_peer_opened_fd_attr.attr,
	&sbi_server_cache_file_attr.attr,
	&sbi_client_cache_file_attr.attr,
	&sbi_seq_attr.attr,
	&sbi_peers_attr.attr,
	&sbi_features_attr.attr,
	NULL,
};

static ssize_t sbi_attr_show(struct kobject *kobj, struct attribute *attr,
			     char *buf)
{
	struct sbi_attribute *sbi_attr = to_sbi_attr(attr);

	if (!sbi_attr->show)
		return -EIO;
	return sbi_attr->show(kobj, sbi_attr, buf);
}

static ssize_t sbi_attr_store(struct kobject *kobj, struct attribute *attr,
			      const char *buf, size_t len)
{
	struct sbi_attribute *sbi_attr = to_sbi_attr(attr);

	if (!sbi_attr->store)
		return -EIO;
	return sbi_attr->store(kobj, sbi_attr, buf, len);
}

static const struct sysfs_ops sbi_sysfs_ops = {
	.show = sbi_attr_show,
	.store = sbi_attr_store,
};

static void sbi_release(struct kobject *kobj)
{
	struct hmdfs_sb_info *sbi = to_sbi(kobj);

	complete(&sbi->s_kobj_unregister);
}

static struct kobj_type sbi_ktype = {
	.sysfs_ops = &sbi_sysfs_ops,
	.default_attrs = sbi_attrs,
	.release = sbi_release,
};

static inline struct sbi_cmd_attribute *to_sbi_cmd_attr(struct attribute *x)
{
	return container_of(x, struct sbi_cmd_attribute, attr);
}

static inline struct hmdfs_sb_info *cmd_kobj_to_sbi(struct kobject *x)
{
	return container_of(x, struct hmdfs_sb_info, s_cmd_timeout_kobj);
}

static ssize_t cmd_timeout_show(struct kobject *kobj, struct attribute *attr,
				char *buf)
{
	int cmd = to_sbi_cmd_attr(attr)->command;
	struct hmdfs_sb_info *sbi = cmd_kobj_to_sbi(kobj);

	if (cmd < 0 && cmd >= F_SIZE)
		return 0;

	return snprintf(buf, PAGE_SIZE, "%u\n", get_cmd_timeout(sbi, cmd));
}

static ssize_t cmd_timeout_store(struct kobject *kobj, struct attribute *attr,
				 const char *buf, size_t len)
{
	unsigned int value;
	int cmd = to_sbi_cmd_attr(attr)->command;
	int ret = kstrtouint(skip_spaces(buf), 0, &value);
	struct hmdfs_sb_info *sbi = cmd_kobj_to_sbi(kobj);

	if (cmd < 0 && cmd >= F_SIZE)
		return -EINVAL;

	if (!ret)
		set_cmd_timeout(sbi, cmd, value);

	return ret ? ret : len;
}

#define HMDFS_CMD_ATTR(_name, _cmd)                                            \
	static struct sbi_cmd_attribute hmdfs_attr_##_name = {                 \
		.attr = { .name = __stringify(_name), .mode = 0664 },          \
		.command = (_cmd),                                             \
	}

HMDFS_CMD_ATTR(open, F_OPEN);
HMDFS_CMD_ATTR(release, F_RELEASE);
HMDFS_CMD_ATTR(readpage, F_READPAGE);
HMDFS_CMD_ATTR(writepage, F_WRITEPAGE);
HMDFS_CMD_ATTR(iterate, F_ITERATE);
HMDFS_CMD_ATTR(rmdir, F_RMDIR);
HMDFS_CMD_ATTR(unlink, F_UNLINK);
HMDFS_CMD_ATTR(rename, F_RENAME);
HMDFS_CMD_ATTR(setattr, F_SETATTR);
HMDFS_CMD_ATTR(connect_echo, F_CONNECT_ECHO);
HMDFS_CMD_ATTR(statfs, F_STATFS);
HMDFS_CMD_ATTR(drop_push, F_DROP_PUSH);
HMDFS_CMD_ATTR(getattr, F_GETATTR);
HMDFS_CMD_ATTR(fsync, F_FSYNC);
HMDFS_CMD_ATTR(syncfs, F_SYNCFS);
HMDFS_CMD_ATTR(getxattr, F_GETXATTR);
HMDFS_CMD_ATTR(setxattr, F_SETXATTR);
HMDFS_CMD_ATTR(listxattr, F_LISTXATTR);

#define ATTR_LIST(_name) (&hmdfs_attr_##_name.attr)

static struct attribute *sbi_timeout_attrs[] = {
	ATTR_LIST(open),     ATTR_LIST(release),
	ATTR_LIST(readpage), ATTR_LIST(writepage),
	ATTR_LIST(iterate),  ATTR_LIST(rmdir),
	ATTR_LIST(unlink),   ATTR_LIST(rename),
	ATTR_LIST(setattr),  ATTR_LIST(connect_echo),
	ATTR_LIST(statfs),   ATTR_LIST(drop_push),
	ATTR_LIST(getattr),  ATTR_LIST(fsync),
	ATTR_LIST(syncfs),   ATTR_LIST(getxattr),
	ATTR_LIST(setxattr), ATTR_LIST(listxattr),
	NULL
};

static const struct sysfs_ops sbi_cmd_sysfs_ops = {
	.show = cmd_timeout_show,
	.store = cmd_timeout_store,
};

static void sbi_timeout_release(struct kobject *kobj)
{
	struct hmdfs_sb_info *sbi = container_of(kobj, struct hmdfs_sb_info,
						 s_cmd_timeout_kobj);

	complete(&sbi->s_timeout_kobj_unregister);
}

static struct kobj_type sbi_timeout_ktype = {
	.sysfs_ops = &sbi_cmd_sysfs_ops,
	.default_attrs = sbi_timeout_attrs,
	.release = sbi_timeout_release,
};

void hmdfs_release_sysfs(struct hmdfs_sb_info *sbi)
{
	kobject_put(&sbi->s_cmd_timeout_kobj);
	wait_for_completion(&sbi->s_timeout_kobj_unregister);
	kobject_put(&sbi->kobj);
	wait_for_completion(&sbi->s_kobj_unregister);
}

int hmdfs_register_sysfs(const char *name, struct hmdfs_sb_info *sbi)
{
	int ret;
	struct kobject *kobj = NULL;

	mutex_lock(&hmdfs_sysfs_mutex);
	kobj = kset_find_obj(hmdfs_kset, name);
	if (kobj) {
		hmdfs_err("mount failed, already exist");
		kobject_put(kobj);
		mutex_unlock(&hmdfs_sysfs_mutex);
		return -EEXIST;
	}

	sbi->kobj.kset = hmdfs_kset;
	init_completion(&sbi->s_kobj_unregister);
	ret = kobject_init_and_add(&sbi->kobj, &sbi_ktype,
				   &hmdfs_kset->kobj, "%s", name);
	mutex_unlock(&hmdfs_sysfs_mutex);

	if (ret) {
		kobject_put(&sbi->kobj);
		wait_for_completion(&sbi->s_kobj_unregister);
		return ret;
	}

	init_completion(&sbi->s_timeout_kobj_unregister);
	ret = kobject_init_and_add(&sbi->s_cmd_timeout_kobj, &sbi_timeout_ktype,
				   &sbi->kobj, "cmd_timeout");
	if (ret) {
		hmdfs_release_sysfs(sbi);
		return ret;
	}

	kobject_uevent(&sbi->kobj, KOBJ_ADD);
	return 0;
}

void hmdfs_unregister_sysfs(struct hmdfs_sb_info *sbi)
{
	kobject_del(&sbi->s_cmd_timeout_kobj);
	kobject_del(&sbi->kobj);
}

static inline int to_sysfs_fmt_evt(unsigned int evt)
{
	return evt == RAW_NODE_EVT_NR ? -1 : evt;
}

static ssize_t features_show(struct kobject *kobj, struct peer_attribute *attr,
			     char *buf)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	return fill_features(buf, peer->features);
}

static ssize_t event_show(struct kobject *kobj, struct peer_attribute *attr,
			  char *buf)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	return snprintf(buf, PAGE_SIZE,
			"cur_async evt %d seq %u\n"
			"cur_sync evt %d seq %u\n"
			"pending evt %d seq %u\n"
			"merged evt %u\n"
			"dup_drop evt %u %u\n"
			"waiting evt %u %u\n"
			"seq_tbl %u %u %u %u\n"
			"seq_rd_idx %u\n"
			"seq_wr_idx %u\n",
			to_sysfs_fmt_evt(peer->cur_evt[0]),
			peer->cur_evt_seq[0],
			to_sysfs_fmt_evt(peer->cur_evt[1]),
			peer->cur_evt_seq[1],
			to_sysfs_fmt_evt(peer->pending_evt),
			peer->pending_evt_seq,
			peer->merged_evt,
			peer->dup_evt[RAW_NODE_EVT_OFF],
			peer->dup_evt[RAW_NODE_EVT_ON],
			peer->waiting_evt[RAW_NODE_EVT_OFF],
			peer->waiting_evt[RAW_NODE_EVT_ON],
			peer->seq_tbl[0], peer->seq_tbl[1], peer->seq_tbl[2],
			peer->seq_tbl[3],
			peer->seq_rd_idx % RAW_NODE_EVT_MAX_NR,
			peer->seq_wr_idx % RAW_NODE_EVT_MAX_NR);
}

static ssize_t stash_show(struct kobject *kobj, struct peer_attribute *attr,
			  char *buf)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	return snprintf(buf, PAGE_SIZE,
			"cur_ok %u\n"
			"cur_nothing %u\n"
			"cur_fail %u\n"
			"total_ok %u\n"
			"total_nothing %u\n"
			"total_fail %u\n"
			"ok_pages %llu\n"
			"fail_pages %llu\n",
			peer->stats.stash.cur_ok,
			peer->stats.stash.cur_nothing,
			peer->stats.stash.cur_fail,
			peer->stats.stash.total_ok,
			peer->stats.stash.total_nothing,
			peer->stats.stash.total_fail,
			peer->stats.stash.ok_pages,
			peer->stats.stash.fail_pages);
}

static ssize_t restore_show(struct kobject *kobj, struct peer_attribute *attr,
			    char *buf)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	return snprintf(buf, PAGE_SIZE,
			"cur_ok %u\n"
			"cur_fail %u\n"
			"cur_keep %u\n"
			"total_ok %u\n"
			"total_fail %u\n"
			"total_keep %u\n"
			"ok_pages %llu\n"
			"fail_pages %llu\n",
			peer->stats.restore.cur_ok,
			peer->stats.restore.cur_fail,
			peer->stats.restore.cur_keep,
			peer->stats.restore.total_ok,
			peer->stats.restore.total_fail,
			peer->stats.restore.total_keep,
			peer->stats.restore.ok_pages,
			peer->stats.restore.fail_pages);
}

static ssize_t rebuild_show(struct kobject *kobj, struct peer_attribute *attr,
			    char *buf)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	return snprintf(buf, PAGE_SIZE,
			"cur_ok %u\n"
			"cur_fail %u\n"
			"cur_invalid %u\n"
			"total_ok %u\n"
			"total_fail %u\n"
			"total_invalid %u\n"
			"time %u\n",
			peer->stats.rebuild.cur_ok,
			peer->stats.rebuild.cur_fail,
			peer->stats.rebuild.cur_invalid,
			peer->stats.rebuild.total_ok,
			peer->stats.rebuild.total_fail,
			peer->stats.rebuild.total_invalid,
			peer->stats.rebuild.time);
}

static struct peer_attribute peer_features_attr = __ATTR_RO(features);
static struct peer_attribute peer_event_attr = __ATTR_RO(event);
static struct peer_attribute peer_stash_attr = __ATTR_RO(stash);
static struct peer_attribute peer_restore_attr = __ATTR_RO(restore);
static struct peer_attribute peer_rebuild_attr = __ATTR_RO(rebuild);

static struct attribute *peer_attrs[] = {
	&peer_features_attr.attr,
	&peer_event_attr.attr,
	&peer_stash_attr.attr,
	&peer_restore_attr.attr,
	&peer_rebuild_attr.attr,
	NULL,
};

static ssize_t peer_attr_show(struct kobject *kobj, struct attribute *attr,
			      char *buf)
{
	struct peer_attribute *peer_attr = to_peer_attr(attr);

	if (!peer_attr->show)
		return -EIO;
	return peer_attr->show(kobj, peer_attr, buf);
}

static ssize_t peer_attr_store(struct kobject *kobj, struct attribute *attr,
			       const char *buf, size_t len)
{
	struct peer_attribute *peer_attr = to_peer_attr(attr);

	if (!peer_attr->store)
		return -EIO;
	return peer_attr->store(kobj, peer_attr, buf, len);
}

static const struct sysfs_ops peer_sysfs_ops = {
	.show = peer_attr_show,
	.store = peer_attr_store,
};

static void peer_sysfs_release(struct kobject *kobj)
{
	struct hmdfs_peer *peer = to_peer(kobj);

	complete(&peer->kobj_unregister);
}

static struct kobj_type peer_ktype = {
	.sysfs_ops = &peer_sysfs_ops,
	.default_attrs = peer_attrs,
	.release = peer_sysfs_release,
};

int hmdfs_register_peer_sysfs(struct hmdfs_sb_info *sbi,
			      struct hmdfs_peer *peer)
{
	int err = 0;

	init_completion(&peer->kobj_unregister);
	err = kobject_init_and_add(&peer->kobj, &peer_ktype, &sbi->kobj,
				   "peer_%llu", peer->device_id);
	return err;
}

void hmdfs_release_peer_sysfs(struct hmdfs_peer *peer)
{
	kobject_del(&peer->kobj);
	kobject_put(&peer->kobj);
	wait_for_completion(&peer->kobj_unregister);
}

void notify(struct hmdfs_peer *node, struct notify_param *param)
{
	struct hmdfs_sb_info *sbi = node->sbi;
	int in_len;

	if (!param)
		return;
	spin_lock(&sbi->notify_fifo_lock);
	in_len =
		kfifo_in(&sbi->notify_fifo, param, sizeof(struct notify_param));
	spin_unlock(&sbi->notify_fifo_lock);
	if (in_len != sizeof(struct notify_param))
		return;
	sysfs_notify(&sbi->kobj, NULL, "cmd");
}

int hmdfs_sysfs_init(void)
{
	hmdfs_kset = kset_create_and_add("hmdfs", NULL, fs_kobj);
	if (!hmdfs_kset)
		return -ENOMEM;

	return 0;
}

void hmdfs_sysfs_exit(void)
{
	kset_unregister(hmdfs_kset);
	hmdfs_kset = NULL;
}
