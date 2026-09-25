/*
 * npu_common.h
 *
 * about npu common
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
#ifndef __NPU_COMMON_H
#define __NPU_COMMON_H

#include <linux/types.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/mutex.h>
#include <linux/time.h>
#include <linux/of_device.h>
#include <linux/notifier.h>
#include <linux/sched.h>
#include <linux/device.h>
#include <linux/version.h>
#include <linux/pm_wakeup.h>
#include <linux/atomic.h>

#include "devdrv_user_common.h"

#define NPU_DEV_NUM               1
#define DEVDRV_NO_NEED_TO_INFORM  0
#define DEVDRV_HAVE_TO_INFORM     1
#define DEVDRV_WAKELOCK_SIZE      56
#define DEVDRV_INVALID_FD_OR_NUM  (-1)
#define DEVDRV_SQ_CQ_MAP     0
#define DEVDRV_SQ_FLOOR      16
#define DEVDRV_CQSQ_INVALID_INDEX  0xFEFE
#define CQ_HEAD_UPDATE_FLAG   0x1
#define DEVDRV_REPORT_PHASE   0x8000
#define MAX_MEM_INFO_NUM       4
#define NPU_DDR_CONFIG_VALID_MAGIC      0x5A5A5A5A

#define UNUSED(expr) \
	do { \
		(void)(expr); \
	} while (0)

#ifndef __ALIGN_MASK
#define __ALIGN_MASK(x, mask)   (((x) + (mask)) & ~(mask))
#endif
#define ALIGN_UP(x, a)  __ALIGN_MASK((x), ((a) - 1))
#ifndef ALIGN_DOWN
#define ALIGN_DOWN(x, a)  ((x) & ~((a) - 1))
#endif

#define BITMAP_GET(val, pos)         (((val) >> (pos)) & 0x01)

typedef unsigned long long vir_addr_t;

struct devdrv_mailbox_sending_queue {
	spinlock_t spinlock;
	volatile int status; /* mailbox busy or free */
	int mailbox_type; /* mailbox communication method: SPI+SRAM or IPC */
	struct workqueue_struct *wq;
	struct list_head list_header;
	void *message;
};

struct devdrv_mailbox {
	struct devdrv_mailbox_sending_queue send_queue;
	u8 __iomem *send_sram;
	u8 __iomem *receive_sram;
	volatile int working;
	struct mutex send_mutex;
};


struct devdrv_parameter {
	struct list_head list;
	pid_t pid;
	u32 cq_slot_size;
	u16 group_id;  /* docker */
	u16 tflops;
	u16 disable_wakelock;
};

typedef struct excep_time_t {
	u64 tv_sec;
	u64 tv_usec;
} excep_time;

struct devdrv_heart_beat_sq {
	u32 number;  /* increment counter */
	u32 cmd;   /* always 0xAABBCCDD */
	struct timespec64 stamp;  /* system time */
	u32 devid;
	u32 reserved;
	struct timespec64 wall;   /* wall time */
	u64 cntpct;   /* ccpu sys counter */
};

struct devdrv_heart_beat_cq {
	u32 number;   /* increment counter */
	u32 cmd;   /* always 0xAABBCCDD */
	u32 syspcie_sysdma_status;  /* upper 16 bit: syspcie, lower 16 bit: sysdma */
	u32 aicpu_heart_beat_exception;
	u32 aicore_bitmap;  /* every bit identify one aicore, bit0 for core0, value 0 is ok */
	u32 ts_status;   /* ts working status, 0 is ok */
	u32 report_type;  /* 0: heart beat report, 1: exception report */
	u32 exception_code;
	struct timespec64 exception_time;
};

struct devdrv_heart_beat_node {
	struct devdrv_heart_beat_sq *sq;
	struct list_head list;
	struct work_struct work; /* HOST manager use this to
	                          * add heart beat work into workqueue
	                          * DEVICE manager not use
	                          */
	volatile u32 useless; /* flag this node is valid or invalid */
};

struct devdrv_cm_info {
	dma_addr_t dma_handle;
	void *cpu_addr;
	u64 user_va; // user space virtual addr
	u64 size;
	u32 valid;
};

struct devdrv_continuous_mem {
	struct devdrv_cm_info mem_info[MAX_MEM_INFO_NUM];
	u64 total_size;
	spinlock_t cm_spinlock;
};

struct devdrv_event_info {
	u32 id;
	u32 devid;
	struct list_head list;
	spinlock_t spinlock;
};

struct devdrv_manager_lpm3_func {
	u32 lpm3_heart_beat_en;
};

struct devdrv_manager_ts_func {
	u32 ts_heart_beat_en;
};

struct devdrv_device_manager_config {
	struct devdrv_manager_ts_func ts_func;
	struct devdrv_manager_lpm3_func lpm3_func;
};

