#include <linux/fs.h>
#include <linux/f2fs_fs.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/blkdev.h>
#include <linux/types.h>
#include <linux/compat.h>
#include <linux/uaccess.h>
#include <linux/pagevec.h>
#include <linux/file.h>
#include <linux/mount.h>

#include "f2fs.h"
#include "node.h"
#include "segment.h"
#include "turbo_zone.h"

#ifdef CONFIG_F2FS_TURBO_ZONE
void inc_bd_ioctl_migrate_block(struct f2fs_sb_info *sbi, bool turbo, int submitted)
{
	bd_mutex_lock(&sbi->bd_mutex);
	if (turbo)
		inc_bd_array_val(sbi, tz_ioctl_migrate_blks, NZ_TO_TZ, submitted);
	else
		inc_bd_array_val(sbi, tz_ioctl_migrate_blks, TZ_TO_NZ, submitted);
	bd_mutex_unlock(&sbi->bd_mutex);
}

void bd_set_return_time(struct f2fs_sb_info *sbi, bool ioctl_ret)
{
	struct timespec now = current_kernel_time();
	bd_mutex_lock(&sbi->bd_mutex);
	if (ioctl_ret)
		set_bd_val(sbi, tz_ioctl_trigger_return_time,
				(unsigned long) now.tv_sec);
	else
		set_bd_val(sbi, tz_f2fs_trigger_return_time,
				(unsigned long) now.tv_sec);
	bd_mutex_unlock(&sbi->bd_mutex);
}

/* set FI_TZ_KEY_FILE and FI_TZ_AGING_FILE */
int f2fs_ioc_set_turbo_file(struct file *filp, unsigned long arg,
					int tz_flag)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	__u32 turbo;
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (get_user(turbo, (__u32 __user *)arg))
		return -EFAULT;

	if (turbo > 1)
		return -EFAULT;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	if (f2fs_readonly(F2FS_I_SB(inode)->sb))
		return -EROFS;

	ret = mnt_want_write_file(filp);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:mnt_want_write_file err. ret %d",
				TZ_LOG_TAG, __func__, ret);
		return ret;
	}

	if (!tz_info->enabled)
		return -EPERM;

	recovery_turbo_init(sbi, false);
	inode_lock(inode);

	process_tz_flag(turbo, inode, tz_flag);

	inode_unlock(inode);
	mnt_drop_write_file(filp);
	return ret;
}

int f2fs_ioc_get_turbo_file(struct file *filp, unsigned long arg,
					int tz_flag)
{
	struct inode *inode = file_inode(filp);
	__u32 turbo = 0;

	if (is_tz_flag_set(inode, tz_flag))
		turbo = 1;

	return put_user(turbo, (u32 __user *)arg);
}

/* get f2fs free space in turbo zone, but it is not available for sure. */
unsigned int get_turbo_zone_free_blocks(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	unsigned int start_segno, end_segno, segno;
	unsigned int used_blocks = 0, total_blocks = 0;

	start_segno = tz_info->start_seg;
	end_segno = tz_info->end_seg;
	total_blocks = (end_segno - start_segno) << sbi->log_blocks_per_seg;

	for (segno = start_segno; segno < end_segno; segno++)
		used_blocks += get_valid_blocks(sbi, segno, false);

	return (total_blocks > used_blocks) ? (total_blocks - used_blocks) : 0;
}

int f2fs_ioc_get_turbo_free_blocks(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	__u32 free_blocks = 0;

	if (tz_info->enabled)
		free_blocks = get_turbo_zone_free_blocks(sbi);

	return put_user(free_blocks, (u32 __user *)arg);
}

int f2fs_ioc_get_turbo_status(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct block_device *turbo_bdev = NULL;
	struct tz_status *status = NULL;
	int ret = 0;

	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return -ENOMEM;

	if (sbi->s_ndevs)
		turbo_bdev = FDEV(F2FS_TURBO_DEV).bdev;
	else
		turbo_bdev = sbi->sb->s_bdev;

	ret = blk_lld_tz_query(turbo_bdev, (char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret < 0) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:failed to get turbo status.ret = %d",
			TZ_LOG_TAG, __func__, ret);
		goto out;
	}

	if (copy_to_user((struct tz_status __user *)arg, status,
				F2FS_TZ_STATUS_SIZE))
		ret = -EFAULT;

out:
	kfree(status);
	return ret;
}

/* if turbo zone tlc is not written allow to switch */
static bool allow_tz_switch(struct f2fs_sb_info *sbi)
{
	if (sbi->total_valid_block_count + sbi->tz_info.reserved_blks -
		(F2FS_TURBO_RESERVED_SEGS << sbi->log_blocks_per_seg) >
			sbi->user_block_count - sbi->current_reserved_blocks)
		return false;
	return true;
}

