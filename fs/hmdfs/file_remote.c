/* SPDX-License-Identifier: GPL-2.0 */
/*
 * file_remote.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: weilongping@huawei.com lvhao13@huawei.com
 * Create: 2020-12-21
 *
 */

#include <linux/backing-dev.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/namei.h>
#include <linux/page-flags.h>
#include <linux/pagemap.h>
#include <linux/pagevec.h>
#include <linux/sched/signal.h>
#include <linux/slab.h>
#include <linux/wait.h>

#include "file_remote.h"

#include "comm/socket_adapter.h"
#include "hmdfs.h"
#include "hmdfs_client.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_trace.h"

static inline bool hmdfs_remote_write_cache_expired(
		struct hmdfs_inode_info *info)
{
	return time_after(jiffies, info->writecache_expire);
}

enum expire_reason {
	ALL_GOOD = 0,
	INO_DISMATCH = 1,
	SIZE_OR_CTIME_DISMATCH = 2,
	TIMER_EXPIRE = 3,
	TIMER_WORKING = 4,
	STABLE_CTIME_DISMATCH = 5,
	KEEP_CACHE = 6,
};

/*
 * hmdfs_open_final_remote - Do final steps of opening a remote file, update
 * local inode cache and decide whether of not to truncate inode pages.
 *
 * @info: hmdfs inode info
 * @open_ret: values returned from remote when opening a remote file
 * @keep_cache: keep local cache & i_size
 */
static int hmdfs_open_final_remote(struct hmdfs_inode_info *info,
				   struct hmdfs_open_ret *open_ret,
				   struct file *file, bool keep_cache)
{
	struct inode *inode = &info->vfs_inode;
	bool truncate = false;
	enum expire_reason reason = ALL_GOOD;
	int ret = 0;

	/*
	 * if remote inode number changed and lookup stale data, we'll return
	 * -ESTALE, and reopen the file with metedate from remote getattr.
	 */
	if (info->remote_ino != open_ret->ino) {
		hmdfs_debug(
			"got stale local inode, ino in local %llu, ino from open %llu",
			info->remote_ino, open_ret->ino);
		hmdfs_send_close(info->conn, &open_ret->fid);
		reason = INO_DISMATCH;
		ret = -ESTALE;
		goto out;
	}

	if (keep_cache) {
		reason = KEEP_CACHE;
		trace_hmdfs_open_final_remote(info, open_ret, file, reason);
		goto set_fid_out;
	}

	/*
	 * if remote size do not match local inode, or remote ctime do not match
	 * the last time same file was opened.
	 */
	if (inode->i_size != open_ret->file_size ||
	    hmdfs_time_compare(&info->remote_ctime, &open_ret->remote_ctime)) {
		truncate = true;
		reason = SIZE_OR_CTIME_DISMATCH;
		goto out;
	}

	/*
	 * If 'writecache_expire' is set, check if it expires. And skip the
	 * checking of stable_ctime.
	 */
	if (info->writecache_expire) {
		truncate = hmdfs_remote_write_cache_expired(info);
		if (truncate)
			reason = TIMER_EXPIRE;
		else
			reason = TIMER_WORKING;
		goto out;
	}

	/* the first time, or remote ctime is ahead of remote time */
	if (info->stable_ctime.tv_sec == 0 && info->stable_ctime.tv_nsec == 0) {
		truncate = true;
		reason = STABLE_CTIME_DISMATCH;
		goto out;
	}

	/*
	 * - if last stable_ctime == stable_ctime, we do nothing.
	 *   a. if ctime < stable_ctime, data is ensured to be uptodate,
	 *   b. if ctime == stable_ctime, stale data might be accessed. This is
	 *      acceptable since pagecache will be droped later.
	 *   c. ctime > stable_ctime is impossible.
	 * - if last stable_ctime < stable_ctime, we clear the cache.
	 *   d. ctime != last stable_ctime is impossible
	 *   e. ctime == last stable_ctime, this is possible to read again from
	 *      b, thus we need to drop the cache.
	 * - if last stable_ctime > stable_ctime, we clear the cache.
	 *   stable_ctime must be zero in this case, this is possible because
	 *   system time might be changed.
	 */
	if (hmdfs_time_compare(&info->stable_ctime, &open_ret->stable_ctime)) {
		truncate = true;
		reason = STABLE_CTIME_DISMATCH;
		goto out;
	}

out:
	trace_hmdfs_open_final_remote(info, open_ret, file, reason);
	if (ret)
		return ret;

