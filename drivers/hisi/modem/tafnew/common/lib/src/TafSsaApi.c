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
#include "PsCommonDef.h"
#include "TafSsaApi.h"
#include "MnClient.h"





/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define THIS_FILE_ID                    PS_FILE_ID_TAF_SSA_API_C


/*****************************************************************************
   2 全局变量定义
*****************************************************************************/


/*****************************************************************************
   3 外部函数声明
*****************************************************************************/

extern VOS_UINT32 AT_GetDestPid(
    MN_CLIENT_ID_T                      usClientId,
    VOS_UINT32                          ulRcvPid
);


/*****************************************************************************
   4 函数实现
*****************************************************************************/

VOS_VOID TAF_SSA_SndTafMsg(
    TAF_SSA_MSG_ID_ENUM_UINT32          enMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLength
)
{
    TAF_SSA_MSG_STRU                   *pstMsg   = VOS_NULL_PTR;
    VOS_UINT32                          ulRealPid;
    /* 填写消息头 */
    ulRealPid = AT_GetDestPid(((TAF_CTRL_STRU *)pData)->usClientId, WUEPS_PID_TAF);

    /* 构造消息 */
    pstMsg = (TAF_SSA_MSG_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                ulRealPid,
                                sizeof(MSG_HEADER_STRU) + ulLength);
    if (VOS_NULL_PTR == pstMsg)
    {
        return;
    }

    pstMsg->stHeader.ulReceiverPid      = ulRealPid;
    pstMsg->stHeader.ulMsgName          = enMsgId;

    /* 填写消息内容 */
    TAF_MEM_CPY_S(pstMsg->aucContent, ulLength, pData, ulLength);

    /* 发送消息 */
    (VOS_VOID)PS_SEND_MSG(ulRealPid, pstMsg);

    return;
}


VOS_VOID TAF_SSA_SetCmolrInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    TAF_SSA_LCS_MOLR_PARA_SET_STRU     *pstMolrPara
)
{
    TAF_SSA_SET_LCS_MOLR_REQ_STRU       stSetCmolrReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stSetCmolrReq, sizeof(stSetCmolrReq), 0x00, sizeof(stSetCmolrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stSetCmolrReq, ulModuleId, usClientId, ucOpId);

    stSetCmolrReq.stMolrPara  = *pstMolrPara;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MOLR_REQ,
                      &stSetCmolrReq,
                      sizeof(stSetCmolrReq));

    return;
}


VOS_VOID TAF_SSA_GetCmolrInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_SSA_GET_LCS_MOLR_REQ_STRU       stGetCmolrReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stGetCmolrReq, sizeof(stGetCmolrReq), 0x00, sizeof(stGetCmolrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stGetCmolrReq, ulModuleId, usClientId, ucOpId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MOLR_REQ,
                      &stGetCmolrReq,
                      sizeof(stGetCmolrReq));

    return;
}


VOS_VOID TAF_SSA_SetCmtlrInfo(
    VOS_UINT32                              ulModuleId,
    VOS_UINT16                              usClientId,
    VOS_UINT8                               ucOpId,
    TAF_SSA_LCS_MTLR_SUBSCRIBE_ENUM_UINT8   enSubscribe
)
{
    TAF_SSA_SET_LCS_MTLR_REQ_STRU       stSetCmtlrReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stSetCmtlrReq, sizeof(stSetCmtlrReq), 0x00, sizeof(stSetCmtlrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stSetCmtlrReq, ulModuleId, usClientId, ucOpId);

    stSetCmtlrReq.enSubscribe   = enSubscribe;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MTLR_REQ,
                      &stSetCmtlrReq,
                      sizeof(stSetCmtlrReq));

    return;
}


VOS_VOID TAF_SSA_GetCmtlrInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_SSA_GET_LCS_MTLR_REQ_STRU       stGetCmtlrReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stGetCmtlrReq, sizeof(stGetCmtlrReq), 0x00, sizeof(stGetCmtlrReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stGetCmtlrReq, ulModuleId, usClientId, ucOpId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MTLR_REQ,
                      &stGetCmtlrReq,
                      sizeof(stGetCmtlrReq));

    return;
}


VOS_VOID TAF_SSA_SetCmtlraInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId,
    TAF_SSA_LCS_MTLRA_PARA_SET_STRU    *pstCmtlraPara
)
{
    TAF_SSA_SET_LCS_MTLRA_REQ_STRU      stSetCmtlraReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stSetCmtlraReq, sizeof(stSetCmtlraReq), 0x00, sizeof(stSetCmtlraReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stSetCmtlraReq, ulModuleId, usClientId, ucOpId);

    stSetCmtlraReq.stCmtlraPara  = *pstCmtlraPara;

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_SET_LCS_MTLRA_REQ,
                      &stSetCmtlraReq,
                      sizeof(stSetCmtlraReq));

    return;
}


VOS_VOID TAF_SSA_GetCmtlraInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_SSA_GET_LCS_MTLRA_REQ_STRU      stGetCmtlraReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stGetCmtlraReq, sizeof(stGetCmtlraReq), 0x00, sizeof(stGetCmtlraReq));

    /* 填写CTRL信息 */
    TAF_API_CTRL_HEADER(&stGetCmtlraReq, ulModuleId, usClientId, ucOpId);

    /* 发送消息 */
    TAF_SSA_SndTafMsg(ID_TAF_SSA_GET_LCS_MTLRA_REQ,
                      &stGetCmtlraReq,
                      sizeof(stGetCmtlraReq));

    return;
}







