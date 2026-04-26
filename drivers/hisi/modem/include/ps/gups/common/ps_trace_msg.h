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

#ifndef __PS_TRACE_MSG_H__
#define __PS_TRACE_MSG_H__

#include "vos.h"
#include "PsLogdef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#pragma pack(push, 4)

typedef struct {
    const char *logFmt;
    VOS_PID pid;
    PS_FILE_ID_DEFINE_ENUM_UINT32 fileId;
    VOS_INT32 line;
} PS_MSG_Mntn;

/*
 * 层间消息过滤函数指针，此回调函数实现必须满足如下规则:
 * 1. 返回值与入参相同，对输入消息不进行脱敏，勾取原始输入消息
 * 2. 返回值为空，对输入消息进行屏蔽处理，不再勾取出来
 * 3. 返回值与入参不同，对输入消息进行替换处理，将返回内存中的内容勾取出来，
 * 返回的内存消息必须使用VOS_MemAlloc进行动态申请(ucPtNo取DYNAMIC_MEM_PT)，勾包完成之后统一释放
 */
#if (VOS_OSA_CPU == OSA_CPU_ACPU)
typedef VOS_VOID* (*PsMsgFilter)(MsgBlock *msg);
#else
typedef MsgBlock* (*PsMsgFilter)(const MsgBlock* msg);
#endif
/* filter为空时，对原始消息进行勾包；filter不为空时，filter进行脱敏处理后勾包 */
VOS_UINT32 PS_FilterAndSendMsg(const VOS_VOID *msg, const PS_MSG_Mntn *mntn, PsMsgFilter filter);

/* filter为空时，对原始消息进行勾包；filter不为空时，filter进行脱敏处理后勾包 */
VOS_UINT32 PS_FilterAndSendUrgentMsg(const VOS_VOID *msg, const PS_MSG_Mntn *mntn, PsMsgFilter filter);

static inline VOS_UINT32 PS_FilterMsgWrapper(VOS_PID pid, const VOS_VOID *msg, PsMsgFilter filter,
    PS_FILE_ID_DEFINE_ENUM_UINT32 fileId, const char *logFmt)
{
    PS_MSG_Mntn tmpMntn = {logFmt, pid, fileId, __LINE__};
    return PS_FilterAndSendMsg(msg, &tmpMntn, filter); /*lint !e603*/
}

static inline VOS_UINT32 PS_FilterUrgentMsgWrapper(VOS_PID pid, const VOS_VOID *msg, PsMsgFilter filter,
    PS_FILE_ID_DEFINE_ENUM_UINT32 fileId, const char *logFmt)
{
    PS_MSG_Mntn tmpMntn = {logFmt, pid, fileId, __LINE__};
    return PS_FilterAndSendUrgentMsg(msg, &tmpMntn, filter);
}

#define PS_SEND_MSG_FAIL_LOG  TTF_FILE_NAME(THIS_FILE_ID) " send msg result=0x%x"

#define PS_FILTER_AND_SND_MSG(pid, msg, filter) \
    PS_FilterMsgWrapper(pid, msg, filter, THIS_FILE_ID, PS_SEND_MSG_FAIL_LOG)

#define PS_FILTER_AND_SND_URGENT_MSG(pid, msg, filter) \
    PS_FilterUrgentMsgWrapper(pid, msg, filter, THIS_FILE_ID, PS_SEND_MSG_FAIL_LOG)

#define PS_TRACE_AND_SND_MSG(pid, msg) \
    PS_FilterMsgWrapper(pid, msg, VOS_NULL_PTR, THIS_FILE_ID, PS_SEND_MSG_FAIL_LOG)

#pragma pack(pop)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif
