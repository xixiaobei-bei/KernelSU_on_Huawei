/*
 * vdm_hal_vp8.h
 *
 * This is vdec hal for vp8 export header file.
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

#ifndef __VDM_HAL_VP8_HERAER__
#define __VDM_HAL_VP8_HERAER__
#include "memory.h"
#include "vfmw_intf.h"
#include "public.h"

#define VP8_MSG_SLOT_RATIO                       4

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

SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 vp8hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif

