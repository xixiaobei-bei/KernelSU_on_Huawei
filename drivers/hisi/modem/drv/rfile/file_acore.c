/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */
#include "mdrv_rfile.h"
#include "rfile_server.h"

#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/stat.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/dirent.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/statfs.h>
#include <product_config.h>

#include <osl_types.h>
#include <osl_sem.h>
#include <bsp_icc.h>
#include <bsp_rfile.h>
#include <bsp_slice.h>
#include <bsp_print.h>
#include <securec.h>

#define THIS_MODU mod_rfile


struct rfile_dirent_info_local {
    rfile_dir_s *dirp; /* 目录描述符 */
    bsp_rfile_dirent_s *dirent_bsp;
    int len;                 /* 总长度 */
    int ptr;                 /* 当前偏移 */
    struct list_head stlist; /* 链表节点 */
    rfile_dirent_s dirent_mdrv;
};


struct rfile_mdrv_ctrl {
    struct list_head dir_listhead;
    osl_sem_id sem_dir_list;
};

struct rfile_mdrv_ctrl g_rfile_mdrv_ctrl;

#define DATA_SIZE_1_K ((0x400 / sizeof(bsp_rfile_dirent_s)) * sizeof(bsp_rfile_dirent_s))
#define RFILE_STACK_MAX 12
#define RFILE_PATHLEN_MAX 255

#define RFILE_OPEN_FILE_PERMISSION 0640
#define RFILE_MKDIR_PERMISSION 0750

static int rfile_check_path(const char *path)
{
    const char *d_str = "../";

    if (path == NULL) {
        bsp_err("<%s> path is NULL.\n", __FUNCTION__);
        return -1;
    }
    if (strstr(path, d_str)) {
        bsp_err("<%s> %s is not allow %s.\n", __FUNCTION__, path, d_str);
        return -1;
    }

    return 0;
}

s32 rfile_mkdir_recursive(char *path, u32 pathlen)
{
    s32 ret;
    char *s = path;
    char c;
    unsigned long old_fs;
    u32 i = 0;

    do {
        c = '\0';
        while ((s[i] != '\0') && (i + 1 < pathlen)) {
            if (s[i] == '/') {
                i++;
                c = s[i];
                s[i] = '\0';
                break;
            }
            if (s[i] == '\0') {
                break;
            }
            i++;
        }
        old_fs = get_fs();
        set_fs((mm_segment_t)KERNEL_DS);
        ret = sys_mkdir(path, RFILE_MKDIR_PERMISSION);
        set_fs(old_fs);
        if (ret && ret != -EEXIST) {
            bsp_err("<%s> sys_mkdir %s failed ret %d.\n", __FUNCTION__, path, ret);
            return ret;
        }
        s[i] = c;
    } while (c != '\0');
    return 0;
}

s32 bsp_open(const s8 *path, s32 flags, s32 mode)
{
    s32 ret;
    unsigned long old_fs;
    char *p = NULL;
    char pathtmp[RFILE_PATHLEN_MAX + 1] = {0};

    if (rfile_check_path(path)) {
        return -1;
    }

    if (strlen(path) > RFILE_PATHLEN_MAX) {
        return -1;
    }

    ret = memcpy_s(pathtmp, sizeof(pathtmp), (char *)path, strlen(path));
    if (ret != EOK) {
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
        return -1;
    }

    /* 路径中包含'/'并且不在根目录，则检查当前目录是否存在，不存在则创建目录 */
    p = strrchr(pathtmp, '/');
    if ((p != NULL) && (p != pathtmp)) {
        /* 查看上一级目录是否存在，如果不存在则创建此目录 */
        *p = '\0';
        ret = rfile_mkdir_recursive(pathtmp, sizeof(pathtmp));
        if (ret) {
            bsp_err("<%s> rfile_mkdir_recursive failed.\n", __FUNCTION__);
        }
    }

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);
    ret = sys_open(path, flags, mode);
    set_fs(old_fs);

    return ret;
}

