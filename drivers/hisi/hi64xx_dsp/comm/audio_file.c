/*
 * audio_file.c
 *
 * audio file operation common
 *
 * Copyright (c) 2015-2019 Huawei Technologies Co., Ltd.
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

#include "audio_file.h"

#include <linux/io.h>
#include <linux/fs.h>
#include <linux/of.h>
#include <linux/rtc.h>
#include <linux/err.h>
#include <linux/time.h>
#include <linux/stat.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/syscalls.h>
#include <linux/hisi/audio_log.h>

#define LOG_TAG "audio_file"
#define DIR_NAME_MAX_LEN 1024
#define FILE_NAME_MAX_LEN 200
#define ROOT_UID 0
#define SYSTEM_GID 1000
#define MAX_PCM_SIZE 0x80000000 /* 2G */

/*
 * Before adding a whitelist, make sure that
 * the path length added is not more than FILE_NAME_MAX_LEN
 * or modify FILE_NAME_MAX_LEN
 */
const static char * const white_list_path[] = {
	"/data/log/wakeup_pcm/",
	"/data/hisi_logs/hi64xxdump/",
	"/data/log/codec_dsp/default/",
	"/data/log/codec_dsp/beta_club/",
	"/data/log/codec_dsp/",
	"/data/log/codechifi_total_logs/",
	"/data/log/codec_dsp/visual_tool/"
};

struct linux_dirent {
	unsigned long d_ino; /* inode number */
	unsigned long d_off; /* offset to next linux_dirent */
	unsigned short d_reclen; /* length of this linux_dirent */
	char d_name[0]; /* filename max length 255 */
};

struct directory_info {
	int fd; /* dirent descriptor */
	int dirents_size; /* sum of linux_dirents length */
	char dirents[DIR_NAME_MAX_LEN]; /* store linux_dirents information */
	char path[FILE_NAME_MAX_LEN]; /* full path */
};

struct directory {
	struct directory_info dir_info;

	/*
	 * operator_file - operating interface to file
	 * file: file object, store file operator and file information
	 *
	 * operator_file is a universal operating interface to file,
	 * it can be get_file_size, get_file_count, remove_file_dir,
	 * and you can add new operation to file whatever you like
	 *
	 */
	long long (*operator_file)(const struct directory *file);

	/*
	 * operator_folder - operating interface to folder
	 * folder: folder object,
	 *           store folder operator and folder information
	 *
	 * operator_folder is a universal operating interface to folder,
	 * it can be get_folder_size, get_folder_count, remove_folder_dir,
	 * and you can add new operation to folder whatever you like
	 */
	long long (*operator_folder)(struct directory *folder);
};

struct audio_file {
	struct directory file;
};

struct audio_folder {
	struct directory folder;
};

/* create file object for file operation */
static struct directory *create_file(const char *cur_path, unsigned int size)
{
	mm_segment_t old_fs;
	struct audio_file *file = NULL;

	if (cur_path == NULL) {
		AUDIO_LOGE("path is NULL, create file failed");
		return NULL;
	}

	if (strlen(cur_path) >= FILE_NAME_MAX_LEN || strlen(cur_path) > size) {
		AUDIO_LOGE("cur path len[%zd] err, size is %u",
			strlen(cur_path), FILE_NAME_MAX_LEN);
		return NULL;
	}

	file = kzalloc(sizeof(struct audio_file), GFP_KERNEL);
	if (file == NULL) {
		AUDIO_LOGE("malloc file failed");
		return NULL;
	}

	strncpy(file->file.dir_info.path, cur_path, strlen(cur_path) + 1);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	file->file.dir_info.fd = sys_open(cur_path, O_RDONLY, 0770);
	if (file->file.dir_info.fd < 0) {
		AUDIO_LOGE("open %s failed", cur_path);
		set_fs(old_fs);
		kfree(file);
		return NULL;
	}

	set_fs(old_fs);

	return (struct directory *)file;
}

/* release the space occupied by file object */
static void destory_file(const struct directory *file)
{
	mm_segment_t old_fs;

	if (file == NULL)
		return;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (file->dir_info.fd > 0)
		sys_close(file->dir_info.fd);

	set_fs(old_fs);
	kfree(file);
}

/* create folder object for folder operation */
static struct directory *create_folder(const char *path, unsigned int size)
{
	mm_segment_t old_fs;
	struct audio_folder *folder = NULL;

	if (path == NULL) {
		AUDIO_LOGE("path is NULL, create file failed");
		return NULL;
	}

	if (strlen(path) >= FILE_NAME_MAX_LEN || strlen(path) > size) {
		AUDIO_LOGE("path len[%zd] err, size is %u",
			strlen(path), FILE_NAME_MAX_LEN);
		return NULL;
	}

	folder = kzalloc(sizeof(struct audio_folder), GFP_KERNEL);
	if (folder == NULL) {
		AUDIO_LOGE("malloc folder failed");
		return NULL;
	}

	strncpy(folder->folder.dir_info.path, path, strlen(path) + 1);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	folder->folder.dir_info.fd = sys_open(path, O_RDONLY, 0770);
	if (folder->folder.dir_info.fd < 0) {
		AUDIO_LOGE("open %s failed", path);
		set_fs(old_fs);
		kfree(folder);
		return NULL;
	}

	set_fs(old_fs);

	return (struct directory *)folder;
}

/* release the space occupied by folder object */
static void destory_folder(const struct directory *folder)
{
	mm_segment_t old_fs;

	if (folder == NULL)
		return;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	if (folder->dir_info.fd > 0)
		sys_close(folder->dir_info.fd);

	set_fs(old_fs);
	kfree(folder);
}

/* create file or folder objects based on type */
static struct directory *create_sub_dir_obj(char type, const char *path,
	unsigned int size)
{
	struct directory *direct = NULL;

	if (type == DT_DIR)
		direct = create_folder(path, size);
	else if (type == DT_REG)
		direct = create_file(path, size);
	else
		AUDIO_LOGE("err dir type:%d", type);

	return direct;
}

/* destory file or folder objects based on type */
static void destory_sub_dir_obj(char type, const struct directory *direct)
{
	if (type == DT_DIR)
		destory_folder(direct);
	else if (type == DT_REG)
		destory_file(direct);
	else
		AUDIO_LOGE("err dir type:%d", type);
}

/*
 * loop through to get dirent,
 * operator the dirent with *operator_file or *operator_folder
 *
 */
static long long operate_sub_folder_impl(const struct directory *folder)
{
	char d_type;
	char *sub_path = NULL;
	unsigned int pos = 0;
	long long comm_value = 0;
	struct linux_dirent *dirent = NULL;
	struct directory *sub_folder = NULL;

	sub_path = kzalloc((unsigned long)FILE_NAME_MAX_LEN, GFP_KERNEL);
	if (sub_path == NULL) {
		AUDIO_LOGE("sub path alloc fail");
		return 0;
	}

	dirent = (struct linux_dirent *)(folder->dir_info.dirents + pos);
	/* loop through to get dirent */
	for (pos = 0; pos < (unsigned int)folder->dir_info.dirents_size;
		pos += dirent->d_reclen) {
		dirent = (struct linux_dirent *)(folder->dir_info.dirents + pos);

		/* file type, offset is (d_reclen - 1) */
		d_type = *(folder->dir_info.dirents + pos +
			dirent->d_reclen - 1);

		snprintf(sub_path, FILE_NAME_MAX_LEN - 1, "%s/%s",
			folder->dir_info.path,
			dirent->d_name);

		if (d_type == DT_DIR &&
			(!strncmp(dirent->d_name, ".", sizeof(".")) ||
			!strncmp(dirent->d_name, "..", sizeof("..")) ||
			!strncmp(dirent->d_name, "...", sizeof("..."))))
			continue;

		sub_folder = create_sub_dir_obj(d_type, sub_path, FILE_NAME_MAX_LEN - 1);
		if (sub_folder == NULL)
			continue;

		if (d_type == DT_DIR)
			comm_value += folder->operator_folder(sub_folder);
		else if (d_type == DT_REG)
			comm_value += folder->operator_file(sub_folder);
		else
			AUDIO_LOGE("err dir type:%d", d_type);

		destory_sub_dir_obj(d_type, sub_folder);
	}
	kfree(sub_path);

	return comm_value;
}

/*
 * get all dirents in current folder,
 * and operate this dirents with function operate_sub_folder_impl
 *
 */
