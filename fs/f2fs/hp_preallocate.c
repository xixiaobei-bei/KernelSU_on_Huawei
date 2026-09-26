/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
 * Description: This file is for preallocating blocks in f2fs
 * Author: laixinyi
 * Create: 2020-12-01
 */

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
#include <linux/fs.h>
#include <linux/f2fs_fs.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"

static void f2fs_curseg_keyinfo_copy(struct curseg_info *dest,
				     struct curseg_info *source)
{
	dest->sum_blk = source->sum_blk;
	dest->journal = source->journal;
	dest->segno = source->segno;
	dest->next_blkoff = source->next_blkoff;
	dest->zone = source->zone;
	dest->next_segno = source->next_segno;
	dest->alloc_type = source->alloc_type;
}

int f2fs_curseg_set(struct f2fs_sb_info *sbi, int type, unsigned int segno,
		    struct curseg_info *backupseg)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);
	struct summary_footer *sum_footer = NULL;

	mutex_lock(&curseg->curseg_mutex);
	f2fs_curseg_keyinfo_copy(backupseg, curseg);

	curseg->segno = segno;
	curseg->zone = GET_ZONE_FROM_SEG(sbi, curseg->segno);
	curseg->next_segno = NULL_SEGNO;
	curseg->next_blkoff = 0;
	curseg->alloc_type = PREALLOC;
	curseg->sum_blk = f2fs_kzalloc(sbi, PAGE_SIZE, GFP_KERNEL);
	if (!curseg->sum_blk)
		curseg->sum_blk =
			f2fs_kzalloc(sbi, PAGE_SIZE, GFP_KERNEL | __GFP_NOFAIL);
	if (unlikely(!curseg->sum_blk)) {
		mutex_unlock(&curseg->curseg_mutex);
		return -ENOMEM;
	}
	curseg->journal =
		f2fs_kzalloc(sbi, sizeof(struct f2fs_journal), GFP_KERNEL);
	if (!curseg->journal)
		curseg->journal = f2fs_kzalloc(sbi, sizeof(struct f2fs_journal),
					       GFP_KERNEL | __GFP_NOFAIL);
	if (unlikely(!curseg->journal)) {
		kfree(curseg->sum_blk);
		mutex_unlock(&curseg->curseg_mutex);
		return -ENOMEM;
	}

	sum_footer = &(curseg->sum_blk->footer);
	memset(sum_footer, 0, sizeof(struct summary_footer));
	SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
	__set_sit_entry_type(sbi, type, curseg->segno, 1);

	mutex_unlock(&curseg->curseg_mutex);

	return 0;
}

void f2fs_curseg_restore(struct f2fs_sb_info *sbi, int type,
			 struct curseg_info *backupseg)
{
	struct curseg_info *curseg = CURSEG_I(sbi, type);

	mutex_lock(&curseg->curseg_mutex);
	kvfree(curseg->sum_blk);
	kvfree(curseg->journal);
	f2fs_curseg_keyinfo_copy(curseg, backupseg);
	mutex_unlock(&curseg->curseg_mutex);
}

static unsigned int least_secnum_needed(struct f2fs_sb_info *sbi, int needed)
{
	int node_secs = get_blocktype_secs(sbi, F2FS_DIRTY_NODES);
	int dent_secs = get_blocktype_secs(sbi, F2FS_DIRTY_DENTS);
	int imeta_secs = get_blocktype_secs(sbi, F2FS_DIRTY_IMETA);

	return (node_secs + dent_secs + imeta_secs + dent_secs +
		reserved_sections(sbi) + needed);
}

unsigned int f2fs_find_continuous_secs(struct f2fs_sb_info *sbi, int type,
				       int secnum)
{
	struct free_segmap_info *free_i = FREE_I(sbi);
	unsigned int secno = MAIN_SECS(sbi);
	unsigned int segno;
	unsigned int i = 0;
	unsigned int j;

	spin_lock(&free_i->segmap_lock);
	if (free_sections(sbi) <= least_secnum_needed(sbi, secnum)) {
		pr_err("[HP]%s: free secnum = %u, needed = %u", __func__,
		       free_sections(sbi), least_secnum_needed(sbi, secnum));
		goto out;
	}
	while (i < MAIN_SECS(sbi)) {
		i = find_next_zero_bit(free_i->free_secmap, MAIN_SECS(sbi), i);
		j = find_next_bit(free_i->free_secmap, MAIN_SECS(sbi), i);
		if (j - i >= (unsigned int)secnum) {
			secno = i;
			break;
		} else {
			i = j;
		}
	}
	if (secno < MAIN_SECS(sbi)) {
		segno = GET_SEG_FROM_SEC(sbi, secno);
		for (i = segno; i < segno + sbi->segs_per_sec * secnum; i++)
			__set_inuse(sbi, i);
	}
out:
	spin_unlock(&free_i->segmap_lock);

	return secno;
}

bool segment_prealloc(struct f2fs_sb_info *sbi, struct curseg_info *curseg,
		      int type)
{
	struct summary_footer *sum_footer = NULL;

	if (curseg->alloc_type != PREALLOC)
		return false;
	write_sum_page(sbi, curseg->sum_blk, GET_SUM_BLOCK(sbi, curseg->segno));
	curseg->segno++;
	curseg->zone = GET_ZONE_FROM_SEG(sbi, curseg->segno);
	curseg->next_blkoff = 0;
	sum_footer = &(curseg->sum_blk->footer);
	memset(sum_footer, 0, sizeof(struct summary_footer));
	SET_SUM_TYPE(sum_footer, SUM_TYPE_DATA);
	__set_sit_entry_type(sbi, type, curseg->segno, 1);
	return true;
}

static int f2fs_hp_file_expand(struct inode *inode, unsigned long long len)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_map_blocks map = {
		.m_next_pgofs = NULL,
		.m_next_extent = NULL,
		.m_seg_type = NO_CHECK_TYPE
	};
	pgoff_t pg_end;
	int err;
	struct curseg_info backupseg;
	unsigned int target_segno, target_secno;

	pg_end = len >> PAGE_SHIFT; /*lint !e644*/
	if (has_not_enough_free_secs(sbi, 0, pg_end / BLKS_PER_SEC(sbi))) {
			pr_err("[HP]No enough space! \r\n");
			return -ENOSPC;
	}

	err = inode_newsize_ok(inode, len); /*lint !e644*/
	if (err)
		return err;

	map.m_lblk = 0;
	map.m_len = pg_end;

	down_write(&sbi->pin_sem);
	mutex_lock(&sbi->cp_mutex);
	map.m_seg_type = CURSEG_COLD_DATA_PINNED;

	target_secno = f2fs_find_continuous_secs(sbi, CURSEG_COLD_DATA,
						 pg_end / BLKS_PER_SEC(sbi));
	if (target_secno >= MAIN_SECS(sbi)) {
		mutex_unlock(&sbi->cp_mutex);
		up_write(&sbi->pin_sem);
		pr_err("[HP]No continuous space!");
		return -ENOSPC;
	}

	target_segno = GET_SEG_FROM_SEC(sbi, target_secno);

	err = f2fs_curseg_set(sbi, CURSEG_COLD_DATA, target_segno, &backupseg);
	if (err) {
		unsigned int i;
		for (i = target_segno;
		     i < target_segno + pg_end / sbi->blocks_per_seg; i++)
			__set_test_and_free(sbi, i);
		mutex_unlock(&sbi->cp_mutex);
		up_write(&sbi->pin_sem);
		return err;
	}
	err = f2fs_map_blocks(inode, &map, 1, F2FS_GET_BLOCK_PRE_DIO);
	f2fs_curseg_restore(sbi, CURSEG_COLD_DATA, &backupseg);
	mutex_unlock(&sbi->cp_mutex);
	up_write(&sbi->pin_sem);

	f2fs_i_size_write(inode, len); /*lint !e644*/

	return err;
}