s32 bsp_close(u32 fp)
{
    s32 ret;
    unsigned long old_fs;
    struct file *file_p = NULL;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    file_p = fget(fp);
    if (file_p != NULL && file_p->f_inode) {
        invalidate_mapping_pages(file_p->f_inode->i_mapping, 0, -1);
    }
    if (file_p != NULL) {
        fput(file_p);
    }

    ret = sys_close(fp);

    set_fs(old_fs);

    return ret;
}

s32 bsp_write(u32 fd, const s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_write(fd, ptr, (s32)size);

    set_fs(old_fs);

    return ret;
}

s32 bsp_write_sync(u32 fd, const s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_write(fd, ptr, (s32)size);

    (void)sys_fsync(fd);

    set_fs(old_fs);

    return ret;
}

s32 bsp_read(u32 fd, s8 *ptr, u32 size)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_read(fd, ptr, (s32)size);

    set_fs(old_fs);

    return ret;
}

s32 bsp_lseek(u32 fd, long offset, s32 whence)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_lseek(fd, offset, (u32)whence);

    set_fs(old_fs);

    return ret;
}

s32 bsp_get_file_size(u32 fd)
{
    s32 ret;
    s32 size;

    ret = bsp_lseek(fd, 0, RFILE_SEEK_END);
    if (ret < 0) {
        bsp_err("<%s> fail to seek fd(%d) to end, ret = %d.\n", __FUNCTION__, fd, ret);
        return ret;
    }
    size = ret;
    ret = bsp_lseek(fd, 0, RFILE_SEEK_SET);
    if (ret < 0) {
        bsp_err("<%s> fail to seek fd(%d) to start, ret = %d.\n", __FUNCTION__, fd, ret);
        return ret;
    }

    return size;
}

long bsp_tell(u32 fd)
{
    s32 ret;
    loff_t offset = 0;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_llseek(fd, 0, 0, &offset, SEEK_CUR);
    if (ret) {
        bsp_err("<%s> sys_llseek err. ret =  %d.\n", __FUNCTION__, ret);
    }

    set_fs(old_fs);

    return (long)offset;
}

s32 bsp_remove(const s8 *pathname)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_unlink(pathname);

    set_fs(old_fs);

    return ret;
}

s32 bsp_mkdir(const s8 *dir_name, s32 mode)
{
    s32 ret;
    char pathtmp[RFILE_PATHLEN_MAX + 1] = {0};

    if (strlen(dir_name) > RFILE_PATHLEN_MAX) {
        return -1;
    }

    ret = memcpy_s(pathtmp, sizeof(pathtmp), (char *)dir_name, strlen(dir_name));
    if (ret != EOK) {
        bsp_err("<%s> memcpy_s err. ret =  %d.\n", __FUNCTION__, ret);
        return -1;
    }

    return rfile_mkdir_recursive(pathtmp, sizeof(pathtmp));
}

s32 bsp_rmdir(const s8 *path)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_rmdir(path);

    set_fs(old_fs);

    return ret;
}

s32 bsp_opendir(const s8 *dir_name)
{
    s32 handle;

    unsigned long old_fs;

    if (rfile_check_path(dir_name)) {
        return -1;
    }

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    handle = sys_open(dir_name, RFILE_RDONLY | RFILE_DIRECTORY, 0);

    set_fs(old_fs);

    return handle;
}

s32 bsp_readdir(u32 fd, void *dirent, u32 count)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_getdents64(fd, dirent, count);

    set_fs(old_fs);

    return ret;
}

s32 bsp_closedir(s32 dir)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_close((u32)dir);

    set_fs(old_fs);

    return ret;
}

s32 bsp_access(const s8 *path, s32 mode)
{
    s32 ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_access(path, mode);

    set_fs(old_fs);

    return ret;
}

