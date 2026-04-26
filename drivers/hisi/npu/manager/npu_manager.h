/*
 * npu_manager.c
 *
 * about npu manager
 *
 * Copyright (c) 2012-2019 Huawei Technologies Co., Ltd.
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
#ifndef __NPU_MANAGER_H
#define __NPU_MANAGER_H

#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/uio_driver.h>
#include <linux/notifier.h>
#include <linux/radix-tree.h>

#include "npu_user_common.h"

#define NPU_MANAGER_DEVICE_ENV  0

#define NPU_AI_SUBSYS_SDMA_WORKING_STATUS_OFFSET  5
#define NPU_AI_SUBSYS_SPCIE_WORKING_STATUS_OFFSET 6

struct npu_black_box {
	struct semaphore black_box_sema;
	spinlock_t spinlock;
	struct list_head exception_list;
	u32 exception_num;
	pid_t black_box_pid;
	u8 thread_should_stop;
};

struct npu_manager_info {
	/* number of devices probed by pcie */
	u32 prob_num;

	u32 num_dev;

	u32 plat_info;   /* 0:device side, 1: host side */
	u32 dev_id_flag[NPU_MAX_DEVICE_NUM]; /* get devce id from host */
	/* device id assigned by host device driver */
	u32 dev_id[NPU_MAX_DEVICE_NUM];
	struct cdev cdev;
	struct device *dev; /* device manager dev */
	spinlock_t spinlock;
	struct workqueue_struct *dev_rdy_work;
	int msg_chan_rdy;  /* wait for msg channel ready */
	wait_queue_head_t msg_chan_wait;
	struct list_head pm_list_header;  /* for power manager */
	spinlock_t pm_list_lock;   /* for power manager */
	struct notifier_block ipc_monitor;
	struct notifier_block m3_ipc_monitor;
	/* for heart beat between TS and driver
	 * DEVICE manager use this workqueue to
	 * start a exception process.
	 */
	struct workqueue_struct *heart_beat_wq;
	struct npu_black_box black_box;
	struct workqueue_struct *ipc_event_recycle;
	u32 host_type;
};

struct npu_manager_info *npu_get_manager_info(void);

#endif /* __NPU_MANAGER_H */