	if (reason == SIZE_OR_CTIME_DISMATCH) {
		inode->i_ctime = open_ret->remote_ctime;
		info->remote_ctime = open_ret->remote_ctime;
	}

	if (truncate) {
		info->writecache_expire = 0;
		truncate_inode_pages(inode->i_mapping, 0);
	}

	atomic64_set(&info->write_counter, 0);
	info->stable_ctime = open_ret->stable_ctime;
	i_size_write(inode, open_ret->file_size);
	info->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
set_fid_out:
	spin_lock(&info->fid_lock);
	info->fid = open_ret->fid;
	spin_unlock(&info->fid_lock);
	return 0;
}

int hmdfs_do_open_remote(struct file *file, bool keep_cache)
{
	struct hmdfs_inode_info *info = hmdfs_i(file_inode(file));
	struct hmdfs_peer *conn = info->conn;
	struct hmdfs_open_ret open_ret;
	__u8 file_type = hmdfs_d(file->f_path.dentry)->file_type;
	char *send_buf;
	int err = 0;

	send_buf = hmdfs_get_dentry_relative_path(file->f_path.dentry);
	if (!send_buf) {
		err = -ENOMEM;
		goto out_free;
	}
	err = hmdfs_send_open(conn, send_buf, file_type, &open_ret);
	if (err) {
		hmdfs_err("hmdfs_send_open return failed with %d", err);
		goto out_free;
	}

	err = hmdfs_open_final_remote(info, &open_ret, file, keep_cache);

out_free:
	kfree(send_buf);
	return err;
}

static inline bool hmdfs_remote_need_reopen(struct hmdfs_inode_info *info)
{
	return test_bit(HMDFS_FID_NEED_OPEN, &info->fid_flags);
}

static inline bool hmdfs_remote_is_opening_file(struct hmdfs_inode_info *info)
{
	return test_bit(HMDFS_FID_OPENING, &info->fid_flags);
}

static int hmdfs_remote_wait_opening_file(struct hmdfs_inode_info *info)
{
	int err;

	if (!hmdfs_remote_is_opening_file(info))
		return 0;

	err = ___wait_event(info->fid_wq, hmdfs_remote_is_opening_file(info),
			    TASK_INTERRUPTIBLE, 0, 0,
			    spin_unlock(&info->fid_lock);
			    schedule();
			    spin_lock(&info->fid_lock));
	if (err)
		err = -EINTR;

	return err;
}

static int hmdfs_remote_file_reopen(struct hmdfs_inode_info *info,
				    struct file *filp)
{
	int err = 0;
	struct hmdfs_peer *conn = info->conn;
	struct inode *inode = NULL;
	struct hmdfs_fid fid;

	if (conn->status == NODE_STAT_OFFLINE)
		return -EAGAIN;

	spin_lock(&info->fid_lock);
	err = hmdfs_remote_wait_opening_file(info);
	if (err || !hmdfs_remote_need_reopen(info)) {
		spin_unlock(&info->fid_lock);
		goto out;
	}

	set_bit(HMDFS_FID_OPENING, &info->fid_flags);
	fid = info->fid;
	spin_unlock(&info->fid_lock);

	inode = &info->vfs_inode;
	inode_lock(inode);
	/*
	 * Most closing cases are meaningless, except for one:
	 *        read process A         read process B
	 *    err = -EBADF              err = -EBADF       (caused by re-online)
	 *    set_need_reopen
	 *    do reopen
	 *    fid = new fid_1 [server hold fid_1]
	 *                              set need_reopen
	 *                              do reopen
	 *                                send close (fid_1) // In case of leak
	 *                              fid = new fid_2
	 */
	if (fid.id != HMDFS_INODE_INVALID_FILE_ID)
		hmdfs_send_close(conn, &fid);
	err = hmdfs_do_open_remote(filp, true);
	inode_unlock(inode);

	spin_lock(&info->fid_lock);
	/*
	 * May make the bit set in offline handler lost, but server
	 * will tell us whether or not the newly-opened file id is
	 * generated before offline, if it is opened before offline,
	 * the operation on the file id will return -EBADF and
	 * HMDFS_FID_NEED_OPEN bit will be set again.
	 */
	if (!err)
		clear_bit(HMDFS_FID_NEED_OPEN, &info->fid_flags);
	clear_bit(HMDFS_FID_OPENING, &info->fid_flags);
	spin_unlock(&info->fid_lock);

	wake_up_interruptible_all(&info->fid_wq);
out:
	return err;
}

static int hmdfs_remote_check_and_reopen(struct hmdfs_inode_info *info,
					 struct file *filp)
{
	if (!hmdfs_remote_need_reopen(info))
		return 0;

	return hmdfs_remote_file_reopen(info, filp);
}

void hmdfs_do_close_remote(struct kref *kref)
{
	struct hmdfs_inode_info *info =
		container_of(kref, struct hmdfs_inode_info, ref);
	struct hmdfs_fid fid;

	hmdfs_remote_fetch_fid(info, &fid);
	/* This function can return asynchronously */
	hmdfs_send_close(info->conn, &fid);
}

static inline bool hmdfs_remote_need_track_file(const struct hmdfs_sb_info *sbi,
						fmode_t mode)
{
	return (hmdfs_is_stash_enabled(sbi) && (mode & FMODE_WRITE));
}

static void
hmdfs_remote_del_wr_opened_inode_nolock(struct hmdfs_inode_info *info)
{
	WARN_ON(list_empty(&info->wr_opened_node));
	if (atomic_dec_and_test(&info->wr_opened_cnt))
		list_del_init(&info->wr_opened_node);
}

void hmdfs_remote_del_wr_opened_inode(struct hmdfs_peer *conn,
				      struct hmdfs_inode_info *info)
{
	spin_lock(&conn->wr_opened_inode_lock);
	hmdfs_remote_del_wr_opened_inode_nolock(info);
	spin_unlock(&conn->wr_opened_inode_lock);
}

void hmdfs_remote_add_wr_opened_inode_nolock(struct hmdfs_peer *conn,
					     struct hmdfs_inode_info *info)
{
	if (list_empty(&info->wr_opened_node)) {
		atomic_set(&info->wr_opened_cnt, 1);
		list_add_tail(&info->wr_opened_node,
			      &conn->wr_opened_inode_list);
	} else {
		atomic_inc(&info->wr_opened_cnt);
	}
}

static void hmdfs_remote_add_wr_opened_inode(struct hmdfs_peer *conn,
					     struct hmdfs_inode_info *info)
{
	spin_lock(&conn->wr_opened_inode_lock);
	hmdfs_remote_add_wr_opened_inode_nolock(conn, info);
	spin_unlock(&conn->wr_opened_inode_lock);
}

int hmdfs_file_open_remote(struct inode *inode, struct file *file)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct kref *ref = &(info->ref);
	int err = 0;

	inode_lock(inode);
	if (kref_read(ref) == 0) {
		err = hmdfs_do_open_remote(file, false);
		if (err == 0)
			kref_init(ref);
	} else {
		kref_get(ref);
	}
	inode_unlock(inode);

	if (!err && hmdfs_remote_need_track_file(hmdfs_sb(inode->i_sb),
						 file->f_mode))
		hmdfs_remote_add_wr_opened_inode(info->conn, info);

	return err;
}

static void hmdfs_set_writecache_expire(struct hmdfs_inode_info *info,
					unsigned int seconds)
{
	unsigned long new_expire = jiffies + seconds * HZ;

	/*
	 * When file has been written before closing, set pagecache expire
	 * if it has not been set yet. This is necessary because ctime might
	 * stay the same after overwrite.
	 */
	if (info->writecache_expire &&
	    time_after(new_expire, info->writecache_expire))
		return;

	info->writecache_expire = new_expire;
}

