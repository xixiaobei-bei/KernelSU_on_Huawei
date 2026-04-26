/*
 * npu_rt_task.h
 *
 * about npu task type
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
#ifndef _NPU_RT_TASK_H_
#define _NPU_RT_TASK_H_

#include <linux/types.h>

#define NPU_MAX_LABEL_ID	      1024
/**
 * @ingroup devdrv
 * @brief the type defination of task
 */
typedef enum npu_task_type {
	NPU_TASK_KERNEL_AICORE = 0,	    /* AI core task */
	NPU_TASK_KERNEL_AICPU = 1,	   /* AI cpu task */
	NPU_TASK_EVENT_RECORD = 2,	   /* event record task */
	NPU_TASK_STREAM_WAIT_EVENT = 3,	  /* stream wait event task */
	NPU_TASK_FUSION_ISSUE = 4,	     /* fusion issue task */
	NPU_TASK_MEMCPY = 5,	          /* memory copy task */
	NPU_TASK_MAINTENANCE = 6,   /* such as destroy the event or stream */
	NPU_TASK_CREATE_STREAM = 7, /* create stream task */
	NPU_TASK_REMOTE_EVENT_WAIT = 9, /* wait for event on another device */
	NPU_TASK_PCTRACE_ENABLE = 10,
	NPU_TASK_CREATE_L2_ADDR = 11, /* create L2 addr info for aicpu kernel */
	NPU_TASK_MODEL_MAINTAINCE = 12,
	NPU_TASK_MODEL_EXECUTE = 13,
	NPU_TASK_RDMA_SEND = 16,  /* hccl rdma send task */
	NPU_TASK_L2_SDMA_TASK_MEMCPY = 17, /* test l2 task memory copy task */
	NPU_TASK_STREAM_SWITCH = 18,
	NPU_TASK_STREAM_ACTIVE = 19,
	NPU_TASK_LABEL_SET = 20, /* set label for control flow ops */
	NPU_TASK_LABEL_SWITCH = 21, /* switch label for control flow ops */
	NPU_TASK_LABEL_GOTO = 22,   /* goto label for control flow ops */
	NPU_TASK_PROFILING_ENABLE = 0x40,
	NPU_TASK_PROFILING_DISABLE = 0x41,
	NPU_TASK_RESERVED = 0x42,
} npu_task_type_t;

typedef enum npu_task_state {
	NPU_TASK_STATE_INIT = 0,
	NPU_TASK_STATE_WAIT = 1,
	NPU_TASK_STATE_RUN = 2,
	NPU_TASK_STATE_COMPLETE = 3,
	NPU_TASK_STATE_PENDING = 4,
	NPU_TASK_STATE_SDMA_PROCESS_FAILED = 5,
	NPU_TASK_STATE_SDMA_PROCESS_SUCCESS = 6,
} npu_task_state_t;

typedef enum npu_report_type {
	NPU_TS_REPORT_TYPE_TASK = 0,                 /* task command report */
	NPU_TS_REPORT_TYPE_PROFILE_TIMELINE = 1,     /* timeline profile data */
	NPU_TS_REPORT_TYPE_PROFILE_TASKBASED = 2,    /* task based profile data */
	NPU_TS_REPORT_TYPE_RECYCLE_SQ_FINISHED = 3,  /* recycle sq report */
	NPU_TS_REPORT_TYPE_RESERVED,
} npu_report_type_t;

enum npu_fusion_flag {
	NPU_FUSION_BEGIN = 0,
	NPU_FUSION_END = 1,
};

enum npu_mmt_type {
	NPU_MT_STREAM_DESTROY = 0,
	NPU_MT_EVENT_DESTROY,
};

