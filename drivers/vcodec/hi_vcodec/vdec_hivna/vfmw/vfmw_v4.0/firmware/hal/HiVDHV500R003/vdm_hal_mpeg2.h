/*
 * vdm_hal_mpeg2.h
 *
 * This is vdec hal for mp2 export header file.
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

#ifndef __VDM_HAL_MPEG2_H__
#define __VDM_HAL_MPEG2_H__

#include "basedef.h"
#include "memory.h"
#include "vfmw_intf.h"

#define SLOT_MSG_RATIO               4

#ifdef MSG_POOL_ADDR_CHECK
typedef struct {
	USIGN pmv_top_addr : 32;
} mp2dnmsg_d48;

typedef struct {
	USIGN first_slc_dnmsg_addr : 32;
} mp2dnmsg_d63;

SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 mp2hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif
