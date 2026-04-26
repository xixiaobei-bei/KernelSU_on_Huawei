/* SPDX-License-Identifier: GPL-2.0 */
/*
 * hmdfs_trace.h
 *
 * Copyright (c) 2020-2021 Huawei Technologies Co., Ltd.
 * Author: maojingjing@huawei.com
 * Create: 2020-04-27
 *
 */

#undef TRACE_SYSTEM
#define TRACE_SYSTEM hmdfs

#if !defined(__HMDFS_TRACE_H__) || defined(TRACE_HEADER_MULTI_READ)

#define __HMDFS_TRACE_H__

#include <linux/tracepoint.h>
#include "comm/protocol.h"
#include "hmdfs_dentryfile.h"
#include "hmdfs_client.h"
#include "hmdfs_device_view.h"
#include "client_writeback.h"
#include "DFS_1_0/adapter_protocol.h"

TRACE_EVENT(hmdfs_permission,

	    TP_PROTO(unsigned long ino),

	    TP_ARGS(ino),

	    TP_STRUCT__entry(__field(unsigned long, ino)),

	    TP_fast_assign(__entry->ino = ino;),

	    TP_printk("permission check for ino %lu failed", __entry->ino));

/* communication */
TRACE_EVENT(hmdfs_recv_mesg_callback,

	TP_PROTO(struct hmdfs_head_cmd *cmd),

	TP_ARGS(cmd),

	TP_STRUCT__entry(
		__field(__u32, msg_id)
		__field(__u32, magic)
		__field(__u16, command)
		__field(__u16, cmd_flag)
		__field(__u32, data_len)
		__field(__u32, ret_code)
	),

	TP_fast_assign(
		__entry->msg_id = le32_to_cpu(cmd->msg_id);
		__entry->magic = cmd->magic;
		__entry->command = cmd->operations.command;
		__entry->cmd_flag = cmd->operations.cmd_flag;
		__entry->data_len = cmd->data_len;
		__entry->ret_code = cmd->ret_code;
	),

	TP_printk("msg_id:%u magic:%u command:%hu, cmd_flag:%hu, data_len:%u, ret_code:%u",
		__entry->msg_id, __entry->magic, __entry->command,
		__entry->cmd_flag, __entry->data_len, __entry->ret_code)
);

TRACE_EVENT(hmdfs_tcp_send_message_1_0,

	TP_PROTO(struct hmdfs_adapter_head *cmd),

	TP_ARGS(cmd),

	TP_STRUCT__entry(
		__field(__u8, magic)
		__field(__u8, version)
		__field(__u8, operations)
		__field(__u8, flags)
		__field(__u32, datasize)
		__field(__u64, source)
		__field(__u16, msg_id)
		__field(__u16, request_id)
	),

	TP_fast_assign(
		__entry->magic = cmd->magic;
		__entry->version = cmd->version;
		__entry->operations = cmd->operations;
		__entry->flags = cmd->flags;
		__entry->datasize = cmd->datasize;
		__entry->source = cmd->source;
		__entry->msg_id = cmd->msg_id;
		__entry->request_id = cmd->request_id;
	),

	TP_printk("msg_id:%hu magic:%hhu command:%hhu, version %hhu, flags %hhu, datasize %u, source %llu, request_id %hu",
		__entry->msg_id, __entry->magic, __entry->operations,
		__entry->version, __entry->flags, __entry->datasize,
		__entry->source, __entry->request_id)
);

TRACE_EVENT(hmdfs_tcp_send_message_2_0,

	TP_PROTO(struct hmdfs_head_cmd *cmd),

	TP_ARGS(cmd),

	TP_STRUCT__entry(
		__field(__u32, msg_id)
		__field(__u32, magic)
		__field(__u16, command)
		__field(__u16, cmd_flag)
		__field(__u32, data_len)
		__field(__u32, ret_code)
	),

	TP_fast_assign(
		__entry->msg_id = le32_to_cpu(cmd->msg_id);
		__entry->magic = cmd->magic;
		__entry->command = cmd->operations.command;
		__entry->cmd_flag = cmd->operations.cmd_flag;
		__entry->data_len = cmd->data_len;
		__entry->ret_code = cmd->ret_code;
	),

	TP_printk("msg_id:%u magic:%u command:%hu, cmd_flag:%hu, data_len:%u, ret_code:%u",
		__entry->msg_id, __entry->magic, __entry->command,
		__entry->cmd_flag, __entry->data_len, __entry->ret_code)
);

