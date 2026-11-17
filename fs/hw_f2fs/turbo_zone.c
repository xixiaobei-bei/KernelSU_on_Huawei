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

#include "huawei_platform/log/imonitor.h"
#include "huawei_platform/log/imonitor_keys.h"

#ifdef CONFIG_F2FS_TURBO_ZONE

#define TZ_EID_TURBO 914012008
#define BD_TIME_MAX_BYTES 64

struct block_device *f2fs_get_bdev(struct f2fs_sb_info *sbi)
{
	struct block_device *bdev = NULL;

	if (sbi->s_ndevs)
		bdev = FDEV(F2FS_TURBO_DEV).bdev;
	else
		bdev = sbi->sb->s_bdev;
	return bdev;
}

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
bool is_ufs_support_tz_v2(struct f2fs_sb_info *sbi)
{
	bool support = false;
	unsigned int feature = 0;
	int ret;

	ret = blk_lld_tz_query(f2fs_get_bdev(sbi), TZ_STATUS,
				(char *)&feature, sizeof(feature));
	if (ret) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"%s%s:failed to get ufs feature, ret = %d",
			TZ_LOG_TAG, __func__, ret);
		goto out;
	}

	if (feature == TZ_VER_2_0)
		support = true;
out:
	return support;
}

void init_f2fs_turbo_v2(struct f2fs_sb_info *sbi)
{
	struct f2fs_turbo_info *turbo_info = &sbi->turbo_info;
	struct tz_status_v1 *status = NULL;
	int ret;

	if (!test_opt(sbi, TURBOZONE_V2))
		return;

	turbo_info->version = F2FS_TURBO_ZONE_V2;
	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return;

	ret = blk_lld_tz_query(f2fs_get_bdev(sbi), TZ_READ_DESC,
				(char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:failed to get turbo status. ret = %d",
			TZ_LOG_TAG, __func__, ret);
		goto out;
	}

	if (!status->enable)
		goto out;

	turbo_info->enable = true;
	f2fs_msg(sbi->sb, KERN_INFO, "%s%s: turbo zone v2 is enabled",
			TZ_LOG_TAG, __func__);
out:
	kfree(status);
}
#endif

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
			(unsigned long)now.tv_sec);
	else
		set_bd_val(sbi, tz_f2fs_trigger_return_time,
			(unsigned long)now.tv_sec);
	bd_mutex_unlock(&sbi->bd_mutex);
}

void f2fs_tz_imonitor_send(struct f2fs_sb_info *sbi)
{
	struct f2fs_bigdata_info *bd = F2FS_BD_STAT(sbi);
	struct imonitor_eventobj* obj = NULL;
	char f2fs_ret_time_str[BD_TIME_MAX_BYTES] = {0};
	char ioctl_ret_time_str[BD_TIME_MAX_BYTES] = {0};
	unsigned int ret = 0;
	unsigned int nz_free_segs = FREE_I(sbi)->free_segments;

	if (is_tz_existed(sbi))
		nz_free_segs = get_free_segs_in_normal_zone(sbi);

	obj = imonitor_create_eventobj(TZ_EID_TURBO);
	if (!obj) {
		f2fs_msg(sbi->sb, KERN_WARNING, "%s create eventobj failed",
			TZ_LOG_TAG);
		return;
	}

	bd_mutex_lock(&sbi->bd_mutex);
	if (snprintf(f2fs_ret_time_str, BD_TIME_MAX_BYTES, "utc:%lu",
		bd->tz_f2fs_trigger_return_time) < 0)
		goto out;
	if (snprintf(ioctl_ret_time_str, BD_TIME_MAX_BYTES, "utc:%lu",
		bd->tz_ioctl_trigger_return_time) < 0)
		goto out;

	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj, "enabled",
		sbi->tz_info.enabled ? 1 : 0);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"switchable", sbi->tz_info.switchable ? 1 : 0);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"total_segs", sbi->tz_info.total_segs);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"free_segs", sbi->tz_info.free_segs);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"reserved_blks", sbi->tz_info.reserved_blks);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"written_valid_blocks", sbi->tz_info.written_valid_blocks);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"nz_free_segs", nz_free_segs);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"tz_bg_gc_count", bd->tz_bg_gc_cnt);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"tz_bg_gc_fail_count", bd->tz_bg_gc_fail_cnt);
	ret = ret | (unsigned int)imonitor_set_param_string_v2(obj,
		"f2fs_trigger_return_time", f2fs_ret_time_str);
	ret = ret | (unsigned int)imonitor_set_param_string_v2(obj,
		"ioctl_trigger_return_time", ioctl_ret_time_str);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"ioctl_migrate_nz_tz_blocks",
		bd->tz_ioctl_migrate_blks[NZ_TO_TZ]);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"ioctl_migrate_tz_nz_blocks",
		bd->tz_ioctl_migrate_blks[TZ_TO_NZ]);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"tz_bg_gc_to_nz_blocks", bd->tz_gc_migrate_blks[NZ_TO_TZ]);
	ret = ret | (unsigned int)imonitor_set_param_integer_v2(obj,
		"nz_gc_to_tz_blocks", bd->tz_gc_migrate_blks[TZ_TO_NZ]);
	if (ret)
		goto out;

	if (imonitor_send_event(obj) < 0)
		f2fs_msg(sbi->sb, KERN_WARNING, "%s send imonitor event failed",
			TZ_LOG_TAG);
	else
		f2fs_msg(sbi->sb, KERN_INFO, "%s send imonitor event ok",
			TZ_LOG_TAG);
out:
	bd_mutex_unlock(&sbi->bd_mutex);
	imonitor_destroy_eventobj(obj);
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

	if (tz_info->enabled)
		recovery_turbo_init(sbi, false);

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (tz_info->enabled || f2fs_is_enabled_tz_v2(sbi)) {
#else
	if (tz_info->enabled) {
#endif
		inode_lock(inode);
		process_tz_flag(turbo, inode, tz_flag);

		inode_unlock(inode);
	} else {
		ret = -EPERM;
	}

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

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_enabled_tz_v2(sbi)) {
		struct tz_cap_info param;
		int ret = f2fs_query_tz_cap(sbi, &param);

		if (!ret)
			free_blocks = param.slc_exist_status ?
					param.remain_slc_blks : 0;
	}
#endif

	return put_user(free_blocks, (u32 __user *)arg);
}

int f2fs_ioc_get_turbo_status(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct block_device *turbo_bdev = NULL;
	struct tz_status_v1 *status = NULL;
	int ret = 0;

	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return -ENOMEM;

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi)) {
		status->enable = sbi->turbo_info.enable;
		goto skip_query;
	}
#endif

	if (sbi->s_ndevs)
		turbo_bdev = FDEV(F2FS_TURBO_DEV).bdev;
	else
		turbo_bdev = sbi->sb->s_bdev;

	ret = blk_lld_tz_query(turbo_bdev, TZ_READ_DESC,
				(char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:failed to get turbo status.ret = %d",
			TZ_LOG_TAG, __func__, ret);
		ret = ret < 0 ? ret : -EAGAIN;
		goto out;
	}

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
skip_query:
#endif
	if (copy_to_user((struct tz_status_v1 __user *)arg, status,
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

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi))
		return -EPERM;
#endif

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
	struct tz_status_v1 *status = NULL;
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

	ret = blk_lld_tz_query(bdev, TZ_READ_DESC,
				(char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_WARNING,
			"%s%s:failed to get turbo status. ret = %d",
			TZ_LOG_TAG, __func__, ret);
		ret = ret < 0 ? ret : -EAGAIN;
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
		f2fs_msg(sbi->sb, KERN_WARNING, "%s%s:tz turn to disabled",
			TZ_LOG_TAG, __func__);
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

void init_f2fs_turbo_v1(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;
	struct seg_entry *sentry = NULL;
	unsigned int start, tz_end_seg;
	unsigned int tz_real_end_seg, tz_tlc_used_seg;

	memset(tz_info, 0, sizeof(struct f2fs_tz_info));
	memset(&sbi->gc_turbo_thread, 0, sizeof(struct f2fs_gc_kthread));
	if (f2fs_get_tz_info(sbi) || tz_info->total_segs == 0)
		return;

	tz_info->version = F2FS_TURBO_ZONE_V1;
	if (FDEV(F2FS_TURBO_DEV).start_blk < MAIN_BLKADDR(sbi)) {
		if (tz_info->total_segs <= FREE_I(sbi)->start_segno) {
			f2fs_msg(sbi->sb, KERN_WARNING,
				"%s%s:Turbo zone is too small",
				TZ_LOG_TAG, __func__);
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

void init_f2fs_turbo_info(struct f2fs_sb_info *sbi)
{
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (is_ufs_support_tz_v2(sbi))
		init_f2fs_turbo_v2(sbi);
	else
#endif
		init_f2fs_turbo_v1(sbi);
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

	inode_lock(inode);
	if (tz_info->enabled)
		ret = f2fs_migrate_file(inode, param.turbo, param.sync);
#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	else if (f2fs_is_enabled_tz_v2(sbi))
		ret = f2fs_migrate_file_v2(inode, param.turbo, param.sync);
#endif
	else
		ret = -EPERM;
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
	struct tz_status_v1 *status = NULL;
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi))
		return -EPERM;
#endif

	if (!sbi->s_ndevs)
		return -EPERM;

	status = kzalloc(F2FS_TZ_STATUS_SIZE, GFP_F2FS_ZERO);
	if (!status)
		return -ENOMEM;

	turbo_bdev = FDEV(F2FS_TURBO_DEV).bdev;

	ret = blk_lld_tz_query(turbo_bdev, TZ_READ_DESC,
				(char *)status, F2FS_TZ_STATUS_SIZE);
	if (ret) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"%s%s:failed to get turbo status.ret = %d",
			TZ_LOG_TAG, __func__, ret);
		ret = ret < 0 ? ret : -EAGAIN;
		goto out;
	}

	if (!status->enable) {
		f2fs_msg(sbi->sb, KERN_WARNING, "%s%s:TZ is already disabled",
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
				"[TURBO] %s:%d blk tz ctrl err, err = %d\n",
				__func__, __LINE__, ret);
			dsm_client_notify(f2fs_dclient, DSM_F2FS_NEED_FSCK);
		}
#endif
	} else {
		f2fs_msg(sbi->sb, KERN_INFO, "%s%s:Turbo zone is forced closed",
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

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi))
		return -EPERM;
#endif

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

static inline bool can_close_tz(struct f2fs_sb_info *sbi)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;

	/* when switchable is 0, we are not allowed to switch SLC to TLC */
	return tz_info->switchable;
}

int f2fs_close_tz(struct f2fs_sb_info *sbi, bool sync)
{
	struct f2fs_tz_info *tz_info = &sbi->tz_info;

	if (is_tz_closed(tz_info))
		return 0;

	tz_info->enabled = false;
	f2fs_stop_gc_turbo_thread(sbi);
	bd_set_return_time(sbi, false);
	f2fs_tz_imonitor_send(sbi);

	if (sync && can_close_tz(sbi))
		return blk_lld_tz_ctrl(FDEV(F2FS_TURBO_DEV).bdev,
				       TZ_FORCE_CLOSE_FLAG, 0);

	return 0;
}

int f2fs_ioc_get_tz_info(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_ioc_tz_info param = { 0 };

	if (!inode_owner_or_capable(inode))
		return -EACCES;

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi)) {
		param.version = sbi->turbo_info.version;
		param.enable = sbi->turbo_info.enable;
	} else
#endif
	{
		param.version = sbi->tz_info.version;
		param.enable = sbi->tz_info.enabled;
	}

	if (copy_to_user((struct f2fs_ioc_tz_info __user *)arg, &param,
				sizeof(struct f2fs_ioc_tz_info)))
		return -EFAULT;
	return 0;
}

unsigned int f2fs_get_turbo_version(struct f2fs_sb_info *sbi)
{
	unsigned int version = (unsigned int)sbi->tz_info.version;

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
	if (f2fs_is_support_tz_v2(sbi)) {
		if (version)
			f2fs_msg(sbi->sb, KERN_WARNING,
				"Non-regular version, continue anyway");
		version = (unsigned int)sbi->turbo_info.version;
	}
#endif
	return version;
}
#endif

#ifdef CONFIG_F2FS_TURBO_ZONE_V2
int f2fs_query_tz_cap(struct f2fs_sb_info *sbi, struct tz_cap_info *cap_info)
{
	struct block_device *bdev = f2fs_get_bdev(sbi);
	int ret;

	ret = blk_lld_tz_query(bdev, TZ_CAP_INFO,
				(char *)cap_info, sizeof(*cap_info));
	if (ret) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"%s%s:failed to get tz cap info, ret = %d",
			TZ_LOG_TAG, __func__, ret);
		ret = ret < 0 ? ret : -EAGAIN;
	}

	return ret;
}

