/*
 * bfmr_common.c
 *
 * define common interface for BFMR (Boot Fail Monitor and Recovery)
 *
 * Copyright (c) 2016-2019 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/rtc.h>
#include <linux/statfs.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <crypto/hash.h>
#include <crypto/sha.h>
#include <chipset_common/bfmr/public/bfmr_public.h>
#include <chipset_common/bfmr/common/bfmr_common.h>
#include <log/log_usertype.h>

#define BFM_PROC_MOUNTS_PATH           "/proc/mounts"
#define BFM_RW_FLAGS                   "rw,"
#define BFM_HISI_WAIT_FOR_VERSION_PART_TIMEOUT 40
#define ASC_TIME_COUNT                         32

static bool is_first_get_usertype = true;
static bfmr_partition_mount_result_info_t s_bfmr_mount_result[] = {
	{ "/log", false },
	{ "/data", false },
	{ "/splash2", false },
};

static long bfmr_full_rw_file(int fd, char *buf,
		size_t buf_size, bool read_file);
static long bfmr_full_rw_file_with_file_path(const char *pfile_path,
		char *buf, size_t buf_size, bool read_file);
static int bfmr_rw_rrecord_misc_msg(bfmr_rrecord_misc_msg_param_t *pparam,
		bool read_misc);

int bfmr_sha256(unsigned char *pout, unsigned int out_len,
	const void *pin, unsigned long in_len)
{
	struct crypto_shash *tfm = NULL;
	struct shash_desc *desc = NULL;
	int ret;
	size_t desc_size;

	if (pout == NULL || out_len < BFMR_SHA256_HASH_LEN || pin == NULL) {
		BFMR_PRINT_ERR("invalid params!");
		ret = -1;
		goto __out;
	}
	tfm = crypto_alloc_shash("sha256", 0, 0);
	if (IS_ERR(tfm)) {
		BFMR_PRINT_ERR("Unable to allocate hash memory (%ld)",
			       PTR_ERR(tfm));
		ret = -1;
		goto __out;
	}
	desc_size = crypto_shash_descsize(tfm) + sizeof(*desc);
	desc = kzalloc(desc_size, GFP_KERNEL);
	if (desc == NULL) {
		BFMR_PRINT_ERR("kzalloc desc failed\n");
		ret = -1;
		goto __out;
	}
	desc->tfm = tfm;
	desc->flags = 0;
	ret = crypto_shash_init(desc);
	if (ret < 0) {
		BFMR_PRINT_ERR("crypto_shash_init failed, ret=%d\n", ret);
		goto __out;
	}
	ret = crypto_shash_update(desc, pin, in_len);
	if (ret < 0) {
		BFMR_PRINT_ERR("crypto_shash_update() failed, ret=%d\n", ret);
		goto __out;
	}
	ret = crypto_shash_final(desc, pout);
	if (ret)
		BFMR_PRINT_ERR("crypto_shash_final() failed, ret=%d\n", ret);

__out:
	if (tfm != NULL) {
		crypto_free_shash(tfm);
		tfm = NULL;
	}
	if (desc != NULL) {
		kfree(desc);
		desc = NULL;
	}
	return ret;
}

void bfmr_change_own_mode(char *path, int uid, int gid, int mode)
{
	mm_segment_t old_fs;
	int ret = -1;

	if (unlikely((!path) || ((int)uid == -1) || ((int)gid == -1))) {
		BFMR_PRINT_INVALID_PARAMS("path or uid or gid error.\n");
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	ret = sys_chown(path, uid, gid);
	if (ret != 0) {
		BFMR_PRINT_ERR("sys_chown [%s] failed, ret: %d\n", path, ret);
		goto __out;
	}

	ret = sys_chmod(path, mode);
	if (ret != 0) {
		BFMR_PRINT_ERR("sys_chmod [%s] failed, ret: %d\n", path, ret);
		goto __out;
	}

__out:
	set_fs(old_fs);
}

void bfmr_change_file_ownership(char *path, uid_t uid, gid_t gid)
{
	mm_segment_t old_fs;
	int ret = -1;

	if (unlikely((!path) || ((int)uid == -1) || ((int)gid == -1))) {
		BFMR_PRINT_INVALID_PARAMS("path or uid or gid error.\n");
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_chown(path, uid, gid);
	if (ret != 0)
		BFMR_PRINT_ERR("sys_chown [%s] failed, ret: %d\n", path, ret);
	set_fs(old_fs);
}

void bfmr_change_file_mode(char *path, umode_t mode)
{
	mm_segment_t old_fs;
	int ret = -1;

	if (unlikely(!path)) {
		BFMR_PRINT_INVALID_PARAMS("path.\n");
		return;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_chmod(path, mode);
	if (ret != 0)
		BFMR_PRINT_ERR("sys_chmod [%s] failed, ret: %d\n", path, ret);
	set_fs(old_fs);
}

int bfmr_get_file_ownership(char *pfile_path, uid_t *puid, gid_t *pgid)
{
	mm_segment_t old_fs;
	bfm_stat_t statbuf = { 0 };
	int ret = -1;

	if (unlikely((!pfile_path) || (!puid) || (!pgid))) {
		BFMR_PRINT_INVALID_PARAMS("path or puid or pgid error\n");
		return ret;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = bfm_sys_lstat(pfile_path, &statbuf);
	if (ret != 0) {
		BFMR_PRINT_ERR("bfm_sys_lstat [%s] failed, ret: %d\n",
			       pfile_path, ret);
		goto __out;
	} else {
		*puid = statbuf.st_uid;
		*pgid = statbuf.st_gid;
	}

__out:
	set_fs(old_fs);

	return ret;
}

int bfmr_read_emmc_raw_part(const char *dev_path,
			    unsigned long long offset, char *buf,
				unsigned long long buf_size)
{
	int fd = -1;
	int ret = -1;
	long bytes_read = 0L;
	long seek_result = 0L;
	mm_segment_t fs;

	if (unlikely(!dev_path) || unlikely(!buf)) {
		BFMR_PRINT_INVALID_PARAMS("dev_path or buf error\n");
		return ret;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	/* 1. open file for reading */
	fd = sys_open(dev_path, O_RDONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("Open file [%s] failed\n", dev_path);
		goto __out;
	}

	/* 2. seek file to the valid position */
	seek_result = sys_lseek(fd, (off_t) offset, SEEK_SET);
	if ((off_t) offset != seek_result) {
		BFMR_PRINT_ERR("lseek file [%s] failed! seek_result: %ld it should be: %ld\n",
		     dev_path, (long)seek_result, (long)offset);
		goto __out;
	}

	/* 3. read data from file */
	bytes_read = bfmr_full_read(fd, buf, buf_size);
	if ((long long)buf_size != bytes_read) {
		BFMR_PRINT_ERR("read file [%s] failed!bytes_read: %ld, it should be: %lld\n",
		     dev_path, bytes_read, (long long)buf_size);
		goto __out;
	}

	/* 4. read successfully, modify the value of ret */
	ret = 0;

__out:
	if (fd >= 0)
		sys_close(fd);

	set_fs(fs);
	return ret;
}

int bfmr_write_emmc_raw_part(const char *dev_path,
		unsigned long long offset, char *buf,
		unsigned long long buf_size)
{
	int fd = -1;
	int ret = -1;
	off_t seek_result;
	long bytes_written;
	mm_segment_t fs;

	if (unlikely(!dev_path) || unlikely(!buf)) {
		BFMR_PRINT_INVALID_PARAMS("dev_path or buf error\n");
		return -1;
	}

	fs = get_fs();
	set_fs(KERNEL_DS);

	/* 1. open file for writing */
	fd = sys_open(dev_path, O_WRONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("Open file [%s] failed!fd: %d\n", dev_path, fd);
		goto __out;
	}

	/* 2. seek file to the valid position */
	seek_result = sys_lseek(fd, (off_t) offset, SEEK_SET);
	if ((off_t) offset != seek_result) {
		BFMR_PRINT_ERR("lseek file [%s] offset failed!seek_result: %ld, it should be: %ld\n",
		     dev_path, (long)seek_result, (long)offset);
		goto __out;
	}

	/* 3. write data to file */
	bytes_written = bfmr_full_write(fd, buf, buf_size);
	if ((long)buf_size != bytes_written) {
		BFMR_PRINT_ERR("write file [%s] failed!bytes_written: %ld, it should be: %ld\n",
		     dev_path, (long)bytes_written, (long)buf_size);
		goto __out;
	}

	/* 4. write successfully, modify the value of ret */
	ret = 0;

__out:
	if (fd >= 0) {
		sys_fsync(fd);
		sys_close(fd);
	}

	set_fs(fs);

	return ret;
}