/* file system interface */
DECLARE_EVENT_CLASS(hmdfs_iterate_op_end,

	TP_PROTO(struct dentry *__d, loff_t start_pos, loff_t end_pos, int err),

	TP_ARGS(__d, start_pos, end_pos, err),

	TP_STRUCT__entry(
		__string(name_str, __d->d_name.name)
		__field(loff_t, start)
		__field(loff_t, end)
		__field(int, err)
	),

	TP_fast_assign(
		__assign_str(name_str, __d->d_name.name);
		__entry->start = start_pos;
		__entry->end = end_pos;
		__entry->err = err;
	),

	TP_printk("dentry[%s] start_pos:%llx, end_pos:%llx, err:%d",
		__get_str(name_str), __entry->start,
		__entry->end, __entry->err)
);

#define define_hmdfs_iterate_op_end_event(event_name)                          \
	DEFINE_EVENT(hmdfs_iterate_op_end, event_name,                         \
		     TP_PROTO(struct dentry *__d, loff_t start_pos,            \
			      loff_t end_pos, int err),                        \
		     TP_ARGS(__d, start_pos, end_pos, err))

define_hmdfs_iterate_op_end_event(hmdfs_iterate_local);
define_hmdfs_iterate_op_end_event(hmdfs_iterate_remote);
define_hmdfs_iterate_op_end_event(hmdfs_iterate_merge);


TRACE_EVENT(hmdfs_lookup,

	TP_PROTO(struct inode *dir, struct dentry *dentry, unsigned int flags),

	TP_ARGS(dir, dentry, flags),

	TP_STRUCT__entry(
		__field(ino_t, ino)
		__string(name_str, dentry->d_name.name)
		__field(unsigned int, flags)
	),

	TP_fast_assign(
		__entry->ino = dir->i_ino;
		__assign_str(name_str, dentry->d_name.name);
		__entry->flags = flags;
	),

	TP_printk("parent_ino = %lu, name:%s, flags:%u",
		__entry->ino, __get_str(name_str), __entry->flags)
);

DECLARE_EVENT_CLASS(hmdfs_lookup_op_end,

	TP_PROTO(struct inode *dir, struct dentry *dentry, int err),

	TP_ARGS(dir, dentry, err),

	TP_STRUCT__entry(
		__field(ino_t, ino)
		__string(name_str, dentry->d_name.name)
		__field(int, err)
	),

	TP_fast_assign(
		__entry->ino = dir->i_ino;
		__assign_str(name_str, dentry->d_name.name);
		__entry->err = err;
	),

	TP_printk("parent_ino = %lu, name:%s, err:%d",
		__entry->ino, __get_str(name_str), __entry->err)
);

#define define_hmdfs_lookup_op_end_event(event_name)                           \
	DEFINE_EVENT(hmdfs_lookup_op_end, event_name,                          \
		     TP_PROTO(struct inode *dir, struct dentry *dentry,        \
			      int err),                                        \
		     TP_ARGS(dir, dentry, err))


define_hmdfs_lookup_op_end_event(hmdfs_root_lookup);
define_hmdfs_lookup_op_end_event(hmdfs_root_lookup_end);

define_hmdfs_lookup_op_end_event(hmdfs_device_lookup);
define_hmdfs_lookup_op_end_event(hmdfs_device_lookup_end);

define_hmdfs_lookup_op_end_event(hmdfs_lookup_local);
define_hmdfs_lookup_op_end_event(hmdfs_lookup_local_end);
define_hmdfs_lookup_op_end_event(hmdfs_mkdir_local);
define_hmdfs_lookup_op_end_event(hmdfs_rmdir_local);
define_hmdfs_lookup_op_end_event(hmdfs_create_local);

define_hmdfs_lookup_op_end_event(hmdfs_lookup_remote);
define_hmdfs_lookup_op_end_event(hmdfs_lookup_remote_end);
define_hmdfs_lookup_op_end_event(hmdfs_mkdir_remote);
define_hmdfs_lookup_op_end_event(hmdfs_rmdir_remote);
define_hmdfs_lookup_op_end_event(hmdfs_create_remote);

define_hmdfs_lookup_op_end_event(hmdfs_lookup_merge);
define_hmdfs_lookup_op_end_event(hmdfs_lookup_merge_end);
define_hmdfs_lookup_op_end_event(hmdfs_mkdir_merge);
define_hmdfs_lookup_op_end_event(hmdfs_rmdir_merge);
define_hmdfs_lookup_op_end_event(hmdfs_create_merge);


define_hmdfs_lookup_op_end_event(hmdfs_symlink_merge);
define_hmdfs_lookup_op_end_event(hmdfs_symlink_local);

define_hmdfs_lookup_op_end_event(hmdfs_get_link_merge);
define_hmdfs_lookup_op_end_event(hmdfs_get_link_local);

TRACE_EVENT(hmdfs_show_comrade,

	TP_PROTO(struct dentry *d, struct dentry *lo_d, uint64_t devid),

	TP_ARGS(d, lo_d, devid),

	TP_STRUCT__entry(
		__string(name, d->d_name.name)
		__string(lo_name, lo_d->d_name.name)
		__field(uint64_t, devid)
	),

	TP_fast_assign(
		__assign_str(name, d->d_name.name)
		__assign_str(lo_name, lo_d->d_name.name)
		__entry->devid = devid;
	),

	TP_printk("parent_name:%s -> lo_d_name:%s, lo_d_devid:%llu",
		  __get_str(name), __get_str(lo_name), __entry->devid)
);

DECLARE_EVENT_CLASS(hmdfs_rename_op_end,

	TP_PROTO(struct inode *olddir, struct dentry *olddentry,
		 struct inode *newdir, struct dentry *newdentry,
		 unsigned int flags),

	TP_ARGS(olddir, olddentry, newdir, newdentry, flags),

	TP_STRUCT__entry(
		__field(ino_t, oldino)
		__string(oldname_str, olddentry->d_name.name)
		__field(ino_t, newino)
		__string(newname_str, newdentry->d_name.name)
		__field(unsigned int, flags)
	),

	TP_fast_assign(
		__entry->oldino = olddir->i_ino;
		__assign_str(oldname_str, olddentry->d_name.name);
		__entry->newino = newdir->i_ino;
		__assign_str(newname_str, newdentry->d_name.name);
		__entry->flags = flags;
	),

	TP_printk("old_pino = %lu, oldname:%s; new_pino = %lu, newname:%s, flags:%u",
		__entry->oldino, __get_str(oldname_str),
		__entry->newino, __get_str(newname_str), __entry->flags)
);

#define define_hmdfs_rename_op_end_event(event_name)                           \
	DEFINE_EVENT(hmdfs_rename_op_end, event_name,                          \
		     TP_PROTO(struct inode *olddir, struct dentry *olddentry,  \
			      struct inode *newdir, struct dentry *newdentry,  \
			      unsigned int flags),                             \
		     TP_ARGS(olddir, olddentry, newdir, newdentry, flags))

define_hmdfs_rename_op_end_event(hmdfs_rename_local);
define_hmdfs_rename_op_end_event(hmdfs_rename_remote);
define_hmdfs_rename_op_end_event(hmdfs_rename_merge);

TRACE_EVENT(hmdfs_statfs,

	TP_PROTO(struct dentry *d, uint8_t type),

	TP_ARGS(d, type),

	TP_STRUCT__entry(
		__string(name, d->d_name.name)
		__field(uint8_t, type)
	),

	TP_fast_assign(
		__assign_str(name, d->d_name.name)
		__entry->type = type;
	),

	TP_printk("dentry_name:%s, lo_d_devid:%u",
		  __get_str(name), __entry->type)
);



TRACE_EVENT(hmdfs_balance_dirty_pages_ratelimited,

	TP_PROTO(struct hmdfs_sb_info *sbi,
		 struct hmdfs_writeback *hwb,
		 int bdp_ratelimits),

	TP_ARGS(sbi, hwb, bdp_ratelimits),

	TP_STRUCT__entry(
		__array(char, dst, 128)
		__field(int, nr_dirtied)
		__field(int, nr_dirtied_pause)
		__field(int, dirty_exceeded)
		__field(long long, bdp_ratelimits)
		__field(long, ratelimit_pages)
	),

	TP_fast_assign(
		    strlcpy(__entry->dst, sbi->local_dst, 128);

		    __entry->nr_dirtied	= current->nr_dirtied;
		    __entry->nr_dirtied_pause = current->nr_dirtied_pause;
		    __entry->dirty_exceeded = hwb->dirty_exceeded;
		    __entry->bdp_ratelimits = bdp_ratelimits;
		    __entry->ratelimit_pages = hwb->ratelimit_pages;
	),

	TP_printk("hmdfs dst:%s nr_dirtied=%d nr_dirtied_pause=%d dirty_exceeded=%d bdp_ratelimits=%lld ratelimit_pages=%ld",
		  __entry->dst, __entry->nr_dirtied, __entry->nr_dirtied_pause,
		  __entry->dirty_exceeded, __entry->bdp_ratelimits,
		  __entry->ratelimit_pages)
);

