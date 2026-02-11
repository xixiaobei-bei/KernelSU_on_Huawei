#ifndef _LINUX_F2FS_TZ_H
#define _LINUX_F2FS_TZ_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <scsi/ufs/ufs.h>

#ifdef CONFIG_F2FS_TURBO_ZONE
#define F2FS_TZ_STATUS_SIZE (sizeof(struct tz_status))
#define LIMIT_TZ_INVALID_BLOCK     40 /* percentage over total user space */
#define LIMIT_TZ_FREE_BLOCK        40 /* percentage over invalid + free space */

struct f2fs_migrate_file {
	u32 turbo;
	u32 sync;
};

/* turbo zone query and ctrl in block/hisi-bkops-core.c */
extern int blk_lld_tz_query(struct block_device *bi_bdev, u8 *buf, u32 buf_len);
extern int blk_lld_tz_ctrl(struct block_device *bi_bdev, int desc_id,
								uint8_t index);

static inline void process_tz_key_flag(u32 turbo, struct inode *inode)
{
	if (turbo)
		F2FS_I(inode)->i_flags |= F2FS_TZ_KEY_FL;
	else
		F2FS_I(inode)->i_flags &= ~F2FS_TZ_KEY_FL;

	f2fs_set_inode_flags(inode);
	f2fs_mark_inode_dirty_sync(inode, true);
}

static inline void process_tz_flag(u32 turbo, struct inode *inode, int tz_flag)
{
	if (turbo)
		set_inode_flag(inode, tz_flag);
	else
		clear_inode_flag(inode, tz_flag);

	if (tz_flag == FI_TZ_KEY_FILE)
		process_tz_key_flag(turbo, inode);
}

static inline bool is_tz_flag_set(struct inode *inode, int tz_flag)
{
	if (is_inode_flag_set(inode, tz_flag))
		return true;

	if (tz_flag == FI_TZ_KEY_FILE &&
		(F2FS_I(inode)->i_flags & F2FS_TZ_KEY_FL))
		return true;

	return false;
}


/* get turbo zone total blocks in SLC. */
static inline unsigned int get_tz_total_blocks(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	unsigned int total_blocks;

	total_blocks = (tz_info->end_seg - tz_info->start_seg) <<
				sbi->log_blocks_per_seg;
	return  total_blocks;
}

static inline block_t free_tz_user_blocks(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;

	if (tz_info->free_segs < F2FS_TURBO_RESERVED_SEGS)
		return 0;
	else
		return (tz_info->free_segs - F2FS_TURBO_RESERVED_SEGS)
			<< sbi->log_blocks_per_seg;
}

static inline block_t limit_invalid_tz_user_blocks(struct f2fs_sb_info *sbi)
{
	return (long)(get_tz_total_blocks(sbi) * LIMIT_TZ_INVALID_BLOCK) / 100;
}

static inline block_t limit_free_tz_user_blocks(struct f2fs_sb_info *sbi)
{
	block_t reclaimable_user_blocks = get_tz_total_blocks(sbi) -
			sbi->tz_info.written_valid_blocks;

	return (long)(reclaimable_user_blocks * LIMIT_TZ_FREE_BLOCK) / 100;
}

/* Copy from has_enough_invalid_blocks */
static inline bool has_enough_tz_invalid_blocks(struct f2fs_sb_info *sbi)
{
	block_t invalid_user_blocks = get_tz_total_blocks(sbi) -
			sbi->tz_info.written_valid_blocks;

	if (invalid_user_blocks > limit_invalid_tz_user_blocks(sbi) &&
			free_tz_user_blocks(sbi) <
				limit_free_tz_user_blocks(sbi))
		return true;
	return false;
}

int f2fs_ioc_set_turbo_file(struct file *filp, unsigned long arg,
					int tz_flag);
int f2fs_ioc_get_turbo_file(struct file *filp, unsigned long arg,
					int tz_flag);

unsigned int get_turbo_zone_free_blocks(struct f2fs_sb_info *sbi);
int f2fs_ioc_get_turbo_free_blocks(struct file *filp, unsigned long arg);

int f2fs_ioc_get_turbo_status(struct file *filp, unsigned long arg);

int f2fs_ioc_set_turbo_return(struct file *filp, unsigned long arg);

int f2fs_get_tz_info(struct f2fs_sb_info *sbi);
void init_f2fs_turbo_info(struct f2fs_sb_info *sbi);
int f2fs_ioc_migrate_file(struct file *filp, unsigned long arg);
int f2fs_ioc_set_tz_force_close(struct file *filp);
#endif

#endif
