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

#include "vos.h"
#include "securec.h"

#include "at_external_module_msg_proc.h"
#include "at_mt_interface.h"
#include "at_cmd_msg_proc.h"
#include "at_mt_msg_proc.h"


/*
 * 协议栈打印打点方式下的.C文件宏定义
 */
#define THIS_FILE_ID PS_FILE_ID_AT_PROC_BBIC_MAIN_C

#if (FEATURE_UE_MODE_NR == FEATURE_ON)

STATIC AT_MsgProcEntry g_atProcBbicMsgTbl[] = {
    /* 消息ID */                       /* AT模块命令状态 */               /* 消息处理函数 */
    { ID_BBIC_CAL_AT_MT_RX_CNF,        AT_CMD_SET_FRXON,                  VOS_NULL_PTR },
};

STATIC CONST AT_MsgProcEntryTblInfo g_atProcBbicMsgTblInfo = {
    .tbl      = g_atProcBbicMsgTbl,
    .entryNum = AT_ARRAY_SIZE(g_atProcBbicMsgTbl),
    .entryLen = sizeof(g_atProcBbicMsgTbl[0])
};

STATIC VOS_UINT32 AT_ProcGetBbicMsgId(CONST VOS_VOID *msg)
{
    AT_MT_MsgHeader *bbicMsg = (AT_MT_MsgHeader *)msg;
    return (VOS_UINT32)(bbicMsg->usMsgId);
}

STATIC VOS_UINT32 AT_ProcGetBbicUserId(CONST VOS_VOID *msg, VOS_UINT8 *indexNum)
{
    AT_MT_Info      *atMtInfoCtx = AT_GetMtInfoCtx();

    if (atMtInfoCtx->atInfo.indexNum >= AT_MAX_CLIENT_NUM) {
        AT_ERR_LOG("AT_ProcGetBbicUserId: AT INDEX ILLEGALS!");
        return VOS_ERR;
    }
    *indexNum = atMtInfoCtx->atInfo.indexNum;

    return VOS_OK;
}

STATIC CONST AT_ModuleMsgProcInfo g_atBbicMsgProcInfoTbl = {
    .moduleId   = AT_PROC_MODULE_BBIC,
    .tblInfo    = &g_atProcBbicMsgTblInfo,
    .GetMsgId   = AT_ProcGetBbicMsgId,
    .GetUserId  = AT_ProcGetBbicUserId,
};

VOS_VOID AT_InitBbicMsgProcTbl(VOS_VOID)
{
    VOS_UINT32 ret = AT_RegModuleMsgProcInfo(&g_atBbicMsgProcInfoTbl);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_InitBbicMsgProcTbl: register fail");
    }
}

/*
 * 功能描述: 来自BBIC模块的消息处理函数
 */
VOS_VOID AT_ProcBbicMsg1(struct MsgCB *msg)
{
    VOS_UINT32 ret = AT_ProcExternalModuleMsg(msg, AT_PROC_MODULE_BBIC);
    if (ret != VOS_OK) {
        AT_ERR_LOG("AT_ProcBbicMsg: fail");
    }
}

#endif

