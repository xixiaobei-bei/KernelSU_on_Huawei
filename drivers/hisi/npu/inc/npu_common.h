/*
 * npu_common.h
 *
 * about npu common
 *
 * Copyright (c) 2012-2020 Huawei Technologies Co., Ltd.
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
#include <linux/rwsem.h>

#include "npu_user_common.h"
#define SIZE_4KB    0x1000
#define SIZE_1MB    0x100000

#define NPU_NO_NEED_TO_INFORM  0
#define NPU_HAVE_TO_INFORM     1
#define NPU_WAKEUP_SIZE        56
#define NPU_INVALID_FD_OR_NUM   (-1)
#define NPU_SQ_CQ_MAP     0
#define NPU_SQ_FLOOR      16
#define NPU_CQSQ_INVALID_INDEX  0xFEFE
#define CQ_HEAD_UPDATE_FLAG   0x1
#define NPU_REPORT_PHASE   0x8000
#define MAX_MEM_INFO_NUM       1
#define NPU_DDR_CONFIG_VALID_MAGIC      0x5A5A5A5A
#define NPU_IDLE_TIME_OUT_DEAFULT_VALUE 5000
#ifndef FPGA_PLAT
#define NPU_TASK_TIME_OUT_DEAFULT_VALUE 18000
#else
#define NPU_TASK_TIME_OUT_DEAFULT_VALUE 300000
#endif
#define NPU_IDLE_TIME_OUT_MIN_VALUE     1000
#define NPU_TASK_TIME_OUT_MIN_VALUE     3000
/* hiaiServer business Max timeout is 25s */
#define NPU_TASK_TIME_OUT_MAX_VALUE     25000
#define NPU_AICORE_ENABLE               0

#define NPU_INTERFRAME_IDLE_TIME_OUT_DEFAULT_VALUE 2

#define NPU_PHASE_TOGGLE_STATE_0 0
#define NPU_PHASE_TOGGLE_STATE_1 1
#define NPU_DEV_WORKMODE_DEFAULT 0x0 /* device work with non dfx */
#define NPU_DEV_WORKMODE_PROFILING (0x01 << 0U) /* device work with profiling */
#define NPU_DEV_WORKMODE_DUMP (0x01 << 1U) /* device work with dump */

/* errno */
#ifndef NOSUPPORT
#define NOSUPPORT             4004
#endif

#ifndef TS_EXCEPTION
#define TS_EXCEPTION          0x30
#endif

#ifndef UINT8_MAX
#define UINT8_MAX             0xFF
#endif

#ifndef UINT32_MAX
#define UINT32_MAX            0xFFFFFFFF
#endif

#define unused(expr) do { \
		(void)(expr); \
	} while (0)

#ifndef __align_mask
#define __align_mask(x, mask)   (((x) + (mask)) & ~(mask))
#endif
#define align_up(x, a)  __align_mask((x), ((a) - 1))
#ifndef align_down
#define align_down(x, a)  ((x) & ~((a) - 1))
#endif

#define bitmap_get(val, pos) (((u64)(val) >> (unsigned int)(pos)) & 0x01)
#define BITMAP_SET(val, pos) ((val) = ((val) | (0x01 << (unsigned int)(pos))))

#ifndef read_uint32
#define read_uint32(uw_value, addr) \
	((uw_value) = *((volatile unsigned int *)((uintptr_t)(addr))))
#endif

#ifndef write_uint32
#define write_uint32(uw_value, addr) \
	(*((volatile u32 *)((uintptr_t)(addr))) = (uw_value))
#endif

enum npu_workmode {
	NPU_NONSEC = 0, /* task */
	NPU_SEC = 1,
	NPU_ISPNN_SEPARATED = 2,
	NPU_ISPNN_SHARED = 3,
	NPU_INIT = 4,   /* RuntimeInit,only power up TS */
	NPU_WORKMODE_MAX
};

