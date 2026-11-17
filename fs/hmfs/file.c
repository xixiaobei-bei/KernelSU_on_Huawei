// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/file.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/fs.h>
#include <linux/hmfs_fs.h>
#include <linux/stat.h>
#include <linux/buffer_head.h>
#include <linux/writeback.h>
#include <linux/blkdev.h>
#include <linux/falloc.h>
#include <linux/types.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/mount.h>
#include <linux/pagevec.h>
#include <linux/uio.h>
#include <linux/uuid.h>
#include <linux/file.h>

#include "hmfs.h"
#include "node.h"
#include "segment.h"
#include "xattr.h"
#include "acl.h"
#include "gc.h"
#include "trace.h"
#include <trace/events/hmfs.h>
//#include <trace/events/block.h>

#define F2FS_WAIT_FSYNC_TIMEOUT 100
#define F2FS_I_FSYNC_FLAG_SCHED_WORK 1
#define F2FS_I_FSYNC_FLAG_WAITING 2

static int f2fs_filemap_fault(struct vm_fault *vmf)
{
	struct inode *inode = file_inode(vmf->vma->vm_file);
	int err;

	err = fscrypt_vm_op_check(inode);
	if (unlikely(err & VM_FAULT_SIGSEGV)) {
		pr_err("[FBE3]%s: mmap_read_check return VM_FAULT_SIGSEGV\n", __func__);
		return err;
	}

	down_read(&F2FS_I(inode)->i_mmap_sem);
	err = filemap_fault(vmf);
	up_read(&F2FS_I(inode)->i_mmap_sem);

	return err;
}

static int f2fs_vm_page_mkwrite(struct vm_fault *vmf)
{
	struct page *page = vmf->page;
	struct inode *inode = file_inode(vmf->vma->vm_file);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct dnode_of_data dn;
	int err;
	bool lock2 = false;

	err = fscrypt_vm_op_check(inode);
	if (unlikely(err & VM_FAULT_SIGSEGV)) {
		pr_err("[FBE3]%s: mmap_read_check return VM_FAULT_SIGSEGV\n", __func__);
		return err;
	}

	if (unlikely(f2fs_cp_error(sbi))) {
		err = -EIO;
		goto err;
	}

	hmfs_io_throttle(sbi, PAGE_SIZE);

	/* should do out of any locked page */
	hmfs_balance_fs(sbi, true);

	sb_start_pagefault(inode->i_sb);

	f2fs_bug_on(sbi, f2fs_has_inline_data(inode));

	file_update_time(vmf->vma->vm_file);
	down_read(&F2FS_I(inode)->i_mmap_sem);
	lock_page(page);
	if (unlikely(page->mapping != inode->i_mapping ||
			page_offset(page) > i_size_read(inode) ||
			!PageUptodate(page))) {
		unlock_page(page);
		err = -EFAULT;
		goto out_sem;
	}

	/* block allocation */
	lock2 = __hmfs_do_map_lock(sbi, F2FS_GET_BLOCK_PRE_AIO, true, false);
	set_new_dnode(&dn, inode, NULL, NULL, 0);
	err = hmfs_get_block(&dn, page->index);
	f2fs_put_dnode(&dn);
	(void)__hmfs_do_map_lock(sbi, F2FS_GET_BLOCK_PRE_AIO, false, lock2);
	if (err) {
		unlock_page(page);
		goto out_sem;
	}

	/* fill the page */
	hmfs_wait_on_page_writeback(page, DATA, false);

	/* wait for GCed page writeback via META_MAPPING */
	hmfs_wait_on_block_writeback(inode, dn.data_blkaddr);

	/*
	 * check to see if the page is mapped already (no holes)
	 */
	if (PageMappedToDisk(page))
		goto out_sem;

	/* page is wholly or partially inside EOF */
	if (((loff_t)(page->index + 1) << PAGE_SHIFT) >
						i_size_read(inode)) {
		loff_t offset;

		offset = i_size_read(inode) & ~PAGE_MASK;
		zero_user_segment(page, offset, PAGE_SIZE);
	}
	set_page_dirty(page);
	if (!PageUptodate(page))
		SetPageUptodate(page);

	f2fs_update_iostat(sbi, APP_MAPPED_IO, F2FS_BLKSIZE);
	f2fs_update_time(sbi, REQ_TIME);

	trace_hmfs_vm_page_mkwrite(page, DATA);
out_sem:
	up_read(&F2FS_I(inode)->i_mmap_sem);

	sb_end_pagefault(inode->i_sb);
err:
	return block_page_mkwrite_return(err);
}

static const struct vm_operations_struct f2fs_file_vm_ops = {
	.fault		= f2fs_filemap_fault,
	.map_pages	= filemap_map_pages,
	.page_mkwrite	= f2fs_vm_page_mkwrite,
};

static int get_parent_ino(struct inode *inode, nid_t *pino)
{
	struct dentry *dentry;

	inode = igrab(inode);
	dentry = d_find_any_alias(inode);
	iput(inode);
	if (!dentry)
		return 0;

	*pino = parent_ino(dentry);
	dput(dentry);
	return 1;
}

static inline enum cp_reason_type need_do_checkpoint(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	enum cp_reason_type cp_reason = CP_NO_NEEDED;
	nid_t nid = fi->i_xattr_nid;

	if (!S_ISREG(inode->i_mode))
		cp_reason = CP_NON_REGULAR;
	else if (inode->i_nlink != 1)
		cp_reason = CP_HARDLINK;
	else if (is_sbi_flag_set(sbi, SBI_NEED_CP))
		cp_reason = CP_SB_NEED_CP;
	else if (file_wrong_pino(inode))
		cp_reason = CP_WRONG_PINO;
	else if (!hmfs_space_for_roll_forward(sbi))
		cp_reason = CP_NO_SPC_ROLL;
	else if (!hmfs_is_checkpointed_node(sbi, F2FS_I(inode)->i_pino))
		cp_reason = CP_NODE_NEED_CP;
	else if (test_opt(sbi, FASTBOOT))
		cp_reason = CP_FASTBOOT_MODE;
	else if (F2FS_OPTION(sbi).active_logs == 2)
		cp_reason = CP_SPEC_LOG_NUM;
	else if (F2FS_OPTION(sbi).fsync_mode == FSYNC_MODE_STRICT &&
		hmfs_need_dentry_mark(sbi, inode->i_ino) &&
		hmfs_exist_written_data(sbi, F2FS_I(inode)->i_pino,
							TRANS_DIR_INO))
		cp_reason = CP_RECOVER_DIR;
	else if (hmfs_is_oob_enable(sbi)) {
		if (hmfs_is_file_switch_stream(inode))
			cp_reason = CP_SWITCH_STREAM;
		else if (nid && hmfs_is_node_page_dirty(sbi, nid))
			cp_reason = CP_XATTR_DIRTY;
		else if (hmfs_is_exceed_oob_cnt(sbi))
			cp_reason = CP_OOB_OVERFLOW;
		else if (hmfs_is_file_atomic_switch(inode))
			cp_reason = CP_OOB_CHG_ATOMIC;
	}

	return cp_reason;
}

static bool need_inode_page_update(struct f2fs_sb_info *sbi, nid_t ino)
{
	bool ret = false;
	struct page *i;

	if (!hmfs_is_oob_enable(sbi)) {
		i = find_get_page(NODE_MAPPING(sbi), ino);
		/* But we need to avoid that there are some inode updates */
		if ((i && PageDirty(i)) || hmfs_need_inode_block_update(sbi, ino))
			ret = true;
		f2fs_put_page(i, 0);
		return ret;
	}

	if (hmfs_need_inode_block_update(sbi, ino))
		ret = true;

	trace_hmfs_ipage_update(ino, ret);
	return ret;

}

static void try_to_fix_pino(struct inode *inode)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	nid_t pino;

	down_write(&fi->i_sem);
	if (file_wrong_pino(inode) && inode->i_nlink == 1 &&
			get_parent_ino(inode, &pino)) {
		f2fs_i_pino_write(inode, pino);
		file_got_pino(inode);
	}
	up_write(&fi->i_sem);
}

static void hmfs_set_fsync_flag(struct f2fs_inode_info *fi, unsigned int flag)
{
	spin_lock(&fi->vfs_inode.i_lock);
	fi->i_fsync_flag = flag;
	spin_unlock(&fi->vfs_inode.i_lock);
}

void hmfs_flush_wait_fsync(struct file *file)
{
	struct inode *inode = file_inode(file);
	struct f2fs_inode_info *fi = F2FS_I(inode);

	if ((file->hm_fsync_flag) && (fi->i_fsync_flag)) {
		cancel_delayed_work_sync(&fi->fsync_work);
		if (fi->i_fsync_flag) {
			hmfs_set_fsync_flag(fi, F2FS_I_FSYNC_FLAG_WAITING);
			filemap_fdatawait_range(fi->vfs_inode.i_mapping, 0, LLONG_MAX);
			hmfs_set_fsync_flag(fi, 0);
		}
	}

	if (file->hm_fsync_flag) {
		spin_lock(&file->f_lock);
		file->hm_fsync_flag = 0;
		spin_unlock(&file->f_lock);
	}
}

void hmfs_wait_writeback_work_fn(struct work_struct *work)
{
	struct f2fs_inode_info *fi = container_of(work,
				struct f2fs_inode_info, fsync_work.work);

	hmfs_set_fsync_flag(fi, F2FS_I_FSYNC_FLAG_WAITING);
	filemap_fdatawait_range(fi->vfs_inode.i_mapping, 0, LLONG_MAX);
	hmfs_set_fsync_flag(fi, 0);
	return;
}

/*
 * sync inode/node for 2 purposes:
 * 1) record dirty inode/quota info
 * 2) record fsync_mark to hint: need recover this inode
 */
static bool need_sync_node(struct inode *inode,
		int dsync, bool dirty_inode, int dirty_pages)
{
	bool need_sync = false;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	unsigned long long cur_ver = cur_cp_version(F2FS_CKPT(sbi));

	/* mainly for 2) purpose */
	if (!dirty_pages && fi->fsync_ver != cur_ver && fi->has_wb) {
		need_sync = true;
		goto done;
	}

	/* left part for 1) purpose */
	if (dsync || !dirty_inode)
		goto done;

	/* only for debug */
	if (sbi->skip_sync_node)
		goto done;

	need_sync = true;
done:
	trace_hmfs_node_sync(inode, dsync, fi->fsync_ver == cur_ver,
			dirty_pages, dirty_inode, need_sync);

	return need_sync;
}

static void f2fs_ff_enable_start_fsync_work(struct f2fs_inode_info *fi)
{
	int i;

	if (!fi->i_fsync_flag) {
		fi->i_fsync_flag = F2FS_I_FSYNC_FLAG_SCHED_WORK;
		for (i = 1; i <= NR_CPUS; i++) {
			if (cpu_online((raw_smp_processor_id() + i) % NR_CPUS))
				break;
		}
		schedule_delayed_work_on(
			(raw_smp_processor_id() + i) % NR_CPUS,
			&fi->fsync_work, msecs_to_jiffies(10)); /* 10 mseconds */
	}
}

static void f2fs_ff_enable_out(struct file *file, struct f2fs_inode_info *fi,
	struct inode *inode, loff_t start, loff_t end)
{
	if (fi->i_fsync_flag == F2FS_I_FSYNC_FLAG_WAITING) {
		filemap_fdatawait_range(fi->vfs_inode.i_mapping, start, end);
	} else {
		spin_lock(&file->f_lock);
		if (!file->hm_fsync_flag)
			file->hm_fsync_flag = 1;
		spin_unlock(&file->f_lock);

		spin_lock(&inode->i_lock);
		f2fs_ff_enable_start_fsync_work(fi);
		spin_unlock(&inode->i_lock);
	}
}

/*
 * 1) w/o oob info:
 *    use FI_APPEND_WRITE/APPEND_INO to indicate file's map had changed,
 *    in this scenario, should write inode/node page
 * 2) w/ oob info:
 *    write file's map into oob while flushing data, so no need to write
 *    inode/node page or set FI_APPEND_WRITE/APPEND_INO;
 *    only write inode when it has FI_DIRTY_INODE flag
 */
