/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2017-2018. All rights reserved.
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

#ifndef _BSP_ONOFF_H_
#define _BSP_ONOFF_H_

#include <osl_common.h>
#include <osl_sem.h>
#include "bsp_softtimer.h"
#include "mdrv_sysboot.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Attention
 *
 * Description : Driver for onoff
 * Core        : Acore、Ccore、Mcore
 * Header File : the following head files need to be modified at the same time
 * : /acore/kernel/drivers/hisi/modem/drv/include/bsp_onoff.h
 * : /ccore/include/ccpu/bsp_onoff.h
 * : /ccore/include/fusion/bsp_onoff.h
 */
#define EXCH_ONOFF_LOG_MAX (0x400 * 16)
#define pr_dbg printk

#define LOCAL_1 static

typedef enum {
    POWER_REBOOT_CMD_WARMRESET = 0x52454348,
    POWER_REBOOT_CMD_BOOTLOADER = 0x5245424F,
    POWER_REBOOT_CMD_RECOVERY = 0x52455245,
    POWER_REBOOT_CMD_FACTORY = 0x52455D51,
    POWER_REBOOT_CMD_UPDATE = 0x52455344,
    POWER_REBOOT_CMD_BUTT = 0xFFFFFFFF
} power_reboot_cmd_e;

typedef enum {
    POWER_OFF_REASON_NORMAL = 0x504F535E,
    POWER_OFF_REASON_BAD_BATTERY = 0x504F4242,
    POWER_OFF_REASON_LOW_BATTERY = 0x504F424C,
    POWER_OFF_REASON_OVER_TEMP = 0x504F4F54,
    POWER_OFF_REASON_RM_CHARGE = 0x504F4235,
    POWER_OFF_REASON_UPDATE = 0x504F4F55,
    POWER_OFF_REASON_RESET = 0x504F4D56,
    POWER_OFF_REASON_BY_CHARGE = 0x5046424C, /* reset and entry power off charging */
    POWER_OFF_REASON_INVALID = 0xFFFFFFFF
} power_off_reboot_flag_e;

/* power on off info saved in axi memory */
typedef struct {
    unsigned int last_shut_reason;
    unsigned int power_on_reason;
    unsigned int wdg_rst_cnt;
    unsigned int reboot_cmd;
    unsigned int dload_complt_magic;
    unsigned int a_power_state;
} power_info_s;

typedef enum {
    POWER_ON_PROP_STATUS = 0,
    POWER_ON_PROP_REASON,
    POWER_ON_PROP_TIME
} power_sysinfo_e;

typedef enum {
    POWER_NO_START = 0,
    POWER_OS_OK,
    POWER_BSP_OK,
    POWER_MSP_OK,
    POWER_DSP_OK, /* 升级阶段保证C核MSP启动OK(AXI值大于等于POWER_MSP_OK),才认为升级成功 */
    POWER_PS_OK,
    POWER_ST_BUILT
} c_power_st_e;

typedef enum {
    POWER_NORMAL = 0x50127895,
    POWER_RECOVERY_A = 0x504F4F33,
    POWER_RECOVERY_B = 0x504F4E54
} a_kernel_st_e; /* hiboot启动模式状态机，只在hiboot中根据启动状态设置 */

/* 操作类型 */
typedef enum {
    E_POWER_ON_MODE_GET = 0,
    E_MODE_NRCPU_DRV_READY, /* 5G底软初始化完成标记 */
    E_MODE_LRCPU_DRV_READY, /* 4G底软初始化完成标记 */
    E_MODE_NRCPU_READY,     /* 5G启动标记 */
    E_MODE_LRCPU_READY,     /* 4G启动标记 */
    E_MODE_SET_MODEM_STATE, /* 设置MODEM状态 */
    E_POWER_SHUT_DOWN,      /* 关机，带关机原因 */
    E_POWER_POWER_OFF,      /* 下电 */
    E_POWER_POWER_REBOOT,   /* 重启 */
    E_POWER_BUILT
} power_ctrl_type_e;

typedef struct {
    u32 modem_id;
    u32 state; // 取值参考drv_modem_state
} modem_state_info_s;

typedef struct {
    u32 pwr_type; // 取值参考power_ctrl_type_e
    u32 mode;     // 取值参考drv_start_mode_e
    u32 reason;   // 取值参考drv_shutdown_reason_e
    modem_state_info_s modem_state_info;
} tag_ctrl_msg_s;

typedef struct _tagPowCtrl {
    unsigned int is_init; /* 1:have initialized */
    osl_sem_id pwr_sem;
    tag_ctrl_msg_s curr;
} tag_power_ctrl_s;

/*
 * Attention END
 */
typedef enum {
    NORMAL_VERTYPE_FLAG = 0, /* 当前版本为正常版本 */
    FACTORY_VERTYPE_FLAG,    /* 当前版本为工厂版本 */
    INVALID_VERTYPE_FLAG
} drv_soft_ver_type_e;

typedef enum {
    NORMAL_CHARGING_MODE = 0, /* 开机充电模式 */
    POWER_DOWN_CHARGING_MODE, /* 关机充电模式 */
    INVALID_CHARGING_MODE
} drv_charging_mode_e;

typedef enum {
    NORMAL_RESET_FLAG = 0, /* 正常复位版本 */
    ABNORMAL_RESET_FLAG,   /* 异常复位版本 */
    INVALID_RESET_FLAG
} drv_reset_type_e;

typedef enum {
    RUN_MODE_NORMAL = 0, /* 正常版本 */
    RUN_MODE_RECOVERY,   /* RECOVERY */
    RUN_MODE_INVALID
} drv_run_mode_e;

drv_charging_mode_e get_pd_charge_flag(void);
drv_run_mode_e get_run_mode(void);
drv_soft_ver_type_e get_version_type(void);
void drv_shut_down(drv_shutdown_reason_e en_reason, unsigned int delay_in_ms);
void bsp_reboot_callback_register(void (*hook)(void));
int bsp_on_off_init(void);
void balong_power_off(void);
void balong_power_restart(char mode, const char *cmd);
void bsp_drv_power_reboot(void);
int bsp_start_mode_get(void);
void bsp_drv_power_off(void);
void power_on_c_status_set(c_power_st_e val);
void bsp_drv_power_reboot_direct(void);
void bsp_drv_set_modem_state(unsigned int modem_id, unsigned int state);

#ifdef __cplusplus
}
#endif
#endif
