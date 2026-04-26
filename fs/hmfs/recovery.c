// SPDX-License-Identifier: GPL-2.0
/*
 * fs/f2fs/recovery.c
 *
 * Copyright (c) 2012 Samsung Electronics Co., Ltd.
 *             http://www.samsung.com/
 */
#include <linux/fs.h>
#include <linux/hmfs_fs.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include "hmfs.h"
#include "node.h"
#include "segment.h"

/*
 * Roll forward recovery scenarios.
 *
 * [Term] F: fsync_mark, D: dentry_mark
 *
 * 1. inode(x) | CP | inode(x) | dnode(F)
 * -> Update the latest inode(x).
 *
 * 2. inode(x) | CP | inode(F) | dnode(F)
 * -> No problem.
 *
 * 3. inode(x) | CP | dnode(F) | inode(x)
 * -> Recover to the latest dnode(F), and drop the last inode(x)
 *
 * 4. inode(x) | CP | dnode(F) | inode(F)
 * -> No problem.
 *
 * 5. CP | inode(x) | dnode(F)
 * -> The inode(DF) was missing. Should drop this dnode(F).
 *
 * 6. CP | inode(DF) | dnode(F)
 * -> No problem.
 *
 * 7. CP | dnode(F) | inode(DF)
 * -> If hmfs_iget fails, then goto next to find inode(DF).
 *
 * 8. CP | dnode(F) | inode(x)
 * -> If hmfs_iget fails, then goto next to find inode(DF).
 *    But it will fail due to no inode(DF).
 */

static struct kmem_cache *fsync_entry_slab;

#define MAX_DEV_OOB_LEN		(512)

bool hmfs_is_oob_enable(struct f2fs_sb_info *sbi)
{
	return sbi->oob_enable;
}

void hmfs_set_oob_switch(struct f2fs_sb_info *sbi, bool is_on)
{
	int i;

	hmfs_msg(sbi->sb, KERN_INFO, "%s oob recovery",
			is_on? "Enable": "Disable");
	sbi->oob_enable = is_on;

	for (i = 0; i < NR_CURSEG_TYPE; ++i)
		atomic_set(&sbi->oob_wr_cnt[i], 0);

	sbi->skip_sync_node = false;
}

static block_t get_recover_ext(struct f2fs_sb_info *sbi,
		struct recover_ext_info *ext,
		struct stor_dev_pwron_info *stor_info)
{
	unsigned start_sec, end_sec;
	block_t start_lba, end_lba;
	block_t next_start_lba;

	/* no oob to recover */
	f2fs_bug_on(sbi, ext->start_lba == ext->end_lba);

	start_lba = ext->start_lba;
	/* lba may exceed MAX_BLKADDR, so need minus 1 */
	end_lba = ext->end_lba - 1;

	start_sec = GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, start_lba));
	end_sec = GET_SEC_FROM_SEG(sbi, GET_SEGNO(sbi, end_lba));

	if (start_sec == end_sec) {
		ext->end_lba = end_lba;
		next_start_lba = NULL_ADDR;
		goto out;
	}

	ext->end_lba = START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi, start_sec)) +
		BLKS_PER_SEC(sbi) - 1;
	next_start_lba = START_BLOCK(sbi, GET_SEG_FROM_SEC(sbi, end_sec));

out:
	ext->rsvd_space = false;
	ext->len = ext->end_lba - ext->start_lba + 1;

	/*
	 * When the whole section need oob recovery,
	 * it should be reserved to avoid alloc by the flow below:
	 * sync write address with UFS when mount filesystem.
	 */
	if (ext->len == sbi->blocks_per_seg * sbi->segs_per_sec) {
		ext->rsvd_space = true;

		f2fs_bug_on(sbi, !IS_FIRST_DATA_BLOCK_IN_SEC(sbi, start_lba));
		__set_inuse(sbi, GET_SEGNO(sbi, start_lba));
	}

	return next_start_lba;
}

void hmfs_log_oob_extent(struct f2fs_sb_info *sbi,
		struct stor_dev_pwron_info *stor_info)
{
	int i, j;
	struct curseg_info *curseg;
	block_t cp_blkaddr, dev_blkaddr;
	struct recover_ext_info ext;

	if (!hmfs_is_oob_enable(sbi))
		return;

	/* need log node stream info */
	for (i = STREAM_META + 1; i < STREAM_NR; i++) {
		sbi->oob_ctrl[i].oob_info = NULL;
		sbi->oob_ctrl[i].oob_cnt = 0;
		sbi->oob_ctrl[i].recover_ext_cnt = 0;

		if (stor_info->dev_stream_addr[i] != 0) {
			curseg = CURSEG_I(sbi, CURSEG_T(i));
			cp_blkaddr = START_BLOCK(sbi, curseg->segno) +
				curseg->next_blkoff;
			dev_blkaddr = stor_info->dev_stream_addr[i];

			if (dev_blkaddr == cp_blkaddr) {
				hmfs_msg(sbi->sb, KERN_DEBUG,
						"oob info: stream[%d-%u] no need to oob recover[%u,%u]",
						i, dev_blkaddr, curseg->segno, curseg->next_blkoff);
				continue;
			}

			for (j = 0; j < MAX_RECOVER_EXT_CNT; ++j) {
				ext.start_lba = cp_blkaddr;
				ext.end_lba = dev_blkaddr;
				cp_blkaddr = get_recover_ext(sbi, &ext, stor_info);

				sbi->oob_ctrl[i].recover_ext_cnt++;
				sbi->oob_ctrl[i].recover_ext[j] = ext;

				if (cp_blkaddr == NULL_ADDR)
					break;
			}
			hmfs_msg(sbi->sb, KERN_INFO,
					"oob info: stream[%d-%u-%d] start[%u,%u] end[%u,%u]",
					i, dev_blkaddr, sbi->oob_ctrl[i].recover_ext_cnt,
					curseg->segno, curseg->next_blkoff,
					GET_SEGNO(sbi, dev_blkaddr - 1),
					GET_BLKOFF_FROM_SEG0(sbi, dev_blkaddr - 1));

			f2fs_bug_on(sbi, cp_blkaddr != NULL_ADDR);
		}
	}
}

static int __calc_oob_len(struct f2fs_sb_info *sbi, int stream_id)
{
	int i, len;
	struct hmfs_oob_control_info *oob_ctrl = &(sbi->oob_ctrl[stream_id]);

	for (i = 0, len = 0; i < oob_ctrl->recover_ext_cnt; ++i) {
		len += oob_ctrl->recover_ext[i].len;
	}

	return len;
}

static int __do_get_oob(struct f2fs_sb_info *sbi,
		int stream_id, unsigned start_lba, int total_len,
		struct stor_dev_stream_oob_info *oob_info)
{
	int err = 0;
	int len;
	struct stor_dev_stream_info stream_info;

	stream_info.stream_id = stream_id;
	stream_info.dm_stream = false;  /* no need recovery oob of dm */

	while (total_len > 0) {
		stream_info.stream_start_lba = start_lba;

		len = min(MAX_DEV_OOB_LEN, total_len);
		err = mas_blk_stream_oob_info_fetch(sbi->sb->s_bdev,
				stream_info, len, oob_info);
		if (unlikely(err)) {
			hmfs_msg(sbi->sb, KERN_ERR,
					"stream[%u]: fetch oob fail[%d]", stream_id, err);
			break;
		}

		start_lba += len;
		oob_info += len;
		total_len -= len;
	}

	return err;
}

static int __get_oob_info(struct f2fs_sb_info *sbi, int stream_id)
{
	int ret, i, len;
	size_t oob_info_size;
	struct recover_ext_info *ext;
	struct stor_dev_stream_oob_info *oob_info;

	if (sbi->oob_ctrl[stream_id].recover_ext_cnt == 0) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"stream%d: no recover extent to get oob", stream_id);
		return -EFAULT;
	}

	len = __calc_oob_len(sbi, stream_id);
	f2fs_bug_on(sbi, len == 0);

	oob_info_size = sizeof(struct stor_dev_stream_oob_info) * len;
	oob_info = f2fs_kvmalloc(sbi, oob_info_size, GFP_KERNEL);
	if (!oob_info) {
		hmfs_msg(sbi->sb, KERN_ERR,
				"stream%d: no memory to alloc oob info", stream_id);
		return -ENOMEM;
	}

	sbi->oob_ctrl[stream_id].oob_info = oob_info;
	sbi->oob_ctrl[stream_id].oob_cnt = len;

	for (i = 0; i < sbi->oob_ctrl[stream_id].recover_ext_cnt; ++i) {
		ext = &(sbi->oob_ctrl[stream_id].recover_ext[i]);
		hmfs_msg(sbi->sb, KERN_INFO,
				"get stream%d's %d section oob: %u->%u, len %u",
				stream_id, i + 1,
				ext->start_lba, ext->end_lba, ext->len);

		ret = __do_get_oob(sbi, stream_id,
				ext->start_lba, ext->len, oob_info);
		if (ret)
			goto oob_err;

		oob_info += ext->len;
	}

	return 0;

oob_err:
	kvfree(sbi->oob_ctrl[stream_id].oob_info);
	sbi->oob_ctrl[stream_id].oob_info = NULL;
	sbi->oob_ctrl[stream_id].oob_cnt = 0;
	return ret;
}

static bool __is_need_oob_stream(struct f2fs_sb_info *sbi,
		int stream_id, bool is_dm)
{
	if (is_dm) {
		/* no need recovery oob of dm stream */
		return false;
	}

	if (stream_id == STREAM_HOT_DATA || stream_id == STREAM_COLD_DATA) {
		/* only get oob info of data stream;  */
		return true;
	}

	return false;
}

static int get_oob_info(struct f2fs_sb_info *sbi)
{
	int i;
	int err = 0;

	for (i = STREAM_META + 1; i < STREAM_NR; ++i) {
		if (!__is_need_oob_stream(sbi, i, false))
			continue;

		if (sbi->oob_ctrl[i].recover_ext_cnt) {
			err = __get_oob_info(sbi, i);
			if (err)
				break;
		}
	}
	return err;
}

static void clean_oob_info(struct f2fs_sb_info *sbi)
{
	int i;

	for (i = STREAM_META + 1; i < STREAM_NR; ++i) {
		if (sbi->oob_ctrl[i].oob_info != NULL) {
			kvfree(sbi->oob_ctrl[i].oob_info);
			sbi->oob_ctrl[i].oob_info = NULL;
		}
	}
}

void hmfs_free_oob_rsvd_space(struct f2fs_sb_info *sbi)
{
	int i, j;
	block_t start_lba;

	for (i = STREAM_META + 1; i < STREAM_NR; ++i) {
		for (j = 0; j < sbi->oob_ctrl[i].recover_ext_cnt; ++j) {
			if (!sbi->oob_ctrl[i].recover_ext[j].rsvd_space)
				continue;

			sbi->oob_ctrl[i].recover_ext[j].rsvd_space = false;

			start_lba = sbi->oob_ctrl[i].recover_ext[j].start_lba;
			__set_free(sbi, start_lba);
		}
	}
}

bool hmfs_space_for_roll_forward(struct f2fs_sb_info *sbi)
{
	s64 nalloc = percpu_counter_sum_positive(&sbi->alloc_valid_block_count);

	if (sbi->last_valid_block_count + nalloc > sbi->user_block_count)
		return false;
	return true;
}

static struct fsync_inode_entry *get_fsync_inode(struct list_head *head,
								nid_t ino)
{
	struct fsync_inode_entry *entry;

	list_for_each_entry(entry, head, list)
		if (entry->inode->i_ino == ino)
			return entry;

	return NULL;
}

static struct fsync_inode_entry *add_fsync_inode(struct f2fs_sb_info *sbi,
			struct list_head *head, nid_t ino, bool quota_inode)
{
	struct inode *inode;
	struct fsync_inode_entry *entry;
	int err;

	inode = hmfs_iget_retry(sbi->sb, ino);
	if (IS_ERR(inode))
		return ERR_CAST(inode);

	err = f2fs_dquot_initialize(inode);
	if (err)
		goto err_out;

	if (quota_inode) {
		err = f2fs_dquot_alloc_inode(inode);
		if (err)
			goto err_out;
	}

	entry = f2fs_kmem_cache_alloc(fsync_entry_slab, GFP_F2FS_ZERO);
	entry->inode = inode;
	list_add_tail(&entry->list, head);

	return entry;
err_out:
	iput(inode);
	return ERR_PTR(err);
}

static void del_fsync_inode(struct fsync_inode_entry *entry, int drop)
{
	if (drop) {
		/* inode should not be recovered, drop it */
		hmfs_inode_synced(entry->inode);
	}
	iput(entry->inode);
	list_del(&entry->list);
	kmem_cache_free(fsync_entry_slab, entry);
}

static void destroy_fsync_dnodes(struct list_head *head, int drop)
{
	struct fsync_inode_entry *entry, *tmp;

	list_for_each_entry_safe(entry, tmp, head, list)
		del_fsync_inode(entry, drop);
}

static int recover_dentry(struct inode *inode, struct page *ipage,
						struct list_head *dir_list)
{
	struct f2fs_inode *raw_inode = F2FS_INODE(ipage);
	nid_t pino = le32_to_cpu(raw_inode->i_pino);
	struct f2fs_dir_entry *de;
	struct fscrypt_name fname;
	struct page *page;
	struct inode *dir, *einode;
	struct fsync_inode_entry *entry;
	int err = 0;
	char *name;

	entry = get_fsync_inode(dir_list, pino);
	if (!entry) {
		entry = add_fsync_inode(F2FS_I_SB(inode), dir_list,
							pino, false);
		if (IS_ERR(entry)) {
			dir = ERR_CAST(entry);
			err = PTR_ERR(entry);
			goto out;
		}
	}

	dir = entry->inode;

	memset(&fname, 0, sizeof(struct fscrypt_name));
	fname.disk_name.len = le32_to_cpu(raw_inode->i_namelen);
	fname.disk_name.name = raw_inode->i_name;

	if (unlikely(fname.disk_name.len > F2FS_NAME_LEN)) {
		WARN_ON(1);
		err = -ENAMETOOLONG;
		goto out;
	}
retry:
	de = __hmfs_find_entry(dir, &fname, &page);
	if (de && inode->i_ino == le32_to_cpu(de->ino))
		goto out_put;

	if (de) {
		einode = hmfs_iget_retry(inode->i_sb, le32_to_cpu(de->ino));
		if (IS_ERR(einode)) {
			WARN_ON(1);
			err = PTR_ERR(einode);
			if (err == -ENOENT)
				err = -EEXIST;
			goto out_put;
		}

		err = f2fs_dquot_initialize(einode);
		if (err) {
			iput(einode);
			goto out_put;
		}

		err = hmfs_acquire_orphan_inode(F2FS_I_SB(inode));
		if (err) {
			iput(einode);
			goto out_put;
		}
		hmfs_delete_entry(de, page, dir, einode);
		iput(einode);
		goto retry;
	} else if (IS_ERR(page)) {
		err = PTR_ERR(page);
	} else {
		err = hmfs_add_dentry(dir, &fname, NULL, inode,
					inode->i_ino, inode->i_mode);
	}
	if (err == -ENOMEM)
		goto retry;
	goto out;

out_put:
	f2fs_put_page(page, 0);
out:
	if (file_enc_name(inode))
		name = "<encrypted>";
	else
		name = raw_inode->i_name;
	hmfs_msg(inode->i_sb, KERN_NOTICE,
			"%s: ino = %x, name = %s, dir = %lx, err = %d",
			__func__, ino_of_node(ipage), name,
			IS_ERR(dir) ? 0 : dir->i_ino, err);
	return err;
}

static int recover_quota_data(struct inode *inode, struct page *page)
{
	struct f2fs_inode *raw = F2FS_INODE(page);
	struct iattr attr;
	uid_t i_uid = le32_to_cpu(raw->i_uid);
	gid_t i_gid = le32_to_cpu(raw->i_gid);
	int err;

	memset(&attr, 0, sizeof(attr));

	attr.ia_uid = make_kuid(inode->i_sb->s_user_ns, i_uid);
	attr.ia_gid = make_kgid(inode->i_sb->s_user_ns, i_gid);

	if (!uid_eq(attr.ia_uid, inode->i_uid))
		attr.ia_valid |= ATTR_UID;
	if (!gid_eq(attr.ia_gid, inode->i_gid))
		attr.ia_valid |= ATTR_GID;

	if (!attr.ia_valid)
		return 0;

	err = dquot_transfer(inode, &attr);
	if (err)
		set_sbi_flag(F2FS_I_SB(inode), SBI_QUOTA_NEED_REPAIR);
	return err;
}

static void recover_inline_flags(struct inode *inode, struct f2fs_inode *ri)
{
	if (ri->i_inline & F2FS_PIN_FILE)
		set_inode_flag(inode, FI_PIN_FILE);
	else
		clear_inode_flag(inode, FI_PIN_FILE);
	if (ri->i_inline & F2FS_DATA_EXIST)
		set_inode_flag(inode, FI_DATA_EXIST);
	else
		clear_inode_flag(inode, FI_DATA_EXIST);
}

static int recover_inode(struct inode *inode, struct page *page)
{
	struct f2fs_inode *raw = F2FS_INODE(page);
	char *name;
	int err;

	inode->i_mode = le16_to_cpu(raw->i_mode);

	err = recover_quota_data(inode, page);
	if (err)
		return err;

	i_uid_write(inode, le32_to_cpu(raw->i_uid));
	i_gid_write(inode, le32_to_cpu(raw->i_gid));

	if (raw->i_inline & F2FS_EXTRA_ATTR) {
		if (f2fs_sb_has_project_quota(F2FS_I_SB(inode)->sb) &&
			F2FS_FITS_IN_INODE(raw, le16_to_cpu(raw->i_extra_isize),
								i_projid)) {
			projid_t i_projid;
			kprojid_t kprojid;

			i_projid = (projid_t)le32_to_cpu(raw->i_projid);
			kprojid = make_kprojid(&init_user_ns, i_projid);

			if (!projid_eq(kprojid, F2FS_I(inode)->i_projid)) {
				err = hmfs_transfer_project_quota(inode,
								kprojid);
				if (err)
					return err;
				F2FS_I(inode)->i_projid = kprojid;
			}
		}
	}

	f2fs_i_size_write(inode, le64_to_cpu(raw->i_size));
	inode->i_atime.tv_sec = le64_to_cpu(raw->i_atime);
	inode->i_ctime.tv_sec = le64_to_cpu(raw->i_ctime);
	inode->i_mtime.tv_sec = le64_to_cpu(raw->i_mtime);
	inode->i_atime.tv_nsec = le32_to_cpu(raw->i_atime_nsec);
	inode->i_ctime.tv_nsec = le32_to_cpu(raw->i_ctime_nsec);
	inode->i_mtime.tv_nsec = le32_to_cpu(raw->i_mtime_nsec);

	F2FS_I(inode)->i_advise = raw->i_advise;
	F2FS_I(inode)->i_flags = le32_to_cpu(raw->i_flags);
	hmfs_set_inode_flags(inode);
	F2FS_I(inode)->i_gc_failures[GC_FAILURE_PIN] =
				le16_to_cpu(raw->i_gc_failures);

	recover_inline_flags(inode, raw);

	hmfs_mark_inode_dirty_sync(inode, true);

	if (file_enc_name(inode))
		name = "<encrypted>";
	else
		name = F2FS_INODE(page)->i_name;

	hmfs_msg(inode->i_sb, KERN_NOTICE,
		"recover_inode: ino = %x, name = %s, inline = %x",
			ino_of_node(page), name, raw->i_inline);
	return 0;
}

static int find_fsync_dnodes(struct f2fs_sb_info *sbi, struct list_head *head,
				bool check_only)
{
	struct page *page = NULL;
	block_t blkaddr;
	unsigned int loop_cnt = 0;
	unsigned int free_blocks = MAIN_SEGS(sbi) * sbi->blocks_per_seg -
						valid_user_blocks(sbi);
	int err = 0;
	int type;
	struct curseg_info *curseg;

	if (hmfs_is_oob_enable(sbi)) {
		type = STREAM_HOT_NODE;

		/* get node pages in the current segment */
		if (sbi->oob_ctrl[type].recover_ext_cnt)
			blkaddr = sbi->oob_ctrl[type].recover_ext[0].start_lba;
		else
			blkaddr = NULL_ADDR;
		hmfs_msg(sbi->sb, KERN_INFO,
			"hot node: cp_blkaddr %llu, dev_blkaddr %llu",
			blkaddr, sbi->oob_ctrl[type].recover_ext[0].end_lba);
	} else {
		type = (F2FS_OPTION(sbi).active_logs == 4) ? CURSEG_COLD_NODE :
			CURSEG_WARM_NODE;

		/* get node pages in the current segment */
		curseg = CURSEG_I(sbi, type);
		blkaddr = NEXT_FREE_BLKADDR(sbi, curseg);
	}

	while (1) {
		struct fsync_inode_entry *entry;

		if (!hmfs_is_valid_blkaddr(sbi, blkaddr, META_POR))
			return 0;

		page = hmfs_get_tmp_page(sbi, blkaddr);
		if (IS_ERR(page)) {
			err = PTR_ERR(page);
			break;
		}

		if (PageChecked(page)) {
			hmfs_msg(sbi->sb, KERN_ERR, "Abandon looped node block list");
			destroy_fsync_dnodes(head, 1);
			break;
		}

		/*
		 * it's not needed to clear PG_checked flag in temp page since we
		 * will truncate all those pages in the end of recovery.
		 */
		SetPageChecked(page);

		if (!is_recoverable_dnode(page))
			break;

		if (!is_fsync_dnode(page))
			goto next;

		entry = get_fsync_inode(head, ino_of_node(page));
		if (!entry) {
			hmfs_msg(sbi->sb, KERN_INFO,
				"find node[%d, %d, %d] to recover",
				ino_of_node(page), nid_of_node(page),
				is_dent_dnode(page));
			bool quota_inode = false;

			if (!check_only &&
				IS_INODE(page) && is_dent_dnode(page)) {
				err = hmfs_recover_inode_page(sbi, page);
				if (err)
					break;
				quota_inode = true;
			}

			/*
			 * CP | dnode(F) | inode(DF)
			 * For this case, we should not give up now.
			 */
			entry = add_fsync_inode(sbi, head, ino_of_node(page),
						quota_inode);
			if (IS_ERR(entry)) {
				err = PTR_ERR(entry);
				if (err == -ENOENT) {
					err = 0;
					goto next;
				}
				break;
			}
		}
		entry->blkaddr = blkaddr;

		if (IS_INODE(page) && is_dent_dnode(page))
			entry->last_dentry = blkaddr;
next:
		/* sanity check in order to detect looped node chain */
		if (++loop_cnt >= free_blocks ||
			blkaddr == next_blkaddr_of_node(page)) {
			hmfs_msg(sbi->sb, KERN_NOTICE,
				"%s: detect looped node chain, "
				"blkaddr:%u, next:%u",
				__func__, blkaddr, next_blkaddr_of_node(page));
			err = -EINVAL;
			break;
		}

		/* check next segment */
		blkaddr = next_blkaddr_of_node(page);
		f2fs_put_page(page, 1);

		hmfs_ra_meta_pages_cond(sbi, blkaddr);
	}
	f2fs_put_page(page, 1);
	return err;
}

