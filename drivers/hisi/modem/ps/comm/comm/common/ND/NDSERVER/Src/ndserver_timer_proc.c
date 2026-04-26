/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.
 * Create: 2012/10/30
 *
 * If distributed as part of the Linux kernel, the following license terms
 * apply:
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 and
 *  only version 2 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA
 *
 * Otherwise, the following license terms apply:
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *  1) Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2) Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *  3) Neither the name of Huawei nor the names of its contributors may
 *     be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
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
/*
 * 1 Include HeadFile
 */

#include "ndserver_timer_proc.h"
#include "securec.h"
#include "ip_comm.h"
#include "ip_nd_server.h"
#include "ndserver_entity.h"
#include "ndserver_ns_proc.h"
#include "ndserver_debug.h"
#include "ndserver_ns_proc.h"
#include "ndserver_ra_proc.h"
#include "ndserver_om.h"


#define THIS_MODU ps_nd
/*lint -e767*/
#define THIS_FILE_ID PS_FILE_ID_NDSERVERTIMERPROC_C
/*lint +e767*/

/*
 * 2 Declare the Global Variable
 */


/*
 * Description: 判断定时器名是否合法
 * Input: ulIndex     --- ND SERVER数据体索引号
 *                   enTimerType --- 定时器类型
 * History: VOS_UINT32
 *    1.      2011-04-02  Draft Enact
 */
STATIC VOS_UINT32 IP_NDSERVER_IsTimerNameValid(VOS_UINT32 addrIndex, NDSER_TimerUint32 timerType)
{
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
        case IP_ND_SERVER_TIMER_FIRST_NS:
        case IP_ND_SERVER_TIMER_RA: {
            if (addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM) {
                return IP_TRUE;
            }

            break;
        }
        default:
            break;
    }

    return IP_FALSE;
}

/*
 * Description: 获取定时器
 * Input: enTimerType------------------定时器类型
 *                   ulIndex----------------------定时器索引号
 * History:
 *    1.      2011-04-02  Draft Enact
 */
IP_TimerInfo *IP_NDSERVER_GetTimer(VOS_UINT32 timerIndex, NDSER_TimerUint32 timerType)
{
    IP_TimerInfo *timerInfo = VOS_NULL_PTR;

    /* 根据定时器不同类型，获取定时器 */
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
        case IP_ND_SERVER_TIMER_FIRST_NS:
        case IP_ND_SERVER_TIMER_RA:
            timerInfo = IP_NDSERVER_ADDRINFO_GET_TIMER(timerIndex);
            break;
        default:
            break;
    }

    return timerInfo;
}

/*
 * Description: 获取定时器时长
 * Input: enTimerType------------------定时器类型
 * History:
 *    1.      2011-04-02  Draft Enact
 */
STATIC VOS_UINT32 IP_NDSERVER_GetTimerLen(NDSER_TimerUint32 timerType)
{
    VOS_UINT32 timerLen;

    /* 根据定时器不同类型，定时器时长不同 */
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
            timerLen = g_nsTimerLen;
            break;
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
            timerLen = g_periodicNsTimerLen;
            break;
        case IP_ND_SERVER_TIMER_FIRST_NS:
            timerLen = g_firstNsTimerLen;
            break;
        case IP_ND_SERVER_TIMER_RA:
            timerLen = g_raTimerLen;
            break;
        default:
            timerLen = IP_NULL;
            break;
    }

    return timerLen;
}
/*lint +e550*/
/*
 * Discription: 启动某一承载的某种类型的定时器
 * History:
 *      1.      2011-04-02  Draft Enact
 */
