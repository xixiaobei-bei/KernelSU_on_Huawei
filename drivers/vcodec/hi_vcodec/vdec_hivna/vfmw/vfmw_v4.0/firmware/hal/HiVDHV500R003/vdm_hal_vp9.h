/*
 * vdm_hal_vp9.h
 *
 * This is VDMV300 hardware abstraction export header file.
 *
 * Copyright (c) 2015-2020 Huawei Technologies CO., Ltd.
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

#ifndef __VDM_HAL_VP9_H__
#define __VDM_HAL_VP9_H__
#include "memory.h"
#include "vfmw_intf.h"
#include "public.h"

#define VP9_MSG_SLOT_RATIO                           4

#ifdef MSG_POOL_ADDR_CHECK
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
#else
SINT32 vp9hal_startdec(omxvdh_reg_cfg_s *vdh_reg_cfg);
#endif
#endif
