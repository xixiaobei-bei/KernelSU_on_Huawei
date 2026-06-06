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
#include "TafIfaceApi.h"




/*****************************************************************************
    协议栈打印打点方式下的.C文件宏定义
*****************************************************************************/
#define    THIS_FILE_ID        PS_FILE_ID_TAF_IFACE_API_C


/*****************************************************************************
   2 全局变量定义
*****************************************************************************/

/*****************************************************************************
   3 外部函数声明
*****************************************************************************/

/*****************************************************************************
   4 函数实现
*****************************************************************************/

VOS_UINT32 TAF_IFACE_SndDsmMsg(
    const VOS_UINT16                    usModemId,
    const VOS_UINT32                    ulMsgId,
    const VOS_VOID                     *pData,
    const VOS_UINT32                    ulLength
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
            PS_PRINTF("TAF_IFACE_SndDsmMsg: ModemId is Error!");
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
    if (VOS_OK != PS_SEND_MSG(ulSendPid, pstMsg))
    {
        return VOS_ERR;
    }

    return VOS_OK;
}


MODULE_EXPORTED VOS_UINT32 TAF_IFACE_Up(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_UP_STRU            *pstIfaceUp
)
{
    VOS_UINT32                          ulResult;
    TAF_IFACE_UP_REQ_STRU               stIfaceUpReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stIfaceUpReq, sizeof(stIfaceUpReq), 0x00, sizeof(TAF_IFACE_UP_REQ_STRU));

    /* 构造ID_MSG_TAF_IFACE_UP_REQ消息 */
    stIfaceUpReq.stCtrl.ulModuleId = pstCtrl->ulModuleId;
    stIfaceUpReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(pstCtrl->usClientId);
    stIfaceUpReq.stCtrl.ucOpId     = pstCtrl->ucOpId;

    TAF_MEM_CPY_S(&stIfaceUpReq.stIfaceUp, sizeof(stIfaceUpReq.stIfaceUp), pstIfaceUp, sizeof(TAF_IFACE_UP_STRU));

    /* 发送消息 */
    ulResult = TAF_IFACE_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(pstCtrl->usClientId),
                                ID_MSG_TAF_IFACE_UP_REQ,
                                &stIfaceUpReq,
                                sizeof(TAF_IFACE_UP_REQ_STRU));

    return ulResult;
}


MODULE_EXPORTED VOS_UINT32 TAF_IFACE_Down(
    const TAF_CTRL_STRU                *pstCtrl,
    const TAF_IFACE_DOWN_STRU          *pstIfaceDown
)
{
    VOS_UINT32                          ulResult;
    TAF_IFACE_DOWN_REQ_STRU             stIfaceDownReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stIfaceDownReq, sizeof(stIfaceDownReq), 0x00, sizeof(TAF_IFACE_DOWN_REQ_STRU));

    /* 构造ID_MSG_TAF_IFACE_DOWN_REQ消息 */
    stIfaceDownReq.stCtrl.ulModuleId = pstCtrl->ulModuleId;
    stIfaceDownReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(pstCtrl->usClientId);
    stIfaceDownReq.stCtrl.ucOpId     = pstCtrl->ucOpId;

    TAF_MEM_CPY_S(&stIfaceDownReq.stIfaceDown, sizeof(stIfaceDownReq.stIfaceDown), pstIfaceDown, sizeof(TAF_IFACE_DOWN_STRU));

    /* 发送消息 */
    ulResult = TAF_IFACE_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(pstCtrl->usClientId),
                                ID_MSG_TAF_IFACE_DOWN_REQ,
                                &stIfaceDownReq,
                                sizeof(TAF_IFACE_DOWN_REQ_STRU));

    return ulResult;
}


VOS_UINT32 TAF_IFACE_GetDynamicPara(
    const TAF_CTRL_STRU                *pstCtrl,
    const VOS_UINT32                    ulBitCid
)
{
    VOS_UINT32                          ulResult;
    TAF_IFACE_GET_DYNAMIC_PARA_REQ_STRU stDynamicParaReq;

    /* 初始化 */
    ulResult = VOS_OK;
    TAF_MEM_SET_S(&stDynamicParaReq,sizeof(stDynamicParaReq), 0x00, sizeof(TAF_IFACE_GET_DYNAMIC_PARA_REQ_STRU));

    /* 构造ID_MSG_TAF_IFACE_GET_DYNAMIC_INFO_REQ消息 */
    stDynamicParaReq.stCtrl.ulModuleId = pstCtrl->ulModuleId;
    stDynamicParaReq.stCtrl.usClientId = TAF_PS_GET_CLIENTID_FROM_EXCLIENTID(pstCtrl->usClientId);
    stDynamicParaReq.stCtrl.ucOpId     = pstCtrl->ucOpId;
    stDynamicParaReq.ulBitCid          = ulBitCid;

    /* 发送消息 */
    ulResult = TAF_IFACE_SndDsmMsg(TAF_PS_GET_MODEMID_FROM_EXCLIENTID(pstCtrl->usClientId),
                                ID_MSG_TAF_GET_IFACE_DYNAMIC_PARA_REQ,
                                &stDynamicParaReq,
                                sizeof(TAF_IFACE_GET_DYNAMIC_PARA_REQ_STRU));

    return ulResult;
}