/* control turbo zone can return (slc->tlc) or not. */
int f2fs_ioc_set_turbo_return(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct block_device *turbo_bdev = NULL;
	__u32 switchable;
	int ret = 0;

	if (get_user(switchable, (__u32 __user *)arg))
		return -EFAULT;

	if (switchable > 1)
		return -EFAULT;

	if (sbi->s_ndevs)
		turbo_bdev = FDEV(F2FS_TURBO_DEV).bdev;
	else
		turbo_bdev = sbi->sb->s_bdev;

	if (!switchable && !allow_tz_switch(sbi)) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"Turbo zone tlc data is written, not allowed to switch");
		return -EPERM;
	}

	ret = blk_lld_tz_ctrl(turbo_bdev, TZ_RETURN_FLAG, switchable);
	if (ret == 0)
		sbi->tz_info.switchable = (bool)switchable;
	return ret;
}

/* Reserved partition size should be excluded from max_lba available
 * for filesystem. Only support tz is the first or last partition.
 */
static unsigned int get_available_lba(struct block_device *bdev,
						unsigned int max_lba)
{
	return max_lba - ((unsigned int)get_start_sect(bdev) >>
					F2FS_LOG_SECTORS_PER_BLOCK);
}

/* query f2fs turbo zone info from block layer */
int f2fs_get_tz_info(struct f2fs_sb_info *sbi)
{
	struct tz_status *status = NULL;
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	struct block_device *bdev = NULL;
	unsigned int total_segs;
	unsigned int available_lba;
	int ret = 0;

	if (!sbi->s_ndevs) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:Single device is not supported in turbozone",
			TZ_LOG_TAG, __func__);
		return -EPERM;
	}

	bdev = FDEV(F2FS_TURBO_DEV).bdev;
	total_segs = FDEV(F2FS_TURBO_DEV).total_segments;

	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return -ENOMEM;

	ret = blk_lld_tz_query(bdev, (char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret < 0) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:failed to get turbo status. ret = %d",
			TZ_LOG_TAG, __func__, ret);
		goto out;
	}

	/* only support turbo zone is the first or last device */
	tz_info->enabled = (bool)status->enable;
	tz_info->switchable = (bool)status->return_en;
	tz_info->cur_ec = status->avg_ec;
	tz_info->cur_vpc = status->total_vpc;

	available_lba = get_available_lba(bdev, status->max_lba);

	/* if it's the first device, it represents total segments from seg0 */
	if (FDEV(F2FS_TURBO_DEV).start_blk < MAIN_BLKADDR(sbi))
		tz_info->total_segs = (available_lba - SEG0_BLKADDR(sbi)) >>
					sbi->log_blocks_per_seg;
	else
		tz_info->total_segs = available_lba >> sbi->log_blocks_per_seg;

	tz_info->reserved_blks = (total_segs - tz_info->total_segs +
			F2FS_TURBO_RESERVED_SEGS) << sbi->log_blocks_per_seg;

	if (tz_info->enabled && need_turn_off_tz(sbi)) {
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:tz turn to disabled",
				TZ_LOG_TAG, __func__);
		bd_set_return_time(sbi, false);
		f2fs_close_tz(sbi, true);
	}

	if (sbi->tz_info.enabled &&
	    get_free_segs_in_normal_zone(sbi) <
	    (reserved_segments(sbi) - F2FS_MIN_SEGS_IN_TZ))
		f2fs_close_tz(sbi, true);

out:	kfree(status);
	return ret;
}

/* Weight of SmartIO is set to sde, so it needs compatible change for sdd */
void set_tz_weighted_bdev(struct f2fs_sb_info *sbi, bool set)
{
	struct block_device *bdev = NULL;
	struct request_queue *q = NULL;

	if (!sbi->s_ndevs)
		return;

	bdev = FDEV(sbi->s_ndevs - 1 - F2FS_TURBO_DEV).bdev;
	q = bdev_get_queue(bdev);
	if (q) {
		if (set)
			q->tz_weighted_bdev = FDEV(F2FS_TURBO_DEV).bdev;
		else
			q->tz_weighted_bdev = NULL;
	}
}

