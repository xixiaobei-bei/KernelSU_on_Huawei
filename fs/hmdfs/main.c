// SPDX-License-Identifier: GPL-2.0
/*
 * main.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: weilongping@huawei.com
 * Create: 2020-03-26
 *
 */

#include "hmdfs.h"

#include <linux/ctype.h>
#include <linux/module.h>
#include <linux/statfs.h>
#include <linux/xattr.h>
#include <linux/idr.h>
#if KERNEL_VERSION(5, 9, 0) < LINUX_VERSION_CODE
#include <linux/prandom.h>
#else
#include <linux/random.h>
#endif

#include "authority/authentication.h"
#include "hmdfs_server.h"
#include "comm/device_node.h"
#include "comm/message_verify.h"
#include "comm/protocol.h"
#include "comm/socket_adapter.h"
#include "hmdfs_merge_view.h"
#include "server_writeback.h"

#ifdef CONFIG_HMDFS_1_0
#include "DFS_1_0/adapter.h"
#include "DFS_1_0/dentry_syncer.h"
#endif
#include "comm/node_cb.h"
#include "stash.h"

#define CREATE_TRACE_POINTS
#include "hmdfs_trace.h"

#define HMDFS_BOOT_COOKIE_RAND_SHIFT 33

#define HMDFS_SB_SEQ_FROM 1

struct hmdfs_mount_priv {
	const char *dev_name;
	const char *raw_data;
};

struct syncfs_item {
	struct list_head list;
	struct completion done;
	bool need_abort;
};

static DEFINE_IDA(hmdfs_sb_seq);

static inline int hmdfs_alloc_sb_seq(void)
{
	return ida_simple_get(&hmdfs_sb_seq, HMDFS_SB_SEQ_FROM, 0, GFP_KERNEL);
}

static inline void hmdfs_free_sb_seq(unsigned int seq)
{
	if (!seq)
		return;
	ida_simple_remove(&hmdfs_sb_seq, seq);
}

static int hmdfs_xattr_local_get(struct dentry *dentry, const char *name,
				 void *value, size_t size)
{
	struct path lower_path;
	ssize_t res = 0;

	hmdfs_get_lower_path(dentry, &lower_path);
	res = vfs_getxattr(lower_path.dentry, name, value, size);
	hmdfs_put_lower_path(&lower_path);
	return res;
}

static int hmdfs_xattr_remote_get(struct dentry *dentry, const char *name,
				  void *value, size_t size)
{
	struct inode *inode = d_inode(dentry);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_peer *conn = info->conn;
	char *send_buf = NULL;
	ssize_t res = 0;

	send_buf = hmdfs_get_dentry_relative_path(dentry);
	if (!send_buf)
		return -ENOMEM;

	res = hmdfs_send_getxattr(conn, send_buf, name, value, size);
	kfree(send_buf);
	return res;
}

static int hmdfs_xattr_get(const struct xattr_handler *handler,
			   struct dentry *dentry, struct inode *inode,
			   const char *name, void *value, size_t size)
{
	int res = 0;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	size_t r_size = size;

	if (!hmdfs_support_xattr(dentry))
		return -EOPNOTSUPP;

	if (strncmp(name, XATTR_USER_PREFIX, XATTR_USER_PREFIX_LEN))
		return -EOPNOTSUPP;

	if (size > HMDFS_XATTR_SIZE_MAX)
		r_size = HMDFS_XATTR_SIZE_MAX;

	if (info->inode_type == HMDFS_LAYER_OTHER_LOCAL)
		res = hmdfs_xattr_local_get(dentry, name, value, r_size);
	else
		res = hmdfs_xattr_remote_get(dentry, name, value, r_size);

	if (res == -ERANGE && r_size != size) {
		hmdfs_info("no support xattr value size over than: %d",
			   HMDFS_XATTR_SIZE_MAX);
		res = -E2BIG;
	}

	return res;
}

static int hmdfs_xattr_local_set(struct dentry *dentry, const char *name,
				 const void *value, size_t size, int flags)
{
	struct path lower_path;
	int res = 0;

	hmdfs_get_lower_path(dentry, &lower_path);
	if (value) {
		res = vfs_setxattr(lower_path.dentry, name, value, size, flags);
	} else {
		WARN_ON(flags != XATTR_REPLACE);
		res = vfs_removexattr(lower_path.dentry, name);
	}

	hmdfs_put_lower_path(&lower_path);
	return res;
}

static int hmdfs_xattr_remote_set(struct dentry *dentry, const char *name,
				  const void *value, size_t size, int flags)
{
	struct inode *inode = d_inode(dentry);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_peer *conn = info->conn;
	char *send_buf = NULL;
	int res = 0;

	send_buf = hmdfs_get_dentry_relative_path(dentry);
	if (!send_buf)
		return -ENOMEM;

	res = hmdfs_send_setxattr(conn, send_buf, name, value, size, flags);
	kfree(send_buf);
	return res;
}

static int hmdfs_xattr_set(const struct xattr_handler *handler,
			   struct dentry *dentry, struct inode *inode,
			   const char *name, const void *value,
			   size_t size, int flags)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);

	if (!hmdfs_support_xattr(dentry))
		return -EOPNOTSUPP;

	if (strncmp(name, XATTR_USER_PREFIX, XATTR_USER_PREFIX_LEN))
		return -EOPNOTSUPP;

	if (size > HMDFS_XATTR_SIZE_MAX) {
		hmdfs_info("no support too long xattr value: %ld", size);
		return -E2BIG;
	}

	if (info->inode_type == HMDFS_LAYER_OTHER_LOCAL)
		return hmdfs_xattr_local_set(dentry, name, value, size, flags);

	return hmdfs_xattr_remote_set(dentry, name, value, size, flags);
}

const struct xattr_handler hmdfs_xattr_handler = {
	.prefix = "", /* catch all */
	.get = hmdfs_xattr_get,
	.set = hmdfs_xattr_set,
};

static const struct xattr_handler *hmdfs_xattr_handlers[] = {
	&hmdfs_xattr_handler,
};

#define HMDFS_NODE_EVT_CB_DELAY 2

struct kmem_cache *hmdfs_inode_cachep;
struct kmem_cache *hmdfs_dentry_cachep;

static void i_callback(struct rcu_head *head)
{
	struct inode *inode = container_of(head, struct inode, i_rcu);

	kmem_cache_free(hmdfs_inode_cachep,
			container_of(inode, struct hmdfs_inode_info,
				     vfs_inode));
}

static void hmdfs_destroy_inode(struct inode *inode)
{
	call_rcu(&inode->i_rcu, i_callback);
}

static void hmdfs_evict_inode(struct inode *inode)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);

	truncate_inode_pages(&inode->i_data, 0);
	clear_inode(inode);
	if (info->inode_type == HMDFS_LAYER_FIRST_DEVICE ||
	    info->inode_type == HMDFS_LAYER_SECOND_REMOTE)
		return;
	if (info->inode_type == HMDFS_LAYER_ZERO ||
	    info->inode_type == HMDFS_LAYER_OTHER_LOCAL ||
	    info->inode_type == HMDFS_LAYER_SECOND_LOCAL) {
		iput(info->lower_inode);
		info->lower_inode = NULL;
	}
}

void hmdfs_put_super(struct super_block *sb)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(sb);
	struct super_block *lower_sb = sbi->lower_sb;

	hmdfs_info("local_dst is %s, local_src is %s", sbi->local_dst,
		   sbi->local_src);

	hmdfs_fault_inject_fini(&sbi->fault_inject);
	hmdfs_cfn_destroy(sbi);
	hmdfs_unregister_sysfs(sbi);
	hmdfs_connections_stop(sbi);
	hmdfs_destroy_server_writeback(sbi);
	hmdfs_exit_stash(sbi);
	atomic_dec(&lower_sb->s_active);
	put_cred(sbi->cred);
	if (sbi->system_cred)
		put_cred(sbi->system_cred);
	hmdfs_destroy_writeback(sbi);
	kfree(sbi->local_src);
	kfree(sbi->local_dst);
	kfree(sbi->real_dst);
	kfree(sbi->cache_dir);
	kfifo_free(&sbi->notify_fifo);
	sb->s_fs_info = NULL;
	sbi->lower_sb = NULL;
	hmdfs_release_sysfs(sbi);
	/* After all access are completed */
	hmdfs_free_sb_seq(sbi->seq);
	kfree(sbi->s_server_statis);
	kfree(sbi->s_client_statis);
	kfree(sbi);
}

static struct inode *hmdfs_alloc_inode(struct super_block *sb)
{
	struct hmdfs_inode_info *gi =
		kmem_cache_alloc(hmdfs_inode_cachep, GFP_KERNEL);
	if (!gi)
		return NULL;
	memset(gi, 0, offsetof(struct hmdfs_inode_info, vfs_inode));
	INIT_LIST_HEAD(&gi->wb_list);
	init_rwsem(&gi->wpage_sem);
	gi->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
	atomic64_set(&gi->write_counter, 0);
	gi->fid.id = HMDFS_INODE_INVALID_FILE_ID;
	spin_lock_init(&gi->fid_lock);
	INIT_LIST_HEAD(&gi->wr_opened_node);
	atomic_set(&gi->wr_opened_cnt, 0);
	init_waitqueue_head(&gi->fid_wq);
	INIT_LIST_HEAD(&gi->stash_node);
	spin_lock_init(&gi->stash_lock);
	return &gi->vfs_inode;
}

