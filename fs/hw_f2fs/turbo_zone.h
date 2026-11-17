#ifndef _LINUX_F2FS_TZ_H
#define _LINUX_F2FS_TZ_H

#include <linux/types.h>
#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <scsi/ufs/ufs.h>

#ifdef CONFIG_F2FS_TURBO_ZONE
#define TZ_LOG_TAG "[TURBO ZONE]"

struct tz_status_v1 {
	uint8_t enable;		/* 0: disable; 1: enable; */
	uint8_t lu_number;	/* LU number */
	uint16_t max_ec;	/* max EC of Turbo Zone */
	uint16_t avg_ec;	/* average EC of turbo zone */
	uint8_t return_en;	/* return to tlc enable */
	uint8_t return_back_en;	/* return back to slc enable */
	uint32_t max_lba;	/* max LBA(4KB unit) */
	uint32_t total_vpc;	/* Total VPC, Valid when TurboZone Enable */
	uint8_t close_qcnt;	/* blocks to return */
	uint8_t forbidden;	/* already used once, cannot enable again */
	uint8_t reserved[2];	/* reserved */
};

#define F2FS_TZ_STATUS_SIZE (sizeof(struct tz_status_v1))
#define LIMIT_TZ_INVALID_BLOCK     40 /* percentage over total user space */
#define LIMIT_TZ_FREE_BLOCK        40 /* percentage over invalid + free space */
#define PREFREE_THRESHOLD 500                  /* 1G */
#define FREE_SEC_REMAIN 25                     /* 50M */

struct f2fs_migrate_file {
	u32 turbo;
	u32 sync;
};

struct f2fs_tz_space_info {
	u32 device_segment;
	u32 reserved_segment;
	u32 tz_total_segment;
};

struct f2fs_ioc_tz_info {
	unsigned char version;
	bool enable;
	unsigned char reserved[2];	/* reserved */
};

/* turbo zone query and ctrl in block/hisi-bkops-core.c */
extern int blk_lld_tz_query(const struct block_device *bi_bdev, u32 type,
							u8 *buf, u32 buf_len);
extern int blk_lld_tz_ctrl(const struct block_device *bi_bdev, int desc_id,
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

static inline bool is_tz_closed(struct f2fs_tz_info *tz_info)
{
	return !tz_info->enabled;
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

void inc_bd_ioctl_migrate_block(struct f2fs_sb_info *sbi, bool turbo, int submitted);

void bd_set_return_time(struct f2fs_sb_info *sbi, bool ioctl_ret);

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
int f2fs_ioc_get_tz_space_info(struct file *filp, unsigned long arg);
void set_tz_weighted_bdev(struct f2fs_sb_info *sbi, bool set);
int f2fs_close_tz(struct f2fs_sb_info *sbi, bool sync);
void f2fs_tz_imonitor_send(struct f2fs_sb_info *sbi);
int f2fs_ioc_get_tz_info(struct file *filp, unsigned long arg);
unsigned int f2fs_get_turbo_version(struct f2fs_sb_info *sbi);

#endif

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
int f2fs_query_tz_cap(struct f2fs_sb_info *sbi, struct tz_cap_info *cap_info);
int f2fs_query_tz_blks(struct f2fs_sb_info *sbi, struct tz_blk_info *blk_info,
			block_t pblk, unsigned int len);
int f2fs_ioc_get_tz_slc_info(struct file *filp, unsigned long arg);

static inline bool f2fs_is_support_tz_v2(struct f2fs_sb_info *sbi)
{
	return sbi->turbo_info.version == F2FS_TURBO_ZONE_V2;
}

static inline bool f2fs_is_enabled_tz_v2(struct f2fs_sb_info *sbi)
{
	return f2fs_is_support_tz_v2(sbi) && (sbi->turbo_info.enable);
}

static inline bool f2fs_is_tz_key_io(struct f2fs_sb_info *sbi,
					struct inode *inode)
{
	return f2fs_is_enabled_tz_v2(sbi) &&
			is_tz_flag_set(inode, FI_TZ_KEY_FILE);
}
#ifdef CONFIG_F2FS_CHECK_FS
#define IOC_BLK_BUF_SIZE 256
struct f2fs_ioc_blk_info {
	unsigned int addr;
	unsigned short len;
	unsigned char flags[IOC_BLK_BUF_SIZE];
};
int f2fs_ioc_get_tz_blk_info(struct file *filp, unsigned long arg);
#endif
#endif

#endif
