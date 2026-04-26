/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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
 */

#ifndef _AT_EXTERNAL_MODULE_MSG_PROC_H_
#define _AT_EXTERNAL_MODULE_MSG_PROC_H_

#include "vos.h"
#include "at_cmd_proc.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 4)

#define AT_CMD_OPT_NO_NEED_CHECK AT_CMD_CURRENT_OPT_BUTT
#define AT_CMD_NO_NEED_FORMAT_RSLT AT_RRETURN_CODE_BUTT

/* AT接收的消息处理函数指针 */
typedef VOS_UINT32 (*AT_MsgProcFunc)(VOS_VOID *msg, VOS_UINT8 indexNum);

enum AT_ProcModuleType {
    AT_PROC_MODULE_MTA,
    AT_PROC_MODULE_BBIC,
    AT_PROC_MODULE_MMA,
    AT_PROC_MODULE_CCM,
    AT_PROC_MODULE_IMSA,
    AT_PROC_MODULE_THROT,

    AT_PROC_MODULE_BUTT,
};
typedef VOS_UINT32 AT_ProcModuleTypeUint32;

/* AT处理消息列表头 */
typedef struct {
    VOS_UINT32          msgId;
    AT_CmdCurOptUint32  opt;
} AT_MsgProcEntryHead;

typedef struct {
    VOS_UINT32          msgId;
    AT_CmdCurOptUint32  opt;
    AT_MsgProcFunc      func;
} AT_MsgProcEntry;

/* AT处理消息列表信息 */
typedef struct {
    VOS_VOID           *tbl;         /* 列表地址 */
    VOS_UINT32          entryNum;    /* 列表条目个数 */
    VOS_UINT32          entryLen;    /* 列表条目结构长度 */
} AT_MsgProcEntryTblInfo;

/*
 * 该结构用于：使用通用消息处理模板处理消息时所需的必要信息
 * *tblInfo ： 消息处理表
 * moduleId ： 模块ID号
 * *GetMsgId： 获取来自各个模块消息的消息ID函数，
 *             由于每个模块的消息结构可能不一致，所以需要每个模块单独提供消息ID获取函数
 *             注意：当模块没有提供自己的GetMsgId函数时，使用通用函数AT_GetMsgId
 * *GetUserId：把来自各个模块消息中的clientid转换为UserId，
 *             由于每个模块的消息结构可能不一致，所以需要每个模块单独提供UserID获取函数
 *             注意：当前没有提供缺省函数
 */
typedef struct {
    AT_ProcModuleTypeUint32       moduleId;
    VOS_UINT32                    reserved; /* 8字节对齐 */
    CONST AT_MsgProcEntryTblInfo *tblInfo;
    VOS_UINT32 (*GetMsgId)(CONST VOS_VOID *msg);
    VOS_UINT32 (*GetUserId)(CONST VOS_VOID *msg, VOS_UINT8 *indexNum);
} AT_ModuleMsgProcInfo;

VOS_UINT32 AT_RegModuleMsgProcInfo(CONST AT_ModuleMsgProcInfo *procInfo);
VOS_UINT32 AT_ProcExternalModuleMsg(VOS_VOID *msg, AT_ProcModuleTypeUint32 moduleId);

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif
