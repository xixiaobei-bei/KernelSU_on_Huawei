/*
 * vdm_hal_mpeg4.h
 *
 * This is vdec hal for mp4 export header file.
 *
 * Copyright (c) 2017-2020 Huawei Technologies CO., Ltd.
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

#ifndef _VDM_HAL_MPEG4_HEADER_
#define _VDM_HAL_MPEG4_HEADER_

#include "basedef.h"
#include "mem_manage.h"
#include "memory.h"
#include "vfmw_intf.h"

#define MPEG4_MSG_SLOT_RATIO     4

#ifdef MSG_POOL_ADDR_CHECK
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 mp4hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif
