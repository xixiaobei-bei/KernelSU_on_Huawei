/*
 * npu_ts_sqe.h
 *
 * about npu ts sqe
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
#ifndef _NPU_TS_SQE_H_
#define _NPU_TS_SQE_H_

#include <linux/types.h>

/**
* @ingroup tsch
* @brief the type defination of task
*/
typedef enum tag_devdrv_ts_task_type {
	TS_TASK_TYPE_KERNEL_AICORE = 0,    /* AI core task */
	TS_TASK_TYPE_KERNEL_AICPU = 1,   /* AI cpu task */
	TS_TASK_TYPE_EVENT_RECORD = 2,   /* event record task */
	TS_TASK_TYPE_STREAM_WAIT_EVENT = 3,  /* stream wait event task */
	TS_TASK_TYPE_FUSION_ISSUE = 4,     /* fusion issue task */
	TS_TASK_TYPE_MEMCPY = 5,          /* memory copy task */
	TS_TASK_TYPE_MAINTENANCE = 6,      /* such as destroy the event or stream */
	TS_TASK_TYPE_CREATE_STREAM = 7,   /* create stream task */
	TS_TASK_TYPE_REMOTE_EVENT_WAIT = 9,  /* wait for event on another device */
	TS_TASK_TYPE_PCTRACE_ENABLE = 10,
	TS_TASK_TYPE_CREATE_L2_ADDR = 11,  /* create L2 addr info for aicpu kernel */
	TS_TASK_TYPE_MODEL_MAINTAINCE = 12,
	TS_TASK_TYPE_MODEL_EXECUTE = 13,
	TS_TASK_TYPE_RDMA_SEND = 16,     /* hccl rdma send task */
	TS_TASK_TYPE_L2_SDMA_TASK_MEMCPY = 17, /* test l2 task memory copy task */
	TS_TASK_TYPE_STREAM_SWITCH = 18,
	TS_TASK_TYPE_STREAM_ACTIVE = 19,
	TS_TASK_TYPE_LABEL_SET = 20,  /* set label for control flow ops */
	TS_TASK_TYPE_LABEL_SWITCH = 21,  /* switch label for control flow ops */
	TS_TASK_TYPE_LABEL_GOTO = 22,  /* goto label for control flow ops */
	TS_TASK_TYPE_PROFILING_ENABLE = 0x40,
	TS_TASK_TYPE_PROFILING_DISABLE = 0x41,
	TS_TASK_TYPE_RESERVED = 0x42
} devdrv_ts_task_type_t;

typedef enum tag_devdrv_ts_task_state {
	TASK_STATE_INIT = 0,
	TASK_STATE_WAIT = 1,
	TASK_STATE_RUN = 2,
	TASK_STATE_COMPLETE = 3,
	TASK_STATE_PENDING = 4,
	TASK_STATE_SDMA_PROCESS_FAILED = 5,
	TASK_STATE_SDMA_PROCESS_SUCCESS = 6
} devdrv_ts_task_state_t;

typedef enum tag_devdrv_ts_report_type {
	TS_REPORT_TYPE_TASK = 0,                 /* task command report */
	TS_REPORT_TYPE_PROFILE_TIMELINE = 1,     /* timeline profile data */
	TS_REPORT_TYPE_PROFILE_TASKBASED = 2,    /* task based profile data */
	TS_REPORT_TYPE_RECYCLE_SQ_FINISHED = 3,  /* recycle sq report */
	TS_REPORT_TYPE_RESERVED
} devdrv_ts_report_type_t;

/**
* @ingroup tsch
* @brief the struct define of report msg when task is completed
*/
typedef struct tag_devdrv_ts_task_report_msg {
	volatile u16 sop : 1; /* start of packet, indicates this is the first 32bit return payload */
	volatile u16 mop : 1; /* middle of packet, indicates the payload is a continuation of previous task return payload */
	volatile u16 eop : 1; /* end of packet, indicates this is the last 32bit return payload.
	                         sop & eop can appear in the same packet, mop & eop can also appear on the same packet */
	volatile u16 report_type : 3;
	volatile u16 stream_id : 10;
	volatile u16 task_id;
	volatile u32 pay_load;
	volatile u16 sq_id : 9;
	volatile u16 reserved : 6;
	volatile u16 phase : 1;
	volatile u16 sq_head;
} devdrv_ts_task_report_msg_t;

/**
* @ingroup tsch
* @brief the struct define of kernel type task
*/
typedef struct tag_devdrv_ts_kernel_task {
	u64 pc_start;
	u64 param_base;
	u64 l2_preload_ctrl;
	u64 literal_src_addr;
	u32 literal_dst_base;
	u32 literal_size;
	u16 block_dim;
	u8 l2_size;
	u8 priority;
	u8 ai_core_alloc_hint_bw;
	u8 ai_core_alloc_hint_l2bw;
	u8 l2_in_main;
	u8 reserved[1];
} devdrv_ts_kernel_task_t;

/**
* @ingroup tsch
* @brief the struct define of event record type task
*/
typedef struct tag_devdrv_ts_event_record_task {
	u16 event_id;        /* offset 8 */
	u8  reserved[46];    /* offset 10 */
} devdrv_ts_event_record_task_t;

/**
* @ingroup tsch
* @brief the struct define of stream wait event type task
*/
typedef struct tag_devdrv_ts_stream_wait_event_task {
	u16 event_id;        /* offset 8 */
	u8  reserved[46];    /* offset 12 */
} devdrv_ts_stream_wait_event_task_t;

/**
* @ingroup tsch
* @brief the struct define of notify record type task
*/
typedef struct tag_devdrv_ts_notify_record_task {
	u16 soc_id;
	u16 notify_id;
	u8 reserved[44];
} devdrv_ts_notify_record_task_t;

/**
* @ingroup tsch
* @brief the struct define of notify wait type task
*/
typedef struct tag_devdrv_ts_notify_wait_task {
	u16 notify_id;
	u8 reserved[46];
} devdrv_ts_notify_wait_task_t;

/**
* @ingroup tsch
* @brief the struct define of fusion type task
*/
typedef struct tag_devdrv_ts_fusion_task {
	u16 flag;           /* offset 8 */
	u8  reserved[46];
} devdrv_ts_fusion_task_t;

enum tag_devdrv_ts_fusion_flag {
	TS_FUSION_BEGIN = 0,
	TS_FUSION_END = 1
};

/**
* @ingroup tsch
* @brief the struct define of memory copy type task
*/
typedef struct tag_devdrv_ts_memcpy_task {
	u64 src_base_addr;
	u64 dst_base_addr;
	u64 length;
	u16 memcpy_type;
	u8  dir;
	u8  reserved2[21];
} devdrv_ts_memcpy_task_t;

/**
* @ingroup tsch
* @brief the struct define of maintenance type task
*/
typedef struct tag_devdrv_ts_maintenance_task {
	u16 goal;  /* offset 8 , 0:stream, 1:event; */
	u16 target_id; /* offset 10 */
	u8 reserved[44];
} devdrv_ts_maintenance_task_t;

#define MAINTENANCE_GOAL_STREAM        0

#define MAINTENANCE_GOAL_EVENT         1

typedef struct tag_devdrv_ts_create_stream {
	u64 pid;
	u64 l2_base_vaddr;
	u64 asid_baddr;
	u64 tcr;  /* translation control register,for ai cpu memory page */
	u32 thread;
	u16 asid;
	u16 smmu_sub_stream_id;
	u16 sq_id;
	u8 priority;
	u8  reserved[5]; // runtime: the size is
} devdrv_ts_create_stream_t;

typedef struct tag_devdrv_ts_create_l2_addr {
	u64 l2_base_vaddr_for_sdma; /* statis page table virtual address for sdma */
	u64 pte_pa;  /* page_4k_table_base[16] physical address */
	u64 pid;  /* profiling for process CreateStream and L2 mistiming */
	u8  reserved[24];
} devdrv_ts_create_l2_addr_t;

typedef struct tag_devdrv_ts_l2_preload_ctrl {
	u64 src_addr;
	u32 preload_size;
	u32 preload_dest;
	u8  reserved[32];
} devdrv_ts_l2_preload_ctrl_t;

typedef struct tag_devdrv_ts_profile_enable {
	u64 pid;
	u8  pmu_event_id[8];  // should use AI_CORE_MAX_PMU_NUM, but it's bad to include profiling_agent.h
	u64 global_start_count_cycle;
	u64 global_stop_count_cycle;
	u8  user_defined_profiling_enable;
	u8  is_timeline_prof_en;
	u8  is_taskbased_prof_en;
	u8  reserved[13];
} devdrv_ts_profile_enable_t;

typedef struct tag_devdrv_ts_profile_disable {
	u64 pid;
	u8  reserved[40];
} devdrv_ts_profile_disable_t;

/* BEGIN  ADD HCCL multi_device feature 2018-01-24 */
typedef struct tag_devdrv_ts_remote_event_wait_task {
	u64 src_mailbox_pa;      /* offset 0 */
	u64 src_doorbell_pa;     /* offset 8 */
	u64 dst_doorbell_pa;     /* offset 16 */
	u16 src_event_id;        /* offset 24 */
	u16 src_device_id;       /* offset 26 */
	u16 dst_device_id;       /* offset 28 */
	u8 channel_type;         /* offset 30 */
	u8 reserved[17];         /* offset 31 */
} devdrv_ts_remote_event_wait_t;

/* END  ADD HCCL multi_device feature 2018-01-24 */
typedef struct tag_ts_pc_trace_task {
	u64 trace_buf_addr;
	u16 task_id;
	u8  reserved[38];
} devdrv_ts_pc_trace_task_t;

typedef struct tag_devdrv_ts_data_dump_task {
	u64 dump_base_addr;
	u32 dump_kind;
	u32 dump_block_size;
	u32 dump_block_dim;
	u16 dump_task_id;
	u8 reserved[26];
} devdrv_ts_data_dump_task_t;

typedef struct tag_devdrv_ts_model_maintaince_task {
	u16 model_id;
	u16 stream_id;
	u16 operation_type;
	u16 stream_type;
	u16 first_task_id;
	u8 reserved[38];
} devdrv_ts_model_maintaince_task_t;

enum tag_devdrv_ts_mmt_operation_type {
	TS_MMT_STREAM_BIND = 0,       /* model stream bind */
	TS_MMT_STREAM_UNBIND = 1,     /* model stream unbind */
	TS_MMT_MODEL_CREATE = 2,      /* model create by task pool */
	TS_MMT_MODEL_DESTROY = 3,     /* model destroy */
	TS_MMT_MODEL_PRE_PROC = 4,
	TS_MMT_RESERVED
};

enum tag_devdrv_ts_mmt_stream_type {
	TS_MMT_HEAD_STREAM = 0,               /* model first stream */
	TS_MMT_WAIT_ACTIVE_STREAM = 1        /* model non-first stream */
};

typedef struct tag_devdrv_ts_model_execute_task {
	u16 model_id;
	u16 first_task_id;
	u8 reserved0[4];
	u64 asid_baddr;
	u64 tcr;
	u16 asid;
	u16 smmu_sub_stream_id;
	u8 reserved[20];
} devdrv_ts_model_execute_task_t;

typedef struct tag_devdrv_ts_rdma_send_task {
	u32 sq_index;
	u32 wqe_index;
	u8  reserved[40];
} devdrv_ts_rdma_send_task_t;

typedef struct tag_devdrv_ts_stream_switch_task {
	u64 pptr;
	u32 value;
	u16 true_stream_id;
	u8 condition;
	u8 reserved[33];
} devdrv_ts_stream_switch_task_t;

typedef struct tag_devdrv_ts_stream_active_task {
	u16 active_stream_id;
	u8 reserved[46];
} devdrv_ts_stream_active_task_t;

typedef struct tag_devdrv_ts_profiler_notify_task {
	u64 profiler_trace_id;
	u8  notify;
	u8 reserved[39];
} devdrv_ts_profiler_notify_task_t;

typedef struct tag_devdrv_ts_label_set_task {
	u16 label_id;
	u8 reserved[46];
} devdrv_ts_label_set_task_t;

typedef struct tag_devdrv_ts_label_switch_task {
	u64 pptr;  // PA
	u32 condition;
	u32 value;
	u16 true_label_id;
	u16 true_label_id_to_node_idx;
	u16 next_task_idx_bak; // backup next_task_idx in persistent_task_pool
	u8 reserved[26];
} devdrv_ts_label_switch_task_t;

typedef struct tag_devdrv_ts_label_goto_task {
	u16 label_id;
	u16 label_id_to_node_idx;
	u16 next_task_idx_bak;  // backup next_task_idx in persistent_task_pool
	u8 reserved[42];
} devdrv_ts_label_goto_task_t;

/**
* @ingroup tsch
* @brief the struct define of task
*/
typedef struct tag_devdrv_ts_task {
	u16 stream_id;      /* offset 0 */
	u16 task_id;        /* offset 2 */
	u16 next_task_idx;  /* offset 4 */
	u16 type;           /* offset 6 */
	u16 next_stream_idx; /* offset 8 */
	u16 task_state;   /* 10 */
	u8  task_prof_en;   /* offset 12 */
	u8  reserved;
	u16 task_info;   /* now used in cloud model rmda stream deal */
	/* 48 bytes */
	union {
		devdrv_ts_kernel_task_t          kernel_task;
		devdrv_ts_event_record_task_t     event_task;
		devdrv_ts_stream_wait_event_task_t stream_wait_event_task;
		devdrv_ts_fusion_task_t          fusion_task;
		devdrv_ts_memcpy_task_t          memcpy_task;
		devdrv_ts_maintenance_task_t     maintenance_task;
		devdrv_ts_create_stream_t        creat_stream;
		devdrv_ts_create_l2_addr_t       create_l2_addr;
		devdrv_ts_profile_enable_t       profile_enable;
		devdrv_ts_profile_disable_t      profile_disable;
		/* BEGIN    ADD HCCL multi_device feature 2018-01-24 */
		devdrv_ts_remote_event_wait_t      remote_event_wait_task;
		/* END      ADD HCCL multi_device feature 2018-01-24 */
		devdrv_ts_pc_trace_task_t         pc_trace_task;
		devdrv_ts_data_dump_task_t      data_dump_task;
		devdrv_ts_notify_wait_task_t    notify_wait_task;
		devdrv_ts_notify_record_task_t  notify_record_task;
		devdrv_ts_model_maintaince_task_t model_maintaince_task;
		devdrv_ts_model_execute_task_t model_execute_task;
		devdrv_ts_rdma_send_task_t         rdma_send_task;
		devdrv_ts_stream_switch_task_t    stream_switch_task;
		devdrv_ts_stream_active_task_t    stream_active_task;
		devdrv_ts_profiler_notify_task_t  profiler_notify_task;
		devdrv_ts_label_set_task_t         label_set_task;
		devdrv_ts_label_switch_task_t      label_switch_task;
		devdrv_ts_label_goto_task_t        label_goto_task;
	} u;
} devdrv_ts_task_t;