struct devdrv_heart_beat {
	struct list_head queue;
	spinlock_t spinlock;
	struct workqueue_struct *hb_wq;
	struct timer_list timer;
	u32 sq;
	u32 cq;
	volatile u32 cmd_inc_counter; /* increment counter for sendind heart beat cmd */
	void *exception_info;
	volatile u32 stop; /* use in host manager heart beat to device,
	                    * avoid access null ptr to heart beat node
	                    * when heart beat is stop */
	volatile u32 too_much; /* flag if too much heart beat waiting in queue to be sent */
	volatile u32 broken;
	volatile u32 working;
};

struct devdrv_dev_ctx {
	 /* device id assigned by local device driver */
	u8 devid;
	u8 plat_type;
	u32 sink_stream_num;
	u32 stream_num;
	u32 event_num;
	u32 sq_num;
	u32 cq_num;
	u32 model_id_num;
	u32 task_id_num;
	u32 notify_id_num;
	u32 will_powerdown;
	u32 ctrl_core_num;
	struct list_head proc_ctx_list;
	struct list_head rubbish_context_list;
	struct list_head sink_stream_available_list;
	struct list_head stream_available_list;
	struct list_head event_available_list;
	struct list_head model_available_list;
	struct list_head task_available_list;
	struct list_head notify_available_list;
	struct list_head sq_available_list;
	struct list_head cq_available_list;
	struct list_head resource_software_list;
	struct list_head resource_hardware_list;
	struct devdrv_mailbox mailbox;
	u32 ai_cpu_core_num;
	u32 ai_core_num;
	u32 ai_subsys_ip_broken_map;
	spinlock_t ts_spinlock; /* spinlock for read write ts status */
	struct devdrv_heart_beat heart_beat;
	struct devdrv_device_manager_config config;
	struct devdrv_hardware_inuse inuse;
	struct cdev devdrv_cdev;
	struct device *npu_dev;
	void *dfx_cqsq_addr;  // pointer struct devdrv_dfx_cqsq
	u32 ts_work_status;
	u32 power_stage;  /* for power manager */
	u32 secure_state;  /* indicates npu state:secure or non_secure */
	void *event_addr;
	void *sq_sub_addr;
	void *cq_sub_addr;
	void *stream_sub_addr;
	void *sink_stream_sub_addr;
	void *model_addr;
	void *task_addr;
	void *notify_addr;
	// add for sub info in LTE
	void *dev_ctx_sub;
	spinlock_t spinlock;   // should rename as dev_ctx_spin_lock
	spinlock_t event_spinlock;
	spinlock_t model_spinlock;
	spinlock_t task_spinlock;
	spinlock_t notify_spinlock;
	spinlock_t resource_software_spinlock;
	spinlock_t resource_hardware_spinlock;
	char wakelock_name[DEVDRV_WAKELOCK_SIZE]; /* for power manager: wakelock */
	struct wakeup_source wakelock;
	// spinlock_t power_spinlock;
	struct list_head parameter_list; /* list for parameter */
	struct mutex cq_mutex_t;
	struct mutex stream_mutex_t; // should delete to proc_ctx
	struct mutex event_mutex_t;
	struct mutex model_mutex_t;
	struct mutex task_mutex_t;
	struct mutex notify_mutex_t;
	struct mutex cm_mutex_t;
	struct devdrv_continuous_mem *cm;
	void *hisi_svm;
	atomic_t open_access;
	atomic_t open_success;
	atomic_t power_access;
	atomic_t power_success;
	struct mutex npu_wake_lock_mutex;
	struct mutex npu_power_up_off_mutex;
	struct mutex npu_open_release_mutex;
};

enum secure_state {
	NPU_NONSEC = 0,
	NPU_SEC = 1,
	NPU_SEC_END
};

struct npu_chip_cfg {
	u32 valid_magic; /* if value is 0x5a5a5a5a, valid_magic is ok */
	u32 aicore_disable_bitmap; /* bit0 is aicore0, bit1 is aicore1;each bit:0:enable 1:disable */
};

void dev_ctx_array_init(void);

int devdrv_add_proc_ctx(struct list_head *proc_ctx, u8 dev_id);

int devdrv_remove_proc_ctx(const struct list_head *proc_ctx, u8 dev_id);

int devdrv_add_proc_ctx_to_rubbish_ctx_list(struct list_head *proc_ctx, u8 dev_id);

void set_dev_ctx_with_dev_id(struct devdrv_dev_ctx *dev_ctx, u8 dev_id);

struct devdrv_dev_ctx *get_dev_ctx_by_id(u8 dev_id);

void devdrv_set_sec_stat(u8 dev_id, u32 state);

u32 devdrv_get_sec_stat(u8 dev_id);
#endif  /* __NPU_COMMON_H */