static void hmdfs_remote_keep_writecache(struct inode *inode, struct file *file)
{
	struct hmdfs_inode_info *info = NULL;
	struct kref *ref = NULL;
	struct hmdfs_getattr_ret *getattr_ret = NULL;
	unsigned int write_cache_timeout =
		hmdfs_sb(inode->i_sb)->write_cache_timeout;
	int err;

	if (!write_cache_timeout)
		return;

	info = hmdfs_i(inode);
	ref = &(info->ref);
	/*
	 * don't do anything if file is still opening or file hasn't been
	 * written.
	 */
	if (kref_read(ref) > 0 || !atomic64_read(&info->write_counter))
		return;

	/*
	 * If remote getattr failed, and we don't update ctime,
	 * pagecache will be truncated the next time file is opened.
	 */
	err = hmdfs_remote_getattr(info->conn, file_dentry(file), 0,
				   &getattr_ret);
	if (err) {
		hmdfs_err("remote getattr failed with err %d", err);
		return;
	}

	if (!(getattr_ret->stat.result_mask & STATX_CTIME)) {
		hmdfs_err("get remote ctime failed with mask 0x%x",
			  getattr_ret->stat.result_mask);
		kfree(getattr_ret);
		return;
	}
	/*
	 * update ctime from remote, in case that pagecahe will be
	 * truncated in next open.
	 */
	inode->i_ctime = getattr_ret->stat.ctime;
	info->remote_ctime = getattr_ret->stat.ctime;
	hmdfs_set_writecache_expire(info, write_cache_timeout);
	kfree(getattr_ret);
}

int hmdfs_file_release_remote(struct inode *inode, struct file *file)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);

	if (hmdfs_remote_need_track_file(hmdfs_sb(inode->i_sb), file->f_mode))
		hmdfs_remote_del_wr_opened_inode(info->conn, info);

	inode_lock(inode);
	kref_put(&info->ref, hmdfs_do_close_remote);
	hmdfs_remote_keep_writecache(inode, file);
	inode_unlock(inode);

	return 0;
}

static int hmdfs_file_flush(struct file *file, fl_owner_t id)
{
	int err = 0;
	struct inode *inode = file_inode(file);

	if (!(file->f_mode & FMODE_WRITE))
		return 0;

	/*
	 * Continue regardless of whether file reopen fails or not,
	 * because there may be no dirty page.
	 */
	hmdfs_remote_check_and_reopen(hmdfs_i(inode), file);

	/*
	 * Wait for wsem here would impact the performance greatly, so we
	 * overlap the time to issue as many wbs as we can, expecting async
	 * wbs are eliminated afterwards.
	 */
	filemap_fdatawrite(inode->i_mapping);
	down_write(&hmdfs_i(inode)->wpage_sem);
	err = filemap_write_and_wait(inode->i_mapping);
	up_write(&hmdfs_i(inode)->wpage_sem);
	return err;
}

static ssize_t hmdfs_file_read_iter_remote(struct kiocb *iocb,
					   struct iov_iter *iter)
{
	struct file *filp = iocb->ki_filp;
	struct hmdfs_inode_info *info = hmdfs_i(file_inode(filp));
	struct file_ra_state *ra = NULL;
	unsigned int rtt;
	int err;
	bool tried = false;

retry:
	err = hmdfs_remote_check_and_reopen(info, filp);
	if (err)
		return err;

	ra = &filp->f_ra;
	/* rtt is measured in 10 msecs */
	rtt = hmdfs_tcpi_rtt(info->conn) / 10000;
	switch (rtt) {
	case 0:
		break;
	case 1:
		ra->ra_pages = 256;
		break;
	case 2:
		ra->ra_pages = 512;
		break;
	default:
		ra->ra_pages = 1024;
		break;
	}

	err = generic_file_read_iter(iocb, iter);
	if (err < 0 && !tried && hmdfs_remote_need_reopen(info)) {
		/* Read from a stale fid, try read again once. */
		tried = true;
		goto retry;
	}

	return err;
}

static inline bool hmdfs_is_file_unwritable(const struct hmdfs_inode_info *info,
					    bool check_stash)
{
	return (check_stash && hmdfs_inode_is_stashing(info)) ||
	       !hmdfs_is_node_online(info->conn);
}

static ssize_t __hmdfs_file_write_iter_remote(struct kiocb *iocb,
					      struct iov_iter *iter,
					      bool check_stash)
{
	struct file *filp = iocb->ki_filp;
	struct inode *inode = file_inode(filp);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	ssize_t ret;

	if (hmdfs_is_file_unwritable(info, check_stash))
		return -EAGAIN;

	ret = hmdfs_remote_check_and_reopen(info, filp);
	if (ret)
		return ret;

	inode_lock(inode);
	if (hmdfs_is_file_unwritable(info, check_stash)) {
		ret = -EAGAIN;
		goto out;
	}
	ret = generic_write_checks(iocb, iter);
	if (ret > 0)
		ret = __generic_file_write_iter(iocb, iter);
out:
	inode_unlock(inode);

	if (ret > 0)
		ret = generic_write_sync(iocb, ret);
	return ret;
}

