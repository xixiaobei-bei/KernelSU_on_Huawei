/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for mp2 export header file.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#ifndef __VDM_HAL_MPEG2_H__
#define __VDM_HAL_MPEG2_H__

#include "basedef.h"
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
typedef struct {
	USIGN pmv_top_addr : 32;
} MP2DNMSG_D48;

typedef struct {
	USIGN first_slc_dnmsg_addr :                32;
} MP2DNMSG_D63;

SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map);
#else
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg);
#endif
#endif
