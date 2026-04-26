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

#ifndef __DMS_TASK_H__
#define __DMS_TASK_H__

#include "vos.h"
#include "dms_port_def.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

/* Log Print Module Define */
#ifndef THIS_MODU
#define THIS_MODU ps_nas
#endif

#if (FEATURE_ACORE_MODULE_TO_CCORE == FEATURE_ON)
#define DMS_APP_DS_TASK_PRIORITY    VOS_PRIORITY_P3
#else
#define DMS_APP_DS_TASK_PRIORITY    84
#endif
#define DMS_EVENT_BASE              (0x00000000)
#define DMS_EVENT_READY_CB_IND      (DMS_EVENT_BASE | 0x0001)

#define DMS_DEFAULT_ERR_RPT_TIME    60000  /* 端口Error info定时上报到HIDS的周期 60s */

/* DMS消息ID */
enum DMS_MsgId {
    ID_DMS_MNTN_MSG_BASE        = 0x0000,
    /* 管脚消息信号ID  START */
    /* DMS->DMS */
    /* _MSGPARSE_Interception DMS_ELF_MNTN_MSC_STRU */
    ID_DMS_MNTN_INPUT_MSC       = ID_DMS_MNTN_MSG_BASE,
    /* _MSGPARSE_Interception DMS_ELF_MNTN_MSC_STRU */
    ID_DMS_MNTN_OUTPUT_MSC      = ID_DMS_MNTN_MSG_BASE + 1,
    /* _MSGPARSE_Interception DMS_ELF_MNTN_MSG_STRU */
    ID_DMS_MNTN_START_FLOW_CTRL = ID_DMS_MNTN_MSG_BASE + 2,
    /* _MSGPARSE_Interception DMS_ELF_MNTN_MSG_STRU */
    ID_DMS_MNTN_STOP_FLOW_CTRL  = ID_DMS_MNTN_MSG_BASE + 3,
    /* 管脚消息信号ID  END */
    /* DMS维测上报信号ID */
    /* _MSGPARSE_Interception DMS_ELF_MNTN_RPT_INFO_STRU */
    ID_DMS_MNTN_RPT_INFO        = ID_DMS_MNTN_MSG_BASE + 4,
    /* BASE->BASE */
    ID_DMS_MSG_BASE             = 0x1000,
    /* DMS->DMS */
    /* _MSGPARSE_Interception DMS_ELF_MNTN_MSC_STRU */
    ID_DMS_MSC_IND              = ID_DMS_MSG_BASE,
    /* _MSGPARSE_Interception DMS_ELF_INTRA_EVENT_STRU */
    ID_DMS_INTRA_PORT_EVENT_IND = ID_DMS_MSG_BASE + 1,

    ID_DMS_MSG_BUTT,
};
typedef VOS_UINT32 DMS_IntraMsgIdUint32;

enum DMS_TIMER_Id {
    /* DMS->TIMER */
    /* _MSGPARSE_Interception DMS_ELF_TIMER_OPER_STRU */
    TI_DMS_TIMER_MNTN_INFO         = 0x00,     /* DMS维测定时ID */

    TI_DMS_TIMER_BUTT
};
typedef VOS_UINT32 DMS_TIMER_IdUint32;

enum DMS_TIMER_OperType {
    DMS_TIMER_OPERATION_START,
    DMS_TIMER_OPERATION_STOP
};
typedef VOS_UINT32 DMS_TIMER_OperTypeUint32;

struct DMS_TIMER_Operation {
    VOS_MSG_HEADER
    DMS_TIMER_IdUint32          msgId;      /* 消息类型 */
    VOS_UINT32                  timeLen;
    DMS_TIMER_OperTypeUint32    timeAction;
};

/* DMS任务上下文 */
struct DMS_CtxInfo {
    HTIMER                      hTimer[TI_DMS_TIMER_BUTT];            /* DMS的定时器 */
    VOS_UINT32                  dmsTaskId;                            /* DMS任务ID */
    VOS_UINT32                  reserv;
};

struct DMS_IntraEvent {
    VOS_MSG_HEADER
    DMS_IntraMsgIdUint32        msgId;      /* 消息类型 */

    DMS_SubscripEventIdUint32   eventId;
    DMS_PortIdUint16            portId;
    VOS_UINT16                  reserv;

    VOS_UINT32                  len;
    VOS_UINT8                   data[0];
};

extern VOS_UINT32 DMS_StartTimer(VOS_UINT32 length, DMS_TIMER_IdUint32 timerId);
VOS_VOID DMS_SendIntraEvent(DMS_SubscripEventIdUint32 eventId, DMS_PortIdUint16 portId, const VOS_VOID *data, VOS_UINT32 len);
VOS_UINT32 DMS_GetTaskId(VOS_VOID);

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /*__DMS_TASK_H__ */
