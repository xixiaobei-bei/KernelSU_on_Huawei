/*
 * devdrv_user_common_model.h
 *
 * about devdrv user model
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
#ifndef __DEVDRV_USER_COMMON_MODEL_H
#define __DEVDRV_USER_COMMON_MODEL_H

#include <linux/types.h>

#define DEVDRV_HWTS_SQ_SLOT_SIZE   128
#define DEVDRV_HWTS_CQ_SLOT_SIZE   16
#define DEVDRV_MAX_HWTS_SQ_DEPTH   1024
#define DEVDRV_MAX_HWTS_CQ_DEPTH   1024
#define DEVDRV_MAX_HWTS_SQ_NUM      24
#define DEVDRV_MAX_HWTS_CQ_NUM      20

#define DEVDRV_MAX_MODEL_INFO_NUM   (DEVDRV_MAX_MODEL_ID)
#define DEVDRV_MAX_STREAM_INFO_NUM   (DEVDRV_MAX_NON_SINK_STREAM_ID)
#define DEVDRV_MAX_SINK_STREAM_INFO_NUM   (DEVDRV_MAX_SINK_STREAM_ID)

#define DEVDRV_MAX_SQ_INFO_NUM   DEVDRV_MAX_SQ_NUM
#define DEVDRV_MAX_CQ_INFO_NUM   DEVDRV_MAX_CQ_NUM
#define DEVDRV_MAX_HWTS_SQ_INFO_NUM     DEVDRV_MAX_HWTS_SQ_NUM
#define DEVDRV_MAX_HWTS_CQ_INFO_NUM  DEVDRV_MAX_HWTS_CQ_NUM

struct devdrv_hwts_sq_info {
	u32 head;      // head sqe
	u32 tail;      // tail sqe
	u32 credit;    // resv num
	u32 index;     // sq index
	s32 uio_num;
	s32 uio_fd;
	s32 uio_map;
	s32 uio_size;
	u64 uio_addr;
	u32 stream_num;
	u32 rsd;
	u64 send_count;
	u64 hwts_sq_sub;
	u64 phy_addr;
};

struct devdrv_hwts_cq_info {
	u32 head;
	u32 tail;
	volatile u32 count_report;
	u32 index;
	u32 phase;
	u32 int_flag;
	s32 uio_num;
	s32 uio_fd;
	s32 uio_map;
	s32 uio_size;
	u64 uio_addr;
	u64 receive_count;
	u64 hwts_cq_sub;
	u64 phy_addr;
	u32 stream_num;
	u8 slot_size;
	u8 rsd[3];
};

#define MODEL_STREAM_MAX_NUM  32
#define MODEL_EVENT_MAX_NUM   128
typedef struct devdrv_model_desc_info {
	uint16_t stream_id[MODEL_STREAM_MAX_NUM];
	uint8_t  event_info[MODEL_EVENT_MAX_NUM];
	uint32_t model_id;
	uint32_t stream_cnt;
	uint8_t  reserve0[56];    // 64 bytes cache line align
	uint64_t to_schedule_times;
	uint32_t report_num;
	uint32_t appraisement;
	uint32_t in_schedule;
	uint32_t result;
	uint8_t  reserve1[40];
} devdrv_model_desc_info_t;

#define DEVDRV_HWTS_SQ_INFO_SIZE	sizeof(struct devdrv_hwts_sq_info)    // 64
#define DEVDRV_HWTS_CQ_INFO_SIZE	sizeof(struct devdrv_hwts_cq_info)    // 16
#define DEVDRV_MODEL_INFO_SIZE	sizeof(struct devdrv_model_desc_info)    // 1536

#define DEVDRV_MODEL_INFO_OCCUPY_SIZE   \
	(DEVDRV_MODEL_INFO_SIZE * DEVDRV_MAX_MODEL_INFO_NUM)
#define DEVDRV_HWTS_SQ_INFO_OCCUPY_SIZE \
	(DEVDRV_HWTS_SQ_INFO_SIZE * DEVDRV_MAX_HWTS_SQ_NUM)
#define DEVDRV_HWTS_CQ_INFO_OCCUPY_SIZE \
	(DEVDRV_HWTS_CQ_INFO_SIZE * DEVDRV_MAX_HWTS_CQ_NUM)
#define DEVDRV_HWTS_SQ_INFO_OFFSET (DEVDRV_SQ_INFO_OCCUPY_SIZE + DEVDRV_CQ_INFO_OCCUPY_SIZE + \
	DEVDRV_STREAM_INFO_OCCUPY_SIZE)
#define DEVDRV_HWTS_CQ_INFO_OFFSET (DEVDRV_HWTS_SQ_INFO_OFFSET + DEVDRV_HWTS_SQ_INFO_OCCUPY_SIZE)
#define DEVDRV_MODEL_INFO_OFFSET (DEVDRV_HWTS_CQ_INFO_OFFSET + DEVDRV_HWTS_CQ_INFO_OCCUPY_SIZE)
#define DEVDRV_SQ_OCCUPY_SIZE \
	(DEVDRV_MAX_SQ_NUM * DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE)
#define DEVDRV_CQ_OCCUPY_SIZE \
	(DEVDRV_MAX_CQ_NUM * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE)
#define DEVDRV_HWTS_CQ_OCCUPY_SIZE \
	(DEVDRV_MAX_HWTS_SQ_NUM * DEVDRV_MAX_HWTS_SQ_DEPTH * DEVDRV_HWTS_SQ_SLOT_SIZE)
#define DEVDRV_DFX_SQ_OCCUPY_SIZE    \
	(DEVDRV_MAX_DFX_SQ_NUM * DEVDRV_MAX_SQ_DEPTH * DEVDRV_SQ_SLOT_SIZE)
#define DEVDRV_DFX_CQ_OCCUPY_SIZE    \
	(DEVDRV_MAX_DFX_CQ_NUM * DEVDRV_MAX_CQ_DEPTH * DEVDRV_CQ_SLOT_SIZE)
#define DEVDRV_DFX_SQ_INFO_OCCUPY_SIZE    \
	(DEVDRV_MAX_DFX_SQ_NUM * DEVDRV_SQ_INFO_SIZE)
#define DEVDRV_DFX_CQ_INFO_OCCUPY_SIZE    \
	(DEVDRV_MAX_DFX_CQ_NUM * DEVDRV_CQ_INFO_SIZE)
#define DEVDRV_DFX_OCCUPY_SIZE    \
	((DEVDRV_DFX_SQ_OCCUPY_SIZE) + (DEVDRV_DFX_CQ_OCCUPY_SIZE) + \
		(DEVDRV_DFX_SQ_INFO_OCCUPY_SIZE) + (DEVDRV_DFX_CQ_INFO_OCCUPY_SIZE))

#define DEVDRV_TS_LOG_OCCUPY_SIZE 0x80000  // 512k
#define DEVDRV_BLACKBOX_OCCUPY_SIZE 0x40000 // 256k

#endif