ssize_t hmdfs_file_write_iter_remote_nocheck(struct kiocb *iocb,
					     struct iov_iter *iter)
{
	return __hmdfs_file_write_iter_remote(iocb, iter, false);
}

static ssize_t hmdfs_file_write_iter_remote(struct kiocb *iocb,
					    struct iov_iter *iter)
{
	return __hmdfs_file_write_iter_remote(iocb, iter, true);
}

/* hmdfs not support mmap write remote file */
static hmdfs_vm_fault_t hmdfs_page_mkwrite(struct vm_fault *vmf)
{
	return VM_FAULT_SIGBUS;
}

static const struct vm_operations_struct hmdfs_file_vm_ops = {
	.fault = filemap_fault,
	.map_pages = filemap_map_pages,
	.page_mkwrite = hmdfs_page_mkwrite,
};

static int hmdfs_file_mmap_remote(struct file *file, struct vm_area_struct *vma)
{
	vma->vm_ops = &hmdfs_file_vm_ops;
	file_accessed(file);

	return 0;
}

static int hmdfs_file_fsync_remote(struct file *file, loff_t start, loff_t end,
				   int datasync)
{
	struct hmdfs_inode_info *info = hmdfs_i(file_inode(file));
	struct hmdfs_peer *conn = info->conn;
	struct hmdfs_fid fid;
	int err;

	trace_hmdfs_fsync_enter_remote(conn->sbi, conn->device_id,
				       info->remote_ino, datasync);
	/*
	 * Continue regardless of whether file reopen fails or not,
	 * because there may be no dirty page.
	 */
	hmdfs_remote_check_and_reopen(info, file);

	filemap_fdatawrite(file->f_mapping);
	down_write(&info->wpage_sem);
	err = file_write_and_wait_range(file, start, end);
	up_write(&info->wpage_sem);
	if (err) {
		hmdfs_err("local fsync fail with %d", err);
		goto out;
	}

	hmdfs_remote_fetch_fid(info, &fid);
	err = hmdfs_send_fsync(conn, &fid, start, end, datasync);
	if (err)
		hmdfs_err("send fsync fail with %d", err);

out:
	trace_hmdfs_fsync_exit_remote(conn->sbi, conn->device_id,
				      info->remote_ino,
				      get_cmd_timeout(conn->sbi, F_FSYNC), err);

	/* Compatible with POSIX retcode */
	if (err == -ETIME)
		err = -EIO;

	return err;
}

const struct file_operations hmdfs_dev_file_fops_remote = {
	.owner = THIS_MODULE,
	.llseek = generic_file_llseek,
	.read_iter = hmdfs_file_read_iter_remote,
	.write_iter = hmdfs_file_write_iter_remote,
	.mmap = hmdfs_file_mmap_remote,
	.open = hmdfs_file_open_remote,
	.release = hmdfs_file_release_remote,
	.flush = hmdfs_file_flush,
	.fsync = hmdfs_file_fsync_remote,
};

static void hmdfs_fill_page_zero(struct page *page)
{
	void *addr = NULL;

	addr = kmap(page);
	memset(addr, 0, PAGE_SIZE);
	kunmap(page);
	SetPageUptodate(page);
	unlock_page(page);
}

static int hmdfs_readpage_remote(struct file *file, struct page *page)
{
	struct inode *inode = file_inode(file);
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	loff_t isize = i_size_read(inode);
	pgoff_t end_index = (isize - 1) >> PAGE_SHIFT;
	struct hmdfs_fid fid;

	if (!isize || page->index > end_index) {
		hmdfs_fill_page_zero(page);
		return 0;
	}

	if (!isize || page->index > end_index) {
		hmdfs_fill_page_zero(page);
		return 0;
	}

	hmdfs_remote_fetch_fid(info, &fid);
	return hmdfs_client_readpage(info->conn, &fid, page);
}

