/*
 * audio_file.h
 *
 * file operations
 *
 * Copyright (c) 2015 Huawei Technologies Co., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 */

#ifndef __AUDIO_FILE_H__
#define __AUDIO_FILE_H__

#include <linux/types.h>

/*
 * audio_create_dir - create all folders or files in path
 * @*path: input path string pointer
 *
 * return: equal 0, success; less than 0, fail
 */
int audio_create_dir(const char *path, unsigned int size);

/*
 * audio_remove_dir - delete all files and folders in path and subpath
 * @*path: input path string pointer
 *
 * return: equal 0, success; less than 0, fail
 */
int audio_remove_dir(const char *path, unsigned int size);

/*
 * audio_get_dir_count - get the number of all files in current path and subpath
 * @*path: input path string pointer
 *
 * return: number of files, success; less than 0, fail
 */
int audio_get_dir_count(const char *path, unsigned int size);
int audio_rename_dir(const char *old_name, unsigned int old_size,
	const char *new_name, unsigned int new_size);

/*
 * audio_append_dir_systime - append systime to root_path, save in full_path
 * @*full_path: store the final full path
 * @*root_path: input root path
 * @full_path_size: full path size
 *
 * return: equal 0, success; less than 0, fail
 */
int audio_append_dir_systime(char *full_path,
	const char *root_path, size_t full_path_size);

/*
 * audio_create_rootpath - append systime to root_path, save in full_path,
 * and create full path
 * @*full_path: store the final full path
 * @*root_path: input root path
 * @full_path_size: full path size
 *
 * return: equal 0, success; less than 0, fail
 */
int audio_create_rootpath(char *full_path, const char *root_path,
	size_t full_path_size);

/*
 * audio_get_dir_count - get the number of all file and folder size
 * in current path and subpath
 * @*path: input path string pointer
 *
 * return: number of all file and folder size, success; less than 0, fail
 */
long long audio_get_dir_size(const char *path, unsigned int size);

#endif