static int check_index_in_prev_nodes(struct f2fs_sb_info *sbi,
			block_t blkaddr, struct dnode_of_data *dn)
{
	struct seg_entry *sentry;
	unsigned int segno = GET_SEGNO(sbi, blkaddr);
	unsigned short blkoff = GET_BLKOFF_FROM_SEG0(sbi, blkaddr);
	struct f2fs_summary_block *sum_node;
	struct f2fs_summary sum;
	struct page *sum_page, *node_page;
	struct dnode_of_data tdn = *dn;
	nid_t ino, nid;
	struct inode *inode;
	unsigned int offset;
	block_t bidx;
	int i;

	sentry = get_seg_entry(sbi, segno);
	if (!f2fs_test_bit(blkoff, sentry->cur_valid_map))
		return 0;

	/* Get the previous summary */
	for (i = CURSEG_HOT_DATA; i <= CURSEG_COLD_DATA; i++) {
		struct curseg_info *curseg = CURSEG_I(sbi, i);
		if (curseg->segno == segno) {
			sum = curseg->sum_blk->entries[blkoff];
			goto got_it;
		}
	}

	sum_page = hmfs_get_sum_page(sbi, segno);
	if (IS_ERR(sum_page))
		return PTR_ERR(sum_page);
	sum_node = (struct f2fs_summary_block *)page_address(sum_page);
	sum = sum_node->entries[blkoff];
	f2fs_put_page(sum_page, 1);
got_it:
	/* Use the locked dnode page and inode */
	nid = le32_to_cpu(sum.nid);
	if (dn->inode->i_ino == nid) {
		tdn.nid = nid;
		if (!dn->inode_page_locked)
			lock_page(dn->inode_page);
		tdn.node_page = dn->inode_page;
		tdn.ofs_in_node = le16_to_cpu(sum.ofs_in_node);
		goto truncate_out;
	} else if (dn->nid == nid) {
		tdn.ofs_in_node = le16_to_cpu(sum.ofs_in_node);
		goto truncate_out;
	}

	/* Get the node page */
	node_page = hmfs_get_node_page(sbi, nid);
	if (IS_ERR(node_page))
		return PTR_ERR(node_page);

	offset = ofs_of_node(node_page);
	ino = ino_of_node(node_page);
	f2fs_put_page(node_page, 1);

	if (ino != dn->inode->i_ino) {
		int ret;

		/* Deallocate previous index in the node page */
		inode = hmfs_iget_retry(sbi->sb, ino);
		if (IS_ERR(inode))
			return PTR_ERR(inode);

		ret = f2fs_dquot_initialize(inode);
		if (ret) {
			iput(inode);
			return ret;
		}
	} else {
		inode = dn->inode;
	}

	bidx = hmfs_start_bidx_of_node(offset, inode) +
				le16_to_cpu(sum.ofs_in_node);

	/*
	 * if inode page is locked, unlock temporarily, but its reference
	 * count keeps alive.
	 */
	if (ino == dn->inode->i_ino && dn->inode_page_locked)
		unlock_page(dn->inode_page);

	set_new_dnode(&tdn, inode, NULL, NULL, 0);
	if (hmfs_get_dnode_of_data(&tdn, bidx, LOOKUP_NODE))
		goto out;

	if (tdn.data_blkaddr == blkaddr)
		hmfs_truncate_data_blocks_range(&tdn, 1);

	f2fs_put_dnode(&tdn);
out:
	if (ino != dn->inode->i_ino)
		iput(inode);
	else if (dn->inode_page_locked)
		lock_page(dn->inode_page);
	return 0;

truncate_out:
	if (datablock_addr(tdn.inode, tdn.node_page,
					tdn.ofs_in_node) == blkaddr)
		hmfs_truncate_data_blocks_range(&tdn, 1);
	if (dn->inode->i_ino == nid && !dn->inode_page_locked)
		unlock_page(dn->inode_page);
	return 0;
}

static int do_recover_data(struct f2fs_sb_info *sbi, struct inode *inode,
					struct page *page)
{
	struct dnode_of_data dn;
	struct node_info ni;
	unsigned int start, end;
	int err = 0, recovered = 0;

	/* step 1: recover xattr */
	if (IS_INODE(page)) {
		hmfs_recover_inline_xattr(inode, page);
	} else if (f2fs_has_xattr_block(ofs_of_node(page))) {
		err = hmfs_recover_xattr_data(inode, page);
		if (!err)
			recovered++;
		goto out;
	}

	/* step 2: recover inline data */
	if (hmfs_recover_inline_data(inode, page))
		goto out;

	/* step 3: recover data indices */
	start = hmfs_start_bidx_of_node(ofs_of_node(page), inode);
	end = start + ADDRS_PER_PAGE(page, inode);

	set_new_dnode(&dn, inode, NULL, NULL, 0);
retry_dn:
	err = hmfs_get_dnode_of_data(&dn, start, ALLOC_NODE);
	if (err) {
		if (err == -ENOMEM) {
			congestion_wait(BLK_RW_ASYNC, HZ/50);
			goto retry_dn;
		}
		goto out;
	}

	hmfs_wait_on_page_writeback(dn.node_page, NODE, true);

	err = f2fs_get_node_info(sbi, dn.nid, &ni);
	if (err)
		goto err;

	f2fs_bug_on(sbi, ni.ino != ino_of_node(page));
	f2fs_bug_on(sbi, ofs_of_node(dn.node_page) != ofs_of_node(page));

	for (; start < end; start++, dn.ofs_in_node++) {
		block_t src, dest;

		src = datablock_addr(dn.inode, dn.node_page, dn.ofs_in_node);
		dest = datablock_addr(dn.inode, page, dn.ofs_in_node);

		/* skip recovering if dest is the same as src */
		if (src == dest)
			continue;

		/* dest is invalid, just invalidate src block */
		if (dest == NULL_ADDR) {
			hmfs_truncate_data_blocks_range(&dn, 1);
			continue;
		}

		if (!file_keep_isize(inode) &&
			(i_size_read(inode) <= ((loff_t)start << PAGE_SHIFT)))
			f2fs_i_size_write(inode,
				(loff_t)(start + 1) << PAGE_SHIFT);

		/*
		 * dest is reserved block, invalidate src block
		 * and then reserve one new block in dnode page.
		 */
		if (dest == NEW_ADDR) {
			hmfs_truncate_data_blocks_range(&dn, 1);
			hmfs_reserve_new_block(&dn);
			continue;
		}

		/* dest is valid block, try to recover from src to dest */
		if (hmfs_is_valid_blkaddr(sbi, dest, META_POR)) {

			if (src == NULL_ADDR) {
				err = hmfs_reserve_new_block(&dn);
				while (err &&
				       IS_ENABLED(CONFIG_HMFS_FAULT_INJECTION))
					err = hmfs_reserve_new_block(&dn);
				/* We should not get -ENOSPC */
				f2fs_bug_on(sbi, err);
				if (err)
					goto err;
			}
retry_prev:
			/* Check the previous node page having this index */
			err = check_index_in_prev_nodes(sbi, dest, &dn);
			if (err) {
				if (err == -ENOMEM) {
					congestion_wait(BLK_RW_ASYNC, HZ/50);
					goto retry_prev;
				}
				goto err;
			}

			/* write dummy data page */
			if (hmfs_is_oob_enable(sbi))
				hmfs_replace_block(sbi, &dn, src, dest,
						ni.version, true, true, true);
			else
				hmfs_replace_block(sbi, &dn, src, dest,
						ni.version, false, false, false);
			recovered++;
		}
	}

	copy_node_footer(dn.node_page, page);
	fill_node_footer(dn.node_page, dn.nid, ni.ino,
					ofs_of_node(page), false);
	set_page_dirty(dn.node_page);
err:
	f2fs_put_dnode(&dn);
out:
	hmfs_msg(sbi->sb, KERN_NOTICE,
		"recover_data: ino = %lx (i_size: %s) recovered = %d, err = %d",
		inode->i_ino, file_keep_isize(inode) ? "keep" : "recover",
		recovered, err);
	return err;
}

static int recover_data(struct f2fs_sb_info *sbi, struct list_head *inode_list,
		struct list_head *tmp_inode_list, struct list_head *dir_list)
{
	struct page *page = NULL;
	int err = 0;
	int type;
	block_t blkaddr;
	struct curseg_info *curseg;

	if (hmfs_is_oob_enable(sbi)) {
		type = STREAM_HOT_NODE;

		/* get node pages in the current segment */
		if (sbi->oob_ctrl[type].recover_ext_cnt)
			blkaddr = sbi->oob_ctrl[type].recover_ext[0].start_lba;
		else
			blkaddr = NULL_ADDR;
	} else {
		type = (F2FS_OPTION(sbi).active_logs == 4) ? CURSEG_HOT_NODE :
			CURSEG_WARM_NODE;

		/* get node pages in the current segment */
		curseg = CURSEG_I(sbi, type);
		blkaddr = NEXT_FREE_BLKADDR(sbi, curseg);
	}

	while (1) {
		struct fsync_inode_entry *entry;

		if (!hmfs_is_valid_blkaddr(sbi, blkaddr, META_POR))
			break;

		hmfs_ra_meta_pages_cond(sbi, blkaddr);

		page = hmfs_get_tmp_page(sbi, blkaddr);
		if (IS_ERR(page)) {
			err = PTR_ERR(page);
			break;
		}

		if (!is_recoverable_dnode(page)) {
			f2fs_put_page(page, 1);
			break;
		}

		entry = get_fsync_inode(inode_list, ino_of_node(page));
		if (!entry)
			goto next;
		/*
		 * inode(x) | CP | inode(x) | dnode(F)
		 * In this case, we can lose the latest inode(x).
		 * So, call recover_inode for the inode update.
		 */
		if (IS_INODE(page)) {
			err = recover_inode(entry->inode, page);
			if (err)
				break;
		}
		if (entry->last_dentry == blkaddr) {
			err = recover_dentry(entry->inode, page, dir_list);
			if (err) {
				f2fs_put_page(page, 1);
				break;
			}
		}
		err = do_recover_data(sbi, entry->inode, page);
		if (err) {
			f2fs_put_page(page, 1);
			break;
		}

		if (entry->blkaddr == blkaddr)
			list_move_tail(&entry->list, tmp_inode_list);
next:
		/* check next segment */
		blkaddr = next_blkaddr_of_node(page);
		f2fs_put_page(page, 1);
	}
	if (!err && !IS_MULTI_SEGS_IN_SEC(sbi))
		hmfs_allocate_new_segments(sbi);
	return err;
}