static int f2fs_do_sync_file(struct file *file, loff_t start, loff_t end,
						int datasync, bool atomic)
{
	struct inode *inode = file->f_mapping->host;
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	nid_t ino = inode->i_ino;
	int ret = 0;
	enum cp_reason_type cp_reason = 0;
	struct writeback_control wbc = {
		.sync_mode = WB_SYNC_ALL,
		.nr_to_write = LONG_MAX,
		.for_reclaim = 0,
	};
	u64 fsync_begin = 0, fsync_end = 0, wr_file_end, cp_begin = 0,
	cp_end = 0, sync_node_begin = 0, sync_node_end = 0,
	flush_begin = 0, flush_end = 0;
	struct request_queue *queue = sbi->sb->s_bdev->bd_disk->queue;
	bool f2fs_ff_enable = blk_queue_query_unistore_enable(queue) &&
				(fi->i_fsync_flag != F2FS_I_FSYNC_FLAG_WAITING);
	int dirty_pages;
	bool dirty_inode;

	if (unlikely(f2fs_readonly(inode->i_sb) ||
				is_sbi_flag_set(sbi, SBI_CP_DISABLED)))
		return 0;

	trace_hmfs_sync_file_enter(inode);
	pgcache_log_path(BIT_FSYNC_SYSCALL_DUMP, &(file->f_path),
			"hmfs sync file start");
	fsync_begin = local_clock();

	/* if no dirty page(dirty_pages == 0), should flush inode page */
	dirty_pages = get_dirty_pages(inode);

	if (!fi->fsync_task) {
		fi->last_atomic = fi->fsync_atomic;
		/* only for unif2fs_ioc_abort_volatile_write */
		fi->fsync_atomic = atomic;
		fi->fsync_task = current;
	} else {
		fi->fsync_task = NULL;  /* fsync from atomic write */
	}

	if (!f2fs_ff_enable || atomic)
		ret = file_write_and_wait_range(file, start, end); /* Normal Fsync */
	else
		ret = __filemap_fdatawrite_range(file->f_mapping, start, end, WB_SYNC_ALL);

	if (fi->fsync_task)
		fi->fsync_task = NULL;

	dirty_inode = (is_inode_flag_set(inode, FI_DIRTY_INODE) &&
			!IS_FI_ONLY_CHG_EXT(inode));

	wr_file_end = local_clock();

	if (ret) {
		trace_hmfs_sync_file_exit(inode, cp_reason, datasync, ret);
		return ret;
	}

	/* if the inode is dirty, let's recover all the time */
	if (!f2fs_skip_inode_update(inode, datasync)) {
		trace_hmfs_inode_update(ino, datasync);
		hmfs_write_inode(inode, NULL);
		goto go_write;
	}

	/*
	 * if there is no written data, don't waste time to write recovery info.
	 */
	if (!is_inode_flag_set(inode, FI_APPEND_WRITE) &&
		!hmfs_exist_written_data(sbi, ino, APPEND_INO)) {

		/* it may call write_inode just prior to fsync */
		if (need_inode_page_update(sbi, ino))
			goto go_write;

		trace_hmfs_sync_file_flush(ino,
				is_inode_flag_set(inode, FI_UPDATE_WRITE) ||
				hmfs_exist_written_data(sbi, ino, UPDATE_INO));

		if (is_inode_flag_set(inode, FI_UPDATE_WRITE) ||
				hmfs_exist_written_data(sbi, ino, UPDATE_INO)) {
			goto flush_out;
		}

		goto out;
	}
go_write:
	/*
	 * Both of fdatasync() and fsync() are able to be recovered from
	 * sudden-power-off.
	 */
	down_read(&fi->i_sem);
	cp_reason = need_do_checkpoint(inode);
	up_read(&fi->i_sem);

	if (cp_reason) {
		trace_hmfs_sync_file_cp(inode, cp_reason);

		/* all the dirty node pages should be flushed for POR */
		cp_begin = local_clock();
		ret = hmfs_sync_fs(inode->i_sb, 1);
		cp_end = local_clock();

		/*
		 * We've secured consistency through sync_fs. Following pino
		 * will be used only for fsynced inodes after checkpoint.
		 */
		try_to_fix_pino(inode);
		clear_inode_flag(inode, FI_APPEND_WRITE);
		clear_inode_flag(inode, FI_UPDATE_WRITE);
		goto out;
	}

	if (!need_sync_node(inode, datasync, dirty_inode, dirty_pages))
		goto out;

	sync_node_begin = local_clock();
	ret = hmfs_fsync_node_pages(sbi, inode, &wbc, atomic, f2fs_ff_enable);
	if (ret)
		goto out;

	/* once recovery info is written, don't need to tack this */
	hmfs_remove_ino_entry(sbi, ino, APPEND_INO);
	clear_inode_flag(inode, FI_APPEND_WRITE);
	sync_node_end = local_clock();
flush_out:
	if (!f2fs_ff_enable && !atomic &&
		F2FS_OPTION(sbi).fsync_mode != FSYNC_MODE_NOBARRIER) {
		flush_begin = local_clock();
		ret = hmfs_issue_flush(sbi, inode->i_ino);
		flush_end = local_clock();
	}
	if (!ret) {
		hmfs_remove_ino_entry(sbi, ino, UPDATE_INO);
		clear_inode_flag(inode, FI_UPDATE_WRITE);
		hmfs_remove_ino_entry(sbi, ino, FLUSH_INO);
	}
	f2fs_update_time(sbi, REQ_TIME);
out:
	if (f2fs_ff_enable)
		f2fs_ff_enable_out(file, fi, inode, start, end);

	trace_hmfs_sync_file_exit(inode, cp_reason, datasync, ret);
	pgcache_log_path(BIT_FSYNC_SYSCALL_DUMP, &(file->f_path),
			"hmfs sync file end");
	f2fs_trace_ios(NULL, 1);
	if (fsync_begin && !ret) {
		fsync_end = local_clock();
		bd_mutex_lock(&sbi->bd_mutex);
		if (S_ISREG(inode->i_mode))
			inc_bd_val(sbi, fsync_reg_file_cnt, 1);
		else if (S_ISDIR(inode->i_mode))
			inc_bd_val(sbi, fsync_dir_cnt, 1);
		inc_bd_val(sbi, fsync_time, fsync_end - fsync_begin);
		max_bd_val(sbi, max_fsync_time, fsync_end - fsync_begin);
		inc_bd_val(sbi, fsync_wr_file_time, wr_file_end - fsync_begin);
		max_bd_val(sbi, max_fsync_wr_file_time, wr_file_end - fsync_begin);
		inc_bd_val(sbi, fsync_cp_time, cp_end - cp_begin);
		max_bd_val(sbi, max_fsync_cp_time, cp_end - cp_begin);
		if (sync_node_end) {
			inc_bd_val(sbi, fsync_sync_node_time,
				   sync_node_end - sync_node_begin);
			max_bd_val(sbi, max_fsync_sync_node_time,
				   sync_node_end - sync_node_begin);
		}
		inc_bd_val(sbi, fsync_flush_time, flush_end - flush_begin);
		max_bd_val(sbi, max_fsync_flush_time, flush_end - flush_begin);
		bd_mutex_unlock(&sbi->bd_mutex);

		fi->fsync_ver = cur_cp_version(F2FS_CKPT(sbi));
		fi->has_wb = false;
		if (cp_reason) {
			spin_lock(&fi->stream_lock);
			fi->is_switch = false;
			fi->last_stream = STREAM_NR;
			spin_unlock(&fi->stream_lock);
		}
	}

	mas_blk_fsync_barrier(inode->i_sb->s_bdev);
	
	return ret;
}

int hmfs_sync_file(struct file *file, loff_t start, loff_t end, int datasync)
{
	int ret;
	struct inode *inode = file_inode(file);

	if (unlikely(f2fs_cp_error(F2FS_I_SB(file_inode(file)))))
		return -EIO;

	inode_lock(inode);
	ret = f2fs_do_sync_file(file, start, end, datasync, false);
	inode_unlock(inode);

	return ret;
}

static pgoff_t __get_first_dirty_index(struct address_space *mapping,
						pgoff_t pgofs, int whence)
{
	struct page *page;
	int nr_pages;

	if (whence != SEEK_DATA)
		return 0;

	/* find first dirty page index */
	nr_pages = find_get_pages_tag(mapping, &pgofs, PAGECACHE_TAG_DIRTY,
				      1, &page);
	if (!nr_pages)
		return ULONG_MAX;
	pgofs = page->index;
	put_page(page);
	return pgofs;
}

static bool __found_offset(struct f2fs_sb_info *sbi, block_t blkaddr,
				pgoff_t dirty, pgoff_t pgofs, int whence)
{
	switch (whence) {
	case SEEK_DATA:
		if ((blkaddr == NEW_ADDR && dirty == pgofs) ||
			is_valid_data_blkaddr(sbi, blkaddr))
			return true;
		break;
	case SEEK_HOLE:
		if (blkaddr == NULL_ADDR)
			return true;
		break;
	}
	return false;
}

static loff_t f2fs_seek_block(struct file *file, loff_t offset, int whence)
{
	struct inode *inode = file->f_mapping->host;
	loff_t maxbytes = inode->i_sb->s_maxbytes;
	struct dnode_of_data dn;
	pgoff_t pgofs, end_offset, dirty;
	loff_t data_ofs = offset;
	loff_t isize;
	int err = 0;

	inode_lock(inode);

	isize = i_size_read(inode);
	if (offset >= isize)
		goto fail;

	/* handle inline data case */
	if (f2fs_has_inline_data(inode) || f2fs_has_inline_dentry(inode)) {
		if (whence == SEEK_HOLE)
			data_ofs = isize;
		goto found;
	}

	pgofs = (pgoff_t)(offset >> PAGE_SHIFT);

	dirty = __get_first_dirty_index(inode->i_mapping, pgofs, whence);

	for (; data_ofs < isize; data_ofs = (loff_t)pgofs << PAGE_SHIFT) {
		set_new_dnode(&dn, inode, NULL, NULL, 0);
		err = hmfs_get_dnode_of_data(&dn, pgofs, LOOKUP_NODE);
		if (err && err != -ENOENT) {
			goto fail;
		} else if (err == -ENOENT) {
			/* direct node does not exists */
			if (whence == SEEK_DATA) {
				pgofs = hmfs_get_next_page_offset(&dn, pgofs);
				continue;
			} else {
				goto found;
			}
		}

		end_offset = ADDRS_PER_PAGE(dn.node_page, inode);

		/* find data/hole in dnode block */
		for (; dn.ofs_in_node < end_offset;
				dn.ofs_in_node++, pgofs++,
				data_ofs = (loff_t)pgofs << PAGE_SHIFT) {
			block_t blkaddr;

			blkaddr = datablock_addr(dn.inode,
					dn.node_page, dn.ofs_in_node);

			if (__is_valid_data_blkaddr(blkaddr) &&
				!hmfs_is_valid_blkaddr(F2FS_I_SB(inode),
						blkaddr, DATA_GENERIC)) {
				f2fs_put_dnode(&dn);
				goto fail;
			}

			if (__found_offset(F2FS_I_SB(inode), blkaddr, dirty,
							pgofs, whence)) {
				f2fs_put_dnode(&dn);
				goto found;
			}
		}
		f2fs_put_dnode(&dn);
	}

	if (whence == SEEK_DATA)
		goto fail;
found:
	if (whence == SEEK_HOLE && data_ofs > isize)
		data_ofs = isize;
	inode_unlock(inode);
	return vfs_setpos(file, data_ofs, maxbytes);
fail:
	inode_unlock(inode);
	return -ENXIO;
}

static loff_t f2fs_llseek(struct file *file, loff_t offset, int whence)
{
	struct inode *inode = file->f_mapping->host;
	loff_t maxbytes = inode->i_sb->s_maxbytes;

	switch (whence) {
	case SEEK_SET:
	case SEEK_CUR:
	case SEEK_END:
		return generic_file_llseek_size(file, offset, whence,
						maxbytes, i_size_read(inode));
	case SEEK_DATA:
	case SEEK_HOLE:
		if (offset < 0)
			return -ENXIO;
		return f2fs_seek_block(file, offset, whence);
	}

	return -EINVAL;
}

static int f2fs_file_mmap(struct file *file, struct vm_area_struct *vma)
{
	struct inode *inode = file_inode(file);
	int err;

	if (unlikely(f2fs_cp_error(F2FS_I_SB(inode))))
		return -EIO;

	/* we don't need to use inline_data strictly */
	err = hmfs_convert_inline_inode(inode);
	if (err)
		return err;

	file_accessed(file);
	vma->vm_ops = &f2fs_file_vm_ops;
	return 0;
}

static int f2fs_file_open(struct inode *inode, struct file *filp)
{
	int err = fscrypt_file_open(inode, filp);

	if (err)
		return err;

	filp->hm_fsync_flag = 0;

	filp->f_mode |= FMODE_NOWAIT;

	return dquot_file_open(inode, filp);
}

void hmfs_truncate_data_blocks_range(struct dnode_of_data *dn, int count)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dn->inode);
	struct f2fs_node *raw_node;
	int nr_free = 0, ofs = dn->ofs_in_node, len = count;
	__le32 *addr;
	int base = 0;

	if (IS_INODE(dn->node_page) && f2fs_has_extra_attr(dn->inode))
		base = get_extra_isize(dn->inode);

	raw_node = F2FS_NODE(dn->node_page);
	addr = blkaddr_in_node(raw_node) + base + ofs;

	for (; count > 0; count--, addr++, dn->ofs_in_node++) {
		block_t blkaddr = le32_to_cpu(*addr);

		if (blkaddr == NULL_ADDR)
			continue;

		dn->data_blkaddr = NULL_ADDR;
		hmfs_set_data_blkaddr(dn);

		if (__is_valid_data_blkaddr(blkaddr) &&
			!hmfs_is_valid_blkaddr(sbi, blkaddr, DATA_GENERIC))
			continue;

		hmfs_invalidate_blocks(sbi, blkaddr);
		if (dn->ofs_in_node == 0 && IS_INODE(dn->node_page))
			clear_inode_flag(dn->inode, FI_FIRST_BLOCK_WRITTEN);
		nr_free++;
	}

	if (nr_free) {
		pgoff_t fofs;
		/*
		 * once we invalidate valid blkaddr in range [ofs, ofs + count],
		 * we will invalidate all blkaddr in the whole range.
		 */
		fofs = hmfs_start_bidx_of_node(ofs_of_node(dn->node_page),
							dn->inode) + ofs;
		hmfs_update_extent_cache_range(dn, fofs, 0, len);
		dec_valid_block_count(sbi, dn->inode, nr_free);
	}
	dn->ofs_in_node = ofs;

	f2fs_update_time(sbi, REQ_TIME);
	trace_hmfs_truncate_data_blocks_range(dn->inode, dn->nid,
					 dn->ofs_in_node, nr_free);
}

