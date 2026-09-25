/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2019. All rights reserved.
 * Description: functions definition for memory
 * Author: yangyichang
 * Create: 2014-12-15
 */

#ifndef __VDEC_MEMORY_H__
#define __VDEC_MEMORY_H__

#include "hi_type.h"
#include "vfmw.h"

#define MEM_BUF_MAPPED     1
#define MEM_BUF_UNMAPPED   0


typedef enum {
	SCENE_VIDEO,
	SCENE_HEIF,
	SCENE_BUTT
} vdec_scene;

typedef struct {
	hi_u8      is_map_virtual;
	hi_u8      is_mapped;
	hi_u32     share_fd;
	UADDR      start_phy_addr;
	hi_u32     size;
	vdec_scene scene;
	hi_void    *start_vir_addr;
} mem_buffer_s;

hi_s32 vdec_mem_probe(void);
hi_s32 vdec_mem_get_map_info(hi_s32 share_fd, mem_buffer_s *ps_m_buf);
hi_s32 vdec_mem_put_map_info(mem_buffer_s *ps_m_buf);
hi_s32 vdec_mem_iommu_map(hi_s32 share_fd, UADDR *iova);
hi_s32 vdec_mem_iommu_unmap(hi_s32 share_fd, UADDR iova);

#endif