TRACE_EVENT(hmdfs_balance_dirty_pages,

	    TP_PROTO(struct hmdfs_sb_info *sbi,
		     struct bdi_writeback *wb,
		     struct hmdfs_dirty_throttle_control *hdtc,
		     unsigned long pause,
		     unsigned long start_time),

	    TP_ARGS(sbi, wb, hdtc, pause, start_time),

	    TP_STRUCT__entry(
		    __array(char, dst, 128)
		    __field(unsigned long, write_bw)
		    __field(unsigned long, avg_write_bw)
		    __field(unsigned long, file_bg_thresh)
		    __field(unsigned long, fs_bg_thresh)
		    __field(unsigned long, file_thresh)
		    __field(unsigned long, fs_thresh)
		    __field(unsigned long, file_nr_dirty)
		    __field(unsigned long, fs_nr_dirty)
		    __field(unsigned long, file_nr_rec)
		    __field(unsigned long, fs_nr_rec)
		    __field(unsigned long, pause)
		    __field(unsigned long, paused)
	    ),

	    TP_fast_assign(
		    strlcpy(__entry->dst, sbi->local_dst, 128);

		    __entry->write_bw		= wb->write_bandwidth;
		    __entry->avg_write_bw	= wb->avg_write_bandwidth;
		    __entry->file_bg_thresh	= hdtc->file_bg_thresh;
		    __entry->fs_bg_thresh	= hdtc->fs_bg_thresh;
		    __entry->file_thresh	= hdtc->file_thresh;
		    __entry->fs_thresh		= hdtc->fs_thresh;
		    __entry->file_nr_dirty	= hdtc->file_nr_dirty;
		    __entry->fs_nr_dirty	= hdtc->fs_nr_dirty;
		    __entry->file_nr_rec	= hdtc->file_nr_reclaimable;
		    __entry->fs_nr_rec		= hdtc->fs_nr_reclaimable;
		    __entry->pause		= pause * 1000 / HZ;
		    __entry->paused		= (jiffies - start_time) *
						  1000 / HZ;
	    ),

	    TP_printk("hmdfs dst:%s write_bw=%lu, awrite_bw=%lu, bg_thresh=%lu,%lu thresh=%lu,%lu dirty=%lu,%lu reclaimable=%lu,%lu pause=%lu paused=%lu",
		      __entry->dst, __entry->write_bw, __entry->avg_write_bw,
		      __entry->file_bg_thresh, __entry->fs_bg_thresh,
		      __entry->file_thresh, __entry->fs_thresh,
		      __entry->file_nr_dirty, __entry->fs_nr_dirty,
		      __entry->file_nr_rec, __entry->fs_nr_rec,
		      __entry->pause, __entry->paused
	    )
);

TRACE_EVENT(hmdfs_start_srv_wb,

	    TP_PROTO(struct hmdfs_sb_info *sbi, int dirty_pages,
		    unsigned int dirty_thresh_pg),

	    TP_ARGS(sbi, dirty_pages, dirty_thresh_pg),

	    TP_STRUCT__entry(
		     __array(char, src, 128)
		     __field(int, dirty_pages)
		     __field(unsigned int, dirty_thresh_pg)
	    ),

	    TP_fast_assign(
		    strlcpy(__entry->src, sbi->local_src, 128);
		    __entry->dirty_pages = dirty_pages;
		    __entry->dirty_thresh_pg = dirty_thresh_pg;
	    ),

	    TP_printk("hmdfs src: %s, start writeback dirty pages. writeback %d pages dirty_thresh is %d pages",
		      __entry->src, __entry->dirty_pages, __entry->dirty_thresh_pg)
);

TRACE_EVENT(hmdfs_fsync_enter_remote,

	TP_PROTO(struct hmdfs_sb_info *sbi, unsigned long long device_id,
		 unsigned long long remote_ino, int datasync),

	TP_ARGS(sbi, device_id, remote_ino, datasync),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint64_t, device_id)
		__field(uint64_t, remote_ino)
		__field(int, datasync)
	),

	TP_fast_assign(
		strlcpy(__entry->src, sbi->local_src, 128);
		__entry->device_id = device_id;
		__entry->remote_ino = remote_ino;
		__entry->datasync = datasync;
	),

	TP_printk("hmdfs: src %s, start remote fsync file(remote dev_id=%llu,ino=%llu), datasync=%d",
		__entry->src, __entry->device_id,
		__entry->remote_ino, __entry->datasync)
);