#define HP_FILE_MAX (5UL * 1024UL * 1024UL * 1024UL)

static int f2fs_hp_param_check(struct inode *inode, unsigned long arg,
			       struct hp_file_cfg *cfg)
{
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	if (unlikely(!S_ISREG(inode->i_mode))) {
		pr_err("[HP]target is directory! \r\n");
		return -EISDIR;
	}

	if (unlikely(!(struct f2fs_hp_file_cfg __user *)arg))
		return -EFAULT;

	if (unlikely(copy_from_user(cfg, (struct f2fs_hp_file_cfg __user *)arg,
				    sizeof(*cfg)))) {
		pr_err("[HP]copy from user error! \r\n");
		return -EINVAL;
	}

	if ((sbi->blocks_per_seg * sbi->segs_per_sec * PAGE_SIZE - 1) &
	    cfg->len) {
		pr_err("[HP]Not section align! \r\n");
		return -EINVAL;
	}

	if (cfg->len > HP_FILE_MAX) {
		pr_err("[HP]preallocate size too large! \r\n");
		return -EINVAL;
	}

	return 0;
}

static int f2fs_hp_output_check(struct file *filp, struct f2fs_sb_info *sbi,
				struct hp_file_cfg *cfg)
{
	struct address_space *mapping = filp->f_mapping;
	unsigned long offset;

	if (!mapping || !mapping->a_ops) {
		pr_err("[HP]No address mapping operations! \r\n");
		return -EINVAL;
	}

	cfg->addr = mapping->a_ops->bmap(mapping, 0);
	offset = mapping->a_ops->bmap(mapping, (cfg->len >> PAGE_SHIFT) - 1); /*lint !e644*/
	if (offset > MAX_BLKADDR(sbi) || offset < cfg->addr) {
		pr_err("[HP]bmap error:start_blk = %lu, off = %lu, maxblk = %u",
		       cfg->addr, offset, MAX_BLKADDR(sbi));
		return -EFAULT;
	}

	return 0;
}

int f2fs_ioc_set_hp_file(struct file *filp, unsigned long arg)
{
	int ret;
	struct hp_file_cfg cfg;
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);

	pr_err("[HP]hp file ino = %lu", inode->i_ino);

	ret = f2fs_hp_param_check(inode, arg, &cfg);
	if (unlikely(ret))
		goto exit;

	ret = mnt_want_write_file(filp);
	if (unlikely(ret)) {
		pr_err("[HP]mnt_want_write_file failed! %d", ret);
		goto exit;
	}

	inode_lock(inode);

	ret = f2fs_convert_inline_inode(inode);
	if (unlikely(ret)) {
		pr_err("[HP]convert inline inode failed! %d", ret);
		goto unlock_exit;
	}

	set_inode_flag(inode, FI_PIN_FILE);
	ret = f2fs_hp_file_expand(inode, cfg.len); /*lint !e644*/
	if (ret) {
		clear_inode_flag(inode, FI_PIN_FILE);
		goto unlock_exit;
	}

	inode->i_mtime = inode->i_ctime = current_time(inode);

	f2fs_update_time(F2FS_I_SB(inode), REQ_TIME);
	ret = f2fs_hp_output_check(filp, sbi, &cfg);
	if (unlikely(ret))
		goto unlock_exit;
	if (unlikely(copy_to_user((struct f2fs_hp_file_cfg __user *)arg, &cfg,
				  sizeof(cfg)))) {
		pr_err("[HP]copy to user error! \r\n");
		ret = -EINVAL;
	}
unlock_exit:
	inode_unlock(inode);
	mnt_drop_write_file(filp);
exit:
	return ret;
}
