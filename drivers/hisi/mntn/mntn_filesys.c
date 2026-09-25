/*
 * Implement file functions used to save log.
 *
 * Copyright (c) Huawei Technologies Co., Ltd. 2013-2019. All rights reserved.
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

#include "mntn_filesys.h"
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/syscalls.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/unistd.h>
#include <linux/sort.h>
#include <linux/hisi/hisi_log.h>
#include <securec.h>
#include <linux/types.h>

#define HISI_LOG_TAG HISI_FILESYS_TAG

/*
 * for: to change the owner of a file/dir
 */
int mntn_filesys_chown(const char *filename, uid_t user, gid_t group)
{
	int iret;

	if (filename == NULL)
		return 0;

	iret = (int)sys_chown((const char __user *)filename, user, group);

	return iret;
}

/*
 * for: to remove a file
 */
int mntn_filesys_rm_file(const char *fullname)
{
	int iret;

	if (fullname == NULL)
		return 0;

	iret = (int)sys_access(fullname, 0);
	if (iret == 0) {
		if (sys_unlink(fullname)) {
			MNTN_FILESYS_PRINT
			    ("mntn_err: Fail to rm the file %s!\n", fullname);
			return -1;
		}
	}

	return 0;
}

/*
 * for: to remove all files in the dir pointed by path
 */
int mntn_filesys_rm_all_file(const char *path)
{
	struct kstat m_stat;
	int fd = -1;
	int nread;
	int bpos;
	int ret;
	char *buf = NULL;
	struct linux_dirent *d = NULL;
	char fullname[MNTN_FILESYS_FNAME_LEN] = { 0 };

	if (path == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: path is NULL when removing all files in dir\n");
		return -1;
	}
	buf = kzalloc(MNTN_FILESYS_DIR_ENTRY_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to kzalloc when removing all files\n");
		return -1;
	}

	/* check path , if path isnt exist, return */
	ret = vfs_stat(path, &m_stat);
	if (ret) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: dir doesn't exist when removing all files, path %s\n", path);
		ret = -1;
		goto free_buf;
	}

	fd = sys_open(path, O_RDONLY, MNTN_FILESYS_DEFAULT_MODE); /* create file */
	if (fd < 0) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to open dir when removing all files, path %s,  fd %d\n",
		     path, fd);
		ret = -1;
		goto free_buf;
	}

	nread = sys_getdents(fd, (struct linux_dirent *)buf, MNTN_FILESYS_DIR_ENTRY_SIZE);
	if (nread == -1) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to getdents when when removing all files\n");
		ret = -1;
		goto close_fd;
	}

	for (bpos = 0; bpos < nread;) {
		d = (struct linux_dirent *)(buf + bpos);
		if (!strncmp(d->d_name, "..", sizeof("..")) ||
			!strncmp(d->d_name, ".", sizeof("."))) {
			bpos += d->d_reclen;
			continue;
		}
		ret = strncat_s(fullname, sizeof(fullname), path, (MNTN_FILESYS_FNAME_LEN - 1) - strlen(fullname));
		if (ret != EOK) {
			MNTN_FILESYS_PRINT("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			ret = -1;
			goto close_fd;
		}

		ret = strncat_s(fullname, sizeof(fullname), d->d_name, strlen(d->d_name));
		if (ret != EOK) {
			MNTN_FILESYS_PRINT("%s():%d:strncat_s fail!\n", __func__, __LINE__);
			ret = -1;
			goto close_fd;
		}

		ret = mntn_filesys_rm_file((const char *)fullname);
		if (ret)
			MNTN_FILESYS_PRINT("mntn_err: fail to mntn_filesys_rm_file\n");

		(void)memset_s(fullname, sizeof(fullname), 0, sizeof(fullname));
		bpos += d->d_reclen;
	}

close_fd:
	sys_close(fd);
free_buf:
	kfree(buf);

	return ret;
}

/*
 * for: to remove a dir
 */
int mntn_filesys_rm_dir(const char *dirname)
{
	char *pdst = (char *)dirname;
	int iret;

	if (dirname == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: dir is null , so fail to open it!\n");
		return -1;
	}
	while (*pdst)
		pdst++;

	pdst--;
	if (*pdst == '/')
		*pdst = '\0';

	iret = sys_rmdir(dirname);
	if (iret != 0)
		MNTN_FILESYS_PRINT
		    ("mntn_err: Fail to remove the dir %s!, iret = %d\n", dirname, iret);
	return iret;
}

/*
 * to create a dir with 1 level name
 */
static int __mntn_filesys_create_dir(const char *path, umode_t umode)
{
	int iret;

	if (path == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: dir is null , so fail to make it!\n");
		return -1;
	}

	iret = sys_mkdir(path, umode);
	if (iret < 0)
		return iret;

	return 0;
}

/*
 * to create a dir with full path name
 */
int mntn_filesys_create_dir(const char *path, umode_t umode)
{
	char cur_path[MNTN_FILESYS_FNAME_LEN] = { 0 };
	int index = 0;
	int iret;

	if (path == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: dir is null , so fail to make it!\n");
		return -1;
	}

	if (*path != '/')
		return -1;

	cur_path[index++] = *path++;

	while (*path != '\0') {
		if (*path == '/') {
			iret = __mntn_filesys_create_dir(cur_path, umode);
			/* the dir may exist if the result is not equal to 0, so needn't return */
			if (iret != 0 && (iret != (-EEXIST)))
				MNTN_FILESYS_PRINT
				    ("mntn_err: Fail to create the dir %s!, result is %d\n", cur_path, iret);
		}
		cur_path[index] = *path;
		path++;
		index++;
	}
	return 0;
}

/*
 * for: to list files or dirs in a dir.
 * input:
 * path: dir name;
 * type: type, file or dir
 * cnt: size of pout_namelist
 * output:
 * pout_namelist: file or dir name list, 64bytes/a name,
 * return:
 * count number of list
 */
int mntn_filesys_dir_list(const char *path, char *pout_namelist, int cnt, int type)
{
	struct kstat m_stat;
	int fd = -1;
	int nread;
	int bpos;
	int ret;
	int tmp_cnt = 0;
	int ulen;
	char *buf = NULL;
	char *ptmp = pout_namelist;
	char d_type;
	struct linux_dirent *d = NULL;

	if (pout_namelist == NULL || path == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn error: pointer is NULL when listing dir\n");
		return 0;
	}
	buf = kzalloc(MNTN_FILESYS_DIR_ENTRY_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to kzalloc when when listing dir\n");
		return 0;
	}

	/* check path , if path isnt exist, return */
	ret = vfs_stat(path, &m_stat);
	if (ret) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: dir doesn't exist when when listing dir\n");
		goto oper_over2;
	}

	fd = sys_open(path, O_RDONLY, MNTN_FILESYS_DEFAULT_MODE); /* create file */
	if (fd < 0) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to open dir when when listing dir, fd %d\n",
		     fd);
		goto oper_over2;
	}

	nread = sys_getdents(fd, (struct linux_dirent *)buf,
			     MNTN_FILESYS_DIR_ENTRY_SIZE);
	if (nread == -1) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to getdents when when listing dir\n");
		goto oper_over1;
	}

	for (bpos = 0; bpos < nread;) {
		if (ptmp > pout_namelist + cnt)
			break;

		d = (struct linux_dirent *)(buf + bpos);
		if (!strncmp(d->d_name, "..", sizeof("..")) ||
			!strncmp(d->d_name, ".", sizeof("."))) {
			bpos += d->d_reclen;
			continue;
		}

		d_type = *(buf + bpos + d->d_reclen - 1);
		if (d_type == type) {
			ulen = (strlen(d->d_name) > MNTN_FILESYS_PURE_DIR_NAME_LEN) ?
			    MNTN_FILESYS_PURE_DIR_NAME_LEN : strlen(d->d_name);
			strncat(ptmp, d->d_name, ulen);
			ptmp = ptmp + MNTN_FILESYS_PURE_DIR_NAME_LEN;
			tmp_cnt++;
		}
		bpos += d->d_reclen;
	}

oper_over1:
	sys_close(fd);
oper_over2:
	kfree(buf);

	return tmp_cnt;
}

/*
 * for: to write the data in pbuf into file named pname. Create the file with mode if it dosen't exist.
 * input:
 * pname: file name including dir;
 * pbuf_vir: VIRTUAL address;
 * ulen: size in byte;
 * mode: open mode.
 * output:
 * return:
 * bytes writen into the file
 */
