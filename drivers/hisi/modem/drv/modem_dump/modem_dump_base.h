
/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2012-2015. All rights reserved.
 * foss@huawei.com
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 * * This program is free software; you can redistribute it and/or modify
 * * it under the terms of the GNU General Public License version 2 and
 * * only version 2 as published by the Free Software Foundation.
 * *
 * * This program is distributed in the hope that it will be useful,
 * * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * * GNU General Public License for more details.
 * *
 * * You should have received a copy of the GNU General Public License
 * * along with this program; if not, write to the Free Software
 * * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 * * Redistribution and use in source and binary forms, with or without
 * * modification, are permitted provided that the following conditions
 * * are met:
 * * 1) Redistributions of source code must retain the above copyright
 * *    notice, this list of conditions and the following disclaimer.
 * * 2) Redistributions in binary form must reproduce the above copyright
 * *    notice, this list of conditions and the following disclaimer in the
 * *    documentation and/or other materials provided with the distribution.
 * * 3) Neither the name of Huawei nor the names of its contributors may
 * *    be used to endorse or promote products derived from this software
 * *    without specific prior written permission.
 *
 * * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef __MDM_DUMP_BASE_H__
#define __MDM_DUMP_BASE_H__

#include "drv_nv_def.h"
#include "bsp_dump.h"

#define OM_DUMP_PATH            "/modem_log/drv/onoff"
#define OM_RESET_LOG            "/modem_log/drv/onoff/reset.log"
#define OM_RESET_LOG_MAX        2048


#define DUMP_MODEMAP_BASE_INFO_SIZE                 (0x180)

#define DUMP_MODEMAP_USER_DATA_SIZE                 (0x1000)

#define DUMP_MODEMAP_TASK_INFO_SIZE                 0x200
#define DUMP_MODEMAP_TASK_INFO_STACK_SIZE           (0x2000)

#define PID_PPID_GET(taskTCB)  ((((struct task_struct *)taskTCB)->pid & 0xffff)| \
                                 ((((struct task_struct *)taskTCB)->real_parent->pid & 0xffff)<< 16))

#define DUMP_T_TASK_ERROR(mod_id)      (mod_id & (1<<24))
#define DUMP_LINUX_TASK_NUM_MAX         128
#define DUMP_REG_SET_MAX                4096
#define DUMP_INIT_WAIT_MAX               60        //60s
#define OM_SYSCTRL_MAGIC        0x5B5C5C5B


#define SRAM_MAX_CPUFREQ_PROFILE_ADDR  (uintptr_t)(&(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MAX_CPUFREQ_PROFILE))
#define SRAM_MIN_CPUFREQ_PROFILE_ADDR  (uintptr_t)(&(((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_MIN_CPUFREQ_PROFILE))
#define SRAM_DUMP_POWER_OFF_FLAG_ADDR  (uintptr_t)(&((SRAM_SMALL_SECTIONS*)(SRAM_BASE_ADDR + SRAM_OFFSET_SMALL_SECTIONS))->SRAM_DUMP_POWER_OFF_FLAG)



typedef struct __mdm_dump_field_addr
{
    void* base_info_addr;
    void* usr_data_addr;
    void* task_stack_addr;
    
}modemap_dump_fields_addr_s;



void save_last_profile(void);
s32 dump_save(char * log_path, u32 nve_value);
void dump_save_log_files(char * dir_name, DUMP_FILE_CFG_STRU * cfg, dump_product_type_t type);
void dump_save_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);
void dump_save_cp_base_info(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);
void dump_save_modem_sysctrl(void);
void dump_notify_target(u32 mod_id);
void dump_save_momdem_reset_info(u32 modid,char* name);
s32 dump_base_info_init(void);
u32 dump_match_rdr_mod_id(u32 drv_mod_id);
void dump_wait_for_reboot(void);
s32 dump_register_hook(void);
void dump_init_done(void);
int bsp_om_debug_init(void);
s32 dump_config_init(void);
void dump_product_type_init(void);
void dump_save_usr_data(char *data, u32 length);
void dump_save_and_reboot(void);
void dump_memmap_modem_ddr(void);
void dump_save_file(char * file_name, void * addr, u32 len);
void dump_show_stack(void);
int dump_append_file(char * dir, char *filename, void * address, u32 length, u32 max_size);
void modem_dump_field_init(void);
void dump_save_exc_task(void);
void* dump_get_rdr_exc_info(void);

s32 bsp_dump_save_task_init(void);

void modem_dump_field_init(void);


#endif