VOS_VOID IP_NDSERVER_TimerStart(VOS_UINT32 timerIndex, NDSER_TimerUint32 timerType)
{
    VOS_UINT32     timerLen;
    IP_TimerInfo  *timerInfo = VOS_NULL_PTR;

    /* 对ulIndex合法性判断 */
    if (IP_NDSERVER_IsTimerNameValid(timerIndex, timerType) == IP_FALSE) {
        /* 打印异常信息 */
        IPND_WARNING_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart: WARN: Input Para(ulIndex) err !", timerIndex,
                          timerType);
        return;
    }

    /* 根据消息对应的索引号和定时器类型,获取相关联的定时器 */
    timerInfo = IP_NDSERVER_GetTimer(timerIndex, timerType);
    if (timerInfo == VOS_NULL_PTR) {
        /* 打印异常信息 */
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart:ERROR: Get Timer failed.");
        return;
    }

    /* 判断定时器是否打开，已打开则关闭 */
    if (timerInfo->hTm != VOS_NULL_PTR) {
        /* 关闭失败，则报警返回 */
        if (PS_STOP_REL_TIMER(&(timerInfo->hTm)) != VOS_OK) {
            /* 打印异常信息 */
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart:WARN: stop reltimer error!");
            return;
        }

        /* 打印异常信息 */
        IPND_INFO_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart:(TimerType) Timer not close!", timerInfo->timerName,
                       timerIndex);
    }

    /* 根据定时器不同类型，定时器信息不同 */
    timerLen = IP_NDSERVER_GetTimerLen(timerType);
    if (timerLen == IP_NULL) {
        /* 打印异常信息 */
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart:ERROR: start unreasonable reltimer.");
        return;
    }

    /* 设定定时器NAME,enTimerType设定定时器Para，打开失败则报警返回 */
    if (PS_START_REL_TIMER(&(timerInfo->hTm), NDIS_NDSERVER_PID, timerLen, (VOS_UINT32)timerType,
                           timerIndex, VOS_RELTIMER_NOLOOP) != VOS_OK) {
        /* 打印异常信息 */
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStart:WARN: start reltimer error!");
        return;
    }

    /* 更新定时器类别 */
    timerInfo->timerName = timerType;

    /* 打印定时器启动信息 */
    IP_NDSERVER_PrintTimeStartInfo(timerIndex, timerType);

    return;
}

/*
 * Discription: 停止某一承载某种类型定时器
 * Return: None
 *      1.      2011-04-02  Draft Enact
 */
