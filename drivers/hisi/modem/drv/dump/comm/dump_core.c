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

#include <linux/sched.h>
#include <linux/timer.h>
#include <linux/rtc.h>
#include <linux/thread_info.h>
#include <linux/syslog.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/semaphore.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/spinlock.h>
#include <linux/notifier.h>
#include <linux/kdebug.h>
#include <linux/reboot.h>
#include <linux/slab.h>
#include <linux/of.h>
#include <linux/delay.h>
#include <linux/wakelock.h>
#include <asm/string.h>
#include <asm/traps.h>
#include "product_config.h"
#include <linux/syscalls.h>
#include <bsp_cold_patch.h>
#include "osl_types.h"
#include "bsp_version.h"
#include "bsp_sram.h"
#include "bsp_dump_mem.h"
#include "bsp_coresight.h"
#include "mdrv_om.h"
#include "nv_stru_drv.h"
#include "bsp_nvim.h"
#include "bsp_dump.h"
#include "bsp_slice.h"
#include "dump_file.h"
#include "dump_config.h"
#include "dump_logs.h"
#include "dump_apr.h"
#include "dump_exc_type.h"
#include "dump_cp_agent.h"
#include "dump_area.h"
#include "dump_cp_core.h"
#include "dump_mdmap_core.h"
#include "dump_exc_type.h"
#undef	THIS_MODU
#define THIS_MODU mod_dump

modem_dump_ctrl_s               g_dump_ctrl;
volatile u32                    g_exc_flag   = false;
/*****************************************************************************
* 函 数 名  : dump_config_init
* 功能描述  : dump 配置初始化
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_set_exc_flag(u32 flag)
{
    g_exc_flag = flag;
}

/*****************************************************************************
* 函 数 名  : dump_check_has_error
* 功能描述  : 判断是否发生过一场
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
u32 dump_check_has_error(void)
{
    if(g_exc_flag)
    {
        dump_ok("dump is processing exception now, skip this excepiton\n");
        return true;
    }
    else
    {
        g_exc_flag = true;
    }
    return false;

}


/*****************************************************************************
* 函 数 名  : dump_save_log_files
* 功能描述  : 保存全部的modem log文件
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_log_files(char * dir_name)
{
    s32 ret = BSP_OK;

    if(dir_name == NULL)
    {
        dump_error("dir_name is null\n");
        return;
    }

    ret = dump_create_dir(dir_name);
    if(ret != BSP_OK )
    {
        dump_error("fail to creat dir\n");
        return;
    }

    dump_ok("start to save log file\n");

    dump_save_mandatory_logs(dir_name);

    bsp_dump_log_notifer_callback(SUB_SYS_LR,dir_name);

    apr_save_result_log(dir_name);

    dump_ok("all files save done\n");

}
/*****************************************************************************
* 函 数 名  : dump_save_and_reboot
* 功能描述  : 触发保存任务，并且复位
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_and_reboot(void)
{
    g_dump_ctrl.dump_task_job = DUMP_TASK_JOB_SAVE_REBOOT;

    up(&g_dump_ctrl.sem_dump_task);

    dump_ok("triger save log task \n");

    return;
}


/*****************************************************************************
* 函 数 名  : dump_save_task
* 功能描述  : 保存modem log的入口函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
int dump_save_task(void *data)
{
    rdr_exc_info_s* rdr_exc_info = dump_get_rdr_exc_info();

    /* coverity[no_escape] */
    while(1)
    {
        down(&g_dump_ctrl.sem_dump_task);

        dump_ok("enter save log task\n");

        if(DUMP_TASK_JOB_SAVE_REBOOT == (g_dump_ctrl.dump_task_job & DUMP_TASK_JOB_SAVE_REBOOT))
        {
            if(BSP_OK == dump_wait_mdmcp_done())
            {
                dump_save_log_files(rdr_exc_info->log_path);
                if(rdr_exc_info->dump_done != BSP_NULL)
                {
                    rdr_exc_info->dump_done(rdr_exc_info->modid, rdr_exc_info->coreid);

                    dump_ok("notify rdr dump handle finished\n");
                }
            }

        }

        g_dump_ctrl.dump_task_job = 0;
    }

    /*lint -e527 -esym(527,*)*/
    return BSP_OK;
    /*lint -e527 +esym(527,*)*/
}

