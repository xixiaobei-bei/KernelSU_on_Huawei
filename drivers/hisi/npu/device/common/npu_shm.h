/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2018-2019. All rights reserved.
 * Description:
 * Author:
 * Create: 2018-12-22
 */
#ifndef __NPU_SHM_H
#define __NPU_SHM_H

#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/types.h>
#include <linux/dma-mapping.h>

#include "npu_common.h"
#include "drv_log.h"
#include "npu_platform.h"

extern struct devdrv_mem_desc *g_sq_desc;

typedef unsigned long long vir_addr_t;

struct devdrv_mem_info {
	phys_addr_t phy_addr;
	vir_addr_t virt_addr;
	size_t size;
};

enum {
	DEVDRV_SQ_MEM = 0,
	DEVDRV_INFO_MEM,
	DEVDRV_DOORBELL_MEM,
	DEVDRV_PERSISTENT_TASK_BUFF,
	DEVDRV_PAD_MEM,
	DEVDRV_MAX_MEM
};

struct devdrv_cm_msg {
	u32 share_num;
	u64 size;
};

#define MEM_INFO_VALID		(0xa5a5a5a5)
#define MEM_TOTAL_SIZE		(0x800000)
#define MEM_INIT_SIZE		(0x200000)
#define SHARE_NUM_OCCUPIED		(0)
#define L2_MAP_ALIGN 		2
// bit8~15 map_type, bit0~7 share num
// for npu drv mmap switch identify
#define MAP_COMBINE(type, share_num)   ((type << 8) | share_num)
#define MAP_GET_TYPE(map_info)                  ((map_info >> 8) & 0xff)
#define MAP_GET_SHARE_NUM(map_info)               (map_info & 0xff)

#define DATA_CEIL(data, size)         ((((data - 1) / size) + 1) * size)
#define DATA_FLOOR(data, size)         ((data / size) * size)

typedef enum {
	MAP_RESERVED = 0,
	MAP_L2_BUFF,
	MAP_CONTIGUOUS_MEM,
	MAP_INFO_SQ_CQ_MEM,	// map info desc(stream、sq、cq) sq channel and cq channel( delete doorbell mmap)
	MAP_MAX,
} devdrv_map_type_t;

int devdrv_shm_init(u8 dev_id);

struct devdrv_stream_info *devdrv_calc_stream_info(u8 devid, u32 index);

struct devdrv_ts_sq_info *devdrv_calc_sq_info(u8 devid, u32 index);

struct devdrv_ts_cq_info *devdrv_calc_cq_info(u8 devid, u32 index);

u32 *devdrv_get_ts_work_status(u8 devid, u32 index);

void devdrv_shm_destroy(u8 dev_id);

int devdrv_map_l2_buff(const struct file *filp, struct vm_area_struct *vma, u8 dev_id);

int devdrv_map_cm(const struct file *filp, struct vm_area_struct *vma,
		  u32 share_num, u8 dev_id);

int devdrv_info_sq_cq_mmap(u8 dev_id,
			   const struct file *filep,
			   struct vm_area_struct *vma);

int devdrv_save_cm_info(struct devdrv_dev_ctx *dev_ctx,
			struct devdrv_cm_info *info,
			u32 *share_num);

int devdrv_save_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx,
			u32 *share_num);

int devdrv_delete_cm_info(struct devdrv_dev_ctx *dev_ctx, u32 share_num);

int devdrv_delete_cm_info_occupied(struct devdrv_dev_ctx *dev_ctx);

int devdrv_is_cm_available(struct devdrv_dev_ctx *dev_ctx, u64 need_size);

int devdrv_inc_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size);

int devdrv_dec_cm_total_size(struct devdrv_dev_ctx *dev_ctx, u64 size);

int devdrv_is_cm_valid(struct devdrv_dev_ctx *dev_ctx, u32 share_num);

int devdrv_cm_resource_recycle(struct devdrv_dev_ctx *dev_ctx);

#endif