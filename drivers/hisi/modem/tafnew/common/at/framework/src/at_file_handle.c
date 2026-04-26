/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
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
 * *	notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *	notice, this list of conditions and the following disclaimer in the
 * *	documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *	be used to endorse or promote products derived from this software
 * *	without specific prior written permission.
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

#include "at_file_handle.h"
#include <linux/syscalls.h>
#include <linux/uaccess.h>
#include <linux/errno.h>


/*
 * at_file_access - Check whether the calling process can access the file.
 * @path: file path
 * @mode: The accessibility check(s) to be performed.
 *
 * On success 0 is returned, On error negative is returned.
 */
int at_file_access(const char *path, int mode)
{
    int           ret;
    unsigned long old_fs;

    if (path == NULL) {
        return -EFAULT;
    }

    old_fs = get_fs();

    set_fs((unsigned long)KERNEL_DS);
    ret = sys_access(path, mode);
    set_fs(old_fs);

    return ret;
}

/*
 * at_file_rename - Rename a file.
 *
 *
 * On success 0 is returned, On error negative is returned.
 */
int at_file_rename(const char *oldpath, const char *newpath)
{
    int           ret;
    unsigned long old_fs;

    if ((oldpath == NULL) || (newpath == NULL)) {
        return -EFAULT;
    }

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_rename(oldpath, newpath);

    set_fs(old_fs);

    return ret;
}

/*
 * at_file_mkdir - Create a directory named dir_name.
 * @dir_name
 *
 * Return 0 on success, or negative if an error occurred.
 */
int at_file_mkdir(const char *dir_name)
{
    int           ret;
    unsigned long old_fs;

    if (dir_name == NULL) {
        return -EFAULT;
    }

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_mkdir(dir_name, 0775); /* 0775表示打开文件的权限为:rwx rwx r-x */

    set_fs(old_fs);

    return ret;
}

/*
 * at_file_write - Writes up to count bytes from the buffer starting at buf to
 * the file referred to by the file descriptor fd.
 *
 * @fd: file descriptor
 *
 * On success, the number of bytes written is returned;
 * On error, negative is returned.
 */
int at_file_write(int fd, const void *buf, unsigned int count)
{
    int           ret;
    unsigned long old_fs;

    if (buf == NULL || count == 0) {
        return -EFAULT;
    }

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_write((unsigned int)fd, buf, count);

    set_fs(old_fs);

    return ret;
}

/*
 * at_check_file_mode - File mode check.
 * @mode: file mode.
 *
 * Return 0 on success, negative on failure.
 */
int at_check_file_mode(const char *mode)
{
    /* check for garbage in second character */
    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        return -EINVAL;
    }

    /* check for garbage in third character */
    if (*mode++ == '\0') {
        return 0; /* no third char */
    }

    if ((*mode != '+') && (*mode != 'b') && (*mode != '\0')) {
        return -EINVAL;
    }

    /* check for garbage in fourth character */
    if (*mode++ == '\0') {
        return 0; /* no fourth char */
    }

    if (*mode != '\0') {
        return -EINVAL;
    } else {
        return 0;
    }
}

/*
 * at_get_file_mode - Get file mode.
 * @mode: access modes in string.
 * @flags: access modes transformed from string.
 *
 * Return flags on success, negative on failure.
 */
int at_get_file_mode(const char *mode, int *flags)
{
    int          ret;
    unsigned int m;
    unsigned int o;

    switch (*mode++) {
        case 'r': /* open for reading */
            m = FILE_RDONLY;
            o = 0;
            break;
        case 'w': /* open for writing */
            m = FILE_WRONLY;
            o = FILE_CREAT | FILE_TRUNC;
            break;
        case 'a': /* open for appending */
            m = FILE_WRONLY;
            o = FILE_CREAT | FILE_APPEND;
            break;
        default: /* illegal mode */
            return -EINVAL;
    }

    /* [rwa]\+ or [rwa]b\+ means read and write */

    if ((*mode == '+') || (*mode == 'b' && mode[1] == '+')) {
        m = FILE_RDWR;
    }

    *flags = (int)(m | o);

    ret = at_check_file_mode(mode);

    return ret;
}

/*
 * at_file_open - Opens the file specified by pathname.
 * @path: file path.
 * @mode: access modes.
 *
 * Return the new file descriptor, or negative if an error occurred.
 */
int at_file_open(const char *path, const char *mode)
{
    int           ret;
    int           fd;
    int           flags;
    unsigned long old_fs;

    if (path == NULL || mode == NULL) {
        return -EFAULT;
    }

    ret = at_get_file_mode(mode, &flags);

    if (ret < 0) {
        return ret;
    }

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    fd = sys_open(path, flags, 0664); /* 0664表示打开文件的权限为:rw- rw- r-- */

    set_fs(old_fs);

    return fd;
}

/*
 * at_file_close - Close a file descriptor.
 * @fd: file descriptor.
 *
 * Return 0 on success, on error, negative is returned.
 */
int at_file_close(int fd)
{
    int           ret;
    unsigned long old_fs;

    old_fs = get_fs();
    set_fs((unsigned long)KERNEL_DS);

    ret = sys_close((unsigned int)fd);

    set_fs(old_fs);

    return ret;
}

