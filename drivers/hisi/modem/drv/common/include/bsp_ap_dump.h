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
#ifndef __AP_DUMP_H__
#define __AP_DUMP_H__
#include <linux/semaphore.h>
#include "bsp_softtimer.h"
#include <linux/wakelock.h>
#include "bsp_om_enum.h"
#include "mdrv_om_common.h"
#include <linux/kernel.h>

#define DUMP_INT_IN_FLAG                    0xAAAA
#define DUMP_INT_EXIT_FLAG                  0xBBBB
#define DUMP_INT_UNLOCK_FLAG                0xCCCC
#define DUMP_SAVE_SUCCESS                   0xA4A4A4A4

#define DUMP_KERNEL_TASK_INFO_SIZE                 0x200
#define DUMP_KERNEL_TASK_INFO_STACK_SIZE           (DUMP_KERNEL_TASK_INFO_SIZE - 32*4)

#define ARM_REGS_NUM                        17

/**************************************************************************
  STRUCT定义
**************************************************************************/

typedef enum _dump_arm_vec_e
{
    DUMP_ARM_VEC_RESET          = 0x0,
    DUMP_ARM_VEC_UNDEF          = 0x4,
    DUMP_ARM_VEC_SWI            = 0x8,
    DUMP_ARM_VEC_PREFETCH       = 0xc,
    DUMP_ARM_VEC_DATA           = 0x10,
    DUMP_ARM_VEC_IRQ            = 0x18,
    DUMP_ARM_VEC_FIQ            = 0x1c,
    DUMP_ARM_VEC_UNKNOW         = 0xff,
}dump_arm_vec_t;

typedef struct
{
    u32 pid;
    u32 entry;
    u32 status;
    u32 policy;
    u32 priority;
    u32 stack_base;
    u32 stack_end;
    u32 stack_high;
    u32 stack_current;
    u8  name[16];
    u32 regs[17];
    u32 offset;
    u32 rsv[1];
    char dump_stack[DUMP_KERNEL_TASK_INFO_STACK_SIZE];
} ap_dump_task_info_s;


typedef struct _ap_dump_flag
{
    u32            stack_trace_flag;
}ap_dump_flag_s;

typedef struct __ap_dump_core_ctrl
{
    u32            init_flag ;
    volatile bool  exc_flag ;
}ap_dump_core_ctrl_s;

typedef struct __dump_ctrl_s
{
    u32 dump_task_id;
    u32 dump_task_job;
    u32 current_task_info;           /*0xA0 */
    struct semaphore sem_dump_task;
}dump_ctrl_s;






#define RDR_AP_DUMP_ARM_RESET_MOD_ID             (0x80000000)
#define RDR_AP_DUMP_ARM_UNDEF_MOD_ID             (0x80000001)
#define RDR_AP_DUMP_ARM_SWI_MOD_ID               (0x80000002)
#define RDR_AP_DUMP_ARM_PREFETCH_MOD_ID          (0x80000003)
#define RDR_AP_DUMP_ARM_DATA_MOD_ID              (0x80000004)
#define RDR_AP_DUMP_ARM_IRQ_MOD_ID               (0x80000005)
#define RDR_AP_DUMP_ARM_FIQ_MOD_ID               (0x80000006)
#define RDR_AP_DUMP_ARM_UNKNOW_MOD_ID            (0x80000007)
#define RDR_AP_DUMP_NORMAL_EXC_MOD_ID            (0x80000008)
#define RDR_AP_DUMP_PANIC_IN_INT_MOD_ID          (0x80000009)
#define RDR_AP_DUMP_AP_WDT_MOD_ID                (0x80000010)
#define DUMP_INIT_WAIT_MAX   60    //60s


/**************************************************************************
  宏定义
**************************************************************************/

#define DUMP_INIT_FLAG_PHASE1               (0x5B5B0000)
#define DUMP_INIT_FLAG_PHASE2               (0x5B5B0001)
#define DUMP_INIT_FLAG                      (0x5B5B0002)
#define DUMP_INIT_FLAG_WAIT                 (0x5C5C5C5C)

#define DUMP_TASK_JOB_RESET_LOG             (0x00000001)
#define DUMP_TASK_JOB_SAVE                  (0x00000002)
#define DUMP_TASK_JOB_INIT                  (0x00000004)
#define DUMP_TASK_JOB_REBOOT                (0x00000008)
#define DUMP_TASK_JOB_SAVE_INIT             (DUMP_TASK_JOB_SAVE | DUMP_TASK_JOB_INIT)
#define DUMP_TASK_JOB_SAVE_REBOOT           (DUMP_TASK_JOB_SAVE | DUMP_TASK_JOB_REBOOT)



#define PID_PPID_GET(taskTCB)  ((((struct task_struct *)taskTCB)->pid & 0xffff)| \
                                 ((((struct task_struct *)taskTCB)->real_parent->pid & 0xffff)<< 16))

#define DUMP_T_TASK_ERROR(mod_id)      (mod_id & (1<<24))
#define DUMP_LINUX_TASK_NUM_MAX         128
#define DUMP_REG_SET_MAX                4096


void ap_system_error(u32 mod_id, u32 arg1, u32 arg2, char *data, u32 length);

typedef enum _ap_dump_reboot_reason_e
{
    AP_DUMP_REASON_NORMAL      = 0x0,
    AP_DUMP_REASON_ARM         = 0x1,
    AP_DUMP_REASON_STACKFLOW   = 0x2,
    AP_DUMP_REASON_UNDEF       = 0xff
}ap_dump_reboot_reason_e;


#define ap_dump_fetal(fmt,...)        printk(KERN_CRIT"[AP_DUMP]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define ap_dump_warning(fmt,...)      printk(KERN_WARNING"[AP_DUMP]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)
#define ap_dump_error(fmt,...)        printk(KERN_ERR"[AP_DUMP]: <%s> line = %d  "fmt, __FUNCTION__,__LINE__, ##__VA_ARGS__)

void ap_dump_set_exc_vec(u32 vec);
void ap_dump_int_enter(u32 dir, u32 irq_num);
void ap_dump_int_exit(u32 dir, u32 irq_num);
int ap_dump_stack_trace_print(unsigned long where);
void ap_dump_show_stack(void);
void ap_dump_save_exc_task(u32 addr);
void ap_dump_trace_stop(void);
dump_handle bsp_ap_dump_register_hook(char * name, dump_hook func);
u8 * bsp_ap_dump_register_field(u32 field_id, char * name, void * virt_addr, void * phy_addr, u32 length, u16 version);
s32 bsp_ap_dump_unregister_hook(dump_handle handle);
void bsp_ap_dump_trace_stop(void);
void bsp_ap_dump_save_all_task_info(u32 mod_id);
u8 * bsp_ap_dump_get_field_phy_addr(u32 field_id);

#endif
