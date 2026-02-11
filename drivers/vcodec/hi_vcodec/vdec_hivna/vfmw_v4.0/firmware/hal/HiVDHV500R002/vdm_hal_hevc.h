/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2019. All rights reserved.
 * Description: vdec hal for hevc export header file.
 * Author: gaoyajun<gaoyajun@hisilicon.com>
 * Create: 2017-11-26
 */

#ifndef __VDM_HAL__HEVC_H__
#define __VDM_HAL__HEVC_H__

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw_intf.h"

#ifdef MSG_POOL_ADDR_CHECK
SINT32 hevchal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg, mem_buffer_s *p_vdh_mem_map);
#else
SINT32 hevchal_startdec(omxvdh_reg_cfg_s *p_vdh_reg_cfg);
#endif
#endif