static int find_fsync_oobs(struct f2fs_sb_info *sbi, struct list_head *head)
{
	int i, j;
	int err = 0;

	/* scan oob info in hot/cold data segment */
	for (i = STREAM_META + 1; i < STREAM_NR; ++i) {
		if (!__is_need_oob_stream(sbi, i, false))
			continue;

		for (j = 0; j < sbi->oob_ctrl[i].oob_cnt; ++j) {
			struct fsync_inode_entry *entry;
			unsigned int ino, offset;

			ino = sbi->oob_ctrl[i].oob_info[j].data_ino;
			offset = sbi->oob_ctrl[i].oob_info[j].data_idx;

			if (!ino) {
				static bool print_once = false;
				if (!print_once) {
					print_once = true;
					hmfs_msg(sbi->sb, KERN_INFO,
							"ino is 0 at stream%d index %u",
							i, j);
				}
			}

			if ((offset & OOB_FSYNC_BIT) == 0)
				continue;

			entry = get_fsync_inode(head, ino);
			if (!entry) {
				hmfs_msg(sbi->sb, KERN_INFO,
						"find data[%d, %d, %d] to recover",
						ino, offset & (~OOB_FSYNC_BIT),
						(offset & OOB_FSYNC_BIT) != 0);

				entry = add_fsync_inode(sbi, head, ino, false);
				if (IS_ERR(entry)) {
					err = PTR_ERR(entry);
					hmfs_msg(sbi->sb, KERN_NOTICE,
						"add ino[%u] fail[%d]", ino, err);
					break;
				}
			}
		}
	}
	return err;
}

static int convert_inline_inode_for_oob(struct inode *inode)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct page *ipage = NULL;
	int err = 0;

	if (!f2fs_has_inline_data(inode))
		return 0;

	ipage = hmfs_get_node_page(sbi, inode->i_ino);
	if (IS_ERR(ipage)) {
		err = PTR_ERR(ipage);
		goto out;
	}

	if (!f2fs_exist_data(inode))
		goto clear_out;

	/* clear inline data and flag */
	hmfs_truncate_inline_inode(inode, ipage, 0);
	clear_inline_node(ipage);
clear_out:
	stat_dec_inline_inode(inode);
	clear_inode_flag(inode, FI_INLINE_DATA);
	hmfs_msg(sbi->sb, KERN_INFO,
			"convert inline ino[%u]", inode->i_ino);

	f2fs_put_page(ipage, 1);
out:
	return err;
}

static int do_recover_ftl(struct f2fs_sb_info *sbi,
		struct inode *inode, unsigned int index,
		block_t blkaddr)
{
	struct dnode_of_data dn;
	struct node_info ni;
	unsigned int start = index;
	int err = 0, recovered = 0;
	block_t src, dest;
	unsigned int ino = inode->i_ino;

	err = convert_inline_inode_for_oob(inode);
	if (err)
		goto out;

	set_new_dnode(&dn, inode, NULL, NULL, 0);
retry_dn:
	err = hmfs_get_dnode_of_data(&dn, start, ALLOC_NODE);
	if (err) {
		if (err == -ENOMEM) {
			congestion_wait(BLK_RW_ASYNC, HZ/50);
			goto retry_dn;
		}
		goto out;
	}

	hmfs_wait_on_page_writeback(dn.node_page, NODE, true);

	err = f2fs_get_node_info(sbi, dn.nid, &ni);
	if (err)
		goto err;

	f2fs_bug_on(sbi, ni.ino != ino);

	src = datablock_addr(dn.inode, dn.node_page, dn.ofs_in_node);
	dest = blkaddr;
	hmfs_msg(sbi->sb, KERN_INFO,
		"ino[%u, %u, %lu]: has src[%lu] off[%u] ver[%x]",
		ino, index, blkaddr, src, dn.ofs_in_node, ni.version);

	/* if allocate new address is same as oob info, just skip */
	if (src == dest)
		goto err;

	if (!file_keep_isize(inode) &&
		(i_size_read(inode) <= ((loff_t)start << PAGE_SHIFT)))
		f2fs_i_size_write(inode, (loff_t)(start + 1) << PAGE_SHIFT);

	f2fs_bug_on(sbi, !hmfs_is_valid_blkaddr(sbi, dest, META_POR));
	/* dest is valid block, try to recover from src to dest */
	if (src == NULL_ADDR) {
		err = hmfs_reserve_new_block(&dn);
		while (err && IS_ENABLED(CONFIG_HMFS_FAULT_INJECTION))
			err = hmfs_reserve_new_block(&dn);
		/* We should not get -ENOSPC */
		f2fs_bug_on(sbi, err);
		if (err)
			goto err;
	}
retry_prev:
	/* Check the previous node page having this index */
	err = check_index_in_prev_nodes(sbi, dest, &dn);
	if (err) {
		if (err == -ENOMEM) {
			congestion_wait(BLK_RW_ASYNC, HZ/50);
			goto retry_prev;
		}
		goto err;
	}

	/* write dummy data page */
	hmfs_replace_block(sbi, &dn, src, dest, ni.version, true, true, true);
	++recovered;

	fill_node_footer(dn.node_page, dn.nid, ni.ino,
			ofs_of_node(dn.node_page), false);
	set_page_dirty(dn.node_page);
err:
	f2fs_put_dnode(&dn);
out:
	return err;
}

static int recover_file_ftl(struct f2fs_sb_info *sbi,
		struct list_head *inode_list)
{
	int i, j, k;
	int err = 0;
	block_t blkaddr, end_lba;
	struct fsync_inode_entry *entry;
	unsigned int ino, offset;