int f2fs_query_tz_blks(struct f2fs_sb_info *sbi, struct tz_blk_info *blk_info,
			block_t pblk, unsigned int len)
{
	struct block_device *bdev = f2fs_get_bdev(sbi);
	int ret;

	blk_info->addr = pblk;
	blk_info->len = len;
	/* ufs cannot translate addr, we need translate to lun addr */
	blk_info->addr += SECTOR_TO_BLOCK(bdev->bd_part->start_sect);
	ret = blk_lld_tz_query(bdev, TZ_BLK_INFO,
				(char *)blk_info, sizeof(*blk_info));
	if (ret) {
		f2fs_msg(sbi->sb, KERN_ERR,
			"%s%s:failed to get tz blk info, ret = %d",
			TZ_LOG_TAG, __func__, ret);
		ret = ret < 0 ? ret : -EAGAIN;
	}

	return ret;
}

int f2fs_ioc_get_tz_slc_info(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct tz_cap_info param = { 0 };
	int ret;

	if (!f2fs_is_support_tz_v2(sbi))
		return -EPERM;

	ret = f2fs_query_tz_cap(sbi, &param);
	if (ret)
		return ret;

	if (copy_to_user((struct tz_cap_info __user *)arg, &param,
				sizeof(struct tz_cap_info)))
		return -EFAULT;
	return 0;
}

#ifdef CONFIG_F2FS_CHECK_FS
static int f2fs_do_get_tz_blk_info(struct inode *inode,
					struct f2fs_ioc_blk_info *param)
{
	struct f2fs_map_blocks map = { 0 };
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct tz_blk_info blk_info;
	int ret = 0, start = 0, len;

	map.m_lblk = param->addr;
	len = param->len;
	/*
	 * hisi scsi use 4 byte blk_info's buf to fill lun id,
	 * so we should make sure buf is greater than 4 bytes.
	 */
	blk_info.buf_len = IOC_BLK_BUF_SIZE;
	blk_info.flags = kzalloc(IOC_BLK_BUF_SIZE, GFP_F2FS_ZERO);
	if (!blk_info.flags)
		return -ENOMEM;

	while (len > 0) {
		map.m_len = len;
		ret = f2fs_map_blocks(inode, &map, 0, F2FS_GET_BLOCK_FIEMAP);
		if (ret)
			goto out;

		if (!is_valid_data_blkaddr(sbi, map.m_pblk) || !map.m_len)
			goto skip;

		ret = f2fs_query_tz_blks(sbi, &blk_info, map.m_pblk, map.m_len);
		if (ret)
			goto out;

		memcpy(&param->flags[start], blk_info.flags, map.m_len);
skip:
		map.m_len = map.m_len ? map.m_len : 1;
		start += map.m_len;
		map.m_lblk += map.m_len;
		len -= map.m_len;
	}

out:
	kfree(blk_info.flags);
	return ret;
}

int f2fs_ioc_get_tz_blk_info(struct file *filp, unsigned long arg)
{
	struct inode *inode = file_inode(filp);
	struct f2fs_sb_info *sbi = F2FS_I_SB(inode);
	struct f2fs_ioc_blk_info param;
	block_t file_blocks = F2FS_BLK_ALIGN(i_size_read(inode));
	int ret = 0;

	if (!inode_owner_or_capable(inode))
		return -EACCES;

	if (!f2fs_is_support_tz_v2(sbi))
		return -EPERM;

	if (f2fs_has_inline_data(inode))
		return -EINVAL;

	if (copy_from_user(&param, (struct f2fs_ioc_blk_info __user *)arg,
				sizeof(param)))
		return -EFAULT;

	if (param.addr >= file_blocks || param.len <= 0)
		return -EINVAL;

	if (param.len > IOC_BLK_BUF_SIZE)
		param.len = IOC_BLK_BUF_SIZE;

	if (param.addr + param.len > file_blocks)
		param.len = file_blocks - param.addr;

	ret = f2fs_do_get_tz_blk_info(inode, &param);
	if (ret)
		goto out;

	if (copy_to_user((struct f2fs_ioc_tz_blk_info __user *)arg, &param,
				sizeof(param)))
		ret = -EFAULT;

out:
	return ret;
}
#endif
#endif