/*****************************************************************************
* 函 数 名  : dump_save_task_init
* 功能描述  : 创建modem ap 保存log 的任务函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 dump_save_task_init(void)
{
    struct task_struct * pid = NULL;
    struct sched_param   param = {0,};

    sema_init(&g_dump_ctrl.sem_dump_task, 0);
    g_dump_ctrl.dump_task_job = 0;

    pid = (struct task_struct *)kthread_run(dump_save_task, 0, "dump_save");
    if (IS_ERR((void*)pid))
    {
        dump_error("fail to create kthread task ,ret=0x%pK\n", pid);
        return BSP_ERROR;
    }
    g_dump_ctrl.dump_task_id = (uintptr_t)pid;

    param.sched_priority = 97;
    if (BSP_OK != sched_setscheduler(pid, SCHED_FIFO, &param))
    {
        dump_error("fail to set scheduler failed\n");
        return BSP_ERROR;
    }

    dump_ok("save log task init ok\n");

    return BSP_OK;
}

/*****************************************************************************
* 函 数 名  : dump_save_balong_rdr_info
* 功能描述  : 在手机平台上更新rdr的global 头
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
void dump_save_balong_rdr_info(u32 mod_id)
{
    struct timex txc = {0,};
    struct rtc_time tm = {0,};
    char temp[32] = {0};
    struct dump_global_base_info_s* global_base_info = NULL;
    struct rdr_exception_info_s* rdr_exc_info = NULL;
    dump_reboot_cpu_t core = DUMP_CPU_BUTTON;
    struct dump_global_top_head_s* top_head = (struct dump_global_top_head_s*)dump_get_rdr_top_head();

    dump_get_reboot_contex((u32*)&core,NULL);

    do_gettimeofday(&(txc.time));
    rtc_time_to_tm((unsigned long)(txc.time.tv_sec), &tm);
    snprintf_s(temp,sizeof(temp), (sizeof(temp)-1), "%04d-%02d-%02d %02d:%02d:%02d", tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

    global_base_info = (struct dump_global_base_info_s*)dump_get_global_baseinfo();
    if(global_base_info == NULL)
    {
        dump_error("fail to get global base info\n");
        return;
    }

    rdr_exc_info = dump_get_exception_info_node(mod_id);

    if(rdr_exc_info == NULL)
    {
        dump_error("fail to find rdr exc info\n");
        return;
    }
    global_base_info->modid = rdr_exc_info->e_modid;
    global_base_info->arg1 = 0;
    global_base_info->arg2 = 0;
    /*coverity[secure_coding]*/
    memcpy_s(global_base_info->e_module,sizeof(global_base_info->e_module) ,rdr_exc_info->e_from_module,(unsigned long)16);
    global_base_info->e_type = rdr_exc_info->e_exce_type;

    /*这里为了hids工具显示，做了特殊处理，填充在rdr的ecore与注册给rdr的不一致*/
    if(core  == DUMP_CPU_COMM)
    {
        global_base_info->e_core = RDR_CP;
    }
    else if(core  == DUMP_CPU_APP)
    {
        global_base_info->e_core = RDR_MODEMAP;
    }
    global_base_info->start_flag = DUMP_START_EXCH;
    global_base_info->savefile_flag = DUMP_SAVE_FILE_END;

    memset_s(global_base_info->e_desc,sizeof(global_base_info->e_desc),0,sizeof(global_base_info->e_desc));
    /*coverity[secure_coding]*/
    memcpy_s((void*)(global_base_info->e_desc),sizeof(global_base_info->e_desc),rdr_exc_info->e_desc,(u32)(strlen((const char*)(rdr_exc_info->e_desc)) < 48 ? strlen((const char*)(rdr_exc_info->e_desc)): 48 ));
    /*coverity[secure_coding]*/
    memcpy_s(global_base_info->datetime,sizeof(global_base_info->datetime),temp,(unsigned long)24);
    global_base_info->timestamp = bsp_get_slice_value();

	if(top_head!=NULL)
	{
		top_head->codepatch = bsp_modem_cold_patch_is_exist();
	}
    dump_ok("update rdr global info ok\n");

}
/*****************************************************************************
* 函 数 名  : bsp_dump_init
* 功能描述  : modem dump 初始化函数
*
* 输入参数  :
* 输出参数  :

* 返 回 值  :

*
* 修改记录  : 2016年1月4日17:05:33   lixiaofan  creat
*
*****************************************************************************/
s32 __init bsp_dump_init(void)
{
    s32 ret ;

    dump_config_init();

    dump_set_init_phase(DUMP_INIT_FLAG_CONFIG);

    ret = dump_register_modem_exc_info();
    if(ret == BSP_ERROR)
    {
        dump_error("fail to register exception info\n");
        return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_RDR_REG);


    ret = dump_save_task_init();
    if(ret == BSP_ERROR)
    {
        return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_SAVETASK);


    ret = dump_mdmap_init();
    if(BSP_OK != ret)
    {
        return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_MDMAP);

    ret = dump_mdmcp_init();
    if(BSP_OK != ret)
    {
         return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_MDMCP);

    ret = apr_init();
    if(BSP_OK != ret)
    {
        dump_error("fail to init apr\n");

        return BSP_ERROR;
    }
    dump_set_init_phase(DUMP_INIT_FLAG_APR);

    dump_set_exc_flag(false);

    dump_set_init_phase(DUMP_INIT_FLAG_DONE);

    return BSP_OK;
}

