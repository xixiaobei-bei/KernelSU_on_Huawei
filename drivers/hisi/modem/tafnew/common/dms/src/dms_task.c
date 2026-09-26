/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
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

#include "dms_task.h"
#include "mn_comm_api.h"
#include "dms_debug.h"
#include "dms_port.h"

#include "securec.h"
#include "dms_msg_chk.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_TASK_C

extern VOS_MsgHookFunc vos_MsgHook;
extern VOS_VOID   At_MsgProc(MsgBlock *msg);
extern VOS_UINT32 At_PidInit(enum VOS_InitPhaseDefine phase);

struct DMS_CtxInfo g_dmsCtxInfo = {{0}};

STATIC struct DMS_CtxInfo *DMS_GetDmsCtxInfo(VOS_VOID)
{
    return &g_dmsCtxInfo;
}

VOS_UINT32 DMS_GetTaskId(VOS_VOID)
{
    return g_dmsCtxInfo.dmsTaskId;
}

STATIC VOS_VOID DMS_SetTaskId(VOS_UINT32 taskId)
{
    g_dmsCtxInfo.dmsTaskId = taskId;
}

STATIC HTIMER *DMS_GetDmsTimer(DMS_TIMER_IdUint32 timerId)
{
    HTIMER             *hTimer = VOS_NULL_PTR;
    struct DMS_CtxInfo *dmsCtxInfo = VOS_NULL_PTR;

    /* 不在使用的定时器范围内 */
    if (timerId >= TI_DMS_TIMER_BUTT) {
        DMS_ERROR_LOG1("DMS_GetDmsTimer: timerId %d is invalid!", timerId);
        return VOS_NULL_PTR;
    }

    dmsCtxInfo = DMS_GetDmsCtxInfo();
    hTimer = &dmsCtxInfo->hTimer[timerId];

    return hTimer;
}

STATIC VOS_UINT32 DMS_StartRelTimer(HTIMER *phTm, VOS_UINT32 length, DMS_TIMER_IdUint32 timerId,
    VOS_UINT32 param, VOS_UINT8 mode)
{
    VOS_UINT32                 ret;
    struct DMS_TIMER_Operation timer;

    if (length == 0) {
        DMS_ERROR_LOG("DMS_StartRelTimer: Start Rel Timer Length is 0!");
        return VOS_ERR;
    }

    if (length >= VOS_TIMER_MAX_LENGTH) {
        length = VOS_TIMER_MAX_LENGTH - 1;
    }

    ret = VOS_StartRelTimer(phTm, PS_PID_DMS, length, timerId, param, mode, VOS_TIMER_NO_PRECISION);
    if (ret != VOS_OK) {
        DMS_ERROR_LOG("DMS_StartRelTimer: Start Rel Timer Fail!");
        return VOS_ERR;
    }

    /* 填写消息头 */
    TAF_CfgMsgHdr((MsgBlock *)(&timer), PS_PID_DMS, VOS_PID_TIMER, sizeof(timer) - VOS_MSG_HEAD_LENGTH);

    timer.msgId      = timerId;
    timer.timeAction = DMS_TIMER_OPERATION_START;
    timer.timeLen    = length;

    mdrv_diag_trace_report(&timer);

    return ret;
}

VOS_UINT32 DMS_StartTimer(VOS_UINT32 length, DMS_TIMER_IdUint32 timerId)
{
    HTIMER *hTimer = VOS_NULL_PTR;

    hTimer = DMS_GetDmsTimer(timerId);
    if (hTimer == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_StartTimer:ERROR:hTimer NULL");
        return VOS_ERR;
    }

    VOS_StopRelTimer(hTimer);
    if (DMS_StartRelTimer(hTimer, length, timerId, 0, VOS_RELTIMER_LOOP) != VOS_OK) {
        DMS_ERROR_LOG("DMS_StartTimer:ERROR:Start Timer");
        return VOS_ERR;
    }

    return VOS_OK;
}

STATIC VOS_UINT32 DMS_PidInit(enum VOS_InitPhaseDefine phase)
{
    VOS_UINT32      i;
    /*
     * 如下三个函数调用顺序:
     * AT_InitPort ->
     * DMS_PORT_Init ->
     * AT_HSUART_ConfigDefaultPara,
     * 主要原因为：at需要先把回调注册给DMS，dms才能把回调注册给底软，
     * 否则，底软检测到接收指针存在时，就会发送数据，数据有可能在DMS被丢弃；
     *
     * 当前这三个函数在At_PidInit函数依次执行，此处不再做初始化处理
     **/

    struct DMS_CtxInfo *dmsCtxInfo = VOS_NULL_PTR;

    dmsCtxInfo = DMS_GetDmsCtxInfo();

    for (i = 0; i < TI_DMS_TIMER_BUTT; i++) {
        dmsCtxInfo->hTimer[i] = VOS_NULL_PTR;
    }

    DMS_SetTaskId(VOS_NULL_LONG);

    DMS_RegisterDumpCallBack();

    return VOS_OK;
}

STATIC VOS_VOID DMS_RcvTimerMsg(MsgBlock *msg)
{
    REL_TimerMsgBlock   *timerMsg = VOS_NULL_PTR;
    struct DMS_MntnInfo *dmsMntnInfo = VOS_NULL_PTR;

    timerMsg = (REL_TimerMsgBlock *)msg;
    dmsMntnInfo = DMS_MNTN_GetDmsMntnInfo();
    dmsMntnInfo->version = DMS_MNTN_STATS_VERSION;
#if (FEATURE_VCOM_EXT == FEATURE_ON)
    dmsMntnInfo->vcomExtFlag = 1;
#else
    dmsMntnInfo->vcomExtFlag = 0;
#endif

    switch (timerMsg->name) {
        /* ERROR INFO定时上报到HiDS */
        case TI_DMS_TIMER_MNTN_INFO:
            DMS_MNTN_TraceReport(0, ID_DMS_MNTN_RPT_INFO, (VOS_UINT8 *)dmsMntnInfo,
                                sizeof(*dmsMntnInfo));
            return;

        default:
            DMS_ERROR_LOG1("DMS_RcvTimerMsg: rcv error msg id ", timerMsg->name);
            return;
    }
}

STATIC VOS_VOID DMS_ProcDmsMscInd(const MsgBlock *msg)
{
    struct DMS_DMS_MscInd *mscIndMsg = VOS_NULL_PTR;

    mscIndMsg= (struct DMS_DMS_MscInd *)msg;

    if (mscIndMsg->msc.dtrFlag == VOS_TRUE && mscIndMsg->msc.dtr == 1) {
        DMS_PORT_ProcDtrAsserted(mscIndMsg->portId);
        return;
    }

    if (mscIndMsg->msc.dtrFlag == VOS_TRUE && mscIndMsg->msc.dtr == 0) {
        DMS_PORT_ProcDtrDeasserted(mscIndMsg->portId);
        return;
    }
}

STATIC VOS_VOID DMS_RcvDmsMsg(MsgBlock *msg)
{
    MSG_Header  *msgHeader = VOS_NULL_PTR;

    msgHeader = (MSG_Header *)msg;

    switch (msgHeader->msgName) {
        case ID_DMS_MSC_IND:
            DMS_ProcDmsMscInd(msg);
            break;

        case ID_DMS_INTRA_PORT_EVENT_IND:
            DMS_ProcIntraPortEvent(msg);
            break;

        default:
            DMS_ERROR_LOG1("DMS_RcvDmsMsg: rcv error msg id ", msgHeader->msgName);
            break;
    }
}

STATIC VOS_VOID DMS_MsgProc(MsgBlock *msg)
{
    VOS_UINT32       sendPid;
    VOS_UINT32       receiverPid;

    if (msg == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_MsgProc msg is NULL");
        return;
    }

    if (DMS_ChkDmsFidRcvMsgLen(msg) != VOS_TRUE) {
        DMS_ERROR_LOG("DMS_MsgProc: message length is invalid!");
        return;
    }

    sendPid     = VOS_GET_SENDER_ID(msg);
    receiverPid = VOS_GET_RECEIVER_ID(msg);

    /* 不是发给DMS的则直接返回 */
    if (receiverPid != PS_PID_DMS) {
        DMS_ERROR_LOG1("DMS_MsgProc Pid invalid receiverPid %d", receiverPid);
        return;
    }

    /* 消息的分发处理 */
    switch (sendPid) {
        /* 来自Timer的消息 */
        case VOS_PID_TIMER:
            DMS_RcvTimerMsg(msg);
            return;

        case PS_PID_DMS:
            DMS_RcvDmsMsg(msg);
            return;

        default:
            DMS_ERROR_LOG1("DMS_MsgProc: rcv error pid msg %d\r\n", sendPid);
            return;
    }
}

