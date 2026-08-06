/*
 * vfmw_intf.h
 *
 * This is for vfmw interface.
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

#ifndef __VFMW_INTF_H__
#define __VFMW_INTF_H__
#include "memory.h"
#include "vdm_drv.h"
#include "scd_drv.h"
#include "platform.h"
#include "public.h"

#define VCTRL_OK                0
#define VCTRL_ERR              (-1)
#define MSG_POOL_ADDR_CHECK
#define VFMW_OSAL_SLEEP_TIME    10
#define VFMW_OSAL_SLEEP_COUNT   30


typedef struct hi_drv_mem_s {
	mem_record_s st_vdh_reg;
} drv_mem_s;

typedef struct {
	mem_buffer_s scd[SCD_SHAREFD_MAX];
	mem_buffer_s vdh[VDH_SHAREFD_MAX];
	struct file  *file;
	clk_rate_e   base_rate;
	hi_u64 load;
} vdec_mem_info;

#ifdef MSG_POOL_ADDR_CHECK
SINT32 check_frm_buf_addr(UADDR  src_pmv_addr, mem_buffer_s *vdh_mem_map);
SINT32 check_pmv_buf_addr(UADDR  src_pmv_addr, mem_buffer_s *vdh_mem_map);
SINT32 vctrl_vdh_unmap_message_pool(mem_buffer_s *mem_map);
#endif

SINT32 vctrl_open_drivers(void);

SINT32 vctrl_open_vfmw(void);

SINT32 vctrl_close_vfmw(void);

SINT32 vctrl_vdm_hal_process(
	omxvdh_reg_cfg_s *vdm_reg_cfg, vdmhal_backup_s *vdm_reg_statei,
	mem_buffer_s *vdh_mem_map);

SINT32 vctrl_scd_hal_process(
	omx_scd_reg_cfg_s *scd_reg_cfg, scd_state_reg_s *scd_state_reg,
	mem_buffer_s *scd_mem_map);

SINT32 vctrl_vdm_hal_is_run(void);

void vctrl_suspend(void);

void vctrl_resume(void);

hi_bool vctrl_scen_ident(hi_u32 cmd);

#endif