#define TEST_HWTS_SQE_LEN    128
#define TEST_HWTS_SQ_LEN     256
#define TEST_HWTS_CQE_LEN    16
#define TEST_HWTS_CQ_LEN     1024
#define TEST_HWTS_SQCQ_NUM   64
#define hwts_sq_offset(sq_idx) \
	((sq_idx) * TEST_HWTS_SQE_LEN * TEST_HWTS_SQ_LEN)
#define hwts_cq_offset(cq_idx) \
	((cq_idx) * TEST_HWTS_CQE_LEN * TEST_HWTS_CQ_LEN)
#define hwts_sq_addr(base, sq_idx) \
	((base) + hwts_sq_offset(sq_idx))
#define hwts_cq_addr(base, cq_idx) \
	((base) + hwts_cq_offset(cq_idx))


typedef struct hwts_sqe_hdr {
	u8 cmd_type;  // 8-bit command identifier
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0; // Reserved field used only by kickstart type (0/1/2)
	u16 stream_id;
	u16 task_id;
	u32 res1[30];
} hwts_sqe_hdr_t;

typedef struct hwts_kernel_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 rd_cond : 1;
	u8 reserved : 1;
	u8 l1_lock : 1;
	u8 l1_unlock : 1;
	u16 block_dim;
	u16 stream_id;
	u16 task_id;
	u32 pc_addr_low;
	u32 pc_addr_high : 16;
	u32 kernel_credit : 8;
	u32 res0 : 4;
	u32 icache_prefetch_cnt : 4;
	u32 param_addr_low;
	u32 param_addr_high : 16;
	u32 l2_in_main : 8;
	u32 res1 : 8;
	u32 literal_addr_low;
	u32 literal_addr_high : 16;
	u32 res2 : 16;
	u32 literal_base_ub;
	u32 res3;
	u32 literal_buff_len;
	u32 res4;
	u32 l2ctrl_addr_low;
	u32 l2ctrl_addr_high : 16;
	u32 res5 : 16;
	u8 l2remap[64];
	u16 l2vaddr_base_low;
	u32 l2vaddr_base_high : 16;
	u32 res6 : 16;
} hwts_kernel_sqe_t;

typedef struct hwts_ph_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0;
	u16 stream_id;
	u16 task_id;
	u32 res1[30];
} hwts_ph_sqe_t;

typedef struct hwts_event_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0;
	u16 stream_id;
	u16 task_id;
	u32 event_id : 10;
	u32 res1 : 22;
	u32 res2[29];
} hwts_event_sqe_t;

typedef struct hwts_notify_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0;
	u16 stream_id;
	u16 task_id;
	u32 notify_id : 10;
	u32 res1 : 22;
	u16 res2;
	u8 kernel_credit; /* task timeout limit = kernel_credit * 2^task_timeout_exp */
	u8 res3;
	u32 res4[28];
} hwts_notify_sqe_t;

typedef struct hwts_write_val_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0;
	u16 stream_id;
	u16 task_id;
	u32 reg_addr_low;
	u16 reg_addr_high;
	u16 awsize : 3; /* 0:8 bits.1:16bits 2:32bits 3:64 bits.4-7:reserved */
	u16 snoop : 1;
	u16 res1 : 4;
	u16 awcache : 4; /* cache property of destnation */
	u16 awprot : 3; /* secure setting,should be setted 2 */
	u16 va : 1;
	u32 write_value_low;
	u32 write_value_high;
	u32 res2[26];
} hwts_write_val_sqe_t;

typedef struct hwts_memcpy_sqe {
	u8 cmd_type;
	u8 ie : 1;
	u8 pre_p : 1;
	u8 post_p : 1;
	u8 wr_cqe : 1;
	u8 reserved : 4;
	u16 res0;
	u16 stream_id;
	u16 task_id;
	u32 res1;
	u32 res2 : 16;
	u32 kernel_credit : 8;
	u32 res3 : 8;
	u32 payload;
	u32 res4[20];
} hwts_memcpy_sqe_t; // 32

typedef struct hwts_cqe {
	volatile u16 p : 1;
	volatile u16 w : 1;
	volatile u16 evt : 1;
	volatile u16 res0 : 1;
	volatile u16 sq_id : 10;
	volatile u16 res1 : 2;
	volatile u16 sq_head;
	volatile u16 stream_id;
	volatile u16 task_id;
	volatile u32 syscnt_low; // status
	volatile u32 syscnt_high; // res0
} hwts_cqe_t;


enum devdrv_hwts_sqe_type {
	DEVDRV_HWTS_SQE_TASK_KICKSTART = 0,
	DEVDRV_HWTS_SQE_PLACE_HOLDER = 3,
	DEVDRV_HWTS_SQE_EVENT_RECORD = 4,
	DEVDRV_HWTS_SQE_WAIT_EVENT = 5,
	DEVDRV_HWTS_SQE_TYPE_RESV
};

typedef struct tag_devdrv_hwts_sqe_hdr {
	u32 type : 8;
	u32 ie : 1;
	u32 pre_p : 1;
	u32 post_p : 1;
	u32 wr_cqe : 1;
	u32 rd_cond : 1;
	u32 res0 : 1;
	u32 l2_lock : 1;
	u32 l2_unlock : 1;
	u32 blh_dim : 16;

	u32 stream_id : 16;
	u32 task_id : 16;
} devdrv_hwts_sqe_hdr_t;

typedef struct tag_devdrv_hwts_kernel_sqe {
// 0
	u32 type : 8;
	u32 ie : 1;
	u32 pre_p : 1;
	u32 post_p : 1;
	u32 wr_cqe : 1;
	u32 rd_cond : 1;
	u32 res0 : 1;
	u32 l2_lock : 1;
	u32 l2_unlock : 1;
	u32 blh_dim : 16;
	u32 stream_id : 16;
	u32 task_id : 16;
// 2word
	u32 pc_addr_low;
	u32 pc_addr_high : 16;
	u32 kernel_credit : 8;
	u32 res2 : 2;
	u32 prefetch_cnt : 6;
// 4w
	u32 param_addr_low;
	u32 param_addr_high : 16;
	u32 l2_in_main : 8;
	u32 res3 : 8;
// 6w
	u32 literal_addr_low;
	u32 literal_addr_high : 16;
	u32 res4 : 16;
// 8w
	u32 literal_base_ub;
	u32 res5;
// 10w
	u32 literal_buff_len;
	u32 res6;
// 12w
	u32 pc_ctrl_low;
	u32 pc_ctrl_high : 16;
	u32 res7 : 16;
// 14w
	u32 l2_remap[16];
// 30w
	u32 l2_vir_base_low;
	u32 l2_vir_base_high : 16;
	u32 res8 : 16;
} devdrv_hwts_kernel_sqe_t;

typedef struct tag_devdrv_hwts_event_sqe {
// 0
	u32 type : 8;
	u32 ie : 1;
	u32 pre_p : 1;
	u32 post_p : 1;
	u32 wr_cqe : 1;
	u32 res0 : 4;
	u32 res1 : 16;

	u32 stream_id : 16;
	u32 task_id : 16;

	u32 event_id : 10;
	u32 res2 : 22;
// 3w
	u32 res3[29];
} devdrv_hwts_event_sqe_t;

typedef struct tag_devdrv_hwts_ph_sqe {
// 0
	u32 type : 8;
	u32 ie : 1;
	u32 pre_p : 1;
	u32 post_p : 1;
	u32 wr_cqe : 1;
	u32 res0 : 4;
	u32 res1 : 16;

	u32 stream_id : 16;
	u32 task_id : 16;
// 2w
	u32 res2[30];
} devdrv_hwts_ph_sqe_t;

int devdrv_format_hwts_sqe(void *hwts_sq_addr, void *ts_sq_addr, u64 ts_sq_len);


#endif