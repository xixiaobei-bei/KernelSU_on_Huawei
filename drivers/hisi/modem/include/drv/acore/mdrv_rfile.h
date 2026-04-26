/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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
/* *
 * @brief   rfile模块在acore上的对外头文件
 * @file    mdrv_rfile.h
 * @author  wangshunxin
 * @version v1.0
 * @date    2019.11.26
 * @note    修改记录(版本号|修订日期|说明)
 * <ul><li>v1.0|2019.11.26|创建文件</li></ul>
 * @since
 */
#ifndef __MDRV_RFILE_H__
#define __MDRV_RFILE_H__
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_MSC_VER)
#pragma pack(push, 4)
#else
#pragma pack(4)
#endif

/* ****************************************************************************
 * Attention                           *
 * *****************************************************************************
 * Description : Driver for rfile
 * Core        : Acore、Ccore
 * Header File : the following head files need to be modified at the same time
 * : /acore/mdrv_rfile.h
 * : /ccore/ccpu/mdrv_rfile.h
 * : /ccore/fusion/mdrv_rfile.h
 * **************************************************************************** */
/* *
 * @brief 文件序列号类型
 */
typedef unsigned int rfile_ino_t;
/* *
 * @brief 文件状态结构体中时间类型
 */
typedef unsigned int rfile_time; /* type for file time fields */
#ifndef FILE
/* *
 * @brief 文件描述符
 */
typedef int FILE;
#endif
/* *
 * @brief modem 存log路径
 */
#define MODEM_LOG_ROOT "/data/hisi_logs/modem_log"
/* *
 * @brief modem 存短信路径
 */
#define MODEM_DATA_ROOT "/data/vendor/hisi_data/modem_data"
/* *
 * @brief 路径最大长度
 */
#define DRV_NAME_MAX 255

/* *
 * @brief open参数：directory
 */
#define DRV_S_IFDIR 0x4000
/* *
 * @brief open参数：empty file
 */
#define DRV_FILE_NULL 0
/* *
 * @brief open参数：open for reading only
 */
#define DRV_O_RDONLY 0x0000
/* *
 * @brief open参数：open for writing only
 */
#define DRV_O_WRONLY 0x0001
/* *
 * @brief open参数：open for reading and writing
 */
#define DRV_O_RDWR 0x0002
/* *
 * @brief open参数：open with file create
 */
#define DRV_O_CREAT 0x0200
/* *
 * @brief open参数：append (writes guaranteed at the end)
 */
#define DRV_O_APPEND 0x0008
/* *
 * @brief open参数：open with truncation
 */
#define DRV_O_TRUNC 0x0400
/* *
 * @brief open参数：error on open if file exists
 */
#define DRV_O_EXCL 0x0800
/* *
 * @brief open参数：non blocking I/O (POSIX style)
 */
#define DRV_O_NONBLOCK 0x4000
/* *
 * @brief open参数：non blocking I/O (POSIX style)
 */
#define DRV_O_NOCTTY 0x8000
/* *
 * @brief sleek参数：absolute offset
 */
#define DRV_SEEK_SET 0
/* *
 * @brief sleek参数：relative to current offset
 */
#define DRV_SEEK_CUR 1
/* *
 * @brief sleek参数：relative to end of file
 */
#define DRV_SEEK_END 2

/* *
 * @brief rfile文件描述符，作为open返回类型
 */
typedef struct {
    short _flags;
    short _file;
} rfile_file_s;

/* *
 * @brief rfile目录信息，作为readdir返回类型
 */
typedef struct {
    rfile_ino_t d_ino;             /* *<file serial number */
    char d_name[DRV_NAME_MAX + 1]; /* *<file name, null-terminated */
} rfile_dirent_s;

/* *
 * @brief rfile目录描述符，作为opendir返回类型
 */
typedef struct {
    int dd_fd;                /* *<file descriptor for open directory */
    int dd_cookie;            /* *<filesys-specific marker within dir */
    int dd_eof;               /* *<readdir EOF flag */
    rfile_dirent_s dd_dirent; /* *<obtained directory entry */
} rfile_dir_s;

/* *
 * @brief rfile文件状态结构体
 */