void hmfs_truncate_data_blocks(struct dnode_of_data *dn)
{
	hmfs_truncate_data_blocks_range(dn, ADDRS_PER_BLOCK);
}

static int truncate_partial_data_page(struct inode *inode, u64 from,
								bool cache_only)
{
	loff_t offset = from & (PAGE_SIZE - 1);
	pgoff_t index = from >> PAGE_SHIFT;
	struct address_space *mapping = inode->i_mapping;
	struct page *page;

	if (!offset && !cache_only)
		return 0;

	if (cache_only) {
		page = find_lock_page(mapping, index);
		if (page && PageUptodate(page))
			goto truncate_out;
		f2fs_put_page(page, 1);
		return 0;
	}

	page = hmfs_get_lock_data_page(inode, index, true);
	if (IS_ERR(page))
		return PTR_ERR(page) == -ENOENT ? 0 : PTR_ERR(page);
truncate_out:
	hmfs_wait_on_page_writeback(page, DATA, true);
	zero_user(page, offset, PAGE_SIZE - offset);

	/* An encrypted inode should have a key and truncate the last page. */
	f2fs_bug_on(F2FS_I_SB(inode), cache_only && f2fs_encrypted_inode(inode));
	if (!cache_only)
		set_page_dirty(page);
	f2fs_put_page(page, 1);
	return 0;
}

int hmfs_truncate_blocks(struct inode *inode, u64 from, bool lock,
							bool buf_write)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct dnode_of_data dn;
	pgoff_t free_from;
	int count = 0, err = 0;
	struct page *ipage;
	bool truncate_page = false;
	int flag = buf_write ? F2FS_GET_BLOCK_PRE_AIO : F2FS_GET_BLOCK_PRE_DIO;
	bool lock2 = false;

	trace_hmfs_truncate_blocks_enter(inode, from);

	free_from = (pgoff_t)F2FS_BLK_ALIGN(from);

	if (free_from >= sbi->max_file_blocks)
		goto free_partial;

	if (lock)
		lock2 = __hmfs_do_map_lock(sbi, flag, true, false);

	ipage = hmfs_get_node_page(sbi, inode->i_ino);
	if (IS_ERR(ipage)) {
		err = PTR_ERR(ipage);
		goto out;
	}

	if (f2fs_has_inline_data(inode)) {
		hmfs_truncate_inline_inode(inode, ipage, from);
		f2fs_put_page(ipage, 1);
		truncate_page = true;
		goto out;
	}

	set_new_dnode(&dn, inode, ipage, NULL, 0);
	err = hmfs_get_dnode_of_data(&dn, free_from, LOOKUP_NODE_RA);
	if (err) {
		if (err == -ENOENT)
			goto free_next;
		goto out;
	}

	count = ADDRS_PER_PAGE(dn.node_page, inode);

	count -= dn.ofs_in_node;
	f2fs_bug_on(sbi, count < 0);

	if (dn.ofs_in_node || IS_INODE(dn.node_page)) {
		hmfs_truncate_data_blocks_range(&dn, count);
		free_from += count;
	}

	f2fs_put_dnode(&dn);
free_next:
	err = hmfs_truncate_inode_blocks(inode, free_from);
out:
	if (lock)
		(void)__hmfs_do_map_lock(sbi, flag, false, lock2);
free_partial:
	/* lastly zero out the first data page */
	if (!err)
		err = truncate_partial_data_page(inode, from, truncate_page);

	trace_hmfs_truncate_blocks_exit(inode, err);
	return err;
}

int hmfs_truncate(struct inode *inode)
{
	int err;

	if (unlikely(f2fs_cp_error(F2FS_I_SB(inode))))
		return -EIO;

	if (!(S_ISREG(inode->i_mode) || S_ISDIR(inode->i_mode) ||
				S_ISLNK(inode->i_mode)))
		return 0;

	trace_hmfs_truncate(inode);

	if (time_to_inject(F2FS_I_SB(inode), FAULT_TRUNCATE)) {
		f2fs_show_injection_info(FAULT_TRUNCATE);
		return -EIO;
	}

	/* we should check inline_data size */
	if (!hmfs_may_inline_data(inode)) {
		err = hmfs_convert_inline_inode(inode);
		if (err)
			return err;
	}

	err = hmfs_truncate_blocks(inode, i_size_read(inode), true, false);
	if (err)
		return err;

	inode->i_mtime = inode->i_ctime = current_time(inode);
	hmfs_mark_inode_dirty_sync(inode, false);
	return 0;
}

int hmfs_getattr(const struct path *path, struct kstat *stat,
		 u32 request_mask, unsigned int query_flags)
{
	struct inode *inode = d_inode(path->dentry);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_inode *ri;
	unsigned int flags;

	if (f2fs_has_extra_attr(inode) &&
			f2fs_sb_has_inode_crtime(inode->i_sb) &&
			F2FS_FITS_IN_INODE(ri, fi->i_extra_isize, i_crtime)) {
		stat->result_mask |= STATX_BTIME;
		stat->btime.tv_sec = fi->i_crtime.tv_sec;
		stat->btime.tv_nsec = fi->i_crtime.tv_nsec;
	}

	flags = fi->i_flags & F2FS_FL_USER_VISIBLE;
	if (flags & F2FS_APPEND_FL)
		stat->attributes |= STATX_ATTR_APPEND;
	if (flags & F2FS_COMPR_FL)
		stat->attributes |= STATX_ATTR_COMPRESSED;
	if (f2fs_encrypted_inode(inode))
		stat->attributes |= STATX_ATTR_ENCRYPTED;
	if (flags & F2FS_IMMUTABLE_FL)
		stat->attributes |= STATX_ATTR_IMMUTABLE;
	if (flags & F2FS_NODUMP_FL)
		stat->attributes |= STATX_ATTR_NODUMP;

	stat->attributes_mask |= (STATX_ATTR_APPEND |
				  STATX_ATTR_COMPRESSED |
				  STATX_ATTR_ENCRYPTED |
				  STATX_ATTR_IMMUTABLE |
				  STATX_ATTR_NODUMP);

	generic_fillattr(inode, stat);

	/* we need to show initial sectors used for inline_data/dentries */
	if ((S_ISREG(inode->i_mode) && f2fs_has_inline_data(inode)) ||
					f2fs_has_inline_dentry(inode))
		stat->blocks += (stat->size + 511) >> 9;

	return 0;
}

#ifdef CONFIG_HMFS_FS_POSIX_ACL
static void __setattr_copy(struct inode *inode, const struct iattr *attr)
{
	unsigned int ia_valid = attr->ia_valid;

	if (ia_valid & ATTR_UID)
		inode->i_uid = attr->ia_uid;
	if (ia_valid & ATTR_GID)
		inode->i_gid = attr->ia_gid;
	if (ia_valid & ATTR_ATIME)
		inode->i_atime = timespec_trunc(attr->ia_atime,
						inode->i_sb->s_time_gran);
	if (ia_valid & ATTR_MTIME)
		inode->i_mtime = timespec_trunc(attr->ia_mtime,
						inode->i_sb->s_time_gran);
	if (ia_valid & ATTR_CTIME)
		inode->i_ctime = timespec_trunc(attr->ia_ctime,
						inode->i_sb->s_time_gran);
	if (ia_valid & ATTR_MODE) {
		umode_t mode = attr->ia_mode;

		if (!in_group_p(inode->i_gid) && !capable(CAP_FSETID))
			mode &= ~S_ISGID;
		set_acl_inode(inode, mode);
	}
}
#else
#define __setattr_copy setattr_copy
#endif

int hmfs_setattr(struct dentry *dentry, struct iattr *attr)
{
	struct inode *inode = d_inode(dentry);
	int err;

	if (unlikely(f2fs_cp_error(F2FS_I_SB(inode))))
		return -EIO;

	err = setattr_prepare(dentry, attr);
	if (err)
		return err;

	err = fscrypt_prepare_setattr(dentry, attr);
	if (err)
		return err;

	if (is_quota_modification(inode, attr)) {
		err = f2fs_dquot_initialize(inode);
		if (err)
			return err;
	}
	if ((attr->ia_valid & ATTR_UID &&
		!uid_eq(attr->ia_uid, inode->i_uid)) ||
		(attr->ia_valid & ATTR_GID &&
		!gid_eq(attr->ia_gid, inode->i_gid))) {
		f2fs_lock_op(F2FS_I_SB(inode));
		err = f2fs_dquot_transfer(inode, attr);
		if (err) {
			set_sbi_flag(F2FS_I_SB(inode),
					SBI_QUOTA_NEED_REPAIR);
			f2fs_unlock_op(F2FS_I_SB(inode));
			return err;
		}
		/*
		 * update uid/gid under lock_op(), so that dquot and inode can
		 * be updated atomically.
		 */
		if (attr->ia_valid & ATTR_UID) {
#ifdef CONFIG_HWDPS
			hwdps_update_context(inode, attr->ia_uid.val);
#endif
			inode->i_uid = attr->ia_uid;
		}
		if (attr->ia_valid & ATTR_GID)
			inode->i_gid = attr->ia_gid;
		hmfs_mark_inode_dirty_sync(inode, true);
		f2fs_unlock_op(F2FS_I_SB(inode));
	}

	if (attr->ia_valid & ATTR_SIZE) {
		bool to_smaller = (attr->ia_size <= i_size_read(inode));

		down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
		down_write(&F2FS_I(inode)->i_mmap_sem);

		truncate_setsize(inode, attr->ia_size);

		if (to_smaller)
			err = hmfs_truncate(inode);
		/*
		 * do not trim all blocks after i_size if target size is
		 * larger than i_size.
		 */
		up_write(&F2FS_I(inode)->i_mmap_sem);
		up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);

		if (err)
			return err;

		if (!to_smaller) {
			/* should convert inline inode here */
			if (!hmfs_may_inline_data(inode)) {
				err = hmfs_convert_inline_inode(inode);
				if (err)
					return err;
			}
			inode->i_mtime = inode->i_ctime = current_time(inode);
		}

		down_write(&F2FS_I(inode)->i_sem);
		F2FS_I(inode)->last_disk_size = i_size_read(inode);
		up_write(&F2FS_I(inode)->i_sem);
	}

	__setattr_copy(inode, attr);

	if (attr->ia_valid & ATTR_MODE) {
		err = posix_acl_chmod(inode, f2fs_get_inode_mode(inode));
		if (err || is_inode_flag_set(inode, FI_ACL_MODE)) {
			inode->i_mode = F2FS_I(inode)->i_acl_mode;
			clear_inode_flag(inode, FI_ACL_MODE);
		}
	}

	/* file size may changed here */
	hmfs_mark_inode_dirty_sync(inode, true);

	/* inode change will produce dirty node pages flushed by checkpoint */
	hmfs_balance_fs(F2FS_I_SB(inode), true);

	return err;
}

const struct inode_operations hmfs_file_inode_operations = {
	.getattr	= hmfs_getattr,
	.setattr	= hmfs_setattr,
	.get_acl	= hmfs_get_acl,
	.set_acl	= hmfs_set_acl,
#ifdef CONFIG_HMFS_FS_XATTR
	.listxattr	= hmfs_listxattr,
#endif
	.fiemap		= hmfs_fiemap,
};

static int fill_zero(struct inode *inode, pgoff_t index,
					loff_t start, loff_t len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct page *page;

	if (!len)
		return 0;

	hmfs_balance_fs(sbi, true);

	f2fs_lock_op(sbi);
	page = hmfs_get_new_data_page(inode, NULL, index, false);
	f2fs_unlock_op(sbi);

	if (IS_ERR(page))
		return PTR_ERR(page);

	hmfs_wait_on_page_writeback(page, DATA, true);
	zero_user(page, start, len);
	set_page_dirty(page);
	f2fs_put_page(page, 1);
	return 0;
}

int hmfs_truncate_hole(struct inode *inode, pgoff_t pg_start, pgoff_t pg_end)
{
	int err;

	while (pg_start < pg_end) {
		struct dnode_of_data dn;
		pgoff_t end_offset, count;

		set_new_dnode(&dn, inode, NULL, NULL, 0);
		err = hmfs_get_dnode_of_data(&dn, pg_start, LOOKUP_NODE);
		if (err) {
			if (err == -ENOENT) {
				pg_start = hmfs_get_next_page_offset(&dn,
								pg_start);
				continue;
			}
			return err;
		}

		end_offset = ADDRS_PER_PAGE(dn.node_page, inode);
		count = min(end_offset - dn.ofs_in_node, pg_end - pg_start);

		f2fs_bug_on(F2FS_I_SB(inode), count == 0 || count > end_offset);

		hmfs_truncate_data_blocks_range(&dn, count);
		f2fs_put_dnode(&dn);

		pg_start += count;
	}
	return 0;
}

static int punch_hole(struct inode *inode, loff_t offset, loff_t len)
{
	pgoff_t pg_start, pg_end;
	loff_t off_start, off_end;
	int ret;

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		return ret;

	pg_start = ((unsigned long long) offset) >> PAGE_SHIFT;
	pg_end = ((unsigned long long) offset + len) >> PAGE_SHIFT;

	off_start = offset & (PAGE_SIZE - 1);
	off_end = (offset + len) & (PAGE_SIZE - 1);

	if (pg_start == pg_end) {
		ret = fill_zero(inode, pg_start, off_start,
						off_end - off_start);
		if (ret)
			return ret;
	} else {
		if (off_start) {
			ret = fill_zero(inode, pg_start++, off_start,
						PAGE_SIZE - off_start);
			if (ret)
				return ret;
		}
		if (off_end) {
			ret = fill_zero(inode, pg_end, 0, off_end);
			if (ret)
				return ret;
		}

		if (pg_start < pg_end) {
			struct address_space *mapping = inode->i_mapping;
			loff_t blk_start, blk_end;
			struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

			hmfs_balance_fs(sbi, true);

			blk_start = (loff_t)pg_start << PAGE_SHIFT;
			blk_end = (loff_t)pg_end << PAGE_SHIFT;

			down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
			down_write(&F2FS_I(inode)->i_mmap_sem);

			truncate_inode_pages_range(mapping, blk_start,
					blk_end - 1);

			f2fs_lock_op(sbi);
			ret = hmfs_truncate_hole(inode, pg_start, pg_end);
			f2fs_unlock_op(sbi);

			up_write(&F2FS_I(inode)->i_mmap_sem);
			up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
		}
	}

	return ret;
}

static int __read_out_blkaddrs(struct inode *inode, block_t *blkaddr,
				int *do_replace, pgoff_t off, pgoff_t len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct dnode_of_data dn;
	int ret, done, i;

next_dnode:
	set_new_dnode(&dn, inode, NULL, NULL, 0);
	ret = hmfs_get_dnode_of_data(&dn, off, LOOKUP_NODE_RA);
	if (ret && ret != -ENOENT) {
		return ret;
	} else if (ret == -ENOENT) {
		if (dn.max_level == 0)
			return -ENOENT;
		done = min((pgoff_t)ADDRS_PER_BLOCK - dn.ofs_in_node, len);
		blkaddr += done;
		do_replace += done;
		goto next;
	}

	done = min((pgoff_t)ADDRS_PER_PAGE(dn.node_page, inode) -
							dn.ofs_in_node, len);
	for (i = 0; i < done; i++, blkaddr++, do_replace++, dn.ofs_in_node++) {
		*blkaddr = datablock_addr(dn.inode,
					dn.node_page, dn.ofs_in_node);
		if (!hmfs_is_checkpointed_data(sbi, *blkaddr)) {

			if (test_opt(sbi, LFS)) {
				f2fs_put_dnode(&dn);
				return -ENOTSUPP;
			}

			/* do not invalidate this block address */
			hmfs_update_data_blkaddr(&dn, NULL_ADDR);
			*do_replace = 1;
		}
	}
	f2fs_put_dnode(&dn);
next:
	len -= done;
	off += done;
	if (len)
		goto next_dnode;
	return 0;
}

static int __roll_back_blkaddrs(struct inode *inode, block_t *blkaddr,
				int *do_replace, pgoff_t off, int len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct dnode_of_data dn;
	int ret, i;

	for (i = 0; i < len; i++, do_replace++, blkaddr++) {
		if (*do_replace == 0)
			continue;

		set_new_dnode(&dn, inode, NULL, NULL, 0);
		ret = hmfs_get_dnode_of_data(&dn, off + i, LOOKUP_NODE_RA);
		if (ret) {
			dec_valid_block_count(sbi, inode, 1);
			hmfs_invalidate_blocks(sbi, *blkaddr);
		} else {
			hmfs_update_data_blkaddr(&dn, *blkaddr);
		}
		f2fs_put_dnode(&dn);
	}
	return 0;
}

static int __clone_blkaddrs(struct inode *src_inode, struct inode *dst_inode,
			block_t *blkaddr, int *do_replace,
			pgoff_t src, pgoff_t dst, pgoff_t len, bool full)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(src_inode);
	pgoff_t i = 0;
	int ret;

	while (i < len) {
		if (blkaddr[i] == NULL_ADDR && !full) {
			i++;
			continue;
		}

		if (do_replace[i] || blkaddr[i] == NULL_ADDR) {
			struct dnode_of_data dn;
			struct node_info ni;
			size_t new_size;
			pgoff_t ilen;

			set_new_dnode(&dn, dst_inode, NULL, NULL, 0);
			ret = hmfs_get_dnode_of_data(&dn, dst + i, ALLOC_NODE);
			if (ret)
				return ret;

			ret = f2fs_get_node_info(sbi, dn.nid, &ni);
			if (ret) {
				f2fs_put_dnode(&dn);
				return ret;
			}

			ilen = min((pgoff_t)
				ADDRS_PER_PAGE(dn.node_page, dst_inode) -
						dn.ofs_in_node, len - i);
			do {
				dn.data_blkaddr = datablock_addr(dn.inode,
						dn.node_page, dn.ofs_in_node);
				hmfs_truncate_data_blocks_range(&dn, 1);

				if (do_replace[i]) {
					f2fs_i_blocks_write(src_inode,
							1, false, false);
					f2fs_i_blocks_write(dst_inode,
							1, true, false);
					hmfs_replace_block(sbi, &dn, dn.data_blkaddr,
					blkaddr[i], ni.version, true, false, false);

					do_replace[i] = 0;
				}
				dn.ofs_in_node++;
				i++;
				new_size = (dst + i) << PAGE_SHIFT;
				if (dst_inode->i_size < new_size)
					f2fs_i_size_write(dst_inode, new_size);
			} while (--ilen && (do_replace[i] || blkaddr[i] == NULL_ADDR));

			f2fs_put_dnode(&dn);
		} else {
			struct page *psrc, *pdst;

			psrc = hmfs_get_lock_data_page(src_inode,
							src + i, true);
			if (IS_ERR(psrc))
				return PTR_ERR(psrc);
			pdst = hmfs_get_new_data_page(dst_inode, NULL, dst + i,
								true);
			if (IS_ERR(pdst)) {
				f2fs_put_page(psrc, 1);
				return PTR_ERR(pdst);
			}
			f2fs_copy_page(psrc, pdst);
			set_page_dirty(pdst);
			f2fs_put_page(pdst, 1);
			f2fs_put_page(psrc, 1);

			ret = hmfs_truncate_hole(src_inode,
						src + i, src + i + 1);
			if (ret)
				return ret;
			i++;
		}
	}
	return 0;
}

static int __exchange_data_block(struct inode *src_inode,
			struct inode *dst_inode, pgoff_t src, pgoff_t dst,
			pgoff_t len, bool full)
{
	block_t *src_blkaddr;
	int *do_replace;
	pgoff_t olen;
	int ret;

	while (len) {
		olen = min((pgoff_t)4 * ADDRS_PER_BLOCK, len);

		src_blkaddr = f2fs_kvzalloc(F2FS_I_SB(src_inode),
					array_size(olen, sizeof(block_t)),
					GFP_KERNEL);
		if (!src_blkaddr)
			return -ENOMEM;

		do_replace = f2fs_kvzalloc(F2FS_I_SB(src_inode),
					array_size(olen, sizeof(int)),
					GFP_KERNEL);
		if (!do_replace) {
			kvfree(src_blkaddr);
			return -ENOMEM;
		}

		ret = __read_out_blkaddrs(src_inode, src_blkaddr,
					do_replace, src, olen);
		if (ret)
			goto roll_back;

		ret = __clone_blkaddrs(src_inode, dst_inode, src_blkaddr,
					do_replace, src, dst, olen, full);
		if (ret)
			goto roll_back;

		src += olen;
		dst += olen;
		len -= olen;

		kvfree(src_blkaddr);
		kvfree(do_replace);
	}
	return 0;

roll_back:
	__roll_back_blkaddrs(src_inode, src_blkaddr, do_replace, src, olen);
	kvfree(src_blkaddr);
	kvfree(do_replace);
	return ret;
}

static int f2fs_do_collapse(struct inode *inode, loff_t offset, loff_t len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	pgoff_t nrpages = (i_size_read(inode) + PAGE_SIZE - 1) / PAGE_SIZE;
	pgoff_t start = offset >> PAGE_SHIFT;
	pgoff_t end = (offset + len) >> PAGE_SHIFT;
	int ret;

	hmfs_balance_fs(sbi, true);

	/* avoid gc operation during block exchange */
	down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
	down_write(&F2FS_I(inode)->i_mmap_sem);

	f2fs_lock_op(sbi);
	hmfs_drop_extent_tree(inode);
	truncate_pagecache(inode, offset);
	ret = __exchange_data_block(inode, inode, end, start, nrpages - end, true);
	f2fs_unlock_op(sbi);

	up_write(&F2FS_I(inode)->i_mmap_sem);
	up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
	return ret;
}

static int f2fs_collapse_range(struct inode *inode, loff_t offset, loff_t len)
{
	loff_t new_size;
	int ret;

	if (offset + len >= i_size_read(inode))
		return -EINVAL;

	/* collapse range should be aligned to block size of f2fs. */
	if (offset & (F2FS_BLKSIZE - 1) || len & (F2FS_BLKSIZE - 1))
		return -EINVAL;

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		return ret;

	/* write out all dirty pages from offset */
	ret = filemap_write_and_wait_range(inode->i_mapping, offset, LLONG_MAX);
	if (ret)
		return ret;

	ret = f2fs_do_collapse(inode, offset, len);
	if (ret)
		return ret;

	/* write out all moved pages, if possible */
	down_write(&F2FS_I(inode)->i_mmap_sem);
	filemap_write_and_wait_range(inode->i_mapping, offset, LLONG_MAX);
	truncate_pagecache(inode, offset);

	new_size = i_size_read(inode) - len;
	truncate_pagecache(inode, new_size);

	ret = hmfs_truncate_blocks(inode, new_size, true, false);
	up_write(&F2FS_I(inode)->i_mmap_sem);
	if (!ret)
		f2fs_i_size_write(inode, new_size);
	return ret;
}

static int f2fs_do_zero_range(struct dnode_of_data *dn, pgoff_t start,
								pgoff_t end)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(dn->inode);
	pgoff_t index = start;
	unsigned int ofs_in_node = dn->ofs_in_node;
	blkcnt_t count = 0;
	int ret;

	for (; index < end; index++, dn->ofs_in_node++) {
		if (datablock_addr(dn->inode, dn->node_page,
					dn->ofs_in_node) == NULL_ADDR)
			count++;
	}

	dn->ofs_in_node = ofs_in_node;
	ret = hmfs_reserve_new_blocks(dn, count);
	if (ret)
		return ret;

	dn->ofs_in_node = ofs_in_node;
	for (index = start; index < end; index++, dn->ofs_in_node++) {
		dn->data_blkaddr = datablock_addr(dn->inode,
					dn->node_page, dn->ofs_in_node);
		/*
		 * hmfs_reserve_new_blocks will not guarantee entire block
		 * allocation.
		 */
		if (dn->data_blkaddr == NULL_ADDR) {
			ret = -ENOSPC;
			break;
		}
		if (dn->data_blkaddr != NEW_ADDR) {
			hmfs_invalidate_blocks(sbi, dn->data_blkaddr);
			dn->data_blkaddr = NEW_ADDR;
			hmfs_set_data_blkaddr(dn);
		}
	}

	hmfs_update_extent_cache_range(dn, start, 0, index - start);

	return ret;
}

static int f2fs_zero_range(struct inode *inode, loff_t offset, loff_t len,
								int mode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct address_space *mapping = inode->i_mapping;
	pgoff_t index, pg_start, pg_end;
	loff_t new_size = i_size_read(inode);
	loff_t off_start, off_end;
	int ret = 0;

	ret = inode_newsize_ok(inode, (len + offset));
	if (ret)
		return ret;

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		return ret;

	ret = filemap_write_and_wait_range(mapping, offset, offset + len - 1);
	if (ret)
		return ret;

	pg_start = ((unsigned long long) offset) >> PAGE_SHIFT;
	pg_end = ((unsigned long long) offset + len) >> PAGE_SHIFT;

	off_start = offset & (PAGE_SIZE - 1);
	off_end = (offset + len) & (PAGE_SIZE - 1);

	if (pg_start == pg_end) {
		ret = fill_zero(inode, pg_start, off_start,
						off_end - off_start);
		if (ret)
			return ret;

		new_size = max_t(loff_t, new_size, offset + len);
	} else {
		if (off_start) {
			ret = fill_zero(inode, pg_start++, off_start,
						PAGE_SIZE - off_start);
			if (ret)
				return ret;

			new_size = max_t(loff_t, new_size,
					(loff_t)pg_start << PAGE_SHIFT);
		}

		for (index = pg_start; index < pg_end;) {
			struct dnode_of_data dn;
			unsigned int end_offset;
			pgoff_t end;

			down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
			down_write(&F2FS_I(inode)->i_mmap_sem);

			truncate_pagecache_range(inode,
				(loff_t)index << PAGE_SHIFT,
				((loff_t)pg_end << PAGE_SHIFT) - 1);

			f2fs_lock_op(sbi);

			set_new_dnode(&dn, inode, NULL, NULL, 0);
			ret = hmfs_get_dnode_of_data(&dn, index, ALLOC_NODE);
			if (ret) {
				f2fs_unlock_op(sbi);
				up_write(&F2FS_I(inode)->i_mmap_sem);
				up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
				goto out;
			}

			end_offset = ADDRS_PER_PAGE(dn.node_page, inode);
			end = min(pg_end, end_offset - dn.ofs_in_node + index);

			ret = f2fs_do_zero_range(&dn, index, end);
			f2fs_put_dnode(&dn);

			f2fs_unlock_op(sbi);
			up_write(&F2FS_I(inode)->i_mmap_sem);
			up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);

			hmfs_balance_fs(sbi, dn.node_changed);

			if (ret)
				goto out;

			index = end;
			new_size = max_t(loff_t, new_size,
					(loff_t)index << PAGE_SHIFT);
		}

		if (off_end) {
			ret = fill_zero(inode, pg_end, 0, off_end);
			if (ret)
				goto out;

			new_size = max_t(loff_t, new_size, offset + len);
		}
	}

