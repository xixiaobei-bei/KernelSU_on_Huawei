/*
 * npu_shm.h
 *
 * about npu shm
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
#ifndef __NPU_SHM_H
#define __NPU_SHM_H

#include <linux/types.h>
#include <linux/mm.h>

#include "npu_common.h"
#include "npu_log.h"
#include "npu_platform.h"
#include "npu_proc_ctx.h"

extern struct npu_mem_desc *g_sq_desc;
#ifndef array_size
#define array_size(n) (sizeof(n) / sizeof((n)[0]))
#endif

struct npu_mem_info {
	phys_addr_t phy_addr;
	vir_addr_t virt_addr;
	size_t size;
};

enum {
	/* NPU_SQ_MEM : v200+other:SQ/CQ/DFX_SQ/DFX_CQ
	 * v100:16*1024*1(cq) + 64*256*64(sq) + 4*256*64(dfx sq) + 10*1024*16(dfx cq) =1264k used
	 * v200:16*1024*20(cq) + 80*256*64(sq) + 4*256*64(dfx sq) + 10*1024*16(dfx cq) = 1824k used
	 */
	NPU_SQ_MEM = 0,
	/* v200:sq_info/cq_info/stream_info/sink_stream_info/hwts_sq_info/hwts_cq_info/model_info/ts_status
	 * other:sq_info/cq_info/stream_info/ts_status
	 */
	NPU_INFO_MEM,
	NPU_DOORBELL_MEM,
	NPU_PERSISTENT_TASK_BUFF,
	NPU_PAD_MEM, // l2
	NPU_MAX_MEM
};

// bit8~15 map_type, bit0~7 share num
// for npu drv mmap switch identify
#define map_get_type(map_info)     (((map_info) >> 8) & 0xff)
#define data_ceil(data, size)    (((((data) - 1) / (size)) + 1) * (size))
#define data_floor(data, size)    (((data) / (size)) * (size))

typedef enum {
	MAP_RESERVED = 0,
	MAP_L2_BUFF,
	MAP_CONTIGUOUS_MEM,
	MAP_MAX,
} npu_map_type_t;

struct npu_vma_mmapping {
	struct list_head list;
	npu_map_type_t map_type;
	struct vm_area_struct *vma;
	struct npu_proc_ctx *proc_ctx;
	u32 map_count;
};

int npu_shm_init(u8 dev_id);

struct npu_stream_info *npu_calc_stream_info(u8 devid, u32 index);

struct npu_ts_sq_info *npu_calc_sq_info(u8 devid, u32 index);

struct npu_ts_cq_info *npu_calc_cq_info(u8 devid, u32 index);

struct npu_prof_info *npu_calc_profiling_info(u8 devid);

u32 *npu_get_ts_work_status(u8 devid);

void npu_shm_destroy(u8 dev_id);

int npu_map_l2_buff(const struct file *filp, struct vm_area_struct *vma,
	u8 dev_id);

int npu_devmem_swapin(struct vm_area_struct *vma, unsigned long devmem_base,
	unsigned long size, unsigned long align_size);

int npu_devmem_swapout(struct vm_area_struct *vma, unsigned long pad_base,
	unsigned long size, unsigned long align_size);

int l2_mem_swapin(struct vm_area_struct *vma);

int l2_mem_swapout(struct vm_area_struct *vma, u8 dev_id);

struct npu_hwts_sq_info *npu_calc_hwts_sq_info(u8 devid, u32 index);

struct npu_hwts_cq_info *npu_calc_hwts_cq_info(u8 devid, u32 index);

struct npu_model_desc_info *npu_calc_model_desc_info(u8 devid, u32 index);

int npu_shm_v200_init(u8 dev_id);

void npu_shm_v200_destroy(u8 dev_id);

#endif
