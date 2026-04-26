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

#ifndef __RFILE_SERVER_H__
#define __RFILE_SERVER_H__


/* ****************************************************************************
  1 其他头文件包含
**************************************************************************** */
#include <linux/sched.h>
#include <linux/module.h>
#include <linux/string.h>
#include <linux/slab.h>
#include <linux/unistd.h>
#include <linux/types.h>
#include <linux/hrtimer.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/device.h>
#include <linux/cdev.h>

#include "drv_comm.h"
#include "osl_types.h"
#include "osl_sem.h"
#include "osl_list.h"
#include "mdrv_memory.h"
#include "bsp_rfile.h"
#include "bsp_reset.h"

#pragma pack(4)

#define DELAY_TIME 1

typedef osl_sem_id rfile_sem_id;

#define RFILE_DEV_NAME "rfile"
#define RFILE_DEV_CLASS "rfile_class"
#define RFILE_DEV_NUM 1
#define RFILE_IOCTL_READ 0xa5a5
#define RFILE_IOCTL_WRITE 0xa6a6
#define RFILE_WAIT_TIME_MS 19000
#define RFILE_WAIT_TIME_S 19
#define RFILE_IOCTL_INTERRUPT_ERRNO 0x5a5a


/* 操作类型 */
enum bsp_rfile_op_type {
    EN_RFILE_OP_OPEN = 0,
    EN_RFILE_OP_CLOSE,
    EN_RFILE_OP_WRITE,
    EN_RFILE_OP_WRITE_SYNC,
    EN_RFILE_OP_READ,
    EN_RFILE_OP_SEEK,
    EN_RFILE_OP_TELL,
    EN_RFILE_OP_REMOVE,
    EN_RFILE_OP_MKDIR,
    EN_RFILE_OP_RMDIR,
    EN_RFILE_OP_OPENDIR,
    EN_RFILE_OP_READDIR,
    EN_RFILE_OP_CLOSEDIR,
    EN_RFILE_OP_STAT,
    EN_RFILE_OP_ACCESS,
    EN_RFILE_OP_RENAME,
    EN_RFILE_OP_CHECK_LOAD_MODE,
    EN_RFILE_OP_RESET,
    EN_RFILE_OP_OPEN_RECOVER,
    EN_RFILE_OP_BUTT
};

/* 处理状态 */
enum rfile_acore_state {
    RFILE_STATE_IDLE = 0,
    RFILE_STATE_DOING,
    RFILE_STATE_RESET,
    RFILE_STATE_UNINITED,
    RFILE_STATE_APP_RECOVERING
};

enum rfile_acore_event {
    RFILE_EVENT_ICC_CB = 0,
    RFILE_EVENT_RESET_BEFORE,
    RFILE_EVENT_RESETTING,
    RFILE_EVENT_SHUTDOWN,
    RFILE_EVENT_APP_KILLED,
    RFILE_EVENT_ICC_FULL
};

enum rfile_open_type {
    RFILE_OPEN_FILE = 0,
    RFILE_OPEN_DIR = 1
};

enum rfile_offset_manage_type {
    RFILE_MANAGE_READ_WRITE = 0,
    RFILE_MANAGE_SEEK = 1
};

struct rfile_fd_list {
    struct list_head stlist;
    u32 open_type;
    u32 fd;
    u32 offset;
    u32 mode;
    u32 flags;
    u32 fd_new;
    u32 fd_outdate_flag;
    s8 path[RFILE_NAME_MAX + 1];
};


struct rfile_dev_info {
    struct cdev cdev;
    dev_t dev_no;
    struct class *rfile_class;
    struct device *device;
};


/* 主控信息 */
struct rfile_main_stru_acore {
    u32 state;
    u32 release_cnt;
    struct rfile_dev_info dev;
    struct task_struct *taskid_process;
    rfile_sem_id sem_taskmain;
    rfile_sem_id sem_read;
    rfile_sem_id sem_write;
    rfile_sem_id sem_reset_before;
    rfile_sem_id sem_resetting;
    struct list_head fd_list; /* C核打开文件&目录链表头 */
    void *data_buffer;
    u32 reset_flag;
    u32 shutdown_flag;
    u32 recover_flag;
};

struct rfile_comm_cmd {
    u32 op_type;
    u32 quelist_addr; /* struct hi_list* */
    u32 cmd_cnt;
    u32 op_num;
    u32 op_cur_cmd;
    u32 fd; /* file:seek,read,write,close,tell;dir:readdir,closedir */
    union {
        s32 mode;     // open,accsess
        s32 offset;   // seek
        u32 readsize; // read,readdir
    };
    union {
        s32 flags;  // open
        s32 whence; // seek
    };
    u32 datalen;
    u8 data[0]; /* path:open,remove,mkdir,opendir,removedir,stat,access,rename, path包含'\0' */
};

struct rfile_comm_resp {
    u32 op_type;
    u32 quelist_addr; /* struct hi_list* */
    u32 cmd_cnt;
    u32 op_num;
    u32 op_cur_cmd;
    s32 ret;
    u32 datalen;
    u8 data[0]; // read, stat, readdir
};

struct rfile_handling_data {
    u32 fd;
    u32 mode;
    u32 flags;
    u8 path[RFILE_NAME_MAX + 1];
};
#pragma pack(0)

struct device *rfile_get_device(void);
struct rfile_main_stru_acore *rfile_get_main_info(void);

#endif /* end of rfile_server.h */
