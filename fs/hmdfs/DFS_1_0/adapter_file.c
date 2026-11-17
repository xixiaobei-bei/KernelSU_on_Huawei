/* SPDX-License-Identifier: GPL-2.0 */
/*
 * adapter_file.c
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: chenyi77@huawei.com
 * Create: 2020-04-15
 *
 */

#include <linux/file.h>
#include <linux/highmem.h>
#include <linux/mm_types.h>
#include <linux/page-flags.h>

#include "adapter_client.h"
#include "hmdfs.h"

static int file_close(struct hmdfs_peer *con, uint64_t device_id, __u32 fno,
		      struct file *file)
{
	int ret = 0;
	struct adapter_req_info *close_data =
		kzalloc(sizeof(*close_data), GFP_KERNEL);
	struct adapter_sendmsg sm = {
		.data = close_data,
		.len = sizeof(*close_data),
		.operations = CLOSE_REQUEST,
		.timeout = MSG_DEFAULT_TIMEOUT,
	};

	if (!close_data)
		return -ENOMEM;
	close_data->fno = fno;
	close_data->src_dno = device_id;
	sm.outmsg = file;

	ret = client_sendmessage_request(con, &sm);
	kfree(close_data);
	return ret;
}

static int file_release(struct inode *inode, struct file *file)
{
	int err = 0;
	struct super_block *sb = inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	uint64_t device_id = sbi->local_info.iid;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	__u32 file_id = info->file_no;

	err = file_close(info->conn, device_id, file_id, file);
	return err;
}

static int file_flush(struct file *file, fl_owner_t id)
{
	int err = 0;
	struct inode *inode = file_inode(file);

	if (file->f_mode & FMODE_WRITE)
		err = filemap_write_and_wait(inode->i_mapping);
	return err;
}

const struct file_operations hmdfs_adapter_remote_file_fops = {
	.owner = THIS_MODULE,
	.llseek = generic_file_llseek,
	.read_iter = generic_file_read_iter,
	.write_iter = generic_file_write_iter,
	.open = generic_file_open,
	.release = file_release,
	.flush = file_flush,
	.fsync = __generic_file_fsync,
};

static int client_remote_readpage(struct hmdfs_peer *con, struct page *page,
				  uint64_t device_id, __u32 fno)
{
	int err = 0;
	struct adapter_read_request *read_data;
	struct adapter_sendmsg sm = {
		.len = sizeof(struct adapter_read_request),
		.operations = READ_REQUEST,
		.timeout = MSG_DEFAULT_TIMEOUT,
		.outmsg = page,
	};

	read_data = kzalloc(sizeof(*read_data), GFP_KERNEL);
	if (!read_data)
		return -ENOMEM;

	sm.data = read_data;
	read_data->req_info.src_dno = device_id;
	read_data->req_info.fno = fno;
	read_data->size = HMDFS_PAGE_SIZE;
	read_data->offset = page->index << HMDFS_PAGE_OFFSET;
	read_data->block = 1; // will wait for server readlock
	read_data->timeout = ADAPTER_LOCK_DEFAULT_TIMEOUT; // ms

	err = client_sendpage_request(con, &sm);
	hmdfs_debug("readpage err %d, offset %llu", err, read_data->offset);

	kfree(read_data);
	return err;
}

static int client_remote_write(struct hmdfs_peer *con, const void *buf,
			       loff_t offset, uint64_t len,
			       const struct adapter_req_info *req_info)
{
	int ret = 0;
	struct adapter_write_request *write_data;
	struct adapter_write_data_buffer *write_buf;
	struct adapter_sendmsg sm = {
		.operations = WRITE_REQUEST,
		.timeout = NO_WAIT,
	};
	int send_len = sizeof(struct adapter_write_request) +
		       sizeof(struct adapter_write_data_buffer) + len;

	write_data = kzalloc(send_len, GFP_KERNEL);
	if (!write_data)
		return -ENOMEM;

	write_data->req_info.fno = req_info->fno;
	write_data->req_info.src_dno = req_info->src_dno;
	write_data->size = sizeof(struct adapter_write_data_buffer) + len;
	write_data->block = 1; // will wait for server filelock
	write_data->timeout = ADAPTER_LOCK_DEFAULT_TIMEOUT;
	sm.data = write_data;
	sm.len = send_len;

	write_buf = (struct adapter_write_data_buffer
			     *)((void *)write_data +
				sizeof(struct adapter_write_request));
	write_buf->payload_size = len;
	write_buf->size = len + sizeof(struct adapter_write_data_buffer);
	write_buf->offset = offset;

	memcpy(write_buf->buf, buf, len);
	ret = client_sendmessage_request(con, &sm);

	kfree(write_data);
	return ret;
}

static int readpage(struct file *file, struct page *page)
{
	int ret = 0;
	struct inode *inode = file_inode(file);
	struct super_block *sb = inode->i_sb;
	struct hmdfs_sb_info *sbi = sb->s_fs_info;
	uint64_t device_id = sbi->local_info.iid;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	__u32 file_id = info->file_no;

	ret = client_remote_readpage(info->conn, page, device_id, file_id);
	return ret;
}

static int writepage(struct page *page, struct writeback_control *wbc)
{
	uint64_t i_size, size;
	struct inode *inode = page->mapping->host;
	struct hmdfs_inode_info *info = hmdfs_i(inode);
	struct hmdfs_sb_info *sbi = inode->i_sb->s_fs_info;
	struct adapter_req_info req_info = {
		.src_dno = sbi->local_info.iid,
		.fno = info->file_no,
	};
	char *buf = kmap(page);

	i_size = (uint64_t)i_size_read(inode);
	if (unlikely(!i_size || page->index > (i_size - 1) >> PAGE_SHIFT)) {
		hmdfs_err("page exceeds end index, skip write page");
		goto out_err;
	}

	size = HMDFS_PAGE_SIZE;
	if (page->index == (i_size - 1) >> PAGE_SHIFT)
		size = ((i_size - 1) & ~PAGE_MASK) + 1;

	set_page_writeback(page);
	client_remote_write(info->conn, buf, page->index << HMDFS_PAGE_OFFSET,
			    size, &req_info);
	SetPageUptodate(page);
	end_page_writeback(page);
out_err:
	unlock_page(page);
	kunmap(page);
	return 0;
}

/* copy from file.c */
static int write_begin(struct file *file, struct address_space *mapping,
		       loff_t pos, unsigned int len, unsigned int flags,
		       struct page **pagep, void **fsdata)
{
	pgoff_t index = ((unsigned long long)pos) >> PAGE_SHIFT;
	struct inode *inode = file_inode(file);
	struct page *page = pagecache_get_page(
		mapping, index, FGP_LOCK | FGP_WRITE | FGP_CREAT, GFP_NOFS);
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
	// We need readpage before write date to this page.
	readpage(file, page);
	lock_page(page);
	return 0;
}

/* copy from file.c */
static int write_end(struct file *file, struct address_space *mapping,
		     loff_t pos, unsigned int len, unsigned int copied,
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

	set_page_dirty(page);

	if (pos + copied > i_size_read(inode))
		i_size_write(inode, pos + copied);
unlock_out:
	unlock_page(page);
	put_page(page);
	return copied;
}

const struct address_space_operations hmdfs_adapter_remote_file_aops = {
	.readpage = readpage,
	.writepage = writepage,
	.write_begin = write_begin,
	.write_end = write_end,
	.set_page_dirty = __set_page_dirty_nobuffers,
};
