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
#ifndef __POWER_OFF_MBB_H__
#define __POWER_OFF_MBB_H__

/*************************************************************
***Macro definition 
***宏定义
*************************************************************/
#define FEATURE_ON  1
#define FEATURE_OFF 0
/*Maximum value of delayed shutdown, used when call drv_shut_down API*/
#define POWER_OFF_MONITOR_TIMEROUT      (5000) /*ms*/

#ifndef MBB_COMMON
#define bsp_log_print printk
#define bsp_log_print_and_save printk
#endif

/****:********************************************************
***datastruct defination,used by hisi and terminal baseline
***终端和海思共用的数据结构
*************************************************************/
enum SHUTDOWN_RESULT_ACTION{
    DO_POWER_OFF = 0,
    DO_SOFT_POWER_OFF,
    DO_REBOOT,
    DO_NOTHING,
    DO_INVALID,
};

typedef struct input_condition{
    unsigned long shutdown_reason;
    unsigned long the_charger_state; /*A comprehensive consideration of inward and outward charger*/
    unsigned long is_power_off_charging;      
}input_condition_t;

typedef struct output_condition{
    unsigned long result;
    unsigned long reboot_reason;
}output_condition_t;

typedef struct shutdown_class{
/*input condition*/
    struct input_condition factor;
/*output information*/ 
    struct output_condition out;
}shutdown_class_t;

/*for reboot and power off callback*/
typedef struct bsp_onoff_callback { 
    struct list_head node;
    void (*fn)(void);
}bsp_onoff_callback_t;

typedef struct power_off_ctrl{
    DRV_SHUTDOWN_REASON_E      reason;
    struct rtc_time            time;
	struct softtimer_list      off_timer;
}power_off_ctrl_t;

/*************************************************************
***The interfaces provided by the product
***终端预埋在海思基线的接口
*************************************************************/
/*power off*/
extern char (*mbb_judge_charger_in)( void );
extern void (*mbb_soft_power_off_operation)( void );
extern shutdown_class_t *mbb_shutdown_judge_table_address ;
extern unsigned int mbb_get_shutdown_table_size(void);
extern void mbb_record_reboot_mode( const char *cmd );

/*power on*/
extern int mbb_is_abnormal_mode( void );
#endif