uint32_t hmdfs_get_writecount(struct page *page)
{
	uint32_t count = 0;
	loff_t pos = (loff_t)page->index << HMDFS_PAGE_OFFSET;
	struct inode *inode = page->mapping->host;
	loff_t size = i_size_read(inode);
	/*
	 * If page offset is greater than i_size, this is possible when
	 * writepage concurrent with truncate. In this case, we don't need to
	 * do remote writepage since it'll be truncated after the page is
	 * unlocked.
	 */
	if (pos >= size)
		count = 0;
	/*
	 * If the page about to write is beyond i_size, we can't write beyond
	 * i_size because remote file size will be wrong.
	 */
	else if (size < pos + HMDFS_PAGE_SIZE)
		count = size - pos;
	/* It's safe to write the whole page */
	else
		count = HMDFS_PAGE_SIZE;

	return count;
}

static bool allow_cur_thread_wpage(struct hmdfs_inode_info *info,
				   bool *rsem_held, bool sync_all)
{
	WARN_ON(!rsem_held);

	if (sync_all) {
		*rsem_held = false;
		return true;
	}
	*rsem_held = down_read_trylock(&info->wpage_sem);
	return *rsem_held;
}

/**
 * hmdfs_writepage_remote - writeback a dirty page to remote
 *
 * INFO:
 * When asked to WB_SYNC_ALL, this function should leave with both the page and
 * the radix tree node clean to achieve close-to-open consitency. Moreover,
 * this shall never return -EIO to help filemap to iterate all dirty pages.
 *
 * INFO:
 * When asked to WB_SYNC_NONE, this function should be mercy if faults(oom or
 * bad pipe) happended to enable subsequent r/w & wb.
 */
static int hmdfs_writepage_remote(struct page *page,
				  struct writeback_control *wbc)
{
	struct inode *inode = page->mapping->host;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_sb_info *sbi = hmdfs_sb(inode->i_sb);
	int ret = 0;
	bool rsem_held = false;
	bool sync = wbc->sync_mode == WB_SYNC_ALL;
	struct hmdfs_writepage_context *param = NULL;

	if (!allow_cur_thread_wpage(info, &rsem_held, sync))
		goto out_unlock;

	set_page_writeback(page);

	param = kzalloc(sizeof(*param), GFP_NOFS);
	if (!param) {
		ret = -ENOMEM;
		goto out_endwb;
	}

	if (sync && hmdfs_usr_sig_pending(current)) {
		ClearPageUptodate(page);
		goto out_free;
	}
	param->count = hmdfs_get_writecount(page);
	if (!param->count)
		goto out_free;
	param->rsem_held = rsem_held;
	hmdfs_remote_fetch_fid(info, &param->fid);
	param->sync_all = sync;
	param->caller = current;
	get_task_struct(current);
	param->page = page;
	param->timeout = jiffies + msecs_to_jiffies(sbi->wb_timeout_ms);
	INIT_DELAYED_WORK(&param->retry_dwork, hmdfs_remote_writepage_retry);
	ret = hmdfs_remote_do_writepage(info->conn, param);
	if (likely(!ret))
		return 0;

	put_task_struct(current);
out_free:
	kfree(param);
out_endwb:
	end_page_writeback(page);
	if (rsem_held)
		up_read(&info->wpage_sem);
out_unlock:
	if (sync || !hmdfs_need_redirty_page(info, ret)) {
		SetPageError(page);
		mapping_set_error(page->mapping, ret);
	} else {
		redirty_page_for_writepage(wbc, page);
	}
	unlock_page(page);
	return ret;
}

static void hmdfs_account_dirty_pages(struct address_space *mapping)
{
	struct hmdfs_sb_info *sbi = mapping->host->i_sb->s_fs_info;

	if (!sbi->h_wb->dirty_writeback_control)
		return;

	this_cpu_inc(*sbi->h_wb->bdp_ratelimits);
}

static int hmdfs_write_begin_remote(struct file *file,
				    struct address_space *mapping, loff_t pos,
				    unsigned int len, unsigned int flags,
				    struct page **pagep, void **fsdata)
{
	pgoff_t index = ((unsigned long long)pos) >> PAGE_SHIFT;
	struct inode *inode = file_inode(file);
	struct page *page = NULL;
	int ret = 0;

start:
	page = grab_cache_page_write_begin(mapping, index, AOP_FLAG_NOFS);
	if (!page)
		return -ENOMEM;
	*pagep = page;
	wait_on_page_writeback(page);