static int hmdfs_remote_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	int error = 0;
	int ret = 0;
	char *dir_path = NULL;
	char *name_path = NULL;
	struct hmdfs_peer *con = NULL;
	struct hmdfs_sb_info *sbi = hmdfs_sb(dentry->d_inode->i_sb);

	dir_path = hmdfs_get_dentry_relative_path(dentry->d_parent);
	if (!dir_path) {
		error = -EACCES;
		goto rmdir_out;
	}

	name_path = hmdfs_connect_path(dir_path, dentry->d_name.name);
	if (!name_path) {
		error = -EACCES;
		goto rmdir_out;
	}
	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(con, &sbi->connections.node_list, list) {
		if (con->status == NODE_STAT_ONLINE &&
		    con->version > USERSPACE_MAX_VER) {
			peer_get(con);
			mutex_unlock(&sbi->connections.node_lock);
			hmdfs_debug("send MSG to remote devID %llu",
				    con->device_id);
			ret = hmdfs_send_statfs(con, name_path, buf);
			if (ret != 0)
				error = ret;
			peer_put(con);
			mutex_lock(&sbi->connections.node_lock);
		}
	}
	mutex_unlock(&sbi->connections.node_lock);

rmdir_out:
	kfree(dir_path);
	kfree(name_path);
	return error;
}

static int hmdfs_statfs(struct dentry *dentry, struct kstatfs *buf)
{
	int err = 0;
	struct path lower_path;
	struct hmdfs_inode_info *info = hmdfs_i(dentry->d_inode);
	struct super_block *sb = d_inode(dentry)->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;

	trace_hmdfs_statfs(dentry, info->inode_type);
	// merge_view & merge_view/xxx & device_view assigned src_inode info
	if (hmdfs_i_merge(info) ||
	    (info->inode_type == HMDFS_LAYER_SECOND_REMOTE)) {
		err = kern_path(sbi->local_src, 0, &lower_path);
		if (err)
			goto out;
		err = vfs_statfs(&lower_path, buf);
		path_put(&lower_path);
	} else if (!IS_ERR_OR_NULL(info->lower_inode)) {
		hmdfs_get_lower_path(dentry, &lower_path);
		err = vfs_statfs(&lower_path, buf);
		hmdfs_put_lower_path(&lower_path);
	} else {
		err = hmdfs_remote_statfs(dentry, buf);
	}

	buf->f_type = HMDFS_SUPER_MAGIC;
out:
	return err;
}

static int hmdfs_show_options(struct seq_file *m, struct dentry *root)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(root->d_sb);

	if (sbi->s_case_sensitive)
		seq_puts(m, ",sensitive");
	else
		seq_puts(m, ",insensitive");

	if (sbi->s_merge_switch)
		seq_puts(m, ",merge_enable");
	else
		seq_puts(m, ",merge_disable");

	seq_printf(m, ",ra_pages=%lu", root->d_sb->s_bdi->ra_pages);
	seq_printf(m, ",account=%s", sbi->local_info.account);

	if (sbi->cache_dir)
		seq_printf(m, ",cache_dir=%s", sbi->cache_dir);
	if (sbi->real_dst)
		seq_printf(m, ",real_dst=%s", sbi->real_dst);

	seq_printf(m, ",%soffline_stash", sbi->s_offline_stash ? "" : "no_");
	seq_printf(m, ",%sdentry_cache", sbi->s_dentry_cache ? "" : "no_");

	return 0;
}

static int hmdfs_sync_fs(struct super_block *sb, int wait)
{
	int time_left;
	int err = 0;
	struct hmdfs_peer *con = NULL;
	struct hmdfs_sb_info *sbi = hmdfs_sb(sb);
	int syncfs_timeout = get_cmd_timeout(sbi, F_SYNCFS);
	struct syncfs_item item, *entry = NULL, *tmp = NULL;

	if (!wait)
		return 0;

	trace_hmdfs_syncfs_enter(sbi);

	spin_lock(&sbi->hsi.list_lock);
	if (!sbi->hsi.is_executing) {
		sbi->hsi.is_executing = true;
		item.need_abort = false;
		spin_unlock(&sbi->hsi.list_lock);
	} else {
		init_completion(&item.done);
		list_add_tail(&item.list, &sbi->hsi.wait_list);
		spin_unlock(&sbi->hsi.list_lock);
		wait_for_completion(&item.done);
	}

	if (item.need_abort)
		goto out;

	/*
	 * Syncfs can not concurrent in hmdfs_sync_fs. Because we should make
	 * sure all remote syncfs calls return back or timeout by waiting,
	 * during the waiting period we must protect @sbi->remote_syncfs_count
	 * and @sbi->remote_syncfs_ret from concurrent executing.
	 */

	spin_lock(&sbi->hsi.v_lock);
	sbi->hsi.version++;
	/*
	 * Attention: We put @sbi->hsi.remote_ret and @sbi->hsi.wait_count
	 * into spinlock protection area to avoid following scenario caused
	 * by out-of-order execution:
	 *
	 *            synfs                                  syncfs_cb
	 *  sbi->hsi.remote_ret = 0;
	 *  atomic_set(&sbi->hsi.wait_count, 0);
	 *                                               lock
	 *                                               version == old_version
	 *                                 sbi->hsi.remote_ret = resp->ret_code
	 *                                 atomic_dec(&sbi->hsi.wait_count);
	 *                                               unlock
	 *         lock
	 *         version = old_version + 1
	 *         unlock
	 *
	 * @sbi->hsi.remote_ret and @sbi->hsi.wait_count can be assigned
	 * before spin lock which may compete with syncfs_cb(), making
	 * these two values' assignment protected by spinlock can fix this.
	 */
	sbi->hsi.remote_ret = 0;
	atomic_set(&sbi->hsi.wait_count, 0);
	spin_unlock(&sbi->hsi.v_lock);

	mutex_lock(&sbi->connections.node_lock);
	list_for_each_entry(con, &sbi->connections.node_list, list) {
		/*
		 * Dirty data does not need to be synchronized to remote
		 * devices that go offline normally. It's okay to drop
		 * them.
		 */
		if (con->status != NODE_STAT_ONLINE)
			continue;

		peer_get(con);
		mutex_unlock(&sbi->connections.node_lock);

		/*
		 * There exists a gap between sync_inodes_sb() and sync_fs()
		 * which may race with remote writing, leading error count
		 * on @sb_dirty_count. The dirty data produced during the
		 * gap period won't be synced in next syncfs operation.
		 * To avoid this, we have to invoke sync_inodes_sb() again
		 * after getting @con->sb_dirty_count.
		 */
		con->old_sb_dirty_count = atomic64_read(&con->sb_dirty_count);
		sync_inodes_sb(sb);

		if (!con->old_sb_dirty_count) {
			peer_put(con);
			mutex_lock(&sbi->connections.node_lock);
			continue;
		}

		err = hmdfs_send_syncfs(con, syncfs_timeout);
		if (err) {
			hmdfs_warning("send syncfs failed with %d on node %llu",
				      err, con->device_id);
			sbi->hsi.remote_ret = err;
			peer_put(con);
			mutex_lock(&sbi->connections.node_lock);
			continue;
		}

		atomic_inc(&sbi->hsi.wait_count);

		peer_put(con);
		mutex_lock(&sbi->connections.node_lock);
	}
	mutex_unlock(&sbi->connections.node_lock);

	/*
	 * Async work in background will make sure @sbi->remote_syncfs_count
	 * decreased to zero finally whether syncfs success or fail.
	 */
	time_left = wait_event_interruptible(
		sbi->hsi.wq, atomic_read(&sbi->hsi.wait_count) == 0);
	if (time_left < 0) {
		hmdfs_warning("syncfs is interrupted by external signal");
		err = -EINTR;
	}

	if (!err && sbi->hsi.remote_ret)
		err = sbi->hsi.remote_ret;

	/* Abandon syncfs processes in pending_list */
	list_for_each_entry_safe(entry, tmp, &sbi->hsi.pending_list, list) {
		entry->need_abort = true;
		complete(&entry->done);
	}
	INIT_LIST_HEAD(&sbi->hsi.pending_list);

	/* Pick the last syncfs process in wait_list */
	spin_lock(&sbi->hsi.list_lock);
	if (list_empty(&sbi->hsi.wait_list)) {
		sbi->hsi.is_executing = false;
	} else {
		entry = list_last_entry(&sbi->hsi.wait_list, struct syncfs_item,
					list);
		list_del_init(&entry->list);
		list_splice_init(&sbi->hsi.wait_list, &sbi->hsi.pending_list);
		entry->need_abort = false;
		complete(&entry->done);
	}
	spin_unlock(&sbi->hsi.list_lock);

out:
	trace_hmdfs_syncfs_exit(sbi, atomic_read(&sbi->hsi.wait_count),
				get_cmd_timeout(sbi, F_SYNCFS), err);

	/* TODO: Return synfs err back to syscall */

	return err;
}

struct super_operations hmdfs_sops = {
	.alloc_inode = hmdfs_alloc_inode,
	.destroy_inode = hmdfs_destroy_inode,
	.evict_inode = hmdfs_evict_inode,
	.put_super = hmdfs_put_super,
	.statfs = hmdfs_statfs,
	.show_options = hmdfs_show_options,
	.sync_fs = hmdfs_sync_fs,
};

static void init_once(void *obj)
{
	struct hmdfs_inode_info *i = obj;

	inode_init_once(&i->vfs_inode);
}

