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




#include "product_config.h"
#if(FEATURE_ON == FEATURE_ACPU_STAT)

/******************************************************************************
   头文件包含
******************************************************************************/

#include "vos_config.h"
#include "ACpuLoadInterface.h"
#include "ACpuLoad.h"
#include "NVIM_Interface.h"
#include "product_config.h"
#include "acore_nv_id_gucttf.h"
#include "acore_nv_stru_gucttf.h"

/*lint -e322 -e7*/
#include <linux/cpumask.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/kernel_stat.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/seq_file.h>
#include <linux/slab.h>
#include <linux/time.h>
#include <linux/irqnr.h>
#include <asm/cputime.h>
#include <linux/msa.h>
/*lint +e322 +e7*/


/*****************************************************************************
   1 协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define THIS_FILE_ID                PS_FILE_ID_CPULOAD_C


/******************************************************************************
   2 外部函数变量声明
******************************************************************************/
/* 参考/fs/proc/stat.c实现 */
#define arch_idle_time(cpu)         0


/*****************************************************************************
   3 私有定义
*****************************************************************************/

/*****************************************************************************
   4 全局变量定义
*****************************************************************************/

/* 根据Timer定时进行的CPU负载统计 */
CPULOAD_STAT_INFO_STRU  g_stRegularCpuLoad;

/* 实时响应用户进行的CPU负载统计 */
CPULOAD_STAT_INFO_STRU  g_stUserDefCpuLoad;

/*记录是否是第一次超时*/
VOS_INT32               g_ulFirstTimeout = VOS_TRUE;

/*回调函数指针数组*/
CPULOAD_RPT_HOOK_FUNC   g_pFunRptHooks[CPULOAD_MAX_HOOK_NUM];

/*CPU统计周期定时器*/
HTIMER                  g_stRegularCpuLoadTmr;

/*CPU LOAD NV 配置信息*/
CPULOAD_CFG_STRU        g_stNvCfg;

/******************************************************************************
   5 函数实现
******************************************************************************/


VOS_VOID CPULOAD_ReadCpuStat(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* V9R1项目中使用宏开关判断是否读取CPU使用情况数据 */
    /* 获取空闲任务的运行时间和总的运行时间 */
    msa_getcpu_idle(&(pstCpu->stCurrRecord.ulTotalTime), &(pstCpu->stCurrRecord.ulIdleTime));

    return;
}


VOS_VOID CPULOAD_UpdateSavInfo(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* 更新保存的结果 */
    PSACORE_MEM_CPY(&(pstCpu->stPrevRecord), sizeof(CPULOAD_STAT_RECORD_STRU), &(pstCpu->stCurrRecord), sizeof(CPULOAD_STAT_RECORD_STRU));

    return;
}


VOS_UINT32 CPULOAD_CalLoad(CPULOAD_STAT_INFO_STRU *pstCpu)
{
    /* V9R1项目中使用宏开关进行打桩处理 */
    VOS_UINT32                          ulIdle;
    VOS_UINT32                          ulTotal;
    VOS_UINT32                          ulLoad;


    ulIdle      = MOD_SUB((pstCpu->stCurrRecord.ulIdleTime),(pstCpu->stPrevRecord.ulIdleTime),SLIENCE_MAX);
    ulTotal     = MOD_SUB((pstCpu->stCurrRecord.ulTotalTime),(pstCpu->stPrevRecord.ulTotalTime),SLIENCE_MAX);

    /* 异常保护，如果上报的tick值有问题，CPU load按照0处理，相当于把此功能off掉 */
    if (0 == ulTotal)
    {
        ulLoad = 0;

    }
    else
    {
        ulLoad = (100 * (ulTotal - ulIdle))/ulTotal;
    }

    /* 保存此次计算结果 */
    pstCpu->ulCpuLoad = ulLoad;

    return ulLoad;
}


VOS_UINT32  CPULOAD_GetCpuLoad(VOS_VOID)
{
    /* 使用宏开关判断是否需要打桩处理 */
    return g_stRegularCpuLoad.ulCpuLoad;
}


VOS_VOID CPULOAD_InvokeRtpHooks(VOS_UINT32  ulLoad)
{
    VOS_UINT32                          ulHookLoop;


    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        if ( VOS_NULL_PTR != g_pFunRptHooks[ulHookLoop] )
        {
            g_pFunRptHooks[ulHookLoop](ulLoad);
        }
    }

    return;
}


VOS_VOID CPULOAD_RegularTimeoutProc(VOS_VOID)
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stRegularCpuLoad;
    VOS_UINT32                          ulLoad;

    /* 读取当前的CPU统计数据 */
    CPULOAD_ReadCpuStat(pstCpu);

    ulLoad = CPULOAD_CalLoad(pstCpu);

    /* 保存此次数据，用于与下次计算 */
    CPULOAD_UpdateSavInfo(pstCpu);

    /* 第一次收到TimeOut消息，由于上次记录为空，计算不准 */
    if ( VOS_TRUE == g_ulFirstTimeout )
    {
        g_ulFirstTimeout = VOS_FALSE;
        return;
    }

    CPULOAD_InvokeRtpHooks(ulLoad);

    return;
}


VOS_VOID CPULOAD_RcvTimerExpireMsg(REL_TIMER_MSG *pTimerMsg)
{
    switch (pTimerMsg->ulName)
    {
        case CPULOAD_REGULAR_TMR_NAME:
            CPULOAD_RegularTimeoutProc();
            break;
        default:
            break;
    }

    return;
}


VOS_UINT32 CPULOAD_Init(VOS_VOID)
{
    VOS_UINT32                          ulHookLoop;
    VOS_UINT32                          ulRtn;
    CPULOAD_CFG_STRU                   *pstNvCfg = &g_stNvCfg;

    /* CPU ID目前只有一个，赋值为0 */
    PSACORE_MEM_SET((VOS_VOID *)&g_stRegularCpuLoad, sizeof(CPULOAD_STAT_INFO_STRU), 0, sizeof(CPULOAD_STAT_INFO_STRU));
    PSACORE_MEM_SET((VOS_VOID *)&g_stUserDefCpuLoad, sizeof(CPULOAD_STAT_INFO_STRU), 0, sizeof(CPULOAD_STAT_INFO_STRU));

    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        g_pFunRptHooks[ulHookLoop] = VOS_NULL_PTR;
    }

    ulRtn = GUCTTF_NV_Read(MODEM_ID_0, en_NV_Item_Linux_CPU_Moniter_Timer_Len, pstNvCfg, sizeof(CPULOAD_CFG_STRU));


    /* 若读取en_NV_Item_Linux_CPU_Moniter_Timer_Len失败,或NV值非法，则使用默认值 */
    if( (NV_OK != ulRtn)
        || (CPULOAD_REGULAR_TMR_MIN_LEN > pstNvCfg->ulMonitorTimerLen)
        || (CPULOAD_REGULAR_TMR_MAX_LEN < pstNvCfg->ulMonitorTimerLen))
    {

        /*如果异常，则将定时器初始化为默认值400ms*/
        pstNvCfg->ulMonitorTimerLen = CPULOAD_REGULAR_TMR_DEF_LEN;
    }

    /* V9R1项目中使用宏开关判断是否需要启动定时器 */
    ulRtn = VOS_StartRelTimer(&g_stRegularCpuLoadTmr, ACPU_PID_CPULOAD,
                              pstNvCfg->ulMonitorTimerLen,
                              CPULOAD_REGULAR_TMR_NAME, 0,
                              VOS_RELTIMER_LOOP, VOS_TIMER_NO_PRECISION);
    if (VOS_OK != ulRtn)
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


VOS_VOID CPULOAD_PidMsgProc(MsgBlock *pRcvMsg)
{
    switch( pRcvMsg->ulSenderPid )
    {
        case VOS_PID_TIMER:
            CPULOAD_RcvTimerExpireMsg( (REL_TIMER_MSG *)pRcvMsg ); /* 接收到TIMER超时消息时的处理 */
            break;

        default:
            break;
    }
}


VOS_UINT32 CPULOAD_FidInit(enum VOS_INIT_PHASE_DEFINE enPhase)
{
    VOS_UINT32  ulResult = VOS_ERR;


    switch (enPhase)
    {
        case   VOS_IP_LOAD_CONFIG:

            ulResult = VOS_RegisterPIDInfo(ACPU_PID_CPULOAD,
                                           (Init_Fun_Type)VOS_NULL_PTR,
                                           (Msg_Fun_Type)CPULOAD_PidMsgProc);
            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            ulResult = VOS_RegisterTaskPrio(ACPU_FID_CPULOAD, TTF_ACPULOAD_TASK_PRIO);

            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            ulResult = CPULOAD_Init();

            if (VOS_OK != ulResult)
            {
                return VOS_ERR;
            }

            break;
        case   VOS_IP_FARMALLOC:
        case   VOS_IP_INITIAL:
        case   VOS_IP_ENROLLMENT:
        case   VOS_IP_LOAD_DATA:
        case   VOS_IP_FETCH_DATA:
        case   VOS_IP_STARTUP:
        case   VOS_IP_RIVAL:
        case   VOS_IP_KICKOFF:
        case   VOS_IP_STANDBY:
        case   VOS_IP_BROADCAST_STATE:
        case   VOS_IP_RESTART:
        case   VOS_IP_BUTT:
            break;
        default:
            break;
    }

    return VOS_OK;
}


VOS_UINT32  CPULOAD_RegRptHook(CPULOAD_RPT_HOOK_FUNC pRptHook)
{
    VOS_UINT32                          ulHookLoop;
    VOS_INT32                           lLockKey;


    lLockKey = VOS_SplIMP();

    for ( ulHookLoop = 0 ; ulHookLoop < CPULOAD_MAX_HOOK_NUM ; ulHookLoop++ )
    {
        if ( VOS_NULL_PTR == g_pFunRptHooks[ulHookLoop] )
        {
            g_pFunRptHooks[ulHookLoop] = pRptHook;
            VOS_Splx(lLockKey);

            return VOS_OK;
        }
    }

    VOS_Splx(lLockKey);

    return VOS_ERR;
}


VOS_VOID CPULOAD_ResetUserDefLoad()
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stUserDefCpuLoad;


    /* 读取当前的CPU统计数据 */
    CPULOAD_ReadCpuStat(pstCpu);

    /* 保存此次数据，用于与下次计算 */
    CPULOAD_UpdateSavInfo(pstCpu);

    return;
}


VOS_UINT32 CPULOAD_GetUserDefLoad()
{
    CPULOAD_STAT_INFO_STRU             *pstCpu = &g_stUserDefCpuLoad;
    VOS_UINT32                          ulLoad;


    /* 读取当前的CPU统计数据 */
    CPULOAD_ReadCpuStat(pstCpu);

    /* 与初始化时数据计算得到 */
    ulLoad = CPULOAD_CalLoad(pstCpu);

    /* 保存此次数据，用于与下次计算 */
    CPULOAD_UpdateSavInfo(pstCpu);

    return ulLoad;
}


VOS_UINT32  CPULOAD_GetRegularTimerLen()
{
    return g_stNvCfg.ulMonitorTimerLen;
}

#endif