void rfile_trans_stat(struct rfile_stat_stru *rfile_stat, struct kstat *rfile_kstat)
{
    /* 兼容 32位和64位 */
    rfile_stat->ino = (u64)rfile_kstat->ino;
    rfile_stat->dev = (u32)rfile_kstat->dev;
    rfile_stat->mode = (u16)rfile_kstat->mode;
    rfile_stat->nlink = (u32)rfile_kstat->nlink;
    rfile_stat->uid = *(u32 *)&rfile_kstat->uid;
    rfile_stat->gid = *(u32 *)&rfile_kstat->gid;
    rfile_stat->rdev = (u32)rfile_kstat->rdev;
    rfile_stat->size = (u64)rfile_kstat->size;
    rfile_stat->atime.tv_sec = (u64)rfile_kstat->atime.tv_sec;
    rfile_stat->atime.tv_nsec = (u64)rfile_kstat->atime.tv_nsec;
    rfile_stat->ctime.tv_sec = (u64)rfile_kstat->ctime.tv_sec;
    rfile_stat->mtime.tv_nsec = (u64)rfile_kstat->mtime.tv_nsec;
    rfile_stat->mtime.tv_sec = (u64)rfile_kstat->mtime.tv_sec;
    rfile_stat->ctime.tv_nsec = (u64)rfile_kstat->ctime.tv_nsec;
    rfile_stat->blksize = (u32)rfile_kstat->blksize;
    rfile_stat->blocks = (u64)rfile_kstat->blocks;
}

s32 bsp_stat(const s8 *name, void *stat)
{
    s32 ret;
    struct kstat kstattmp = { 0 };
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = vfs_stat(name, &kstattmp);
    if (ret == 0) {
        rfile_trans_stat(stat, &kstattmp);
    }

    set_fs(old_fs);

    return ret;
}

s32 bsp_rename(const char *oldname, const char *newname)
{
    s32 ret;
    unsigned long old_fs;

    if ((oldname == NULL) || (newname == NULL)) {
        return -1;
    }

    old_fs = get_fs();
    set_fs((mm_segment_t)KERNEL_DS);

    ret = sys_rename(oldname, newname);

    set_fs(old_fs);

    return ret;
}

EXPORT_SYMBOL(bsp_open);
EXPORT_SYMBOL(bsp_access);
EXPORT_SYMBOL(bsp_close);
EXPORT_SYMBOL(bsp_write);
EXPORT_SYMBOL(bsp_write_sync);
EXPORT_SYMBOL(bsp_get_file_size);
EXPORT_SYMBOL(bsp_read);
EXPORT_SYMBOL(bsp_lseek);
EXPORT_SYMBOL(bsp_tell);
EXPORT_SYMBOL(bsp_remove);
EXPORT_SYMBOL(bsp_mkdir);
EXPORT_SYMBOL(bsp_rmdir);
EXPORT_SYMBOL(bsp_opendir);
EXPORT_SYMBOL(bsp_readdir);
EXPORT_SYMBOL(bsp_closedir);
EXPORT_SYMBOL(bsp_stat);

unsigned long mdrv_file_get_errno()
{
    return 0;
}

int rfile_getmode_check(const char *mode, int ret)
{
    /* check for garbage in second character */
    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        bsp_err("[%s]:1. mode:%c.\n", __FUNCTION__, *mode);
        return 0;
    }

    /* check for garbage in third character */
    if (*mode++ == '\0') {
        return ret; /* no third char */
    }

    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        bsp_err("[%s]:3. mode:%c.\n", __FUNCTION__, *mode);
        return 0;
    }

    /* check for garbage in fourth character */
    if (*mode++ == '\0') {
        return ret; /* no fourth char */
    }

    if (*mode != '\0') {
        bsp_err("[%s]:5. mode:%c.\n", __FUNCTION__, *mode);
        return (0);
    } else {
        return ret;
    }
}

