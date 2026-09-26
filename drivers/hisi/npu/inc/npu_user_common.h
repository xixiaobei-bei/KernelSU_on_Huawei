/*
 * npu_user_common.h
 *
 * about devdrv user common
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
#ifndef __NPU_USER_COMMON_H
#define __NPU_USER_COMMON_H

#include "npu_common_resource.h"
#include "npu_model_description.h"

typedef uint64_t  uint64;
typedef uint32_t  uint32;
typedef uint16_t  uint16;
typedef uint8_t   uint8;

#define MAX_UINT16_NUM  0xFFFF

#define NPU_SHM_MAPS_SIZE   (4 * 1024 * 1024)

#define NPU_MAX_CQ_SLOT_SIZE 128

/* size of npu_rt_task_t is 64B */
#define NPU_RT_TASK_SIZE			64
#define NPU_MAX_STREAM_PRIORITY	7
#define UINT16_MAX					65535
#define npu_bit_mask(width)			(((uint64_t)1 << width) - 1)

#define NPU_CACHELINE_OFFSET	6
#define NPU_CACHELINE_SIZE   64
#define NPU_CACHELINE_MASK   (NPU_CACHELINE_SIZE - 1)

#define NPU_MAX_DEVICE_NUM	1

#define NPU_FUNCTIONAL_SQ_FIRST_INDEX  112
#define NPU_FUNCTIONAL_CQ_FIRST_INDEX  116

#define NPU_MAX_FUNCTIONAL_SQ_NUM    4
#define NPU_MAX_FUNCTIONAL_CQ_NUM    10

#define NPU_MAILBOX_MAX_FEEDBACK  16
#define NPU_MAILBOX_STOP_THREAD   0x0FFFFFFF
#define NPU_BUS_DOWN     0x0F00FFFF

#define NPU_HCCL_NAME_SIZE       64
#define NPU_HCCL_MAX_NODE_NUM    128

#define NPU_MAX_MEMORY_DUMP_SIZE (4 * 1024 * 1024)

#define NPU_BB_DEVICE_ID_INFORM  0x66020004
#define CHIP_BASEADDR_PA_OFFSET 0x200000000000ULL

#define NPU_VALID_ION            1
#define NPU_INVALID_ION          0

enum npu_ts_status {
	NPU_TS_WORK = 0x0,
	NPU_TS_SLEEP,
	NPU_TS_DOWN,
	NPU_TS_INITING,
	NPU_TS_BOOTING,
	NPU_TS_FAIL_TO_SUSPEND,
	NPU_TS_SEC_WORK, // secure power up
	NPU_TS_MAX_STATUS
};

typedef struct npu_ts_sq_info {
	u32 head;
	u32 tail;
	u32 credit;
	u32 index;
	int pid;
	u32 stream_num;
	u8 *vir_addr;
	u64 send_count;
	u64 resv;
} npu_ts_sq_info_t;

struct npu_ts_cq_info {
	u32 head;
	u32 tail;
	volatile u32 count_report;
	u32 index;
	u32 phase;
	u32 int_flag;
	int pid;
	u32 stream_num;
	u8 *vir_addr;
	u64 receive_count;
	u64 resv;
	u8 slot_size;
	u8 rsv[3];
	u32 communication_stream_num; // only use in v200
};

typedef struct npu_stream_info {
	int id;
	u32 devid;
	// used for non-sink stream or sink stream
	u32 cq_index;
	u32 sync_cq_index;
	u32 sq_index;
	u32 resv0;
	u64 resv1;
	int pid;
	u32 strategy;
	u32 create_tid;
	// used for sink stream
	u16 smmu_substream_id;
	u8 priority;
	u8 resv[1];
} npu_stream_info_t;

#define NPU_MESSAGE_PAYLOAD 64
struct npu_mailbox_user_message {
	u8 message_payload[NPU_MESSAGE_PAYLOAD];
	int message_length;
	int feedback_num;
	/* if a sync message need feedback, must alloc buffer for feedback data
	 * if a async message need feedback, set this to null,
	 * because driver will send a callback parameter to callback func,
	 * app has no need to free callback parameter in callback func.
	 */
	u8 *feedback_buffer;
	int sync_type;
	int cmd_type;
	int message_index;
	int message_pid;
};

struct npu_mailbox_feedback {
	void (*callback)(void *data);
	u8 *buffer;
	int feedback_num;
	int process_result;
};

struct npu_svm_to_devid {
	u32 src_devid;
	u32 dest_devid;
	unsigned long src_addr;
	unsigned long dest_addr;
};

struct npu_channel_info_devid {
	char name[NPU_HCCL_NAME_SIZE];
	u32 handle;
	u32 event_id;
	u32 src_devid;
	u32 dest_devid;
	void *dest_doorbell_pa;
	void *ack_doorbell_pa;
	void *dest_mailbox_pa;
	/* for ipc event query */
	u32 status;
	u64 timestamp;
};

struct npu_trans_info {
	u32 src_devid;
	u32 dest_devid;
	u8 ways;
};

struct npu_hardware_inuse {
	u32 devid;
	u32 ai_core_num;
	u32 ai_core_error_bitmap;
	u32 ai_cpu_num;
	u32 ai_cpu_error_bitmap;
};

struct npu_manager_devinfo {
	u8 env_type;
	u32 dev_id;
	u32 ctrl_cpu_ip;
	u32 ctrl_cpu_id;
	u32 ctrl_cpu_core_num;
	u32 ctrl_cpu_endian_little;
	u32 ts_cpu_core_num;
	u32 ai_cpu_core_num;
	u32 ai_core_num;
	u32 ai_cpu_bitmap;
	u32 ai_core_id;
	u32 ai_cpu_core_id;
	u32 num_dev;
	u32 devids[NPU_MAX_DEVICE_NUM];
};

struct npu_occupy_stream_id {
	u32 sqcq_strategy;
	u16 count;
	u16 id[NPU_MAX_STREAM_ID];
};

struct npu_module_status {
	u8 lpm3_start_fail;
	u8 lpm3_lost_heart_beat;
	u8 ts_start_fail;
	u8 ts_lost_heart_beat;
	u8 ts_sram_broken;
	u8 ts_sdma_broken;
	u8 ts_bs_broken;
	u8 ts_l2_buf0_broken;
	u8 ts_l2_buf1_broken;
	u8 ts_spcie_broken;
};

/* The info memory of reserved memory order: 1.SQ_INFO, 2.CQ_INFO,
 * 3.STREAM_INFO, 4.HWTS_SQ_INFO, 5.HWTS_CQ_INFO, 6.MODEL_INFO,
 * 7.PROFILINGL_INFO 8.TS_STAUS_INFO
 */

/* reserved memory info size */
#define NPU_SQ_INFO_SIZE   sizeof(struct npu_ts_sq_info)
#define NPU_CQ_INFO_SIZE   sizeof(struct npu_ts_cq_info)
#define NPU_STREAM_INFO_SIZE   sizeof(struct npu_stream_info)

#define NPU_SQ_INFO_OCCUPY_SIZE \
	(NPU_SQ_INFO_SIZE * NPU_MAX_SQ_NUM)
#define NPU_CQ_INFO_OCCUPY_SIZE \
	(NPU_CQ_INFO_SIZE * NPU_MAX_CQ_NUM)
#define NPU_STREAM_INFO_OCCUPY_SIZE \
	(NPU_STREAM_INFO_SIZE * NPU_MAX_STREAM_ID)

#define NPU_MAX_INFO_SIZE \
	(NPU_INFO_OCCUPY_SIZE + sizeof(u32))
#define NPU_MAX_INFO_ORDER   (get_order(NPU_MAX_INFO_SIZE))

/* add necessary dfx function if you need */
enum npu_dfx_cmd {
	NPU_DFX_QUERY_STATUS,
	NPU_DFX_MAX_CMD
};