TRACE_EVENT(hmdfs_fsync_exit_remote,

	TP_PROTO(struct hmdfs_sb_info *sbi, unsigned long long device_id,
		 unsigned long long remote_ino, unsigned int timeout, int err),

	TP_ARGS(sbi, device_id, remote_ino, timeout, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint64_t, device_id)
		__field(uint64_t, remote_ino)
		__field(uint32_t, timeout)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, sbi->local_src, 128);
		__entry->device_id = device_id;
		__entry->remote_ino = remote_ino;
		__entry->timeout = timeout;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, finish remote fsync file(remote dev_id=%llu,ino=%llu), timeout=%u, err=%d",
		__entry->src, __entry->device_id, __entry->remote_ino,
		__entry->timeout, __entry->err)
);

TRACE_EVENT(hmdfs_syncfs_enter,

	TP_PROTO(struct hmdfs_sb_info *sbi),

	TP_ARGS(sbi),

	TP_STRUCT__entry(
		__array(char, src, 128)
	),

	TP_fast_assign(
		strlcpy(__entry->src, sbi->local_src, 128);
	),

	TP_printk("hmdfs: src %s, start syncfs", __entry->src)
);

TRACE_EVENT(hmdfs_syncfs_exit,

	TP_PROTO(struct hmdfs_sb_info *sbi, int remain_count,
		 unsigned int timeout, int err),

	TP_ARGS(sbi, remain_count, timeout, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(int, remain_count)
		__field(uint32_t, timeout)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, sbi->local_src, 128);
		__entry->remain_count = remain_count;
		__entry->timeout = timeout;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, finish syncfs(timeout=%u), remain %d remote devices to response, err=%d",
		__entry->src, __entry->timeout,
		__entry->remain_count, __entry->err)
);

TRACE_EVENT(hmdfs_server_release,

	TP_PROTO(struct hmdfs_peer *con, uint32_t file_id,
		uint64_t file_ver, int err),

	TP_ARGS(con, file_id, file_ver, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint32_t, file_id)
		__field(uint64_t, file_ver)
		__field(uint64_t, device_id)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, con->sbi->local_src, 128);
		__entry->file_id = file_id;
		__entry->file_ver = file_ver;
		__entry->device_id = con->device_id;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, server release file, fid=%u, fid_ver=%llu, remote_dev=%llu, err=%d",
		__entry->src, __entry->file_id, __entry->file_ver,
		__entry->device_id, __entry->err)
);

TRACE_EVENT(hmdfs_client_recv_readpage,

	TP_PROTO(struct hmdfs_peer *con, unsigned long long remote_ino,
		unsigned long page_index, int err),

	TP_ARGS(con, remote_ino, page_index, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint64_t, remote_ino)
		__field(unsigned long, page_index)
		__field(uint64_t, device_id)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, con->sbi->local_src, 128);
		__entry->remote_ino = remote_ino;
		__entry->page_index = page_index;
		__entry->device_id = con->device_id;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, client readpage callback from remote device %llu, remote_ino=%llu, page_idx=%lu, err=%d",
		__entry->src, __entry->device_id,
		__entry->remote_ino, __entry->page_index, __entry->err)
);

TRACE_EVENT(hmdfs_writepage_cb_enter,

	TP_PROTO(struct hmdfs_peer *con, unsigned long long remote_ino,
		unsigned long page_index, int err),

	TP_ARGS(con, remote_ino, page_index, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint64_t, remote_ino)
		__field(unsigned long, page_index)
		__field(uint64_t, device_id)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, con->sbi->local_src, 128);
		__entry->remote_ino = remote_ino;
		__entry->page_index = page_index;
		__entry->device_id = con->device_id;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, writepage_cb start, return from remote device %llu, remote_ino=%llu, page_idx=%lu, err=%d",
		__entry->src, __entry->device_id,
		__entry->remote_ino, __entry->page_index, __entry->err)
);

TRACE_EVENT(hmdfs_writepage_cb_exit,

	TP_PROTO(struct hmdfs_peer *con, unsigned long long remote_ino,
		unsigned long page_index, int err),

	TP_ARGS(con, remote_ino, page_index, err),

	TP_STRUCT__entry(
		__array(char, src, 128)
		__field(uint64_t, remote_ino)
		__field(unsigned long, page_index)
		__field(uint64_t, device_id)
		__field(int, err)
	),

	TP_fast_assign(
		strlcpy(__entry->src, con->sbi->local_src, 128);
		__entry->remote_ino = remote_ino;
		__entry->page_index = page_index;
		__entry->device_id = con->device_id;
		__entry->err = err;
	),

	TP_printk("hmdfs: src %s, writepage_cb exit, return from remote device %llu, remote_ino=%llu, page_index=%lu, err=%d",
		__entry->src, __entry->device_id,
		__entry->remote_ino, __entry->page_index, __entry->err)
);

