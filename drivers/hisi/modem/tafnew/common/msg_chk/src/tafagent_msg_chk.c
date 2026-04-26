/*
 * Copyright (C) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved.
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

#include "taf_type_def.h"
#include "taf_msg_chk_api.h"
#include "taf_ccm_api.h"
#include "taf_agent_interface.h"


#define THIS_FILE_ID PS_FILE_ID_TAFAGENT_MSG_CHK_C

STATIC VOS_UINT32 TAF_AGENT_ChkMtaReadAcoreNvCnfMsgLen(const MSG_Header *msgHeader);
STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkMtaMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 TAF_AGENT_GetChkMtaMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkCcmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 TAF_AGENT_GetChkCcmMsgLenTblSize(VOS_VOID);
STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkDsmMsgLenTblAddr(VOS_VOID);
STATIC VOS_UINT32 TAF_AGENT_GetChkDsmMsgLenTblSize(VOS_VOID);

static TAF_ChkMsgLenNameMapTbl g_acpuTafAgentChkCcmMsgLenTbl[] = {
    { ID_TAF_CCM_QRY_CALL_INFO_CNF, sizeof(TAFAGENT_CCM_QryCallInfoCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_acpuTafAgentChkDsmMsgLenTbl[] = {
    { ID_TAFAGENT_PS_GET_CID_PARA_CNF, sizeof(TAFAGENT_PS_GetCidParaCnf), VOS_NULL_PTR },
};

static TAF_ChkMsgLenNameMapTbl g_acpuTafAgentChkMtaMsgLenTbl[] = {
    { ID_TAFAGENT_MTA_GET_SYSMODE_CNF, sizeof(TAFAGENT_MTA_GetSysmodeCnf), VOS_NULL_PTR },
    { ID_TAFAGENT_MTA_GET_ANT_STATE_CNF, sizeof(TAFAGENT_MTA_GetAntStateCnf), VOS_NULL_PTR },
    { ID_TAFAGENT_MTA_WRITE_ACORE_NV_CNF, sizeof(TAFAGENT_MTA_WriteAcoreNvCnf), VOS_NULL_PTR },
    { ID_TAFAGENT_MTA_PROC_ACORE_NV_CNF, sizeof(TAFAGENT_MTA_ProcAcoreNvCnf), VOS_NULL_PTR },
    { ID_TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF, 0, TAF_AGENT_ChkMtaReadAcoreNvCnfMsgLen }
};

static TAF_ChkMsgLenSndPidMapTbl g_acpuTafAgentChkCcoreMsgLenTbl[] = {
    { I0_UEPS_PID_MTA, TAF_AGENT_GetChkMtaMsgLenTblAddr, TAF_AGENT_GetChkMtaMsgLenTblSize },
    { I1_UEPS_PID_MTA, TAF_AGENT_GetChkMtaMsgLenTblAddr, TAF_AGENT_GetChkMtaMsgLenTblSize },
    { I2_UEPS_PID_MTA, TAF_AGENT_GetChkMtaMsgLenTblAddr, TAF_AGENT_GetChkMtaMsgLenTblSize },
    { I0_UEPS_PID_DSM, TAF_AGENT_GetChkDsmMsgLenTblAddr, TAF_AGENT_GetChkDsmMsgLenTblSize },
    { I1_UEPS_PID_DSM, TAF_AGENT_GetChkDsmMsgLenTblAddr, TAF_AGENT_GetChkDsmMsgLenTblSize },
    { I2_UEPS_PID_DSM, TAF_AGENT_GetChkDsmMsgLenTblAddr, TAF_AGENT_GetChkDsmMsgLenTblSize },
    { I0_UEPS_PID_CCM, TAF_AGENT_GetChkCcmMsgLenTblAddr, TAF_AGENT_GetChkCcmMsgLenTblSize },
    { I1_UEPS_PID_CCM, TAF_AGENT_GetChkCcmMsgLenTblAddr, TAF_AGENT_GetChkCcmMsgLenTblSize },
    { I2_UEPS_PID_CCM, TAF_AGENT_GetChkCcmMsgLenTblAddr, TAF_AGENT_GetChkCcmMsgLenTblSize }
};

/* 列表ACPU_FID_TAFAGENT下各PID接收消息长度检查处理映射表: 第一列:消息接收PID，第二列:指定接收PID的消息长度检查表 */
static TAF_ChkMsgLenRcvPidMapTbl g_acpuTafAgentFidChkMsgLenTbl[] = {
    TAF_MSG_CHECK_TBL_ITEM(ACPU_PID_TAFAGENT, g_acpuTafAgentChkCcoreMsgLenTbl),
};

STATIC VOS_UINT32 TAF_AGENT_ChkMtaReadAcoreNvCnfMsgLen(const MSG_Header *msgHeader)
{
    const TAFAGENT_MTA_ReadAcoreEncryptionNvCnf *cnf = VOS_NULL_PTR;
    VOS_UINT32                                   minMsgLen;

    minMsgLen = sizeof(TAFAGENT_MTA_ReadAcoreEncryptionNvCnf) - TAFAGENT_MTA_READ_ACORE_ENCRYPTION_NV_CNF_DATA_ARRAY_LEN;
    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < minMsgLen) {
        return VOS_FALSE;
    }
    cnf = (const TAFAGENT_MTA_ReadAcoreEncryptionNvCnf *)msgHeader;

    if ((VOS_GET_MSG_LEN(msgHeader) + VOS_MSG_HEAD_LENGTH) < (minMsgLen + cnf->nvLength)) {
        return VOS_FALSE;
    }

    return VOS_TRUE;
}

STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkDsmMsgLenTblAddr(VOS_VOID)
{
    return g_acpuTafAgentChkDsmMsgLenTbl;
}

STATIC VOS_UINT32 TAF_AGENT_GetChkDsmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_acpuTafAgentChkDsmMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkMtaMsgLenTblAddr(VOS_VOID)
{
    return g_acpuTafAgentChkMtaMsgLenTbl;
}

STATIC VOS_UINT32 TAF_AGENT_GetChkMtaMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_acpuTafAgentChkMtaMsgLenTbl);
}

STATIC TAF_ChkMsgLenNameMapTbl* TAF_AGENT_GetChkCcmMsgLenTblAddr(VOS_VOID)
{
    return g_acpuTafAgentChkCcmMsgLenTbl;
}

STATIC VOS_UINT32 TAF_AGENT_GetChkCcmMsgLenTblSize(VOS_VOID)
{
    return TAF_GET_ARRAY_NUM(g_acpuTafAgentChkCcmMsgLenTbl);
}

VOS_UINT32 TAF_AGENT_ChkTafAgentFidRcvMsgLen(const MsgBlock *msg)
{
    return TAF_ChkMsgLen(msg, g_acpuTafAgentFidChkMsgLenTbl, TAF_GET_ARRAY_NUM(g_acpuTafAgentFidChkMsgLenTbl));
}

VOS_VOID TAF_AGENT_SortTafAgentFidChkMsgLenTbl(VOS_VOID)
{
    TAF_SortChkMsgLenTblByMsgName(g_acpuTafAgentChkMtaMsgLenTbl, TAF_GET_ARRAY_NUM(g_acpuTafAgentChkMtaMsgLenTbl));
}