int mntn_filesys_write_log(const char *pname, const void *pbuf_vir,
				unsigned int ulen, umode_t mode)
{
	int bytes = 0;
	long fd = -1;
	umode_t udefault = (mode == 0) ? MNTN_FILESYS_DEFAULT_MODE : mode;
	int iret;

	if (pname == NULL) {
		MNTN_FILESYS_PRINT("mntn_err: File name is NULL\n");
		return 0;
	}
	if (pbuf_vir == NULL) {
		MNTN_FILESYS_PRINT("mntn_err: pbuf_vir is NULL\n");
		return 0;
	}
	/* create dir, no error if it has been created */
	iret = mntn_filesys_create_dir(pname, udefault);
	if (iret != 0)
		MNTN_FILESYS_PRINT("mntn_err: Fail to create dir!\n");

	fd = sys_open(pname, O_CREAT | O_RDWR, udefault);
	if (fd < 0) {
		MNTN_FILESYS_PRINT("mntn_err: Fail to open file %s\n", pname);
	} else {
		if (sys_lseek((int)(fd), 0, SEEK_END) < 0)
			MNTN_FILESYS_PRINT("%s(): sys_lseek failed\n", __func__);

		bytes = sys_write(fd, pbuf_vir, ulen);
		if (bytes != ulen)
			MNTN_FILESYS_PRINT
			    ("mntn_err: Fail to write all the data into the file %s, %d/%u\n", pname, bytes, ulen);
		iret = sys_fsync(fd);
		if (iret < 0)
			MNTN_FILESYS_PRINT
			    ("mntn_err: Fail to sys_fsync data when saving log\n");

		sys_close(fd);
	}
	return bytes;
}

/*
 * for: used for sorting dirs with name ascendingly.
 */
int mntn_str_cmp(const void *a, const void *b)
{
	if (a == NULL || b == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: pointer a or pointer b is null!\n");
		return -1;
	}

	return strncmp((char *)a, (char *)b, MNTN_FILESYS_PURE_DIR_NAME_LEN);
}

/*
 * for: rm old log of a dir, if the total number of log has been the max
 * input:
 * path: dir name
 * unumber: the max number
 */
void mntn_rm_old_log(const char *ppath, unsigned int unumber)
{
	char fullpath_arr[MNTN_FILESYS_FNAME_LEN + 1] = { 0 };
	int i;
	int iret;
	int tmp_cnt = MNTN_FILESYS_MAX_CYCLE * MNTN_FILESYS_PURE_DIR_NAME_LEN;
	char *pbuff = NULL;

	if (ppath == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: path is NULL when removing old log\n");
		return;
	}
	pbuff = kzalloc(tmp_cnt, GFP_KERNEL);
	if (pbuff == NULL) {
		MNTN_FILESYS_PRINT
		    ("mntn_err: fail to kzalloc when removing old log\n");
		return;
	}
	tmp_cnt = mntn_filesys_dir_list(ppath, pbuff, tmp_cnt, DT_DIR);
	if ((unsigned int)tmp_cnt >= unumber) {
		sort((void *)pbuff, tmp_cnt, MNTN_FILESYS_PURE_DIR_NAME_LEN, mntn_str_cmp, NULL);
		for (i = 0; (unsigned int)i < (tmp_cnt - unumber + 1); i++) {
			strncat(fullpath_arr, ppath, (MNTN_FILESYS_FNAME_LEN) - strlen(fullpath_arr));
			strncat(fullpath_arr, (const char *)(pbuff + (long)i * MNTN_FILESYS_PURE_DIR_NAME_LEN),
				(MNTN_FILESYS_FNAME_LEN) - strlen(fullpath_arr));
			strncat(fullpath_arr, "/", (MNTN_FILESYS_FNAME_LEN) - strlen(fullpath_arr));

			iret = mntn_filesys_rm_all_file(fullpath_arr);
			iret += mntn_filesys_rm_dir(fullpath_arr);
			if (iret != 0)
				MNTN_FILESYS_PRINT("mntn_err: fail to rm dir, %d", i);
			(void)memset_s((void *)fullpath_arr, MNTN_FILESYS_FNAME_LEN + 1, 0, MNTN_FILESYS_FNAME_LEN + 1);
		}
	}
	kfree(pbuff);
}