VOS_VOID IP_NDSERVER_TimerStop(VOS_UINT32 timerIndex, NDSER_TimerUint32 timerType)
{
    IP_TimerInfo *timerInfo = VOS_NULL_PTR;

    /* 对ulIndex合法性判断 */
    if (IP_NDSERVER_IsTimerNameValid(timerIndex, timerType) == IP_FALSE) {
        /* 打印异常信息 */
        IPND_WARNING_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStop: WARN: Input Para(ulIndex) err !", timerIndex,
                          timerType);
        return;
    }

    /* 根据消息对应的索引号和定时器类型,获取相关联的定时器 */
    timerInfo = IP_NDSERVER_GetTimer(timerIndex, timerType);
    if (timerInfo == VOS_NULL_PTR) {
        /* 打印异常信息 */
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStop:ERROR:Get Timer failed.");
        return;
    }

    /* 定时器处于打开状态，则关闭，否则，忽略 */
    if (timerInfo->hTm != VOS_NULL_PTR) {
        if (timerType != timerInfo->timerName) {
            /* 打印异常信息 */
            IPND_WARNING_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStop: TimerType not match:", timerType,
                              timerInfo->timerName);
        }
        /* 关闭失败，则报警返回 */
        if (PS_STOP_REL_TIMER(&(timerInfo->hTm)) != VOS_OK) {
            /* 打印异常信息 */
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerStop:WARN: stop reltimer error!");
            return;
        }

        /* 清除超时计数 */
        timerInfo->loopTimes = 0;

        /* 打印定时器关闭信息 */
        IP_NDSERVER_PrintTimeStopInfo(timerIndex, timerType);
    }

    return;
}
/*
 * Function Name: IP_NDSERVER_ProcTimerMsgNsExp
 * Description: 发送邻居请求后，等待邻居公告超时
 * Input: pMsg -------- 消息指针
 * History:
 *      1.      2011-04-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_ProcTimerMsgNsExp(VOS_UINT32 addrIndex)
{
    IP_NdserverAddrInfo    *infoAddr = IP_NULL_PTR;
    IP_NdserverTeAddrInfo  *teInfo   = IP_NULL_PTR;

    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgNsExp is entered.");

    /* 从消息中取出ND SERVER索引 */
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    teInfo   = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);

    if ((infoAddr->validFlag != IP_TRUE) ||
        (infoAddr->teAddrInfo.teAddrState == IP_NDSERVER_TE_ADDR_INEXISTENT)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgNsExp: ND Server state err!");
        return;
    }

    /* 判定超时次数是否小于规定超时次数 */
    if (infoAddr->timerInfo.loopTimes < g_nsTimerMaxExpNum) {
        /* 发送NS消息到PC */
        if (PS_SUCC != IP_NDSERVER_SendNsMsg(addrIndex, teInfo->teGlobalAddr)) {
            IP_NDSERVER_AddTransPktFailNum(addrIndex);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgNsExp:Send NS Msg failed.");
        }

        /* 将定时器超时次数加1 */
        infoAddr->timerInfo.loopTimes++;

        /* 重启邻居请求定时器 */
        IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_NS);
    } else {
        /* 清除定时器信息 */
        infoAddr->timerInfo.loopTimes = 0;

        /* 如果未完成状态的TE地址不响应地址解析请求，则清除TE地址，重发RA */
        if (infoAddr->teAddrInfo.teAddrState == IP_NDSERVER_TE_ADDR_INCOMPLETE) {
            infoAddr->teAddrInfo.teAddrState = IP_NDSERVER_TE_ADDR_INEXISTENT;

            /* 发送RA消息到PC */
            if (IP_NDSERVER_SendRaMsg(addrIndex, VOS_NULL_PTR) != PS_SUCC) {
                IP_NDSERVER_AddTransPktFailNum(addrIndex);
                IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgNsExp:Send RA Msg failed.");
            }

            /* 启动路由公告定时器 */
            IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_RA);
            IP_NDSERVER_ClearDlPktQue(addrIndex); /* 清除下行缓存队列中的PKT */
        } else {
            /* 启动周期性邻居请求定时器 */
            IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_PERIODIC_NS);
        }
    }

    return;
}

/*
 * Function Name: IP_NDSERVER_ProcTimerMsgPeriodicNsExp
 * Description: 周期性邻居请求超时
 * Input: pMsg -------- 消息指针
 * History:
 *      1.      2011-04-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_ProcTimerMsgPeriodicNsExp(VOS_UINT32 addrIndex)
{
    IP_NdserverAddrInfo    *infoAddr = IP_NULL_PTR;
    IP_NdserverTeAddrInfo  *teInfo   = IP_NULL_PTR;

    /* 打印进入该函数 */
    IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgPeriodicNsExp is entered.");

    /* 从消息中取出ND SERVER索引 */
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    teInfo   = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);

    if ((infoAddr->validFlag != IP_TRUE) ||
        (infoAddr->teAddrInfo.teAddrState == IP_NDSERVER_TE_ADDR_INEXISTENT)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgPeriodicNsExp: ND Server state err!");
        return;
    }

    /* 增加周期性发送RA计时，防止Router过期 */
    if (g_periodicRaTimeCnt[addrIndex]-- == 0) {
        g_periodicRaTimeCnt[addrIndex] = g_periodicRaTimerLen / g_periodicNsTimerLen;
        ;

        /* 发送RA消息到PC */
        if (IP_NDSERVER_SendRaMsg(addrIndex, VOS_NULL_PTR) != PS_SUCC) {
            IP_NDSERVER_AddTransPktFailNum(addrIndex);
            IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgPeriodicNsExp:Send RA Msg failed.");
        }
    }

    /* 发送NS消息到PC */
    if (IP_NDSERVER_SendNsMsg(addrIndex, teInfo->teGlobalAddr) != PS_SUCC) {
        IP_NDSERVER_AddTransPktFailNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgPeriodicNsExp:Send NS Msg failed.");
    }

    /* 清除定时器信息 */
    infoAddr->timerInfo.loopTimes = 0;
    /* 启动邻居请求定时器 */
    IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_NS);

    return;
}