out:
	if (new_size > i_size_read(inode)) {
		if (mode & FALLOC_FL_KEEP_SIZE)
			file_set_keep_isize(inode);
		else
			f2fs_i_size_write(inode, new_size);
	}
	return ret;
}

static int f2fs_insert_range(struct inode *inode, loff_t offset, loff_t len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	pgoff_t nr, pg_start, pg_end, delta, idx;
	loff_t new_size;
	int ret = 0;

	new_size = i_size_read(inode) + len;
	ret = inode_newsize_ok(inode, new_size);
	if (ret)
		return ret;

	if (offset >= i_size_read(inode))
		return -EINVAL;

	/* insert range should be aligned to block size of f2fs. */
	if (offset & (F2FS_BLKSIZE - 1) || len & (F2FS_BLKSIZE - 1))
		return -EINVAL;

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		return ret;

	hmfs_balance_fs(sbi, true);

	down_write(&F2FS_I(inode)->i_mmap_sem);
	ret = hmfs_truncate_blocks(inode, i_size_read(inode), true, false);
	up_write(&F2FS_I(inode)->i_mmap_sem);
	if (ret)
		return ret;

	/* write out all dirty pages from offset */
	ret = filemap_write_and_wait_range(inode->i_mapping, offset, LLONG_MAX);
	if (ret)
		return ret;

	pg_start = offset >> PAGE_SHIFT;
	pg_end = (offset + len) >> PAGE_SHIFT;
	delta = pg_end - pg_start;
	idx = (i_size_read(inode) + PAGE_SIZE - 1) / PAGE_SIZE;

	/* avoid gc operation during block exchange */
	down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
	down_write(&F2FS_I(inode)->i_mmap_sem);
	truncate_pagecache(inode, offset);

	while (!ret && idx > pg_start) {
		nr = idx - pg_start;
		if (nr > delta)
			nr = delta;
		idx -= nr;

		f2fs_lock_op(sbi);
		hmfs_drop_extent_tree(inode);

		ret = __exchange_data_block(inode, inode, idx,
					idx + delta, nr, false);
		f2fs_unlock_op(sbi);
	}
	up_write(&F2FS_I(inode)->i_mmap_sem);
	up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);

	/* write out all moved pages, if possible */
	down_write(&F2FS_I(inode)->i_mmap_sem);
	filemap_write_and_wait_range(inode->i_mapping, offset, LLONG_MAX);
	truncate_pagecache(inode, offset);
	up_write(&F2FS_I(inode)->i_mmap_sem);

	if (!ret)
		f2fs_i_size_write(inode, new_size);
	return ret;
}

static int expand_inode_data(struct inode *inode, loff_t offset,
					loff_t len, int mode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_map_blocks map = { .m_next_pgofs = NULL,
			.m_next_extent = NULL, .m_seg_type = NO_CHECK_TYPE };
	pgoff_t pg_end;
	loff_t new_size = i_size_read(inode);
	loff_t off_end;
	int err;

	err = inode_newsize_ok(inode, (len + offset));
	if (err)
		return err;

	err = hmfs_convert_inline_inode(inode);
	if (err)
		return err;

	hmfs_balance_fs(sbi, true);

	pg_end = ((unsigned long long)offset + len) >> PAGE_SHIFT;
	off_end = (offset + len) & (PAGE_SIZE - 1);

	map.m_lblk = ((unsigned long long)offset) >> PAGE_SHIFT;
	map.m_len = pg_end - map.m_lblk;
	if (off_end)
		map.m_len++;

	err = hmfs_map_blocks(inode, &map, 1, F2FS_GET_BLOCK_PRE_AIO);
	if (err) {
		pgoff_t last_off;

		if (!map.m_len)
			return err;

		last_off = map.m_lblk + map.m_len - 1;

		/* update new size to the failed position */
		new_size = (last_off == pg_end) ? offset + len :
					(loff_t)(last_off + 1) << PAGE_SHIFT;
	} else {
		new_size = ((loff_t)pg_end << PAGE_SHIFT) + off_end;
	}

	if (new_size > i_size_read(inode)) {
		if (mode & FALLOC_FL_KEEP_SIZE)
			file_set_keep_isize(inode);
		else
			f2fs_i_size_write(inode, new_size);
	}

	return err;
}

static long f2fs_fallocate(struct file *file, int mode,
				loff_t offset, loff_t len)
{
	struct inode *inode = file_inode(file);
	long ret = 0;

	if (unlikely(f2fs_cp_error(F2FS_I_SB(inode))))
		return -EIO;

	/* f2fs only support ->fallocate for regular file */
	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	if (f2fs_encrypted_inode(inode) &&
		(mode & (FALLOC_FL_COLLAPSE_RANGE | FALLOC_FL_INSERT_RANGE)))
		return -EOPNOTSUPP;

	if (mode & ~(FALLOC_FL_KEEP_SIZE | FALLOC_FL_PUNCH_HOLE |
			FALLOC_FL_COLLAPSE_RANGE | FALLOC_FL_ZERO_RANGE |
			FALLOC_FL_INSERT_RANGE))
		return -EOPNOTSUPP;

	inode_lock(inode);

	if (mode & FALLOC_FL_PUNCH_HOLE) {
		if (offset >= inode->i_size)
			goto out;

		ret = punch_hole(inode, offset, len);
	} else if (mode & FALLOC_FL_COLLAPSE_RANGE) {
		ret = f2fs_collapse_range(inode, offset, len);
	} else if (mode & FALLOC_FL_ZERO_RANGE) {
		ret = f2fs_zero_range(inode, offset, len, mode);
	} else if (mode & FALLOC_FL_INSERT_RANGE) {
		ret = f2fs_insert_range(inode, offset, len);
	} else {
		ret = expand_inode_data(inode, offset, len, mode);
	}

	if (!ret) {
		inode->i_mtime = inode->i_ctime = current_time(inode);
		hmfs_mark_inode_dirty_sync(inode, false);
		f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	}

out:
	inode_unlock(inode);

	trace_hmfs_fallocate(inode, mode, offset, len, ret);
	return ret;
}

static int f2fs_release_file(struct inode *inode, struct file *filp)
{
	/*
	 * f2fs_relase_file is called at every close calls. So we should
	 * not drop any inmemory pages by close called by other process.
	 */
	if (!(filp->f_mode & FMODE_WRITE) ||
			atomic_read(&inode->i_writecount) != 1)
		return 0;

	/* some remained atomic pages should discarded */
	if (f2fs_is_atomic_file(inode))
		hmfs_drop_inmem_pages(inode);
	if (f2fs_is_volatile_file(inode)) {
		set_inode_flag(inode, FI_DROP_CACHE);
		filemap_fdatawrite(inode->i_mapping);
		clear_inode_flag(inode, FI_DROP_CACHE);
		clear_inode_flag(inode, FI_VOLATILE_FILE);
		stat_dec_volatile_write(inode);
	}
	return 0;
}

static int f2fs_file_flush(struct file *file, fl_owner_t id)
{
	struct inode *inode = file_inode(file);

	hmfs_flush_wait_fsync(file);

	/*
	 * If the process doing a transaction is crashed, we should do
	 * roll-back. Otherwise, other reader/write can see corrupted database
	 * until all the writers close its file. Since this should be done
	 * before dropping file lock, it needs to do in ->flush.
	 */
	if (f2fs_is_atomic_file(inode) &&
			F2FS_I(inode)->inmem_task == current)
		hmfs_drop_inmem_pages(inode);
	return 0;
}

static int f2fs_ioc_getflags(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	unsigned int flags = fi->i_flags;

	if (f2fs_encrypted_inode(inode))
		flags |= F2FS_ENCRYPT_FL;
	if (f2fs_has_inline_data(inode) || f2fs_has_inline_dentry(inode))
		flags |= F2FS_INLINE_DATA_FL;
	if (is_inode_flag_set(inode, FI_PIN_FILE))
		flags |= F2FS_NOCOW_FL;

	flags &= F2FS_FL_USER_VISIBLE;

	return put_user(flags, (int __user *)arg);
}

static int __f2fs_ioc_setflags(struct inode *inode, unsigned int flags)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	unsigned int oldflags;

	/* Is it quota file? Do not allow user to mess with it */
	if (IS_NOQUOTA(inode))
		return -EPERM;

	flags = f2fs_mask_flags(inode->i_mode, flags);

	oldflags = fi->i_flags;

	if ((flags ^ oldflags) & (F2FS_APPEND_FL | F2FS_IMMUTABLE_FL))
		if (!capable(CAP_LINUX_IMMUTABLE))
			return -EPERM;

	flags = flags & F2FS_FL_USER_MODIFIABLE;
	flags |= oldflags & ~F2FS_FL_USER_MODIFIABLE;
	fi->i_flags = flags;

	if (fi->i_flags & F2FS_PROJINHERIT_FL)
		set_inode_flag(inode, FI_PROJ_INHERIT);
	else
		clear_inode_flag(inode, FI_PROJ_INHERIT);

	inode->i_ctime = current_time(inode);
	hmfs_set_inode_flags(inode);
	hmfs_mark_inode_dirty_sync(inode, true);
	return 0;
}

static int f2fs_ioc_setflags(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	unsigned int flags;
	int ret;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (get_user(flags, (int __user *)arg))
		return -EFAULT;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	ret = __f2fs_ioc_setflags(inode, flags);

	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_getversion(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);

	return put_user(inode->i_generation, (int __user *)arg);
}

static int f2fs_ioc_start_atomic_write(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int ret;

	pgcache_log_path(BIT_FSYNC_SYSCALL_DUMP, &(filp->f_path),
			"hmfs atomic write start");
	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	if (f2fs_is_atomic_file(inode)) {
		if (is_inode_flag_set(inode, FI_ATOMIC_REVOKE_REQUEST))
			ret = -EINVAL;
		goto out;
	}

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		goto out;

	down_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
	/*
	 * Should wait end_io to count F2FS_WB_CP_DATA correctly by
	 * f2fs_is_atomic_file.
	 */
	if (get_dirty_pages(inode))
		hmfs_msg(F2FS_I_SB(inode)->sb, KERN_WARNING,
		"Unexpected flush for atomic writes: ino=%lu, npages=%u",
					inode->i_ino, get_dirty_pages(inode));
	ret = filemap_write_and_wait_range(inode->i_mapping, 0, LLONG_MAX);
	if (ret) {
		up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);
		goto out;
	}

	spin_lock(&sbi->inode_lock[ATOMIC_FILE]);
	if (list_empty(&fi->inmem_ilist))
		list_add_tail(&fi->inmem_ilist, &sbi->inode_list[ATOMIC_FILE]);
	sbi->atomic_files++;
	spin_unlock(&sbi->inode_lock[ATOMIC_FILE]);

	/* add inode in inmem_list first and set atomic_file */
	set_inode_flag(inode, FI_ATOMIC_FILE);
	clear_inode_flag(inode, FI_ATOMIC_REVOKE_REQUEST);
	up_write(&F2FS_I(inode)->i_gc_rwsem[WRITE]);

	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	F2FS_I(inode)->inmem_task = current;
	stat_inc_atomic_write(inode);
	stat_update_max_atomic_write(inode);
out:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_commit_atomic_write(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	int ret;

	pgcache_log_path(BIT_FSYNC_SYSCALL_DUMP, &(filp->f_path),
			"hmfs atomic write commit");
	if (!inode_owner_or_capable(inode))
		return -EACCES;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	hmfs_balance_fs(F2FS_I_SB(inode), true);

	inode_lock(inode);

	if (f2fs_is_volatile_file(inode)) {
		ret = -EINVAL;
		goto err_out;
	}

	if (f2fs_is_atomic_file(inode)) {
		ret = hmfs_commit_inmem_pages(inode);
		if (ret)
			goto err_out;

		ret = f2fs_do_sync_file(filp, 0, LLONG_MAX, 0, true);
		if (!ret)
			hmfs_drop_inmem_pages(inode);
	} else {
		ret = f2fs_do_sync_file(filp, 0, LLONG_MAX, 1, false);
	}
err_out:
	if (is_inode_flag_set(inode, FI_ATOMIC_REVOKE_REQUEST)) {
		clear_inode_flag(inode, FI_ATOMIC_REVOKE_REQUEST);
		ret = -EINVAL;
	}
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_start_volatile_write(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	int ret;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	if (f2fs_is_volatile_file(inode))
		goto out;

	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		goto out;

	stat_inc_volatile_write(inode);
	stat_update_max_volatile_write(inode);

	set_inode_flag(inode, FI_VOLATILE_FILE);
	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
out:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_release_volatile_write(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	int ret;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	if (!f2fs_is_volatile_file(inode))
		goto out;

	if (!f2fs_is_first_block_written(inode)) {
		ret = truncate_partial_data_page(inode, 0, true);
		goto out;
	}

	ret = punch_hole(inode, 0, F2FS_BLKSIZE);
out:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_abort_volatile_write(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	int ret;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	if (f2fs_is_atomic_file(inode))
		hmfs_drop_inmem_pages(inode);
	if (f2fs_is_volatile_file(inode)) {
		clear_inode_flag(inode, FI_VOLATILE_FILE);
		stat_dec_volatile_write(inode);
		ret = f2fs_do_sync_file(filp, 0, LLONG_MAX, 0, true);
	}

	clear_inode_flag(inode, FI_ATOMIC_REVOKE_REQUEST);

	inode_unlock(inode);

	mnt_drop_write_file(filp);
	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	return ret;
}

static int f2fs_ioc_shutdown(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct super_block *sb = sbi->sb;
	__u32 in;
	int ret = 0;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (get_user(in, (__u32 __user *)arg))
		return -EFAULT;

	if (in != F2FS_GOING_DOWN_FULLSYNC) {
		ret = mnt_want_write_file(filp);
		if (ret)
			return ret;
	}

	switch (in) {
	case F2FS_GOING_DOWN_FULLSYNC:
		sb = freeze_bdev(sb->s_bdev);
		if (IS_ERR(sb)) {
			ret = PTR_ERR(sb);
			goto out;
		}
		if (sb) {
			hmfs_stop_checkpoint(sbi, false);
			set_sbi_flag(sbi, SBI_IS_SHUTDOWN);
			thaw_bdev(sb->s_bdev, sb);
		}
		break;
	case F2FS_GOING_DOWN_METASYNC:
		/* do checkpoint only */
		ret = hmfs_sync_fs(sb, 1);
		if (ret)
			goto out;
		hmfs_stop_checkpoint(sbi, false);
		set_sbi_flag(sbi, SBI_IS_SHUTDOWN);
		break;
	case F2FS_GOING_DOWN_NOSYNC:
		hmfs_stop_checkpoint(sbi, false);
		set_sbi_flag(sbi, SBI_IS_SHUTDOWN);
		break;
	case F2FS_GOING_DOWN_METAFLUSH:
		hmfs_sync_meta_pages(sbi, META, LONG_MAX, FS_META_IO);
		hmfs_stop_checkpoint(sbi, false);
		set_sbi_flag(sbi, SBI_IS_SHUTDOWN);
		break;
	default:
		ret = -EINVAL;
		goto out;
	}

	hmfs_stop_gc_thread(sbi);
	hmfs_stop_discard_thread(sbi);

	hmfs_drop_discard_cmd(sbi);
	clear_opt(sbi, DISCARD);

	f2fs_update_time(sbi, REQ_TIME);
out:
	if (in != F2FS_GOING_DOWN_FULLSYNC)
		mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_fitrim(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct super_block *sb = inode->i_sb;
	struct fstrim_range range;
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (!f2fs_hw_support_discard(F2FS_SB(sb)))
		return -EOPNOTSUPP;

	if (copy_from_user(&range, (struct fstrim_range __user *)arg,
				sizeof(range)))
		return -EFAULT;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	/*
	 * disable aligning minlen to discard granularity of device,
	 * so that smaller discard could be issued from FITRIM interface.
	 */

	printk_ratelimited(KERN_INFO
		"%s: Receive fstrim command from userspace!\n", __func__);

	ret = hmfs_trim_fs(F2FS_SB(sb), &range);
	mnt_drop_write_file(filp);
	if (ret < 0)
		return ret;

	if (copy_to_user((struct fstrim_range __user *)arg, &range,
				sizeof(range)))
		return -EFAULT;
	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	return 0;
}

static bool uuid_is_nonzero(__u8 u[16])
{
	int i;

	for (i = 0; i < 16; i++)
		if (u[i])
			return true;
	return false;
}

static int f2fs_ioc_set_encryption_policy(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);

	if (!f2fs_sb_has_encrypt(inode->i_sb))
		return -EOPNOTSUPP;

	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);

	return fscrypt_ioctl_set_policy(filp, (const void __user *)arg);
}

static int f2fs_ioc_get_encryption_policy(struct file *filp, unsigned long arg)
{
	if (!f2fs_sb_has_encrypt(file_inode(filp)->i_sb))
		return -EOPNOTSUPP;
	return fscrypt_ioctl_get_policy(filp, (void __user *)arg);
}

static int f2fs_ioc_get_encryption_pwsalt(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int err;

	if (!f2fs_sb_has_encrypt(inode->i_sb))
		return -EOPNOTSUPP;

	err = mnt_want_write_file(filp);
	if (err)
		return err;

	down_write(&sbi->sb_lock);

	if (uuid_is_nonzero(sbi->raw_super->encrypt_pw_salt))
		goto got_it;

	/* update superblock with uuid */
	generate_random_uuid(sbi->raw_super->encrypt_pw_salt);

	err = hmfs_commit_super(sbi, false);
	if (err) {
		/* undo new data */
		memset(sbi->raw_super->encrypt_pw_salt, 0, 16);
		goto out_err;
	}
got_it:
	if (copy_to_user((__u8 __user *)arg, sbi->raw_super->encrypt_pw_salt,
									16))
		err = -EFAULT;
out_err:
	up_write(&sbi->sb_lock);
	mnt_drop_write_file(filp);
	return err;
}

static int IOC_GC_count = 0;

static int f2fs_ioc_gc(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	__u32 sync;
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (get_user(sync, (__u32 __user *)arg))
		return -EFAULT;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	if (!sync) {
		if (!mutex_trylock(&sbi->gc_mutex)) {
			ret = -EBUSY;
			goto out;
		}
	} else {
		mutex_lock(&sbi->gc_mutex);
	}

	hmfs_msg(sbi->sb, KERN_INFO,
		"IOC_GC: Size=%lldMB,Free=%lldMB,count=%d,sync=%d\n",
		blks_to_mb(sbi->user_block_count, sbi->blocksize),
		blks_to_mb(sbi->user_block_count - valid_user_blocks(sbi),
			sbi->blocksize),
		IOC_GC_count++, sync);

	current->flags |= PF_MUTEX_GC;
	ret = hmfs_gc(sbi, sync, true, NULL_SEGNO);
	current->flags &= (~PF_MUTEX_GC);
out:
	mnt_drop_write_file(filp);
	return ret;
}


static int f2fs_ioc_gc_range(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct hmfs_gc_range range;
	u64 end;
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (copy_from_user(&range, (struct hmfs_gc_range __user *)arg,
							sizeof(range)))
		return -EFAULT;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	end = range.start + range.len;
	if (range.start < MAIN_BLKADDR(sbi) || end >= MAX_BLKADDR(sbi)) {
		return -EINVAL;
	}


	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

do_more:
	if (!range.sync) {
		if (!mutex_trylock(&sbi->gc_mutex)) {
			ret = -EBUSY;
			goto out;
		}
	} else {
		mutex_lock(&sbi->gc_mutex);
	}

	current->flags |= PF_MUTEX_GC;
	ret = hmfs_gc(sbi, range.sync, true, GET_SEGNO(sbi, range.start));
	current->flags &= (~PF_MUTEX_GC);
	range.start += sbi->blocks_per_seg;
	if (range.start <= end)
		goto do_more;
out:
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_write_checkpoint(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED))) {
		hmfs_msg(sbi->sb, KERN_INFO,
			"Skipping Checkpoint. Checkpoints currently disabled.");
		return -EINVAL;
	}

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	ret = hmfs_sync_fs(sbi->sb, 1);

	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_defragment_range(struct f2fs_sb_info *sbi,
					struct file *filp,
					struct f2fs_defragment *range)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_map_blocks map = { .m_next_extent = NULL,
					.m_seg_type = NO_CHECK_TYPE };
	struct extent_info ei = {0, 0, 0};
	pgoff_t pg_start, pg_end, next_pgofs;
	unsigned int blk_per_seg = sbi->blocks_per_seg;
	unsigned int total = 0, sec_num;
	block_t blk_end = 0;
	bool fragmented = false;
	int err;

	/* if in-place-update policy is enabled, don't waste time here */
	if (hmfs_should_update_inplace(inode, NULL))
		return -EINVAL;

	pg_start = range->start >> PAGE_SHIFT;
	pg_end = (range->start + range->len) >> PAGE_SHIFT;

	hmfs_balance_fs(sbi, true);

	inode_lock(inode);

	/* writeback all dirty pages in the range */
	err = filemap_write_and_wait_range(inode->i_mapping, range->start,
						range->start + range->len - 1);
	if (err)
		goto out;

	/*
	 * lookup mapping info in extent cache, skip defragmenting if physical
	 * block addresses are continuous.
	 */
	if (hmfs_lookup_extent_cache(inode, pg_start, &ei)) {
		if (ei.fofs + ei.len >= pg_end)
			goto out;
	}

	map.m_lblk = pg_start;
	map.m_next_pgofs = &next_pgofs;

	/*
	 * lookup mapping info in dnode page cache, skip defragmenting if all
	 * physical block addresses are continuous even if there are hole(s)
	 * in logical blocks.
	 */
	while (map.m_lblk < pg_end) {
		map.m_len = pg_end - map.m_lblk;
		err = hmfs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_DEFAULT);
		if (err)
			goto out;

		if (!(map.m_flags & F2FS_MAP_FLAGS)) {
			map.m_lblk = next_pgofs;
			continue;
		}

		if (blk_end && blk_end != map.m_pblk)
			fragmented = true;

		/* record total count of block that we're going to move */
		total += map.m_len;

		blk_end = map.m_pblk + map.m_len;

		map.m_lblk += map.m_len;
	}

	if (!fragmented)
		goto out;

	sec_num = (total + BLKS_PER_SEC(sbi) - 1) / BLKS_PER_SEC(sbi);

	/*
	 * make sure there are enough free section for LFS allocation, this can
	 * avoid defragment running in SSR mode when free section are allocated
	 * intensively
	 */
	if (has_not_enough_free_secs(sbi, 0, sec_num)) {
		err = -EAGAIN;
		goto out;
	}

	map.m_lblk = pg_start;
	map.m_len = pg_end - pg_start;
	total = 0;

	while (map.m_lblk < pg_end) {
		pgoff_t idx;
		int cnt = 0;

do_map:
		map.m_len = pg_end - map.m_lblk;
		err = hmfs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_DEFAULT);
		if (err)
			goto clear_out;

		if (!(map.m_flags & F2FS_MAP_FLAGS)) {
			map.m_lblk = next_pgofs;
			continue;
		}

		set_inode_flag(inode, FI_DO_DEFRAG);

		idx = map.m_lblk;
		while (idx < map.m_lblk + map.m_len && cnt < blk_per_seg) {
			struct page *page;

			page = hmfs_get_lock_data_page(inode, idx, true);
			if (IS_ERR(page)) {
				err = PTR_ERR(page);
				goto clear_out;
			}

			set_page_dirty(page);
			f2fs_put_page(page, 1);

			idx++;
			cnt++;
			total++;
		}

		map.m_lblk = idx;

		if (idx < pg_end && cnt < blk_per_seg)
			goto do_map;

		clear_inode_flag(inode, FI_DO_DEFRAG);

		err = filemap_fdatawrite(inode->i_mapping);
		if (err)
			goto out;
	}
clear_out:
	clear_inode_flag(inode, FI_DO_DEFRAG);
out:
	inode_unlock(inode);
	if (!err)
		range->len = (u64)total << PAGE_SHIFT;
	return err;
}

static int f2fs_ioc_defragment(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_defragment range;
	int err;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (!S_ISREG(inode->i_mode) || f2fs_is_atomic_file(inode))
		return -EINVAL;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	if (copy_from_user(&range, (struct f2fs_defragment __user *)arg,
							sizeof(range)))
		return -EFAULT;

	/* verify alignment of offset & size */
	if (range.start & (F2FS_BLKSIZE - 1) || range.len & (F2FS_BLKSIZE - 1))
		return -EINVAL;

	if (unlikely((range.start + range.len) >> PAGE_SHIFT >
					sbi->max_file_blocks))
		return -EINVAL;

	err = mnt_want_write_file(filp);
	if (err)
		return err;

	err = f2fs_defragment_range(sbi, filp, &range);
	mnt_drop_write_file(filp);

	f2fs_update_time(sbi, REQ_TIME);
	if (err < 0)
		return err;

	if (copy_to_user((struct f2fs_defragment __user *)arg, &range,
							sizeof(range)))
		return -EFAULT;

	return 0;
}

static int f2fs_move_file_range(struct file *file_in, loff_t pos_in,
			struct file *file_out, loff_t pos_out, size_t len)
{
	struct inode *src = file_inode(file_in);
	struct inode *dst = file_inode(file_out);
	struct f2fs_sb_info *sbi = F2FS_I_SB(src);
	size_t olen = len, dst_max_i_size = 0;
	size_t dst_osize;
	int ret;

	if (file_in->f_path.mnt != file_out->f_path.mnt ||
				src->i_sb != dst->i_sb)
		return -EXDEV;

	if (unlikely(f2fs_readonly(src->i_sb)))
		return -EROFS;

	if (!S_ISREG(src->i_mode) || !S_ISREG(dst->i_mode))
		return -EINVAL;