void init_f2fs_turbo_info(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	struct seg_entry *sentry = NULL;
	unsigned int start, tz_end_seg;
	unsigned int tz_real_end_seg, tz_tlc_used_seg;

	memset(tz_info, 0, sizeof(struct f2fs_tz_info));
	memset(&sbi->gc_turbo_thread, 0, sizeof(struct f2fs_gc_kthread));

	if (f2fs_get_tz_info(sbi) || tz_info->total_segs == 0)
		return;

	if (FDEV(F2FS_TURBO_DEV).start_blk < MAIN_BLKADDR(sbi)) {
		if (tz_info->total_segs <= FREE_I(sbi)->start_segno) {
			f2fs_msg(sbi->sb, KERN_WARNING,
					"%s%s:Turbo zone is too small",
					TZ_LOG_TAG, __func__);
			bd_set_return_time(sbi, false);
			f2fs_close_tz(sbi, false);
			return;
		}
		tz_info->start_seg = 0;
		tz_info->end_seg = tz_info->total_segs -
				FREE_I(sbi)->start_segno;
	} else {
		tz_info->start_seg = GET_SEGNO_FROM_SEG0(sbi,
				FDEV(F2FS_TURBO_DEV).start_blk) -
				FREE_I(sbi)->start_segno;
		tz_info->end_seg = tz_info->total_segs + tz_info->start_seg;
	}

	tz_end_seg = tz_info->end_seg;

	if (tz_info->enabled) {
		tz_real_end_seg = tz_end_seg +
				  FDEV(F2FS_TURBO_DEV).total_segments -
				  tz_info->total_segs;
		tz_tlc_used_seg = find_next_inuse(FREE_I(sbi), tz_real_end_seg,
						  tz_end_seg);
		if (tz_tlc_used_seg < tz_real_end_seg)
			f2fs_close_tz(sbi, true);
	}

	for (start = tz_info->start_seg; start < tz_end_seg; start++) {
		sentry = get_seg_entry(sbi, start);
		if (!sentry->valid_blocks)
			tz_info->free_segs++;
		else
			tz_info->written_valid_blocks += sentry->valid_blocks;
	}

	/* support SmartIO */
	set_tz_weighted_bdev(sbi, true);
}

int f2fs_ioc_migrate_file(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	struct f2fs_migrate_file param;
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (copy_from_user(&param, (struct f2fs_migrate_file __user *)arg,
								sizeof(param)))
		return -EFAULT;

	if (!S_ISREG(inode->i_mode))
		return -EINVAL;

	if (f2fs_readonly(F2FS_I_SB(inode)->sb))
		return -EROFS;

	ret = mnt_want_write_file(filp);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:mnt_want_write_file err, ret %d",
				TZ_LOG_TAG, __func__, ret);
		return ret;
	}

	if (!tz_info->enabled)
		return -EPERM;

	inode_lock(inode);
	ret = f2fs_migrate_file(inode, param.turbo, param.sync);
	inode_unlock(inode);

	mnt_drop_write_file(filp);
	return ret;
}

/* force close for TZ */
int f2fs_ioc_set_tz_force_close(struct file *filp)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct block_device *turbo_bdev = NULL;
	struct tz_status *status = NULL;
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (!sbi->s_ndevs)
		return -EPERM;

	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return -ENOMEM;

	turbo_bdev = FDEV(F2FS_TURBO_DEV).bdev;

	ret = blk_lld_tz_query(turbo_bdev, (char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret < 0) {
		f2fs_msg(sbi->sb, KERN_ERR,
				"%s%s:failed to get turbo status.ret = %d",
				TZ_LOG_TAG, __func__, ret);
		goto out;
	}

	if (!status->enable) {
		f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:TZ is already disabled",
				TZ_LOG_TAG, __func__);
		ret = -1;
		goto out;
	}

	ret = blk_lld_tz_ctrl(turbo_bdev, TZ_FORCE_CLOSE_FLAG, 0);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_ERR,
				"%s%s:Failed to force to close TZ, ret %d",
				TZ_LOG_TAG, __func__, ret);
#ifdef CONFIG_HUAWEI_F2FS_DSM
		if (f2fs_dclient && !dsm_client_ocuppy(f2fs_dclient)) {
			dsm_client_record(f2fs_dclient,
			"[TRUBO] %s:%d blk_lld_tz_ctrl err, err = %d\n",
			__func__, __LINE__, ret);
			dsm_client_notify(f2fs_dclient, DSM_F2FS_NEED_FSCK);
		}
#endif
	} else {
		f2fs_msg(sbi->sb, KERN_INFO,
				"%s%s:Turbo zone is forced closed",
				TZ_LOG_TAG, __func__);
		bd_set_return_time(sbi, true);
		f2fs_close_tz(sbi, false);
	}

out:
	kfree(status);
	return ret;
}

int f2fs_ioc_get_tz_space_info(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	int ret = 0;
	struct f2fs_tz_space_info param = { 0 };

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (!sbi->s_ndevs)
		return -EPERM;

	if (!sbi->tz_info.enabled) {
		f2fs_msg(sbi->sb, KERN_WARNING, "TZ is already disabled\n");
		ret = -1;
		goto out;
	}

	param.device_segment = FDEV(F2FS_TURBO_DEV).total_segments;
	param.tz_total_segment = sbi->tz_info.total_segs;
	param.reserved_segment = param.device_segment - param.tz_total_segment;

	if (copy_to_user((struct f2fs_tz_space_info __user *)arg, &param,
				sizeof(struct f2fs_tz_space_info)))
		ret = -EFAULT;
out:
	return ret;
}

int f2fs_close_tz(struct f2fs_sb_info *sbi, bool sync)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;

	if (is_tz_closed(tz_info))
		return 0;

	tz_info->enabled = false;
	f2fs_stop_gc_turbo_thread(sbi);

	if (sync)
		return blk_lld_tz_ctrl(FDEV(F2FS_TURBO_DEV).bdev,
				       TZ_FORCE_CLOSE_FLAG, 0);

	return 0;
}

#endif