#ifdef CONFIG_HMDFS_1_0

TRACE_EVENT(hmdfs_adapter_remote_readdir,

	TP_PROTO(const char *account, const char *path, int err),

	TP_ARGS(account, path, err),

	TP_STRUCT__entry(
		__string(account_str, account)
		__string(path_str, path)
		__field(int, err)
	),

	TP_fast_assign(
		__assign_str(account_str, account);
		__assign_str(path_str, path);
		__entry->err = err;
	),

	TP_printk("account:%s path:%s, err %d",
		__get_str(account_str), __get_str(path_str), __entry->err)
);

TRACE_EVENT(hmdfs_adapter_remote_lookup,

	TP_PROTO(struct hmdfs_peer *con, const char *dirpath, const char *name,
		 void *ret),

	TP_ARGS(con, dirpath, name, ret),

	TP_STRUCT__entry(
		__string(account_str, con->sbi->local_info.account)
		__field(uint64_t, devid)
		__string(dirpath_str, dirpath)
		__string(name_str, name)
		__field(void *, ret)
	),

	TP_fast_assign(
		__assign_str(account_str, con->sbi->local_info.account);
		__entry->devid = con->iid;
		__assign_str(dirpath_str, dirpath);
		__assign_str(name_str, name);
		__entry->ret = ret;
	),

	TP_printk("account:%s devid:%llu dirpath:%s name:%s, lookup_ret %pK",
		__get_str(account_str), __entry->devid, __get_str(dirpath_str),
		__get_str(name_str),  __entry->ret)
);

DECLARE_EVENT_CLASS(hmdfs_cud_op_end,

	TP_PROTO(const char *account, const char *path, const char *name),

	TP_ARGS(account, path, name),

	TP_STRUCT__entry(
		__string(account_str, account)
		__string(path_str, path)
		__string(name_str, name)
	),

	TP_fast_assign(
		__assign_str(account_str, account);
		__assign_str(path_str, path);
		__assign_str(name_str, name);
	),

	TP_printk("account:%s path:%s name:%s", __get_str(account_str),
			__get_str(path_str), __get_str(name_str))
);

#define define_hmdfs_cup_op_end_event(event_name) DEFINE_EVENT( \
	hmdfs_cud_op_end, event_name, TP_PROTO(const char *account, \
	const char *path, const char *name), TP_ARGS(account, path, name))

define_hmdfs_cup_op_end_event(hmdfs_adapter_create);
define_hmdfs_cup_op_end_event(hmdfs_adapter_update);
define_hmdfs_cup_op_end_event(hmdfs_adapter_delete);

TRACE_EVENT(hmdfs_adapter_rename,

	TP_PROTO(const char *account,
		 const char *old_path, const char *old_name,
		 const char *new_path, const char *new_name, int is_dir),

	TP_ARGS(account, old_path, old_name, new_path, new_name, is_dir),

	TP_STRUCT__entry(
		__string(account_str, account)
		__string(old_path_str, old_path)
		__string(old_name_str, old_name)
		__string(new_path_str, new_path)
		__string(new_name_str, new_name)
		__field(int, is_dir)
	),

	TP_fast_assign(
		__assign_str(account_str, account);
		__assign_str(old_path_str, old_path);
		__assign_str(old_name_str, old_name);
		__assign_str(new_path_str, new_path);
		__assign_str(new_name_str, new_name);
		__entry->is_dir = is_dir;
	),

	TP_printk("account:%s old_path:%s old_name:%s new_path:%s new_name:%s, is_dir=%d",
		__get_str(account_str),
		__get_str(old_path_str), __get_str(old_name_str),
		__get_str(new_path_str), __get_str(new_name_str),
		__entry->is_dir)
);

DECLARE_EVENT_CLASS(hmdfs_adapter_file_id_gen_task,

	TP_PROTO(uint32_t type, const char *path, uint32_t id),

	TP_ARGS(type, path, id),

	TP_STRUCT__entry(
		__field(uint32_t, type)
		__string(path_str, path)
		__field(uint32_t, id)
	),

	TP_fast_assign(
		__entry->type = type;
		__assign_str(path_str, path);
		__entry->id = id;
	),

	TP_printk("type=%u, path:%s, id=%u",
		__entry->type, __get_str(path_str), __entry->id)
);

#define define_hmdfs_adapter_file_id_gen_task_event(event_name) DEFINE_EVENT( \
	hmdfs_adapter_file_id_gen_task, event_name, TP_PROTO(uint32_t type, \
	const char *path, uint32_t id), TP_ARGS(type, path, id))