	if (f2fs_encrypted_inode(src) || f2fs_encrypted_inode(dst))
		return -EOPNOTSUPP;

	if (src == dst) {
		if (pos_in == pos_out)
			return 0;
		if (pos_out > pos_in && pos_out < pos_in + len)
			return -EINVAL;
	}

	inode_lock(src);
	if (src != dst) {
		ret = -EBUSY;
		if (!inode_trylock(dst))
			goto out;
	}

	ret = -EINVAL;
	if (pos_in + len > src->i_size || pos_in + len < pos_in)
		goto out_unlock;
	if (len == 0)
		olen = len = src->i_size - pos_in;
	if (pos_in + len == src->i_size)
		len = ALIGN(src->i_size, F2FS_BLKSIZE) - pos_in;
	if (len == 0) {
		ret = 0;
		goto out_unlock;
	}

	dst_osize = dst->i_size;
	if (pos_out + olen > dst->i_size)
		dst_max_i_size = pos_out + olen;

	/* verify the end result is block aligned */
	if (!IS_ALIGNED(pos_in, F2FS_BLKSIZE) ||
			!IS_ALIGNED(pos_in + len, F2FS_BLKSIZE) ||
			!IS_ALIGNED(pos_out, F2FS_BLKSIZE))
		goto out_unlock;

	ret = hmfs_convert_inline_inode(src);
	if (ret)
		goto out_unlock;

	ret = hmfs_convert_inline_inode(dst);
	if (ret)
		goto out_unlock;

	/* write out all dirty pages from offset */
	ret = filemap_write_and_wait_range(src->i_mapping,
					pos_in, pos_in + len);
	if (ret)
		goto out_unlock;

	ret = filemap_write_and_wait_range(dst->i_mapping,
					pos_out, pos_out + len);
	if (ret)
		goto out_unlock;

	hmfs_balance_fs(sbi, true);

	down_write(&F2FS_I(src)->i_gc_rwsem[WRITE]);
	if (src != dst) {
		ret = -EBUSY;
		if (!down_write_trylock(&F2FS_I(dst)->i_gc_rwsem[WRITE]))
			goto out_src;
	}

	f2fs_lock_op(sbi);
	ret = __exchange_data_block(src, dst, pos_in >> F2FS_BLKSIZE_BITS,
				pos_out >> F2FS_BLKSIZE_BITS,
				len >> F2FS_BLKSIZE_BITS, false);

	if (!ret) {
		if (dst_max_i_size)
			f2fs_i_size_write(dst, dst_max_i_size);
		else if (dst_osize != dst->i_size)
			f2fs_i_size_write(dst, dst_osize);
	}
	f2fs_unlock_op(sbi);

	if (src != dst)
		up_write(&F2FS_I(dst)->i_gc_rwsem[WRITE]);
out_src:
	up_write(&F2FS_I(src)->i_gc_rwsem[WRITE]);
out_unlock:
	if (src != dst)
		inode_unlock(dst);
out:
	inode_unlock(src);
	return ret;
}

static int f2fs_ioc_move_range(struct file *filp, unsigned long arg)
{
	struct f2fs_move_range range;
	struct fd dst;
	int err;

	if (!(filp->f_mode & FMODE_READ) ||
			!(filp->f_mode & FMODE_WRITE))
		return -EBADF;

	if (copy_from_user(&range, (struct f2fs_move_range __user *)arg,
							sizeof(range)))
		return -EFAULT;

	dst = fdget(range.dst_fd);
	if (!dst.file)
		return -EBADF;

	if (!(dst.file->f_mode & FMODE_WRITE)) {
		err = -EBADF;
		goto err_out;
	}

	err = mnt_want_write_file(filp);
	if (err)
		goto err_out;

	err = f2fs_move_file_range(filp, range.pos_in, dst.file,
					range.pos_out, range.len);

	mnt_drop_write_file(filp);
	if (err)
		goto err_out;

	if (copy_to_user((struct f2fs_move_range __user *)arg,
						&range, sizeof(range)))
		err = -EFAULT;
err_out:
	fdput(dst);
	return err;
}

static int f2fs_ioc_flush_device(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct sit_info *sm = SIT_I(sbi);
	unsigned int start_segno = 0, end_segno = 0;
	unsigned int dev_start_segno = 0, dev_end_segno = 0;
	struct f2fs_flush_device range;
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	if (unlikely(is_sbi_flag_set(sbi, SBI_CP_DISABLED)))
		return -EINVAL;

	if (copy_from_user(&range, (struct f2fs_flush_device __user *)arg,
							sizeof(range)))
		return -EFAULT;

	if (sbi->s_ndevs <= 1 || sbi->s_ndevs - 1 <= range.dev_num ||
			IS_MULTI_SEGS_IN_SEC(sbi)) {
		hmfs_msg(sbi->sb, KERN_WARNING,
			"Can't flush %u in %d for segs_per_sec %u != 1\n",
				range.dev_num, sbi->s_ndevs,
				sbi->segs_per_sec);
		return -EINVAL;
	}


	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	if (range.dev_num != 0)
		dev_start_segno = GET_SEGNO(sbi, FDEV(range.dev_num).start_blk);
	dev_end_segno = GET_SEGNO(sbi, FDEV(range.dev_num).end_blk);

	start_segno = sm->last_victim[FLUSH_DEVICE];
	if (start_segno < dev_start_segno || start_segno >= dev_end_segno)
		start_segno = dev_start_segno;
	end_segno = min(start_segno + range.segments, dev_end_segno);

	while (start_segno < end_segno) {
		if (!mutex_trylock(&sbi->gc_mutex)) {
			ret = -EBUSY;
			goto out;
		}
		sm->last_victim[GC_CB] = end_segno + 1;
		sm->last_victim[GC_GREEDY] = end_segno + 1;
		sm->last_victim[ALLOC_NEXT] = end_segno + 1;
		current->flags |= PF_MUTEX_GC;
		ret = hmfs_gc(sbi, true, true, start_segno);
		current->flags &= (~PF_MUTEX_GC);
		if (ret == -EAGAIN)
			ret = 0;
		else if (ret < 0)
			break;
		start_segno++;
	}
out:
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_get_features(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	u32 sb_feature = le32_to_cpu(F2FS_I_SB(inode)->raw_super->feature);

	/* Must validate to set it with SQLite behavior in Android. */
	sb_feature |= F2FS_FEATURE_ATOMIC_WRITE;

	return put_user(sb_feature, (u32 __user *)arg);
}

#ifdef CONFIG_QUOTA
int hmfs_transfer_project_quota(struct inode *inode, kprojid_t kprojid)
{
	struct dquot *transfer_to[MAXQUOTAS] = {};
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct super_block *sb = sbi->sb;
	int err = 0;

	transfer_to[PRJQUOTA] = dqget(sb, make_kqid_projid(kprojid));
	if (!IS_ERR(transfer_to[PRJQUOTA])) {
		err = __dquot_transfer(inode, transfer_to);
		if (err)
			set_sbi_flag(sbi, SBI_QUOTA_NEED_REPAIR);
		dqput(transfer_to[PRJQUOTA]);
	}
	return err;
}

static int f2fs_ioc_setproject(struct file *filp, __u32 projid)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct super_block *sb = sbi->sb;
	struct page *ipage;
	kprojid_t kprojid;
	int err;

	if (!f2fs_sb_has_project_quota(sb)) {
		if (projid != F2FS_DEF_PROJID)
			return -EOPNOTSUPP;
		else
			return 0;
	}

	if (!f2fs_has_extra_attr(inode))
		return -EOPNOTSUPP;

	kprojid = make_kprojid(&init_user_ns, (projid_t)projid);

	if (projid_eq(kprojid, F2FS_I(inode)->i_projid))
		return 0;

	err = -EPERM;
	/* Is it quota file? Do not allow user to mess with it */
	if (IS_NOQUOTA(inode))
		return err;

	ipage = hmfs_get_node_page(sbi, inode->i_ino);
	if (IS_ERR(ipage))
		return PTR_ERR(ipage);

	if (!F2FS_FITS_IN_INODE(F2FS_INODE(ipage), fi->i_extra_isize,
								i_projid)) {
		err = -EOVERFLOW;
		f2fs_put_page(ipage, 1);
		return err;
	}
	f2fs_put_page(ipage, 1);

	err = dquot_initialize(inode);
	if (err)
		return err;

	f2fs_lock_op(sbi);
	err = hmfs_transfer_project_quota(inode, kprojid);
	if (err)
		goto out_unlock;

	F2FS_I(inode)->i_projid = kprojid;
	inode->i_ctime = current_time(inode);
	hmfs_mark_inode_dirty_sync(inode, true);
out_unlock:
	f2fs_unlock_op(sbi);
	return err;
}
#else
int hmfs_transfer_project_quota(struct inode *inode, kprojid_t kprojid)
{
	return 0;
}

static int f2fs_ioc_setproject(struct file *filp, __u32 projid)
{
	if (projid != F2FS_DEF_PROJID)
		return -EOPNOTSUPP;
	return 0;
}
#endif

/* Transfer internal flags to xflags */
static inline __u32 f2fs_iflags_to_xflags(unsigned long iflags)
{
	__u32 xflags = 0;

	if (iflags & F2FS_SYNC_FL)
		xflags |= FS_XFLAG_SYNC;
	if (iflags & F2FS_IMMUTABLE_FL)
		xflags |= FS_XFLAG_IMMUTABLE;
	if (iflags & F2FS_APPEND_FL)
		xflags |= FS_XFLAG_APPEND;
	if (iflags & F2FS_NODUMP_FL)
		xflags |= FS_XFLAG_NODUMP;
	if (iflags & F2FS_NOATIME_FL)
		xflags |= FS_XFLAG_NOATIME;
	if (iflags & F2FS_PROJINHERIT_FL)
		xflags |= FS_XFLAG_PROJINHERIT;
	return xflags;
}

#define F2FS_SUPPORTED_FS_XFLAGS (FS_XFLAG_SYNC | FS_XFLAG_IMMUTABLE | \
				  FS_XFLAG_APPEND | FS_XFLAG_NODUMP | \
				  FS_XFLAG_NOATIME | FS_XFLAG_PROJINHERIT)

/* Transfer xflags flags to internal */
static inline unsigned long f2fs_xflags_to_iflags(__u32 xflags)
{
	unsigned long iflags = 0;

	if (xflags & FS_XFLAG_SYNC)
		iflags |= F2FS_SYNC_FL;
	if (xflags & FS_XFLAG_IMMUTABLE)
		iflags |= F2FS_IMMUTABLE_FL;
	if (xflags & FS_XFLAG_APPEND)
		iflags |= F2FS_APPEND_FL;
	if (xflags & FS_XFLAG_NODUMP)
		iflags |= F2FS_NODUMP_FL;
	if (xflags & FS_XFLAG_NOATIME)
		iflags |= F2FS_NOATIME_FL;
	if (xflags & FS_XFLAG_PROJINHERIT)
		iflags |= F2FS_PROJINHERIT_FL;

	return iflags;
}

static int f2fs_ioc_fsgetxattr(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct fsxattr fa;

	memset(&fa, 0, sizeof(struct fsxattr));
	fa.fsx_xflags = f2fs_iflags_to_xflags(fi->i_flags &
				F2FS_FL_USER_VISIBLE);

	if (f2fs_sb_has_project_quota(inode->i_sb))
		fa.fsx_projid = (__u32)from_kprojid(&init_user_ns,
							fi->i_projid);

	if (copy_to_user((struct fsxattr __user *)arg, &fa, sizeof(fa)))
		return -EFAULT;
	return 0;
}

static int hmfs_ioctl_check_project(struct inode *inode, struct fsxattr *fa)
{
	/*
	 * Project Quota ID state is only allowed to change from within the init
	 * namespace. Enforce that restriction only if we are trying to change
	 * the quota ID state. Everything else is allowed in user namespaces.
	 */
	if (current_user_ns() == &init_user_ns)
		return 0;

	if (__kprojid_val(F2FS_I(inode)->i_projid) != fa->fsx_projid)
		return -EINVAL;

	if (F2FS_I(inode)->i_flags & F2FS_PROJINHERIT_FL) {
		if (!(fa->fsx_xflags & FS_XFLAG_PROJINHERIT))
			return -EINVAL;
	} else {
		if (fa->fsx_xflags & FS_XFLAG_PROJINHERIT)
			return -EINVAL;
	}

	return 0;
}

static int f2fs_ioc_fssetxattr(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct fsxattr fa;
	unsigned int flags;
	int err;

	if (copy_from_user(&fa, (struct fsxattr __user *)arg, sizeof(fa)))
		return -EFAULT;

	/* Make sure caller has proper permission */
	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (fa.fsx_xflags & ~F2FS_SUPPORTED_FS_XFLAGS)
		return -EOPNOTSUPP;

	flags = f2fs_xflags_to_iflags(fa.fsx_xflags);
	if (f2fs_mask_flags(inode->i_mode, flags) != flags)
		return -EOPNOTSUPP;

	err = mnt_want_write_file(filp);
	if (err)
		return err;

	inode_lock(inode);
	err = hmfs_ioctl_check_project(inode, &fa);
	if (err)
		goto out;
	flags = (fi->i_flags & ~F2FS_FL_XFLAG_VISIBLE) |
				(flags & F2FS_FL_XFLAG_VISIBLE);
	err = __f2fs_ioc_setflags(inode, flags);
	if (err)
		goto out;

	err = f2fs_ioc_setproject(filp, fa.fsx_projid);
out:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return err;
}