	// If this page will be covered completely.
	if (len == HMDFS_PAGE_SIZE || PageUptodate(page))
		return 0;

	/*
	 * If data existed in this page will covered,
	 * we just need to clear this page.
	 */
	if (!((unsigned long long)pos & (HMDFS_PAGE_SIZE - 1)) &&
	    (pos + len) >= i_size_read(inode)) {
		zero_user_segment(page, len, HMDFS_PAGE_SIZE);
		return 0;
	}
	/*
	 * We need readpage before write date to this page.
	 */
	ret = hmdfs_readpage_remote(file, page);
	if (!ret) {
		if (PageLocked(page)) {
			ret = __lock_page_killable(page);
			if (!ret)
				unlock_page(page);
		}

		if (!ret && PageUptodate(page)) {
			put_page(page);
			goto start;
		}
		if (!ret)
			ret = -EIO;
	}
	put_page(page);
	return ret;
}

static int hmdfs_write_end_remote(struct file *file,
				  struct address_space *mapping, loff_t pos,
				  unsigned int len, unsigned int copied,
				  struct page *page, void *fsdata)
{
	struct inode *inode = page->mapping->host;

	if (!PageUptodate(page)) {
		if (unlikely(copied != len))
			copied = 0;
		else
			SetPageUptodate(page);
	}
	if (!copied)
		goto unlock_out;

	if (!PageDirty(page)) {
		hmdfs_account_dirty_pages(mapping);
		set_page_dirty(page);
	}

	if (pos + copied > i_size_read(inode)) {
		i_size_write(inode, pos + copied);
		hmdfs_i(inode)->getattr_isize = HMDFS_STALE_REMOTE_ISIZE;
	}
unlock_out:
	unlock_page(page);
	put_page(page);

	/* hmdfs private writeback control */
	hmdfs_balance_dirty_pages_ratelimited(mapping);
	return copied;
}

const struct address_space_operations hmdfs_dev_file_aops_remote = {
	.readpage = hmdfs_readpage_remote,
	.write_begin = hmdfs_write_begin_remote,
	.write_end = hmdfs_write_end_remote,
	.writepage = hmdfs_writepage_remote,
	.set_page_dirty = __set_page_dirty_nobuffers,
};

unsigned long hmdfs_set_pos(unsigned long dev_id, unsigned long group_id,
			    unsigned long offset)
{
	unsigned long pos;

	pos = (dev_id << (POS_BIT_NUM - 1 - DEV_ID_BIT_NUM)) +
	      (group_id << OFFSET_BIT_NUM) + offset;
	if (dev_id)
		pos |= (1UL << (POS_BIT_NUM - 1));
	return pos;
}

static int analysis_dentry_file_from_con(struct hmdfs_sb_info *sbi,
					 struct file *file,
					 struct file *handler,
					 struct dir_context *ctx)
{
	struct hmdfs_dentry_group *dentry_group = NULL;
	unsigned long pos = (unsigned long)(ctx->pos);
	unsigned long dev_id = (pos << 1) >> (POS_BIT_NUM - DEV_ID_BIT_NUM);
	unsigned long group_id = (pos << (1 + DEV_ID_BIT_NUM)) >>
				 (POS_BIT_NUM - GROUP_ID_BIT_NUM);
	unsigned long offset = pos & OFFSET_BIT_MASK;
	int group_num = 0;
	char *dentry_name = NULL;
	int iterate_result = 0;
	int i, j;

	dentry_group = kzalloc(sizeof(*dentry_group), GFP_KERNEL);

	if (!dentry_group)
		return -ENOMEM;

	if (IS_ERR_OR_NULL(handler)) {
		kfree(dentry_group);
		return -ENOENT;
	}

	group_num = get_dentry_group_cnt(file_inode(handler));
	dentry_name = kzalloc(DENTRY_NAME_MAX_LEN, GFP_KERNEL);
	if (!dentry_name) {
		kfree(dentry_group);
		return -ENOMEM;
	}