/*
 * Function Name: IP_NDSERVER_ProcTimerMsgPeriodicNsExp
 * Description: 收到重复地址检测后等待定时器超时
 * Input: pMsg -------- 消息指针
 * History:
 *      1.      2011-04-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_ProcTimerMsgFirstNsExp(VOS_UINT32 addrIndex)
{
    IP_NdserverAddrInfo    *infoAddr = IP_NULL_PTR;
    IP_NdserverTeAddrInfo  *teInfo   = IP_NULL_PTR;

    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgFirstNsExp is entered.");

    /* 从消息中取出ND SERVER索引 */
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    teInfo   = IP_NDSERVER_ADDRINFO_GET_TEINFO(addrIndex);

    if ((infoAddr->validFlag != IP_TRUE) ||
        (infoAddr->teAddrInfo.teAddrState == IP_NDSERVER_TE_ADDR_INEXISTENT)) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgFirstNsExp: ND Server state err!");
        return;
    }

    /* 发送NS消息到PC */
    if (IP_NDSERVER_SendNsMsg(addrIndex, teInfo->teGlobalAddr) != PS_SUCC) {
        IP_NDSERVER_AddTransPktFailNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgFirstNsExp:Send NS Msg failed.");
    }

    /* 清除定时器信息 */
    infoAddr->timerInfo.loopTimes = 0;
    /* 启动邻居请求定时器 */
    IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_NS);

    return;
}

/*
 * Function Name: IP_NDSERVER_ProcTimerMsgRaExp
 * Description: 收到重复地址检测前周期发送路由公告超时
 * Input: pMsg -------- 消息指针
 * History:
 *      1.      2011-04-09  Draft Enact
 */
STATIC VOS_VOID IP_NDSERVER_ProcTimerMsgRaExp(VOS_UINT32 addrIndex)
{
    IP_NdserverAddrInfo        *infoAddr                    = IP_NULL_PTR;
#if (VOS_OS_VER != VOS_WIN32)
    VOS_UINT8                  *data                        = VOS_NULL_PTR;
    VOS_INT32                   lockKey;
#endif
    VOS_INT32                   rlt;
    IMM_Zc                     *immZc                       = VOS_NULL_PTR;
    VOS_UINT8                   dstIPAddr[IP_IPV6_ADDR_LEN];

    /* 打印进入该函数 */
    IPND_INFO_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgRaExp is entered.");

    /* 从消息中取出ND SERVER索引 */
    IP_ASSERT(addrIndex < IP_NDSERVER_ADDRINFO_MAX_NUM);

    infoAddr = IP_NDSERVER_ADDRINFO_GET_ADDR(addrIndex);
    if (infoAddr->validFlag != IP_TRUE) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgRaExp: ND Server state err!");
        return;
    }

    /* 发送RA消息到PC */
    if (IP_NDSERVER_SendRaMsg(addrIndex, VOS_NULL_PTR) != PS_SUCC) {
        IP_NDSERVER_AddTransPktFailNum(addrIndex);
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgRaExp:Send RA Msg failed.");
    }

    /* 清除定时器信息 */
    infoAddr->timerInfo.loopTimes = 0;
    /* 启动路由公告定时器 */
    IP_NDSERVER_TimerStart(addrIndex, IP_ND_SERVER_TIMER_RA);

    /* NDIS下移C核，无缓存队列 */
#if (VOS_OS_VER != VOS_WIN32)
    /* *********** 下行IP包的目的地址作为NS包的目标地址，进行地址解析 ********* */
    lockKey = VOS_SplIMP();
    if (LUP_PeekQueHead(IP_NDSERVER_ADDRINFO_GET_DLPKTQUE(addrIndex), (VOS_VOID **)(&immZc)) != PS_SUCC) {
        VOS_Splx(lockKey);
        return;
    }
    VOS_Splx(lockKey);

    data = IMM_ZcGetDataPtr(immZc);
    if (data == VOS_NULL_PTR) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgRaExp, IMM_ZcGetDataPtr return NULL");
        return;
    }

    /* 得到目的IPV6地址，触发NS发送 */
    rlt = memcpy_s(dstIPAddr, IP_IPV6_ADDR_LEN, data + IP_IPV6_DST_ADDR_OFFSET, IP_IPV6_ADDR_LEN);
    IP_CHK_SEC_RETURN_NO_VAL(rlt != EOK);
    if (IP_NDSERVER_SendNsMsg(addrIndex, dstIPAddr) != PS_SUCC) {
        IPND_ERROR_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_ProcTimerMsgRaExp, IP_NDSERVER_SendNsMsg return NULL");
        return;
    }
#endif
    return;
}

/*
 * Function Name: IP_NDSERVER_TimerMsgDistr
 * Description: ND SERVER TIMER消息分发函数
 * History:
 *      1.     2011-04-02  Draft Enact
 */
NDSER_TimerUint32 IP_NDSERVER_TimerMsgDistr(const MsgBlock *rcvMsg)
{
    VOS_UINT32              timerIndex;
    NDSER_TimerUint32       timerType;
    IP_TimerInfo           *timerInfo = VOS_NULL_PTR;

    /* 从消息中取出ulIndex和enTimerType */
    timerIndex     = IP_GetTimerPara(rcvMsg);
    timerType = (NDSER_TimerUint32)IP_GetTimerName(rcvMsg);
    /* 判断合法性 */
    if (IP_NDSERVER_IsTimerNameValid(timerIndex, timerType) == IP_FALSE) {
        /* 打印异常信息 */
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerMsgDistr: Invalid Timer Name or Para !");
        return IP_MSG_DISCARD;
    }

    /* 根据消息对应的索引号和定时器类型,获取相关联的定时器 */
    timerInfo = IP_NDSERVER_GetTimer(timerIndex, timerType);
    if (timerInfo == VOS_NULL_PTR) {
        /* 打印异常信息 */
        IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerMsgDistr: Get Timer failed.");
        return IP_MSG_DISCARD;
    }

    /* 判断消息类型与定时器类型是否一致 */
    if (timerType != timerInfo->timerName) {
        /* 打印异常信息 */
        IPND_WARNING_LOG2(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerMsgDistr: TimerType not match:", timerType,
                          timerInfo->timerName);
        return IP_MSG_DISCARD;
    }

    /* 定时器超时处理 */
    switch (timerType) {
        case IP_ND_SERVER_TIMER_NS:
            IP_NDSERVER_ProcTimerMsgNsExp(timerIndex);
            break;
        case IP_ND_SERVER_TIMER_PERIODIC_NS:
            IP_NDSERVER_ProcTimerMsgPeriodicNsExp(timerIndex);
            break;
        case IP_ND_SERVER_TIMER_FIRST_NS:
            IP_NDSERVER_ProcTimerMsgFirstNsExp(timerIndex);
            break;
        case IP_ND_SERVER_TIMER_RA:
            IP_NDSERVER_ProcTimerMsgRaExp(timerIndex);
            break;
        default:
            IPND_WARNING_LOG(NDIS_NDSERVER_PID, "IP_NDSERVER_TimerMsgDistr: Illegal Timer Type!");
            break;
    }

    return IP_MSG_HANDLED;
}



