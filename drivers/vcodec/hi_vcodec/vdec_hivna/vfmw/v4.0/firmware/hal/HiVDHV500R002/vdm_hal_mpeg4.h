/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for mp4 export header file.
 * Author: gaoyajun
 * Create: 2017-11-26
 */

#ifndef _VDM_HAL_MPEG4_HEADER_
#define _VDM_HAL_MPEG4_HEADER_

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw_intf.h"

#define MPEG4_MSG_SLOT_RATIO        4

#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif
