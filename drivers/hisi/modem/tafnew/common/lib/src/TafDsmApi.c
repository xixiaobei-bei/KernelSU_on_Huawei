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
#include "TafPsApi.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_TAF_DSM_API_C


/*****************************************************************************
   2 全局变量定义
*****************************************************************************/

/*****************************************************************************
   3 外部函数声明
*****************************************************************************/

/*****************************************************************************
   4 函数实现
*****************************************************************************/

VOS_UINT32 TAF_PS_SndDsmMsg(
    VOS_UINT16                          usModemId,
    VOS_UINT32                          ulMsgId,
    VOS_VOID                           *pData,
    VOS_UINT32                          ulLength
)
{
    TAF_PS_MSG_STRU                    *pstMsg   = VOS_NULL_PTR;
    VOS_UINT32                          ulSendPid;
    VOS_UINT32                          ulRcvPid;

    switch (usModemId)
    {
        case MODEM_ID_0:
            ulSendPid = I0_WUEPS_PID_TAF;
            ulRcvPid  = I0_UEPS_PID_DSM;
            break;

        case MODEM_ID_1:
            ulSendPid = I1_WUEPS_PID_TAF;
            ulRcvPid  = I1_UEPS_PID_DSM;
            break;

        case MODEM_ID_2:
            ulSendPid = I2_WUEPS_PID_TAF;
            ulRcvPid  = I2_UEPS_PID_DSM;
            break;

        default:
            PS_PRINTF("TAF_PS_SndDsmMsg: ModemId is Error!");
            return VOS_ERR;
    }

    /* 构造消息 */
    pstMsg = (TAF_PS_MSG_STRU*)PS_ALLOC_MSG_WITH_HEADER_LEN(
                                ulSendPid,
                                sizeof(MSG_HEADER_STRU) + ulLength);
    if (VOS_NULL_PTR == pstMsg)
    {
        return VOS_ERR;
    }

    pstMsg->stHeader.ulSenderPid        = ulSendPid;
    pstMsg->stHeader.ulReceiverPid      = ulRcvPid;
    pstMsg->stHeader.ulMsgName          = ulMsgId;

    /* 填写消息内容 */
    TAF_MEM_CPY_S(pstMsg->aucContent, ulLength, pData, ulLength);

    /* 发送消息 */
    (VOS_VOID)PS_SEND_MSG(ulSendPid, pstMsg);

    return VOS_OK;
}


