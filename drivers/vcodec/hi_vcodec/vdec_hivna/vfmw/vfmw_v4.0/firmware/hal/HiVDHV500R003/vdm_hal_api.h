/*
 * vdm_hal_api.h
 *
 * This is vdm hal api export header file.
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

#ifndef _VDM_HAL_API_HEADER_
#define _VDM_HAL_API_HEADER_

#include "basedef.h"
#include "mem_manage.h"
#include "vfmw.h"
#include "vdm_hal_local.h"
#include "vfmw_intf.h"

void vdmhal_imp_reset_vdm(SINT32 vdh_id);
void vdmhal_imp_glb_reset(void);
void vdmhal_imp_clear_int_state(SINT32 vdh_id);
SINT32 vdmhal_imp_check_reg(reg_id_e reg_id, SINT32 vdh_id);
void vdmhal_imp_starthwrepair(SINT32 vdh_id);
void vdmhal_imp_starthwdecode(SINT32 vdh_id);
SINT32 vdmhal_imp_prepare_dec(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
SINT32 vdmhal_imp_prepare_repair(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
SINT32 vdmhal_imp_backupinfo(void);
void vdmhal_imp_getcharacter(void);
void vdmhal_imp_writescdemarid(void);
void vdmhal_imp_init(void);
void vdmhal_imp_deinit(void);

void vdmhal_isr(SINT32 vdh_id);
SINT32 vdmhal_hw_dec_proc(omxvdh_reg_cfg_s *vdh_reg_cfg, mem_buffer_s *vdh_mem_map);
void vdmhal_getregstate(vdmhal_backup_s *vdm_reg_state);
SINT32 vdmhal_is_vdm_run(SINT32 vdh_id);

SINT32 vdmhal_prepare_sleep(void);
void vdmhal_force_sleep(void);
void vdmhal_exit_sleep(void);
vdmdrv_sleep_stage_e vdmhal_get_sleep_stage(void);
void vdmhal_set_sleep_stage(vdmdrv_sleep_stage_e sleep_state);
#ifdef MSG_POOL_ADDR_CHECK
SINT32 vdmhal_imp_open_hal(mem_buffer_s *open_param);
SINT32 vdmhal_imp_open_heif_hal(mem_buffer_s *open_param);
#endif
#endif