define_hmdfs_adapter_file_id_gen_task_event(hmdfs_adapter_file_id_gen_add_task);
define_hmdfs_adapter_file_id_gen_task_event(
			hmdfs_adapter_file_id_gen_process_task);

TRACE_EVENT(hmdfs_create_id_symlink,

	TP_PROTO(const char *src, const char *link, int err),

	TP_ARGS(src, link, err),

	TP_STRUCT__entry(
		__string(src_str, src)
		__string(link_str, link)
		__field(int, err)
	),

	TP_fast_assign(
		__assign_str(src_str, src);
		__assign_str(link_str, link);
		__entry->err = err;
	),

	TP_printk("src:%s link:%s, err %d",
		__get_str(src_str), __get_str(link_str), __entry->err)
);

TRACE_EVENT(hmdfs_delete_id_symlink,

	TP_PROTO(const char *link, int err),

	TP_ARGS(link, err),

	TP_STRUCT__entry(
		__string(link_str, link)
		__field(int, err)
	),

	TP_fast_assign(
		__assign_str(link_str, link);
		__entry->err = err;
	),

	TP_printk("link:%s, err %d", __get_str(link_str), __entry->err)
);

#endif

TRACE_EVENT(hmdfs_server_rebuild_dents,

	TP_PROTO(struct hmdfs_dcache_header *__h, int err),

	TP_ARGS(__h, err),

	TP_STRUCT__entry(
		__field(uint64_t, crtime)
		__field(uint64_t, crtime_nsec)
		__field(uint64_t, ctime)
		__field(uint64_t, ctime_nsec)
		__field(uint64_t, num)
		__field(int, err)
	),

	TP_fast_assign(
		__entry->crtime = le64_to_cpu(__h->dcache_crtime);
		__entry->crtime_nsec = le64_to_cpu(__h->dcache_crtime_nsec);
		__entry->ctime = le64_to_cpu(__h->dentry_ctime);
		__entry->ctime_nsec = le64_to_cpu(__h->dentry_ctime_nsec);
		__entry->num = le64_to_cpu(__h->num);
		__entry->err = err;
	),

	TP_printk("dcache crtime %llu:%llu ctime %llu:%llu has %llu dentry err %d",
		__entry->crtime, __entry->crtime_nsec, __entry->ctime,
		__entry->ctime_nsec, __entry->num, __entry->err)
);

TRACE_EVENT(hmdfs_server_readdir,

	TP_PROTO(struct readdir_request *req),

	TP_ARGS(req),

	TP_STRUCT__entry(
		__string(path, req->path)
	),

	TP_fast_assign(
		__assign_str(path, req->path);
	),

	TP_printk("hmdfs_server_readdir %s", __get_str(path))
);

TRACE_EVENT(hmdfs_open_final_remote,

	TP_PROTO(struct hmdfs_inode_info *info,
		 struct hmdfs_open_ret *open_ret,
		 struct file *file,
		 int reason),

	TP_ARGS(info, open_ret, file, reason),

	TP_STRUCT__entry(
		__array(char, file_path, MAX_FILTER_STR_VAL)
		__field(uint32_t, reason)
		__field(uint32_t, file_id)
		__field(uint64_t, file_ver)
		__field(uint64_t, remote_file_size)
		__field(uint64_t, remote_ino)
		__field(uint64_t, remote_ctime)
		__field(uint64_t, remote_ctime_nsec)
		__field(uint64_t, remote_stable_ctime)
		__field(uint64_t, remote_stable_ctime_nsec)
		__field(uint64_t, local_file_size)
		__field(uint64_t, local_ino)
		__field(uint64_t, local_ctime)
		__field(uint64_t, local_ctime_nsec)
		__field(uint64_t, local_stable_ctime)
		__field(uint64_t, local_stable_ctime_nsec)
	),

	TP_fast_assign(
		strlcpy(__entry->file_path, file->f_path.dentry->d_name.name,
			MAX_FILTER_STR_VAL);
		__entry->reason = reason;
		__entry->file_id = open_ret->fid.id;
		__entry->file_ver = open_ret->fid.ver;
		__entry->remote_file_size = open_ret->file_size;
		__entry->remote_ino = open_ret->ino;
		__entry->remote_ctime = open_ret->remote_ctime.tv_sec;
		__entry->remote_ctime_nsec = open_ret->remote_ctime.tv_nsec;
		__entry->remote_stable_ctime = open_ret->stable_ctime.tv_sec;
		__entry->remote_stable_ctime_nsec =
					open_ret->stable_ctime.tv_nsec;
		__entry->local_file_size = info->vfs_inode.i_size;
		__entry->local_ino = info->remote_ino;
		__entry->local_ctime = info->remote_ctime.tv_sec;
		__entry->local_ctime_nsec = info->remote_ctime.tv_nsec;
		__entry->local_stable_ctime = info->stable_ctime.tv_sec;
		__entry->local_stable_ctime_nsec = info->stable_ctime.tv_nsec;
	),

	TP_printk("file path: %s, file id: %u, file ver: %llu, reason: %d, file size: %llu/%llu, ino: %llu/%llu, ctime: %llu.%llu/%llu.%llu, stable_ctime: %llu.%llu/%llu.%llu from remote/local",
		  __entry->file_path, __entry->file_id, __entry->file_ver,
		  __entry->reason, __entry->remote_file_size,
		  __entry->local_file_size, __entry->remote_ino,
		  __entry->local_ino, __entry->remote_ctime,
		  __entry->remote_ctime_nsec, __entry->local_ctime,
		  __entry->local_ctime_nsec, __entry->remote_stable_ctime,
		  __entry->remote_stable_ctime_nsec,
		  __entry->local_stable_ctime, __entry->local_stable_ctime_nsec)
);