#define npu_pm_safe_call(dev_ctx, subip, expr) do { \
	mutex_lock(&((dev_ctx)->npu_power_up_off_mutex)); \
	if (((((dev_ctx)->pm.work_mode >> NPU_SEC) & 0x1) == 0) && \
		((((dev_ctx)->pm.cur_subip_set) >> (subip)) & 0x1)) { \
		(expr); \
	} \
	mutex_unlock(&((dev_ctx)->npu_power_up_off_mutex)); \
} while (0)

#define npu_pm_safe_call_with_return(dev_ctx, subip, expr, ret) do { \
	mutex_lock(&((dev_ctx)->npu_power_up_off_mutex)); \
	if (((((dev_ctx)->pm.work_mode >> NPU_SEC) & 0x1) == 0) && \
		((((dev_ctx)->pm.cur_subip_set) >> (subip)) & 0x1)) { \
		(ret) = (expr); \
	} \
	mutex_unlock(&((dev_ctx)->npu_power_up_off_mutex)); \
} while (0)

typedef unsigned long long vir_addr_t;

struct npu_mailbox_sending_queue {
	spinlock_t spinlock;
	volatile int status; /* mailbox busy or free */
	int mailbox_type; /* mailbox communication method: SPI+SRAM or IPC */
	struct workqueue_struct *wq;
	struct list_head list_header;
	void *message;
};

struct npu_mailbox {
	struct npu_mailbox_sending_queue send_queue;
	u8 __iomem *send_sram;
	u8 __iomem *receive_sram;
	volatile int working;
	struct mutex send_mutex;
};

struct npu_parameter {
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

struct npu_heart_beat_sq {
	u32 number;  /* increment counter */
	u32 cmd;   /* always 0xAABBCCDD */
	struct timespec64 stamp;  /* system time */
	u32 devid;
	u32 reserved;
	struct timespec64 wall;   /* wall time */
	u64 cntpct;   /* ccpu sys counter */
};

struct exception_report_info {
	/* upper 16 bit: syspcie, lower 16 bit: sysdma */
	u32 syspcie_sysdma_status;
	u32 aicpu_heart_beat_exception;
	/* every bit identify one aicore, bit0 for core0, value 0 is ok */
	u32 aicore_bitmap;
	u32 ts_status;   /* ts working status, 0 is ok */
};

struct hwts_exception_report_info {
	u32 model_id;
	u16 persist_stream_id;
	u16 persist_task_id;
	u16 channel_id;
	u16 hwts_sq_id;
	u16 task_id;
	u8  exception_type;
	u8  service_type;
};

struct npu_heart_beat_cq {
	u32 number;   /* increment counter */
	u32 cmd;   /* always 0xAABBCCDD */
	u32 report_type;  /* 0: heart beat report, 1: exception report */
	u32 exception_code;
	struct timespec64 exception_time;
	union {
		struct exception_report_info exception_info;
		struct hwts_exception_report_info hwts_exception_info;
	} u;
};

struct npu_heart_beat_node {
	struct npu_heart_beat_sq *sq;
	struct list_head list;
	/* HOST manager use this to add heart beat work into workqueue
	 * DEVICE manager not use
	 */
	struct work_struct work;
	volatile u32 useless; /* flag this node is valid or invalid */
};

struct npu_cm_info {
	dma_addr_t dma_handle;
	void *cpu_addr;
};

struct npu_continuous_mem {
	struct npu_cm_info mem_info[MAX_MEM_INFO_NUM];
};

struct npu_attach_sc {
	int fd;
	u64 offset;
	u64 size;
};

struct npu_event_info {
	u32 id;
	u32 devid;
	struct list_head list;
	spinlock_t spinlock;
};

struct npu_manager_lpm3_func {
	u32 lpm3_heart_beat_en;
};

struct npu_manager_ts_func {
	u32 ts_heart_beat_en;
};

struct npu_device_manager_config {
	struct npu_manager_ts_func ts_func;
	struct npu_manager_lpm3_func lpm3_func;
};

struct npu_dev_ctx;

struct npu_heart_beat {
	struct list_head queue;
	spinlock_t spinlock;
	struct workqueue_struct *hb_wq;
	struct timer_list timer;
	u32 sq;
	u32 cq;
	/* increment counter for sendind heart beat cmd */
	volatile u32 cmd_inc_counter;
	void *exception_info;
	volatile u32 stop; /* use in host manager heart beat to device,
						* avoid access null ptr to heart beat node
						* when heart beat is stop
						*/
	/* flag if too much heart beat waiting in queue to be sent */
	volatile u32 too_much;
	volatile u32 broken;
	volatile u32 working;

