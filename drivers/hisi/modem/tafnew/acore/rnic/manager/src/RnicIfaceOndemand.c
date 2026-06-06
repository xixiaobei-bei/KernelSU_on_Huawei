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

/*****************************************************************************
  1 头文件包含
*****************************************************************************/
#include "vos.h"
#include "RnicIfaceOndemand.h"
#include "RnicCtx.h"
#include "RnicLog.h"
#include "RnicMntn.h"
#include "RnicMsgProc.h"
#include "rnic_ondemand_i.h"
#include <linux/wakelock.h>



/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_RNIC_IFACE_ONDEMAND_C


/*****************************************************************************
  2 全局变量定义
*****************************************************************************/

/*****************************************************************************
  3 函数实现
*****************************************************************************/


VOS_UINT32 RNIC_IFACE_OndemandSendDialEvent(
    VOS_UINT32                          ulDeviceId,
    VOS_UINT32                          ulEventId
)
{
    DEVICE_EVENT                        stEvent;
    VOS_INT                             lRet;

    stEvent.device_id   = (DEVICE_ID)ulDeviceId;
    stEvent.event_code  = (VOS_INT)ulEventId;
    stEvent.len         = 0;

    lRet = device_event_report(&stEvent, sizeof(DEVICE_EVENT));
    if (VOS_OK != lRet)
    {
        RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandSendDialEvent: can't add event");
        return VOS_ERR;
    }

    RNIC_INFO_LOG2(ACPU_PID_RNIC, "RNIC_IFACE_OndemandSendDialEvent Done ulEventId  ulDeviceId",ulEventId,ulDeviceId);

    return VOS_OK;
}


VOS_UINT32 RNIC_IFACE_OndemandDisconnTimeoutProc(RNIC_DEV_ID_ENUM_UINT8 enRmNetId)
{
    VOS_UINT32                          ulPktsNum;
    VOS_UINT32                          ulCount;
    VOS_UINT32                          ulUserExistFlg;
    VOS_UINT32                          ulExpiredCount;

    /* 将用户设置的时长按定时器时长等分 */
    ulExpiredCount  = (RNIC_GET_PROC_IDLE_TIME() * TI_RNIC_UNIT) / TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN;

    /* 如果当前是手动拨号,则直接返回,不需要通知 */
    if (RNIC_DIAL_MODE_DEMAND_CONNECT != RNIC_GET_PROC_DIAL_MODE())
    {
        RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandDisconnTimeoutProc: Dialmode is manual!");
        return VOS_ERR;
    }

    /* 获取在定时器启动后上下行统计的数据包数以及当前用户连接状态 */
    ulPktsNum = RNIC_GET_IFACE_PERIOD_SEND_PKT(enRmNetId);
    ulUserExistFlg = DRV_AT_GET_USER_EXIST_FLAG();

    /* 按需拨号断开逻辑如下:
         有用户连接且有数据包:
             计数器清零
         其他情况:
             计数器++
       以下场景无法识别:
          当前连接USB，但用户无数据发送，而网络有数据下发，导致也会认为
          是有用户连接且有数据包，而重新计数。
    */
    if ((0 != ulPktsNum)
     && (VOS_TRUE == ulUserExistFlg))
    {
        RNIC_CLEAR_TI_DIALDOWN_EXP_CONT();
    }
    else
    {
        RNIC_ADD_TI_DIALDOWN_EXP_CONT();
    }

    /* 判断统计计数器是否大于等于用户设置断开拨号时长与定时器时长等分 */
    ulCount = RNIC_GET_TI_DIALDOWN_EXP_CONT();

    if ((ulCount >= ulExpiredCount)
     && (RNIC_ALLOW_EVENT_REPORT == RNIC_GET_PROC_EVENT_REPORT()))
    {
        /*通知应用断开拨号 */
        if (VOS_OK == RNIC_IFACE_OndemandSendDialEvent(DEVICE_ID_WAN, RNIC_DAIL_EVENT_DOWN))
        {
            /* 上报断开拨号事件后投票不进入睡眠，在断开拨号成功时解除 */
            wake_lock_timeout(&g_stRnicCtx.stOnDemandDisconnWakeLock, (long)msecs_to_jiffies(5000));
            RNIC_DBG_SEND_APP_DIALDOWN_SUCC_NUM(1);
            RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandDisconnTimeoutProc: Send RNIC_DAIL_EVENT_DOWN Msg succ.");
        }
        else
        {
            RNIC_DBG_SEND_APP_DIALDOWN_FAIL_NUM(1);
            RNIC_ERROR_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandDisconnTimeoutProc:Send RNIC_DAIL_EVENT_DOWN Msg FAIL");
        }

        RNIC_MNTN_TraceDialDisconnEvt(ulPktsNum, ulUserExistFlg);
    }

    return VOS_OK;
}