typedef struct {
    unsigned long st_dev;     /* *<Device ID number */
    unsigned long st_ino;     /* *<File serial number */
    int st_mode;              /* *<Mode of file */
    unsigned long st_nlink;   /* *<Number of hard links to file */
    unsigned short st_uid;    /* *<User ID of file */
    unsigned short st_gid;    /* *<Group ID of file */
    unsigned long st_rdev;    /* *<Device ID if special file */
    signed long long st_size; /* *<File size in bytes */
    rfile_time ul_atime;      /* *<Time of last access */
    rfile_time ul_mtime;      /* *<Time of last modification */
    rfile_time ul_ctime;      /* *<Time of last status change */
    long st_blksize;          /* *<File system block size */
    unsigned long st_blocks;  /* *<Number of blocks containing file */
    unsigned char st_attrib;  /* *<DOSFS only - file attributes */
    int st_reserved1;         /* *<reserved for future use */
    int st_reserved2;         /* *<reserved for future use */
    int st_reserved3;         /* *<reserved for future use */
    int st_reserved4;         /* *<reserved for future use */
} rfile_stat_s;

/* ****************************************************************************
 * Attention   END                        *
 * *************************************************************************** */
/* *
 * @brief 打开文件。
 *
 * @par 描述:
 * 打开文件。
 *
 * @param[in]  path 被打开的文件绝对路径，路径不能为空
 * @param[in]  mode 打开文件的模式，字符'r'、'w'、'a'、'b'、'+'的组合。
 *
 * @retval =NULL，表示函数执行失败。
 * @retval !=NULL，表示函数执行成功，其值表示被打开文件的文件描述符。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
void *mdrv_file_open(const char *path, const char *mode);
/* *
 * @brief 关闭文件。
 *
 * @par 描述:
 * 关闭文件。
 *
 * @param[in]  fp 被打开的文件的文件描述符
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>close文件前应open过该文件，并将open得到的文件描述符作为参数传入。</li></ul>
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_close(void *fp);
/* *
 * @brief 读取文件。
 *
 * @par 描述:
 * 读取文件。
 *
 * @param[in]  ptr 读文件的数据目的地址
 * @param[in]  size 读文件的数据类型所占空间
 * @param[in]  number 以size为单位读文件的大小，因此读数据的总大小为size*number
 * @param[in]  stream 被读文件的文件描述符
 *
 * @retval <0，表示函数执行失败。
 * @retval >=0，表示函数执行成功，返回以size为单位的实际读取大小，若返回值不等于number则代表读取数据异常。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_read(void *ptr, unsigned int size, unsigned int number, const void *stream);
/* *
 * @brief 写文件。
 *
 * @par 描述:
 * 写文件，注意该函数不保证一定写入flash中，若必须要写入flash，请调用mdrv_file_write_sync接口。
 *
 * @param[in]  ptr 写文件数据的源地址
 * @param[in]  size 写文件的数据类型所占空间
 * @param[in]  number 以size为单位写文件的大小，因此写数据的总大小为size*number
 * @param[in]  stream 被写文件的文件描述符
 *
 * @retval <0，表示函数执行失败。
 * @retval >=0，表示函数执行成功，返回以size为单位的实际写入大小，若返回值不等于number则代表写入数据异常。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_write(const void *ptr, unsigned int size, unsigned int number, const void *stream);
/* *
 * @brief 同步写文件。
 *
 * @par 描述:
 * 同步写文件，在写操作完成后会将数据同步到flash中。
 *
 * @param[in]  ptr 写文件数据的源地址
 * @param[in]  size 写文件的数据类型所占空间
 * @param[in]  number 以size为单位写文件的大小，因此写数据的总大小为size*number
 * @param[in]  stream 被写文件的文件描述符
 *
 * @retval <0，表示函数执行失败。
 * @retval >=0，表示函数执行成功，返回以size为单位的实际写入大小，若返回值不等于number则代表写入数据异常。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_write_sync(const void *ptr, unsigned int size, unsigned int number, const void *stream);
/* *
 * @brief 移动文件流的读写位置。
 *
 * @par 描述:
 * 移动文件流的读写位置。
 *
 * @param[in]  stream 被移动文件的文件描述符
 * @param[in]  offset 相对于whence的偏移量
 * @param[in]  whence  DRV_SEEK_SET（文件指针开始），DRV_SEEK_CUR（文件指针当前位置），DRV_SEEK_END（为文件指针尾）
 *
 * @retval <0，表示函数执行失败。
 * @retval >=0，表示函数执行成功，返回当前文件指针位置。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 *
 * @see DRV_SEEK_SET  DRV_SEEK_CUR  DRV_SEEK_END
 */
