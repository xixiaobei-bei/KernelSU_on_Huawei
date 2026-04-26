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

#ifndef __BSP_RFILE_H__
#define __BSP_RFILE_H__

#include <product_config.h>
#include "mdrv_rfile.h"

#ifdef __cplusplus /* __cplusplus */
extern "C" {
#endif /* __cplusplus */

/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for rfile
 * Core        : Acore/Ccore
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/drivers/hisi/modem/drv/include/bsp_rfile.h
 * : /ccore/include/ccpu/bsp_rfile.h
 * : /ccore/include/fusion/bsp_rfile.h
 * **************************************************************************** */
#define RFILE_RDONLY (0x00000000)
#define RFILE_WRONLY (0x00000001)
#define RFILE_RDWR (0x00000002)
#define RFILE_CREAT (0x00000040)
#define RFILE_EXCL (0x00000080)
#define RFILE_TRUNC (0x00000200)
#define RFILE_APPEND (0x00000400)
#define RFILE_DIRECTORY (0x00004000)

#define RFILE_SEEK_SET 0 /* absolute offset, was L_SET */
#define RFILE_SEEK_CUR 1 /* relative to current offset, was L_INCR */
#define RFILE_SEEK_END 2 /* relative to end of file, was L_XTND */

#define RFILE_NAME_MAX 255 /* max length of the file name */

typedef enum {
    BSP_FS_OK = 0,
    BSP_FS_NOT_OK = 1,
    BSP_FS_BUTT
} bsp_fs_status_e;

typedef struct {
    u64 d_ino;
    u64 d_off;
    u16 d_reclen;
    u8 d_type;
    s8 d_name[0];     //lint !e43
} bsp_rfile_dirent_s; //lint !e959

/* struct timespec */
typedef struct {
    u64 tv_sec;  /* seconds */
    u64 tv_nsec; /* nanoseconds */
} bsp_rfile_timespec_s;

struct rfile_stat_stru {
    u64 ino;
    u32 dev;
    u16 mode;
    u32 nlink;
    u32 uid;
    u32 gid;
    u32 rdev;
    u64 size;
    bsp_rfile_timespec_s atime;
    bsp_rfile_timespec_s mtime;
    bsp_rfile_timespec_s ctime;
    u32 blksize;
    u64 blocks;
};


#ifndef CONFIG_RFILE_ON
static inline bsp_fs_status_e bsp_fs_ok(void)
{
    return BSP_FS_NOT_OK;
}
static inline bsp_fs_status_e bsp_fs_tell_load_mode(void)
{
    return BSP_FS_NOT_OK;
}
static inline void bsp_rfile_get_load_mode(long type)
{
    return;
}
static inline s32 bsp_stat(const s8 *name, void *stat)
{
    return 0;
}
static inline s32 bsp_close(u32 fp)
{
    return 0;
}
static inline s32 bsp_open(const s8 *path, s32 flags, s32 mode)
{
    return 0;
}
static inline s32 bsp_read(u32 fd, s8 *ptr, u32 size)
{
    return 0;
}
static inline s32 bsp_lseek(u32 fd, long offset, s32 whence)
{
    return 0;
}
static inline s32 bsp_access(const s8 *path, s32 mode)
{
    return 0;
}
static inline s32 bsp_remove(const s8 *pathname)
{
    return 0;
}
static inline s32 bsp_rename(const char *oldname, const char *newname)
{
    return 0;
}
static inline s32 bsp_write(u32 fd, const s8 *ptr, u32 size)
{
    return 0;
}
static inline s32 bsp_write_sync(u32 fd, const s8 *ptr, u32 size)
{
    return 0;
}
static inline s32 bsp_get_file_size(s32 fd)
{
    return 0;
}
static inline long bsp_tell(u32 fd)
{
    return 0;
}
static inline s32 bsp_mkdir(const s8 *dir_name, s32 mode)
{
    return 0;
}
static inline s32 bsp_rmdir(const s8 *path)
{
    return 0;
}
static inline s32 bsp_opendir(const s8 *dir_name)
{
    return 0;
}
static inline s32 bsp_readdir(u32 fd, void *dirent, u32 count)
{
    return 0;
}
static inline s32 bsp_closedir(s32 dir)
{
    return 0;
}
static inline int modem_rfile_init(void)
{
    (void)file_acore_init();
    return 0;
}
#else
/*
 * 功能描述: 判断a核文件系统是否可用
 * 返 回 值: 返回枚举bsp_fs_status_e中的值
 */
bsp_fs_status_e bsp_fs_ok(void);
/*
 * 功能描述: 获取加载模式，是否支持flash文件系统
 * 返 回 值: 返回枚举bsp_fs_status_e中的值
 */
bsp_fs_status_e bsp_fs_tell_load_mode(void);
/*
 * 功能描述: 告知rfile启动方式
 * 返 回 值: 无
 */
void bsp_rfile_get_load_mode(long type);
/*
 * 功能描述: 打开文件，如果中间目录不存在会逐级创建目录，目录权限755
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回文件句柄
 */
s32 bsp_open(const s8 *path, s32 flags, s32 mode);
/*
 * 功能描述: 关闭文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_close(u32 fp);
/*
 * 功能描述: 写文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回实际写入文件的大小
 */
s32 bsp_write(u32 fd, const s8 *ptr, u32 size);
/*
 * 功能描述: 同步写文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回实际写入文件的大小
 */
s32 bsp_write_sync(u32 fd, const s8 *ptr, u32 size);
/*
 * 功能描述: 读文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回实际读取文件的大小
 */
s32 bsp_read(u32 fd, s8 *ptr, u32 size);
/*
 * 功能描述: 偏移文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回偏移后文件指针位置
 */
s32 bsp_lseek(u32 fd, long offset, s32 whence);
/*
 * 功能描述: 获取文件大小，接口返回后会把文件指针偏移到文件起始位置
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回文件大小
 */
s32 bsp_get_file_size(u32 fd);
/*
 * 功能描述: 获取当前文件指针偏移位置
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回当前偏移位置
 */
long bsp_tell(u32 fd);
/*
 * 功能描述: 删除文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_remove(const s8 *pathname);
/*
 * 功能描述: 创建目录，如果中间目录不存在会逐级创建，目录权限底层统一配为755，与入参mode无关
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0。
 * 注    意：如果目录已存在，不会返回失败
 */
s32 bsp_mkdir(const s8 *dir_name, s32 mode);
/*
 * 功能描述: 删除目录
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_rmdir(const s8 *path);
/*
 * 功能描述: 打开目录
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_opendir(const s8 *dir_name);
/*
 * 功能描述: 读取目录
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_readdir(u32 fd, void *dirent, u32 count);
/*
 * 功能描述: 关闭目录
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_closedir(s32 dir);
/*
 * 功能描述: 重命名文件
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_rename(const char *oldname, const char *newname);
/*
 * 功能描述: 获取文件状态，接口不允许使用，待删除
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_stat(const s8 *name, void *stat);
/*
 * 功能描述: 判断文件权限，接口不允许使用，待删除
 * 返 回 值: 失败返回c语言错误码，见<errno.h>，成功返回0
 */
s32 bsp_access(const s8 *path, s32 mode);
int modem_rfile_init(void);
#endif
int file_acore_init(void);
s32 rfile_mkdir_recursive(char *path, u32 pathlen);

#ifdef __cplusplus /* __cplusplus */
}
#endif /* __cplusplus */

#endif /*  __BSP_RFILE_H__ */