VOS_VOID RNIC_IFACE_OndemandTxDataProc(VOS_VOID)
{
    RNIC_TIMER_STATUS_ENUM_UINT8        enTiStatus;

    /* 获取当前拨号保护定时器的状态 */
    enTiStatus  = RNIC_GetTimerStatus(TI_RNIC_DEMAND_DIAL_PROTECT);

    /*为了防止按需拨号上报太快，启动一个两秒定时器，*/
    if (RNIC_TIMER_STATUS_STOP == enTiStatus)
    {
        /* 通知应用进行拨号操作 */
        if (RNIC_ALLOW_EVENT_REPORT == RNIC_GET_PROC_EVENT_REPORT())
        {
            if (VOS_OK == RNIC_IFACE_OndemandSendDialEvent(DEVICE_ID_WAN, RNIC_DAIL_EVENT_UP))
            {
                /* 启动拨号保护定时器  */
                RNIC_StartTimer(TI_RNIC_DEMAND_DIAL_PROTECT, TI_RNIC_DEMAND_DIAL_PROTECT_LEN);
                RNIC_DBG_SEND_APP_DIALUP_SUCC_NUM(1);
                RNIC_NORMAL_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandTxDataProc: Send RNIC_DAIL_EVENT_UP event succ.");
            }
            else
            {
                RNIC_DBG_SEND_APP_DIALUP_FAIL_NUM(1);
                RNIC_WARNING_LOG(ACPU_PID_RNIC, "RNIC_IFACE_OndemandTxDataProc: Send RNIC_DAIL_EVENT_UP event fail.");
            }

            RNIC_MNTN_TraceDialConnEvt();
        }
    }

    return;
}


VOS_VOID RNIC_IFACE_OndemandEventReportProc(VOS_UINT32 ulEventReport)
{
    if (ulEventReport >= RNIC_DIAL_EVENT_REPORT_FLAG_BUTT)
    {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_IFACE_OndemandEventReportProc: invalid ulEventReport: ", ulEventReport);
        return;
    }

    /* 保存拨号模式上下文中 */
    RNIC_SET_PROC_EVENT_REPORT(ulEventReport);
    /* 发消息勾包 */
    RNIC_SendDialInfoMsg(ID_RNIC_MNTN_EVENT_REPORT_INFO);

    return;
}


VOS_VOID RNIC_IFACE_OndemandIdleTimeProc(VOS_UINT32 ulIdleTime)
{
    /* PDP激活，且与上次设置的时长不一致，需要启动拨号断开定时器 */
    if ((RNIC_DIAL_MODE_DEMAND_DISCONNECT == RNIC_GET_PROC_DIAL_MODE())
     && (ulIdleTime != RNIC_GET_PROC_IDLE_TIME()))
    {
        RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT);
        RNIC_StartTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT, TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN);
    }

    /* 保存拨号模式上下文中 */
    RNIC_SET_PROC_IDLE_TIME(ulIdleTime);

    /* 发消息勾包 */
    RNIC_SendDialInfoMsg(ID_RNIC_MNTN_IDLE_TIMEOUT_INFO);

    return;
}


VOS_VOID RNIC_IFACE_OndemandDialModeProc(VOS_UINT32 ulDialMode)
{
    if (ulDialMode >= RNIC_DIAL_MODE_BUTT)
    {
        RNIC_ERROR_LOG1(ACPU_PID_RNIC, "RNIC_IFACE_OndemandDialModeProc: invalid ulDialMode ", ulDialMode);
        return;
    }

    if (RNIC_DIAL_MODE_DEMAND_CONNECT == ulDialMode)
    {
        RNIC_StartTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT, TI_RNIC_DEMAND_DIAL_DISCONNECT_LEN);
    }
    else
    {
        RNIC_StopTimer(TI_RNIC_DEMAND_DIAL_DISCONNECT);
    }

    /* 保存拨号模式上下文中 */
    RNIC_SET_PROC_DIAL_MODE(ulDialMode);

    /* 发消息勾包 */
    RNIC_SendDialInfoMsg(ID_RNIC_MNTN_DIAL_MODE_INFO);

    return;
}