	for (i = group_id; i < group_num; i++) {
		int ret = hmdfs_metainfo_read(sbi, handler, dentry_group,
					      sizeof(struct hmdfs_dentry_group),
					      i);
		if (ret != sizeof(struct hmdfs_dentry_group)) {
			hmdfs_err("read dentry group failed ret:%d", ret);
			goto done;
		}

		for (j = offset; j < DENTRY_PER_GROUP; j++) {
			int len;
			int file_type = DT_UNKNOWN;
			bool is_continue;

			len = le16_to_cpu(dentry_group->nsl[j].namelen);
			if (!test_bit_le(j, dentry_group->bitmap) || len == 0)
				continue;

			memset(dentry_name, 0, DENTRY_NAME_MAX_LEN);
			// TODO: Support more file_type
			if (S_ISDIR(le16_to_cpu(dentry_group->nsl[j].i_mode)))
				file_type = DT_DIR;
			else if (S_ISREG(le16_to_cpu(
					 dentry_group->nsl[j].i_mode)))
				file_type = DT_REG;

			strncat(dentry_name, dentry_group->filename[j], len);
			pos = hmdfs_set_pos(dev_id, i, j);
			is_continue =
				dir_emit(ctx, dentry_name, len,
					 pos + INUNUMBER_START, file_type);
			if (!is_continue) {
				ctx->pos = pos;
				iterate_result = 1;
				goto done;
			}
		}
		offset = 0;
	}

done:
	kfree(dentry_name);
	kfree(dentry_group);
	return iterate_result;
}

int hmdfs_dev_readdir_from_con(struct hmdfs_peer *con, struct file *file,
			       struct dir_context *ctx)
{
	int iterate_result = 0;

	iterate_result = analysis_dentry_file_from_con(
		con->sbi, file, file->private_data, ctx);
	return iterate_result;
}

static int hmdfs_iterate_remote(struct file *file, struct dir_context *ctx)
{
	int err = 0;
	loff_t start_pos = ctx->pos;
	struct hmdfs_peer *con = NULL;
	struct hmdfs_dentry_info *di = hmdfs_d(file->f_path.dentry);
	bool is_local = !((unsigned long)(ctx->pos) >> (POS_BIT_NUM - 1));
	uint64_t dev_id = di->device_id;

	if (ctx->pos == -1)
		return 0;
	if (is_local)
		ctx->pos = hmdfs_set_pos(dev_id, 0, 0);

	con = hmdfs_lookup_from_devid(file->f_inode->i_sb->s_fs_info, dev_id);
	if (con) {
		// ctx->pos = 0;
		err = con->conn_operations->remote_readdir(con, file, ctx);
		if (unlikely(!con)) {
			hmdfs_err("con is null");
			goto done;
		}
		peer_put(con);
		if (err)
			goto done;
	}

done:
	if (err <= 0)
		ctx->pos = -1;

	trace_hmdfs_iterate_remote(file->f_path.dentry, start_pos, ctx->pos,
				   err);
	return err;
}

int hmdfs_dir_open_remote(struct inode *inode, struct file *file)
{
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct clearcache_item *cache_item = NULL;

	if (info->conn && info->conn->version <= USERSPACE_MAX_VER) {
		return 0;
	} else if (info->conn) {
		if (!hmdfs_cache_revalidate(READ_ONCE(info->conn->conn_time),
					    info->conn->device_id,
					    file->f_path.dentry))
			get_remote_dentry_file_sync(file->f_path.dentry,
						    info->conn);
		cache_item = hmdfs_find_cache_item(info->conn->device_id,
						   file->f_path.dentry);
		if (cache_item) {
			file->private_data = cache_item->filp;
			get_file(file->private_data);
			kref_put(&cache_item->ref, release_cache_item);
			return 0;
		}
		return -ENOENT;
	}
	return -ENOENT;
}

static int hmdfs_dir_release_remote(struct inode *inode, struct file *file)
{
	if (file->private_data)
		fput(file->private_data);
	file->private_data = NULL;
	return 0;
}

const struct file_operations hmdfs_dev_dir_ops_remote = {
	.owner = THIS_MODULE,
	.iterate = hmdfs_iterate_remote,
	.open = hmdfs_dir_open_remote,
	.release = hmdfs_dir_release_remote,
	.fsync = __generic_file_fsync,
};