int rfile_getmode(const char *mode, int *flag)
{
    int ret, ret_temp;
    unsigned int m;
    unsigned int o;

    switch (*mode++) {
        case 'r': /* open for reading */
            ret_temp = 0x0004;
            m = RFILE_RDONLY;
            o = 0;
            break;

        case 'w': /* open for writing */
            ret_temp = 0x0008;
            m = RFILE_WRONLY;
            o = RFILE_CREAT | RFILE_TRUNC;
            break;

        case 'a': /* open for appending */
            ret_temp = 0x0008;
            m = RFILE_WRONLY;
            o = RFILE_CREAT | RFILE_APPEND;
            break;

        default: /* illegal mode */
            return 0;
    }

    /* [rwa]\+ or [rwa]b\+ means read and write */
    if ((*mode == '+') || (*mode == 'b' && mode[1] == '+')) {
        ret_temp = 0x0010;
        m = RFILE_RDWR;
    }

    *flag = (int)(m | o);

    ret = rfile_getmode_check(mode, ret_temp);

    return ret;
}

rfile_file_s *rfile_fp_create(void)
{
    rfile_file_s *fp = (rfile_file_s *)kzalloc(sizeof(rfile_file_s), GFP_KERNEL);
    if (fp == NULL) {
        bsp_err("fp alloc failed, len 0x%lx.\n", sizeof(rfile_file_s));
        return NULL;
    }
    fp->_flags = 1; /* caller sets real flags */
    fp->_file = -1; /* no file */

    return fp;
}

void rfile_fp_destroy(rfile_file_s *fp)
{
    kfree((char *)fp);
    return;
}

/* 文件系统接口 */
void *mdrv_file_open(const char *path, const char *mode)
{
    int ret;
    int oflags;
    int flags;
    rfile_file_s *fp = NULL;

    if ((path == NULL) || (mode == NULL)) {
        bsp_err("[%s] para err.\n", __FUNCTION__);
        return NULL;
    }
    fp = rfile_fp_create();
    if (fp == NULL) {
        return NULL;
    }

    /* 将字符串参数转换成整数 */
    flags = rfile_getmode(mode, &oflags);
    if (flags == 0) {
        bsp_err("[%s] rfile_getmode failed. ret = %d.\n", __FUNCTION__, flags);
        rfile_fp_destroy(fp);
        return NULL;
    }

    ret = bsp_open((const s8 *)path, oflags, RFILE_OPEN_FILE_PERMISSION);
    if (ret < 0) {
        bsp_err("[%s] bsp_open failed, path=%s, ret = %x.\n", __FUNCTION__, path, ret);
        rfile_fp_destroy(fp);
        return NULL;
    }

    fp->_file = (short)ret;
    fp->_flags = (short)flags;

    return (void *)fp;
}

int mdrv_file_close(void *fp)
{
    int ret;

    if (fp == NULL) {
        return -1;
    }

    ret = bsp_close(((rfile_file_s *)fp)->_file);
    if (ret == 0) {
        rfile_fp_destroy(fp);
    }

    return ret;
}

int mdrv_file_read(void *ptr, unsigned int size, unsigned int number, const void *stream)
{
    int cnt;

    if ((ptr == NULL) || (stream == NULL) || (size == 0)) {
        return -1;
    }

    cnt = bsp_read(((rfile_file_s *)stream)->_file, ptr, (size * number));

    return cnt / ((int)size);
}

int mdrv_file_write(const void *ptr, unsigned int size, unsigned int number, const void *stream)
{
    int cnt;

    if ((ptr == NULL) || (stream == NULL) || (size == 0)) {
        return -1;
    }

    cnt = bsp_write(((rfile_file_s *)stream)->_file, ptr, (size * number));

    return cnt / ((int)size);
}

int mdrv_file_write_sync(const void *ptr, unsigned int size, unsigned int number, const void *stream)
{
    int cnt;

    if ((ptr == NULL) || (stream == NULL) || (size == 0)) {
        return -1;
    }

    cnt = bsp_write_sync(((rfile_file_s *)stream)->_file, ptr, (size * number));

    return cnt / ((int)size);
}

int mdrv_file_seek(const void *stream, long offset, int whence)
{
    int ret;

    if (stream == NULL) {
        return -1;
    }
    ret = bsp_lseek(((rfile_file_s *)stream)->_file, offset, whence);
    if (ret < 0) {
        return -1;
    }
    return 0;
}

long mdrv_file_tell(const void *stream)
{
    if (stream == NULL) {
        return -1;
    }

    return bsp_tell(((rfile_file_s *)stream)->_file);
}

int mdrv_file_remove(const char *pathname)
{
    if (pathname == NULL) {
        return -1;
    }

    return bsp_remove((const s8 *)pathname);
}

int mdrv_file_mkdir(const char *dir_name)
{
    if (dir_name == NULL) {
        return -1;
    }

    return bsp_mkdir((s8 *)dir_name, RFILE_MKDIR_PERMISSION);
}

int mdrv_file_rmdir(const char *path)
{
    if (path == NULL) {
        return -1;
    }

    return bsp_rmdir(path);
}

rfile_dir_s *rfile_dir_create(void)
{
    rfile_dir_s *dir = (rfile_dir_s *)kzalloc(sizeof(rfile_dir_s), GFP_KERNEL);
    if (dir == NULL) {
        bsp_err("dir alloc failed, len 0x%lx.\n", sizeof(rfile_dir_s));
        return NULL;
    }
    dir->dd_fd = -1;
    dir->dd_cookie = 0;
    dir->dd_eof = 0;

    (void)memset_s(&dir->dd_dirent, sizeof(rfile_dirent_s), 0, sizeof(rfile_dirent_s));
    return dir;
}

void rfile_dir_destroy(rfile_dir_s *dir)
{
    dir->dd_fd = -1;
    (void)memset_s(&dir->dd_dirent, sizeof(rfile_dirent_s), 0, sizeof(rfile_dirent_s));
    kfree((char *)dir);

    return;
}

rfile_dir_s *mdrv_file_opendir(const char *dir_name)
{
    int ret;
    rfile_dir_s *dir = NULL;
    size_t min_length;
    if (dir_name == NULL) {
        bsp_err("[%s] param is NULL.\n", __FUNCTION__);
        return NULL;
    }
    dir = rfile_dir_create();
    if (dir == NULL) {
        return NULL;
    }

    ret = bsp_opendir(dir_name);
    if (ret < 0) {
        rfile_dir_destroy(dir);
        return NULL;
    }
    dir->dd_fd = ret;
    min_length = min_t(size_t, strlen(dir_name), DRV_NAME_MAX);
    ret = memcpy_s(dir->dd_dirent.d_name, DRV_NAME_MAX, dir_name, min_length);
    if (ret != EOK) {
        bsp_err("[%s] memcpy_s err. ret = %d.\n", __FUNCTION__, ret);
        rfile_dir_destroy(dir);
        return NULL;
    }

    return dir;
}

struct rfile_dirent_info_local *rfile_readdir_get_node(rfile_dir_s *dirp)
{
    struct list_head *me = NULL;
    struct rfile_dirent_info_local *dirent_local = NULL;

    osl_sem_down(&g_rfile_mdrv_ctrl.sem_dir_list);

    list_for_each(me, &g_rfile_mdrv_ctrl.dir_listhead)
    {
        dirent_local = list_entry(me, struct rfile_dirent_info_local, stlist);

        if (dirent_local != NULL && dirp == dirent_local->dirp) {
            break;
        } else {
            dirent_local = NULL;
        }
    }

    osl_sem_up(&g_rfile_mdrv_ctrl.sem_dir_list);

    return dirent_local;
}

void rfile_readdir_add_node(struct rfile_dirent_info_local *pdirent_list)
{
    osl_sem_down(&g_rfile_mdrv_ctrl.sem_dir_list);

    list_add(&pdirent_list->stlist, &g_rfile_mdrv_ctrl.dir_listhead);

    osl_sem_up(&g_rfile_mdrv_ctrl.sem_dir_list);

    return;
}