static long long operate_sub_folder(struct directory *folder)
{
	long long comm_value = 0;
	mm_segment_t old_fs;

	while (folder->dir_info.dirents_size >= 0) {
		old_fs = get_fs();
		set_fs(KERNEL_DS);

		/* get a batch of dirents */
		folder->dir_info.dirents_size = sys_getdents(
			folder->dir_info.fd,
			(struct linux_dirent *)folder->dir_info.dirents,
			sizeof(folder->dir_info.dirents));

		set_fs(old_fs);
		if (folder->dir_info.dirents_size <= 0)
			break;

		comm_value += operate_sub_folder_impl(folder);
	}

	return comm_value;
}

static long long get_file_size(const struct directory *file)
{
	struct kstat stat;
	mm_segment_t old_fs;

	if (file == NULL) {
		AUDIO_LOGW("create file object fail");
		return -EINVAL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	memset(&stat, 0, sizeof(struct kstat)); /* unsafe_function_ignore: memset */
	if (vfs_stat(file->dir_info.path, &stat) != 0) {
		AUDIO_LOGE("stat failed:%s", file->dir_info.path);
		set_fs(old_fs);
		return 0;
	}

	set_fs(old_fs);

	return stat.size;
}

/* get all file and folder size under the current folder and subfolder */
static long long get_folder_size(struct directory *folder)
{
	struct kstat stat;
	unsigned long long size;
	mm_segment_t old_fs;

	if (folder == NULL) {
		AUDIO_LOGW("create file object fail");
		return -EINVAL;
	}

	folder->operator_file = get_file_size;
	folder->operator_folder = get_folder_size;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	memset(&stat, 0, sizeof(struct kstat)); /* unsafe_function_ignore: memset */
	if (vfs_stat(folder->dir_info.path, &stat) != 0)
		AUDIO_LOGE("stat failed:%s", folder->dir_info.path);

	set_fs(old_fs);

	size = stat.size;
	size += operate_sub_folder(folder);

	return size;
}

/* if file exist, file count increase 1 */
static long long get_file_count(const struct directory *file)
{
	if (file == NULL) {
		AUDIO_LOGE("create file object fail");
		return -EINVAL;
	}

	return 1;
}

/* get the number of files under the current folder and subfolder */
static long long get_folder_count(struct directory *folder)
{
	if (folder == NULL) {
		AUDIO_LOGE("create file object fail");
		return -EINVAL;
	}

	folder->operator_file = get_file_count;
	folder->operator_folder = get_folder_count;

	return operate_sub_folder(folder);
}

static int rm_dir(char *path)
{
	int ret;
	char *pdst = path;
	mm_segment_t old_fs;

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	while (*pdst)
		pdst++;

	pdst--;
	if (*pdst == '/')
		*pdst = '\0';

	ret = sys_rmdir(path);
	if (ret != 0)
		AUDIO_LOGE("del %s failed. ret:%d", path, ret);

	*pdst = '/';

	set_fs(old_fs);

	return ret;
}

/*
 * dlelete hard links to files, when the reference number is 0,
 * the underlying files can be deleted
 *
 */
static long long remove_file_dir(const struct directory *file)
{
	long long ret;
	mm_segment_t old_fs;

	if (file == NULL) {
		AUDIO_LOGE("create file object fail");
		return -EINVAL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	ret = sys_unlink(file->dir_info.path);
	if (ret != 0)
		AUDIO_LOGE("unlink dir %s fail", file->dir_info.path);

	set_fs(old_fs);

	return ret;
}

/*
 * delete hard links to files in current folders and subfolders
 *
 */
static long long remove_folder_dir(struct directory *folder)
{
	long long ret;

	if (folder == NULL) {
		AUDIO_LOGE("create file object fail");
		return -EINVAL;
	}

	folder->operator_file = remove_file_dir;
	folder->operator_folder = remove_folder_dir;

	ret = operate_sub_folder(folder);
	rm_dir(folder->dir_info.path);

	return ret;
}

static int check_valid_path(const char *path, unsigned int size)
{
	int i;

	/* null pointer check */
	if (path == NULL) {
		AUDIO_LOGE("path is null");
		return -EINVAL;
	}

	/* path length check */
	if (strlen(path) >= FILE_NAME_MAX_LEN || strlen(path) > size) {
		AUDIO_LOGE("path is too long, len is: %zd", strlen(path));
		return -EINVAL;
	}

	/* absolute path check */
	for (i = 0; i < strlen(path); i++) {
		/* sizeof("xx")-1 is remove the effect of '\0' */
		if (path[i] == '.' &&
			(!strncmp(path + i, "..", sizeof("..") - 1) ||
			!strncmp(path + i, "...", sizeof("...") - 1))) {
			AUDIO_LOGE("path contain err, invalid path");
			return -EINVAL;
		}
	}

	/* white list check */
	for (i = 0; i < ARRAY_SIZE(white_list_path); i++) {
		if (strncmp(path, white_list_path[i],
			strlen(white_list_path[i])) == 0) {
			AUDIO_LOGI("white list check, success");
			return 0;
		}
	}

	return -EINVAL;
}

/*
 * get_dirents_size - purpose is to determine path type based on return value
 * path:  file or folder path
 *
 * this function calls sys_getdents to get return
 *
 */
static int get_dirents_size(const char *path)
{
	int fd;
	int dirents_size;
	mm_segment_t old_fs;
	char dirents[DIR_NAME_MAX_LEN] = {0};

	if (path == NULL) {
		AUDIO_LOGE("path is null");
		return -EINVAL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_open(path, O_RDONLY, 0770);
	if (fd < 0) {
		AUDIO_LOGW("open %s failed", path);
		set_fs(old_fs);
		return fd;
	}

	dirents_size = sys_getdents(fd, (struct linux_dirent *)dirents,
		sizeof(dirents));

	sys_close(fd);
	set_fs(old_fs);

	return dirents_size;
}

static int create_dir_with_chown(const char *path)
{
	int fd;
	mm_segment_t old_fs;

	if (path == NULL) {
		AUDIO_LOGE("path is null");
		return -EINVAL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);

	fd = sys_access(path, 0);
	if (fd) {
		AUDIO_LOGI("need create dir %s", path);
		fd = sys_mkdir(path, 0770);
		if (fd < 0) {
			AUDIO_LOGE("create dir %s failed, fd:%d", path, fd);
			set_fs(old_fs);
			return -EPERM;
		}
		AUDIO_LOGI("create dir %s successed, fd: %d", path, fd);
	}

	if ((int)sys_chown((const char __user *)path, ROOT_UID, SYSTEM_GID)) {
		AUDIO_LOGE("chown %s failed", path);
		sys_rmdir(path);
		set_fs(old_fs);
		return -EPERM;
	}

	set_fs(old_fs);

	return 0;
}

int audio_create_dir(const char *path, unsigned int size)
{
	char cur_path[FILE_NAME_MAX_LEN] = {0};
	int index = 0;

	if (check_valid_path(path, size) != 0) {
		AUDIO_LOGE("invalid path %s", path);
		return -EINVAL;
	}

	if (*path != '/')
		return -EINVAL;

	cur_path[index++] = *path++;

	while (*path != '\0') {
		if (index >= FILE_NAME_MAX_LEN) {
			AUDIO_LOGE("path is too long");
			return -EINVAL;
		}

		if (*path == '/')
			create_dir_with_chown(cur_path);

		cur_path[index++] = *path++;
	}

	return 0;
}

int audio_rename_dir(const char *old_name, unsigned int old_size,
	const char *new_name, unsigned int new_size)
{
	mm_segment_t old_fs;
	int ret;

	if (check_valid_path(old_name, old_size) ||
		check_valid_path(new_name, new_size)) {
		AUDIO_LOGE("invalid old_name %s or new_name %s",
			old_name, new_name);
		return -EINVAL;
	}

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	ret = sys_rename(old_name, new_name);
	if (ret != 0) {
		AUDIO_LOGE("rename dir from %s to %s fail", old_name, new_name);
		set_fs(old_fs);
		return ret;
	}

	ret = sys_chown(new_name, ROOT_UID, SYSTEM_GID);
	if (ret != 0) {
		AUDIO_LOGE("chown dir %s fail", new_name);
		set_fs(old_fs);
		return ret;
	}

	set_fs(old_fs);

	return 0;
}

int audio_remove_dir(char *path, unsigned int size)
{
	int ret = 0;
	int dirents_size;
	struct directory *file = NULL;
	struct directory *folder = NULL;

	if (check_valid_path(path, size) != 0) {
		AUDIO_LOGE("invalid path %s", path);
		return -EINVAL;
	}

	dirents_size = get_dirents_size(path);

	/* judge path type based on dirents_size, file or folder */
	if (dirents_size == 0) {
		file = create_file(path, size);
		ret = (int)remove_file_dir(file);
		destory_file(file);
	} else if (dirents_size > 0) {
		folder = create_folder(path, size);
		ret = (int)remove_folder_dir(folder);
		destory_folder(folder);
	} else {
		AUDIO_LOGE("get dirents %s failed", path);
	}

	return ret;
}

long long audio_get_dir_size(const char *path, unsigned int size)
{
	int dirents_size;
	unsigned long long dir_size = 0;
	struct directory *file = NULL;
	struct directory *folder = NULL;

	if (check_valid_path(path, size) != 0) {
		AUDIO_LOGE("invalid path %s", path);
		return -1;
	}

	dirents_size = get_dirents_size(path);

	/* judge path type based on dirents_size, file or folder */
	if (dirents_size == 0) {
		file = create_file(path, size);
		dir_size = get_file_size(file);
		destory_file(file);
	} else if (dirents_size > 0) {
		folder = create_folder(path, size);
		dir_size = get_folder_size(folder);
		destory_folder(folder);
	} else {
		AUDIO_LOGW("get dirents %s failed", path);
	}

	return dir_size;
}

int audio_get_dir_count(const char *path, unsigned int size)
{
	int count = 0;
	int dirents_size;
	struct directory *file = NULL;
	struct directory *folder = NULL;

	if (check_valid_path(path, size) != 0) {
		AUDIO_LOGE("invalid path %s", path);
		return -1;
	}

	dirents_size = get_dirents_size(path);

	/* judge path type based on dirents_size, file or folder */
	if (dirents_size == 0) {
		file = create_file(path, size);
		count = (int)get_file_count(file);
		destory_file(file);
	} else if (dirents_size > 0) {
		folder = create_folder(path, size);
		count = (int)get_folder_count(folder);
		destory_folder(folder);
	} else {
		AUDIO_LOGE("get dirents %s failed", path);
	}

	return count;
}

/* append root path by system time */
int audio_append_dir_systime(char *full_path, const char *root_path,
	size_t full_path_size)
{
	struct rtc_time tm;
	struct timeval tv;

	if (full_path == NULL) {
		AUDIO_LOGE("full_path is null");
		return -EINVAL;
	}

	if (root_path == NULL) {
		AUDIO_LOGE("root full_path is null");
		return -EINVAL;
	}

	if (full_path_size == 0 || full_path_size > FILE_NAME_MAX_LEN) {
		AUDIO_LOGE("input dir size error, size: %zd", full_path_size);
		return -EINVAL;
	}

	memset(&tm, 0, sizeof(tm)); /* unsafe_function_ignore: memset */
	memset(&tv, 0, sizeof(tv)); /* unsafe_function_ignore: memset */

	do_gettimeofday(&tv);
	tv.tv_sec -= (long)sys_tz.tz_minuteswest * 60;
	rtc_time_to_tm(tv.tv_sec, &tm);

	snprintf(full_path, full_path_size, "%s%04d%02d%02d%02d%02d%02d/",
		root_path, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
		tm.tm_hour, tm.tm_min, tm.tm_sec);

	return 0;
}

/* append system time to root path, saved in full path and create full path */
int audio_create_rootpath(char *full_path, char *root_path, size_t full_path_size)
{
	if (full_path == NULL) {
		AUDIO_LOGE("full_path is null");
		return -EINVAL;
	}

	if (root_path == NULL) {
		AUDIO_LOGE("root full_path is null");
		return -EINVAL;
	}

	if (full_path_size == 0 || full_path_size > FILE_NAME_MAX_LEN) {
		AUDIO_LOGE("input dir size error, size: %zd", full_path_size);
		return -EINVAL;
	}

	if (audio_get_dir_size(root_path, full_path_size) > MAX_PCM_SIZE)
		audio_remove_dir(root_path, full_path_size);

	if (audio_append_dir_systime(full_path, root_path, full_path_size)) {
		AUDIO_LOGE("append dir name failed");
		return -EINVAL;
	}

	if (audio_create_dir(full_path, full_path_size)) {
		AUDIO_LOGE("create dir or chown %s failed", full_path);
		return -EINVAL;
	}

	return 0;
}

