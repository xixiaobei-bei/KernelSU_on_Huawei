/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for vp8 export header file.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#ifndef __VDM_HAL_VP8_HERAER__
#define __VDM_HAL_VP8_HERAER__
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
typedef struct {
	USIGN sed_top_addr :                         32;
} vp8_picdnmsg_d36;

typedef struct {
	USIGN pmv_top_addr :                         32;
} vp8_picdnmsg_d37;

typedef struct {
	USIGN rcn_top_addr :                         32;
} vp8_picdnmsg_d38;

typedef struct {
	USIGN tab_addr :                             32;
} vp8_picdnmsg_d39;
typedef struct {
	USIGN dblk_top_addr :                        32;
} vp8_picdnmsg_d40;

SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map);
#else
SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg);
#endif
#endif

