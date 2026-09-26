/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
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

#include "vos.h"
#include "taf_type_def.h"
#include "mn_comm_api.h"
#include "dms_debug.h"
#include "dms_port.h"

#include "securec.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_DMS_PORT_EVENT_C

struct DMS_PORT_SubscripEventInfo g_subscripEventInfo[ID_DMS_EVENT_BUTT];
struct DMS_PORT_ConnectEventInfo  g_connectEventInfo;

struct DMS_PORT_ConnectEventInfo *DMS_PORT_GetConnectEventInfo(VOS_VOID)
{
    return &(g_connectEventInfo);
}

STATIC HI_LIST_S *DMS_PORT_GetSubscriperList(DMS_SubscripEventIdUint32 eventId)
{
    return &(g_subscripEventInfo[eventId].subscriperList);
}

VOS_VOID DMS_PORT_InitConnectEventInfo(VOS_VOID)
{
    struct DMS_PORT_ConnectEventInfo *eventInfo = VOS_NULL_PTR;

    eventInfo = DMS_PORT_GetConnectEventInfo();
    HI_INIT_LIST_HEAD(&eventInfo->list);

    /* 锁初始化 */
    VOS_SpinLockInit(&(eventInfo->lock));
}

VOS_VOID DMS_PORT_InitSubscripEventInfo(VOS_VOID)
{
    HI_LIST_S  *head = VOS_NULL_PTR;
    VOS_UINT32  i;

    for (i = 0; i < ID_DMS_EVENT_BUTT; i++ ) {
        head = DMS_PORT_GetSubscriperList(i);
        /*lint -e717*/
        HI_INIT_LIST_HEAD(head);
    }
}

/*lint -e429*/

VOS_VOID DMS_PORT_SubscripEvent(DMS_SubscripEventIdUint32 eventId, VOS_UINT32 pid)
{
    struct DMS_PORT_Subscriber   *subscriber = VOS_NULL_PTR;
    HI_LIST_S                    *head       = VOS_NULL_PTR;

    if (eventId >= ID_DMS_EVENT_BUTT) {
        DMS_ERROR_LOG1("DMS_PORT_SubscripEvent:para invalid.", eventId);
        return;
    }

    if (VOS_CheckPSPidValidity(pid) != VOS_TRUE) {
        DMS_ERROR_LOG1("DMS_PORT_SubscripEvent:pid invalid.", pid);
        return;
    }

    /* 申请节点、缓存消息内存 */
    subscriber = (struct DMS_PORT_Subscriber *)PS_MEM_ALLOC(PS_PID_DMS, sizeof(struct DMS_PORT_Subscriber));

    if (subscriber == VOS_NULL_PTR) {
        DMS_ERROR_LOG("DMS_PORT_SubscripEvent:malloc subscriber fail.");
        return;
    }

    /* 初始化节点 */
    (VOS_VOID)memset_s(subscriber, sizeof(struct DMS_PORT_Subscriber), 0x00, sizeof(struct DMS_PORT_Subscriber));

    subscriber->pid = pid;

    head = DMS_PORT_GetSubscriperList(eventId);

    msp_list_add_tail(&subscriber->list, head);
}
/*lint +e429*/

VOS_VOID DMS_PORT_DesubscripEvent(DMS_SubscripEventIdUint32 eventId, VOS_UINT32 pid)
{
    struct DMS_PORT_Subscriber  *subscriber = VOS_NULL_PTR;
    HI_LIST_S                   *me   = VOS_NULL_PTR;
    HI_LIST_S                   *tmp  = VOS_NULL_PTR;
    HI_LIST_S                   *head = VOS_NULL_PTR;

    if (eventId >= ID_DMS_EVENT_BUTT) {
        DMS_ERROR_LOG1("DMS_PORT_DesubscripEvent:eventId invalid.", eventId);
        return;
    }

    if (VOS_CheckPSPidValidity(pid) != VOS_TRUE) {
        DMS_ERROR_LOG1("DMS_PORT_DesubscripEvent:pid invalid.", pid);
        return;
    }

    head = DMS_PORT_GetSubscriperList(eventId);

    msp_list_for_each_safe(me, tmp, head) {
        subscriber  = msp_list_entry(me, struct DMS_PORT_Subscriber, list);

        if (subscriber->pid == pid) {
            /* 从缓存队列中删除节点 */
            msp_list_del(&subscriber->list);

            PS_MEM_FREE(PS_PID_DMS, subscriber);
            subscriber = VOS_NULL_PTR;
        }
    }
}

STATIC VOS_VOID DMS_PORT_SendSubscripEvent(DMS_SubscripEventIdUint32 eventId, VOS_UINT16 clientId,
    VOS_UINT32 pid, const VOS_VOID *data, VOS_UINT32 len)
{
    struct DMS_PORT_SubscripEventNotify   *event = VOS_NULL_PTR;
    errno_t                                memResult;

    if ((data == VOS_NULL_PTR) && (len != 0)) {
        DMS_ERROR_LOG("DMS_PORT_SendSubscripEvent:para invalid.");
        return;
    }

    /* 申请消息内存 */
    event = (struct DMS_PORT_SubscripEventNotify *)TAF_AllocMsgWithHeaderLen(PS_PID_DMS,
                                                 sizeof(struct DMS_PORT_SubscripEventNotify) + len);

    /* 内存申请失败，返回 */
    if (event == VOS_NULL_PTR) {
        DMS_ERROR_LOG("Alloc DMS_PORT_SendSubscripEvent msg fail");
        return;
    }

    TAF_CfgMsgHdr((MsgBlock *)event, PS_PID_DMS, pid,
                  sizeof(struct DMS_PORT_SubscripEventNotify) + len - VOS_MSG_HEAD_LENGTH);

    event->msgId    = ID_DMS_PORT_SUBSCRIP_EVENT_NOTIFY;
    event->eventId  = eventId;
    event->clientId = clientId;

    if ((data != VOS_NULL_PTR) && (len > 0)) {
        memResult = memcpy_s(event->data, len, data, len);
        TAF_MEM_CHK_RTN_VAL(memResult, len, len);
    }

    /* 发送消息 */
    (VOS_VOID)TAF_TraceAndSendMsg(PS_PID_DMS, event);
}

VOS_VOID DMS_ProcIntraPortEvent(const MsgBlock *msg)
{
    struct DMS_IntraEvent          *intraEvent  = VOS_NULL_PTR;
    struct DMS_PORT_Subscriber     *subscriber  = VOS_NULL_PTR;
    HI_LIST_S                      *head        = VOS_NULL_PTR;
    HI_LIST_S                      *me          = VOS_NULL_PTR;
    struct DMS_ClientInfo          *clientInfo  = VOS_NULL_PTR;
    DMS_SubscripEventIdUint32       eventId;

    intraEvent = (struct DMS_IntraEvent *)msg;
    clientInfo = DMS_PORT_GetClientInfo(intraEvent->portId);

    eventId    = intraEvent->eventId;
    head = DMS_PORT_GetSubscriperList(eventId);

    /* 遍历链表, 匹配EVENT事件 */
    msp_list_for_each(me, head) {
        subscriber = msp_list_entry(me, struct DMS_PORT_Subscriber, list);

        DMS_PORT_SendSubscripEvent(eventId, clientInfo->id, subscriber->pid,
                              intraEvent->data, intraEvent->len);
    }
}