void rfile_readdir_del_node(rfile_dir_s *dirp)
{
    struct list_head *me = NULL;
    struct list_head *n = NULL;
    struct rfile_dirent_info_local *dirent_local = NULL;

    osl_sem_down(&g_rfile_mdrv_ctrl.sem_dir_list);

    list_for_each_safe(me, n, &g_rfile_mdrv_ctrl.dir_listhead)
    {
        dirent_local = list_entry(me, struct rfile_dirent_info_local, stlist);
        if (dirent_local != NULL && dirp == dirent_local->dirp) {
            list_del(&dirent_local->stlist);
            kfree(dirent_local->dirent_bsp);
            kfree(dirent_local);
            break;
        }
    }

    osl_sem_up(&g_rfile_mdrv_ctrl.sem_dir_list);

    return;
}
struct rfile_dirent_info_local *rfile_readdir_from_fs(rfile_dir_s *dirp)
{
    int len;
    bsp_rfile_dirent_s *dirent_bsp = NULL;
    struct rfile_dirent_info_local *dirent_local = NULL;

    dirent_bsp = (bsp_rfile_dirent_s *)kzalloc(DATA_SIZE_1_K, GFP_KERNEL);
    if (dirent_bsp == NULL) {
        bsp_err("dirent_bsp alloc failed, len 0x%lx.\n", DATA_SIZE_1_K);
        return NULL;
    }
    len = bsp_readdir((unsigned int)dirp->dd_fd, dirent_bsp, DATA_SIZE_1_K);
    if (len <= 0) {
        bsp_err("bsp_readdir failed, ret 0x%x.\n", len);
        kfree(dirent_bsp);
        return NULL;
    }
    dirent_local = kzalloc(sizeof(struct rfile_dirent_info_local), GFP_KERNEL);
    if (dirent_local == NULL) {
        bsp_err("dirent_local alloc failed, len 0x%lx.\n", sizeof(struct rfile_dirent_info_local));
        kfree(dirent_bsp);
        return NULL;
    }

    dirent_local->dirp = dirp;
    dirent_local->dirent_bsp = dirent_bsp;
    dirent_local->len = len;
    dirent_local->ptr = 0;

    rfile_readdir_add_node(dirent_local);

    return dirent_local;
}

rfile_dirent_s *mdrv_file_readdir(rfile_dir_s *dirp)
{
    int ret;
    struct rfile_dirent_info_local *dirent_local = NULL;
    bsp_rfile_dirent_s *dirent_bsp_cur = NULL;
    rfile_dirent_s *dirent_mdrv_ret = NULL;

    if (dirp == NULL) {
        return NULL;
    }

    dirent_local = rfile_readdir_get_node(dirp);
    if (dirent_local == NULL) {
        dirent_local = rfile_readdir_from_fs(dirp);
        if (dirent_local == NULL) {
            return NULL;
        }
    }

    if ((dirent_local->ptr < 0) || (dirent_local->ptr >= dirent_local->len)) {
        bsp_err("[%s] ptr %d, len %d.\n", __FUNCTION__, dirent_local->ptr, dirent_local->len);
        goto err;
    }

    dirent_bsp_cur = (bsp_rfile_dirent_s *)((u8 *)(dirent_local->dirent_bsp) + dirent_local->ptr);

    dirent_mdrv_ret = &(dirent_local->dirent_mdrv);
    dirent_mdrv_ret->d_ino = (rfile_ino_t)dirent_bsp_cur->d_ino;
    (void)memset_s((void *)dirent_mdrv_ret->d_name, (DRV_NAME_MAX + 1), 0, (DRV_NAME_MAX + 1));
    if (strlen((char *)dirent_bsp_cur->d_name) > DRV_NAME_MAX) {
        ret = memcpy_s(dirent_mdrv_ret->d_name, DRV_NAME_MAX + 1, dirent_bsp_cur->d_name, DRV_NAME_MAX);
        if (ret != EOK) {
            bsp_err("[%s] memcpy_s err. ret = %d.\n", __FUNCTION__, ret);
            goto err;
        }
    } else {
        ret = strcpy_s(dirent_mdrv_ret->d_name, DRV_NAME_MAX + 1, (char *)dirent_bsp_cur->d_name);
        if (ret != EOK) {
            bsp_err("[%s] strcpy_s err. ret = %d.\n", __FUNCTION__, ret);
            goto err;
        }
    }

    dirent_local->ptr += dirent_bsp_cur->d_reclen;

    return dirent_mdrv_ret;
err:
    rfile_readdir_del_node(dirp);
    return NULL;
}