VOS_UINT32 TAF_PS_SetPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_PRIM_CONTEXT_EXT_STRU      *pstPdpContextInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stSetPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetPdpCtxInfoReq, sizeof(stSetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stSetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpCtxInfoReq.stPdpContextInfo  = *pstPdpContextInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stSetPdpCtxInfoReq,
                                sizeof(TAF_PS_SET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stGetPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU        stGetPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDPCONT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_SEC_CONTEXT_EXT_STRU       *pstPdpContextInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU    stSetPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetPdpCtxInfoReq, sizeof(stSetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ消息 */
    stSetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpCtxInfoReq.stPdpContextInfo  = *pstPdpContextInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ,
                                &stSetPdpCtxInfoReq,
                                sizeof(TAF_PS_SET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU    stGetPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpCtxInfoReq, sizeof(stGetPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ消息 */
    stGetPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ,
                                &stGetPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_TFT_EXT_STRU                   *pstTftInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_TFT_INFO_REQ_STRU        stSetTftInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetTftInfoReq, sizeof(stSetTftInfoReq), 0x00, sizeof(TAF_PS_SET_TFT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_TFT_INFO_REQ消息 */
    stSetTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetTftInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetTftInfoReq.stTftInfo         = *pstTftInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_TFT_INFO_REQ,
                                &stSetTftInfoReq,
                                sizeof(TAF_PS_SET_TFT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_TFT_INFO_REQ_STRU        stGetTftInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetTftInfoReq, sizeof(stGetTftInfoReq), 0x00, sizeof(TAF_PS_GET_TFT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_SEC_PDP_CONTEXT_INFO_REQ消息 */
    stGetTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetTftInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_TFT_INFO_REQ,
                                &stGetTftInfoReq,
                                sizeof(TAF_PS_GET_TFT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_UMTS_QOS_EXT_STRU              *pstUmtsQosInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU   stSetUmtsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetUmtsQosInfoReq, sizeof(stSetUmtsQosInfoReq), 0x00, sizeof(TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_UMTS_QOS_INFO_REQ消息 */
    stSetUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetUmtsQosInfoReq.stUmtsQosInfo     = *pstUmtsQosInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_UMTS_QOS_INFO_REQ,
                                &stSetUmtsQosInfoReq,
                                sizeof(TAF_PS_SET_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU   stGetUmtsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetUmtsQosInfoReq, sizeof(stGetUmtsQosInfoReq), 0x00, sizeof(TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_UMTS_QOS_INFO_REQ消息 */
    stGetUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_UMTS_QOS_INFO_REQ,
                                &stGetUmtsQosInfoReq,
                                sizeof(TAF_PS_GET_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetUmtsQosMinInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_UMTS_QOS_EXT_STRU              *pstUmtsQosMinInfo
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU   stSetUmtsQosMinInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetUmtsQosMinInfoReq, sizeof(stSetUmtsQosMinInfoReq), 0x00, sizeof(TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ消息 */
    stSetUmtsQosMinInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetUmtsQosMinInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetUmtsQosMinInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetUmtsQosMinInfoReq.stUmtsQosMinInfo  = *pstUmtsQosMinInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ,
                                &stSetUmtsQosMinInfoReq,
                                sizeof(TAF_PS_SET_UMTS_QOS_MIN_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetUmtsQosMinInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU   stGetUmtsQosMinInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetUmtsQosMinInfoReq, sizeof(stGetUmtsQosMinInfoReq), 0x00, sizeof(TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ消息 */
    stGetUmtsQosMinInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetUmtsQosMinInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetUmtsQosMinInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ,
                                &stGetUmtsQosMinInfoReq,
                                sizeof(TAF_PS_GET_UMTS_QOS_MIN_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicUmtsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU   stGetDynamicUmtsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDynamicUmtsQosInfoReq, sizeof(stGetDynamicUmtsQosInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ消息 */
    stGetDynamicUmtsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicUmtsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicUmtsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicUmtsQosInfoReq.stCidListInfo     = *pstCidListInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ,
                                &stGetDynamicUmtsQosInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_UMTS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetPdpContextState(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STATE_STRU            *pstCidListStateInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_PDP_STATE_REQ_STRU       stSetPdpStateReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetPdpStateReq, sizeof(stSetPdpStateReq), 0x00, sizeof(TAF_PS_SET_PDP_STATE_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_PDP_STATE_REQ消息 */
    stSetPdpStateReq.stCtrl.ulModuleId  = ulModuleId;
    stSetPdpStateReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpStateReq.stCtrl.ucOpId      = ucOpId;
    stSetPdpStateReq.stCidListStateInfo = *pstCidListStateInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PDP_CONTEXT_STATE_REQ,
                                &stSetPdpStateReq,
                                sizeof(TAF_PS_SET_PDP_STATE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpContextState(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_PDP_STATE_REQ_STRU       stGetPdpStateReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpStateReq, sizeof(stGetPdpStateReq), 0x00, sizeof(TAF_PS_GET_PDP_STATE_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_PDP_STATE_REQ消息 */
    stGetPdpStateReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpStateReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpStateReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_CONTEXT_STATE_REQ,
                                &stGetPdpStateReq,
                                sizeof(TAF_PS_GET_PDP_STATE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallModify(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_MODIFY_REQ_STRU         stCallModifyReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stCallModifyReq, sizeof(stCallModifyReq), 0x00, sizeof(TAF_PS_CALL_MODIFY_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CALL_MODIFY_REQ消息 */
    stCallModifyReq.stCtrl.ulModuleId = ulModuleId;
    stCallModifyReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallModifyReq.stCtrl.ucOpId     = ucOpId;
    stCallModifyReq.stCidListInfo     = *pstCidListInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_MODIFY_REQ,
                                &stCallModifyReq,
                                sizeof(TAF_PS_CALL_MODIFY_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallAnswer(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_ANSWER_STRU                 *pstAnsInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_ANSWER_REQ_STRU         stCallAnswerReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stCallAnswerReq, sizeof(stCallAnswerReq), 0x00, sizeof(TAF_PS_CALL_ANSWER_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CALL_ANSWER_REQ消息 */
    stCallAnswerReq.stCtrl.ulModuleId = ulModuleId;
    stCallAnswerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallAnswerReq.stCtrl.ucOpId     = ucOpId;
    stCallAnswerReq.stAnsInfo         = *pstAnsInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_ANSWER_REQ,
                                &stCallAnswerReq,
                                sizeof(TAF_PS_CALL_ANSWER_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_CallHangup(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_HANGUP_REQ_STRU         stCallHangupReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stCallHangupReq, sizeof(stCallHangupReq), 0x00, sizeof(TAF_PS_CALL_HANGUP_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CALL_HANGUP_REQ消息 */
    stCallHangupReq.stCtrl.ulModuleId = ulModuleId;
    stCallHangupReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallHangupReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_HANGUP_REQ,
                                &stCallHangupReq,
                                sizeof(TAF_PS_CALL_HANGUP_REQ_STRU));

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallOrig(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_DIAL_PARA_STRU              *pstDialParaInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_ORIG_REQ_STRU           stCallOrigReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stCallOrigReq, sizeof(stCallOrigReq), 0x00, sizeof(TAF_PS_CALL_ORIG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CALL_ORIG_REQ消息 */
    stCallOrigReq.stCtrl.ulModuleId = ulModuleId;
    stCallOrigReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallOrigReq.stCtrl.ucOpId     = ucOpId;

    TAF_MEM_CPY_S(&stCallOrigReq.stDialParaInfo,
               sizeof(stCallOrigReq.stDialParaInfo),
               pstDialParaInfo,
               sizeof(TAF_PS_DIAL_PARA_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_ORIG_REQ,
                                &stCallOrigReq,
                                sizeof(TAF_PS_CALL_ORIG_REQ_STRU));

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallEnd(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    return TAF_PS_CallEndEx(ulModuleId, usExClientId, ucOpId, ucCid, TAF_PS_CALL_END_CAUSE_NORMAL);
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_CallEndEx(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid,
    TAF_PS_CALL_END_CAUSE_ENUM_UINT8    enCause
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_CALL_END_REQ_STRU            stCallEndReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stCallEndReq, sizeof(stCallEndReq), 0x00, sizeof(TAF_PS_CALL_END_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CALL_END_REQ消息 */
    stCallEndReq.stCtrl.ulModuleId = ulModuleId;
    stCallEndReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stCallEndReq.stCtrl.ucOpId     = ucOpId;
    stCallEndReq.ucCid             = ucCid;
    stCallEndReq.enCause           = enCause;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CALL_END_REQ,
                                &stCallEndReq,
                                sizeof(TAF_PS_CALL_END_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpIpAddrInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_CID_LIST_STRU                  *pstCidListInfo
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU    stGetPdpIpAddrReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpIpAddrReq, sizeof(stGetPdpIpAddrReq), 0x00, sizeof(TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_PDP_IP_ADDR_INFO_REQ消息 */
    stGetPdpIpAddrReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpIpAddrReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpIpAddrReq.stCtrl.ucOpId     = ucOpId;
    stGetPdpIpAddrReq.stCidListInfo     = *pstCidListInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_IP_ADDR_INFO_REQ,
                                &stGetPdpIpAddrReq,
                                sizeof(TAF_PS_GET_PDP_IP_ADDR_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetAnsModeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT32                          ulAnsMode
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU    stSetAnsModeReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetAnsModeReq, sizeof(stSetAnsModeReq), 0x00, sizeof(TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ消息 */
    stSetAnsModeReq.stCtrl.ulModuleId = ulModuleId;
    stSetAnsModeReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetAnsModeReq.stCtrl.ucOpId     = ucOpId;
    stSetAnsModeReq.ulAnsMode         = ulAnsMode;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ,
                                &stSetAnsModeReq,
                                sizeof(TAF_PS_SET_ANSWER_MODE_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetAnsModeInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU    stGetAnsModeReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetAnsModeReq, sizeof(stGetAnsModeReq), 0x00, sizeof(TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_ANSWER_MODE_INFO_REQ消息 */
    stGetAnsModeReq.stCtrl.ulModuleId   = ulModuleId;
    stGetAnsModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetAnsModeReq.stCtrl.ucOpId       = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_ANSWER_MODE_INFO_REQ,
                                &stGetAnsModeReq,
                                sizeof(TAF_PS_GET_ANSWER_MODE_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicPrimPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                          ulResult;
    TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU   stGetDynamicPrimPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDynamicPrimPdpCtxInfoReq, sizeof(stGetDynamicPrimPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ消息 */
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicPrimPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicPrimPdpCtxInfoReq.ucCid             = ucCid;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ,
                                &stGetDynamicPrimPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_PRIM_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicSecPdpContextInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                          ulResult;
    TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU    stGetDynamicSecPdpCtxInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDynamicSecPdpCtxInfoReq, sizeof(stGetDynamicSecPdpCtxInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ消息 */
    stGetDynamicSecPdpCtxInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicSecPdpCtxInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicSecPdpCtxInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicSecPdpCtxInfoReq.ucCid             = ucCid;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ,
                                &stGetDynamicSecPdpCtxInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_SEC_PDP_CONTEXT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicTftInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU    stGetDynamicTftInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDynamicTftInfoReq, sizeof(stGetDynamicTftInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DYNAMIC_TFT_INFO_REQ消息 */
    stGetDynamicTftInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicTftInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicTftInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicTftInfoReq.ucCid             = ucCid;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_TFT_INFO_REQ,
                                &stGetDynamicTftInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_TFT_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_EPS_QOS_EXT_STRU               *pstEpsQosInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_EPS_QOS_INFO_REQ_STRU    stSetEpsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetEpsQosInfoReq, sizeof(stSetEpsQosInfoReq), 0x00, sizeof(TAF_PS_SET_EPS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_EPS_QOS_INFO_REQ消息 */
    stSetEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetEpsQosInfoReq.stEpsQosInfo      = *pstEpsQosInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_EPS_QOS_INFO_REQ,
                                &stSetEpsQosInfoReq,
                                sizeof(TAF_PS_SET_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_EPS_QOS_INFO_REQ_STRU    stGetEpsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetEpsQosInfoReq, sizeof(stGetEpsQosInfoReq), 0x00, sizeof(TAF_PS_GET_EPS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_EPS_QOS_INFO_REQ消息 */
    stGetEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_EPS_QOS_INFO_REQ,
                                &stGetEpsQosInfoReq,
                                sizeof(TAF_PS_GET_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicEpsQosInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                                  ulResult;
    TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU    stGetDynamicEpsQosInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetDynamicEpsQosInfoReq, sizeof(stGetDynamicEpsQosInfoReq), 0x00, sizeof(TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ消息 */
    stGetDynamicEpsQosInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetDynamicEpsQosInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetDynamicEpsQosInfoReq.stCtrl.ucOpId     = ucOpId;
    stGetDynamicEpsQosInfoReq.ucCid             = ucCid;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ,
                                &stGetDynamicEpsQosInfoReq,
                                sizeof(TAF_PS_GET_DYNAMIC_EPS_QOS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetPdpDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PDP_DNS_EXT_STRU               *pstPdpDnsInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_PDP_DNS_INFO_REQ_STRU    stSetPdpDnsInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetPdpDnsInfoReq, sizeof(stSetPdpDnsInfoReq), 0x00, sizeof(TAF_PS_SET_PDP_DNS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ消息 */
    stSetPdpDnsInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetPdpDnsInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPdpDnsInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetPdpDnsInfoReq.stPdpDnsInfo      = *pstPdpDnsInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_PDP_DNS_INFO_REQ,
                                &stSetPdpDnsInfoReq,
                                sizeof(TAF_PS_SET_PDP_DNS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetPdpDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_PDP_DNS_INFO_REQ_STRU    stGetPdpDnsInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetPdpDnsInfoReq, sizeof(stGetPdpDnsInfoReq), 0x00, sizeof(TAF_PS_GET_PDP_DNS_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_PDP_DNS_INFO_REQ消息 */
    stGetPdpDnsInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetPdpDnsInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPdpDnsInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_PDP_DNS_INFO_REQ,
                                &stGetPdpDnsInfoReq,
                                sizeof(TAF_PS_GET_PDP_DNS_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ConfigNbnsFunction(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT32                          ulEnabled
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU    stConfigNbnsFunReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stConfigNbnsFunReq, sizeof(stConfigNbnsFunReq), 0x00, sizeof(TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_CONFIG_NBNS_FUNCTION_REQ消息 */
    stConfigNbnsFunReq.stCtrl.ulModuleId = ulModuleId;
    stConfigNbnsFunReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stConfigNbnsFunReq.stCtrl.ucOpId     = ucOpId;
    stConfigNbnsFunReq.ulEnabled         = ulEnabled;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_CONFIG_NBNS_FUNCTION_REQ,
                                &stConfigNbnsFunReq,
                                sizeof(TAF_PS_CONFIG_NBNS_FUNCTION_REQ_STRU));

    return ulResult;
}



VOS_UINT32 TAF_PS_SetAuthDataInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_AUTHDATA_EXT_STRU              *pstAuthDataInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_AUTHDATA_INFO_REQ_STRU   stSetAuthDataInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetAuthDataInfoReq, sizeof(stSetAuthDataInfoReq), 0x00, sizeof(TAF_PS_SET_AUTHDATA_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ消息 */
    stSetAuthDataInfoReq.stCtrl.ulModuleId = ulModuleId;
    stSetAuthDataInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetAuthDataInfoReq.stCtrl.ucOpId     = ucOpId;
    stSetAuthDataInfoReq.stAuthDataInfo    = *pstAuthDataInfo;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_AUTHDATA_INFO_REQ,
                                &stSetAuthDataInfoReq,
                                sizeof(TAF_PS_SET_AUTHDATA_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetAuthDataInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_GET_AUTHDATA_INFO_REQ_STRU   stGetAuthDataInfoReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetAuthDataInfoReq, sizeof(stGetAuthDataInfoReq), 0x00, sizeof(TAF_PS_GET_AUTHDATA_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ消息 */
    stGetAuthDataInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetAuthDataInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetAuthDataInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ,
                                &stGetAuthDataInfoReq,
                                sizeof(TAF_PS_GET_AUTHDATA_INFO_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetGprsActiveType(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_ATD_PARA_STRU                  *pstAtdPara
)
{
    VOS_UINT32                              ulResult;
    TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU  stGetGprsActiveTypeReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stGetGprsActiveTypeReq, sizeof(stGetGprsActiveTypeReq), 0x00, sizeof(TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_AUTHDATA_INFO_REQ消息 */
    stGetGprsActiveTypeReq.stCtrl.ulModuleId = ulModuleId;
    stGetGprsActiveTypeReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetGprsActiveTypeReq.stCtrl.ucOpId     = ucOpId;
    stGetGprsActiveTypeReq.stAtdPara         = *pstAtdPara;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ,
                                &stGetGprsActiveTypeReq,
                                sizeof(TAF_PS_GET_D_GPRS_ACTIVE_TYPE_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_PppDialOrig(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid,
    TAF_PPP_REQ_CONFIG_INFO_STRU       *pstPppReqConfigInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_PPP_DIAL_ORIG_REQ_STRU       stPppDialOrigReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stPppDialOrigReq, sizeof(stPppDialOrigReq), 0x00, sizeof(TAF_PS_PPP_DIAL_ORIG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ消息 */
    stPppDialOrigReq.stCtrl.ulModuleId          = ulModuleId;
    stPppDialOrigReq.stCtrl.usClientId          = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stPppDialOrigReq.stCtrl.ucOpId              = ucOpId;

    stPppDialOrigReq.stPppDialParaInfo.ucCid    = ucCid;

    stPppDialOrigReq.stPppDialParaInfo.bitOpPppConfigInfo = VOS_TRUE;
    TAF_MEM_CPY_S(&(stPppDialOrigReq.stPppDialParaInfo.stPppReqConfigInfo),
               sizeof(stPppDialOrigReq.stPppDialParaInfo.stPppReqConfigInfo),
               pstPppReqConfigInfo,
               sizeof(TAF_PPP_REQ_CONFIG_INFO_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_PPP_DIAL_ORIG_REQ,
                                &stPppDialOrigReq,
                                sizeof(TAF_PS_PPP_DIAL_ORIG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetDynamicDnsInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucCid
)
{
    VOS_UINT32                          ulResult;

    TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU stNegoDns;

    /* 初始化 */
    TAF_MEM_SET_S(&stNegoDns, sizeof(stNegoDns), 0x00, sizeof(TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU));

    /* 填充消息结构 */
    stNegoDns.stCtrl.ulModuleId = ulModuleId;
    stNegoDns.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stNegoDns.stCtrl.ucOpId     = ucOpId;
    stNegoDns.ucCid             = ucCid;

    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_NEGOTIATION_DNS_REQ,
                                &stNegoDns,
                                sizeof(TAF_PS_GET_NEGOTIATION_DNS_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetCtaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucTimeLen
)
{
    TAF_PS_SET_CTA_INFO_REQ_STRU        stSetPktCdataInactivityTmrlenReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stSetPktCdataInactivityTmrlenReq, sizeof(TAF_PS_SET_CTA_INFO_REQ_STRU), 0x00, sizeof(TAF_PS_SET_CTA_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_DSFLOW_WRITE_NV_CFG_REQ消息 */
    stSetPktCdataInactivityTmrlenReq.stCtrl.ulModuleId  = ulModuleId;
    stSetPktCdataInactivityTmrlenReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetPktCdataInactivityTmrlenReq.stCtrl.ucOpId      = ucOpId;


    stSetPktCdataInactivityTmrlenReq.ucPktCdataInactivityTmrLen = ucTimeLen;


    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_CTA_INFO_REQ,
                                &stSetPktCdataInactivityTmrlenReq,
                                sizeof(stSetPktCdataInactivityTmrlenReq));

    return ulResult;
}

VOS_UINT32 TAF_PS_GetCtaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_CTA_INFO_REQ_STRU        stGetPktCdataInactivityTmrlenReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stGetPktCdataInactivityTmrlenReq, sizeof(TAF_PS_GET_CTA_INFO_REQ_STRU), 0x00, sizeof(TAF_PS_GET_CTA_INFO_REQ_STRU));

    stGetPktCdataInactivityTmrlenReq.stCtrl.ulModuleId  = ulModuleId;
    stGetPktCdataInactivityTmrlenReq.stCtrl.usClientId  = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetPktCdataInactivityTmrlenReq.stCtrl.ucOpId      = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_CTA_INFO_REQ,
                                &stGetPktCdataInactivityTmrlenReq,
                                sizeof(stGetPktCdataInactivityTmrlenReq));

    return ulResult;
}





VOS_UINT32 TAF_PS_SetImsPdpCfg(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_IMS_PDP_CFG_STRU               *pstImsPdpCfg
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_IMS_PDP_CFG_REQ_STRU     stSetImsPdpCfgReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stSetImsPdpCfgReq, sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU), 0x00, sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_IMS_PDP_CFG_REQ消息 */
    stSetImsPdpCfgReq.stCtrl.ulModuleId = ulModuleId;
    stSetImsPdpCfgReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetImsPdpCfgReq.stCtrl.ucOpId     = ucOpId;
    stSetImsPdpCfgReq.stImsPdpCfg       = *pstImsPdpCfg;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_IMS_PDP_CFG_REQ,
                                &stSetImsPdpCfgReq,
                                sizeof(TAF_PS_SET_IMS_PDP_CFG_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_SetCdmaDormantTimer(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucDormantTimer
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SET_1X_DORM_TIMER_REQ_STRU   st1xDormTimerReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&st1xDormTimerReq, sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU), 0x00, sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_SET_1X_DORMANT_TIMER_REQ消息 */
    st1xDormTimerReq.stCtrl.ulModuleId = ulModuleId;
    st1xDormTimerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    st1xDormTimerReq.stCtrl.ucOpId     = ucOpId;
    st1xDormTimerReq.ucDormantTimer    = ucDormantTimer;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_1X_DORM_TIMER_REQ,
                                &st1xDormTimerReq,
                                sizeof(TAF_PS_SET_1X_DORM_TIMER_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_PS_ProcCdmaDormTimerQryReq(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_1X_DORM_TIMER_REQ_STRU   stGet1xDormTimerReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stGet1xDormTimerReq, sizeof(TAF_PS_GET_1X_DORM_TIMER_REQ_STRU), 0x00, sizeof(TAF_PS_GET_1X_DORM_TIMER_REQ_STRU));

    stGet1xDormTimerReq.stCtrl.ulModuleId = ulModuleId;
    stGet1xDormTimerReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGet1xDormTimerReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_1X_DORM_TIEMR_REQ,
                                &stGet1xDormTimerReq,
                                sizeof(stGet1xDormTimerReq));

    return ulResult;
}




VOS_UINT32 TAF_PS_SetMipMode(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_MIP_MODE_ENUM_UINT8          enMipMode
)
{
    TAF_PS_SET_MIP_MODE_REQ_STRU        stSetMipModeReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stSetMipModeReq, sizeof(TAF_PS_SET_MIP_MODE_REQ_STRU), 0x00, sizeof(TAF_PS_SET_MIP_MODE_REQ_STRU));

    stSetMipModeReq.stCtrl.ulModuleId   = ulModuleId;
    stSetMipModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetMipModeReq.stCtrl.ucOpId       = ucOpId;
    stSetMipModeReq.enMipMode           = enMipMode;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_MIP_MODE_REQ,
                                &stSetMipModeReq,
                                sizeof(stSetMipModeReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetMipMode(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_MIP_MODE_REQ_STRU        stGetMipModeReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stGetMipModeReq, sizeof(TAF_PS_GET_MIP_MODE_REQ_STRU), 0x00, sizeof(TAF_PS_GET_MIP_MODE_REQ_STRU));

    stGetMipModeReq.stCtrl.ulModuleId   = ulModuleId;
    stGetMipModeReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetMipModeReq.stCtrl.ucOpId       = ucOpId;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_MIP_MODE_REQ,
                                &stGetMipModeReq,
                                sizeof(stGetMipModeReq));

    return ulResult;
}




MODULE_EXPORTED VOS_UINT32 TAF_PS_GetCidSdfParaInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_SDF_PARA_QUERY_INFO_STRU       *pstSdfQueryInfo
)
{
    VOS_UINT32                          ulResult;
    TAF_PS_SDF_INFO_REQ_STRU            stSdfInfoReq;

    ulResult  = VOS_ERR;
    TAF_MEM_SET_S(&stSdfInfoReq, sizeof(stSdfInfoReq), 0x00, sizeof(TAF_PS_SDF_INFO_REQ_STRU));


    if (VOS_NULL_PTR == pstSdfQueryInfo)
    {
        /* 目前暂不支持异步方式 */
        return VOS_ERR;
    }

    TAF_MEM_SET_S(pstSdfQueryInfo, sizeof(TAF_SDF_PARA_QUERY_INFO_STRU), 0x00, sizeof(TAF_SDF_PARA_QUERY_INFO_STRU));

/* 同步方式目前仅支持C核 */

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_PS_GetUnusedCid(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                          *puCid
)
{
    VOS_UINT32                          ulResult;
    MODEM_ID_ENUM_UINT16                enModemId;

    enModemId = TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId);

    if (enModemId >= MODEM_ID_BUTT)
    {
        PS_PRINTF("TAF_PS_GetUnusedCid: ModemId is Error!");

        return VOS_ERR;
    }

    ulResult = VOS_OK;

    if (VOS_NULL_PTR == puCid)
    {
        /* 目前暂不支持异步方式 */
        return VOS_ERR;
    }

/* 同步方式目前仅支持C核 */

    return ulResult;
}


VOS_UINT32 TAF_PS_SetVzwApneInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucIsCustomCmd,
    TAF_PS_VZWAPNE_INFO_STRU           *pstVzwapneInfo
)
{
    TAF_PS_SET_VZWAPNE_REQ_STRU         stSetVzwApneReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stSetVzwApneReq, sizeof(stSetVzwApneReq), 0x00, sizeof(stSetVzwApneReq));

    stSetVzwApneReq.stCtrl.ulModuleId   = ulModuleId;
    stSetVzwApneReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetVzwApneReq.stCtrl.ucOpId       = ucOpId;
    stSetVzwApneReq.ucIsCustomCmd       = ucIsCustomCmd;

    TAF_MEM_CPY_S(&stSetVzwApneReq.stVzwApneInfo,
                  sizeof(TAF_PS_VZWAPNE_INFO_STRU),
                  pstVzwapneInfo,
                  sizeof(TAF_PS_VZWAPNE_INFO_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_VZWAPNE_REQ,
                                &stSetVzwApneReq,
                                sizeof(stSetVzwApneReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetVzwApneInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    VOS_UINT8                           ucIsCustomCmd
)
{
    TAF_PS_GET_VZWAPNE_REQ_STRU         stGetVzwApneReq;
    VOS_UINT32                          ulResult;

    TAF_MEM_SET_S(&stGetVzwApneReq, sizeof(stGetVzwApneReq), 0x00, sizeof(stGetVzwApneReq));

    stGetVzwApneReq.stCtrl.ulModuleId   = ulModuleId;
    stGetVzwApneReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetVzwApneReq.stCtrl.ucOpId       = ucOpId;
    stGetVzwApneReq.ucIsCustomCmd       = ucIsCustomCmd;

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_GET_VZWAPNE_REQ,
                                &stGetVzwApneReq,
                                sizeof(stGetVzwApneReq));

    return ulResult;
}



VOS_UINT32 TAF_PS_SetApnThrotInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId,
    TAF_PS_APN_THROT_INFO_STRU         *pstApnThrotInfo
)
{
    TAF_PS_SET_APN_THROT_INFO_REQ_STRU      stSetApnThrotInfoReq;
    VOS_UINT32                              ulResult;

    TAF_MEM_SET_S(&stSetApnThrotInfoReq, sizeof(stSetApnThrotInfoReq), 0x00, sizeof(TAF_PS_SET_APN_THROT_INFO_REQ_STRU));

    /* 填写消息内容 */
    stSetApnThrotInfoReq.stCtrl.ulModuleId   = ulModuleId;
    stSetApnThrotInfoReq.stCtrl.usClientId   = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stSetApnThrotInfoReq.stCtrl.ucOpId       = ucOpId;

    TAF_MEM_CPY_S(&(stSetApnThrotInfoReq.stApnThrotInfo),
                  sizeof(TAF_PS_APN_THROT_INFO_STRU),
                  pstApnThrotInfo,
                  sizeof(TAF_PS_APN_THROT_INFO_STRU));

    /* 发送消息 */
    ulResult = TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                                ID_MSG_TAF_PS_SET_APN_THROT_INFO_REQ,
                                &stSetApnThrotInfoReq,
                                sizeof(stSetApnThrotInfoReq));

    return ulResult;
}


VOS_UINT32 TAF_PS_GetLteAttchInfo(
    VOS_UINT32                          ulModuleId,
    VOS_UINT16                          usExClientId,
    VOS_UINT8                           ucOpId
)
{
    TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU stGetLteAttchInfoReq;

    /* 初始化 */
    TAF_MEM_SET_S(&stGetLteAttchInfoReq, sizeof(stGetLteAttchInfoReq), 0x00, sizeof(TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU));

    /* 构造ID_MSG_TAF_PS_GET_LTE_ATTACH_ESM_INFO_REQ消息 */
    stGetLteAttchInfoReq.stCtrl.ulModuleId = ulModuleId;
    stGetLteAttchInfoReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(usExClientId);
    stGetLteAttchInfoReq.stCtrl.ucOpId     = ucOpId;

    /* 发送消息 */
    return TAF_PS_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(usExClientId),
                            ID_MSG_TAF_PS_GET_LTE_ATTACH_INFO_REQ,
                            &stGetLteAttchInfoReq,
                            sizeof(TAF_PS_GET_LTE_ATTACH_INFO_REQ_STRU));

}