static int __init hmdfs_init_caches(void)
{
	int err = -ENOMEM;

	hmdfs_inode_cachep =
		kmem_cache_create("hmdfs_inode_cache",
				  sizeof(struct hmdfs_inode_info), 0,
				  SLAB_RECLAIM_ACCOUNT, init_once);
	if (unlikely(!hmdfs_inode_cachep))
		goto out;
	hmdfs_dentry_cachep =
		kmem_cache_create("hmdfs_dentry_cache",
				  sizeof(struct hmdfs_dentry_info), 0,
				  SLAB_RECLAIM_ACCOUNT, NULL);
	if (unlikely(!hmdfs_dentry_cachep))
		goto out_des_ino;
	hmdfs_dentry_merge_cachep =
		kmem_cache_create("hmdfs_dentry_merge_cache",
				  sizeof(struct hmdfs_dentry_info_merge), 0,
				  SLAB_RECLAIM_ACCOUNT, NULL);
	if (unlikely(!hmdfs_dentry_merge_cachep))
		goto out_des_dc;
	return 0;

out_des_dc:
	kmem_cache_destroy(hmdfs_dentry_cachep);
out_des_ino:
	kmem_cache_destroy(hmdfs_inode_cachep);
out:
	return err;
}

static void hmdfs_destroy_caches(void)
{
	rcu_barrier();
	kmem_cache_destroy(hmdfs_inode_cachep);
	hmdfs_inode_cachep = NULL;
	kmem_cache_destroy(hmdfs_dentry_cachep);
	hmdfs_dentry_cachep = NULL;
	kmem_cache_destroy(hmdfs_dentry_merge_cachep);
	hmdfs_dentry_merge_cachep = NULL;
}

uint64_t path_hash(const char *path, int len, bool case_sense)
{
	uint64_t res = 0;
	const char *kp = path;
	char c;
	/* Mocklisp hash function. */
	while (*kp) {
		c = *kp;
		if (!case_sense)
			c = tolower(c);
		res = (res << 5) - res + (uint64_t)(c);
		kp++;
	}
	return res;
}

static char *get_full_path(struct path *path)
{
	char *buf, *tmp;
	char *ret = NULL;

	buf = kmalloc(PATH_MAX, GFP_KERNEL);
	if (!buf)
		goto out;

	tmp = d_path(path, buf, PATH_MAX);
	if (IS_ERR(tmp))
		goto out;

	ret = kstrdup(tmp, GFP_KERNEL);
out:
	kfree(buf);
	return ret;
}

static void hmdfs_init_cmd_timeout(struct hmdfs_sb_info *sbi)
{
	memset(sbi->s_cmd_timeout, 0xff, sizeof(sbi->s_cmd_timeout));

	set_cmd_timeout(sbi, F_OPEN, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_RELEASE, TIMEOUT_NONE);
	set_cmd_timeout(sbi, F_READPAGE, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_WRITEPAGE, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_ITERATE, TIMEOUT_30S);
	set_cmd_timeout(sbi, F_CREATE, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_MKDIR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_RMDIR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_UNLINK, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_RENAME, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_SETATTR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_CONNECT_ECHO, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_STATFS, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_CONNECT_REKEY, TIMEOUT_NONE);
	set_cmd_timeout(sbi, F_DROP_PUSH, TIMEOUT_NONE);
	set_cmd_timeout(sbi, F_GETATTR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_FSYNC, TIMEOUT_90S);
	set_cmd_timeout(sbi, F_SYNCFS, TIMEOUT_30S);
	set_cmd_timeout(sbi, F_GETXATTR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_SETXATTR, TIMEOUT_COMMON);
	set_cmd_timeout(sbi, F_LISTXATTR, TIMEOUT_COMMON);
}

static int hmdfs_init_sbi(struct hmdfs_sb_info *sbi)
{
	int ret;

	ret = kfifo_alloc(&sbi->notify_fifo, PAGE_SIZE, GFP_KERNEL);
	if (ret)
		goto out;

	/*
	 * We have to use dynamic memory since struct server/client_statistic
	 * are DECLARED in hmdfs.h but DEFINED in socket_adapter.h.
	 */
	sbi->s_server_statis =
		kzalloc(sizeof(*sbi->s_server_statis) * F_SIZE, GFP_KERNEL);
	sbi->s_client_statis =
		kzalloc(sizeof(*sbi->s_client_statis) * F_SIZE, GFP_KERNEL);
	if (!sbi->s_server_statis || !sbi->s_client_statis) {
		ret = -ENOMEM;
		goto out;
	}

	ret = hmdfs_alloc_sb_seq();
	if (ret < 0) {
		hmdfs_err("no sb seq available err %d", ret);
		goto out;
	}
	sbi->seq = ret;
	ret = 0;

	spin_lock_init(&sbi->notify_fifo_lock);
	sbi->s_case_sensitive = false;
	sbi->s_features = HMDFS_FEATURE_READPAGES |
			  HMDFS_FEATURE_READPAGES_OPEN |
			  HMDFS_ATOMIC_OPEN;
	sbi->s_merge_switch = false;
	sbi->dcache_threshold = DEFAULT_DCACHE_THRESHOLD;
	sbi->dcache_precision = DEFAULT_DCACHE_PRECISION;
	sbi->dcache_timeout = DEFAULT_DCACHE_TIMEOUT;
	sbi->write_cache_timeout = DEFAULT_WRITE_CACHE_TIMEOUT;
	hmdfs_init_cmd_timeout(sbi);
	sbi->async_cb_delay = HMDFS_NODE_EVT_CB_DELAY;
	sbi->async_req_max_active = DEFAULT_SRV_REQ_MAX_ACTIVE;
	sbi->s_offline_stash = true;
	sbi->s_dentry_cache = true;
	sbi->wb_timeout_ms = HMDFS_DEF_WB_TIMEOUT_MS;
	/* Initialize before hmdfs_register_sysfs() */
	atomic_set(&sbi->connections.conn_seq, 0);
	mutex_init(&sbi->connections.node_lock);
	INIT_LIST_HEAD(&sbi->connections.node_list);

	init_waitqueue_head(&sbi->async_readdir_wq);
	INIT_LIST_HEAD(&sbi->async_readdir_msg_list);
	INIT_LIST_HEAD(&sbi->async_readdir_work_list);
	spin_lock_init(&sbi->async_readdir_msg_lock);
	spin_lock_init(&sbi->async_readdir_work_lock);

	return 0;

out:
	return ret;
}