/*
 * |___SQ___|____INFO_____|__DOORBELL___|___CQ____|
 */
#define NPU_VM_BLOCK_OFFSET  (32 * 1024 * 1024)
#define NPU_VM_CQ_QUEUE_SIZE  (64 * 1024 * 1024)
#define NPU_VM_CQ_SLOT_SIZE  (128 * 1024)

#define NPU_VM_MEM_START  0xFFC0000000ULL
#define NPU_VM_MEM_SIZE \
	(NPU_VM_BLOCK_OFFSET * 3 + NPU_VM_CQ_QUEUE_SIZE)

/* *custom ioctrl* */
typedef enum {
	NPU_IOC_GET_SVM_SSID,
	NPU_IOC_GET_CHIP_INFO,
	NPU_IOC_REBOOT,
	NPU_IOC_LOAD_MODEL_BUFF,
	NPU_IOC_RESERVED
} npu_custom_ioc_t;

typedef struct {
	u32 version;
	u32 cmd;
	u32 result;
	u32 reserved;
	u64 arg;
} npu_custom_para_t;

typedef struct tag_stream_buffer_info {
	uint32_t stream_id;
	uint32_t mod_id;
	uint64_t len;
	uint64_t *addr;
	uint8_t  priority;
	uint8_t  reserve[7];
} npu_stream_buff_info_t; // total 32 Bytes

/* just for ai core bypass mode */
#define AI_PAGE_SIZE  4096
#define AI_PAGE_MASK  (~(AI_PAGE_SIZE - 1))

struct process_info {
	pid_t vpid;
	uint64 ttbr;
	uint64 tcr;
	int pasid;
	uint32 flags;
};

struct npu_chip_info {
	uint32 l2_size;
	uint32 reserved[3];
};

#define STREAM_STRATEGY_NONSINK    0
#define STREAM_STRATEGY_SINK       (0x1 << 0)
#define STREAM_STRATEGY_LONG       (0x1 << 1)
#define STREAM_STRATEGY_MAX        (0x1 << 2)

enum {
	SQCQ_PROCESSED_BY_USER_DRIVER = 0,
	SQCQ_PROCESSED_BY_KERNEL = 1
};

enum {
	EVENT_STRATEGY_NONSINK = 0,
	EVENT_STRATEGY_SINK = 1,
	EVENT_STRATEGY_TS = 2,
	EVENT_STRATEGY_HWTS = 3,
	EVENT_STRATEGY_MAX
};

typedef struct npu_stream_alloc_ioctl_info {
	u16 strategy;
	u16 priority;
	int stream_id;
	u32 devid;
	u32 sqcq_strategy;
} npu_stream_alloc_ioctl_info_t;

struct npu_stream_free_ioctl_info {
	int stream_id;
	u32 sqcq_strategy;
};

typedef struct npu_event_alloc_ioctl_info {
	u16 strategy;
	int event_id;
	u32 devid;
} npu_event_alloc_ioctl_info_t;

typedef struct npu_event_free_ioctl_info {
	u16 strategy;
	int event_id;
} npu_event_free_ioctl_info_t;

// for custom ioctl power up&down
typedef struct npu_work_mode_info {
	uint32_t work_mode;
	uint32_t flags;
} npu_work_mode_info_t;

struct npu_power_down_info {
	npu_work_mode_info_t secure_info;
	u32 sqcq_strategy;
};

struct npu_limit_time_info {
	u32 type;
	u32 time_out;
};

typedef struct hwts_switch_pool_info {
	uint32_t work_mode;
	uint32_t action;
	uint32_t flags;
} hwts_switch_pool_info_t;

typedef struct {
	int ionfd;
	int result;    /* 0 -- NPU_INVALID_ION; 1 -- NPU_VALID_ION */
} npu_check_ion_t;

struct npu_iova_ioctl_info {
	int fd;
	int prot;
	u64 vaddr;
};

#endif
