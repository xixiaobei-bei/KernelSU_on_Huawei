/*
 * linux_kernel_osal.c
 *
 * This is for linux_kernel_osal proc.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#ifndef __VFMW_LINUX_KERNEL_OSAL_HEADER__
#define  __VFMW_LINUX_KERNEL_OSAL_HEADER__

#include <asm/cacheflush.h>
#include <linux/kthread.h>
#include <linux/hrtimer.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/stat.h>
#include <linux/wait.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/fcntl.h>
#include <linux/miscdevice.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/string.h>
#include <linux/delay.h>
#include <linux/version.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/ioctl.h>
#include <linux/semaphore.h>
#include "vfmw.h"

#define UM_COUNT_OF_A_MM        1000000
#define MM_COUNT_OF_A_S         1000

typedef struct hi_kern_event_s {
	wait_queue_head_t queue_head;
	SINT32 flag;
} osal_event;

typedef struct hi_kern_irq_lock_s {
	spinlock_t    irq_lock;
	unsigned long irq_lockflags;
	int           is_init;
} osal_irq_spin_lock;

typedef  irq_handler_t osal_irq_handler_t;
typedef  struct task_struct    *osal_task;
typedef  struct file            osal_file;
typedef  struct semaphore       osal_sema;
typedef  osal_event             osal_task_mutex;

/* time: get in ms/us */
UINT64 oasl_get_time_in_ms(void);
UINT64 osal_get_time_in_us(void);

/* file: open/close/read/write */
SINT32 osal_file_write(const char *buf, int len, struct file *filp);

/* linux kernel osal function pointer initialize */
void       osal_init_interface(void);

#endif