void hmdfs_client_resp_statis(struct hmdfs_sb_info *sbi, u8 cmd,
			      enum hmdfs_resp_type type, unsigned long start,
			      unsigned long end)
{
	unsigned long duration;

	switch (type) {
	case HMDFS_RESP_DELAY:
		sbi->s_client_statis[cmd].delay_resp_cnt++;
		break;
	case HMDFS_RESP_TIMEOUT:
		sbi->s_client_statis[cmd].timeout_cnt++;
		break;
	case HMDFS_RESP_NORMAL:
		duration = end - start;
		sbi->s_client_statis[cmd].total += duration;
		sbi->s_client_statis[cmd].resp_cnt++;
		if (sbi->s_client_statis[cmd].max < duration)
			sbi->s_client_statis[cmd].max = duration;
		break;
	default:
		hmdfs_err("Wrong cmd %d with resp type %d", cmd, type);
	}
}

static int hmdfs_update_dst(struct hmdfs_sb_info *sbi)
{
	int err = 0;
	const char *path_local = UPDATE_LOCAL_DST;
	int len = 0;

	sbi->real_dst = kstrdup(sbi->local_dst, GFP_KERNEL);
	if (!sbi->real_dst) {
		err = -ENOMEM;
		goto out_err;
	}
	kfree(sbi->local_dst);

	len = strlen(sbi->real_dst) + strlen(path_local) + 1;
	if (len > PATH_MAX) {
		err = -EINVAL;
		goto out_err;
	}
	sbi->local_dst = kmalloc(len, GFP_KERNEL);
	if (!sbi->local_dst) {
		err = -ENOMEM;
		goto out_err;
	}
	snprintf(sbi->local_dst, strlen(sbi->real_dst) + strlen(path_local) + 1,
		 "%s%s", sbi->real_dst, path_local);
out_err:
	return err;
}

/*
 * Generate boot cookie like following format:
 *
 * | random |   boot time(ms) |  0x00 |
 * |--------|-----------------|-------|
 *     16            33          15    (bits)
 *
 * This will make sure boot cookie is unique in a period
 * 2^33 / 1000 / 3600 / 24 = 99.4(days).
 */
uint64_t hmdfs_gen_boot_cookie(void)
{
	uint64_t now;
	uint16_t rand;

	now = ktime_to_ms(ktime_get());
	prandom_bytes(&rand, sizeof(rand));

	now &= (1ULL << HMDFS_BOOT_COOKIE_RAND_SHIFT) - 1;
	now |= ((uint64_t)rand << HMDFS_BOOT_COOKIE_RAND_SHIFT);

	return now << HMDFS_FID_VER_BOOT_COOKIE_SHIFT;
}