	/* scan oob info in hot/code data segment */
	for (i = STREAM_META + 1; i < STREAM_NR; ++i) {
		if (!__is_need_oob_stream(sbi, i, false))
			continue;

		k = 0;
		for (j = 0; j < sbi->oob_ctrl[i].recover_ext_cnt; ++j) {
			blkaddr = sbi->oob_ctrl[i].recover_ext[j].start_lba;
			end_lba = sbi->oob_ctrl[i].recover_ext[j].end_lba;

			for (; blkaddr <= end_lba; ++k, ++blkaddr) {
				ino = sbi->oob_ctrl[i].oob_info[k].data_ino;
				entry = get_fsync_inode(inode_list, ino);
				if (!entry)
					continue;
				f2fs_bug_on(sbi, entry->inode->i_ino != ino);

				offset = sbi->oob_ctrl[i].oob_info[k].data_idx;
				offset &= (~OOB_FSYNC_BIT);

				err = do_recover_ftl(sbi,
						entry->inode, offset, blkaddr);
				if (err) {
					hmfs_msg(sbi->sb, KERN_ERR,
						"oob recover ino[%u] fail[%d, %u, %lu, i_size %s]",
						ino, err, offset, blkaddr,
						file_keep_isize(entry->inode) ? "keep" : "recover");
					goto out;
				}
				sbi->oob_ctrl[i].recover_ext[j].rsvd_space = false;
			}
		}
	}

out:
	return err;
}

static int __do_oob_recover(struct f2fs_sb_info *sbi, bool check_only,
		unsigned long s_flags,
		struct list_head *inode_list,
		int *ret, bool *need_writecp)
{
	int err = get_oob_info(sbi);
	if (err)
		goto skip_oob;

	err = find_fsync_oobs(sbi, inode_list);
	if (err || list_empty(inode_list))
		goto skip_oob;

	if (check_only) {
		*ret = 1;
		goto skip_oob;
	}
	*need_writecp = true;

	err = recover_file_ftl(sbi, inode_list);
	if (err) {
		/* restore s_flags to let iput() trash data */
		sbi->sb->s_flags = s_flags;
	}

skip_oob:
	clean_oob_info(sbi);
	return err;
}

int hmfs_recover_fsync_data(struct f2fs_sb_info *sbi, bool check_only,
		bool *need_cp)
{
	struct list_head inode_list, tmp_inode_list;
	struct list_head dir_list;
	int err;
	int ret = 0;
	unsigned long s_flags = sbi->sb->s_flags;
	bool need_writecp = false;
#ifdef CONFIG_QUOTA
	int quota_enabled;
#endif

	if (s_flags & MS_RDONLY) {
		hmfs_msg(sbi->sb, KERN_INFO,
			"recover fsync data on readonly fs");
		sbi->sb->s_flags &= ~MS_RDONLY;
	}

#ifdef CONFIG_QUOTA
	/* Needed for iput() to work correctly and not trash data */
	sbi->sb->s_flags |= MS_ACTIVE;
	/* Turn on quotas so that they are updated correctly */
	quota_enabled = hmfs_enable_quota_files(sbi, s_flags & MS_RDONLY);
#endif

	fsync_entry_slab = f2fs_kmem_cache_create("f2fs_fsync_inode_entry",
			sizeof(struct fsync_inode_entry));
	if (!fsync_entry_slab) {
		err = -ENOMEM;
		goto out;
	}

	INIT_LIST_HEAD(&inode_list);
	INIT_LIST_HEAD(&tmp_inode_list);
	INIT_LIST_HEAD(&dir_list);

	/* prevent checkpoint */
	mutex_lock(&sbi->cp_mutex);

	/* step #1: find fsynced inode numbers */
	err = find_fsync_dnodes(sbi, &inode_list, check_only);
	if (err || list_empty(&inode_list))
		goto skip;

	if (check_only) {
		ret = 1;
		goto skip;
	}

	need_writecp = true;

	/* step #2: recover data */
	err = recover_data(sbi, &inode_list, &tmp_inode_list, &dir_list);
	if (!err)
		f2fs_bug_on(sbi, !list_empty(&inode_list));
	else {
		/* restore s_flags to let iput() trash data */
		sbi->sb->s_flags = s_flags;
	}
skip:
	/*
	 * step #3: recover data addr in dnode
	 *          with oob info if oob recovery is enable
	 */
	if (ret == 0 && err == 0) {
		/*
		 * recovery inode from 2 source:
		 * 1) ino in oob info of data stream by fsync
		 * 2) ino in node footer that has fsync_mark
		 * so use tmp_inode_list(already had these inodes before)
		 */
		err = __do_oob_recover(sbi, check_only, s_flags,
				&tmp_inode_list, &ret, &need_writecp);
	}
	hmfs_free_oob_rsvd_space(sbi);

	destroy_fsync_dnodes(&inode_list, err);
	destroy_fsync_dnodes(&tmp_inode_list, err);

	/* truncate meta pages to be used by the recovery */
	truncate_inode_pages_range(META_MAPPING(sbi),
			(loff_t)MAIN_BLKADDR(sbi) << PAGE_SHIFT, -1);

	if (err) {
		truncate_inode_pages_final(NODE_MAPPING(sbi));
		truncate_inode_pages_final(META_MAPPING(sbi));
	} else {
		clear_sbi_flag(sbi, SBI_POR_DOING);
	}
	mutex_unlock(&sbi->cp_mutex);

	/* let's drop all the directory inodes for clean checkpoint */
	destroy_fsync_dnodes(&dir_list, err);

	if (need_writecp) {
		set_sbi_flag(sbi, SBI_IS_RECOVERED);

		if (!err) {
			struct cp_control cpc = {
				.reason = CP_RECOVERY,
			};
			err = hmfs_write_checkpoint(sbi, &cpc);
			*need_cp = false;
		}
	}

	kmem_cache_destroy(fsync_entry_slab);
out:
#ifdef CONFIG_QUOTA
	/* Turn quotas off */
	if (quota_enabled)
		hmfs_quota_off_umount(sbi->sb);
#endif
	sbi->sb->s_flags = s_flags; /* Restore MS_RDONLY status */

	return ret ? ret: err;
}