TRACE_EVENT(hmdfs_server_open_enter,

	TP_PROTO(struct hmdfs_peer *con,
		 struct open_request *recv),

	TP_ARGS(con, recv),

	TP_STRUCT__entry(
		__array(char, open_path, MAX_FILTER_STR_VAL)
		__array(char, dst_path, MAX_FILTER_STR_VAL)
		__field(uint64_t, device_iid)
		__field(uint32_t, file_type)
	),

	TP_fast_assign(
		strlcpy(__entry->open_path, recv->buf, MAX_FILTER_STR_VAL);
		strlcpy(__entry->dst_path, con->sbi->local_dst,
			MAX_FILTER_STR_VAL);
		__entry->device_iid = con->iid;
		__entry->file_type = recv->file_type;
	),

	TP_printk("server open file %s from %s, file_type is %u, device id is %llu",
		  __entry->open_path, __entry->dst_path,
		  __entry->file_type, __entry->device_iid)
);

TRACE_EVENT(hmdfs_server_open_exit,

	TP_PROTO(struct hmdfs_peer *con,
		 struct open_response *resp,
		 struct file *file,
		 int ret),

	TP_ARGS(con, resp, file, ret),

	TP_STRUCT__entry(
		__array(char, file_path, MAX_FILTER_STR_VAL)
		__array(char, src_path, MAX_FILTER_STR_VAL)
		__field(uint32_t, file_id)
		__field(uint64_t, device_iid)
		__field(uint64_t, file_size)
		__field(uint64_t, ino)
		__field(uint64_t, ctime)
		__field(uint64_t, ctime_nsec)
		__field(uint64_t, stable_ctime)
		__field(uint64_t, stable_ctime_nsec)
		__field(int, retval)
	),

	TP_fast_assign(
		if (file)
			strlcpy(__entry->file_path,
				file->f_path.dentry->d_name.name,
				MAX_FILTER_STR_VAL);
		else
			strlcpy(__entry->file_path, "null", MAX_FILTER_STR_VAL);
		strlcpy(__entry->src_path, con->sbi->local_src,
			MAX_FILTER_STR_VAL);
		__entry->device_iid = con->iid;
		__entry->file_id = resp ? resp->file_id : UINT_MAX;
		__entry->file_size = resp ? resp->file_size : ULLONG_MAX;
		__entry->ino = resp ? resp->ino : 0;
		__entry->ctime = resp ? resp->ctime : 0;
		__entry->ctime_nsec = resp ? resp->ctime_nsec : 0;
		__entry->stable_ctime = resp ? resp->stable_ctime : 0;
		__entry->stable_ctime_nsec = resp ? resp->stable_ctime_nsec : 0;
		__entry->retval = ret;
	),

	TP_printk("server file %s is opened from %s, devid iid: %llu, open result: %d, file id: %u, file size: %llu, ino: %llu, ctime: %llu.%llu, stable ctime: %llu.%llu",
		  __entry->file_path, __entry->src_path,
		  __entry->device_iid, __entry->retval, __entry->file_id,
		  __entry->file_size, __entry->ino, __entry->ctime,
		  __entry->ctime_nsec, __entry->stable_ctime,
		  __entry->stable_ctime_nsec)
);
#endif

#undef TRACE_INCLUDE_PATH
#undef TRACE_INCLUDE_FILE
#define TRACE_INCLUDE_PATH .
#define TRACE_INCLUDE_FILE hmdfs_trace
#include <trace/define_trace.h>
