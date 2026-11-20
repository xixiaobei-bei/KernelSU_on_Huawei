/*
 * drv_cmdlist.h
 *
 * This is for Operations Related to cmdlist.
 *
 * Copyright (c) 2019-2020 Huawei Technologies CO., Ltd.
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

#ifndef __DRV_CMDLIST_H__
#define __DRV_CMDLIST_H__

#include "drv_common.h"

enum cmdlist_type {
	CMDLIST_NORMAL = 0,
	CMDLIST_PROTECT,
	CMDLIST_BUTT
};

/* The start physical address, iova address, and size of the mmap memory must be 4 KB aligned. */
#define MIN_PAGE_ALIGN_SIZE PAGE_SIZE
#define CMDLIST_BUFFER_SIZE (2 * MIN_PAGE_ALIGN_SIZE)
#define CMDLIST_ALIGN_SIZE 128
#define CMDLIST_RESERVE_MEM_SIZE 256
#define CMDLIST_MAX_HEAD_NUM (CMDLIST_BUTT + 1)

/* Records the current location information. */
struct cmdlist_cfg {
	uint32_t* base_addr;
	uint64_t iova_addr;
	/* Offset of the configuration area relative to the start memory.The span is 4 bytes. */
	uint32_t cfg_offset;
	uint32_t cfg_len0;
	uint32_t cfg_len1;
};

/* The cmdlist memory is divided into three parts:
 1.software usage area
 2.cmdlist data dump area (The length of each frame is the same)
 3.cmdlist configuration area (The length of each frame is different)
   3.1.len0 (Before the venc eop, run the following command)
   3.2.len1 (After the venc eop, run the following command)
*/
struct cmdlist_node {
	struct list_head list;

	struct channel_info channel_info;
	struct venc_fifo_buffer *buffer;

	void *virt_addr; /* cmdlist data dump area virtual address */
	uint64_t iova_addr;   /* cmdlist data dump area iova address */

	uint32_t cfg_len;     /* cmdlist configuration area length */

	/* Record the virtual address of the memory
	  where the address and length of the cmdlist configuration area of the previous frame are located.
	  After the current frame is configured, update the previous frame. */
	uint32_t *pre_cmdlist_addr;
	uint32_t *pre_len_addr;
};

struct cmdlist_head {
	struct list_head list;
	struct cmdlist_cfg cfg;
	uint64_t start_time;
	uint32_t num;
};

struct cmdlist_manager {
	struct cmdlist_head head[CMDLIST_MAX_HEAD_NUM];
	uint32_t cur_idx;
	uint32_t wait_idx[CMDLIST_BUTT];
	struct mutex lock; /* Ensure that the list cannot be modified. */
	volatile unsigned long bit_map; /* record used head */
};

void cmdlist_init_ops(void);

#endif