STATIC VOS_VOID DMS_ConnectEventProc(VOS_VOID)
{
    struct DMS_PORT_ConnectEventNode *node = VOS_NULL_PTR;
    struct DMS_PORT_ConnectEventInfo *eventInfo = VOS_NULL_PTR;
    HI_LIST_S *me = VOS_NULL_PTR;
    HI_LIST_S *tmp = VOS_NULL_PTR;
    HI_LIST_S  head = HI_LIST_HEAD_INIT(head);
    VOS_ULONG  lockLevel = 0;

    eventInfo = DMS_PORT_GetConnectEventInfo();

    VOS_SpinLockIntLock(&(eventInfo->lock), lockLevel);
    msp_list_splice_tail_init(&eventInfo->list, &head);
    VOS_SpinUnlockIntUnlock(&(eventInfo->lock), lockLevel);

    /* 循环遍历事件表，并释放事件表的所有节点 */
    msp_list_for_each_safe(me, tmp, &head) {
        node = msp_list_entry(me, struct DMS_PORT_ConnectEventNode, list);
        msp_list_del(&node->list);

        /* 收到设备连接状态通知，打开/关闭端口 */
        DMS_PORT_ProcConnectEvent(node->portId, node->connState);

        /* 释放节点内存 */
        VOS_MemFree(PS_PID_DMS, node);
    }
}

STATIC VOS_VOID DMS_FidEventProc(VOS_UINT32 event)
{
    if (event & DMS_EVENT_READY_CB_IND) {
        DMS_ConnectEventProc();
    }
}

STATIC VOS_VOID DMS_FidMsgProc(VOS_VOID)
{
    MsgBlock   *msg = VOS_NULL_PTR;
    VOS_UINT32  receiverPid;

    /* 消息处理 */
    msg = (MsgBlock *)VOS_GetMsg(DMS_GetTaskId());
    if (msg == VOS_NULL_PTR) {
        return;
    }

    if (vos_MsgHook != VOS_NULL_PTR) {
        (VOS_VOID)(vos_MsgHook)((VOS_VOID *)(msg));
    }
    receiverPid = VOS_GET_RECEIVER_ID(msg);
    /* 消息的分发处理 */
    switch (receiverPid) {
        /* 发送给AT的消息 */
        case WUEPS_PID_AT:
            At_MsgProc(msg);
            break;

        case PS_PID_DMS:
            DMS_MsgProc(msg);
            break;

        default:
            break;
    }

    PS_FREE_MSG(receiverPid, msg);
}

STATIC VOS_VOID DMS_FidEventDispatch(VOS_UINT32 event)
{
    /* 事件处理 */
    if (event != VOS_MSG_SYNC_EVENT) {
        DMS_FidEventProc(event);
        return;
    }

    /* 消息处理 */
    DMS_FidMsgProc();
}

STATIC VOS_VOID DMS_FidTask(VOS_UINT32 queueID, VOS_UINT32 fidValue, VOS_UINT32 para1, VOS_UINT32 para2)
{
    VOS_UINT32 taskID;
    VOS_UINT32 expectEvent;
    VOS_UINT32 eventMask;
    VOS_UINT32 event = 0;
    VOS_UINT32 ret;

    taskID = VOS_GetCurrentTaskID();
    if (taskID == VOS_NULL_LONG) {
        DMS_ERROR_LOG("DMS_FidTask: ERROR, TaskID is invalid.");
        return;
    }

    if (VOS_CreateEvent(taskID) != VOS_OK) {
        DMS_ERROR_LOG("DMS_FidTask: ERROR, create event fail.");
        return;
    }

    DMS_SetTaskId(taskID);
    (VOS_VOID)VOS_EventWrite(taskID, DMS_EVENT_READY_CB_IND);

    expectEvent = DMS_EVENT_READY_CB_IND | VOS_MSG_SYNC_EVENT;
    eventMask   = VOS_EVENT_ANY | VOS_EVENT_WAIT;

    for (;;) {
        ret = VOS_EventRead(expectEvent, eventMask, 0, &event);
        if (ret != VOS_OK) {
            DMS_ERROR_LOG("DMS_FidTask: ERROR, read event error.");
            continue;
        }

        DMS_FidEventDispatch(event);
    }
}