static int hmdfs_fill_super(struct super_block *sb, void *data, int silent)
{
	struct hmdfs_mount_priv *priv = (struct hmdfs_mount_priv *)data;
	const char *dev_name = priv->dev_name;
	const char *raw_data = priv->raw_data;
	struct hmdfs_sb_info *sbi;
	int err = 0;
	struct inode *root_inode;
	struct path lower_path;
	struct super_block *lower_sb;
	struct dentry *root_dentry;
	char ctrl_path[CTRL_PATH_MAX_LEN];
	uint64_t ctrl_hash;

	sbi = kzalloc(sizeof(*sbi), GFP_KERNEL);
	if (!sbi) {
		err = -ENOMEM;
		goto out_err;
	}
	err = hmdfs_init_sbi(sbi);
	if (err)
		goto out_freesbi;
	sbi->sb = sb;
	err = hmdfs_parse_options(sbi, raw_data);
	if (err)
		goto out_freesbi;

	sb->s_fs_info = sbi;
	sb->s_magic = HMDFS_SUPER_MAGIC;
	sb->s_xattr = hmdfs_xattr_handlers;
	sb->s_op = &hmdfs_sops;

	sbi->boot_cookie = hmdfs_gen_boot_cookie();

	err = hmdfs_init_writeback(sbi);
	if (err)
		goto out_freesbi;
	err = hmdfs_init_server_writeback(sbi);
	if (err)
		goto out_freesbi;

	err = hmdfs_init_stash(sbi);
	if (err)
		goto out_freesbi;

	// add ctrl sysfs node
	ctrl_hash = path_hash(sbi->local_dst, strlen(sbi->local_dst), true);
	scnprintf(ctrl_path, CTRL_PATH_MAX_LEN, "%llu", ctrl_hash);
	hmdfs_debug("hash %llu", ctrl_hash);
	err = hmdfs_register_sysfs(ctrl_path, sbi);
	if (err)
		goto out_freesbi;

	err = hmdfs_update_dst(sbi);
	if (err)
		goto out_unreg_sysfs;

	err = kern_path(dev_name, LOOKUP_FOLLOW | LOOKUP_DIRECTORY,
			&lower_path);
	if (err) {
		hmdfs_err("open dev failed, errno = %d", err);
		goto out_unreg_sysfs;
	}

	lower_sb = lower_path.dentry->d_sb;
	atomic_inc(&lower_sb->s_active);
	sbi->lower_sb = lower_sb;
	sbi->local_src = get_full_path(&lower_path);
	if (!sbi->local_src) {
		hmdfs_err("get local_src failed!");
		goto out_sput;
	}

	sb->s_time_gran = lower_sb->s_time_gran;
	sb->s_maxbytes = lower_sb->s_maxbytes;
	sb->s_stack_depth = lower_sb->s_stack_depth + 1;
	if (sb->s_stack_depth > FILESYSTEM_MAX_STACK_DEPTH) {
		hmdfs_err("maximum fs stacking depth exceeded");
		err = -EINVAL;
		goto out_sput;
	}
	root_inode = fill_root_inode(sb, d_inode(lower_path.dentry));
	if (IS_ERR(root_inode)) {
		err = PTR_ERR(root_inode);
		goto out_sput;
	}
	hmdfs_root_inode_perm_init(root_inode);
	sb->s_root = root_dentry = d_make_root(root_inode);
	if (!root_dentry) {
		err = -ENOMEM;
		goto out_sput;
	}
#ifdef CONFIG_HMDFS_1_0
	hmdfs_i(root_inode)->adapter_dentry_flag = ADAPTER_OTHER_DENTRY_FLAG;
#endif
	err = init_hmdfs_dentry_info(sbi, root_dentry, HMDFS_LAYER_ZERO);
	if (err)
		goto out_freeroot;
	hmdfs_set_lower_path(root_dentry, &lower_path);
	d_rehash(sb->s_root);
	sbi->cred = get_cred(current_cred());
	INIT_LIST_HEAD(&sbi->client_cache);
	INIT_LIST_HEAD(&sbi->server_cache);
	INIT_LIST_HEAD(&sbi->to_delete);
	mutex_init(&sbi->cache_list_lock);
	hmdfs_cfn_load(sbi);

	/* Initialize syncfs info */
	spin_lock_init(&sbi->hsi.v_lock);
	init_waitqueue_head(&sbi->hsi.wq);
	sbi->hsi.version = 0;
	sbi->hsi.is_executing = false;
	INIT_LIST_HEAD(&sbi->hsi.wait_list);
	INIT_LIST_HEAD(&sbi->hsi.pending_list);
	spin_lock_init(&sbi->hsi.list_lock);
	hmdfs_fault_inject_init(&sbi->fault_inject, ctrl_path);

	return err;
out_freeroot:
	dput(sb->s_root);
	sb->s_root = NULL;
out_sput:
	atomic_dec(&lower_sb->s_active);
	path_put(&lower_path);
out_unreg_sysfs:
	hmdfs_unregister_sysfs(sbi);
	hmdfs_release_sysfs(sbi);
out_freesbi:
	if (sbi) {
		sb->s_fs_info = NULL;
		hmdfs_exit_stash(sbi);
		hmdfs_destroy_writeback(sbi);
		hmdfs_destroy_server_writeback(sbi);
		kfifo_free(&sbi->notify_fifo);
		hmdfs_free_sb_seq(sbi->seq);
		kfree(sbi->local_src);
		kfree(sbi->local_dst);
		kfree(sbi->real_dst);
		kfree(sbi->cache_dir);
		kfree(sbi->s_server_statis);
		kfree(sbi->s_client_statis);
		kfree(sbi);
	}
out_err:
	return err;
}

static struct dentry *hmdfs_mount(struct file_system_type *fs_type, int flags,
				  const char *dev_name, void *raw_data)
{
	struct hmdfs_mount_priv priv = {
		.dev_name = dev_name,
		.raw_data = raw_data,
	};
	return mount_nodev(fs_type, flags, &priv, hmdfs_fill_super);
}