	void (*hwts_exception_callback)
		(struct npu_dev_ctx *, struct hwts_exception_report_info *);
};

struct npu_interframe_idle_manager {
	atomic_t enable; /* 1:enable, 0:disable, only protect enable */
	vir_addr_t strategy_table_addr;
	u32 idle_subip;
	u32 idle_time_out;
	struct delayed_work work;
	atomic_t wq_processing;
	void *private_data;
};

enum npu_status {
	NPU_STATUS_NORMAL = 0,
	NPU_STATUS_EXCEPTION = 1,
	NPU_STATUS_MAX
};

struct npu_power_manage {
	u32 work_mode;  /* indicates npu work mode:secure,non_secure,ispnn */
	u32 work_mode_flags; /* work mode flags for dfx */
	u32 cur_subip_set;
	u32 npu_idle_time_out;
	u32 npu_task_time_out;
	atomic_t task_ref_cnt;
	vir_addr_t action_table_addr;
	vir_addr_t strategy_table_addr;
	void *private_data[2];
	atomic_t idle_wq_processing;
	struct delayed_work idle_work;
	hwts_switch_pool_info_t switch_info;
	struct npu_interframe_idle_manager interframe_idle_manager;

	struct rw_semaphore exception_lock;
	struct mutex task_set_lock;
	struct list_head *task_set;
	struct list_head task_available_list;
	void *task_buffer;
	u32 npu_exception_status;
	u16 npu_exception_task_id;
	u16 npu_exception_persist_stream_id;
	u16 npu_exception_persist_task_id;
};

struct npu_task_info {
	struct list_head node;
	u16  task_id;
	u16  stream_id;
};

struct npu_id_entity {
	struct list_head list;
	u32 id;
	u8 is_allocated;
	u8 reserved[3];
	char data[0];
};

struct npu_id_allocator {
	struct mutex lock;
	void *id_entity_base_addr;
	u32 entity_size; /* id node size */
	u32 entity_num; /* id num */
	u32 start_id; /* id start */
	struct list_head available_list;
	u32 available_id_num;
};

typedef enum {
	NPU_ID_TYPE_STREAM = 0,
	NPU_ID_TYPE_SINK_LONG_STREAM = 1,
	NPU_ID_TYPE_SINK_STREAM = 2,
	NPU_ID_TYPE_MODEL = 3,
	NPU_ID_TYPE_TASK = 4,
	NPU_ID_TYPE_EVENT = 5,
	NPU_ID_TYPE_HWTS_EVENT = 6,
	NPU_ID_TYPE_NOTIFY = 7,
	NPU_ID_TYPE_MAX
} npu_id_type;

struct npu_dev_ctx {
	/* device id assigned by local device driver */
	u8 devid;
	u8 plat_type;
	u32 sq_num;
	u32 cq_num;
	u32 notify_id_num;
	u32 will_powerdown;
	u32 ctrl_core_num;
	pid_t pid;
	struct list_head proc_ctx_list;
	struct list_head rubbish_context_list;
	struct list_head notify_available_list;
	struct list_head sq_available_list;
	struct list_head cq_available_list;
	struct list_head resource_hardware_list;
	struct npu_mailbox mailbox;
	struct npu_id_allocator id_allocator[NPU_ID_TYPE_MAX];
	u32 ai_cpu_core_num;
	u32 ai_core_num;
	u32 ai_subsys_ip_broken_map;
	spinlock_t ts_spinlock; /* spinlock for read write ts status */
	struct npu_heart_beat heart_beat;
	struct npu_device_manager_config config;
	struct npu_hardware_inuse inuse;
	struct npu_power_manage pm;
	struct cdev npu_cdev;
	struct device *npu_dev;
	void *dfx_cqsq_addr;  // pointer struct npu_dfx_cqsq
	u32 ts_work_status;
	u32 power_stage;  /* for power manager */
	void *sq_sub_addr;
	void *cq_sub_addr;
	void *notify_addr;
	/* sync cq */
	void *sync_cq;
	// add for sub info in LTE
	void *dev_ctx_sub;
	spinlock_t spinlock;   // should rename as dev_ctx_spin_lock
	spinlock_t notify_spinlock;
	spinlock_t resource_hardware_spinlock;
	struct wakeup_source wakeup; /* for power manager */
	struct list_head parameter_list; /* list for parameter */
	struct mutex cq_mutex_t;
	struct mutex notify_mutex_t;
	struct npu_continuous_mem *cm;
	void *svm_manager;
	atomic_t open_access;
	atomic_t open_success;
	atomic_t power_access;
	atomic_t power_success;
	struct mutex npu_power_up_off_mutex;
	struct mutex npu_open_release_mutex;
	struct npu_platform_info *plat_info;
};

enum npu_time_out_type {
	NPU_LOW_POWER_TIMEOUT = 0,
	NPU_STREAM_SYNC_TIMEOUT,
};

enum npu_workmode_flag {
	POWER_OFF = 0,
	POWER_ON = 1
};

struct npu_chip_cfg {
	u32 valid_magic; /* if value is 0x5a5a5a5a, valid_magic is ok */
	/* bit0 is aicore0, bit1 is aicore1; each bit:0:enable 1:disable */
	u32 aicore_disable_bitmap;
	uint32_t platform_specification; /* follow efuse Grading chip type */
};

enum npu_exception_type {
	NPU_EXCEPTION_TYPE_HWTS_TASK_EXCEPTION = 0x0,
	NPU_EXCEPTION_TYPE_HWTS_TIMEOUT_EXCEPTION,
	NPU_EXCEPTION_TYPE_HWTS_TRAP_EXCEPTION,
	NPU_EXCEPTION_TYPE_HWTS_SQE_ERROR,
	NPU_EXCEPTION_TYPE_HWTS_SW_STATUS_ERROR,
	NPU_EXCEPTION_TYPE_HWTS_BUS_ERROR,
	NPU_EXCEPTION_TYPE_HWTS_POOL_CONFLICT_ERROR,
	NPU_EXCEPTION_TYPE_MAX
};

typedef enum npu_service_type_e {
	NPU_SERVICE_TYPE_NONSEC = 0,
	NPU_SERVICE_TYPE_SEC = 1,
	NPU_SERVICE_TYPE_ISPNN = 2,
	NPU_SERVICE_TYPE_TINY = 3,
	NPU_SERVICE_TYPE_MAX,
} npu_service_type_t;

void dev_ctx_array_init(void);

int npu_add_proc_ctx(struct list_head *proc_ctx, u8 dev_id);

int npu_remove_proc_ctx(const struct list_head *proc_ctx, u8 dev_id);

void set_dev_ctx_with_dev_id(struct npu_dev_ctx *dev_ctx, u8 dev_id);

struct npu_dev_ctx *get_dev_ctx_by_id(u8 dev_id);

void npu_set_sec_stat(u8 dev_id, u32 state);

u32 npu_get_sec_stat(u8 dev_id);

int copy_from_user_safe(void *to, const void __user *from, unsigned long len);

int copy_to_user_safe(void __user *to, const void *from, unsigned long n);

#endif  /* __NPU_COMMON_H */