MODULE_EXPORTED VOS_UINT32 DMS_FidInit(enum VOS_InitPhaseDefine ip)
{
    VOS_UINT32 relVal = 0;

    switch (ip) {
        case VOS_IP_LOAD_CONFIG:

            relVal = VOS_RegisterPIDInfo(WUEPS_PID_AT, (InitFunType)At_PidInit, (MsgFunType)At_MsgProc);
            if (relVal != VOS_OK) {
                return VOS_ERR;
            }

            relVal = VOS_RegisterPIDInfo(PS_PID_DMS, (InitFunType)DMS_PidInit, (MsgFunType)DMS_MsgProc);
            if (relVal != VOS_OK) {
                return VOS_ERR;
            }

            /* 注册TASK Entry */
            relVal = VOS_RegisterMsgTaskEntry(PS_FID_DMS, DMS_FidTask);
            if (relVal != VOS_OK) {
                return VOS_ERR;
            }

            relVal = VOS_RegisterTaskPrio(PS_FID_DMS, DMS_APP_DS_TASK_PRIORITY);
            if (relVal != VOS_OK) {
                return VOS_ERR;
            }

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
            if (VOS_RegisterFidTaskCoreBind(PS_FID_DMS, VOS_CORE_MASK_CORE0) != VOS_OK) {
                return VOS_ERR;
            }
#endif

            DMS_SortDmsFidChkMsgLenTbl();
            break;

        default:
            break;
    }

    return VOS_OK;
}

VOS_VOID DMS_SendDmsMscInd(DMS_PortIdUint16 portId, struct DMS_MscInfo *msc)
{
    struct DMS_DMS_MscInd *mscIndMsg = VOS_NULL_PTR;
    errno_t                memResult;

    /* 申请消息内存 */
    mscIndMsg = (struct DMS_DMS_MscInd *)TAF_AllocMsgWithHeaderLen(PS_PID_DMS, sizeof(struct DMS_DMS_MscInd));

    /* 内存申请失败，返回 */
    if (mscIndMsg == VOS_NULL_PTR) {
        DMS_ERROR_LOG("Alloc DMS_DMS_MscInd msg fail");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)mscIndMsg, PS_PID_DMS, PS_PID_DMS, sizeof(struct DMS_DMS_MscInd) - VOS_MSG_HEAD_LENGTH);

    mscIndMsg->msgId  = ID_DMS_MSC_IND;
    mscIndMsg->portId = portId;

    memResult = memcpy_s(&(mscIndMsg->msc), sizeof(mscIndMsg->msc), msc, sizeof(struct DMS_MscInfo));
    TAF_MEM_CHK_RTN_VAL_CONTINUE(memResult, sizeof(mscIndMsg->msc), sizeof(struct DMS_MscInfo));

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(PS_PID_DMS, mscIndMsg);
}

VOS_VOID DMS_SendIntraEvent(DMS_SubscripEventIdUint32 eventId, DMS_PortIdUint16 portId, const VOS_VOID *data, VOS_UINT32 len)
{
    struct DMS_IntraEvent *intraEvent = VOS_NULL_PTR;
    errno_t                memResult;

    if ((data == VOS_NULL_PTR) && (len != 0)) {
        DMS_ERROR_LOG("DMS_SendIntraEvent:para invalid.");
        return;
    }

    /* 申请消息内存 */
    intraEvent = (struct DMS_IntraEvent *)TAF_AllocMsgWithHeaderLen(PS_PID_DMS, sizeof(struct DMS_IntraEvent) + len);

    /* 内存申请失败，返回 */
    if (intraEvent == VOS_NULL_PTR) {
        DMS_ERROR_LOG("Alloc DMS_SendIntraEvent msg fail");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)intraEvent, PS_PID_DMS, PS_PID_DMS,
        sizeof(struct DMS_IntraEvent) + len - VOS_MSG_HEAD_LENGTH);

    intraEvent->msgId   = ID_DMS_INTRA_PORT_EVENT_IND;
    intraEvent->eventId = eventId;
    intraEvent->portId  = portId;

    if ((data != VOS_NULL_PTR) && (len > 0)) {
        intraEvent->len = len;
        memResult = memcpy_s(intraEvent->data, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(PS_PID_DMS, intraEvent);
}