static void hmdfs_cancel_async_readdir(struct hmdfs_sb_info *sbi)
{
	struct sendmsg_wait_queue *msg_wq = NULL;
	struct hmdfs_readdir_work *rw = NULL;
	struct hmdfs_readdir_work *tmp = NULL;
	struct list_head del_work;

	/* cancel work that are not runing */

	INIT_LIST_HEAD(&del_work);
	spin_lock(&sbi->async_readdir_work_lock);
	list_for_each_entry_safe(rw, tmp, &sbi->async_readdir_work_list, head) {
		if (cancel_delayed_work(&rw->dwork))
			list_move(&rw->head, &del_work);
	}
	spin_unlock(&sbi->async_readdir_work_lock);

	list_for_each_entry_safe(rw, tmp, &del_work, head) {
		dput(rw->dentry);
		peer_put(rw->con);
		kfree(rw);
	}

	/* wake up async readdir that are waiting for remote */
	spin_lock(&sbi->async_readdir_msg_lock);
	sbi->async_readdir_prohibit = true;
	list_for_each_entry(msg_wq, &sbi->async_readdir_msg_list, async_msg)
		hmdfs_response_wakeup(msg_wq, -EINTR, 0, NULL);
	spin_unlock(&sbi->async_readdir_msg_lock);

	/* wait for all async readdir to finish */
	if (!list_empty(&sbi->async_readdir_work_list))
		wait_event_interruptible_timeout(sbi->async_readdir_wq,
			(list_empty(&sbi->async_readdir_work_list)), HZ);

	WARN_ON(!(list_empty(&sbi->async_readdir_work_list)));
}

static void hmdfs_kill_super(struct super_block *sb)
{
	struct hmdfs_sb_info *sbi = hmdfs_sb(sb);

	/*
	 * async readdir is holding ref for dentry, not for vfsmount. Thus
	 * shrink_dcache_for_umount() will warn about dentry still in use
	 * if async readdir is not done.
	 */
	if (sbi)
		hmdfs_cancel_async_readdir(sbi);
	kill_anon_super(sb);
}

static struct file_system_type hmdfs_fs_type = {
	.owner = THIS_MODULE,
	.name = "hmdfs",
	.mount = hmdfs_mount,
	.kill_sb = hmdfs_kill_super,
};

static int __init hmdfs_init(void)
{
	int err = 0;

	err = hmdfs_init_caches();
	if (err)
		goto out_err;

	hmdfs_node_evt_cb_init();

	hmdfs_stash_add_node_evt_cb();
	hmdfs_client_add_node_evt_cb();
	hmdfs_server_add_node_evt_cb();

	err = register_filesystem(&hmdfs_fs_type);
	if (err) {
		hmdfs_err("hmdfs register failed!");
		goto out_err;
	}
#ifdef CONFIG_HMDFS_ANDROID
	err = hmdfs_packagelist_init();
	if (err)
		goto out_err;
#endif
	err = hmdfs_sysfs_init();
	if (err)
		goto out_err;

#ifdef CONFIG_HMDFS_1_0
	err = dentry_syncer_init();
	if (err) {
		hmdfs_err("hmdfs dentry_syncer inode register failed");
		goto out_err;
	}
	err = hmdfs_adapter_file_id_generator_init();
	if (err) {
		hmdfs_err("hmdfs_adapter_file_id_generator_init failed");
		goto out_err;
	}
#endif
	hmdfs_message_verify_init();
	hmdfs_create_debugfs_root();
	return 0;
out_err:
#ifdef CONFIG_HMDFS_ANDROID
	hmdfs_packagelist_exit();
#endif
	hmdfs_sysfs_exit();
	unregister_filesystem(&hmdfs_fs_type);
	hmdfs_destroy_caches();
	hmdfs_err("hmdfs init failed!");
	return err;
}

static void __exit hmdfs_exit(void)
{
#ifdef CONFIG_HMDFS_1_0
	hmdfs_adapter_file_id_generator_cleanup();
	dentry_syncer_exit();
#endif
	hmdfs_destroy_debugfs_root();
	hmdfs_sysfs_exit();
#ifdef CONFIG_HMDFS_ANDROID
	hmdfs_packagelist_exit();
#endif
	unregister_filesystem(&hmdfs_fs_type);
	ida_destroy(&hmdfs_sb_seq);
	hmdfs_destroy_caches();
	hmdfs_info("hmdfs exited!");
}

module_init(hmdfs_init);
module_exit(hmdfs_exit);

EXPORT_TRACEPOINT_SYMBOL_GPL(hmdfs_recv_mesg_callback);

MODULE_LICENSE("GPL v2");
MODULE_AUTHOR("LongPing.WEI, Jingjing.Mao");
MODULE_DESCRIPTION("Harmony distributed file system");