int mdrv_file_seek(const void *stream, long offset, int whence);
/* *
 * @brief 获取文件流的读写位置。
 *
 * @par 描述:
 * 获取文件流的读写位置。
 *
 * @param[in]  stream 文件描述符
 *
 * @retval <0，表示函数执行失败。
 * @retval >=0，表示函数执行成功。返回当前文件指针位置。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
long mdrv_file_tell(const void *stream);
/* *
 * @brief 删除文件。
 *
 * @par 描述:
 * 删除文件。
 *
 * @param[in]  pathname 文件绝对路径
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_remove(const char *pathname);
/* *
 * @brief 创建目录。
 *
 * @par 描述:
 * 创建目录。
 *
 * @param[in]  dir_name 目录绝对路径
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_mkdir(const char *dir_name);
/* *
 * @brief 删除目录。
 *
 * @par 描述:
 * 删除目录。
 *
 * @param[in]  path 目录绝对路径
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_rmdir(const char *path);
/* *
 * @brief 打开目录。
 *
 * @par 描述:
 * 打开目录。
 *
 * @param[in]  dir_name 目录绝对路径
 *
 * @retval =NULL，表示函数执行失败。
 * @retval !=NULL，表示函数执行成功，返回打开的目录描述符。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 *
 * @see rfile_dir_s
 */
rfile_dir_s *mdrv_file_opendir(const char *dir_name);
/* *
 * @brief 读取目录。
 *
 * @par 描述:
 * 读取目录。
 *
 * @param[in]  dirp 目录描述符
 *
 * @retval =NULL，表示函数执行失败。
 * @retval !=NULL，表示函数执行成功，返回读取的目录信息。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 *
 * @see rfile_dir_s rfile_dirent_s
 */
rfile_dirent_s *mdrv_file_readdir(rfile_dir_s *dirp);
/* *
 * @brief 关闭目录。
 *
 * @par 描述:
 * 关闭目录。
 *
 * @param[in]  dirp 目录描述符
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_closedir(rfile_dir_s *dirp);
/* *
 * @brief 获取文件状态。
 *
 * @par 描述:
 * 获取文件状态。
 *
 * @param[in] path 文件绝对路径
 * @param[out] buf 文件状态
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 *
 * @see rfile_stat_s
 */
int mdrv_file_get_stat(const char *path, rfile_stat_s *buf);
/* *
 * @todo 无效接口，待删除。
 */
unsigned long mdrv_file_get_errno(void);
/* *
 * @brief 判断是否能以amode的权限访问文件。
 *
 * @par 描述:
 * 判断是否能以amode的权限访问文件。
 *
 * @param[in] path 文件绝对路径
 * @param[in] amode 权限
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_access(const char *path, int amode);
/* *
 * @brief 修改文件名字。
 *
 * @par 描述:
 * 修改文件名字。
 *
 * @param[in] oldname 原文件名(绝对路径)
 * @param[in] newname 新文件名(绝对路径)
 *
 * @retval !=0，表示函数执行失败。
 * @retval =0，表示函数执行成功。
 *
 * @par 依赖:
 * <ul><li>mdrv_rfile.h：该接口声明所在的头文件。</li></ul>
 */
int mdrv_file_rename(const char *oldname, const char *newname);
/* *
 * @todo 无效接口，待删除。
 */
int mdrv_file_xcopy(const char *source, const char *dest);
/* *
 * @todo 无效接口，待删除。
 */
int mdrv_file_xdelete(const char *source);
#if defined(_MSC_VER)
#pragma pack(pop)
#else
#pragma pack()
#endif

#ifdef __cplusplus
}
#endif
#endif