int hmfs_pin_file_control(struct inode *inode, bool inc)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	/* Use i_gc_failures for normal file as a risk signal. */
	if (inc)
		f2fs_i_gc_failures_write(inode,
				fi->i_gc_failures[GC_FAILURE_PIN] + 1);

	if (fi->i_gc_failures[GC_FAILURE_PIN] > sbi->gc_pin_file_threshold) {
		hmfs_msg(sbi->sb, KERN_WARNING,
			"%s: Enable GC = ino %lx after %x GC trials\n",
			__func__, inode->i_ino,
			fi->i_gc_failures[GC_FAILURE_PIN]);
		clear_inode_flag(inode, FI_PIN_FILE);
		return -EAGAIN;
	}
	return 0;
}

static int f2fs_ioc_set_pin_file(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	__u32 pin;
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (get_user(pin, (__u32 __user *)arg))
		return -EFAULT;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	if (f2fs_readonly(F2FS_I_SB(inode)->sb))
		return -EROFS;

	ret = mnt_want_write_file(filp);
	if (ret)
		return ret;

	inode_lock(inode);

	if (hmfs_should_update_outplace(inode, NULL)) {
		ret = -EINVAL;
		goto out;
	}

	if (!pin) {
		clear_inode_flag(inode, FI_PIN_FILE);
		f2fs_i_gc_failures_write(inode, 0);
		goto done;
	}

	if (hmfs_pin_file_control(inode, false)) {
		ret = -EAGAIN;
		goto out;
	}
	ret = hmfs_convert_inline_inode(inode);
	if (ret)
		goto out;

	set_inode_flag(inode, FI_PIN_FILE);
	ret = F2FS_I(inode)->i_gc_failures[GC_FAILURE_PIN];
done:
	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
out:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

static int f2fs_ioc_get_pin_file(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	__u32 pin = 0;

	if (is_inode_flag_set(inode, FI_PIN_FILE))
		pin = F2FS_I(inode)->i_gc_failures[GC_FAILURE_PIN];
	return put_user(pin, (u32 __user *)arg);
}

int hmfs_precache_extents(struct inode *inode)
{
	struct f2fs_inode_info *fi = F2FS_I(inode);
	struct f2fs_map_blocks map;
	pgoff_t m_next_extent;
	loff_t end;
	int err;

	if (is_inode_flag_set(inode, FI_NO_EXTENT))
		return -EOPNOTSUPP;

	map.m_lblk = 0;
	map.m_next_pgofs = NULL;
	map.m_next_extent = &m_next_extent;
	map.m_seg_type = NO_CHECK_TYPE;
	end = F2FS_I_SB(inode)->max_file_blocks;

	while (map.m_lblk < end) {
		map.m_len = end - map.m_lblk;

		down_write(&fi->i_gc_rwsem[WRITE]);
		err = hmfs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_PRECACHE);
		up_write(&fi->i_gc_rwsem[WRITE]);
		if (err)
			return err;

		map.m_lblk = m_next_extent;
	}

	return err;
}

static int f2fs_ioc_precache_extents(struct file *filp, unsigned long arg)
{
	return hmfs_precache_extents(file_inode(filp));
}

static int f2fs_ioc_resize_fs(struct file *filp, unsigned long arg)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(file_inode(filp));
	struct f2fs_resize_from_end param;
	int ret;

	if (!capable(CAP_SYS_ADMIN))
		return -EPERM;

	if (f2fs_readonly(sbi->sb))
		return -EROFS;

	if (copy_from_user(&param, (struct f2fs_resize_from_end __user *)arg,
				sizeof(param)))
		return -EFAULT;

	ret = hmfs_resize_fs(sbi, param.len);

	return ret;
}

long hmfs_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	if (unlikely(f2fs_cp_error(F2FS_I_SB(file_inode(filp)))))
		return -EIO;

	switch (cmd) {
	case F2FS_IOC_GETFLAGS:
		return f2fs_ioc_getflags(filp, arg);
	case F2FS_IOC_SETFLAGS:
		return f2fs_ioc_setflags(filp, arg);
	case F2FS_IOC_GETVERSION:
		return f2fs_ioc_getversion(filp, arg);
	case F2FS_IOC_START_ATOMIC_WRITE:
		return f2fs_ioc_start_atomic_write(filp);
	case F2FS_IOC_COMMIT_ATOMIC_WRITE:
		return f2fs_ioc_commit_atomic_write(filp);
	case F2FS_IOC_START_VOLATILE_WRITE:
		return f2fs_ioc_start_volatile_write(filp);
	case F2FS_IOC_RELEASE_VOLATILE_WRITE:
		return f2fs_ioc_release_volatile_write(filp);
	case F2FS_IOC_ABORT_VOLATILE_WRITE:
		return f2fs_ioc_abort_volatile_write(filp);
	case F2FS_IOC_SHUTDOWN:
		return f2fs_ioc_shutdown(filp, arg);
	case FITRIM:
		return f2fs_ioc_fitrim(filp, arg);
	case F2FS_IOC_SET_ENCRYPTION_POLICY:
		return f2fs_ioc_set_encryption_policy(filp, arg);
	case F2FS_IOC_GET_ENCRYPTION_POLICY:
		return f2fs_ioc_get_encryption_policy(filp, arg);
	case F2FS_IOC_GET_ENCRYPTION_PWSALT:
		return f2fs_ioc_get_encryption_pwsalt(filp, arg);
	case F2FS_IOC_GARBAGE_COLLECT:
		return f2fs_ioc_gc(filp, arg);
	case F2FS_IOC_GARBAGE_COLLECT_RANGE:
		return f2fs_ioc_gc_range(filp, arg);
	case F2FS_IOC_WRITE_CHECKPOINT:
		return f2fs_ioc_write_checkpoint(filp, arg);
	case F2FS_IOC_DEFRAGMENT:
		return f2fs_ioc_defragment(filp, arg);
	case F2FS_IOC_MOVE_RANGE:
		return f2fs_ioc_move_range(filp, arg);
	case F2FS_IOC_FLUSH_DEVICE:
		return f2fs_ioc_flush_device(filp, arg);
	case F2FS_IOC_GET_FEATURES:
		return f2fs_ioc_get_features(filp, arg);
	case F2FS_IOC_FSGETXATTR:
		return f2fs_ioc_fsgetxattr(filp, arg);
	case F2FS_IOC_FSSETXATTR:
		return f2fs_ioc_fssetxattr(filp, arg);
	case F2FS_IOC_GET_PIN_FILE:
		return f2fs_ioc_get_pin_file(filp, arg);
	case F2FS_IOC_SET_PIN_FILE:
		return f2fs_ioc_set_pin_file(filp, arg);
	case F2FS_IOC_PRECACHE_EXTENTS:
		return f2fs_ioc_precache_extents(filp, arg);
	case F2FS_IOC_RESIZE_FS:
		return f2fs_ioc_resize_fs(filp, arg);
#if F2FS_FS_SDP_ENCRYPTION
	case F2FS_IOC_SET_SDP_ENCRYPTION_POLICY:
		return hmfs_ioc_set_sdp_encryption_policy(filp, arg);
	case F2FS_IOC_GET_SDP_ENCRYPTION_POLICY:
		return hmfs_ioc_get_sdp_encryption_policy(filp, arg);
	case F2FS_IOC_GET_ENCRYPTION_POLICY_TYPE:
		return hmfs_ioc_get_encryption_policy_type(filp, arg);
#endif
#ifdef CONFIG_HWDPS
	case F2FS_IOC_SET_DPS_ENCRYPTION_POLICY:
		return hmfs_ioc_set_dps_encryption_policy(filp, arg);
#endif
	default:
		return -ENOTTY;
	}
}

static ssize_t f2fs_file_write_iter(struct kiocb *iocb, struct iov_iter *from)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file_inode(file);
	ssize_t ret;

	if (unlikely(f2fs_cp_error(F2FS_I_SB(inode)))) {
		ret = -EIO;
		goto out;
	}

	if ((iocb->ki_flags & IOCB_NOWAIT) && !(iocb->ki_flags & IOCB_DIRECT)) {
		ret = -EINVAL;
		goto out;
	}

	if (!inode_trylock(inode)) {
		if (iocb->ki_flags & IOCB_NOWAIT) {
			ret = -EAGAIN;
			goto out;
		}
		inode_lock(inode);
	}

	ret = generic_write_checks(iocb, from);
	if (ret > 0) {
		bool preallocated = false;
		size_t target_size = 0;
		int err;

		if (iov_iter_fault_in_readable(from, iov_iter_count(from)))
			set_inode_flag(inode, FI_NO_PREALLOC);

		if (iocb->ki_flags & IOCB_DIRECT) {
			err = hmfs_convert_inline_inode(inode);
			if (err) {
				inode_unlock(inode);
				return err;
			}
		}

		if ((iocb->ki_flags & IOCB_NOWAIT) &&
			(iocb->ki_flags & IOCB_DIRECT)) {
				if (!hmfs_overwrite_io(inode, iocb->ki_pos,
						iov_iter_count(from)) ||
					f2fs_has_inline_data(inode) ||
					f2fs_force_buffered_io(inode,
							iocb, from)) {
						clear_inode_flag(inode,
								FI_NO_PREALLOC);
						inode_unlock(inode);
						ret = -EAGAIN;
						goto out;
				}

		} else {
			preallocated = true;
			target_size = iocb->ki_pos + iov_iter_count(from);

			err = hmfs_preallocate_blocks(iocb, from);
			if (err) {
				clear_inode_flag(inode, FI_NO_PREALLOC);
				inode_unlock(inode);
				ret = err;
				goto out;
			}
		}
		ret = __generic_file_write_iter(iocb, from);
		clear_inode_flag(inode, FI_NO_PREALLOC);

		/* if we couldn't write data, we should deallocate blocks. */
		if (preallocated && i_size_read(inode) < target_size)
			hmfs_truncate(inode);

		if (ret > 0)
			f2fs_update_iostat(F2FS_I_SB(inode), APP_WRITE_IO, ret);
	}
	inode_unlock(inode);
out:
	trace_hmfs_file_write_iter(inode, iocb->ki_pos,
					iov_iter_count(from), ret);
	if (ret > 0)
		ret = generic_write_sync(iocb, ret);
	return ret;
}

#ifdef CONFIG_COMPAT
long hmfs_compat_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch (cmd) {
	case F2FS_IOC32_GETFLAGS:
		cmd = F2FS_IOC_GETFLAGS;
		break;
	case F2FS_IOC32_SETFLAGS:
		cmd = F2FS_IOC_SETFLAGS;
		break;
	case F2FS_IOC32_GETVERSION:
		cmd = F2FS_IOC_GETVERSION;
		break;
	case F2FS_IOC_START_ATOMIC_WRITE:
	case F2FS_IOC_COMMIT_ATOMIC_WRITE:
	case F2FS_IOC_START_VOLATILE_WRITE:
	case F2FS_IOC_RELEASE_VOLATILE_WRITE:
	case F2FS_IOC_ABORT_VOLATILE_WRITE:
	case F2FS_IOC_SHUTDOWN:
	case F2FS_IOC_SET_ENCRYPTION_POLICY:
	case F2FS_IOC_GET_ENCRYPTION_PWSALT:
	case F2FS_IOC_GET_ENCRYPTION_POLICY:
	case F2FS_IOC_GARBAGE_COLLECT:
	case F2FS_IOC_GARBAGE_COLLECT_RANGE:
	case F2FS_IOC_WRITE_CHECKPOINT:
	case F2FS_IOC_DEFRAGMENT:
	case F2FS_IOC_MOVE_RANGE:
	case F2FS_IOC_FLUSH_DEVICE:
	case F2FS_IOC_GET_FEATURES:
	case F2FS_IOC_FSGETXATTR:
	case F2FS_IOC_FSSETXATTR:
	case F2FS_IOC_GET_PIN_FILE:
	case F2FS_IOC_SET_PIN_FILE:
	case F2FS_IOC_PRECACHE_EXTENTS:
	case F2FS_IOC_RESIZE_FS:
#if F2FS_FS_SDP_ENCRYPTION
	case F2FS_IOC_SET_SDP_ENCRYPTION_POLICY:
	case F2FS_IOC_GET_ENCRYPTION_POLICY_TYPE:
	case F2FS_IOC_GET_SDP_ENCRYPTION_POLICY:
#endif
		break;
	default:
		return -ENOIOCTLCMD;
	}
	return hmfs_ioctl(file, cmd, (unsigned long) compat_ptr(arg));
}
#endif

const struct file_operations hmfs_file_operations = {
	.llseek		= f2fs_llseek,
	.read_iter	= generic_file_read_iter,
	.write_iter	= f2fs_file_write_iter,
	.open		= f2fs_file_open,
	.release	= f2fs_release_file,
	.mmap		= f2fs_file_mmap,
	.flush		= f2fs_file_flush,
	.fsync		= hmfs_sync_file,
	.fallocate	= f2fs_fallocate,
	.unlocked_ioctl	= hmfs_ioctl,
#ifdef CONFIG_COMPAT
	.compat_ioctl	= hmfs_compat_ioctl,
#endif
	.splice_read	= generic_file_splice_read,
	.splice_write	= iter_file_splice_write,
};