typedef enum npu_mmt_operation_type {
	NPU_MMT_STREAM_BIND = 0,       /* model stream bind */
	NPU_MMT_STREAM_UNBIND = 1,     /* model stream unbind */
	NPU_MMT_MODEL_CREATE = 2,      /* model create by task pool */
	NPU_MMT_MODEL_DESTROY = 3,     /* model destroy */
	NPU_MMT_MODEL_PRE_PROC = 4,
	NPU_MMT_RESERVED
} npu_mmt_operation_type_t;

typedef struct npu_rt_event_task {
	u16 event_id;
	u8 reserved[46];
} npu_rt_event_task_t;

typedef struct npu_rt_fusion_task {
	u16 flag;
	u8 reserved[46];
} npu_rt_fusion_task_t;

typedef struct npu_rt_maintenance_task {
	u16 goal;                      /* destory 0:stream, 1:event; */
	u16 target_id;
	u8 reserved[44];
} npu_rt_maintenance_task_t;

typedef struct npu_rt_model_maintenance_task {
	u16 model_id;
	u16 stream_id;
	u16 operation_type;
	u16 stream_type;
	u16 first_task_id;
	u8 reserved[38];
} npu_rt_model_maintenance_task_t;

typedef struct npu_rt_model_execute_task {
	u16 model_id;
	u16 first_task_id;
	u8 res0[4];
	u64 asid_baddr;
	u64 tcr;
	u16 asid;
	u16 smmu_svm_ssid; /* sub_stream_id */
	u32 pid;
	u32 execute_pid;
	u32 sequence;
	u8 priority;
	u8 reserved[7];
} npu_rt_model_execute_task_t;

typedef struct npu_rt_create_stream_task {
	u64 pid;
	u64 l2_base_vaddr;
	u64 asid_baddr;
	u64 tcr;
	u32 thread_id;
	u16 asid;
	u16 smmu_svm_ssid; /* sub_stream_id */
	u16 sq_id;
	u16 model_id;
	u8 priority;
	u8 reserved[3];
} npu_rt_create_stream_task_t;

typedef struct npu_rt_memcpy_task {
	u64 src_addr;
	u64 dst_addr;
	u64 length;
	u16 memcpy_type;
	u8 dir;
	u8 reserved[21];
} npu_rt_memcpy_task_t;

typedef struct npu_rt_kernel_task {
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
	u8 rd_cond;
	u8 reserved;
} npu_rt_kernel_task_t;

typedef struct npu_rt_label_switch_task {
	u64 left_addr;
	u64 right;
	u16 true_label_idx;
	u16 true_label_id;
	u8 condition;
	u8 reserved[27];
} npu_rt_label_switch_task_t;

typedef struct npu_rt_label_goto_task {
	u16 label_idx;
	u16 label_id;
	u8 reserved[44];
} npu_rt_label_goto_task_t;

typedef struct npu_rt_notify_task {
	u16 notify_id;
	u8 reserved[46];
} npu_rt_notify_task_t;

/**
 * @brief Runtime Task 64B
 */
typedef struct npu_rt_task {
	/* 16 bytes */
	u16 stream_id;
	u16 task_id;
	u16 next_task_id;
	u16 type;
	u16 next_stream_id;
	u16 task_state;
	u8 task_prof_en;
	u8 reserved[3];
	/* 48 bytes */
	union {
		npu_rt_kernel_task_t kernel_task;  //type 0,1
		npu_rt_event_task_t event_task; // type 2,3
		npu_rt_fusion_task_t fusion_task; // type 4
		npu_rt_memcpy_task_t memcpy_task;  // type 5
		npu_rt_maintenance_task_t maintenance_task; // type 6
		npu_rt_create_stream_task_t create_stream_task; // type 7
		npu_rt_model_maintenance_task_t model_maintenance_task; // type 12
		npu_rt_model_execute_task_t model_execute_task; // type 13
		npu_rt_label_switch_task_t label_switch_task;  //type 21
		npu_rt_label_goto_task_t label_goto_task;  // type 22
		npu_rt_notify_task_t notify_task;
	} u;
} npu_rt_task_t;

#endif