int mdrv_file_closedir(rfile_dir_s *dirp)
{
    int ret;

    if (dirp == NULL) {
        return -1;
    }

    rfile_readdir_del_node(dirp);

    ret = bsp_closedir(dirp->dd_fd);
    if (ret == 0) {
        rfile_dir_destroy(dirp);
    }

    return ret;
}

int mdrv_file_get_stat(const char *path, rfile_stat_s *buf)
{
    int ret;
    struct rfile_stat_stru ststat;

    if (buf == NULL) {
        return -1;
    }
    (void)memset_s(&ststat, sizeof(struct rfile_stat_stru), 0, sizeof(struct rfile_stat_stru));

    ret = bsp_stat(path, &ststat);
    if (ret == 0) {
        buf->st_dev = (unsigned long)ststat.dev;
        buf->st_ino = (unsigned long)ststat.ino;
        buf->st_mode = (int)ststat.mode;
        buf->st_nlink = (unsigned long)ststat.nlink;
        buf->st_uid = (unsigned short)ststat.uid;
        buf->st_gid = (unsigned short)ststat.gid;
        buf->st_rdev = (unsigned long)ststat.rdev;
        buf->st_size = (signed long long)ststat.size;
        buf->ul_atime = (unsigned long)ststat.atime.tv_sec;
        buf->ul_mtime = (unsigned long)ststat.mtime.tv_sec;
        buf->ul_ctime = (unsigned long)ststat.ctime.tv_sec;
        buf->st_blksize = (long)ststat.blksize;
        buf->st_blocks = (unsigned long)ststat.blocks;
        buf->st_attrib = 0;
    }

    return ret;
}

int mdrv_file_access(const char *path, int amode)
{
    return bsp_access(path, amode);
}

int mdrv_file_rename(const char *oldname, const char *newname)
{
    return bsp_rename(oldname, newname);
}

EXPORT_SYMBOL(mdrv_file_read);
EXPORT_SYMBOL(mdrv_file_opendir);
EXPORT_SYMBOL(mdrv_file_seek);
EXPORT_SYMBOL(mdrv_file_get_stat);
EXPORT_SYMBOL(mdrv_file_close);
EXPORT_SYMBOL(mdrv_file_closedir);
EXPORT_SYMBOL(mdrv_file_open);
EXPORT_SYMBOL(mdrv_file_rmdir);
EXPORT_SYMBOL(mdrv_file_write);
EXPORT_SYMBOL(mdrv_file_mkdir);
EXPORT_SYMBOL(mdrv_file_tell);
EXPORT_SYMBOL(mdrv_file_readdir);
EXPORT_SYMBOL(mdrv_file_remove);

EXPORT_SYMBOL(mdrv_file_write_sync);
EXPORT_SYMBOL(mdrv_file_rename);
EXPORT_SYMBOL(mdrv_file_access);
EXPORT_SYMBOL(mdrv_file_get_errno);

int file_acore_init(void)
{
    INIT_LIST_HEAD(&g_rfile_mdrv_ctrl.dir_listhead);

    sema_init(&g_rfile_mdrv_ctrl.sem_dir_list, 1);
    return 0;
}