bool bfmr_is_file_existed(const char *pfile_path)
{
	mm_segment_t old_fs;
	bfm_stat_t st;
	int ret;

	if (unlikely(!pfile_path)) {
		BFMR_PRINT_INVALID_PARAMS("pfile_path\n");
		return false;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	memset((void *)&st, 0, sizeof(st));
	ret = bfm_sys_lstat(pfile_path, &st);
	set_fs(old_fs);

	return (ret == 0) ? (true) : (false);
}

bool bfmr_is_dir_existed(const char *pdir_path)
{
	return bfmr_is_file_existed(pdir_path);
}

int bfmr_save_log(const char *log_path, const char *filename,
		void *buf, unsigned int len, unsigned int is_append)
{
	int ret = -1;
	int fd = -1;
	int flags = 0;
	mm_segment_t old_fs;
	char *path = NULL;
	long bytes_write = 0L;

	BFMR_PRINT_ENTER();
	if (unlikely(!log_path) || unlikely(!filename) ||
	    unlikely(!buf) || unlikely(len <= 0)) {
		BFMR_PRINT_INVALID_PARAMS("log_path or filename or buf or len\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	path = bfmr_malloc(BFMR_TEMP_BUF_LEN);
	if (!path) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		goto __out;
	}

	memset(path, 0, BFMR_TEMP_BUF_LEN);
	snprintf(path, BFMR_TEMP_BUF_LEN - 1, "%s/%s", log_path, filename);
	flags = O_CREAT | O_WRONLY | (is_append ? O_APPEND : O_TRUNC);

	/* 1. open file for writing */
	fd = sys_open(path, flags, BFMR_FILE_LIMIT);
	if (fd < 0) {
		BFMR_PRINT_ERR("Open file [%s] failed!fd: %d\n", path, fd);
		goto __out;
	}

	/* 2. seek file to the valid position */
	sys_lseek(fd, 0, (is_append != 0) ? SEEK_END : SEEK_SET);

	/* 3. write data to file */
	bytes_write = bfmr_full_write(fd, buf, len);
	if ((long)len != bytes_write) {
		BFMR_PRINT_ERR("write file [%s] failed!bytes_write: %ld, it should be: %ld\n",
		     path, (long)bytes_write, (long)len);
		goto __out;
	}

	/* 4. write successfully, modify the value of ret */
	ret = 0;

__out:
	if (fd >= 0) {
		sys_sync();
		sys_close(fd);
	}

	set_fs(old_fs);

	bfmr_change_own_mode(path, BFMR_AID_ROOT,
			     BFMR_AID_SYSTEM, BFMR_FILE_LIMIT);

	if (!path)
		bfmr_free(path);

	BFMR_PRINT_EXIT();

	return ret;
}

long bfmr_get_proc_file_length(const char *pfile_path)
{
	char c;
	mm_segment_t old_fs;
	int fd = -1;
	long file_size = 0;

	if (unlikely(!pfile_path)) {
		BFMR_PRINT_INVALID_PARAMS("pfile_path\n");
		return 0;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(pfile_path, O_RDONLY, 0);
	if (fd < 0)
		goto __out;

	while (sys_read(fd, (char *)&c, sizeof(char)) != 0)
		file_size++;

__out:
	if (fd >= 0)
		sys_close(fd);

	set_fs(old_fs);

	return file_size;
}


void bfmr_set_mount_state(const char *bfmr_mount_point, bool mount_result,
                                            unsigned int size)
{
	int i;
	int count = ARRAY_SIZE(s_bfmr_mount_result);

	if (unlikely(!bfmr_mount_point)) {
		BFMR_PRINT_INVALID_PARAMS("bfmr_mount_name\n");
		return;
	}

	for (i = 0; i < count; i++) {
		if (!strncmp(s_bfmr_mount_result[i].mount_point,
			     bfmr_mount_point, BFMR_MOUNT_NAME_SIZE))
			s_bfmr_mount_result[i].mount_result = mount_result;
	}
}

bool bfmr_is_part_mounted_rw(const char *pmount_point)
{
	int i;
	int count = ARRAY_SIZE(s_bfmr_mount_result);
	bool part_mounted_rw = false;

	if (unlikely(!pmount_point)) {
		BFMR_PRINT_INVALID_PARAMS("pmount_point\n");
		return part_mounted_rw;
	}

	for (i = 0; i < count; i++) {
		if (!strncmp(s_bfmr_mount_result[i].mount_point,
			     pmount_point, BFMR_MOUNT_NAME_SIZE))
			part_mounted_rw = s_bfmr_mount_result[i].mount_result;
	}

	return part_mounted_rw;
}

long bfmr_get_file_length(const char *pfile_path)
{
	mm_segment_t old_fs;
	bfm_stat_t st;
	int ret = -1;

	if (unlikely(!pfile_path)) {
		BFMR_PRINT_INVALID_PARAMS("pfile_path\n");
		return 0L;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	memset((void *)&st, 0, sizeof(st));
	ret = bfm_sys_lstat(pfile_path, &st);
	if (ret != 0) {
		BFMR_PRINT_ERR("sys_newlstat [%s] failed![ret = %d]\n",
			       pfile_path, ret);
		set_fs(old_fs);
		return 0L;
	}

	set_fs(old_fs);

	return (long)st.st_size;
}

long long bfmr_get_fs_available_space(const char *pmount_point)
{
	int ret = -1;
	struct statfs statfs_buf;
	long long bytes_avail = 0LL;
	mm_segment_t old_fs;

	if (unlikely(!pmount_point)) {
		BFMR_PRINT_INVALID_PARAMS("pmount_point\n");
		return 0LL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	memset((void *)&statfs_buf, 0, sizeof(statfs_buf));
	ret = sys_statfs(pmount_point, &statfs_buf);
	if (ret != 0) {
		BFMR_PRINT_ERR("sys_statfs [%s] failed\n", pmount_point);
		goto __out;
	}

	bytes_avail = (long long)statfs_buf.f_bavail *
		(long long)statfs_buf.f_bsize;

__out:
	set_fs(old_fs);

	return bytes_avail;
}

bool bfmr_is_part_ready_without_timeout(char *dev_name)
{
	char *part_full_path = NULL;
	bool find_part = false;

	part_full_path = bfmr_malloc(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (!part_full_path) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		return find_part;
	}

	memset((void *)part_full_path, 0, BFMR_DEV_FULL_PATH_MAX_LEN + 1);

	(void)bfmr_get_device_full_path(dev_name, part_full_path,
					BFMR_DEV_FULL_PATH_MAX_LEN);
	if (bfmr_is_file_existed(part_full_path))
		find_part = true;

	bfmr_free(part_full_path);
	return find_part;
}

int bfmr_wait_for_part_mount_without_timeout(const char *pmount_point)
{
	bool part_mounted_rw = false;

	if (unlikely(!pmount_point)) {
		BFMR_PRINT_INVALID_PARAMS("pmount_point\n");
		return -1;
	}

	part_mounted_rw = bfmr_is_part_mounted_rw(pmount_point);
	if (part_mounted_rw)
		BFMR_PRINT_KEY_INFO("Line: %d [%s] has been mounted rw\n",
				    __LINE__, pmount_point);
	else
		BFMR_PRINT_KEY_INFO("Line: %d [%s] has not been mounted rw\n",
				    __LINE__, pmount_point);

	return (part_mounted_rw) ? (0) : (-1);
}

int bfmr_wait_for_part_mount_with_timeout(const char *pmount_point,
					  int timeouts)
{
	bool part_mounted_rw = false;

	if (unlikely(!pmount_point)) {
		BFMR_PRINT_INVALID_PARAMS("pmount_point\n");
		return -1;
	}

	while (timeouts > 0) {
		part_mounted_rw = bfmr_is_part_mounted_rw(pmount_point);
		if (part_mounted_rw) {
			BFMR_PRINT_ERR("Line: %d [%s] has been mounted rw\n",
				       __LINE__, pmount_point);
			break;
		}

		current->state = TASK_INTERRUPTIBLE;
		/* wait for 1 second */
		(void)schedule_timeout(HZ);
		timeouts--;
	}

	if (!part_mounted_rw)
		BFMR_PRINT_ERR("failed to check if [%s] has been mounted rw or not\n",
		     pmount_point);

	return (part_mounted_rw) ? (0) : (-1);
}

static int __bfmr_create_dir(char *path)
{
	int fd = -1;
	mm_segment_t old_fs;

	if (unlikely(!path)) {
		BFMR_PRINT_INVALID_PARAMS("path\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_access(path, 0);
	if (fd != 0) {
		fd = sys_mkdir(path, BFMR_DIR_LIMIT);
		if (fd < 0) {
			BFMR_PRINT_ERR("create dir [%s] failed! [ret = %d]\n",
				       path, fd);
			set_fs(old_fs);
			return -1;
		}
	}

	set_fs(old_fs);

	return 0;
}

static int bfmr_create_dir(const char *path)
{
	char *cur_path = NULL;
	int index = 0;

	if (unlikely(!path)) {
		BFMR_PRINT_INVALID_PARAMS("path\n");
		return -1;
	}

	if (*path != '/')
		return -1;

	cur_path = bfmr_malloc(BFMR_MAX_PATH + 1);
	if (!cur_path) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		return -1;
	}
	memset(cur_path, 0, BFMR_MAX_PATH + 1);

	cur_path[index++] = *path++;
	while (*path != '\0') {
		if (*path == '/')
			__bfmr_create_dir(cur_path);

		cur_path[index] = *path;
		path++;
		index++;
	}

	__bfmr_create_dir(cur_path);
	bfmr_free(cur_path);

	return 0;
}

int bfmr_create_log_path(char *path)
{
	int ret;

	if (unlikely(!path)) {
		BFMR_PRINT_INVALID_PARAMS("path\n");
		return -1;
	}

	if (bfmr_is_dir_existed(path)) {
		bfmr_change_own_mode(path, BFMR_AID_ROOT,
				     BFMR_AID_SYSTEM, BFMR_DIR_LIMIT);
		return 0;
	}

	ret = bfmr_create_dir(path);
	if (ret == 0)
		bfmr_change_own_mode(path, BFMR_AID_ROOT,
				     BFMR_AID_SYSTEM, BFMR_DIR_LIMIT);

	BFMR_PRINT_KEY_INFO("Create dir [%s] %s\n", path,
			    (ret != 0) ? ("failed") : ("successfully"));

	return ret;
}

char *bfmr_convert_rtc_time_to_asctime(unsigned long long rtc_time)
{
	struct rtc_time tm;
	static char asctime[ASC_TIME_COUNT];

	memset((void *)asctime, 0, sizeof(asctime));
	memset((void *)&tm, 0, sizeof(struct rtc_time));
	rtc_time_to_tm(rtc_time, &tm);
	snprintf(asctime, sizeof(asctime) - 1, "%04d%02d%02d%02d%02d%02d",
		 tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		 tm.tm_hour, tm.tm_min, tm.tm_sec);

	return asctime;
}

char *bfmr_reverse_find_string(char *psrc, const char *pstr_to_be_found)
{
	char *ptemp = psrc;
	char *pprev = NULL;
	unsigned int len = 0;

	if (unlikely((!psrc) || (!pstr_to_be_found))) {
		BFMR_PRINT_INVALID_PARAMS("psrc or pstr_to_be_found\n");
		return NULL;
	}

	len = strlen(pstr_to_be_found);
	ptemp = strstr(ptemp, pstr_to_be_found);
	while (ptemp) {
		pprev = ptemp;
		if (strlen(ptemp) < len)
			break;
		ptemp += len;
		ptemp = strstr(ptemp, pstr_to_be_found);
	}

	return pprev;
}

bool bfm_get_symbol_link_path(const char *file_path,
		char *psrc_path, size_t src_path_size)
{
	mm_segment_t old_fs;
	bool ret = false;

	if (unlikely((!file_path) || (!psrc_path) || (src_path_size == 0))) {
		BFMR_PRINT_INVALID_PARAMS("file_path or src_path\n");
		return false;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (sys_readlink(file_path, psrc_path, src_path_size - 1) > 0)
		ret = true;

	set_fs(old_fs);

	return ret;
}

static long bfmr_full_rw_file(int fd, char *buf,
			      size_t buf_size, bool read_file)
{
	mm_segment_t old_fs;
	long bytes_total_to_rw = (long)buf_size;
	long bytes_total_rw = 0L;
	long bytes_this_time;
	char *ptemp = buf;

	if (unlikely((fd < 0) || (!buf))) {
		BFMR_PRINT_INVALID_PARAMS("fd or buf\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	while (bytes_total_to_rw > 0) {
		bytes_this_time = read_file ?
			sys_read(fd, ptemp, bytes_total_to_rw) :
			sys_write(fd, ptemp, bytes_total_to_rw);
		if (read_file ?
				(bytes_this_time <= 0) :
				(bytes_this_time < 0)) {
			BFMR_PRINT_ERR("%s\n", read_file ?
				((bytes_this_time == 0) ? ("End of file\n") :
				("Failed to read file")) : ("Failed to write file"));
			break;
		}
		ptemp += bytes_this_time;
		bytes_total_to_rw -= bytes_this_time;
		bytes_total_rw += bytes_this_time;
	}
	set_fs(old_fs);

	return bytes_total_rw;
}

long bfmr_full_read(int fd, char *buf, size_t buf_size)
{
	return bfmr_full_rw_file(fd, buf, buf_size, true);
}

long bfmr_full_write(int fd, char *buf, size_t buf_size)
{
	return bfmr_full_rw_file(fd, buf, buf_size, false);
}

static long bfmr_full_rw_file_with_file_path(const char *pfile_path,
		char *buf, size_t buf_size, bool read_file)
{
	int bytes_total_rw = -1;
	int fd = -1;
	mm_segment_t old_fs;

	if (unlikely((!pfile_path) || (!buf))) {
		BFMR_PRINT_INVALID_PARAMS("pfile_path or buf\n");
		return -1;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	fd = sys_open(pfile_path, read_file ? O_RDONLY : O_WRONLY, 0);
	if (fd < 0) {
		BFMR_PRINT_ERR("Open file [%s] failed! ret: %d\n",
				pfile_path, fd);
		goto __out;
	} else {
		if (read_file)
			bytes_total_rw = bfmr_full_read(fd, buf, buf_size);
		else
			bytes_total_rw = bfmr_full_write(fd, buf, buf_size);
	}

__out:
	if (fd >= 0)
		sys_close(fd);

	set_fs(old_fs);

	return bytes_total_rw;
}

long bfmr_full_read_with_file_path(const char *pfile_path,
		char *buf, size_t buf_size)
{
	return bfmr_full_rw_file_with_file_path(pfile_path, buf,
			buf_size, true);
}

long bfmr_full_write_with_file_path(const char *pfile_path,
		char *buf, size_t buf_size)
{
	return bfmr_full_rw_file_with_file_path(pfile_path, buf,
			buf_size, false);
}

void bfmr_unlink_file(char *pfile_path)
{
	mm_segment_t oldfs;
	int ret;

	if (unlikely(!pfile_path)) {
		BFMR_PRINT_INVALID_PARAMS("pfile_path\n");
		return;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_unlink(pfile_path);
	BFMR_PRINT_KEY_INFO("unlink [%s], ret: [%d]\n", pfile_path, ret);
	set_fs(oldfs);
}

int bfmr_get_uid_gid(uid_t *puid, gid_t *pgid)
{
	mm_segment_t oldfs;

	if (unlikely((!puid) || (!pgid))) {
		BFMR_PRINT_INVALID_PARAMS("puid or pgid\n");
		return -1;
	}

	oldfs = get_fs();
	set_fs(KERNEL_DS);
	*puid = sys_getuid();
	*pgid = sys_getgid();
	set_fs(oldfs);

	return 0;
}

static int bfmr_rw_rrecord_misc_msg(bfmr_rrecord_misc_msg_param_t *pparam,
		bool read_misc)
{
	int ret;
	char *dev_path = NULL;
	unsigned long long buf_length;

	if (!pparam) {
		BFMR_PRINT_INVALID_PARAMS("reason_param\n");
		return -1;
	}

	dev_path = bfmr_malloc(BFMR_DEV_FULL_PATH_MAX_LEN + 1);
	if (!dev_path) {
		BFMR_PRINT_ERR("bfmr_malloc failed\n");
		return -1;
	}
	memset((void *)dev_path, 0, BFMR_DEV_FULL_PATH_MAX_LEN + 1);

	ret = bfmr_get_device_full_path(BFR_RRECORD_PART_NAME, dev_path,
			BFMR_DEV_FULL_PATH_MAX_LEN);
	if (ret != 0) {
		BFMR_PRINT_ERR("get full path of device [%s] failed\n",
				BFR_RRECORD_PART_NAME);
		goto __out;
	}

	buf_length = (unsigned long long)sizeof(bfmr_rrecord_misc_msg_param_t);
	if (read_misc)
		ret = bfmr_read_emmc_raw_part(dev_path, BFR_MISC_PART_OFFSET,
				(char *)pparam, buf_length);
	else
		ret = bfmr_write_emmc_raw_part(dev_path, BFR_MISC_PART_OFFSET,
				(char *)pparam, buf_length);

	if (ret != 0) {
		BFMR_PRINT_ERR("%s [%s] failed!\n", dev_path,
				(read_misc) ? ("read") : ("write"));
		goto __out;
	}

__out:
	if (dev_path)
		bfmr_free(dev_path);

	return ret;
}

int bfmr_read_rrecord_misc_msg(bfmr_rrecord_misc_msg_param_t *pparam)
{
	return bfmr_rw_rrecord_misc_msg(pparam, true);
}

int bfmr_write_rrecord_misc_msg(bfmr_rrecord_misc_msg_param_t *pparam)
{
	return bfmr_rw_rrecord_misc_msg(pparam, false);
}

char *bfm_get_boot_stage_name(unsigned int boot_stage)
{
	char *boot_stage_name = NULL;

	if (bfmr_is_bl1_stage(boot_stage))
		boot_stage_name = "BL1";
	else if (bfmr_is_bl2_stage(boot_stage))
		boot_stage_name = "BL2";
	else if (bfmr_is_kernel_stage(boot_stage))
		boot_stage_name = "kernel";
	else if (bfmr_is_native_stage(boot_stage))
		boot_stage_name = "native";
	else if (bfmr_is_android_framework_stage(boot_stage))
		boot_stage_name = "framework";
	else if (bfmr_is_boot_success(boot_stage))
		boot_stage_name = "boot-success";
	else
		boot_stage_name = "unknown";

	return boot_stage_name;
}

static unsigned int bfm_get_version_type(void)
{
	int i;
	unsigned int user_flag;

	if (is_first_get_usertype) {
		for (i = 0; i < BFM_HISI_WAIT_FOR_VERSION_PART_TIMEOUT; i++) {
			user_flag = get_logusertype_flag();
			if (user_flag != 0)
				break;
			msleep(1000);
		}
		is_first_get_usertype = false;
	} else {
		user_flag = get_logusertype_flag();
	}

	return user_flag;
}

bool bfm_is_beta_version(void)
{
	unsigned int usertype = bfm_get_version_type();

	return ((usertype == BETA_USER) || (usertype == OVERSEA_USER));
}

bool bfmr_is_oversea_commercail_version(void)
{
	unsigned int usertype = bfm_get_version_type();

	return ((usertype != BETA_USER) &&
		(usertype != OVERSEA_USER) && (usertype != COMMERCIAL_USER));
}

static inline int bfm_write_reserved_phys_mem(unsigned int magic_num,
		void *phys_addr)
{
	void *paddr = NULL;

	paddr = ioremap_nocache((phys_addr_t)(uintptr_t)phys_addr,
				sizeof(magic_num));
	if (paddr != NULL) {
		writel(magic_num, (void *)paddr);
		iounmap(paddr);
		return 0;
	}

	return -1;
}

int bfm_write_sub_bootfail_magic_num(unsigned int magic_num, void *phys_addr)
{
	return bfm_write_reserved_phys_mem(magic_num, phys_addr);
}

int bfm_write_sub_bootfail_num(unsigned int bootfail_errno, void *phys_addr)
{
	return bfm_write_reserved_phys_mem(bootfail_errno, phys_addr);
}

int bfm_write_sub_bootfail_count(unsigned int bootfail_count, void *phys_addr)
{
	return bfm_write_reserved_phys_mem(bootfail_count, phys_addr);
}

int bfmr_common_init(void)
{
	return 0;
}
